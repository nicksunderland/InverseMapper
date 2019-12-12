#include "processing.h"
#include "igl/snap_points.h"
#include "igl/slice.h"
#include "Helper_Functions/inverse.h"
#include "Helper_Functions/forward.h"
#include "Helper_Functions/printmatrixinfo.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <fftw3.h>


Processing::Processing(QWidget *parent) :
    QWidget(parent),
    meshList( max_num_meshes )
{
    std::cout << "Processing --> ctor()" << std::endl;

    if( fftwf_init_threads() > 0 )
    {
        std::cout << "\tFFTW threads init SUCCESSFUL" << std::endl;
    }else {
        std::cout << "\tFFTW threads init FAILED" << std::endl;
    }

    std::cout << "Processing <-- ctor()" << std::endl;
}

void Processing::loadMesh( QString file_path, MeshType mesh_type )
{
    ///Create mesh object
    meshList[ mesh_type ] = QSharedPointer<Mesh>(new Mesh(mesh_type)); //create new mesh object and pointer to it, put into meshList vector;

    ///Emit signal that connects triggers connections between mesh and geomwindows (happens in Mainwindow.cpp)
    emit update_meshLists( meshList );

    ///Emiot signal to tell geomwindows to create some VAOs for the new mesh
    emit new_mesh_added_signal( mesh_type );

    ///Import the mesh data
    meshList[ mesh_type ]->importMesh( file_path, mesh_type );

    ///Extract some data for the inverse variable struct
    if( mesh_type == catheter )
    {
        inverse_vars.setNumCathChannels( meshList[ mesh_type ]->getMeshGeom().V.rows() );
    }else if( mesh_type == atrium )
    {
        inverse_vars.num_atrial_nodes = ( meshList[ mesh_type ]->getMeshGeom().V.rows() );
    }

}

void Processing::loadData( QString file_path, MeshType mesh_type )
{
    if( meshList[ mesh_type ] ) //not nullptr
    {
        meshList[ mesh_type ]->importData( file_path );
    }
}

void Processing::centreCatheter( )
{
    if( meshList[ catheter ] && meshList[ atrium ] ) //both meshes active
    {
        if( general_vars.mode == atrial_input )
        {
            meshList[ catheter ]->centreMesh( Eigen::RowVector3f::Zero() ); //move to origin

        }else if (  general_vars.mode == catheter_input  )
        {
            meshList[ catheter ]->centreMesh( meshList[ atrium ]->getMeshGeom().mesh_centre ); //move in relation to atrial centre
        }
    }
}

Eigen::VectorXi Processing::findNearestNodes(Eigen::VectorXi these_node_idxs, MeshType this_mesh_type, MeshType comparison_mesh_type )
{
    if( !meshList[ this_mesh_type ] || !meshList[ comparison_mesh_type ] )//mesh pointer is null
    {
        QMessageBox::warning( this, tr("Warning"), tr("Unable to find nearest node as no atrial and/or catheter mesh"), QMessageBox::Ok );
        return Eigen::VectorXi::Constant(1, -1);
    }

    Eigen::MatrixXf input_nodes; //Input mesh node
    Eigen::VectorXi closest_indices;
    Eigen::MatrixXf closest_point;//Outputs
    Eigen::VectorXf distance;
    if( meshList[ this_mesh_type ] && meshList[ comparison_mesh_type ] )
    {
        igl::slice( meshList[ this_mesh_type ]->getMeshGeom().V, these_node_idxs, 1, input_nodes );
        igl::snap_points( input_nodes, meshList[ comparison_mesh_type ]->getMeshGeom().V, closest_indices, distance, closest_point);
    }

    return closest_indices;
}

