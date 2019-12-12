#ifndef PHASEMAP_H
#define PHASEMAP_H

#include <Eigen/Core>
#include <iostream>
#include <chrono>
#include "definesandstructs.h"
#include "Helper_Functions/printmatrixinfo.h"
#include "fftw3.h"
#include <omp.h>


using namespace std::chrono;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXf;
typedef Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXcf;

namespace helper {


void phaseMap( const Eigen::MatrixXf& signals_for_filtering, Eigen::MatrixXf& output, const Filter_vars& vars )
{
    std::cout<<"\t-->Phase map"<<std::endl;
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
    int diffpow2 = nextPow2 - num_time_pts;

    //Split the difference between front and back of the signal
    int front_pad = int( floor( float( diffpow2 )/2.0 ) );

    ///Get gradient
    RowMajMatXf diff = RowMajMatXf::Zero( num_channels, num_time_pts );
    ///Evens
    Eigen::MatrixXf A    = signals_for_filtering(Eigen::all, Eigen::seq(0,Eigen::last,2));
    ///Tmp matrix to hold the differences
    Eigen::MatrixXf tmp  = A.rightCols( A.cols()-1 ) - A.leftCols( A.cols()-1 );
    ///Splice in the even differences at the odd columns
    diff( Eigen::all, Eigen::seq( 1, Eigen::last-1, 2 ) ) = tmp;
    ///Odds
    A = signals_for_filtering(Eigen::all, Eigen::seq(1,Eigen::last,2));
    ///Difference between odds
    tmp = A.rightCols( A.cols()-1 ) - A.leftCols( A.cols()-1 );
    ///Splice in odds at the even columns
    diff( Eigen::all, Eigen::seq( 2, Eigen::last-1, 2 ) ) = tmp;


    ///Padded signal
    ///Allocate space for complex signals and copy signals into real
    RowMajMatXcf working = RowMajMatXcf::Zero( num_channels, nextPow2 );
    working.real().block( 0,front_pad,num_channels,num_time_pts ) = signals_for_filtering;

    ///The period for the sinusoid
    int period = round( vars.phase_base_period_secs * vars.getSampleRate() );

    ///Sinusoidal wavelet
    if( period % 2 == 0 )
    {
        period++;
    }
    Eigen::RowVectorXf sinusoidal_wavelet = Eigen::RowVectorXf::Zero( period );
    for( int i=0; i<sinusoidal_wavelet.size(); i++ )
    {
        sinusoidal_wavelet( i ) = sin( 2.0 * M_PI * (i-period/2)/period );
    }

    int sin_half = floor( sinusoidal_wavelet.size() / 2 );
    int idx_s = 0;
    int idx_e = 0;
    int lenDiff = 0;
    int actLen  = 0;
    bool checkEnds = true;
    if( front_pad - sin_half > 0 && nextPow2 - sin_half > num_time_pts-1 )
    {
        checkEnds = false;
    }

    for( int chan=0; chan<num_channels; chan++ )
    {
    for( int time=1; time<num_time_pts-1; time++ )
    {
        if( diff( chan, time ) < 0 ) ///if difference <0
        {
            if( checkEnds )
            {
                idx_s = front_pad+time-sin_half;
                idx_e = front_pad+time+sin_half;
                if( idx_s<0 )
                {
                    idx_s = 0;
                };
                if( idx_e > nextPow2-1 )
                {
                    idx_e = nextPow2-1;
                }
                lenDiff = sinusoidal_wavelet.size() - ( idx_e-idx_s+1 );
                actLen  = idx_e-idx_s+1;

                if( lenDiff>0 && idx_s==0 )
                {
                    working.real().block(chan, idx_s, 1, actLen ) =
                            working.real()    .block(chan, idx_s, 1, actLen ).array() +
                            (sinusoidal_wavelet.block(0,lenDiff,1,actLen).array()*diff( chan, time ));

                }else if( lenDiff>0 && idx_e==num_time_pts-1 )
                {
                    working.real().block(chan, idx_s, 1, actLen ) =
                            working.real()    .block(chan, idx_s, 1, actLen ).array() +
                            (sinusoidal_wavelet.block(0,0,1,actLen).array()*diff( chan, time ));
                }else{

                    working.real().block(chan, idx_s, 1, actLen ) =
                            working.real()    .block(chan, idx_s, 1, actLen ).array() +
                            (sinusoidal_wavelet.array()*diff( chan, time ));
                }
            }else{

                working.real().block(chan, front_pad+time-sin_half, 1, sinusoidal_wavelet.size() ) =

                working.real().block(chan, front_pad+time-sin_half, 1, sinusoidal_wavelet.size() ).array() +
                              (sinusoidal_wavelet.array()*diff( chan, time )  );

            }

        }
    }
    }


    //////////////////////////////////////////////
    /// Hilbert
    ///
    /// The analytic signal for a sequence xr has a one-sided Fourier transform.
    /// That is, the transform vanishes for negative frequencies.
    /// To approximate the analytic signal, hilbert calculates the FFT of the input sequence,
    /// replaces those FFT coefficients that correspond to negative frequencies with zeros,
    /// and calculates the inverse FFT of the result.
    ///
    /// Hilbert uses a four-step algorithm:
    ///        Calculate the FFT of the input sequence, storing the result in a vector x.
    ///        Create a vector h whose elements h(i) have the values:
    ///            1 for i = 1 & (n/2)+1
    ///            2 for i = 2, 3, ... , (n/2)
    ///            0 for i = (n/2)+2, ... , n
    ///        Calculate the element-wise product of x and h.
    ///        Calculate the inverse FFT of the sequence obtained in step 3 and returns the first n elements of the result.
    ///
    //////////////////////////////////////////////

    int limit1 = nextPow2/2;

    Eigen::RowVectorXcf h_vec        = Eigen::RowVectorXcf::Zero( nextPow2 );
        h_vec.real().block(0,0,1,limit1).setConstant( 2.0 );
        h_vec.imag().block(0,1,1,limit1).setConstant( 2.0 );
        h_vec.real()(0)       = 1.0;
        h_vec.imag()(0)       = 1.0;
        h_vec.real()(limit1+1)= 1.0;
        h_vec.imag()(limit1+1)= 1.0;

    fftwf_plan_with_nthreads(omp_get_max_threads()); ///~4 threads (my mac has 4 cores)
    fftwf_complex *in = reinterpret_cast <fftwf_complex*>( working.data() );
    fftwf_plan fP = fftwf_plan_many_dft( 1,
                                         &nextPow2,
                                         num_channels,
                                         in, nullptr, 1, nextPow2,
                                         in, nullptr, 1, nextPow2,
                                         FFTW_FORWARD,
                                         FFTW_ESTIMATE   );
    fftwf_execute( fP );
    fftwf_cleanup();

    working = working.array().rowwise() * h_vec.array();

    fftwf_plan_with_nthreads(omp_get_max_threads()); ///~4 threads (my mac has 4 cores)
    fftwf_plan bP = fftwf_plan_many_dft( 1,
                                         &nextPow2,
                                         num_channels,
                                         in, nullptr, 1, nextPow2,
                                         in, nullptr, 1, nextPow2,
                                         FFTW_BACKWARD,
                                         FFTW_ESTIMATE   );

     fftwf_execute( bP );
     fftwf_cleanup();

     working = working.array() / nextPow2;

     ///Resize the output matrix to take the phase result
     output.resize( num_channels, num_time_pts );
     output = working.block( 0,front_pad,num_channels,num_time_pts ).real()
             .binaryExpr( -working.block( 0,front_pad,num_channels,num_time_pts ).imag(),
              [] (float a, float b) { return std::atan2(a,b); } );


    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout<<"\t<--Phase map (" << duration.count() << " msec, using "<<omp_get_max_threads()<<" threads)" <<std::endl;

}//end function
}//end namespace

#endif // PHASEMAP_H
