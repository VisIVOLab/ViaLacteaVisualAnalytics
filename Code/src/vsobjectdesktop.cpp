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

#include <cstdlib>
#include <cstring>
#include "vsobjectdesktop.h"

#include <iostream>

VSObjectDesktop::VSObjectDesktop()
{
    m_description = "";
    m_name = "";
}


VSObjectDesktop::VSObjectDesktop(std::string name, std::string description /*= ""*/)
{
    m_description = description;
    m_name = name;
}


VSObjectDesktop::~VSObjectDesktop()
{
}

void VSObjectDesktop::printSelf()
{
    std::clog << "Name: "        << m_name        << std::endl;
    std::clog << "Description: " << m_description << std::endl;

    return;
}
