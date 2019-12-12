#ifndef TIKHONOVREGULARISATION_H
#define TIKHONOVREGULARISATION_H

#include <Eigen/Dense>
#include <iostream>

namespace helper{


Eigen::VectorXf tikhonovRegularisation(const Eigen::JacobiSVD<Eigen::MatrixXf> &svd,
                                             const Eigen::MatrixXf &cathDataOneTimePoint,
                                             double lambda)
{


    //Key bit here - see LB's code, for the matrix dimensions we're using she switches U and V... unsure of maths behind it
    Eigen::MatrixXf mat_U = svd.matrixV();
//    std::cout << "New U: " << mat_U.rows() << " by " << mat_U.cols() << "\n"
//                           << mat_U.block(0,0,5,5) << "\n\n\n\n\n" << std::endl;
//    std::cout << "New V: " << svd.matrixU().rows() << " by " << svd.matrixU().cols() << std::endl;

    Eigen::MatrixXf mat_V = svd.matrixU();//.block( 0,0, 1+V_a_source.rows()+V_c.rows(), 2*V_c.rows() );
//    std::cout << "New V: " << mat_V.rows() << " by " << mat_V.cols() << "\n"
//                           << mat_V.block(0,0,5,5) << "\n\n\n\n\n" << std::endl;

//    std::cout << "singularValues: " << svd.singularValues().rows() << " by " << svd.singularValues().cols() << std::endl;
    int m = mat_U.rows();
    int n = mat_V.rows();
    int p = svd.singularValues().rows();
    Eigen::VectorXf beta = mat_U.block(0,0,m,p).transpose() * cathDataOneTimePoint;
//    std::cout << "beta: " << beta.rows() << "\n"
//                          << beta << std::endl;
    Eigen::VectorXf zeta = (svd.singularValues().array() * beta.array() ).matrix();
//    std::cout << "zeta: " << zeta.rows() << "\n"
//                          << zeta << std::endl;
    Eigen::VectorXf s = svd.singularValues();

    Eigen::VectorXf x_lambda(n);

//    std::cout << "Regularisation:\n m: " <<m<<" n: "<<n<<" p: "<<p
//              <<" \nbeta size: "<< beta.rows() <<" by "<<beta.cols()
//              << "\nzeta size: " << zeta.rows() <<" by "<<zeta.cols()
//              << "\ns size: "<<s.rows()<<" by "<<s.cols()
//              <<" \nx_lambda size: "<< x_lambda.rows() << " by " << x_lambda.cols() << std::endl;

    int VmatRows = mat_V.rows();

    x_lambda = mat_V.block(0,0,VmatRows,p) *
               ( zeta.array() /  ( (   s.array().pow(2)   ) + (  std::pow(lambda, 2)  ) )).matrix()  ;


//    std::cout << "x_lambda: " << x_lambda.rows() << " by " << x_lambda.cols() << "\n"
//                              << x_lambda << std::endl;



    return x_lambda;
}

}//end namespace

#endif // TIKHONOVREGULARISATION_H
