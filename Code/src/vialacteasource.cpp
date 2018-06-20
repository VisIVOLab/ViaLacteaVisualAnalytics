/***************************************************************************
 *   Copyright (C) 2014 by Fabio Vitello *
 *  fabio.vitello@oact.inaf.it *
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
#include <cstdlib>
#include <cstring>


#include "vialacteasource.h"

#include "visivoutilsdesktop.h"
#include "base64.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <QDebug>
#include <boost/array.hpp>



VialacteaSource::VialacteaSource(std::string name)
{
    m_pointsFileName=name;
    readHeader();
    readData();
}

//---------------------------------------------------------------------
int VialacteaSource::readData( )
//---------------------------------------------------------------------
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //	in case of missing data we put 0 as default value
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int i = 0;
    int j = 0;

    MISSING_VALUE="missing";

    std::string::size_type index = std::string::npos;
    std::string fields;

    std::vector<std::string> lineData; //!will contain each row of file

    std::ifstream inFile;

    unsigned long long int  sum=0;
    std::ios::off_type pos=0;

    inFile.open(m_pointsFileName.c_str());

    if(!inFile)
        return 1;


    std::string tmp = "";
    bool discardLine=true;

    getline(inFile, tmp); //!read first line
    tmp = trimDesktop(tmp);

    int indexp = tmp.find('#');

    while(indexp == 0)
    {
        getline(inFile, tmp);
        indexp = tmp.find('#');
    }

    findAndReplaceDesktop(tmp, '\t', ' ');
    findAndReplaceDesktop(tmp, '#', ' ');
    tmp = trimDesktop(tmp);


    if(tmp.compare(""))
        fields=tmp;

    std::string data;
    std::stringstream ss;
    ss << fields;
    int suffix=1;
    while(!ss.eof()) //!fill m_fieldNames: name of columns
    {
        ss >> data;

        if(data.compare(""))
        {
            for(int k=0;k< m_fieldNames.size();k++)
            {
                if(data==m_fieldNames[k])
                {
                    std::cerr<<"Warning. Duplicate column name "<<data;
                    std::stringstream ksst;
                    ksst<<suffix;
                    data=data+"_"+ksst.str();
                    suffix++;
                    std::cerr<<" is changed with "<<data<<std::endl;
                }
            }
            m_fieldNames.push_back(data.c_str());
            data = "";
        }
    }

    m_nCols=m_fieldNames.size();

    int nLoad=(int)(10000000/m_nCols);  //!nLoad is the number of rows that I will read

    try
    {
        matrix = new std::string*[m_nCols];
    }
    catch (std::bad_alloc e)
    {
        return 1;
    }

    for(i = 0; i < m_nCols; i++)

    {
        try
        {
            matrix[i] = new std::string[m_nRows];
        }
        catch (std::bad_alloc e)
        {
            return 1;
        }
    }

    while(!inFile.eof())  //!read file content
    {
        tmp = "";

        getline(inFile, tmp); //!read row
        findAndReplaceDesktop(tmp, '\t', ' ');
        tmp = trimDesktop(tmp);

        indexp = tmp.find('#');

        if(indexp == 0)
            continue;

        if(tmp.compare(""))
            lineData.push_back(tmp);  //!fill lineData with the row



        if( inFile.eof())  //!arrived to maximum allowed
        {
            for(i = 0; i <lineData.size(); i++)  //!for each vector element
            {
                std::stringstream ss;
                ss << lineData[i];  //!line extraction
                for(j = 0; j < m_nCols; j++)
                {
                    if(ss.eof())
                    {

                        matrix[j][i]=MISSING_VALUE;


                        continue;
                    }

                    std::string data;
                    ss>>data;

                    char *cstr;
                    cstr = new char [data.size()+1];
                    strcpy (cstr, data.c_str());

                    matrix[j][i]= data.c_str();
                    // matrix[j].push_back(data.c_str());

                    if(data.length()==0)
                    {
                        std::cerr<<"WARNIG. Missing value "<<data<<" is found. Replaced with VALUE="<<MISSING_VALUE <<std::endl;
                        matrix[j][i]=MISSING_VALUE;

                    }
                }

            }

            sum=sum+lineData.size();
            lineData.clear();
        }



    }
   /*
    if(matrix)  //! delete matrix
    {
        for(int i = 0; i < m_nCols; i++)
        {
            if(matrix[i])
            {
                delete [] matrix[i];
            }
        }

        delete [] matrix;

    }
*/

    inFile.close();





    //makeHeader(sum,m_pointsBinaryName,m_fieldNames,m_cellSize,m_cellComp,m_volumeOrTable); //!create header file (in utility)

    return 1;
}

