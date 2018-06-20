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
//#include "VisIVOFiltersConfigure.h"

#define M_PI 3.14159265358979323846f
#define M_PI_2 (M_PI/2.F)
#define DEG_TO_RAD_FACTOR (M_PI/180.F)

#include "visivoutilsdesktop.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <math.h>

#include <cstring>
#include <cstdlib>
#include <time.h>



static size_t writeDataDesktop(void *buffer, size_t size, size_t nMemb, void *userP);

//---------------------------------------------------------------------
double masToRadDesktop(double mas)
//---------------------------------------------------------------------
{
    double seconds = mas/1000;
    double degs = seconds/3600;

    return degs*DEG_TO_RAD_FACTOR;
}

//---------------------------------------------------------------------
void masToRadDesktop(double *mas, int n)
//---------------------------------------------------------------------
{
    int i = 0;

    double seconds = 0;
    double degs = 0;

    for(i = 0; i < n; i++)
    {
        seconds = mas[i]/1000;
        degs = seconds/3600;
        mas[i] = degs * DEG_TO_RAD_FACTOR;
    }

    return;
}


//---------------------------------------------------------------------
double degToRadDesktop(double deg)
//---------------------------------------------------------------------
{
    return deg*DEG_TO_RAD_FACTOR;
}

//---------------------------------------------------------------------
void degToRadDesktop(double *deg, int n)
//---------------------------------------------------------------------
{
    for(int i = 0; i < n; i++)
        deg[i] *= DEG_TO_RAD_FACTOR;

    return;
}

//---------------------------------------------------------------------
double hmsToRadDesktop(char *hms)
//---------------------------------------------------------------------
{
    double hours = 0;
    double minutes = 0;
    double seconds = 0;

    std::stringstream tmpStream;

    tmpStream << hms;

    tmpStream >> hours;
    tmpStream >> minutes;
    tmpStream >> seconds;

    double outcome = hours + (minutes/60) + (seconds/3600);
    outcome = (outcome/24)*360;
    outcome = outcome*DEG_TO_RAD_FACTOR;

    return outcome;
}

//---------------------------------------------------------------------
void hmsToRadDesktop(char **hmsIn, double *radOut, int n)
//---------------------------------------------------------------------
{
    double hours = 0;
    double minutes = 0;
    double seconds = 0;

    for(int i = 0; i < n; i++)
    {
        std::stringstream tmpStream;

        tmpStream << hmsIn[i];

        tmpStream >> hours;
        tmpStream >> minutes;
        tmpStream >> seconds;

        radOut[i] = hours + (minutes/60) + (seconds/3600);
        radOut[i] = (radOut[i]/24)*360;
        radOut[i] = radOut[i]*DEG_TO_RAD_FACTOR;
    }

    return;
}

//---------------------------------------------------------------------
double dmsToRadDesktop(char *dms)
//---------------------------------------------------------------------
{
    double degs = 0;
    double minutes = 0;
    double seconds = 0;

    std::stringstream tmpStream;

    tmpStream << dms;

    tmpStream >> degs;
    tmpStream >> minutes;
    tmpStream >> seconds;

    if(degs < 0)
    {
        minutes = -minutes;
        seconds = -seconds;
    }

    double outcome = degs + (minutes/60) + (seconds/3600);
    outcome = outcome*DEG_TO_RAD_FACTOR;

    return outcome;
}


//---------------------------------------------------------------------
void dmsToRadDesktop(char **dmsIn, double *radOut, int n)
//---------------------------------------------------------------------
{
    double degs = 0;
    double minutes = 0;
    double seconds = 0;

    for(int i = 0; i < n; i++)
    {
        std::stringstream tmpStream;

        tmpStream << dmsIn[i];

        tmpStream >> degs;
        tmpStream >> minutes;
        tmpStream >> seconds;

        if(degs < 0)
        {
            minutes = -minutes;
            seconds = -seconds;
        }

        radOut[i] = degs + (minutes/60) + (seconds/3600);
        radOut[i] = radOut[i]*DEG_TO_RAD_FACTOR;
    }

    return;
}

//---------------------------------------------------------------------
int iCompareDesktop(std::string str1, std::string str2)
//---------------------------------------------------------------------
{
#ifdef WIN32
    return strcmpi(str1.c_str(), str2.c_str());
#else
    return strcasecmp(str1.c_str(), str2.c_str());
#endif
}

//---------------------------------------------------------------------
std::string getTempFileNameDesktop(std::string suffix)
//---------------------------------------------------------------------
{
    std::stringstream tmpFileName;

#ifdef WIN32
    char *tempDir = getenv("TEMP");

    if(!tempDir)
        tempDir = getenv("TMP");

    if(tempDir)
    {
        tmpFileName << tempDir;
        tmpFileName << "\\";
    }
#else

    tmpFileName << "/tmp/";

#endif

    tmpFileName << "VisIVOTmp" << rand() << suffix;

    return tmpFileName.str();
}

