#ifndef FORWARD_H
#define FORWARD_H

#include "processing.h"
#include <Eigen/Core>
#include <iostream>
#include <chrono>
#include "igl/all_pairs_distances.h"
#include "igl/slice.h"
#include "definesandstructs.h"
#include "Helper_Functions/printmatrixinfo.h"

using namespace std::chrono;


void Processing::forward( QSharedPointer<Mesh> cath, QSharedPointer<Mesh> atrium, const Inverse_vars& vars )
{
    std::cout << "...Forward() start" << std::endl;
    auto start = high_resolution_clock::now();


    //Generate the sources for the atrial mesh
    Eigen::MatrixXf V_source;
    Eigen::VectorXi V_source_inds;
    generateSources( atrium->getMeshGeom().V,
                     atrium->getMeshGeom().F,
                     atrium->getMeshGeom().N,
                     vars.atrial_forward_source_downsampling,  //usually no downsampling i.e. =1.0
                     SourceGen::inflate,
                     SourceGen::alongNormals,
                     vars.source_scaling[MeshType::atrium],
                     V_source,
                     V_source_inds);

   /* Take the colocation points (usually use all of them for forward) and get the data at these points. igl::slice is equivalent to matlab's X(rows, :) */
    Eigen::MatrixXf V_coloc;
    igl::slice( atrium->getMeshGeom().V,
                V_source_inds,
                1,
                V_coloc);
    Eigen::MatrixXf dataAtColocationPts;
    igl::slice( atrium->getMeshData().data[ raw ],
                V_source_inds,
                1,
                dataAtColocationPts);

   /* Calculate the distances between the colocation and source points - then 1/r to get transfer matrix */
    Eigen::MatrixXf coloc_source_dist_mat;
    igl::all_pairs_distances( V_coloc, V_source, false, coloc_source_dist_mat);
    coloc_source_dist_mat = coloc_source_dist_mat.array().inverse();

   /* Solve coefficients of:  DistMat * coeffs = surface potentials */
    Eigen::MatrixXf coefficients = coloc_source_dist_mat.fullPivLu().solve(dataAtColocationPts);

   /* Calculate the distances between the transformed basket points and the source points - then 1/r to get transfer matrix */
    Eigen::MatrixXf basket_source_dist_mat;
    Eigen::MatrixXf Vc = cath->getMeshGeom().V;
    igl::all_pairs_distances( Vc, V_source, false, basket_source_dist_mat);
    basket_source_dist_mat = basket_source_dist_mat.array().inverse();

   /* Use the coefficients to calculate the basket potentials */
    cath->setRawData( basket_source_dist_mat * coefficients );
    cath->setProcesssedData( basket_source_dist_mat * coefficients );


    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...Forward() end (" << duration.count() << " msec)" << std::endl;
}




#endif // FORWARD_H
