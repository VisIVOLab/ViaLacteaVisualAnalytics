// ***********************************************************************
// * License and Disclaimer                                              *
// *                                                                     *
// * Copyright 2016 Simone Riggi                                         *
// *                                                                     *
// * This file is part of Caesar.                                        *
// * Caesar is free software: you can redistribute it and/or modify it   *
// * under the terms of the GNU General Public License as published by   *
// * the Free Software Foundation, either * version 3 of the License,    *
// * or (at your option) any later version.                              *
// * Caesar is distributed in the hope that it will be useful, but       *
// * WITHOUT ANY WARRANTY; without even the implied warranty of          *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
// * See the GNU General Public License for more details. You should     *
// * have received a copy of the GNU General Public License along with   *
// * Caesar. If not, see http://www.gnu.org/licenses/.                   *
// ***********************************************************************

/**
 * @file DS9RegionParser.h
 * @class DS9RegionParser
 * @brief Class to parse DS9 region files
 *
 * Class to parse DS9 region files
 * @author S. Riggi
 * @date 27/02/2019
 */

#ifndef _DS9_REGION_PARSER_h
#define _DS9_REGION_PARSER_h 1

#include "Consts.h"

// C++ headers
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

class DS9RegionMetaData;
class DS9Region;
class DS9PolygonRegion;
class DS9BoxRegion;
class DS9CircleRegion;
class DS9EllipseRegion;

class DS9RegionParser
{
public:
    /**
     \brief Class constructor: initialize structures.
     */
    DS9RegionParser();

    /**
     * \brief Class destructor: free allocated memory
     */
    virtual ~DS9RegionParser();

    /**
     * \brief Parse DS9 region file and returns list of regions
     */
    static int Parse(std::vector<DS9Region *> &regions, std::string filename, std::string &error);

protected:
    /**
     * \brief Read DS9 region file and returns list of regions
     */
    static int Read(std::vector<std::vector<std::string>> &data,
                    std::vector<std::vector<std::string>> &metadata, int &wcsType,
                    std::string filename);
    /**
     * \brief Parse region from text
     */
    static DS9Region *ParseRegion(const std::vector<std::string> &fields);

    /**
     * \brief Parse region metadata from text
     */
    static int ParseRegionMetaData(DS9RegionMetaData &metadata,
                                   const std::vector<std::string> &fields);
    /**
     * \brief Parse polygon region from text
     */
    static DS9PolygonRegion *ParsePolygonRegion(const std::string &data_str);

    /**
     * \brief Parse box region from text
     */
    static DS9BoxRegion *ParseBoxRegion(const std::string &data_str);

    /**
     * \brief Parse circle region from text
     */
    static DS9CircleRegion *ParseCircleRegion(const std::string &data_str);

    /**
     * \brief Parse ellipse region from text
     */
    static DS9EllipseRegion *ParseEllipseRegion(const std::string &data_str);
};

#endif
