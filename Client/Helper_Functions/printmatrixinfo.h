#ifndef PRINTMATRIXINFO_H
#define PRINTMATRIXINFO_H

#include <Eigen/Dense>
#include <iostream>
#include <QString>

namespace helper {

template <typename Derived>
void printMatrixInfo( const Eigen::MatrixBase<Derived> &b, QString matrixName )
{
    int rows = 0;
    int cols = 0;
    bool show_block = true;

    if( b.size() == 0 )
    {
        show_block = false;
    }

    if( b.rows() > 5 )
    {
        rows = 5;
    }else {
        rows = b.rows();
    }

    if( b.cols() > 5 )
    {
        cols = 5;
    }else{
        cols = b.cols();
    }

    std::cout << "***************************" << std::endl;
    std::cout << "Matrix Info: '" << matrixName.toStdString() << "'" << std::endl;
    std::cout << "Size: " << b.size() << std::endl;
    std::cout << "Rows: " << b.rows() << "  by  Cols: " << b.cols() << std::endl;
    std::cout << "Matrix Data: " << std::endl;
    if( show_block == false )
    {
        std::cout << "...is empty" << std::endl;
    }else {
        std::cout << b.block(0,0,rows,cols) << std::endl;
    }
    std::cout << "***************************" << std::endl;

}





}//end namespace

#endif // PRINTMATRIXINFO_H
