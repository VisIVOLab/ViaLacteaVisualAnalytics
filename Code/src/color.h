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

#ifndef COLOR_H
#define COLOR_H

//----------------------------------------------------------------------------
/**
Color is a class representing an RGBA color:

- Provides conversion among RGB and HSV as static member functions.

- Contain both RGBA and HSV representation of a colour.
  RGB,SV in range [0..255], H in range [0..360].
  If updated through the various Set methods, then the two representation
  are kept in sync. This provide a kind of implicit conversion.

- Provide conversion among the usual color format ( rgb on [0..255] ),
  the vtk format ( rgba on [0..1] ) and wxWindows ( wxColour ). These are accepted
  from all the Set,Get and Constructors.

- Provide weighted interpolation among two colors. Both in RGB and HSV space.

- Provide a visual representation of a transparent color.
  Filling the pixels (x,y) of a bitmap with Color::CheckeredColor(color, x,y)
  will draw an image with a checkered pattern.
  The pattern visibility is proportional with the alpha value.
 */
class Color
//----------------------------------------------------------------------------
{
  public:
    /** default ctor*/
    Color();
    /** ctor accepting an integer RGBA */
    Color( int r, int g, int b, int a=0 );
    /** ctor accepting an vtk color */


    /** Set using integer RGBA - components in [0..255] */
    void setRGB( int r, int g, int b, int a=-1 );


    /** Set using integer HSV - H in [0..360] ,S,V in [0..255] */
    void setHSV(int h, int s, int v);
    /** Set returning integer HSV - H in [0..360] ,S,V in [0..255] */
    void getHSV(int *h, int *s, int *v);



  /** Force updating the RGB representation.
    - useful if the HSV member variable are set directly */
    void HSVToRGB();
  /** Force updating the HSV representation.
    - useful if the RGB member variable are set directly */
    void RGBToHSV();

    /** static function - conversion from RGB to HSV, all represented on Integer */
    static void RGBToHSV(int r, int g, int b, int *h, int *s, int *v);
    /** static function - conversion from HSV to RGB, all represented on Integer */
    static void HSVToRGB(int h, int s, int v, int *r, int *g, int *b);


  // member variables
    int m_red,m_green,m_blue; ///<  rgb color representation
    int m_hue,m_saturation,m_value; ///<  hsv color representation
    int m_alpha;         ///<  alpha value
};
#endif
