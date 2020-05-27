/***************************************************************************
 *   Copyright (C) 2010 by Ugo Becciani, Alessandro Costa                  *
 *   ugo.becciani@oact.inaf.it                                             *
 *   alessandro.costa@oact.inaf.it                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef VSTABLEDESKTOP_H
#define VSTABLEDESKTOP_H

/**
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
        @author Ugo Becciani <ugo.becciani@oact.inaf.it>
*/

#include "vsobjectdesktop.h"
#include <string>
#include <vector>
#include <QWidget>
#include <QProcess>
#include <QHash>
/*
extern "C" {
    #include "visivo.h"
}
*/

class VSTableDesktop : public VSObjectDesktop, QWidget
{
public:
    bool readHeader(); //! fuction that read  the header table (filling the above values)
private:
    std::string m_locator;    //! table path
    std::string m_endiannes;  //! endianism
    std::string m_type;       //! data format (float, double ecc..)

    static const unsigned long long int MAX_NUMBER_TO_SKIP; //! maximum number to skip in a single skip
    static const unsigned long long int MAX_NUMBER_ROW_REQUEST; //! maximum number of rows for each request
    static const unsigned long long int MAX_NUMBER_OF_BYTES; //! maximum number of bytes for each request

    unsigned int m_nCols;  //! number of columns
    unsigned long long int m_nRows; //! number of Rows
    bool m_tableExist; //! flag that is true if tabnle already exist (written on disk)

    bool m_isVolume;  //! true if table represent a volumetrivìc data
    unsigned int m_cellNumber[3];  //! number of cell of the volume (mesh)
    float m_cellSize[3]; //!cell geometry


    std::vector<std::string> m_colVector; //! vector containing columns names std::vector

    int m_binNumber; //! Each table has its bin number.
    int ** m_histogramArray;
    float ** m_rangeArray;
    float ** m_histogramValueArray;

    std::string **m_tableData;
    //std::vector< std::vector<std::string> > m_tableData;

    QHash <QString, int> n500;
    QHash <QString, int> n350;
    QHash <QString, int> n250;
    QHash <QString, int> n160;
    QHash <QString, int> n70;
    int m_vialactea_wavelen;
/*
    VBT myVBT;
*/
public:
    VSTableDesktop();
    VSTableDesktop(std::string locator, std::string name = "", std::string description = "", bool statistic=true);
    int getColumnString(unsigned int *colList, unsigned int nOfCol, unsigned long long int fromRow, unsigned long long int toRow, std::string **matrix);
    void setNumberOfBins(int b){m_binNumber=b;}
    ~VSTableDesktop();

    void printSelf();  //!  write in stdlog the header of the table

    std::string getLocator() {return m_locator;}  //! return the path

    unsigned int getNumberOfColumns() { return m_nCols;}
    unsigned long long int getNumberOfRows() { return m_nRows;}
    QString histogramFilePath;
    void setIsVolume(bool isVolume) { m_isVolume = isVolume; }
    bool getIsVolume() { return m_isVolume; }
    const unsigned int* getCellNumber() {return m_cellNumber;}
    const float* getCellSize() {return m_cellSize;}

    std::string getEndiannes() { return m_endiannes;}
    std::string getType() { return m_type;}

    bool setEndiannes(std::string endiannes);
    bool setType(std::string type);
    void setLocator(std::string locator);  //! set the table path (used in writeheader and readheader)
    void setNumberOfRows(unsigned long long int nRows) {m_nRows = nRows;}
    void setNumberOfColumns( int nCols) {m_nCols = nCols;}
    void setColsNames(std::vector<std::string> colName) {m_colVector =colName ;}
    void setTableData(std::string **tableData) {m_tableData =tableData ;}
    void setWavelength(int wavelen) {m_vialactea_wavelen =wavelen ;}
    int getWavelength() {return m_vialactea_wavelen;}
    //void setTableData( std::vector< std::vector<std::string> > tableData) {m_tableData =tableData ;}


    std::string** getTableData() {return m_tableData;}
   // std::vector< std::vector<std::string> > getTableData() {return m_tableData;}
    void setCellNumber(unsigned int xCellNumber, unsigned int yCellNumber, unsigned int zCellNumber);
    void setCellSize(float xCellSize, float yCellSize, float zCellSize);
    //void SetNumberOfCols(unsigned int nCols) {m_nCols = nCols;};
    bool addCol(std::string name); //! add an element (a column name) to the vector m_colVector


    void setColName(unsigned int i, std::string newColName); //! change the column name in the i position. The first column is 0.
    std::string getColName(unsigned int i); //! return the column name in the i position

    int getColId(std::string name); //! return the position of the column with specified name. The first column is 0.

    bool writeHeader(); //! write a new table header with the given member variables

    int getColumn(unsigned int *colList, unsigned int nOfCol, unsigned long long int fromRow, unsigned long long int toRow, float **fArray); //! colList is a vector of nOfCol Elements, listing the columns ID I want to read. fArray is a 2D allocated array fArray[nOfCol,(toRow-fromRow+1)]: the method fill fArray with the column listed in colList (first column is 0: colList contains data in the range  0 - m_nCols -1) and with the postion specified in fromRow toRow. fromRow starts from 0.
    int getColumnList(unsigned int *colList, unsigned  int nOfCol, unsigned long long int *list,unsigned long long int nOfEle, float **fArray);//! colList is a vector of nOfCol Elements, listing the columns ID I want to read. First column is 0. list is a vectorn of nOfEle elements I want to read from the table. fArray is a 2D allocated array fArray[nOfCol,nOfEle]: the method fill fArray with the column listed in colList (first column is 0: colList contains data in the range  0 - m_nCols -1) and with elements listed in list
    int putColumn(unsigned int *colList, unsigned int nOfCol, unsigned long long int fromRow, unsigned long long int toRow, float **fArray);  //!  put data (see getColumn)
    int putColumnList(unsigned int *colList,unsigned int nOfCol, unsigned long long int *list, unsigned long long int nOfEle, float **fArray);//! put data (see getColumnList)

    bool writeTable(float **fArray);  //!write header and table (fArray must contain all data table)
    bool createTable(float **fArray); //!write data (header already exist) in the table
    bool writeTable();  //!write only header
    bool createTable(); //!write a zero filled table
    bool tableExist() { return m_tableExist;} //! table already exist
    bool checkTable() { return true;} //! Integrity check --- to be implemented!


    void setHistogramArray(int ** h){m_histogramArray=h;}
    void setRangeArray(float ** r){ m_rangeArray=r;}
    void setHistogramValueArray(float ** h){m_histogramValueArray=h;}


    void getRange(unsigned int i, float *array);
    //void getHistogram(unsigned int i, int *histogram);
    QVector<double> getHistogram(unsigned int i);
   // void getHistogramValue(unsigned int i, float *histogram);
    QVector<double> getHistogramValue(unsigned int i);
    int getColumnStringToFloat(unsigned int *colList, unsigned int nOfCol, unsigned long long int fromRow, unsigned long long int toRow, float **fArray);


    bool readStatistics(); //! fuction that read  the  table statistic
    int getbinNumber();

    QHash <QString, int> getBandMerged500();
    QHash <QString, int> getBandMerged350();
    QHash <QString, int> getBandMerged250();
    QHash <QString, int> getBandMerged160();
    QHash <QString, int> getBandMerged70();

    void setBandMerged500(QHash <QString, int> bm);
    void setBandMerged350(QHash <QString, int> bm);
    void setBandMerged250(QHash <QString, int> bm);
    void setBandMerged160(QHash <QString, int> bm);
    void setBandMerged70(QHash <QString, int> bm);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif
