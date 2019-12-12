#ifndef CWTFUNCTIONS_H
#define CWTFUNCTIONS_H

#include <Eigen/Core>
#include <iostream>
#include <chrono>
#include "Helper_Functions/printmatrixinfo.h"
#include "fftw3.h"
#include "wavelib.h"
#include "omp.h"

using namespace std::chrono;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXf;
typedef Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXcf;

namespace helper {

//Prototypes
Eigen::VectorXcf makeDaughterWavelet( int wavelet_order, double scale, const Eigen::VectorXf &k );


// Continuous Wavelet Transform using Gaussian derivative wavelets
RowMajMatXf gaus_cwt(
           const RowMajMatXf& signals_for_filtering,      //reference to the signals
           double dt,                                          //the period
           double scale_res,                                   //scale resolution, spacing between scales
           double scale_min,                                   //minimum, or start, wavelet scale
           int    num_scales,                                  //number of desired scales
           int wavelet_order    )                              //order of Gaus derivative
{
    std::cout<<"\t-->gaus_cwt"<<std::endl;
    auto start = high_resolution_clock::now();


    //Needed variables
    int num_time_pts = signals_for_filtering.cols();
    int num_channels = signals_for_filtering.rows();

    //Get the next highest power of 2
    int nextPow2 = num_time_pts;
    nextPow2--;
    nextPow2 |= nextPow2 >> 1;
    nextPow2 |= nextPow2 >> 2;
    nextPow2 |= nextPow2 >> 4;
    nextPow2 |= nextPow2 >> 8;
    nextPow2 |= nextPow2 >> 16;
    nextPow2++;

    //Get size difference between signal length and next power2
    int diff = nextPow2 - num_time_pts;

    //Split the difference between front and back of the signal
    int front_pad = int( floor( float( diff )/2.0 ) );

    //Create zero padded signal (zero matrix with data added to the middle)
    RowMajMatXcf padded_signal = RowMajMatXcf::Zero( num_channels, nextPow2 );
    padded_signal.real().block( 0,front_pad,num_channels,num_time_pts ) = signals_for_filtering;

    //Create wavenumber array
    Eigen::VectorXf k     = Eigen::VectorXf::Zero( nextPow2 );
    int halfVecSize       = floor( float(nextPow2  )/2.0 );
    int halfVecSize2      = floor( float(nextPow2-1)/2.0 );
    Eigen::VectorXf k_tmp = Eigen::VectorXf::LinSpaced( halfVecSize,1,halfVecSize )
                            .array() * ((2*M_PI)/(float(nextPow2)*dt));
    //Positives
    k.block(1,0,halfVecSize,1)              = k_tmp;
    //Negatives
    k.block(halfVecSize+1,0,halfVecSize2,1) = -(k_tmp.block(0,0,halfVecSize2,1).colwise().reverse());

    //Compute the Fourier transforms of the wavelets at their desired scales
    Eigen::VectorXf base   = Eigen::VectorXf::Constant( num_scales, 2.0 );
    Eigen::VectorXf powers = Eigen::VectorXf::LinSpaced( num_scales, 0, num_scales-1 );
    Eigen::VectorXf scales = base.array().pow( (powers.array()*scale_res) ) * scale_min;

    //std::cout<<"Scales: "<<scales<<std::endl;

  //  std::cout<<"daughters:\n"<<daughters<<std::endl;

    fftwf_plan_with_nthreads(omp_get_max_threads()); ///4 threads (my mac has 4 cores)
    fftwf_complex *in = reinterpret_cast <fftwf_complex*>( padded_signal.data() );
    fftwf_plan fP = fftwf_plan_many_dft( 1,
                                         &nextPow2,
                                         num_channels,
                                         in, nullptr, 1, nextPow2,
                                         in, nullptr, 1, nextPow2,
                                         FFTW_FORWARD,
                                         FFTW_ESTIMATE   );
    fftwf_execute( fP );
    fftwf_cleanup();

    //Initialise the matrix for the coefficients [num_scales*num_channels by num_time_pts+padding]
    RowMajMatXcf cwtCoeffs( num_scales*num_channels, nextPow2 );

    for(int level=0; level<num_scales; level++)
    {
        cwtCoeffs( Eigen::seq(level, cwtCoeffs.rows()-1, num_scales), Eigen::all ) = padded_signal.array().rowwise() * makeDaughterWavelet( wavelet_order, scales(level), k ).transpose().array();
    }


    fftwf_plan_with_nthreads(omp_get_max_threads()); ///4 threads (my mac has 4 cores)
    fftwf_complex *out = reinterpret_cast <fftwf_complex*>( cwtCoeffs.data() );
    fftwf_plan bP = fftwf_plan_many_dft( 1,
                                         &nextPow2,
                                         num_scales*num_channels,
                                         out, nullptr, 1, nextPow2,
                                         out, nullptr, 1, nextPow2,
                                         FFTW_BACKWARD,
                                         FFTW_ESTIMATE   );

     fftwf_execute( bP );
     fftwf_cleanup();

    cwtCoeffs = cwtCoeffs.array() / nextPow2;


    RowMajMatXf real_output = cwtCoeffs.real().block(0,front_pad,num_scales*num_channels,num_time_pts);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout<<"\t<--gaus_cwt (" << duration.count() << " msec using " << omp_get_max_threads() << " threads)" <<std::endl;

    //Return the real coefficients without the padding
   return real_output;
}


// Invert the coefficients
//RowMajMatXd i_gaus_cwt( RowMajMatXd coefficients,
//                        double dt,
//                        double scale_res,                                   //scale resolution, spacing between scales
//                        double scale_min,                                   //minimum, or start, wavelet scale
//                        int    num_scales,                                  //number of desired scales
//                        int wavelet_order    )
//{
//    std::cout<<"\t-->i_gaus_cwt"<<std::endl;

//    //Variables
//    int num_time_pts = coefficients.cols();
//    int num_channels = coefficients.rows() / num_scales;

//    //Create wavenumber array
//    Eigen::VectorXd k     = Eigen::VectorXd::Zero( num_time_pts );
//    int halfVecSize       = floor( double(num_time_pts  )/2 );
//    int halfVecSize2      = floor( double(num_time_pts-1)/2 );
//    Eigen::VectorXd k_tmp = Eigen::VectorXd::LinSpaced( halfVecSize,1,halfVecSize )
//                            .array() * ((2*M_PI)/(double(num_time_pts)*dt));
//    //Positives
//    k.block(1,0,halfVecSize,1)              = k_tmp;
//    //Negatives
//    k.block(halfVecSize+1,0,halfVecSize2,1) = -(k_tmp.block(0,0,halfVecSize2,1).colwise().reverse());

//    //Compute the Fourier transforms of the wavelets at their desired scales
//    Eigen::VectorXd base   = Eigen::VectorXd::Constant( num_scales, 2.0 );
//    Eigen::VectorXd powers = Eigen::VectorXd::LinSpaced( num_scales, 0, num_scales-1 );
//    Eigen::VectorXd scales = base.array().pow( (powers.array()*scale_res) ) * scale_min;
//    Eigen::MatrixXd scaleMat = (scales.replicate(1,num_time_pts)).replicate(num_channels,1).array().sqrt();


//    Eigen::VectorXcd tmp_daughter( num_time_pts );
//    Eigen::VectorXd Wdelta( num_scales );
//    for(int i=0; i<num_scales; i++)
//    {
//        tmp_daughter = makeDaughterWavelet( wavelet_order, scales(i), k );
//        Wdelta(i) = (1.0/num_time_pts) * tmp_daughter.real().sum();
//    }

//    double C = 1.0 / ( Wdelta.array() /  scales.array().sqrt() ).sum();

//    RowMajMatXd coeff2 = coefficients.array() / scaleMat.array();
//    RowMajMatXd signals_output( num_channels, num_time_pts );
//    for( int i=0; i<num_channels; i++ )
//    {
//        int rowIdx = i*num_scales;
//        signals_output.row(i) = coeff2.block(rowIdx,0,num_scales,num_time_pts).colwise().sum();
//    }

//    signals_output = C * signals_output.array();

//    //helper::printMatrixInfo(signals_output,"Signals Output from iCWT");
//    std::cout<<"\t<--i_gaus_cwt"<<std::endl;

//    return signals_output;
//}




// Make the wavelets
Eigen::VectorXcf makeDaughterWavelet( int wavelet_order, double scale, const Eigen::VectorXf &k )
{
    int n = k.size();

    Eigen::VectorXcf ki = Eigen::VectorXcf::Zero( n );
    ki.real()           = k;

    Eigen::VectorXcf expnt = Eigen::VectorXcf::Zero( n );
    expnt.real()           = -( ( k.array() * scale ).cwiseAbs2() ).array() / 2.0;

    float norm               = sqrt(( scale * k(1,0) ) / tgamma( wavelet_order+0.5 )) * sqrt(n);
    Eigen::VectorXcf normArr = Eigen::VectorXcf::Zero( n ); //Re=0
    normArr.imag()           = Eigen::VectorXf::Ones( n );  //Im=1
    normArr = -normArr.array().pow(wavelet_order).array()*norm;
    Eigen::VectorXcf scale_K = (k.array()*scale).array().pow(wavelet_order);


    Eigen::VectorXcf daughter = normArr.array() * scale_K.array() * expnt.array().exp();

//    Eigen::VectorXcf daughterHalf = daughter.block(0,0,n/2+1,1);

//    if( wavelet_order % 2 ) //is odd
//    {
//        daughterHalf.real() = Eigen::VectorXf::Zero( n/2+1 );
//    }else {
//        daughterHalf.imag() = Eigen::VectorXf::Zero( n/2+1 );
//    }

    return daughter;
}



}//end of namespace

#endif // CWTFUNCTIONS_H
