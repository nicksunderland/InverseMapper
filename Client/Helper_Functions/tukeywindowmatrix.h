#ifndef TUKEYWINDOWMATRIX_H
#define TUKEYWINDOWMATRIX_H

#include <iostream>
#include <Eigen/Core>
#include <QDebug>
#include <chrono>

using namespace std::chrono;

namespace helper {


Eigen::MatrixXd createTukeyWinMatrix( int win_len,
                                      int peakIdx=-1, //if -1 then return peak centred on the midpoint of the window, else centre on the peakIdx
                                      int num_scales=1,
                                      Eigen::VectorXd alphaParams     = Eigen::VectorXd::Ones(1),
                                      Eigen::VectorXi subBaseLenths   = Eigen::VectorXi::Ones(1),
                                      Eigen::VectorXd subHeightRatios = Eigen::VectorXd::Ones(1)  )
{
    std::cout << "\t-->createTukeyWinMatrix() start" << std::endl;
    auto start = high_resolution_clock::now();


    //The matrix to receive the data
    Eigen::MatrixXd tukeyMat = Eigen::MatrixXd::Zero( num_scales, win_len );

    //The Tukey window
    for( int i=0; i<num_scales; i++ )
    {
        double a      = alphaParams(i);         //the shape parameter
        int subBL     = subBaseLenths(i);       //the base length
        double subHR  = subHeightRatios(i);     //the height ratio (0-1)

        //Make some x-data between 0 and 1 for the length of the base
        Eigen::VectorXd x = Eigen::VectorXd::LinSpaced( subBL, 0, 1 );

        //Make a vector for the y-data to go in
        Eigen::VectorXd y( subBL );

        //Fill the y-vector for each value of x
        for( int j=0; j<subBL; j++ )
        {
            if( x(j) >= 0 && x(j) < a/2 )
            {
                y(j) = 0.5*( 1 + cos( (2*M_PI/a)*(x(j)-a/2) ) );
            }else if( x(j) >= a/2 && x(j) < (1- a/2) )
            {
                y(j) = 1;
            }else if ( x(j) >= (1- a/2) && x(j) <= 1 )
            {
                y(j) = 0.5*( 1 + cos( (2*M_PI/a)*(x(j)-1+a/2) ) );
            }else {
                std::cout << "ERROR making Tukey window" << std::endl;
                exit(0);
            }
        }

        if( peakIdx == -1 )
        {
            //place over mid point
            int startIdx = (win_len - subBL) / 2;
            if( startIdx < 0 )
            {
                tukeyMat.row(i) = y.block( -startIdx,0,subBL-2*(-startIdx)-1,1 ).array().transpose() * subHR;
            }else {
                tukeyMat.block(i,startIdx,1,subBL) = y.array().transpose() * subHR;  //add to middle of row and adjust by height ratio
            }
        }else{
            //place over peak index
            int startIdx = floor(double(subBL)/2) - peakIdx;
            int endIdx   = ceil(double(subBL)/2) + peakIdx - 1;

            //std::cout << "WinLen: " << win_len << "\nsubBL: " << subBL << "\nstartIdx: " << startIdx << "\nendIdx: " << endIdx << std::endl;

            if( startIdx>=0 && endIdx<win_len-1 )           //tuk front is off the end by startIdx
            {                                               //tuk back at endIdx
                tukeyMat.block(i,0,1,subBL-startIdx) = y.block(startIdx,0,subBL-startIdx,1).transpose();

            }else if(  startIdx<0 && endIdx<win_len-1  )    //tuk front is at -start idx
            {                                               //tuk back at endIdx
                tukeyMat.block(i,-startIdx,1,subBL) = y.transpose();

            }else if(  startIdx<0 && endIdx>=win_len-1  )   //tuk front is at -start idx
            {                                               //tuk back off by winLen-endIdx
                tukeyMat.block(i,(-startIdx)-1,1,subBL-(endIdx-win_len)) = y.block(0,0,subBL-(endIdx-win_len),1).transpose();

            }else if(  startIdx>=0 && endIdx>=win_len-1  )  //tuk front is off the end by startIdx
            {                                               //tuk back off by winLen-endIdx
                tukeyMat.row(i) = y.block(startIdx+1,0,win_len,1).transpose();
            }
        }

    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << "\t<--createTukeyWinMatrix() end (" << duration.count() << " msec)" << std::endl;

    //Return the matrix
    return tukeyMat;
}



}//end namespace

#endif // TUKEYWINDOWMATRIX_H
