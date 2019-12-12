#ifndef CREATEACTIVATIONHISTORYMAP_H
#define CREATEACTIVATIONHISTORYMAP_H

#include <Eigen/Dense>
#include <chrono>
#include <iostream>
#include "definesandstructs.h"
#include "Helper_Functions/printmatrixinfo.h"
#include "Helper_Functions/atrialactivationfinder.h"
#include "mesh.h"


using namespace std::chrono;

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXd;

namespace helper {


void createActivationHistoryMap( Eigen::MatrixXf& input_signals, const Eigen::MatrixXf& acti_info_mat, const Filter_vars &vars )
{
    std::cout << "...createActivationHistoryMap() start" << std::endl;
    auto start = high_resolution_clock::now();


    //helper::printMatrixInfo( mEndo_activations, "Endocardial activations info matrix" );
    //Needed variables
    double sample_rate   = vars.getSampleRate();
    int hyst_lag_len     = round( vars.activation_history_win_secs * sample_rate );
    int num_time_pts     = input_signals.cols();
    int max_val          = vars.activation_history_max_val;

    //Set the output data matrix to zeros
    input_signals.setConstant( 0.00001 ); //set to grey color

    //Hysteresis data
    Eigen::RowVectorXf hyst_data = Eigen::RowVectorXf::LinSpaced(hyst_lag_len,max_val,0);
    for( int i=0; i< acti_info_mat.rows(); i++ )
    {
        int chan                = acti_info_mat( i, AA::channel   );
        int time                = acti_info_mat( i, AA::time_idx  );
        int hyst_end_idx        = time + hyst_lag_len - 1;
        int actual_len = 0;
        if( hyst_end_idx > num_time_pts-1 )
        {
            hyst_end_idx = num_time_pts-1;
            actual_len   = hyst_end_idx-time+1;
        }else{
            actual_len   = hyst_lag_len;
        }

        //Scale activation history map
        input_signals.block( chan,time,1,actual_len ) = hyst_data.block(0,0,1,actual_len);
    }


    auto stop     = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...createActivationHistoryMap() end (" << duration.count() << " msec)" << std::endl;
}


}//end namespace

#endif // CREATEACTIVATIONHISTORYMAP_H