void Processing::processPick( QMouseEvent *event, int width, int height, glm::mat4 projMat, glm::mat4 viewMat, glm::vec3 cam_pos )
{
    if( !meshList[ atrium ] )//mesh pointer is null
    {
        QMessageBox::warning( this, tr("Warning"), tr("No atrial mesh to pick from"), QMessageBox::Ok );
        return ;
    }

    std::cout << "Processing --> processPick()" << std::endl;

    /* Get the mouse pixel coordinates from the screen click */
     float mouse_x = event->x();
     float mouse_y = event->y();

    /* 3D normalised x and y device coordinates, techincally don't need a z coordinate here */
     float x_ndc = ((2.0f*mouse_x) / width) - 1.0f;
     float y_ndc = 1.0f - (2.0f*mouse_y) / height;
     glm::vec2 ray_ndc = glm::vec2(x_ndc,y_ndc);

    /* 4D homogenous clip coordinates - negative z (pointing forwards) and add a positive w. Defines a
     * ray in clip space going from the click point into the screen */
     glm::vec4 ray_clip = glm::vec4(ray_ndc, -1.0, 1.0);

    /* 4D Eye/Camera coordinates - defines a ray in camera space by reversing the projection matrix
     * transformation using it's inverse */
     glm::vec4 ray_eye = glm::inverse(projMat) * ray_clip;

    /* Only need to unproject the x and y part; z needs to be -1.0 (forwards) and w needs
     * to be 0.0 (direction and not a point). */
     ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    /* Get the ray in world coordinates by using the inverse of the viewMatrix. Normalise this direction vector */
     glm::vec3 ray_world = (glm::inverse(viewMat) * ray_eye);
     ray_world = glm::normalize(ray_world);

    /* Also need to know where the camera is in terms of its world space coordinates - so transform the camera
     * position using the inverse of the viewMatrix */
     glm::vec3 cam_pos_trans = (glm::inverse(viewMat) * glm::vec4(cam_pos, 1.0));

    /* Create a vector to hold the picking hits and their minimum distances to the camera (we want to pick the
     * cloest hits to the camera if there are multiple hits */
     std::vector<int> pick_hit_list;
     std::vector<float> pick_distance;

    /* Cycle through each node in the atrial mesh and test where the ray intersects with an imaginary sphere
     * around each node. Each imaginary sphere has radius 1. See: http://antongerdelan.net/opengl/raycasting.html
     * tutorial for details */
     size_t num_vertices = meshList[ atrium ]->getMeshGeom().V.rows();

     for(size_t i=0; i < num_vertices; i++)
     {
         glm::vec3 node_centre = glm::vec3( meshList[ atrium ]->getMeshGeom().V(i,0),
                                            meshList[ atrium ]->getMeshGeom().V(i,1),
                                            meshList[ atrium ]->getMeshGeom().V(i,2)    );
         float b = glm::dot(ray_world, (cam_pos_trans-node_centre));
         float b_squared = glm::pow(b, 2);
         float c = glm::dot((cam_pos_trans-node_centre), (cam_pos_trans-node_centre)) - glm::pow(1,2);
         float b_squared_minus_c = b_squared - c;

         if( b_squared_minus_c >= 0 )
         {
             pick_hit_list.push_back(i);

             float distance_t1 = -b + glm::sqrt( (b_squared - c));
             float distance_t2 = -b - glm::sqrt( (b_squared - c));
             float distance_smallest = std::min(distance_t1, distance_t2);
             pick_distance.push_back(distance_smallest);
         }
     }
    /* Handle no hits */
     if( pick_hit_list.size() == 0)
     {
         std::cout << "Picking MISS" << std::endl;
         return;
     }
    /* Handle one hit */
     int hit_index=0;
     if( pick_hit_list.size() == 1)
     {
         std::cout << "Pick FOUND - index: " << pick_hit_list[0] << std::endl;
         hit_index = pick_hit_list[0];
     }
     if( pick_hit_list.size() > 1 )
     {
         int min_distance_vector_index = std::min_element(pick_distance.begin(),pick_distance.end()) - pick_distance.begin();
         std::cout << "Pick(s) FOUND - closest index: " << pick_hit_list[min_distance_vector_index] << std::endl;
         hit_index = pick_hit_list[min_distance_vector_index];
     }


     int nodeInds[ MeshType::max_num_meshes ];
     nodeInds[ atrium   ] = hit_index;
     nodeInds[ catheter ] = this->findNearestNodes( Eigen::VectorXi::Constant(1,hit_index), atrium, catheter )(0);
     std::cout << "\t About to create pick with indices[A" << nodeInds[ atrium   ] << ", C" << nodeInds[ catheter ] << "]" << std::endl;
     Pickwindow *pickwin = new Pickwindow( EMG_atrial, nodeInds, meshList, nullptr );
     pick_win_list.push_back(pickwin);
     emit new_pickwindown_added_signal( pickwin );

     std::cout << "Processing <-- processPick()" << std::endl;
}

