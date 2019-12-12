#ifndef BANDSTOPFILTER_H
#define BANDSTOPFILTER_H

#include <Eigen/Dense>
#include <QDebug>
#include <iostream>
#include <chrono>
#include "DspFilters/Dsp.h"
#include "definesandstructs.h"
#include "wavelib.h"
#include "mesh.h"
#include "Helper_Functions/printmatrixinfo.h"

using namespace std::chrono;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXf;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXd;

namespace helper {

void bandstopFilter( Eigen::MatrixXf& input_signals, const Filter_vars &vars )
{
    std::cout << "...bandstopFilter() start" << std::endl;
    auto start = high_resolution_clock::now();

    //Number of signals and time points
    int num_time_pts = input_signals.cols();
    int num_channels = input_signals.rows();

    //Get the next highest power of 2
    int nextPow2 = num_time_pts;
    nextPow2--;
    nextPow2 |= nextPow2 >> 1;
    nextPow2 |= nextPow2 >> 2;
    nextPow2 |= nextPow2 >> 4;
    nextPow2 |= nextPow2 >> 8;
    nextPow2 |= nextPow2 >> 16;
    nextPow2++;

    if( nextPow2 < 4096 )
    {
        nextPow2 = pow(2.0, 12.0);  ///ensure long enough for DWT 10 scales
    }


    //Get size difference between signal length and next power2; Split the difference between front and back of the signal
    int diff      = nextPow2 - num_time_pts;
    int front_pad = int( floor( double( diff )/2 ) );

    //std::cout << "num_time_pts : " <<num_time_pts<<std::endl;
    //std::cout << "nextPow2     : " <<nextPow2<<std::endl;
    //std::cout << "Front pad    : " <<front_pad<<std::endl;

    //Create zero padded signal (zero matrix with data added to the middle)
    RowMajMatXf padded_signal = RowMajMatXf::Zero( num_channels, nextPow2 );
    padded_signal.block( 0,front_pad,num_channels,num_time_pts ) = input_signals;
    //std::cout << "Size of padded: " << padded_signal.rows() << " by " << padded_signal.cols() << std::endl;

    //Bandstop filters and harmonics
    double centreHz  = vars.bandstop_lower + ((vars.bandstop_upper - vars.bandstop_lower)/2.0);
    double bandwidth = vars.bandstop_upper - vars.bandstop_lower;
    Dsp::SimpleFilter< Dsp::Butterworth::BandStop<5>,1> bandStop;
    bandStop.setup( vars.bandstop_order, vars.getSampleRate(),  centreHz, bandwidth );
    Dsp::SimpleFilter< Dsp::Butterworth::BandStop<5>,1> bandStop2;
    bandStop2.setup( vars.bandstop_order, vars.getSampleRate(), centreHz*2.0, bandwidth );
    Dsp::SimpleFilter< Dsp::Butterworth::BandStop<5>,1> bandStop3;
    bandStop3.setup( vars.bandstop_order, vars.getSampleRate(), centreHz*3.0, bandwidth );

    //Lowpass filter
    Dsp::SimpleFilter< Dsp::Butterworth::LowPass<5>,1> lowPass;
    lowPass.setup( vars.bandstop_order, vars.getSampleRate(), vars.lowpass_cutoff );

    //Highpass filter
    Dsp::SimpleFilter< Dsp::Butterworth::HighPass<5>,1> highPass;
    highPass.setup( vars.bandstop_order, vars.getSampleRate(), vars.highpass_cutoff );

    float* channelPtr[ 1 ]; //filter.process must take an array of pointers...
    for(int i=0; i<num_channels; i++)
    {
       //Define the signal start
       channelPtr[0] = padded_signal.row(i).data();

       //Run forwards if requested
       if( vars.bool_use_bandstop )
       {
           bandStop .process( nextPow2, channelPtr );
           bandStop2.process( nextPow2, channelPtr );
           bandStop3.process( nextPow2, channelPtr );
       }
       if( vars.bool_use_lowpass )
       {
           lowPass.process( nextPow2, channelPtr );
       }
       if( vars.bool_use_highpass )
       {
           highPass.process( nextPow2, channelPtr );
       }

       //Reverse the signal in place
       padded_signal.row(i).reverseInPlace();
       //Redefine pointer (might not to do this...)
       channelPtr[0] = padded_signal.row(i).data();

       //Run backwards to stop phase shift appearing
       if( vars.bool_use_bandstop )
       {
           bandStop .process( nextPow2, channelPtr );
           bandStop2.process( nextPow2, channelPtr );
           bandStop3.process( nextPow2, channelPtr );
       }
       if( vars.bool_use_lowpass )
       {
           lowPass.process( nextPow2, channelPtr );
       }
       if( vars.bool_use_highpass )
       {
           highPass.process( nextPow2, channelPtr );
       }

       //Reverse the signal back to normal
       padded_signal.row(i).reverseInPlace();
    }


    //MODWT drift removal
    if( vars.bool_use_DWTdrift_remove )
    {
        std::cout << "\t-->MODWT" << std::endl;
        auto startMODWT = high_resolution_clock::now();

        Eigen::VectorXd lev_mean( num_channels );
        Eigen::VectorXd lev_std ( num_channels );
        wave_object obj = wave_init( "db4" );
        wt_object wt    = wt_init( obj, "modwt", nextPow2, 9 );

        Eigen::RowVectorXd dat( padded_signal.cols() );
        for( int j=0; j<num_channels; j++ )
        {
            dat = padded_signal.row(j).cast<double>();
            modwt( wt, dat.data() );
            //Map the C-array to an Eigen matrix
            Eigen::Map<RowMajMatXd> modwtMap( wt->output, 10, nextPow2 );

            //The mean
            lev_mean = modwtMap.rowwise().mean();
            //The sdev
            lev_std  = ( ( ( modwtMap.colwise()-lev_mean ).cwiseAbs2().rowwise().sum() ).array()  /  (num_time_pts-1)    ).cwiseSqrt();

            //Now adjust coefficients to take out levels
            for( int i=0; i<vars.DWTdrift_levels_to_keep.size(); i++)
            {
                if( vars.DWTdrift_levels_to_keep[i] == true )
                {
                    //If abs(coeff) > mean+std*thresh then keep coeff, else set zero.
                    modwtMap.row(i) = ( modwtMap.row(i).cwiseAbs().array()
                                        >
                                        lev_mean(i) +
                                        lev_std(i)  * vars.DWTdrift_levels_std_weights(i) )
                               .select
                                      ( modwtMap.row(i).array(), 0);
                }else {  //false
                    modwtMap.row(i).setZero();
                }
            }
            //Invert the coefficients
            imodwt( wt, dat.data() );
            padded_signal.row(j) = dat.cast<float>();
        }
        auto stopMODWT = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stopMODWT - startMODWT);
        std::cout << "\t<--MODWT (" << duration.count() << " msec)" << std::endl;
    }

    //Remove padding
    input_signals = padded_signal.block(0,front_pad,num_channels,num_time_pts);


    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...bandstopFilter() end (" << duration.count() <<" msec)" << std::endl;
}




}//end namespace

#endif // BANDSTOPFILTER_H
