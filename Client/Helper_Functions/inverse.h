#ifndef INVERSE_H
#define INVERSE_H

#include "processing.h"
#include <Eigen/Core>
#include <iostream>
#include <chrono>
#include "igl/decimate.h"
#include "igl/slice.h"
#include "igl/slice_mask.h"
#include "igl/all_pairs_distances.h"
#include "Helper_Functions/creso.h"
#include "Helper_Functions/tikhonovregularisation.h"
#include "definesandstructs.h"
#include "Helper_Functions/printmatrixinfo.h"

using namespace std::chrono;


void Processing::inverse(QSharedPointer<Mesh> cath, QSharedPointer<Mesh> atrium, const Inverse_vars& vars )
{
    std::cout << "...Inverse() start" << std::endl;
    auto start = high_resolution_clock::now();

    if( !cath || !atrium ) //ensure both objects live
    {
        return;

    }else if( cath->hasNoData() ) // ensure some data
    {
        return;

    }else if( !cath->hasProcessedData() ) //if no processed copy in the raw and work from this
    {
        cath->setProcesssedData( cath->getMeshData().data[ raw ] );
    }

    size_t num_V        = atrium->getMeshGeom().V.rows();
    size_t num_time_pts = cath->getMeshData().data[ processed ].cols();

    //Generate the sources
    Eigen::MatrixXf V_source;
    Eigen::VectorXi V_source_inds;
    generateSources( atrium->getMeshGeom().V,
                     atrium->getMeshGeom().F,
                     atrium->getMeshGeom().N,
                     vars.atrial_inverse_source_downsampling,
                     SourceGen::inflate,
                     SourceGen::alongNormals,
                     vars.source_scaling[MeshType::atrium],
                     V_source,
                     V_source_inds);
    size_t num_Vscoure = V_source.rows();  ///recalculate incase downsampled

    Eigen::MatrixXf Vc_source;
    Eigen::VectorXi Vc_source_inds;
    generateSources( cath->getMeshGeom().V,
                     cath->getMeshGeom().F,
                     cath->getMeshGeom().N,
                     1.0,
                     SourceGen::deflate,
                     SourceGen::alongNormals,
                     vars.source_scaling[MeshType::catheter],
                     Vc_source,
                     Vc_source_inds);

    //Remove the unwanted cath channels from the transfer and data matrices
    Eigen::MatrixXf Vc_good;
    Eigen::MatrixXf Vc_source_good;
    Eigen::MatrixXf cath_data_good;
    if( vars.cath_channels_to_use.all() )//all true
    {
        Vc_good        = cath->getMeshGeom().V;
        Vc_source_good = Vc_source;    //keep if all good
        cath_data_good = cath->getMeshData().data[processed];
    }else{
        igl::slice_mask( cath->getMeshGeom().V              , vars.cath_channels_to_use, 1, Vc_good        );
        igl::slice_mask( Vc_source                          , vars.cath_channels_to_use, 1, Vc_source_good );   //slice out good ones if some bad
        igl::slice_mask( cath->getMeshData().data[processed], vars.cath_channels_to_use, 1, cath_data_good );
    }
    size_t num_VcSource  = Vc_good.rows();

    //Combine sources
    Eigen::MatrixXf fictSources( num_Vscoure+num_VcSource, 3 );
    fictSources << V_source,
                   Vc_source_good;

    //Transfer matrix - 1/r, cath to all-sources
    Eigen::MatrixXf transferA;
    igl::all_pairs_distances( Vc_good, fictSources, false, transferA );
    transferA =  transferA.array().inverse();

    //Transfer matrix - 1/r, atrium to all-sources
    Eigen::MatrixXf forward_matrix;
    Eigen::MatrixXf V_tmp = atrium->getMeshGeom().V; //passing this directly messes up the igl::templates for some reason
    igl::all_pairs_distances( V_tmp, fictSources, false, forward_matrix );
    forward_matrix = forward_matrix.array().inverse();

    //Singular Value Decomposition of the transfer matrix
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(transferA.transpose(), Eigen::ComputeThinU | Eigen::ComputeThinV);

    double lambda = 0;
    switch( vars.lambdaMethod )
    {
        case LambdaMethod::userDefined :
            lambda = vars.lambda;
            break;
        case LambdaMethod::CRESO :
            lambda = helper::creso(svd, cath_data_good); break;
        default:
            std::cout << "Lamda value setting error" <<std::endl;
            return;
    }

    std::cout << "\t-->tikhonovRegularisation() start" << std::endl;
    auto startTik = high_resolution_clock::now();
    Eigen::MatrixXf outField( transferA.cols() , num_time_pts );
    for(size_t time_pt=0; time_pt < num_time_pts; time_pt++)
    {
        Eigen::MatrixXf dataIn = cath_data_good.col(time_pt);
        outField.col(time_pt) = helper::tikhonovRegularisation( svd, dataIn, lambda );
    }
    auto stopTik = high_resolution_clock::now();
    auto durationTik = duration_cast<milliseconds>(stopTik - startTik);
    std::cout << "\t<--tikhonovRegularisation() end (" << durationTik.count() << " msec)" << std::endl;


    //helper::printMatrixInfo( dataIn, "INVERSE - dataIn");
    //helper::printMatrixInfo( forward_matrix, "INVERSE - forward");
    //helper::printMatrixInfo( forward_matrix, "INVERSE - outfield");

    //The result
    atrium->setProcesssedData( forward_matrix * outField );
    atrium->updateDataGraphicsBuffers( processed );


    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...Inverse() end (" << duration.count() << " msec)" << std::endl;
}//end inverse function