void Processing::updatePickIndices()
{
    if( pick_win_list.size() != 0 )
    {
        //std::cout << "Processing --> updatePickIndices()" << std::endl;
        for( size_t i=0; i<pick_win_list.size(); i++ )
        {
            int new_nodeInds[ MeshType::max_num_meshes ];
            new_nodeInds[ atrium   ] = pick_win_list[i]->getChannelIndices( atrium );
            new_nodeInds[ catheter ] = this->findNearestNodes( Eigen::VectorXi::Constant(1,new_nodeInds[ atrium ]), atrium, catheter )(0);

            pick_win_list[i]->setChannelIndices( new_nodeInds );
        }

        emit pick_windows_changed_v2( pick_win_list );
       // std::cout << "Processing <-- updatePickIndices()" << std::endl;
    }
}

void Processing::noiseFilter()
{
    std::cout << "Processing --> noiseFilter()" << std::endl;

    if( meshList[ catheter ] ) //not nullptr
    {
        if( meshList[ catheter ]->hasNoData() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No catheter data to filter"), QMessageBox::Ok );
            return;
        }
        meshList[ catheter ]->bandstopFilterData( filter_vars );
    }

    std::cout << "Processing <-- noiseFilter()" << std::endl;
}

void Processing::findQRS()
{
    if( meshList[ catheter ] ) //not nullptr
    {
        if( meshList[ catheter ]->hasNoData() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No catheter data to use"), QMessageBox::Ok );
            return;
        }
        meshList[ catheter ]->findQRSactivations( filter_vars );

        if( !meshList[ catheter ]->hasQRSdata() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No QRS activations found"), QMessageBox::Ok );
            return;
        }
        helper::printMatrixInfo( meshList[ catheter ]->meshData.data[ QRS_actis ], "findQRS result" );
    }
}

void Processing::subtractQRS()
{
    if( meshList[ catheter ] ) //not nullptr
    {
        if( meshList[ catheter ]->hasNoData() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No catheter data to use"), QMessageBox::Ok );
            return;
        }
        if( !meshList[ catheter ]->hasQRSdata() )
        {
            this->findQRS();

            if( !meshList[ catheter ]->hasQRSdata() )
            {
                return;
            }
        }
        meshList[ catheter ]->subtractQRSactivations( filter_vars );

        helper::printMatrixInfo( meshList[ catheter ]->meshData.data[ processed ], "subtractQRS result" );
    }
}

void Processing::findCathActivations()
{
    if( meshList[ catheter ] ) //not nullptr
    {
        if( meshList[ catheter ]->hasNoData() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No catheter data to use"), QMessageBox::Ok );
            return;
        }
        meshList[ catheter ]->findAtrialActivations( filter_vars );

        if( !meshList[ catheter ]->hasAtrialActiData() ) //none found
        {
            QMessageBox::warning( this, tr("Warning"), tr("No catheter activations found"), QMessageBox::Ok );
            return;
        }
        helper::printMatrixInfo( meshList[ catheter ]->meshData.data[ atrial_actis ], "atrialActivationFinder result" );
    }
}

void Processing::reconstructSignal()
{
    if( meshList[ catheter ] ) //not nullptr
    {
        if( meshList[ catheter ]->hasNoData() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No catheter data to use"), QMessageBox::Ok );
            return;
        }
        if( !meshList[ catheter ]->hasAtrialActiData() )
        {
            this->findCathActivations();

            if( !meshList[ catheter ]->hasAtrialActiData() ) //none found
            {
                return;
            }
        }
        meshList[ catheter ]->reconstructSignalFromActivations( filter_vars );

        helper::printMatrixInfo( meshList[ catheter ]->meshData.data[ processed ], "signal reconstruction result" );
    }
}

