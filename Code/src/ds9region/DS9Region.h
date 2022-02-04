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
 * @file DS9Region.h
 * @class DS9Region
 * @brief DS9 region class
 *
 * DS9 region class
 * @author S. Riggi
 * @date 27/02/2019
 */

#ifndef _DS9_REGION_h
#define _DS9_REGION_h 1

#include "Consts.h"

// C++ headers
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

//================================
//==    DS9REGION METADATA CLASS
//================================
class DS9RegionMetaData
{
public:
    /**
     * \brief Standard constructor
     */
    DS9RegionMetaData() { Init(); }

    /**
     * \brief Destructor
     */
    virtual ~DS9RegionMetaData() { }

private:
    void Init()
    {
        sourceName = "";
        sourceFitQuality = eUnknownFitQuality;
        sourceType = eUnknownType;
        sourceFlag = eUnknownSourceFlag;
        hasSourceName = false;
        hasSourceType = false;
        hasSourceFitQuality = false;
        hasSourceFlag = false;
    }

public:
    bool hasSourceName;
    std::string sourceName;
    bool hasSourceType;
    int sourceType;
    bool hasSourceFitQuality;
    int sourceFitQuality;
    bool hasSourceFlag;
    int sourceFlag;
}; // close DS9RegionMetaData()

//===========================
//==    DS9REGION CLASS
//===========================
class DS9Region
{
public:
    /**
     * \brief Standard constructor
     */
    DS9Region();

    /**
     * \brief Parametric constructor
     */
    DS9Region(int shape, int cs);

    /**
     * \brief Destructor
     */
    virtual ~DS9Region();

    /**
     * \brief Region shape type
     */
    enum ShapeType {
        eUNKNOWN_SHAPE = 0,
        eCIRCLE_SHAPE = 1,
        eBOX_SHAPE = 2,
        ePOLYGON_SHAPE = 3,
        eELLIPSE_SHAPE = 4
    };

    /**
     * \brief Print region info
     */
    virtual void Print() {};

    /**
     * \brief Set region metadata
     */
    virtual void SetMetaData(const DS9RegionMetaData &m)
    {
        metadata = m;
        hasMetaDataSet = true;
    }

    /**
     * \brief Get region metadata
     */
    DS9RegionMetaData &GetMetaData() { return metadata; }

    /**
     * \brief Has region metadata filled
     */
    bool HasMetaDataSet() { return hasMetaDataSet; }

    int shapeType;
    int csType;

protected:
    DS9RegionMetaData metadata;
    bool hasMetaDataSet;

    double x0;
    double y0;
    double x1;
    double x2;
    double y1;
    double y2;
};

//=================================
//==    DS9 POLYGON REGION CLASS
//=================================
class DS9PolygonRegion : public DS9Region
{
public:
    /**
     * \brief Parametric constructor
     */
    DS9PolygonRegion(int cs = eUNKNOWN_CS);

    /**
     * \brief Destructor
     */
    virtual ~DS9PolygonRegion();

    /**
     * \brief Print region info
     */
    virtual void Print();

    std::vector<std::pair<double, double>> points;
};

//=================================
//==    DS9 BOX REGION CLASS
//=================================
class DS9BoxRegion : public DS9Region
{
public:
    /**
     * \brief Parametric constructor
     */
    DS9BoxRegion(int cs = eUNKNOWN_CS);

    /**
     * \brief Destructor
     */
    virtual ~DS9BoxRegion();

    /**
     * \brief Print region info
     */
    virtual void Print();

    double cx;
    double cy;
    double width;
    double height;
    double theta;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    std::vector<std::pair<double, double>> points;
};

//=================================
//==    DS9 CIRCLE REGION CLASS
//=================================
class DS9CircleRegion : public DS9Region
{
public:
    /**
     * \brief Parametric constructor
     */
    DS9CircleRegion(int cs = eUNKNOWN_CS);

    /**
     * \brief Destructor
     */
    virtual ~DS9CircleRegion();

    /**
     * \brief Print region info
     */
    virtual void Print();

    double cx;
    double cy;
    double r;
};

//=================================
//==    DS9 ELLIPSE REGION CLASS
//=================================
class DS9EllipseRegion : public DS9Region
{
public:
    /**
     * \brief Parametric constructor
     */
    DS9EllipseRegion(int cs = eUNKNOWN_CS);

    /**
     * \brief Destructor
     */
    virtual ~DS9EllipseRegion();

    /**
     * \brief Print region info
     */
    virtual void Print();

    double cx;
    double cy;
    double a; // semi-axis
    double b; // semi-axis
    double theta;
};

#endif
