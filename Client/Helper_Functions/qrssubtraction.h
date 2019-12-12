#ifndef QRSSUBTRACTION_H
#define QRSSUBTRACTION_H

#include <Eigen/Dense>
#include <QDebug>
#include <chrono>
#include <wavelib.h>
#include "igl/median.h"
#include "definesandstructs.h"
#include "Helper_Functions/cwtfunctions.h"
#include "Helper_Functions/tukeywindowmatrix.h"
#include "mesh.h"

using namespace std::chrono;

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXd;
typedef Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajMatXcd;

namespace helper {


//Prototypes
static void getMedianQRSCoeffs(Eigen::Map<RowMajMatXcd> &cwtMap, Eigen::MatrixXd &mQRSwave, Eigen::MatrixXd &dataQRS, const Eigen::Ref<const Eigen::VectorXi> QRS_activation_indices,  const int &pre_QRS, const int &post_QRS, const int &num_time_pts   );
static void subtractWeightedQRS(Eigen::Map<RowMajMatXcd> &cwtMap, const Eigen::Ref<const Eigen::MatrixXd> mQRSwave, Eigen::Ref<Eigen::MatrixXd> dataQRS, const Eigen::Ref<const Eigen::VectorXi> QRS_activation_indices, const Eigen::Ref<const Eigen::MatrixXd> tukeyMatLong, const int &pre_QRS, const int &post_QRS, const int &num_time_pts   );


void qrsSubtractionAlgo( Eigen::MatrixXf& input_signals, const Eigen::MatrixXf& QRS_inds_in, const Filter_vars& vars )
{
    std::cout << "...qrsSubtractionAlgo() start" << std::endl;
    auto start = high_resolution_clock::now();

    //Needed varibles
    RowMajMatXd signals_for_filtering = input_signals.cast<double>();
    Eigen::VectorXi QRS_inds = QRS_inds_in.col(AA::time_idx).cast<int>();
    double sample_rate = vars.getSampleRate();
    double dt          = 1.0 / sample_rate;
    double scale_res   = vars.QRSsub_scale_res;
    double scale_min   = vars.QRSsub_scale_min;
    int num_scales     = vars.getNumQRSSubScales();
    int wavelet_order  = vars.QRSsub_wavelet_order;
    int num_time_pts   = signals_for_filtering.cols();
    int num_channels   = signals_for_filtering.rows();
    int pre_QRS        = round( sample_rate * vars.QRSsub_preQRSwin ); //50ms
    int post_QRS       = round( sample_rate * vars.QRSsub_postQRSwin ); //400ms
    int QRSwinLen      = pre_QRS + post_QRS + 1;


    //Input signal is a row major matrix with signals along the rows
    //Get the total number of data points [rows x cols]
    int total_num_pts = num_channels*num_time_pts; //total num data points

    //In storage terms the signals are just contiguous end to end
    //So feed into the CWT the number of total data points and pointer to the beginning
    cwt_object wt = cwt_init( "dog", wavelet_order, total_num_pts, dt, num_scales ); //CWT object initialisation
    setCWTScales( wt, scale_min, scale_res, "pow", 2 ); //setthe scales power of 2 method
    cwt( wt, signals_for_filtering.data() ); //run the CWT by giving the pointer to the start of the signal matrix


    /* Unsure at the minute if artefact will appear at the beginnings/ends of the
     * signals because the CWT is being run across all of the signals end to end */



    //Map the complex CWT C-array to a complex Eigen matrix
    double* Re_ptr = &wt->output[0].re;                                                 //double pointer to the first data point (real)
    std::complex<double> *convertCD = reinterpret_cast <std::complex<double>*>(Re_ptr); //convert double* to complex*
    Eigen::Map< RowMajMatXcd > cwtMap                                                   //Rowmajor map
                 ( convertCD,                                                           //Pointer to the first real double
                   num_scales*num_channels,                                             //Num rows in map
                   num_time_pts );                                                      //Num time pts
    /*** Map layout for n channels, m scales, and t timepoints
     *             time 0  1  2  3  4  5  6  7  8  ...  t
     * scale 1, chan 1
     * scale 1, chan 2
     * scale 1, chan 3
     *     ...
     * scale 1, chan n         ***  each entry is
     * scale 2, chan 1            a complex double ***
     *     ...
     *     ...
     * scale m, chan 1
     *     ...
     * scale m, chan n
     *
     * ********************************************************/

    /* Work out the scaling change that the iCWT causes with the current settings (can be around 10-20% off with some narrow wavelet settings) */
    RowMajMatXd test( num_channels, num_time_pts );
    icwt( wt, test.data() ); ///iCWT without having changed any coefficients - compared max and min of each channel to original signal
    Eigen::VectorXd posScaling =  ( ( signals_for_filtering.array() > 0 ).select( signals_for_filtering, 0 ).rowwise().maxCoeff().array() /
                                 ( test.array()                  > 0 ).select( test, 0                  ).rowwise().maxCoeff().array()      ).cwiseAbs();
    Eigen::VectorXd negScaling = (  ( signals_for_filtering.array() < 0 ).select( signals_for_filtering, 0 ).rowwise().minCoeff().array() /
                                 ( test.array()                  < 0 ).select( test, 0                  ).rowwise().minCoeff().array()      ).cwiseAbs();






    //Loop through the QRSs and get the coefficients within each window, take the mean and store in mQRSwave matrix
    Eigen::MatrixXd mQRSwave = Eigen::MatrixXd::Zero( num_scales*num_channels, QRSwinLen );   //Preallocate matrix to hold meanQRS result
    Eigen::MatrixXd dataQRS( num_scales*num_channels, QRSwinLen );                            //Preallocate matrix to hold the data of the current QRS in the loops
    helper::getMedianQRSCoeffs( cwtMap, mQRSwave, dataQRS, QRS_inds, pre_QRS, post_QRS, num_time_pts );


    //Create a Tukey window weighting/ratio matrix for the QRS window
    Eigen::MatrixXd tukeyMat = helper::createTukeyWinMatrix( QRSwinLen,
                                                             pre_QRS,
                                                             num_scales,
                                                             vars.QRSsub_Tuk_alphaParams,          //alphaParams
                                                             vars.QRSsub_Tuk_subLengths,           //subLengths
                                                             vars.QRSsub_Tuk_subLenHeightRatios ); //subLenHeightRatios
    Eigen::MatrixXd tukeyMatLong = Eigen::MatrixXd::Zero( num_scales*num_channels, QRSwinLen );    //A matrix placing the appropriate Tukey window over the appropriate levels
    for( int i=0; i<num_scales; i++ )
    {
        tukeyMatLong.block(i*num_channels,0,num_channels,QRSwinLen) = tukeyMat.row(i).replicate(num_channels,1);
    }


    //helper::printMatrixInfo( tukeyMatLong, "tukeyMatLong");


    //Re-loop through the coefficients matrix but subtract a weighted QRS window at each QRS position
    helper::subtractWeightedQRS( cwtMap, mQRSwave, dataQRS, QRS_inds, tukeyMatLong, pre_QRS, post_QRS, num_time_pts );


    // Invert the CWT coefficients and copy to filtered signals matrix
    cwtMap.imag().setZero();
    icwt( wt, signals_for_filtering.data() );


    //Resolve the scaling issues
    signals_for_filtering = ( signals_for_filtering.array() >= 0 ).select( signals_for_filtering.array().colwise()*posScaling.array(),
                                                                           signals_for_filtering.array().colwise()*negScaling.array() );



    //Back to cath object
    input_signals = signals_for_filtering.cast<float>();


    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "...qrsSubtractionAlgo() end (" << duration.count() << " msec)" << std::endl;
}



/////this can't take mQRS or dataQRS Eigen::Refs as it needs to be able to resize the matrices and Ref:: doesn't allow; think it still all ok
static void getMedianQRSCoeffs( Eigen::Map<RowMajMatXcd> &cwtMap, Eigen::MatrixXd &mQRSwave, Eigen::MatrixXd &dataQRS, const Eigen::Ref<const Eigen::VectorXi> QRS_activation_indices,  const int &pre_QRS, const int &post_QRS, const int &num_time_pts   )
{
    std::cout << "\t-->getMedianQRSCoeffs" << std::endl;
    auto start = high_resolution_clock::now();



    int QRSwinLen        = pre_QRS + post_QRS + 1;
    int num_QRS          = QRS_activation_indices.size();
    int store_row_length = mQRSwave.rows();
    RowMajMatXd QRS_coeff_store( store_row_length*QRSwinLen, num_QRS );

    for( int i=0; i<num_QRS; i++ )  //loop through the QRSs
    {
        int qrsPeakTime = QRS_activation_indices(i);    /// index of this QRS's peak (place to construct the QRS window around)
        int qrsWinStart = qrsPeakTime - pre_QRS;        /// index of the start of the QRS window
        int qrsWinEnd   = qrsPeakTime + post_QRS;       /// index of the end of the QRS window
        int actual_QRSwinLen = 0;                       /// reset the actual QRS window length to zero
        if( qrsWinStart < 0 )                           /// i.e. the window overlaps the beginning of the signal
        {   qrsWinStart = 0;                            /// start index can't be < 0 so reset to 0
            actual_QRSwinLen = qrsWinEnd-qrsWinStart+1; /// compute the actual length of this QRS window now the startIdx has been set correctly
        }else if( qrsWinEnd >= num_time_pts )           /// i.e. the window overlaps the end of the signal
        {   qrsWinEnd = num_time_pts-1;                 /// end index can't be > signal array size
            actual_QRSwinLen = qrsWinEnd-qrsWinStart+1; /// compute the actual length of this QRS window now the endIdx has been set correctly
        }else{                                          /// i.e. the window is ok and somewhere in the middle of the signal
            actual_QRSwinLen = QRSwinLen;               /// actual window length is the QRSwinLen, no adjustment
        }
        int sizeDiff = QRSwinLen - actual_QRSwinLen;    /// Get the size difference between the actual and the input QRS window length i.e. ?adjusted
        if( sizeDiff > 0 && qrsWinStart==0 ){           /// i.e. front overlap
            dataQRS.block(0,sizeDiff,store_row_length,actual_QRSwinLen) = cwtMap.block(0,0,store_row_length,actual_QRSwinLen).real();
        }else if( sizeDiff > 0 && qrsWinEnd==num_time_pts-1 ){ /// i.e. end overlap
            dataQRS.block(0,0,store_row_length,actual_QRSwinLen) = cwtMap.block(0,num_time_pts-actual_QRSwinLen,store_row_length,actual_QRSwinLen).real();
        }else {                                         /// i.e. no overlap
            dataQRS = cwtMap.block(0,qrsWinStart,store_row_length,actual_QRSwinLen).real();
        }
        dataQRS.resize( store_row_length*QRSwinLen, 1 );/// make same shape (vector)
        QRS_coeff_store.col(i) = dataQRS;               /// add vector to the median QRS matrix
        dataQRS.resize( store_row_length, QRSwinLen );  /// put matrix back to correct size for next loop
        dataQRS.setZero();                              /// reset to zeros

    } ///end loop gathering QRS coefficients


    mQRSwave.resize( store_row_length*QRSwinLen, 1 );   /// make correct shape to take median from each QRS_coeff_store row
    for( int i=0; i<QRS_coeff_store.rows(); i++ )
    {
        igl::median( QRS_coeff_store.row(i), mQRSwave(i,0) ); ///median of each row, value into mQRSwave(:)
    }
    mQRSwave.resize( store_row_length, QRSwinLen );             /// The mean of all the QRS windows - was using median in Matlab but would need to do a sort algorithm in C++... maybe later


    ///Adjust ends to zero
    Eigen::VectorXd first = mQRSwave.col(0);
    Eigen::VectorXd last  = mQRSwave.col(QRSwinLen-1);
    Eigen::MatrixXd x     = Eigen::VectorXd::LinSpaced(QRSwinLen,0,1).transpose().replicate( mQRSwave.rows(), 1 );
    x = (x.array().colwise() * (last.array() - first.array())) + first.replicate(1,QRSwinLen).array();
    mQRSwave = mQRSwave.array() - x.array();



    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "\t<--getMedianQRSCoeffs (" << duration.count() << " msec)" << std::endl;
}



static void subtractWeightedQRS(Eigen::Map<RowMajMatXcd> &cwtMap, const Eigen::Ref<const Eigen::MatrixXd> mQRSwave, Eigen::Ref<Eigen::MatrixXd> dataQRS, const Eigen::Ref<const Eigen::VectorXi> QRS_activation_indices, const Eigen::Ref<const Eigen::MatrixXd> tukeyMatLong, const int &pre_QRS, const int &post_QRS, const int &num_time_pts   )
{
    auto start = high_resolution_clock::now();
    std::cout << "\t-->subtractWeightedQRS" << std::endl;


    int QRSwinLen        = pre_QRS + post_QRS + 1;
    int num_QRS          = QRS_activation_indices.size();
    int store_row_length = mQRSwave.rows();

    for( int i=0; i<num_QRS; i++ )
    {
        //Reset to zero
        dataQRS  = Eigen::MatrixXd::Zero( store_row_length, QRSwinLen );

        int qrsPeakTime = QRS_activation_indices(i);
        int qrsWinStart = qrsPeakTime - pre_QRS;
        int qrsWinEnd   = qrsPeakTime + post_QRS;
        int actual_QRSwinLen = 0;
        if( qrsWinStart < 0 )
        {   qrsWinStart = 0; //start index can't be <0
            actual_QRSwinLen = qrsWinEnd-qrsWinStart+1;
        }else if( qrsWinEnd >= num_time_pts )
        {   qrsWinEnd = num_time_pts-1; //end index can't be > signal array size
            actual_QRSwinLen = qrsWinEnd-qrsWinStart+1;
        }else{
            actual_QRSwinLen = qrsWinEnd-qrsWinStart+1;
        }
        int sizeDiff = QRSwinLen - actual_QRSwinLen;
        if( sizeDiff > 0 && qrsWinStart==0 )
        {   dataQRS.block(0,sizeDiff,store_row_length,actual_QRSwinLen) = cwtMap.block(0,0,store_row_length,actual_QRSwinLen).real();
        }else if( sizeDiff > 0 && qrsWinEnd==num_time_pts-1 )
        {   dataQRS.block(0,0,store_row_length,actual_QRSwinLen) = cwtMap.block(0,num_time_pts-actual_QRSwinLen,store_row_length,actual_QRSwinLen).real();
        }else {
            dataQRS = cwtMap.block(0,qrsWinStart,store_row_length,actual_QRSwinLen).real();
        }

        Eigen::MatrixXd subtraction;
        if( sizeDiff > 0 && qrsWinStart==0 )
        {   subtraction = (dataQRS.array()*tukeyMatLong.array() + mQRSwave.array()*(1-tukeyMatLong.array())).block(0,sizeDiff,store_row_length,actual_QRSwinLen);
        }else if( sizeDiff > 0 && qrsWinEnd==num_time_pts-1 )
        {   subtraction = (dataQRS.array()*tukeyMatLong.array() + mQRSwave.array()*(1-tukeyMatLong.array())).block(0,0,store_row_length,actual_QRSwinLen);
        }else {
            subtraction = dataQRS.array()*tukeyMatLong.array() + mQRSwave.array()*(1-tukeyMatLong.array());
        }

        //Adjust the ends to zero, to stop steps appearing in the signal
        Eigen::VectorXd first = subtraction.col(0);
        Eigen::VectorXd last  = subtraction.col(actual_QRSwinLen-1);
        Eigen::MatrixXd x     = Eigen::VectorXd::LinSpaced(actual_QRSwinLen,0,1).transpose().replicate( subtraction.rows(), 1 );
        x = (x.array().colwise() * (last.array() - first.array())) + first.replicate(1,actual_QRSwinLen).array();
        subtraction = subtraction.array() - x.array();

        //Subtract the weighted and adjusted QRS window from the signal
        cwtMap.block(0,qrsWinStart,store_row_length,actual_QRSwinLen).real() =
                cwtMap.block(0,qrsWinStart,store_row_length,actual_QRSwinLen).real() - subtraction;

    }


    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "\t<--subtractWeightedQRS (" << duration.count() << " msec)" << std::endl;
}



} //end namespace helper

#endif // QRSSUBTRACTION_H