//---------------------------------------------------------------------
std::string getDirDesktop(std::string path)
//---------------------------------------------------------------------
{
#ifdef _WIN32
    int idx = path.rfind('\\');
#else
    int idx = path.rfind('/');
#endif
    if(idx >= 0)
        path.erase(idx + 1, path.length() - (idx + 1));
    else
        path = "";

    return path;
}

//---------------------------------------------------------------------
std::string getExtDesktop(std::string path)
//---------------------------------------------------------------------
{
    int idx = path.rfind('.');

    if(idx < 0)
        return "";

    return path.erase(0, idx + 1);
}

//---------------------------------------------------------------------
std::string trimRightDesktop(const std::string & source, const std::string & t /*= " "*/)
//---------------------------------------------------------------------
{
    std::string str = source;
    return str.erase(str.find_last_not_of(t) + 1);
}

//---------------------------------------------------------------------
std::string trimLeftDesktop(const std::string & source, const std::string & t /*= " "*/)
//---------------------------------------------------------------------
{
    std::string str = source;
    return str.erase(0, str.find_first_not_of(t));
}

//---------------------------------------------------------------------
std::string trimDesktop(const std::string & source, const std::string & t /*= " "*/)
//---------------------------------------------------------------------
{
    std::string str = source;
    return trimLeftDesktop(trimRightDesktop(str, t), t);
}

//---------------------------------------------------------------------
void findAndReplaceDesktop(std::string &str, char find, char replace)
//---------------------------------------------------------------------
{
    size_t i;

    for(; (i = str.find(find)) != std::string::npos;)
        str.replace(i, 1, 1, replace);

    return;
}

//---------------------------------------------------------------------
static size_t writeDataDesktop(void *buffer, size_t size, size_t nMemb, void *userP)
//---------------------------------------------------------------------
{
    unsigned int byteToWrite = size*nMemb;

    std::ofstream *out = (std::ofstream *)userP;
    out->write((char *)buffer, byteToWrite);

    return byteToWrite;
}

//---------------------------------------------------------------------
std::string getNameDesktop(std::string path)
//---------------------------------------------------------------------
{
#ifdef _WIN32
    int idx = path.rfind('\\');
#else
    int idx = path.rfind('/');
#endif
    if(idx >= 0)
        path.erase(0, (idx + 1));
    else
        path = "";

    return path;
}
//----------------------------
float floatSwapDesktop(char *value)

//---------------------------
{

    int size =sizeof(float);
    float swapped;
    char *buffer;
    buffer = new char [sizeof(float)];

    for (int i=0; i<size; i++)
        buffer[ i ] = value[ size-1-i ];

    swapped= *( (float *) buffer );
    delete [] buffer;
    return swapped;

}

//----------------------------
double doubleSwapDesktop(char *value)

//---------------------------
{
    int size =sizeof(double);
    char *buffer;
    double swapped;
    buffer = new char[sizeof(double)];

    for (int i=0; i<size; i++)
        buffer[ i ] = value[ size-1-i ];


    swapped= *( (double *) buffer );
    delete [] buffer;
    return swapped;

}
//----------------------------
long double longdoubleSwapDesktop(char *value)

//---------------------------
{
    int size =sizeof(long double);
    char *buffer;
    long double swapped;
    buffer = new char[sizeof(long double)];

    for (int i=0; i<size; i++)
        buffer[ i ] = value[ size-1-i ];


    swapped= *( (long double *) buffer );
    delete [] buffer;
    return swapped;
}
//----------------------------
int intSwapDesktop(char *value)

//---------------------------
{
    int size =sizeof(int);
    char *buffer;
    int swapped;
    buffer = new char[sizeof(int)];

    for (int i=0; i<size; i++)
        buffer[ i ] = value[ size-1-i ];


    swapped= *( (int *) buffer );
    delete [] buffer;
    return swapped;
}
//----------------------------
long int longintSwapDesktop(char *value)

//---------------------------
{
    int size =sizeof(long int);
    char *buffer;
    long int swapped;
    buffer = new char[sizeof(long int)];

    for (int i=0; i<size; i++)
        buffer[ i ] = value[ size-1-i ];


    swapped= *( (long int *) buffer );
    delete [] buffer;
    return swapped;
}
//----------------------------
long long int longlongintSwapDesktop(char *value)

//---------------------------
{
    int size =sizeof(long long int);
    char *buffer;
    long long int swapped;
    buffer = new char[sizeof(long long int)];

    for (int i=0; i<size; i++)
        buffer[ i ] = value[ size-1-i ];


    swapped= *( (long long int *) &buffer );
    delete [] buffer;
    return swapped;
}
//----------------------------

void sortarrayDesktop(int *vect, int elements)

//---------------------------
{
    for(int i=elements-1;i>=0;i=i-1)
    {
        for(int j=1;j<=i;j++)
        {
            if(vect[j-1] > vect[j])
            {
                int temp=vect[j];
                vect[j]=vect[j-1];
                vect[j-1]=temp;

            }

        }
    }
    return;
}