void VialacteaSource::computeHistogram()
{
    // Calculating bin number
    m_binNumber=-1;
    if (m_nRows/30 < 1000 ) m_binNumber=(int)m_nRows/30;
    else m_binNumber=1000;


    try{
        m_histogramArray = new int*[m_nCols];
        m_histogramValueArray = new float*[m_nCols];
    }
    catch(std::bad_alloc & e)
    {
        m_histogramArray=NULL;
        m_histogramValueArray=NULL;
    }
    if(m_histogramArray==NULL)
        qDebug() << "int ** m_histogramArray allocation:  out of memory";

    for(int i=0; i<m_nCols; ++i)
    {
        try{
            m_histogramArray[i]=new int[m_binNumber];
            m_histogramValueArray[i]=new float[m_binNumber];
        }
        catch(std::bad_alloc & e)
        {
            m_histogramArray[i]=NULL;
            m_histogramValueArray[i]=NULL;
        }
        if(m_histogramArray[i]==NULL)
            qDebug() << "int * m_histogramArray allocation:  out of memory";
    }
    try{
        m_rangeArray = new float*[m_nCols];
    }
    catch(std::bad_alloc & e)
    {
        m_rangeArray=NULL;
    }
    if(m_rangeArray==NULL)
        qDebug() << "float ** m_rangeArray allocation:  out of memory";
    for(int i=0; i<m_nCols; ++i)
    {
        try{
            m_rangeArray[i]=new float[3];
        }
        catch(std::bad_alloc & e)
        {
            m_rangeArray[i]=NULL;
        }
        if (m_rangeArray[i]==NULL)
            qDebug() << "float * m_rangeArray allocation:  out of memory";
    }

    //da qui
/*    float  binInterval=((float)(maxMaxValue-minMinValue))/(float) m_binNumber;

    counterCols=-1;
    offset=minMinValue;
    if(range) offset=minRange;
    for(iter = colNumberSet.begin(); iter != end; iter++)
    {
        unsigned long long int totEle=nOfEle;
        unsigned long long int fromRow, toRow, startCounter=0;
        unsigned int colList[1];
        colList[0]=*iter;
        counterCols++;
        totEle=nOfEle;
        startCounter=0;
        int histoIndex;
        while(totEle!=0)
        {
            fromRow=startCounter;
            toRow=fromRow+maxEle-1;
            if(toRow>totRows-1)toRow=totRows-1;
            m_tables[0]->getColumn(colList, m_nOfCol, fromRow, toRow, m_fArray);
            for(unsigned int j=0;j<(toRow-fromRow+1);j++)
            {
                if(range && (m_fArray[0][j]>maxRange || m_fArray[0][j]<minRange)) continue;
                if(binInterval==0)
                    histoIndex=0;
                else
                    histoIndex=(int) ((m_fArray[0][j]-offset)/binInterval);
                if(histoIndex>numberOfBin)
                    histoIndex=numberOfBin;
                m_histogram[counterCols][histoIndex]++;
            }
            startCounter=toRow+1;
            totEle=totEle-(toRow-fromRow+1);
        }
        m_histogram[counterCols][numberOfBin-1]=m_histogram[counterCols][numberOfBin-1]+m_histogram[counterCols][numberOfBin];
    }
*/

}

//---------------------------------------------------------------------
int VialacteaSource::readHeader()
//---------------------------------------------------------------------
{

    m_fieldNames.clear();

    int i = 0;
    int rows=-1;

    std::vector<std::string> counting;
    std::string::size_type index = std::string::npos;

    std::ifstream inFile;
    inFile.open(m_pointsFileName.c_str());  if(!inFile) return 1;

    std::string tmp = "";

    while(!inFile.eof())  //! count the number of row: rows variable is the toltal numer (excluding the first one
    {
        getline(inFile, tmp);

        index = tmp.find('#');

        if(index != std::string::npos)
        {
            tmp.erase(index);
            tmp = trimRightDesktop(tmp);
        }

        if(tmp.compare(""))
        {

            counting.push_back(tmp);
            ++rows;
            counting.clear();
        }
    }
    m_nRows=rows;
    //std::clog<<"rowsheader="<<m_nRows<<endl;
    //!tmp now is the line containing the field names

    inFile.close();

    if(rows==0)
        return 1;

    return 0;
}

