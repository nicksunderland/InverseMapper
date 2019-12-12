#include "mesh.h"
#include "igl/read_triangle_mesh.h"
#include "igl/per_vertex_normals.h"
#include "Helper_Functions/bandstopfilter.h"
#include "Helper_Functions/phasemap.h"
#include "Helper_Functions/qrspeakfinder.h"
#include "Helper_Functions/qrssubtraction.h"
#include "Helper_Functions/atrialactivationfinder.h"
#include "Helper_Functions/atrialsignalreconstruct.h"
#include "Helper_Functions/createactivationhistorymap.h"
#include "Helper_Functions/readdatafomtextfile.h"
#include "Helper_Functions/printmatrixinfo.h"
#include "definesandstructs.h"


Mesh::Mesh( MeshType mesh_type_in, QObject *parent ) :
    QObject(parent),
    meshBuffers{ nullptr, nullptr, nullptr, {nullptr}, {nullptr} }
{
    meshGeom.meshType = mesh_type_in;
    meshData.meshType = mesh_type_in;
    this->createBuffers();
}

void Mesh::importMesh(QString file_path, MeshType mesh_type)
{
    std::cout << "Mesh --> importMesh()" << std::endl;

    if( meshGeom.V.size() > 0 )
    {
        this->clearData();
    }

    try
    {
        igl::read_triangle_mesh( file_path.toStdString(), meshGeom.V, meshGeom.F );
        igl::per_vertex_normals( meshGeom.V, meshGeom.F, igl::PER_VERTEX_NORMALS_WEIGHTING_TYPE_ANGLE, meshGeom.N );

        if( meshGeom.V.size()==0 || meshGeom.F.size()==0 || meshGeom.N.size()==0  )
        {
            throw 1;
        }

        //Set the original mesh centre
        meshGeom.mesh_centre = meshGeom.V.colwise().mean();

        //Copy the V into the originalV matrix
        meshGeom.V_orig = meshGeom.V;

        //Set mesh types
        meshGeom.meshType    = mesh_type;
        meshData.meshType    = mesh_type;


        //Fill the graphics buffers
        this->updateMeshGraphicsBuffers();
    }

    catch ( int x )
    {
        std::cout<<"***ERROR***\nMesh::importMesh(): " << x << std::endl;
    }

    std::cout << "Mesh <-- importMesh()" << std::endl;
}

void Mesh::importData(QString file_path)
{
    std::cout << "Mesh --> importData()" << std::endl;

    if( meshData.data[DataType::raw].size() > 0 )
    {
        this->clearData();
    }

    try
    {
        helper::readDataFromTextFile( file_path, meshData.data[DataType::raw] );

        if( meshData.data[DataType::raw].size()==0 )
        {
            throw 1;
        }

        if( this->hasRawData() )
        {
            //Fill the graphics buffers
            this->updateDataGraphicsBuffers( raw );
        }
    }
    catch ( int x )
    {
        std::cout<<"***ERROR***\nMesh::importData(): " << x << std::endl;
    }

    std::cout << "Mesh <-- importData()" << std::endl;
}

void Mesh::centreMesh( Eigen::RowVector3f centreOn )
{
    std::cout << "Mesh --> centreMesh()" << std::endl;

    meshGeom.V      = meshGeom.V.rowwise()      - centreOn;
    meshGeom.V_orig = meshGeom.V_orig.rowwise() - centreOn;

    this->updateMeshGraphicsBuffers();

    std::cout << "Mesh <-- centreMesh()" << std::endl;
}

void Mesh::setProcesssedData(Eigen::MatrixXf new_data)
{
    std::cout << "Mesh --> setProcesssedData()" << std::endl;

    meshData.data[ processed ] = new_data;
    updateDataGraphicsBuffers( processed );

    std::cout << "Mesh <-- setProcesssedData()" << std::endl;
}

void Mesh::setRawData(Eigen::MatrixXf new_data)
{
    std::cout << "Mesh --> setProcesssedData()" << std::endl;

    meshData.data[ raw ] = new_data;
    updateDataGraphicsBuffers( raw );

    std::cout << "Mesh <-- setProcesssedData()" << std::endl;
}

