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
#include <cstdlib>
#include <cstring>

#include "color.h"

//----------------------------------------------------------------------------
Color::Color()
//----------------------------------------------------------------------------
{
    m_red = m_green = m_blue = m_hue = m_saturation = m_value =0;
    m_alpha = 0;
}
//----------------------------------------------------------------------------
Color::Color( int r, int g, int b, int a )
//----------------------------------------------------------------------------
{
    setRGB(r,g,b,a);
}


//----------------------------------------------------------------------------
void Color::setRGB( int r, int g, int b, int a )
//----------------------------------------------------------------------------
{
    if( r<0 ) r=0; if( r>255 ) r=255;
    if( g<0 ) g=0; if( g>255 ) g=255;
    if( b<0 ) b=0; if( b>255 ) b=255;

    m_red = r; m_green = g; m_blue =b;
    if(a != -1)
    {
        if( a<0 ) a=0; if( a>255 ) a=255;
        m_alpha = a;
    }
    RGBToHSV();
}

//----------------------------------------------------------------------------
void Color::setHSV(int h, int s, int v)
//----------------------------------------------------------------------------
{
    if( s<0 ) s=0; if( s>255 ) s=255;
    if( v<0 ) v=0; if( v>255 ) v=255;
    while(h<0)   h+= 360;
    while(h>360) h-= 360;

    m_hue = h; m_saturation = s; m_value =v;
    HSVToRGB();
}
//----------------------------------------------------------------------------
void Color::getHSV(int *h, int *s, int *v)
//----------------------------------------------------------------------------
{
    *h = m_hue; *s = m_saturation;  *v =m_value;
}


//----------------------------------------------------------------------------
// rgb,sv in range [0..255], h in range [0..360]
void Color::RGBToHSV(int r, int g, int b, int *h, int *s, int *v)
//----------------------------------------------------------------------------
{
    float max = r;
    if (max < g ) max = g;
    if (max < b ) max = b;

    float min = r;
    if (min > g ) min = g;
    if (min > b ) min = b;

    *h = 0;
    *s = max-min;
    *v = max;

    if(s==0) return;

    float delta = max - min;
    float H;

    if (max == r) H = 0 + (g-b)/delta;
    else if (max == g) H = 2 + (b-r)/delta;
    else if (max == b) H = 4 + (r-g)/delta;

    *h = H * 60;
    if (*h<0) *h+=360;
}
//----------------------------------------------------------------------------
// rgb,sv in range [0..255], h in range [0..360]
void Color::HSVToRGB(int h, int s, int v, int *r, int *g, int *b)
//----------------------------------------------------------------------------
{
    *r = *g = *b = v;
    if (s == 0) return;

    if (h == 360) h=0;

    float H = h / 60.0;  // H is in [0..6)
    float S = s / 255.0; // S is in [0..1]

    int i=0;  // i is the largest integer <= H
    if( H>=1 ) i=1;
    if( H>=2 ) i=2;
    if( H>=3 ) i=3;
    if( H>=4 ) i=4;
    if( H>=5 ) i=5;

    float F = H-i; // f is the fractional part of fh;

    float p,q,t;
    p = v * (1.0 - S         );
    q = v * (1.0 - S * F     );
    t = v * (1.0 - S * (1-F) );

    switch(i)
    {
    case 0: *r=v; *g=t; *b=p ; break;
    case 1: *r=q; *g=v; *b=p ; break;
    case 2: *r=p; *g=v; *b=t ; break;
    case 3: *r=p; *g=q; *b=v ; break;
    case 4: *r=t; *g=p; *b=v ; break;
    case 5: *r=v; *g=p; *b=q ; break;
    }
}
//----------------------------------------------------------------------------
void Color::HSVToRGB()
//----------------------------------------------------------------------------
{
    HSVToRGB( m_hue, m_saturation, m_value, &m_red, &m_green, &m_blue );
}
//----------------------------------------------------------------------------
void Color::RGBToHSV()
//----------------------------------------------------------------------------
{
    RGBToHSV( m_red, m_green, m_blue, &m_hue, &m_saturation, &m_value );
}
