#ifndef VIALACTEASOURCE_H
#define VIALACTEASOURCE_H

#include <iostream>
#include <string>
#include <vector>

class VialacteaSource
{
public:
    VialacteaSource(std::string name);
    int readHeader();
    int readData();
    int getNumberOfColumns(){return m_nCols;}
    int getNumberOfRows(){return m_nRows;}
    int getNumberOfBins(){return m_binNumber;}
    void computeHistogram();

    int ** getHistogramArray(){return m_histogramArray;}
    float ** getRangeArray(){return m_rangeArray;}
    float ** getHistogramValueArray(){return m_histogramValueArray;}

    std::string** getData(){return matrix;}
    //  std::vector< std::vector<std::string> > getData(){return matrix;}

    std::vector<std::string> getColumnsNames(){return m_fieldNames;}


private:
    std::string m_pointsFileName;
    std::string m_pointsBinaryName;
    double m_cellSize[3], m_cellComp[3];
    std::string m_volumeOrTable;
    std::string **matrix;
    //std::vector< std::vector<std::string> > matrix; // default construction

    std::vector<std::string> m_fieldNames;  //!column List
    int m_nCols;
    unsigned long long int m_nRows;
    int m_binNumber; //! Each table has its bin number.
    std::string MISSING_VALUE; //! a negative value used in case of missing data
    int ** m_histogramArray;
    float ** m_rangeArray;
    float ** m_histogramValueArray;

};

#endif // VIALACTEASOURCE_H
