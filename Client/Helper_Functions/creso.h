#ifndef CRESO_H
#define CRESO_H

#include <Eigen/Dense>
#include <dlib/optimization.h>



namespace helper{

//Prototype
double cresoFunct(double t, const Eigen::VectorXf &S, const Eigen::VectorXf &vectAlpha, double &creso_lambda);



double creso( const Eigen::JacobiSVD<Eigen::MatrixXf> &svd, const Eigen::MatrixXf &vectB )
{

    //std::cout << "Creso Function" <<std::endl;
    //Remember this...- see LB's code, for the matrix dimensions we're using she switches U and V... unsure of maths behind it
    Eigen::MatrixXf mat_U = svd.matrixV();
//    std::cout << "New U: " << mat_U.rows() << " by " << mat_U.cols() << "\n"
//                           << mat_U.block(0,0,5,5) << "\n\n\n\n\n" << std::endl;

    Eigen::VectorXf S = svd.singularValues();

    int timePts = vectB.cols();
    Eigen::VectorXf cresoLambdas(timePts);
    for(int i=0; i<timePts; i++) //timePts
    {

        //std::cout << "VectB: " << vectB.col(0) << std::endl;
        //std::cout << "VectS: " << vectS << std::endl;

        Eigen::VectorXf vectAlpha = mat_U.transpose() * vectB.col(i);
        //Eigen::VectorXd S defined above
        //int numElem define above

        //Minimiser function
        const double begin = 1e-8;
        const double end = 1e-2;
        double starting_point = 1e-2;//(maxReg-minReg)/2;
        const double eps = 1e-8;
        const long max_iter = 500;
        const double initial_search_radius = (end-begin)/2;

        double creso_lambda=0;

        double min = dlib::find_min_single_variable([&](double t) {return cresoFunct(t, S, vectAlpha, creso_lambda);},
                                       starting_point,
                                       begin,
                                       end,
                                       eps,
                                       max_iter,
                                       initial_search_radius);
        // CHECK
        //https://stackoverflow.com/questions/47778205/how-to-use-find-min-single-variable-of-dlib-in-c?rq=1




        //MFS's member variable creso_lambda will be set by the minimiser
       // std::cout << "min: " << min << std::endl;
       // std::cout << "cresoLambda : " <<i<<" " <<creso_lambda << std::endl;
        cresoLambdas(i) = creso_lambda;
        //std::cout << "cresoLambda : " <<i<<" " <<creso_lambda << std::endl;

        /***INFO***/
        /*
         * For some reason this creso method behaves quite binary e.g. below, the timePoint
         * lambdas are either 0.01 (the upper range) or very small.... unsure why....
         * changing the setting son the dlib::find_min_single_variable function changes the
         * values a lot (e.g. different tolerances) - doesn't seem to do the same as MATLABs
         * fminbnd function.
         *
                  MATLAB               THIS
            0.00995202737488175	0.0100000000000000
            0.00995202737488175	0.0100000000000000
            0.00995202737488175	0.0100000000000000
            0.00995202737488175	0.0100000000000000
            0.00995202737488175	0.0100000000000000
            0.00252116950068903	1.47684000000000e-08
            0.00195564011753994	1.47684000000000e-08
            0.00184789824986572	1.47684000000000e-08
            0.00194159261279704	1.47684000000000e-08
            0.00199363700750020	1.47684000000000e-08
            0.00208944385762774	1.47684000000000e-08
         *
         *
         *
         *
        */

    }

//std::cout << "cresoLambdas: " << cresoLambdas << std::endl;
    double lambda = sqrt( cresoLambdas.mean() );

    return lambda;
}

double cresoFunct(double t, const Eigen::VectorXf &S, const Eigen::VectorXf &vectAlpha, double& creso_lambda)
{
    int numElem = S.size();

//    if(creso_lambda==0.0038){
//        std::cout << "vecS: " << vectS << std::endl;
//        std::cout << "vecAlpha: " << vectAlpha << std::endl;
//    }

    Eigen::VectorXf oneVector = Eigen::VectorXf::Ones(numElem);
    Eigen::VectorXf tvec = oneVector * t;

   // std::cout << "tvec: " << tvec << std::endl;


//    if(creso_lambda==0.0038){
//        std::cout << "tvec: " << tvec << std::endl;
//    }

    Eigen::VectorXf cresoval = ( (S.array() * vectAlpha.array())  /
                                 (S.array().pow(2) + tvec.array()).array()   ).pow(2);

     //   std::cout << "cresoval_1: " << cresoval << std::endl;

    cresoval = cresoval.array() * (oneVector.array() - ((4*tvec).array() / (S.array().pow(2) + tvec.array())));

     //   std::cout << "cresoval_2: " << cresoval << std::endl;

    double cresoDub = -1 * cresoval.sum();

    //std::cout << "T: " << t << std::endl;
   // std::cout << "cresoDub: " << cresoDub << std::endl;

    creso_lambda = t;   //load the latest lambda value

    return cresoDub;
}


}//end namespace

#endif // CRESO_H