void Mesh::transformMesh( const glm::vec3 &cath_translation, const glm::vec3 &cath_rotation, const glm::vec3 &cath_scale)
{
    Eigen::Transform<float, 3, Eigen::Affine> transform = Eigen::Transform<float, 3, Eigen::Affine>::Identity();

    transform.scale( cath_scale.x );//the cath_scaling should be the same in all directions so just use .x
    transform.rotate   (Eigen::AngleAxisf( cath_rotation.x, Eigen::Vector3f::UnitX() ));
    transform.rotate   (Eigen::AngleAxisf( cath_rotation.y, Eigen::Vector3f::UnitY() ));
    transform.rotate   (Eigen::AngleAxisf( cath_rotation.z, Eigen::Vector3f::UnitZ() ));
    transform.translate(Eigen::Vector3f( cath_translation.x, cath_translation.y, cath_translation.z ));

    meshGeom.V = ( transform * meshGeom.V_orig.transpose().colwise().homogeneous() ).transpose();

    this->updateMeshGraphicsBuffers();
}

void Mesh::clearData()
{
    if( hasRawData() )
    {
        meshData.data[ raw ].setZero();
    }
    if( hasProcessedData() )
    {
        meshData.data[ processed ].setZero();
    }
    if( hasQRSdata() )
    {
        meshData.data[ QRS_actis ].setZero();
    }
    if( hasAtrialActiData() )
    {
        meshData.data[ atrial_actis ].setZero();
    }

    this->updateDataGraphicsBuffers( raw );
    this->updateDataGraphicsBuffers( processed );

}

void Mesh::clearProcessedData()
{
    if( hasRawData() )
    {
        setProcesssedData( meshData.data[ raw ] );
        this->updateDataGraphicsBuffers( processed );
    }
    if( hasQRSdata() )
    {
        meshData.data[ QRS_actis ].resize(0,0);
        this->updateDataGraphicsBuffers( raw );
        this->updateDataGraphicsBuffers( processed );
    }
    if( hasAtrialActiData() )
    {
        meshData.data[ atrial_actis ].resize(0,0);
        this->updateDataGraphicsBuffers( raw );
        this->updateDataGraphicsBuffers( processed );
    }
}

void Mesh::bandstopFilterData( const Filter_vars& vars )
{
    std::cout << "Mesh --> bandstopFilterData()" << std::endl;

    if( !this->hasProcessedData() && this->hasRawData() ) //if nothing in processed matrix...
    {
        this->setProcesssedData( meshData.data[raw] );    //copy in from raw matrix
    }
    helper::bandstopFilter( meshData.data[ processed ], vars ); //inplace filtering of processed matrix
    this->updateDataGraphicsBuffers( processed );


    std::cout << "Mesh <-- bandstopFilterData()" << std::endl;
}

void Mesh::findQRSactivations( const Filter_vars& vars )
{
    if( !this->hasProcessedData() && this->hasRawData() )
    {
        this->setProcesssedData( meshData.data[raw] );    //copy in from raw matrix
        this->updateDataGraphicsBuffers( processed );
    }
    helper::QRSpeakFinder( meshData.data[ processed ], meshData.data[ QRS_actis ], vars );
    this->updateDataGraphicsBuffers( processed ); ///will update QRSs with processed mV

}

void Mesh::subtractQRSactivations( const Filter_vars& vars )
{
    helper::qrsSubtractionAlgo( meshData.data[ processed ], meshData.data[ QRS_actis ], vars );
    this->updateDataGraphicsBuffers( processed );

}

void Mesh::findAtrialActivations( const Filter_vars& vars )
{
    std::cout << "Mesh --> findAtrialActivations()" << std::endl;

    if( !this->hasProcessedData() && this->hasRawData() )
    {
        this->setProcesssedData( meshData.data[raw] );    //copy in from raw matrix
        this->updateDataGraphicsBuffers( processed );
    }
    helper::atrialActivationFinder( meshData.data[ processed ], meshData.data[ atrial_actis ], vars );
    this->updateDataGraphicsBuffers( processed );


    std::cout << "Mesh <-- findAtrialActivations()" << std::endl;
}

void Mesh::reconstructSignalFromActivations( const Filter_vars& vars )
{
    helper::atrialSignalReconstructor( meshData.data[ processed ], meshData.data[ atrial_actis ], vars );
    this->updateDataGraphicsBuffers( processed );
}

