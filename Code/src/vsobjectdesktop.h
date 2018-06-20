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
#ifndef VSOBJECTDESKTOP_H
#define VSOBJECTDESKTOP_H

/**
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
        @author Ugo Becciani <ugo.becciani@oact.inaf.it>
*/

#include <string>
#include <QString>

class VSObjectDesktop
{
    std::string m_description;
    std::string m_name;

public:
    VSObjectDesktop();
    VSObjectDesktop(std::string name, std::string description = "");

    ~VSObjectDesktop();
    virtual void printSelf();
    std::string getDescription() {return m_description;};
    std::string getName()        {return m_name;};

    void setDescription(std::string description) {m_description = description;};
    void setName(std::string name) {m_name = name;};
};

#endif
