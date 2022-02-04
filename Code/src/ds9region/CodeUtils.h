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
 * @file CodeUtils.h
 * @class CodeUtils
 * @brief Utility functions for programming shortcut tasks
 *
 * Utility functions for programming shortcut tasks
 * @author S. Riggi
 * @date 15/01/2016
 */

#ifndef _CODE_UTILS_h
#define _CODE_UTILS_h 1

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class CodeUtils
{
public:
    /**
     * \brief Delete object pointer collection
     */
    template<class T>
    static void DeletePtrCollection(std::vector<T *> &data)
    {
        for (size_t i = 0; i < data.size(); i++) {
            if (data[i]) {
                delete data[i];
                data[i] = 0;
            }
        }
        data.clear();
    }

    /**
     * \brief Collapse a collection in a string (equivalent of python join)
     */
    template<typename Iter>
    static std::string JoinCollection(Iter begin, Iter end, std::string separator = "")
    {
        std::ostringstream result;
        if (begin != end)
            result << *begin++;
        while (begin != end)
            result << separator << *begin++;
        return result.str();
    }

    /**
     * \brief Join vectors
     */
    template<typename T>
    static std::string JoinVec(const std::vector<T> &data, std::string separator = "")
    {
        return JoinCollection(data.begin(), data.end(), separator);
    }

    /**
     * \brief Find pattern in string
     */
    static bool HasPatternInString(std::string str, std::string pattern)
    {
        std::size_t found = str.find(pattern);
        if (found != std::string::npos)
            return true;
        return false;
    }

    /**
     * \brief Split string on whitespaces
     */
    static std::vector<std::string> SplitStringOnWhitespaces(const std::string &s)
    {
        std::vector<std::string> result;
        std::istringstream iss(s);
        for (std::string s; iss >> s;)
            result.push_back(s);
        return result;
    }

    /**
     * \brief Split string on pattern
     */
    static std::vector<std::string> SplitStringOnPattern(const std::string &s, char delim)
    {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            if (item.length() > 0) {
                result.push_back(item);
            }
        }
        return result;
    }

    /**
     * \brief String find and replace
     */
    static void StringFindAndReplace(std::string &str, const std::string &oldstr,
                                     const std::string &newstr)
    {
        size_t pos = 0;
        while ((pos = str.find(oldstr, pos)) != std::string::npos) {
            str.replace(pos, oldstr.length(), newstr);
            pos += newstr.length();
        }
    }

    /**
     * \brief Remove pattern in string
     */
    static void RemovePatternInString(std::string &str, const std::string pattern)
    {
        StringFindAndReplace(str, pattern, "");
    }
};
#endif
