// ***********************************************************************
// * License and Disclaimer                                              *
// *                                                                     *
// * Copyright 2016 Simone Riggi                                         *
// *                                                                     *
// * This file is part of Caesar.                                        *
// *                                                                     *
// * Caesar is free software: you can redistribute it and/or modify it   *
// * under the terms of the GNU General Public License as published by   *
// * the Free Software Foundation, either * version 3 of the License,    *
// * or (at your option) any later version.                              *
// * Caesar is distributed in the hope that it will be useful, but 	 *
// * WITHOUT ANY WARRANTY; without even the implied warranty of          *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
// * See the GNU General Public License for more details. You should     *
// * have received a copy of the GNU General Public License along with   *
// * Caesar. If not, see http://www.gnu.org/licenses/.                   *
// ***********************************************************************

/**
 * @file Consts.h
 * @class Consts
 * @brief Definitions & constants
 *
 * Definitions & constants
 * @author S. Riggi
 * @date 15/01/2016
 */

#ifndef _CONSTS_h
#define _CONSTS_h 1

#include <string>

//====================================
//===          ENUM              =====
//====================================
enum WCSType {
    eUNKNOWN_CS = -1,
    eJ2000 = 0,
    eB1950 = 1,
    eGALACTIC = 2,
    eECLIPTIC = 3,
    eALTAZ = 4,
    eLINEAR = 5,
    eIMG_CS = 6,
};

/**
 * \brief Source type enumeration
 */
enum SourceType {
    eUnknownType = 0,
    eCompact = 1,
    ePointLike = 2,
    eExtended = 3,
    eCompactPlusExtended = 4,
    eDiffuse = 5
};

/**
 * \brief Source flag enumeration
 */
enum SourceFlag { eUnknownSourceFlag = -1, eReal = 1, eCandidate = 2, eFake = 3 };

/**
 * \brief Source fit quality enumeration
 */
enum SourceFitQuality {
    eUnknownFitQuality = -1, // unknown
    eBadFit = 0, // not converged
    eLQFit = 1, // converged with pars at boundary, error matrix not posdef
    eMQFit = 2, // converged with good error estimates, not passing quality selection cuts
    eHQFit = 3 // passing quality selection
};

/**
 * \brief Convert source flag enumeration to string
 */
inline std::string GetSourceFlagStr(int sourceFlag)
{
    std::string flagStr = "";
    if (sourceFlag == eReal)
        flagStr = "real";
    else if (sourceFlag == eCandidate)
        flagStr = "candidate";
    else if (sourceFlag == eFake)
        flagStr = "fake";
    else if (sourceFlag == eUnknownSourceFlag)
        flagStr = "unknown-flag";
    else
        flagStr = "unknown-flag";
    return flagStr;
}

/**
 * \brief Convert source flag enumeration to sourceness label string
 */
inline std::string GetSourcenessLabel(int sourceFlag)
{
    std::string flagStr = "";
    if (sourceFlag == eReal)
        flagStr = "REAL";
    else if (sourceFlag == eCandidate)
        flagStr = "CANDIDATE";
    else if (sourceFlag == eFake)
        flagStr = "FALSE";
    else if (sourceFlag == eUnknownSourceFlag)
        flagStr = "UNKNOWN";
    else
        flagStr = "UNKNOWN";
    return flagStr;
}

/**
 * \brief Convert source flag enumeration from string
 */
inline int GetSourceFlag(std::string flagStr)
{
    int sourceFlag = eUnknownSourceFlag;
    if (flagStr == "real")
        sourceFlag = eReal;
    else if (flagStr == "candidate")
        sourceFlag = eCandidate;
    else if (flagStr == "fake")
        sourceFlag = eFake;
    else if (flagStr == "unknown-flag")
        sourceFlag = eUnknownSourceFlag;
    else
        sourceFlag = eUnknownSourceFlag;
    return sourceFlag;
}

/**
 * \brief Convert source type enumeration to string
 */
inline std::string GetSourceTypeStr(int sourceType)
{
    std::string typeStr = "";
    if (sourceType == eUnknownType)
        typeStr = "unknown-type";
    else if (sourceType == eCompact)
        typeStr = "compact";
    else if (sourceType == ePointLike)
        typeStr = "point-like";
    else if (sourceType == eExtended)
        typeStr = "extended";
    else if (sourceType == eCompactPlusExtended)
        typeStr = "compact-extended";
    else
        typeStr = "unknown-type";
    return typeStr;
}

/**
 * \brief Convert source type enumeration to morph label string
 */
inline std::string GetSourceMorphLabel(int sourceType)
{
    std::string typeStr = "";
    if (sourceType == eUnknownType)
        typeStr = "UNKNOWN";
    else if (sourceType == eCompact)
        typeStr = "COMPACT";
    else if (sourceType == ePointLike)
        typeStr = "POINT-LIKE";
    else if (sourceType == eExtended)
        typeStr = "EXTENDED";
    else if (sourceType == eCompactPlusExtended)
        typeStr = "COMPACT-EXTENDED";
    else
        typeStr = "UNKNOWN";
    return typeStr;
}

/**
 * \brief Convert source type string to enumeration
 */
inline int GetSourceType(std::string typeStr)
{
    int type = eUnknownType;
    if (typeStr == "" || typeStr == "unknown-type")
        type = eUnknownType;
    else if (typeStr == "compact")
        type = eCompact;
    else if (typeStr == "point-like")
        type = ePointLike;
    else if (typeStr == "extended")
        type = eExtended;
    else if (typeStr == "compact-extended")
        type = eCompactPlusExtended;
    else
        type = eUnknownType;
    return type;
}

/**
 * \brief Convert source fit quality enumeration to string
 */
inline std::string GetSourceFitQualityStr(int fitQuality)
{
    std::string flagStr = "";
    if (fitQuality == eBadFit)
        flagStr = "bad-fit";
    else if (fitQuality == eLQFit)
        flagStr = "lq-fit";
    else if (fitQuality == eMQFit)
        flagStr = "mq-fit";
    else if (fitQuality == eHQFit)
        flagStr = "hq-fit";
    else
        flagStr = "uq-fit"; // unknown
    return flagStr;
}

/**
 * \brief Convert source fit quality enumeration to string (v2)
 */
inline std::string GetSourceFitQualityStr_V2(int fitQuality)
{
    std::string flagStr = "";
    if (fitQuality == eBadFit)
        flagStr = "BAD";
    else if (fitQuality == eLQFit)
        flagStr = "LOW";
    else if (fitQuality == eMQFit)
        flagStr = "MEDIUM";
    else if (fitQuality == eHQFit)
        flagStr = "HIGH";
    else
        flagStr = "UNKNOWN"; // unknown
    return flagStr;
}

/**
 * \brief Convert source fit quality enumeration to string
 */
inline int GetSourceFitQuality(std::string flagStr)
{
    int fitQuality = eUnknownFitQuality;
    if (flagStr == "bad-fit")
        fitQuality = eBadFit;
    else if (flagStr == "lq-fit")
        fitQuality = eLQFit;
    else if (flagStr == "mq-fit")
        fitQuality = eMQFit;
    else if (flagStr == "hq-fit")
        fitQuality = eHQFit;
    else if (flagStr == "uq-fit")
        fitQuality = eUnknownFitQuality;
    else
        fitQuality = eUnknownFitQuality;
    return fitQuality;
}

#endif
