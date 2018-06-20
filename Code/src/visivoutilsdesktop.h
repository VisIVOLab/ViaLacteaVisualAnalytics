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

#ifndef VISIVOUTILSDESKTOP_H
#define VISIVOUTILSDESKTOP_H

/**
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
        @author Ugo Becciani <ugo.becciani@oact.inaf.it>
*/

#include <string>
#include <QString>
#include <vector>


double masToRadDesktop(double mas);
void masToRadDesktop(double *mas, int n);
double degToRadDesktop(double deg);
void degToRadDesktop(double *deg, int n);
double hmsToRadDesktop(char *hms);
void hmsToRadDesktop(char **hmsIn, double *radOut, int n);
double dmsToRadDesktop(char *dms);
void dmsToRadDesktop(char **dmsIn, double *radOut, int n);

int iCompareDesktop(std::string str1, std::string str2);

std::string getTempFileNameDesktop(std::string suffix);
std::string getDirDesktop(std::string path);
std::string getExtDesktop(std::string path);
std::string getNameDesktop(std::string path);

std::string trimRightDesktop(const std::string & source, const std::string & t = " ");
std::string trimLeftDesktop(const std::string & source, const std::string & t = " ");
std::string trimDesktop(const std::string & source, const std::string & t = " ");

void findAndReplaceDesktop(std::string &str, char find, char replace);


//int VisIVORemoteLoadToFile(std::string path, std::string tempFileName);
 double doubleSwapDesktop(char *value);
 long double longdoubleSwapDesktop(char *value);
 float floatSwapDesktop(char *value);
 int intSwapDesktop(char *value);
 long int longintSwapDesktop(char *value);
 long long int longlongintSwapDesktop(char *value);
 void sortarrayDesktop(int *vect, int elements);
#endif