void Mesh::createActiHistoryMap( const Filter_vars& vars )
{
    std::cout << "Mesh --> createActiHistoryMap()" << std::endl;

    if( !this->hasAtrialActiData() )
    {
        return;
    }else{

        if( !this->hasActiHistData() && this->hasRawData() )
        {
            meshData.data[ acti_history ] = meshData.data[ raw ]; //raw mat needs to be the right size/ it'll get over written

        }else if( !this->hasActiHistData() && this->hasProcessedData() )
        {
            meshData.data[ acti_history ] = meshData.data[ processed ];
        }
        helper::createActivationHistoryMap( meshData.data[ acti_history ], meshData.data[ atrial_actis ], vars );
        this->updateDataGraphicsBuffers( acti_history );
    }

    std::cout << "Mesh <-- createActiHistoryMap()" << std::endl;
}

void Mesh::createPhaseMap( const Filter_vars& vars )
{
    std::cout << "Mesh --> createPhaseMap()" << std::endl;

    if( this-> hasRawData() && !this->hasProcessedData() )
    {
        this->setProcesssedData( meshData.data[ raw ] );
        this->updateDataGraphicsBuffers( processed );
    }
    if( this->hasProcessedData() )
    {
        helper::phaseMap( meshData.data[ processed ], meshData.data[ phase ],  vars );
        this->updateDataGraphicsBuffers( phase );
    }

    std::cout << "Mesh <-- createPhaseMap()" << std::endl;
}

void Mesh::wipeData()
{
    for( int data_type=0; data_type<DataType::max_data_types; data_type++ )
    {
        if( meshData.data[ data_type ].size() > 0 )
        {
            std::cout<<"Wiping data, data_type(" << data_type << ")" << std::endl;
            meshData.data[ data_type ].setZero();

            DataType d = static_cast<DataType>(data_type);
            this->updateDataGraphicsBuffers( d );
        }
    }

}

