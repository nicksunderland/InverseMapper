#ifndef ATRIALSIGNALRECONSTRUCT_H
#define ATRIALSIGNALRECONSTRUCT_H

#include <Eigen/Dense>
#include <chrono>
#include "definesandstructs.h"
#include "Helper_Functions/tukeywindowmatrix.h"
#include "Helper_Functions/printmatrixinfo.h"
#include "wavelib.h"

using namespace std::chrono;

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXd;
typedef Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXcd;

namespace helper {


void atrialSignalReconstructor( Eigen::MatrixXf& signals_in, const Eigen::MatrixXf& acti_info_mat, const Filter_vars &vars )
{
    std::cout << "...atrialSignalReconstructor() start" << std::endl;
    auto start = high_resolution_clock::now();


    //Variables
    RowMajMatXd input_signals = signals_in.cast<double>();
    int num_time_pts   = input_signals.cols();
    int num_chans      = input_signals.rows();
    int total_pts      = num_chans * num_time_pts;
    int wavelet_order  = vars.atrial_recon_wavelet_order;
    int num_scales     = vars.getNumAtrialReconScales();
    double scale_min   = vars.atrial_recon_scale_min;
    double scale_res   = vars.atrial_recon_scale_res;
    int pre_atrial     = round( vars.getSampleRate() * vars.atrial_recon_pre_win_secs  );
    int post_atrial    = round( vars.getSampleRate() * vars.atrial_recon_post_win_secs );
    int win_len        = pre_atrial + post_atrial + 1;
    double sample_rate = vars.getSampleRate();
    double dt          = 1.0 / sample_rate;
    int num_activations= acti_info_mat.rows();


    //CWT object
    cwt_object wt = cwt_init( "dog", wavelet_order, total_pts, dt, num_scales );
    setCWTScales( wt, scale_min, scale_res, "pow", 2 );

    //Run CWT on all data
    cwt( wt, input_signals.data() );

    //Set up an Eigen::Map to the C array output of the CWT
    double* Re_ptr = &wt->output[0].re;                                                 //double pointer to the first data point (real)
    std::complex<double> *convertCD = reinterpret_cast <std::complex<double>*>(Re_ptr); //convert double* to complex*
    Eigen::Map< RowMajMatXcd > cwtMap( convertCD, num_scales, total_pts );

    //Invert each scale
    RowMajMatXcd icwt_copy = cwtMap;
    RowMajMatXd  icwt_sigs( num_scales, total_pts );
    for( int i=0; i<num_scales; i++ )
    {
        //Reset memory pointed to in wt object to zeros
        cwtMap.setZero();

        //Fill memory with 1 scale at a time
        cwtMap.row(i) = icwt_copy.row(i);

        //Run iCWT with only that scale and run the answer into the sigs matrix, repeat for the other scales
        icwt( wt, icwt_sigs.row(i).data() );
    }


    //Create a time mask outside the loop as will be the same for all reconstructions
    RowMajMatXd tukeyTimeMask = createTukeyWinMatrix( win_len,
                                                          pre_atrial + 1,
                                                          num_scales,
                                                          vars.atrial_recon_alphaParams,
                                                          Eigen::VectorXi::Constant(num_scales,win_len),
                                                          vars.atrial_recon_subLenHeightRatios );

    //Reset the input signals to zeros
    input_signals.setZero();

    //Preallocate the scale matrix - it'll change each loop (but stay the same size)
    Eigen::VectorXd scaleMask( num_scales );
    RowMajMatXd tmp_data( num_scales, win_len );
    Eigen::RowVectorXd tmp_sig_out;
    double ratio = 1;

    //Loop the activations and pull out the data, mask, then splice back into input signals
    for( int i=0; i<num_activations; i++ )
    {
        int chan                = acti_info_mat( i, AA::channel   );
        int time                = acti_info_mat( i, AA::time_idx  );
        int scale               = acti_info_mat( i, AA::wav_scale );
        int time_in_icwt_sigs   = time + ( chan*num_time_pts );
        int beg_idx_this_window = chan * num_time_pts;
        int end_idx_this_window = (chan * num_time_pts) + (num_time_pts - 1);


        //Create scale mask
        scaleMask.block(0,0,scale+1,1)                  = Eigen::VectorXd::LinSpaced(scale+1,1,1);
        scaleMask.block(scale+1,0,num_scales-scale-1,1) = Eigen::VectorXd::LinSpaced(num_scales-scale-1,0.9,0);


        //Set up the atrial window and grab the window data for this activation
        int sig_win_s    = time - pre_atrial;
        if( sig_win_s < 0 ){   sig_win_s   = 0; }


        int atrial_win_s = time_in_icwt_sigs - pre_atrial;
        int atrial_win_e = time_in_icwt_sigs + post_atrial;
        int actual_win_len = 0;
        if( atrial_win_s < beg_idx_this_window )
        {   atrial_win_s   = beg_idx_this_window;
            actual_win_len = atrial_win_e-atrial_win_s+1;
        } else if( atrial_win_e > end_idx_this_window )
        {   atrial_win_e = end_idx_this_window;
            actual_win_len = atrial_win_e-atrial_win_s+1;
        }else {
            actual_win_len = win_len;
        }
        int sizeDiff = win_len - actual_win_len;


        ///Get original max and min
        std::cout<<"before"<<std::endl;
        double range_orig = signals_in.block(chan,sig_win_s,1,actual_win_len).maxCoeff() - signals_in.block(chan,sig_win_s,1,actual_win_len).minCoeff();

        //helper::printMatrixInfo( signals_in, "signals_in");

        if( sizeDiff > 0 && atrial_win_s==beg_idx_this_window ){           /// i.e. front overlap

            std::cout<<"1"<<std::endl;

            RowMajMatXd tmp_data_F = icwt_sigs.block(0,atrial_win_s,num_scales,actual_win_len);
            tmp_data_F = tmp_data_F.colwise() - tmp_data_F.rowwise().mean();
            tmp_data_F = tmp_data_F.array() * scaleMask.replicate(1,tmp_data_F.cols()).array();
            tmp_data_F = tmp_data_F.array() * tukeyTimeMask.block(0,sizeDiff,num_scales,actual_win_len).array();
            Eigen::RowVectorXd tmp_sig_out_F = tmp_data_F.colwise().sum();
            ratio = range_orig / (tmp_sig_out_F.maxCoeff()-tmp_sig_out_F.minCoeff());
            input_signals.block(chan,sig_win_s,1,actual_win_len) = input_signals.block(chan,sig_win_s,1,actual_win_len).array() + (tmp_sig_out_F.array() * ratio);

            std::cout<<"2"<<std::endl;
        }else if( sizeDiff > 0 && atrial_win_e==end_idx_this_window ){ /// i.e. end overlap

            std::cout<<"3"<<std::endl;
            RowMajMatXd tmp_data_B = icwt_sigs.block(0,atrial_win_s,num_scales,actual_win_len);
            tmp_data_B = tmp_data_B.colwise() - tmp_data_B.rowwise().mean();
            tmp_data_B = tmp_data_B.array() * scaleMask.replicate(1,tmp_data_B.cols()).array();
            tmp_data_B = tmp_data_B.array() * tukeyTimeMask.block(0,0,num_scales,actual_win_len).array();
            Eigen::RowVectorXd tmp_sig_out_B = tmp_data_B.colwise().sum();
            ratio = range_orig / (tmp_sig_out_B.maxCoeff()-tmp_sig_out_B.minCoeff());
            input_signals.block(chan,sig_win_s,1,actual_win_len) = input_signals.block(chan,sig_win_s,1,actual_win_len).array() + (tmp_sig_out_B.array() * ratio);

            std::cout<<"4"<<std::endl;
        }else {                                         /// i.e. no overlap

            std::cout<<"5"<<std::endl;
            tmp_data = icwt_sigs.block(0,atrial_win_s,num_scales,actual_win_len);
            tmp_data = tmp_data.colwise() - tmp_data.rowwise().mean();
            //Apply scale mask
            tmp_data = tmp_data.array() * scaleMask.replicate(1,tmp_data.cols()).array() * tukeyTimeMask.array();
            //Sum the levels
            tmp_sig_out = tmp_data.colwise().sum();
            //Ensure signal out same size as signal in
            ratio = range_orig / (tmp_sig_out.maxCoeff()-tmp_sig_out.minCoeff());
            input_signals.block(chan,sig_win_s,1,actual_win_len) = input_signals.block(chan,sig_win_s,1,actual_win_len).array() + (tmp_sig_out.array() * ratio);

            std::cout<<"6"<<std::endl;

        }

    }

    //Back to the catheter object
    signals_in = input_signals.cast<float>();


    auto stop     = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...atrialSignalReconstructor() end (" << duration.count() << " msec)" << std::endl;

}





}//end namespace


#endif // ATRIALSIGNALRECONSTRUCT_H