void Processing::findEndoActivations()
{
    if( meshList[ atrium ] ) //not nullptr
    {
        if( !meshList[ atrium ]->hasProcessedData() )
        {
            QMessageBox::warning( this, tr("Warning"), tr("No processed atrial data to search for activations in"), QMessageBox::Ok );
            return;
        }
        meshList[ atrium ]->findAtrialActivations( filter_vars );

        if( !meshList[ atrium ]->hasAtrialActiData() ) //none found
        {
            QMessageBox::warning( this, tr("Warning"), tr("No atrial activations found"), QMessageBox::Ok );
            return;
        }
        helper::printMatrixInfo( meshList[ atrium ]->meshData.data[ atrial_actis ], "EndocardialActivationFinder result" );
    }
}

void Processing::createEndoActivationMap()
{
    if( meshList[ atrium ] ) //not nullptr
    {
        if( meshList[ atrium ]->hasNoData() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No atrial data to create activation history map"), QMessageBox::Ok );
            return;
        }
        if( !meshList[ atrium ]->hasAtrialActiData() )
        {
            QMessageBox::warning( this, tr("Warning"), tr("No activations. Will now search for some (this may take a few seconds)"), QMessageBox::Ok );

            meshList[ atrium ]->findAtrialActivations( filter_vars );

            if( !meshList[ atrium ]->hasAtrialActiData() ) //none found
            {
                QMessageBox::warning( this, tr("Warning"), tr("No atrial activations found"), QMessageBox::Ok );
                return;
            }
        }
        meshList[ atrium ]->createActiHistoryMap( filter_vars );

        helper::printMatrixInfo( meshList[ atrium ]->meshData.data[ acti_history ], "activation history map result" );
    }
}

void Processing::createPhaseMap()
{
    if( meshList[ atrium ] ) //not nullptr
    {
        if( meshList[ atrium ]->hasNoData() ) //no data on the mesh
        {
            QMessageBox::warning( this, tr("Warning"), tr("No atrial data to create phase map"), QMessageBox::Ok );
            return;
        }else{

            meshList[ atrium ]->createPhaseMap( filter_vars );
            helper::printMatrixInfo( meshList[ atrium ]->meshData.data[ phase ], "Phase map result" );
        }
    }

}

void Processing::runInverse()
{
    if( meshList.size() == 0 )
    {
        QMessageBox::warning( this, tr("Warning"), tr("Meshes or data loaded"), QMessageBox::Ok );
        return;
    }
    if( meshList[ catheter ] && meshList[ atrium ] ) //both not null
    {
        if( general_vars.mode == catheter_input )
        {
            if( meshList[ catheter ]->hasRawData() )
            {
                this->inverse( meshList[catheter], meshList[atrium], inverse_vars );

            }else{
                QMessageBox::warning( this, tr("Warning"), tr("Trying to run an inverse from no catheter input (in cath input mode)"), QMessageBox::Ok );
                return;
            }
        }else if( general_vars.mode == atrial_input )
        {
            if( meshList[ atrium ]->hasRawData() )
            {
                this->forward( meshList[catheter], meshList[atrium], inverse_vars );  ///generate some data and put in basket raw

            }else{
                QMessageBox::warning( this, tr("Warning"), tr("Trying to run a forward from no atrial data"), QMessageBox::Ok );
                return;
            }
            if( meshList[ catheter ]->hasRawData() )
            {
                this->inverse( meshList[catheter], meshList[atrium], inverse_vars );

            }else{
                QMessageBox::warning( this, tr("Warning"), tr("Forward did not result in basket data, aborting inverse"), QMessageBox::Ok );
                return;
            }
        }
    }else{

        QMessageBox::warning( this, tr("Warning"), tr("Either cath or atrial mesh is NULL"), QMessageBox::Ok );
        return;
    }
    helper::printMatrixInfo( meshList[ atrium ]->meshData.data[processed], "MFS result" );

}

void Processing::clearFiltering()
{
    meshList[ catheter ]->clearProcessedData();
}

void Processing::clearAllData()
{
    for( int mesh_type=0; mesh_type<MeshType::max_num_meshes; mesh_type++ ) //all the meshes
    {
        if( meshList[ mesh_type ] ) //if not null
        {
            meshList[ mesh_type ]->wipeData();
        }

    }
}