void Mesh::createBuffers()
{
    std::cout << "Mesh --> createBuffers()" << std::endl;

    ///Geom buffers
    meshBuffers.qVBO = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
    meshBuffers.qNBO = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
    meshBuffers.qEBO = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
    meshBuffers.qVBO ->setUsagePattern  ( QOpenGLBuffer::DynamicDraw  );
    meshBuffers.qNBO ->setUsagePattern  ( QOpenGLBuffer::DynamicDraw  );
    meshBuffers.qEBO ->setUsagePattern  ( QOpenGLBuffer::DynamicDraw  );
    meshBuffers.qVBO ->create();
    meshBuffers.qNBO ->create();
    meshBuffers.qEBO ->create();

    ///Data buffers
    for( int data_type=0; data_type<DataType::max_data_types; data_type++ )
    {
        meshBuffers.qDataBO[ data_type ]      = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
        meshBuffers.q_mV_rangeBO[ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
        meshBuffers.qDataBO[ data_type ]      ->setUsagePattern  ( QOpenGLBuffer::DynamicDraw  );
        meshBuffers.q_mV_rangeBO[ data_type ] ->setUsagePattern  ( QOpenGLBuffer::DynamicDraw  );
        meshBuffers.qDataBO[ data_type ]      ->create();
        meshBuffers.q_mV_rangeBO[ data_type ] ->create();

//        Eigen::VectorXf tmp = Eigen::VectorXf::Zero( meshGeom.V.rows() );
//        meshBuffers.qDataBO[ data_type ]->bind();
//        meshBuffers.qDataBO[ data_type ]->allocate( tmp.data(), tmp.size() * sizeof(float) );
//        meshBuffers.qDataBO[ data_type ]->release();
//        meshBuffers.q_mV_rangeBO[ data_type ]->bind();
//        meshBuffers.q_mV_rangeBO[ data_type ]->allocate( tmp.data(), tmp.size() * sizeof(float) );
//        meshBuffers.q_mV_rangeBO[ data_type ]->release();

//        DataType d = static_cast<DataType>(data_type);
//        emit mesh_buffers_updated_signal( meshGeom.meshType, d );

    }

    std::cout << "Mesh <-- createBuffers()" << std::endl;
}

void Mesh::updateMeshGraphicsBuffers()
{
    std::cout << "Mesh --> updateMeshGraphicsBuffers()" << std::endl;

    meshBuffers.qVBO->bind();
    meshBuffers.qVBO->allocate( meshGeom.V.data(), meshGeom.V.size() * sizeof(float) );
    meshBuffers.qVBO->release();

    meshBuffers.qNBO->bind();
    meshBuffers.qNBO->allocate( meshGeom.N.data(), meshGeom.N.size() * sizeof(float) );
    meshBuffers.qNBO->release();

    meshBuffers.qEBO->bind();
    meshBuffers.qEBO->allocate( meshGeom.F.cast<unsigned>().eval().data(), meshGeom.F.size() * sizeof(unsigned) );
    meshBuffers.qEBO->release();

    std::cout<<"\t>>> mesh_buffers_updated_signal sent. MeshType("<<meshData.meshType<<"), DataType(" << no_data << ")" << std::endl;
    emit mesh_buffers_updated_signal( meshGeom.meshType, no_data );

    std::cout << "Mesh <-- updateMeshGraphicsBuffers()" << std::endl;
}

void Mesh::updateDataGraphicsBuffers( DataType data_type )
{
    std::cout << "Mesh --> updateDataGraphicsBuffers()" << std::endl;

    if( data_type==no_data )///shouldn't happen but could in a data update loop
    {
        return;
    }

    if( data_type==raw )
    {
        helper::printMatrixInfo( meshData.data[ data_type ],"Raw DATA LOADING in mesh object");
    }



    ///Potential data
    meshBuffers.qDataBO[ data_type ]->bind();
    meshBuffers.qDataBO[ data_type ]->allocate( meshData.data[ data_type ].data(), meshData.data[ data_type ].size() * sizeof(float) );
    meshBuffers.qDataBO[ data_type ]->release();




//    ///Vertex potential range
    if ( hasProcessedData() )  ///Favour giving the processed mV range to all data types, if processed data exists
    {
        Eigen::VectorXf ranges = ( meshData.data[ processed ].rowwise().maxCoeff() -
                                   meshData.data[ processed ].rowwise().minCoeff()    );/// * 0.5; ///adjusting factor

        meshBuffers.q_mV_rangeBO[ data_type ]->bind();
        meshBuffers.q_mV_rangeBO[ data_type ]->allocate( ranges.data(), ranges.size() * sizeof(float) );
        meshBuffers.q_mV_rangeBO[ data_type ]->release();

    }else{  ///back up - shouldn't happen
        Eigen::VectorXf ranges = ( meshData.data[ data_type ].rowwise().maxCoeff() -
                                   meshData.data[ data_type ].rowwise().minCoeff()    );/// * 0.5; ///adjusting factor

        meshBuffers.q_mV_rangeBO[ data_type ]->bind();
        meshBuffers.q_mV_rangeBO[ data_type ]->allocate( ranges.data(), ranges.size() * sizeof(float) );
        meshBuffers.q_mV_rangeBO[ data_type ]->release();
    }


    std::cout<<"\t>>> mesh_buffers_updated_signal sent. MeshType("<<meshData.meshType<<"), DataType(" << data_type << ")" << std::endl;
    emit mesh_buffers_updated_signal     ( meshGeom.meshType, data_type );


    std::cout << "Mesh <-- updateDataGraphicsBuffers()" << std::endl;
}

const MeshGeom &Mesh::getMeshGeom() const
{
    return meshGeom;
}

const MeshData &Mesh::getMeshData() const
{
    return meshData;
}

const MeshBuffers &Mesh::getMeshBuffers() const
{
    return meshBuffers;
}

bool Mesh::hasNoData()
{
    if( meshData.data[ raw ].size() == 0 && meshData.data[ processed ].size() == 0 )
    {
        return true;
    }else{
        return false;
    }
}

bool Mesh::hasData( DataType data_type )
{
    if( meshData.data[ data_type ].size() > 0 )
    {
        return true;
    }else{
        return false;
    }
}

bool Mesh::hasRawData()
{
    if( meshData.data[ raw ].size() == 0 )
    {
        return false;
    }else{
        return true;
    }
}

bool Mesh::hasProcessedData()
{
    if( meshData.data[ processed ].size() == 0 )
    {
        return false;
    }else{
        return true;
    }
}

bool Mesh::hasActiHistData()
{
    if( meshData.data[ acti_history ].size() == 0 )
    {
        return false;
    }else{
        return true;
    }
}

bool Mesh::hasQRSdata()
{
    if( meshData.data[ QRS_actis ].size() == 0 )
    {
        return false;
    }else{
        return true;
    }
}

bool Mesh::hasAtrialActiData()
{
    if( meshData.data[ atrial_actis ].size() == 0 )
    {
        return false;
    }else{
        return true;
    }
}

bool Mesh::hasPhaseData()
{
    if( meshData.data[ phase ].size() == 0 )
    {
        return false;
    }else{
        return true;
    }
}

