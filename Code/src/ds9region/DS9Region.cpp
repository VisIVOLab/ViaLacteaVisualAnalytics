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
 * @file DS9Region.cc
 * @class DS9Region
 * @brief DS9 region class
 *
 * DS9 region class
 * @author S. Riggi
 * @date 27/02/2019
 */

#include "DS9Region.h"

#include "Consts.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

//===========================
//==    DS9REGION CLASS
//===========================
DS9Region::DS9Region()
{
    shapeType = eUNKNOWN_SHAPE;
    csType = eUNKNOWN_CS;
    hasMetaDataSet = false;
    x0 = 0;
    y0 = 0;
    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;
}

DS9Region::DS9Region(int shape, int cs) : shapeType(shape), csType(cs)
{
    hasMetaDataSet = false;
    x0 = 0;
    y0 = 0;
    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;
}

DS9Region::~DS9Region() = default;

//================================
//==    DS9 POLYGON REGION CLASS
//================================
DS9PolygonRegion::DS9PolygonRegion(int cs) : DS9Region(ePOLYGON_SHAPE, cs) { }

DS9PolygonRegion::~DS9PolygonRegion() = default;

void DS9PolygonRegion::Print()
{
    std::cout << std::setprecision(12) << "POLYGON REGION: {";
    for (size_t i = 0; i < points.size() - 1; i++)
        std::cout << "(" << points[i].first << "," << points[i].second << "), ";
    std::cout << "(" << points[points.size() - 1].first << "," << points[points.size() - 1].second
              << ")}" << std::endl;
}

//================================
//==    DS9 BOX REGION CLASS
//================================
DS9BoxRegion::DS9BoxRegion(int cs) : DS9Region(eBOX_SHAPE, cs)
{
    cx = 0;
    cy = 0;
    height = 0;
    width = 0;
    theta = 0;
    xmin = 0;
    xmax = 0;
    ymin = 0;
    ymax = 0;
}

DS9BoxRegion::~DS9BoxRegion() = default;

void DS9BoxRegion::Print()
{
    std::cout << std::setprecision(12) << "BOX REGION: "
              << " x[" << xmin << "," << xmax << "], y[" << ymin << "," << ymax << "]" << std::endl;
}

//================================
//==    DS9 CIRCLE REGION CLASS
//================================
DS9CircleRegion::DS9CircleRegion(int cs) : DS9Region(eCIRCLE_SHAPE, cs)
{
    cx = 0;
    cy = 0;
    r = 0;
}

DS9CircleRegion::~DS9CircleRegion() = default;

void DS9CircleRegion::Print()
{
    std::cout << std::setprecision(12) << "CIRCLE REGION: C(" << cx << "," << cy << "), R=" << r
              << std::endl;
}

//================================
//==    DS9 ELLIPSE REGION CLASS
//================================
DS9EllipseRegion::DS9EllipseRegion(int cs) : DS9Region(eELLIPSE_SHAPE, cs)
{
    cx = 0;
    cy = 0;
    a = 0;
    b = 0;
    theta = 0;
}

DS9EllipseRegion::~DS9EllipseRegion() = default;

void DS9EllipseRegion::Print()
{
    std::cout << std::setprecision(12) << "ELLIPSE REGION: C(" << cx << "," << cy << "), a=" << a
              << ", b=" << b << ", theta=" << theta << std::endl;
}
