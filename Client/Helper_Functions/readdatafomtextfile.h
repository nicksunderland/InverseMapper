#ifndef READDATAFOMTEXTFILE_H
#define READDATAFOMTEXTFILE_H

#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <QString>
#include <Eigen/Dense>

namespace helper {

void readDataFromTextFile( QString file_path, Eigen::MatrixXf &data )
{
    //std::cout << "readDataFromTextFile()" << std::endl;

    std::ifstream inputFile ( file_path.toStdString() );
    if(!inputFile.is_open())
    {
        std::cout << "Error opening data file" << std::endl;
        QMessageBox::warning(nullptr, "Mesh Data Load Error", "The data file could not be opened, please check the file path");
        return;
    }

    std::string line;
    std::getline(inputFile, line);
    std::istringstream ss(line);
    std::string word;
    int timePointCount = 0;
    while (ss >> word)
    {
        std::istringstream maybenumber(word);
        int number = 0;
        if (maybenumber >> number)
        {
            timePointCount++;
        }
    }
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);
    int vertexCount = 0;
    while (inputFile >> line)
    {
        vertexCount++;
    }
    vertexCount = vertexCount/timePointCount;   //messy, should be a better way

    data.resize( vertexCount, timePointCount );

    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);
    double potential;
    int row = 0; int col = 0;
    while (inputFile >> potential)
    {
        data(row, col) = potential;
        col++;
        if(col == timePointCount)
        {
            col = 0;
            row++;
        }
    }
    inputFile.close();
}


}
#endif // READDATAFOMTEXTFILE_H
