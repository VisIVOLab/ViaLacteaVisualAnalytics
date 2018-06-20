/***************************************************************************
 *   Copyright (C) 2010 by Alessandro Costa                                *
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

#include <QString>
#include "vtkLookupTable.h"
#include "observedobject.h"
#ifndef VISPOINT_H
#define VISPOINT_H
class VSTableDesktop;
class TreeItem;

class VisPoint: public ObservedObject

{
    QString m_X;
    QString m_Y;
    QString m_Z;

    QString m_VectorX;
    QString m_VectorY;
    QString m_VectorZ;

    QString m_LutScalar;
    QString m_RadiusScalar;
    QString m_HeightScalar;

    enum m_GLYPH
    {
        POINTS,
        SPHERES,
        CYLINDERS,
        CONES
    };

    bool m_ShowPoints;
    bool m_ShowVectors;
    bool m_Scale;
    bool m_LogX;
    bool m_LogY;
    bool m_LogZ;
    vtkLookupTable* m_lut;
    VSTableDesktop * m_Table;



public:
    VisPoint();
    VisPoint(VSTableDesktop * table);
    VSTableDesktop* getOrigin();
    void setX(QString value);
    void setY(QString value);
    void setZ(QString value);
    void setVectorX(QString value);
    void setVectorY(QString value);
    void setVectorZ(QString value);
    void setLutScalar(QString value);
    void setRadiusScalar(QString value);
    void setHeightScalar(QString value);

    void setShowPoints(bool value);
    void setShowVectors(bool value);
    void setScale(bool value);
    void setLogX(bool value);
    void setLogY(bool value);
    void setLogZ(bool value);

    QString getX();
    QString getY();
    QString getZ();
    QString getVectorX();
    QString getVectorY();
    QString getVectorZ();
    QString getLutScalar();
    QString getRadiusScalar();
    QString getHeightScalar();
    bool isEnabledShowPoints();
    bool isEnabledShowVectors();
    bool isEnabledScale();
    bool isEnabledLogX();
    bool isEnabledLogY();
    bool isEnabledLogZ();



    vtkLookupTable*getLut();
    void setLut(vtkLookupTable* lut);

};

#endif // VISPOINT_H