void Processing::generateSources(  const Eigen::MatrixXf &V,
                                   const Eigen::MatrixXi &F,
                                   const Eigen::MatrixXf &N,
                                   float downsample, SourceGen inf_or_def, SourceGen method, float source_elevation_in,
                                   Eigen::MatrixXf &sources,
                                   Eigen::VectorXi &inds            )
{
    std::cout << "\t-->generateSources() start" << std::endl;
    auto start = high_resolution_clock::now();


    Eigen::MatrixXd V_Xd = V.cast<double>();
    Eigen::MatrixXi F_Xi = F.cast<int   >();
    Eigen::MatrixXd sources_out;

    if( downsample < 0.0 )
    {
        exit(0);
    }
    if( downsample < 1.0 )
    {

        size_t num_input_faces  = F.rows();
        size_t num_output_faces = round( float(num_input_faces) * downsample );

        Eigen::MatrixXi tmpFout;
        Eigen::VectorXi tmpFinds;

        igl::decimate( V_Xd, F_Xi, num_output_faces, sources_out, tmpFout, tmpFinds, inds );

    }else {

        sources_out = V_Xd;
        inds = Eigen::VectorXi::LinSpaced( sources_out.rows(), 0, sources_out.rows()-1 );
    }

   /* Create source points by scaling the colocation points out along their normals. Need to normalise the
    * normals in order to standardise the distance each vertex moves away from its colocation point. The distance
    * moved out needs to be relative to the size of the mesh and so where LB and Emma used a fixed 1.16 scaling
    * factor I have used a similar measure which is 0.16*average radius*vertexUnitNormal. The scaling value is presented
    * on the GUI as 1.16 so need to -1 here */
    Eigen::RowVector3d centre = V_Xd.colwise().mean();
    if( method == SourceGen::alongNormals )
    {
        double av_mesh_radius   = (V_Xd.array().rowwise() - centre.array()).cwiseAbs2().rowwise().sum().cwiseSqrt().mean();
        double source_elevation = av_mesh_radius * source_elevation_in;

        Eigen::MatrixXd N_Xd;
        igl::slice( N.cast<double>(), inds, 1, N_Xd );
        N_Xd.rowwise().normalize();

        if( inf_or_def == SourceGen::deflate )
        {
            sources_out = sources_out - (source_elevation * N_Xd );

        }else if( inf_or_def == SourceGen::inflate )
        {
            sources_out = sources_out + (source_elevation * N_Xd );
        }

    }else if( method == SourceGen::fromCentre )
    {
        if( inf_or_def == SourceGen::deflate )
        {
            sources_out = (sources_out.rowwise() - centre) * (1-source_elevation_in);

        }else if( inf_or_def == SourceGen::inflate )
        {
            sources_out = (sources_out.rowwise() - centre) * (source_elevation_in+1);
        }

    }

    //Cast back
    sources = sources_out.cast<float>();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "\t<--generateSources() end (" << duration.count() << " msec)" << std::endl;
}


#endif // INVERSE_H
