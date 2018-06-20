/***************************************************************************
 *   Copyright (C) 2008 by Gabriella Caniglia *
 *  gabriella.caniglia@oact.inaf.it *
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

#ifndef LUTEDITOR_H
#define LUTEDITOR_H


class vtkLookupTable;
class QString;
class Color;

void SelectLookTable(  QString palette,vtkLookupTable *lut=NULL);

void lutVolRenGreen( vtkLookupTable *lut);
void lutVolRenGlow( vtkLookupTable *lut);
void lutTenStep( vtkLookupTable *lut);
void lutTemperature( vtkLookupTable *lut);
void lutSar( vtkLookupTable *lut);
void lutPhysicsContour( vtkLookupTable *lut);
void lutGlow( vtkLookupTable *lut);
void lutEField( vtkLookupTable *lut);
void lutDefault( vtkLookupTable *lut);
void lutDefaultStep( vtkLookupTable *lut);
void lutGray( vtkLookupTable *lut);
void lutRun1( vtkLookupTable *lut);
void lutRun2( vtkLookupTable *lut);
void lutPureRed( vtkLookupTable *lut);
void lutPureGreen( vtkLookupTable *lut);
void lutPureBlue( vtkLookupTable *lut);

void lutAllYellow( vtkLookupTable *lut);
void lutAllCyane( vtkLookupTable *lut);
void lutAllViolet( vtkLookupTable *lut);
void lutAllWhite( vtkLookupTable *lut);
void lutAllBlack( vtkLookupTable *lut);
void lutAllRed( vtkLookupTable *lut);
void lutAllGreen( vtkLookupTable *lut);
void lutAllBlu( vtkLookupTable *lut);

void lutMinMax( vtkLookupTable *lut);
void lutVolRenRGB( vtkLookupTable *lut);
void lutVolRenTwoLev( vtkLookupTable *lut);

#endif


