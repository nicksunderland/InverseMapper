#ifndef QRSPEAKFINDER_H
#define QRSPEAKFINDER_H

#include <Eigen/Core>
#include <chrono>
#include <wavelib.h>
#include "definesandstructs.h"
#include "Helper_Functions/printmatrixinfo.h"
#include "igl/find.h"
#include "mesh.h"

using namespace std::chrono;


namespace helper{


void QRSpeakFinder( const Eigen::MatrixXf& input_signals, Eigen::MatrixXf& output_activations, const Filter_vars &vars )
{
    std::cout << "...QRSpeakFinder() start" << std::endl;
    auto start = high_resolution_clock::now();

   // helper::printMatrixInfo( input_signals, "input_signals");

    //Variables
    int chan_to_use  = vars.QRS_channel;
    int num_time_pts = input_signals.cols(); //signal length

    //Copy of the signal
    Eigen::VectorXd signal = input_signals.row( chan_to_use ).cast<double>();

    //Make wave object
    wave_object waveMODWT = wave_init( "db4" );
    //wave_summary( waveMODWT ); //wavelet details

    //Initialise the wavelet transform object
    int dwt_levels = 9; //will yield 9 detail coeff levels and 1 approx level
    wt_object wt = wt_init( waveMODWT, "modwt", num_time_pts, dwt_levels );

    //Run MODWT
    modwt( wt, signal.data() );
    //wt_summary( wt ); //print MODWT output summary

    //Map the C-array to an Eigen matrix
    Eigen::Map<Eigen::MatrixXd> modwtMap( wt->output, num_time_pts, dwt_levels+1 );

   // helper::printMatrixInfo( modwtMap, "modwtMap");

    //Now adjust coefficients to take out levels(now in columns in the Eigen matrix)
    std::vector<bool> keepLevel = {0,0,0,1,1,1,0,0,0,0};
    for( size_t i=0; i<keepLevel.size(); i++)
    {
        if( keepLevel[i] == false )
        {
            modwtMap.col(i) = Eigen::VectorXd::Zero( num_time_pts );
        }
    }

    //Invert the coefficients and overwrite 'signal' with the new data
    imodwt( wt, signal.data() );

    //std::cout << "signal: \n" << signal << std::endl;

    //Get filtered signal standard deviation and set threshold
    double m         = signal.mean();
    double std_dev   = std::sqrt((signal.array() - m).square().sum()/(signal.size()-1));
    double threshold = m + std_dev*vars.QRS_threshold;

    //std::cout << "threshold: " << threshold << std::endl;

    //std::cout << "Signal: " << signal.rows() << " by " << signal.cols() << "\n" << signal.transpose() << std::endl;

    if( vars.QRS_direction == true ) ///positive QRS shape
    {
        signal = ( signal.array() < threshold ) ///zero out all things less than the positive bound of the threshold
                 .select(0,signal);
    }else{                          ///negative QRS shape

        signal = ( signal.array() > -threshold ) ///zero out all things greater than the negative bound of the threshold
                 .select(0,signal);
        signal = signal.cwiseAbs(); ///make the negative 'peaks' positive
    }


    //std::cout << "signalThresh: \n" << signal << std::endl;

    //Loop down the signal looking for peaks
    std::vector< std::vector<float> >peakIdxs;
    bool find_peak = true;
    for( int i=0; i<num_time_pts-1; i++ )
    {
        if( find_peak == true ) //ready to find peaks
        {
            if( signal(i) == 0 ) continue; //skip if in zero part of the signal

            if( signal(i) > signal(i+1) ) //signal was high but then going down
            {
                std::vector<float> d = { float(i), float(signal(i)), 1.0 };
                peakIdxs.push_back( d );  //pushback a peak index and the signal value
                find_peak = false;        //stop finding peaks
            }

        }else {  //not looking for peaks, unless...

            if( signal(i) < signal(i+1) ) //must be going up
            {
                find_peak = true; //restart looking
            }
        }
    }
   // std::cout << "peakIdxs: "<<peakIdxs.size() << std::endl;

    //Clean the peaks
    std::vector<int> cleanPeaks; //
    int window_len = round( vars.getSampleRate() * 0.250 ); //250ms
    if( peakIdxs.size()==0 ) //send '0' if no peaks found
    {
        cleanPeaks.push_back(0);

    }else if( peakIdxs.size() == 1 )
    {
        cleanPeaks.push_back( int(peakIdxs[0][0]) );

    }else if( peakIdxs.size() > 1 )
    {
        for( size_t i=1; i<peakIdxs.size(); i++ )
        {
            int    prev_peak_idx = peakIdxs[i-1][0];
            int    prev_peak_val = peakIdxs[i-1][1];
            int    peak_idx = peakIdxs[i][0];
            float  peak_val = peakIdxs[i][1];
            int win_s = peak_idx - window_len;
            if( win_s < 0 ) win_s = 0;

            if( peak_idx-prev_peak_idx < window_len  )
            {
                if( peak_val > prev_peak_val )
                {
                    peakIdxs[i-1][2] = float(false);
                }else {
                    peakIdxs[ i ][2] = float(false);
                }
            }
        }
        for( size_t i=0; i<peakIdxs.size(); i++ )
        {
            if( peakIdxs[i][2]==float(true) )
            {
                cleanPeaks.push_back( int(peakIdxs[i][0]) );
            }
        }

       // std::cout << "CleanPeaks: "<<cleanPeaks.size() << std::endl;

    }
    Eigen::Map<Eigen::VectorXi> peaks( cleanPeaks.data(), cleanPeaks.size() );

    // helper::printMatrixInfo( peaks, "peaks");

    //Return the QRS timing indicies
    output_activations = Eigen::MatrixXf::Zero( peaks.rows(), 5 );
    output_activations.col(AA::time_idx) = peaks.cast<float>(); //only casting into this size matrix so it fits with the atrial data in the MeshData struct (see Mesh class)

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...QRSpeakFinder() end (" << duration.count() << " msec)" << std::endl;
}



}//end namespace helper

#endif // QRSPEAKFINDER_H
