#ifndef ATRIALACTIVATIONFINDER_H
#define ATRIALACTIVATIONFINDER_H

#include <Eigen/Dense>
#include <chrono>
#include "Helper_Functions/cwtfunctions.h"
#include "definesandstructs.h"
#include "Helper_Functions/printmatrixinfo.h"
#include "igl/mat_max.h"
#include "igl/sort.h"
#include "igl/slice.h"
#include "igl/unique.h"
#include "igl/slice_mask.h"
#include "enums.h"

using namespace std::chrono;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXf;


namespace helper {


//Prototypes
static void lagWindowThresholding ( Eigen::Ref< RowMajMatXf > lagData, Eigen::Ref<RowMajMatXf> S, Eigen::Ref<RowMajMatXf> tmp, Eigen::VectorXi &max_idxs, Eigen::VectorXf &max_vals, Eigen::MatrixXf &data_info_mat, const Filter_vars &vars, int lagWin_start_idx  );
static void sortByChannelsThenTime( Eigen::Ref< Eigen::MatrixXf >acti_info_mat );
static void pruneActivations      ( Eigen::MatrixXf &acti_info_mat, const Filter_vars &vars );



void atrialActivationFinder( const Eigen::MatrixXf& input_signals, Eigen::MatrixXf& acti_info_mat, const Filter_vars &vars )
{
    std::cout << "...atrialActivationFinder() start" << std::endl;
    auto start = high_resolution_clock::now();

    //Needed variables
    double sample_rate = vars.getSampleRate();
    double dt          = 1.0 / sample_rate;
    double scale_res   = vars.atrial_search_scale_res;
    double scale_min   = vars.atrial_search_scale_min;
    int num_scales     = vars.atrial_search_num_scales;
    int wavelet_order  = vars.atrial_search_wavelet_order;
    int lagLen         = round( sample_rate * vars.atrial_search_lag_win_length_secs );
    int stepLen        = round( sample_rate * vars.atrial_search_step_win_length_secs );
    int num_time_pts   = input_signals.cols();
    int num_channels   = input_signals.rows();
    int total_rows     = num_scales * num_channels;


    // Get the CWT coefficients using the settings for atrial activation searching (normally Gaus1 wavelet)
    RowMajMatXf gaus_coeffs = gaus_cwt( input_signals,
                                        dt,
                                        scale_res,
                                        scale_min,
                                        num_scales,
                                        wavelet_order  );

    //Zero out the negative coefficients and square the rest to get the power
    gaus_coeffs = ( gaus_coeffs.array() > 0 ).select( gaus_coeffs.array(), 0 );
    gaus_coeffs.cwiseAbs2();

    /*** Matrix layout for n channels, m scales, and t timepoints
     *             time 0  1  2  3  4  5  6  7  8  ...  t
     * scale 1, chan 1
     * scale 2, chan 1
     * scale 3, chan 1
     *     ...             <---  Row major storage --->
     * scale m, chan 1         ***  each entry is
     * scale 1, chan 2                a double    ***
     *     ...
     *     ...
     * scale m, chan 2
     *     ...
     * scale 1, chan n
     *     ...
     * scale m, chan n
     * ********************************************************/
    //helper::printMatrixInfo( gaus_coeffs, "Gaus1 coeffs");


    //Preallocate matrices for the s_dev calculations
    RowMajMatXf S      ( total_rows, stepLen );         //the holds thresholded sdev values - size( num_chans*num_scales  by  stepLength )
    RowMajMatXf tmp    ( total_rows, lagLen-stepLen );  //a tmp matrix that holds some of the sdev calculations
    RowMajMatXf lagData( total_rows, lagLen  );         //a matrix to hold the block of the coefficients we're currently working on
    Eigen::VectorXi max_idxs( total_rows );             //Vector to hold the indices of the max power value in each row
    Eigen::VectorXf max_vals( total_rows );             //Vector to hold the value of the max power in each row


    //Initialise matrix to take final activation information
    /*
     *   row# = activation# found
     *   col#1-5: time_idx - the time index the activation was found at
     *            channel  - the channel in which it was found
     *            wav_scale- the wavelet scale in which the power peak was highest
     *            power    - the power peak value
                  keep_bool- a bool value used for pruning (should all =1/true by the time this function returns)
     */
    //////RowMajMatX5f acti_info_mat - is a member variable now, passed in by reference

    //Run a search across the signals (there are now num_channels*num_scales signals/rows)
    std::cout << "\t-->lagWindowThresholding loop" << std::endl;
    auto startloop = high_resolution_clock::now();

    for( int j=lagLen-1; j<num_time_pts; j+=stepLen ) //advance in steps because doing every timestep takes ages and doesn't add much
    {
        int lagWin_s  = j-lagLen+1;  ///index of start of the lag window

        /****** MATRIX SEARCH SETUP *****************************
         *
         *   storage = row major
         *   s = scale
         *   c = channel
         *   n = num_channels
         *   m = num_scales
         *
         *
         *          0  1  2  3  4  5  6  7  8  9  10  11  12  13  14 ... num_time_pts
         *                                  stepWin_s        stepWin_e
         *      lagWin_s                                     lagWin_e
         *  s1c1   |<-----------------Lag window--------------->|
         *  s2c1   |<------Sublag window------>|
         *  s3c1                                |<-Step window->|
         *   ...
         *  s1cn
         *   ...
         *  smcn      *calculate rowwise std*      *compare data*
         *             of the sublag window       in this window to
         *                     data              the std (x threshold)
         *
         * ********************************************************/

        // to copy the block data out into a preallocated matrix and then compute the SDEV rather than feeding
        // in the the block directly - it seems to be quicker (I guess because the rows needed for the calculation
        // are actually far away in terms of memory because it is row major and the signals can be quite long
        lagData = gaus_coeffs.block(0,lagWin_s,total_rows,lagLen).cast<float>(); //cast float, faster

        //Threshold the values and return the activations in the acti_info_mat
        helper::lagWindowThresholding( lagData, S, tmp, max_idxs, max_vals, acti_info_mat, vars, lagWin_s );

    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - startloop);
    std::cout << "\t<--lagWindowThresholding loop (" << duration.count() << " msec)" << std::endl;


    //Sort activations by time then by channel - inplace
    helper::sortByChannelsThenTime( acti_info_mat );


    //Run a search down the found activation data to prune off ones that don't meet
    //certain criteria
    helper::pruneActivations( acti_info_mat, vars );   //<---output return in place in acti_info_mat


    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...atrialActivationFinder() end (" << duration.count() << " msec)" << std::endl;
}







static void lagWindowThresholding( Eigen::Ref< RowMajMatXf > lagData, //data from the whole lag window (sublag+step)
                            Eigen::Ref< RowMajMatXf > S,       //matrix to hold calculated sdev during compairson with step window
                            Eigen::Ref< RowMajMatXf > tmp,     //preallocated tmp to to hold calculation during sdev (seems to speed things up)
                            Eigen::VectorXi &max_idxs,       //igl function wont take a Ref<> ...  preallocated vector to take max col indices in each channel of the step window
                            Eigen::VectorXf &max_vals,       //igl function wont take a Ref<> ...  preallocated vector to take max col power values in each channel of the step window
                            Eigen::MatrixXf &data_info_mat,  //Matrix to keep the found activations and associated data - see enums
                            const Filter_vars &vars,         //variables
                            int lagWin_start_idx  )          //the start index of the lag window
{
    // Function thresholds the data in the step window based on the sdev of the data in the preceding/trailing
    // sublag window

    //Window and subwindow lengths
    int lagLen     = lagData.cols();
    int stepLen    = S.cols();
    int subLen     = lagLen-stepLen;
    int rowLen     = lagData.rows();

    //Sdev replicated across the step win
    tmp = (lagData.block(0,0,rowLen,subLen).colwise() - (lagData.block(0,0,rowLen,subLen).rowwise().mean()) ).cwiseAbs2();
    S   = ( tmp.rowwise().sum().array() / (subLen-1) ).cwiseSqrt().replicate(1,S.cols());

    //The step window data...
    lagData.block(0,subLen,rowLen,stepLen) =
            //if step window data                         > sdev of data in sublag*thresh
         ( lagData.block(0,subLen,rowLen,stepLen).array() > S.array() * vars.atrial_search_threshold )
            //then keep step window data           else...set to zero
  .select( lagData.block(0,subLen,rowLen,stepLen).array(), 0 );

    if( lagWin_start_idx == 0 ) //if at the beginning remove the beginning data
    {
        lagData.block( 0,0,rowLen,subLen ).setZero();
    }

    //Find the max value in each row of the step window
    igl::mat_max( lagData.block(0,subLen,rowLen,stepLen), 2, max_vals, max_idxs );

    //Ignore max value if the power is zero (whole row must be zeros), else insert max points into data_info matrix
    int initial_rows = data_info_mat.rows();
    int nnz          = (max_vals.array() != 0).count();
    data_info_mat.conservativeResize( initial_rows+nnz, 5 );

    int counter = 0;
    for( int i=0; i<max_vals.size(); i++ )
    {
        if( max_vals(i) == 0 )
        {
            continue;
        }else {
            int insertion_idx = initial_rows + counter;
            data_info_mat( insertion_idx, AA::time_idx  ) = lagWin_start_idx + subLen + max_idxs(i);    //beginning of lag + sublag + time index of step window = overall timeIdx
            data_info_mat( insertion_idx, AA::channel   ) = i / vars.atrial_search_num_scales; //channels
            data_info_mat( insertion_idx, AA::wav_scale ) = i % vars.atrial_search_num_scales; //scales stored down rows in a 1->10 (idx 0->9) repeating pattern
            data_info_mat( insertion_idx, AA::power     ) = max_vals(i);
            data_info_mat( insertion_idx, AA::keep_bool ) = 1;      //use this later
            counter++;
        }
    }

}






static void sortByChannelsThenTime(Eigen::Ref<Eigen::MatrixXf> acti_info_mat )
{
    std::cout << "\t-->sortByChannelsThenTime" << std::endl;
    auto start = high_resolution_clock::now();


    Eigen::MatrixXf sorted   ( acti_info_mat.rows(), acti_info_mat.cols() );
    Eigen::MatrixXf tmp_data ( acti_info_mat.rows(), 1 );
    Eigen::MatrixXi sort_idxs( acti_info_mat.rows(), 1 );

    igl::sort ( acti_info_mat.col(AA::time_idx), 1, true, tmp_data, sort_idxs );
    igl::slice( acti_info_mat, sort_idxs, 1, sorted );

    //std::cout << "sorted data: \n" << sorted << std::endl;

    Eigen::VectorXf unique_chans;
    {
        Eigen::VectorXi tmp_ix, tmp_iix;
        igl::unique( sorted.col(AA::channel), unique_chans, tmp_ix, tmp_iix );
    }

    //std::cout << "unique_chans: \n" << unique_chans << std::endl;

    int start_idx = 0;
    for( int i=0; i<unique_chans.size(); i++ )
    {
        Eigen::Array<bool,Eigen::Dynamic,1> chanBool =
                ( sorted.col(AA::channel).array() == unique_chans(i) )
                .select( Eigen::Array<bool,Eigen::Dynamic,1>::Constant(sorted.rows(),true), false);

        int num_entries = ( sorted.col(AA::channel).array() == unique_chans(i) ).count();
        Eigen::MatrixXf chop( num_entries, 5 );
        igl::slice_mask( sorted, chanBool, 1, chop );
        acti_info_mat.block(start_idx,0,num_entries,5) = chop;
        start_idx += num_entries;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "\t<--sortByChannelsThenTime (" << duration.count() << " msec)" <<  std::endl;
}




static void pruneActivations(Eigen::MatrixXf &acti_info_mat, const Filter_vars &vars )
{
    std::cout << "\t-->pruneActivations" << std::endl;
    auto start = high_resolution_clock::now();


    //The minimum allowed distance between activations
    int acti_intv = round( vars.getSampleRate() * vars.atrial_search_min_activation_interval_secs );
    //std::cout << "Acti interval: " << acti_intv << "(ints), " << vars.atrial_search_min_activation_interval_secs << "(secs in struct)" << std::endl;
    //helper::printMatrixInfo( acti_info_mat, "INFO MAT PrePrune");

    //Loop through all the found activations
    for( int i=0; i<acti_info_mat.rows()-1; i++ )
    {

        if( acti_info_mat( i, AA::keep_bool ) == true )  //ignore enteries that have already been nulled for some reason
        {
            bool makeComparisons = true;
            int  comparitor      = i+1;   //for each activation start at it's own index and compare it with subsequent enteries until makeComparisons (we're comparing time, i.e. are they too close together)

            //Do comparisons
            while( makeComparisons == true ) //keep comparing until an activation is found that isn't too close, or until this activation is nulled because there is a bigger power activation in close proximity
            {
                //Get time difference between 2 subsequent activations
                int t_diff = acti_info_mat( comparitor , AA::time_idx ) -
                             acti_info_mat(     i      , AA::time_idx );

                //If the time difference is less than the min activation interval
                //(and >=0, negatives happen when going across different channels
                //i.e acti from end of one compared to acti at beginning of another )
                if( t_diff < acti_intv && t_diff >= 0 )
                {
                    if( acti_info_mat( comparitor , AA::power ) > acti_info_mat( i, AA::power ) )
                    {
                        acti_info_mat(      i     , AA::keep_bool ) = 0; //next power was bigger so null this
                        makeComparisons = false;  //removed this activation so stop comparing and move to the next (i++)
                    }else {
                        acti_info_mat( comparitor , AA::keep_bool ) = 0; //this power was bigger so null the comparitor
                    }

                    if( comparitor < acti_info_mat.rows()-1 )
                    {
                        comparitor++;  //still within the range, can keep checking, advance comparitor to the next
                    }else {
                        makeComparisons = false;  //at the end, stop checking
                    }
                }else {
                    makeComparisons = false;  //the next activation is > min distance, i.e. ok, so stop checking this activation and advance to the next activation that hasn't been nulled
                }//end if time diff too short
            }//end while
        }else {
            continue;
        }//end keep bool
    }//end loop of all activations

    //helper::printMatrixInfo( acti_info_mat, "INFO MAT PostBoolLoop");

    //Remove the enteries that have been nulled
    int num_to_keep = (acti_info_mat.col(AA::keep_bool).array() == 1).count();
    Eigen::Array<bool,Eigen::Dynamic,1> keepBoolTrue =
            ( acti_info_mat.col(AA::keep_bool).array() == 1 )
            .select( Eigen::Array<bool,Eigen::Dynamic,1>::Constant(acti_info_mat.rows(),true), false);
    Eigen::MatrixXf chop( num_to_keep, 5 );
    igl::slice_mask( acti_info_mat, keepBoolTrue, 1, chop );
    acti_info_mat.conservativeResize( num_to_keep, 5 );
    acti_info_mat = chop;

    //helper::printMatrixInfo( acti_info_mat, "INFO MAT Post Resize/Chop");

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "\t<--pruneActivations (" << duration.count() << " msec)" << std::endl;
}




}//end namespace

#endif // ATRIALACTIVATIONFINDER_H
