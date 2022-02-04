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
 * @file DS9RegionParser.cc
 * @class DS9RegionParser
 * @brief Class to parse DS9 region files
 *
 * Class to parse DS9 region files
 * @author S. Riggi
 * @date 27/02/2019
 */

#define DEBUG_LOG(x) std::cout << x << std::endl
#define WARN_LOG(x) std::cout << x << std::endl
#define ERROR_LOG(x) std::cerr << x << std::endl

#include "DS9RegionParser.h"
#include "CodeUtils.h"
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
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

DS9RegionParser::DS9RegionParser() = default;

DS9RegionParser::~DS9RegionParser() = default;

int DS9RegionParser::Parse(std::vector<DS9Region *> &regions, std::string filename,
                           std::string &error)
{
    std::ostringstream oss;

    //## Init data
    regions.clear();

    //## Read region file and get region text lines
    std::vector<std::vector<std::string>> raw_data;
    std::vector<std::vector<std::string>> raw_metadata;
    int wcsType = eUNKNOWN_CS;
    if (Read(raw_data, raw_metadata, wcsType, filename) < 0) {
        oss << "Failed to read region data from file " << filename << "!";
        error = oss.str();
        std::cerr << error << std::endl;
        return -1;
    }

    //## Check lines
    if (raw_data.empty()) {
        oss << "Empty data list read from file " << filename << "!";
        error = oss.str();
        std::cerr << error << std::endl;
        return -1;
    }
    if (!raw_metadata.empty() && raw_metadata.size() != raw_data.size()) {
        oss << "Region metadata read from " << filename << " but differs in size wrt region data!";
        error = oss.str();
        std::cerr << error << std::endl;
        return -1;
    }

    //## Extract regions from parsed lines
    bool parseErr = false;
    for (size_t i = 0; i < raw_data.size(); i++) {
        DS9Region *region = ParseRegion(raw_data[i]);
        if (!region) {
            oss << "Failed to parse region from data line no. " << i + 1
                << ", stop parsing and return error!";
            error = oss.str();
            std::cerr << error << std::endl;
            parseErr = true;
            break;
        }

        // Add WCS info
        region->csType = wcsType;

        // Add metadata info
        if (!raw_metadata[i].empty()) {
            DS9RegionMetaData metadata;
            int status = ParseRegionMetaData(metadata, raw_metadata[i]);
            if (status == 0) {
                region->SetMetaData(metadata);
            } else {
                std::cout << "Failed to parse region metadata from data line no. " << i + 1
                          << ", will not set metadata for this region!" << std::endl;
            }
        } else {
            std::cout << "Empty region raw metadata from data line no. " << i + 1
                      << ", will not set metadata for this region!" << std::endl;
        }

        // Append region to list
        regions.push_back(region);

    } // end loop data

    // Clear data in case of errors
    if (parseErr) {
        CodeUtils::DeletePtrCollection<DS9Region>(regions);
        return -1;
    }

    return 0;
}

DS9Region *DS9RegionParser::ParseRegion(const std::vector<std::string> &fields)
{
    // Check if empty fields
    if (fields.empty()) {
        std::cout << "Empty number of fields given!" << std::endl;
        return nullptr;
    }

    // Detect type of region
    std::string data_str = CodeUtils::JoinVec(fields, " ");

    DS9Region *region = 0;

    if (CodeUtils::HasPatternInString(data_str, "polygon")) {
        region = ParsePolygonRegion(data_str);
    } else if (CodeUtils::HasPatternInString(data_str, "box")) {
        region = ParseBoxRegion(data_str);
    } else if (CodeUtils::HasPatternInString(data_str, "circle")) {
        region = ParseCircleRegion(data_str);
    } else if (CodeUtils::HasPatternInString(data_str, "ellipse")) {
        region = ParseEllipseRegion(data_str);
    } else {
        std::cout << "Unknown/unsupported region type found!" << std::endl;
        return nullptr;
    }

    return region;
}

int DS9RegionParser::ParseRegionMetaData(DS9RegionMetaData &metadata,
                                         const std::vector<std::string> &fields)
{
    // Define search string
    std::string name_delim_start = "text={";
    std::string name_delim_stop = "}";
    std::string tag_delim_start = "tag={";
    std::string tag_delim_stop = "}";
    std::vector<std::string> sourceTypes { "unknown-type", "compact", "point-like", "extended",
                                           "compact-extended" };
    std::vector<std::string> sourceFlags { "unknown-flag", "real", "candidate", "fake" };
    std::vector<std::string> sourceFitQualities { "uq-fit", "hq-fit", "mq-fit", "lq-fit",
                                                  "bad-fit" };

    // Loop over metadata and parse
    for (size_t i = 0; i < fields.size(); i++) {
        std::string data_str = fields[i];

        // Parse region name if present
        size_t first = data_str.find(name_delim_start);
        size_t last = data_str.find(name_delim_stop);
        if (first != std::string::npos && last != std::string::npos) {
            std::string name = data_str.substr(first + name_delim_start.size(),
                                               last - first - name_delim_start.size());
            metadata.sourceName = name;
            metadata.hasSourceName = true;
        }

        // Parse region tag
        first = data_str.find(tag_delim_start);
        last = data_str.find(tag_delim_stop);
        if (first != std::string::npos && last != std::string::npos) {
            std::string tag = data_str.substr(first + tag_delim_start.size(),
                                              last - first - tag_delim_start.size());

            // Parse source type
            for (size_t j = 0; j < sourceTypes.size(); j++) {
                bool hasPattern = CodeUtils::HasPatternInString(tag, sourceTypes[j]);
                if (!hasPattern)
                    continue;
                int sourceType = GetSourceType(sourceTypes[j]);
                metadata.sourceType = sourceType;
                metadata.hasSourceType = true;
            }

            // Parse source flag
            for (size_t j = 0; j < sourceFlags.size(); j++) {
                bool hasPattern = CodeUtils::HasPatternInString(tag, sourceFlags[j]);
                if (!hasPattern)
                    continue;
                int sourceFlag = GetSourceFlag(sourceFlags[j]);
                metadata.sourceFlag = sourceFlag;
                metadata.hasSourceFlag = true;
            }

            // Parse source fit qualities
            for (size_t j = 0; j < sourceFitQualities.size(); j++) {
                bool hasPattern = CodeUtils::HasPatternInString(tag, sourceFitQualities[j]);
                if (!hasPattern)
                    continue;
                int sourceFitQuality = GetSourceFitQuality(sourceFitQualities[j]);
                metadata.sourceFitQuality = sourceFitQuality;
                metadata.hasSourceFitQuality = true;
            }
        } // close if has tag

    } // end loop metadata fields

    return 0;
}

DS9PolygonRegion *DS9RegionParser::ParsePolygonRegion(const std::string &data_str)
{
    // Check string
    if (data_str == "")
        return nullptr;

    // Parse string and get coord data
    bool hasComment = CodeUtils::HasPatternInString(data_str, "#");
    std::string delim_start = "polygon(";
    std::string delim_stop = ")";
    size_t first = data_str.find(delim_start);
    size_t last = data_str.find(delim_stop);
    if (first == std::string::npos) {
        delim_start = "polygon"; // try with another region format (without parenthesis)
        first = data_str.find(delim_start);
        if (first == std::string::npos) {
            std::cout << "Failed to find polygon stirng pattern (this should not occur!)"
                      << std::endl;
            return nullptr;
        }
    }
    if (last == std::string::npos) {
        if (hasComment) {
            delim_stop = "#";
            last = data_str.find(delim_stop);
        } else {
            last = data_str.size();
        }
    }

    std::string parsed_data =
            data_str.substr(first + delim_start.size(), last - first - delim_start.size());

    // Check if data are not given in real digit format (e.g. sexagesimal not supported)
    bool hasColon = CodeUtils::HasPatternInString(parsed_data, ":");
    if (hasColon) {
        std::cerr << "Sexagesimal data format not supported, use degrees!" << std::endl;
        return nullptr;
    }

    // Check if data are separated by ',' or by whitespaces
    bool hasCommaDelimiter = CodeUtils::HasPatternInString(parsed_data, ",");
    std::vector<std::string> coords_str;

    if (hasCommaDelimiter) {
        coords_str = CodeUtils::SplitStringOnPattern(parsed_data, ',');
    } else { // splitting data on whitespaces
        coords_str = CodeUtils::SplitStringOnWhitespaces(parsed_data);
    }

    // Check coords
    if (coords_str.empty() || coords_str.size() % 2 == 1) {
        std::cerr << "Empty or invalid number of coordinates (must be odd!)" << std::endl;
        return nullptr;
    }

    // Extract coordinates and convert to double
    DS9PolygonRegion *region = new DS9PolygonRegion();
    for (size_t i = 0; i < coords_str.size(); i += 2) {
        double x = atof(coords_str[i].c_str());
        double y = atof(coords_str[i + 1].c_str());
        auto pair = std::make_pair(x, y);
        (region->points).push_back(pair);
    }

    return region;
}

DS9BoxRegion *DS9RegionParser::ParseBoxRegion(const std::string &data_str)
{
    // Check string
    if (data_str == "")
        return nullptr;

    // Parse string and get coord data
    bool hasComment = CodeUtils::HasPatternInString(data_str, "#");
    std::string delim_start = "box(";
    std::string delim_stop = ")";
    size_t first = data_str.find(delim_start);
    size_t last = data_str.find(delim_stop);
    if (first == std::string::npos) {
        delim_start = "box"; // try with another region format (without parenthesis)
        first = data_str.find(delim_start);
        if (first == std::string::npos) {
            std::cout << "Failed to find polygon stirng pattern (this should not occur!)"
                      << std::endl;
            return nullptr;
        }
    }
    if (last == std::string::npos) {
        if (hasComment) {
            delim_stop = "#";
            last = data_str.find(delim_stop);
        } else {
            last = data_str.size();
        }
    }

    std::string parsed_data =
            data_str.substr(first + delim_start.size(), last - first - delim_start.size());

    // Check if data are not given in real digit format (e.g. sexagesimal not supported)
    bool hasColon = CodeUtils::HasPatternInString(parsed_data, ":");
    if (hasColon) {
        std::cerr << "Sexagesimal data format not supported, use degrees!" << std::endl;
        return nullptr;
    }

    // Check if data are separated by ',' or by whitespaces
    bool hasCommaDelimiter = CodeUtils::HasPatternInString(parsed_data, ",");
    std::vector<std::string> coords_str;

    if (hasCommaDelimiter) {
        coords_str = CodeUtils::SplitStringOnPattern(parsed_data, ',');
    } else { // splitting data on whitespaces
        coords_str = CodeUtils::SplitStringOnWhitespaces(parsed_data);
    }

    // Check coords
    if (coords_str.empty() || coords_str.size() != 5) {
        std::cerr << "Empty or invalid number of coordinates (must be =5!)" << std::endl;
        return nullptr;
    }

    // Extract coordinates and convert to double
    DS9BoxRegion *region = new DS9BoxRegion();
    double cx = atof(coords_str[0].c_str());
    double cy = atof(coords_str[1].c_str());
    double width = atof(coords_str[2].c_str());
    double height = atof(coords_str[3].c_str());
    double theta = atof(coords_str[4].c_str());
    region->cx = cx;
    region->cy = cy;
    region->width = width;
    region->height = height;
    region->theta = theta;

    return region;
}

DS9CircleRegion *DS9RegionParser::ParseCircleRegion(const std::string &data_str)
{
    // Check string
    if (data_str == "")
        return nullptr;

    // Parse string and get coord data
    bool hasComment = CodeUtils::HasPatternInString(data_str, "#");
    std::string delim_start = "circle(";
    std::string delim_stop = ")";
    size_t first = data_str.find(delim_start);
    size_t last = data_str.find(delim_stop);
    if (first == std::string::npos) {
        delim_start = "circle"; // try with another region format (without parenthesis)
        first = data_str.find(delim_start);
        if (first == std::string::npos) {
            std::cout << "Failed to find polygon stirng pattern (this should not occur!)"
                      << std::endl;
            return nullptr;
        }
    }
    if (last == std::string::npos) {
        if (hasComment) {
            delim_stop = "#";
            last = data_str.find(delim_stop);
        } else {
            last = data_str.size();
        }
    }

    std::string parsed_data =
            data_str.substr(first + delim_start.size(), last - first - delim_start.size());

    // Check if data are not given in real digit format (e.g. sexagesimal not supported)
    bool hasColon = CodeUtils::HasPatternInString(parsed_data, ":");
    if (hasColon) {
        std::cerr << "Sexagesimal data format not supported, use degrees!" << std::endl;
        return nullptr;
    }

    // Check if data are separated by ',' or by whitespaces
    bool hasCommaDelimiter = CodeUtils::HasPatternInString(parsed_data, ",");
    std::vector<std::string> coords_str;

    if (hasCommaDelimiter) {
        coords_str = CodeUtils::SplitStringOnPattern(parsed_data, ',');
    } else { // splitting data on whitespaces
        coords_str = CodeUtils::SplitStringOnWhitespaces(parsed_data);
    }

    // Check coords
    if (coords_str.empty() || coords_str.size() != 3) {
        std::cerr << "Empty or invalid number of coordinates (must be =3!)" << std::endl;
        return nullptr;
    }

    // Check radius units
    // double convFactor = 1; // conversion factor to degrees
    bool hasUnits = CodeUtils::HasPatternInString(coords_str[2], "\"");
    std::string radiusStr = coords_str[2];
    if (hasUnits) {
        // convFactor = 1. / 3600.;
        CodeUtils::RemovePatternInString(radiusStr, "\"");
    }

    // Extract coordinates and convert to double
    DS9CircleRegion *region = new DS9CircleRegion();
    double cx = atof(coords_str[0].c_str());
    double cy = atof(coords_str[1].c_str());
    double r = /*convFactor * */ atof(radiusStr.c_str());
    region->cx = cx;
    region->cy = cy;
    region->r = r;

    return region;
}

DS9EllipseRegion *DS9RegionParser::ParseEllipseRegion(const std::string &data_str)
{
    // Check string
    if (data_str == "")
        return nullptr;

    // Parse string and get coord data
    bool hasComment = CodeUtils::HasPatternInString(data_str, "#");
    std::string delim_start = "ellipse(";
    std::string delim_stop = ")";
    size_t first = data_str.find(delim_start);
    size_t last = data_str.find(delim_stop);
    if (first == std::string::npos) {
        delim_start = "ellipse"; // try with another region format (without parenthesis)
        first = data_str.find(delim_start);
        if (first == std::string::npos) {
            std::cout << "Failed to find polygon stirng pattern (this should not occur!)"
                      << std::endl;
            return nullptr;
        }
    }
    if (last == std::string::npos) {
        if (hasComment) {
            delim_stop = "#";
            last = data_str.find(delim_stop);
        } else {
            last = data_str.size();
        }
    }

    std::string parsed_data =
            data_str.substr(first + delim_start.size(), last - first - delim_start.size());

    // Check if data are not given in real digit format (e.g. sexagesimal not supported)
    bool hasColon = CodeUtils::HasPatternInString(parsed_data, ":");
    if (hasColon) {
        std::cerr << "Sexagesimal data format not supported, use degrees!" << std::endl;
        return nullptr;
    }

    // Check if data are separated by ',' or by whitespaces
    bool hasCommaDelimiter = CodeUtils::HasPatternInString(parsed_data, ",");
    std::vector<std::string> coords_str;

    if (hasCommaDelimiter) {
        coords_str = CodeUtils::SplitStringOnPattern(parsed_data, ',');
    } else { // splitting data on whitespaces
        coords_str = CodeUtils::SplitStringOnWhitespaces(parsed_data);
    }

    // Check coords
    if (coords_str.empty() || coords_str.size() != 5) {
        std::cerr << "Empty or invalid number of coordinates (must be =5!)" << std::endl;
        return nullptr;
    }

    // Check radius units
    // double convFactor = 1; // conversion factor to degrees
    bool hasUnits = CodeUtils::HasPatternInString(coords_str[2], "\"");
    std::string r1Str = coords_str[2];
    std::string r2Str = coords_str[3];
    if (hasUnits) {
        // convFactor = 1. / 3600.;
        CodeUtils::RemovePatternInString(r1Str, "\"");
        CodeUtils::RemovePatternInString(r2Str, "\"");
    }

    // Extract coordinates and convert to double
    DS9EllipseRegion *region = new DS9EllipseRegion();
    double cx = atof(coords_str[0].c_str());
    double cy = atof(coords_str[1].c_str());
    double r1 = /*convFactor * */ atof(r1Str.c_str());
    double r2 = /*convFactor * */ atof(r2Str.c_str());
    double theta = atof(coords_str[4].c_str());
    region->cx = cx;
    region->cy = cy;
    region->a = r1;
    region->b = r2;
    region->theta = theta;

    return region;
}

int DS9RegionParser::Read(std::vector<std::vector<std::string>> &data,
                          std::vector<std::vector<std::string>> &metadata, int &wcsType,
                          std::string filename)
{
    // Open file for reading
    std::ifstream in;
    in.open(filename.c_str());
    if (!in.good()) {
        std::cerr << "Failed to open file " << filename << " for reading!" << std::endl;
        return -1;
    }

    // Parsing file
    std::string parsedline = "";
    int line_counter = 0;
    std::map<std::string, int> wcsTypeMap;
    wcsTypeMap.insert(std::make_pair("image", eIMG_CS));
    wcsTypeMap.insert(std::make_pair("fk5", eJ2000));
    wcsTypeMap.insert(std::make_pair("fk4", eB1950));
    wcsTypeMap.insert(std::make_pair("galactic", eGALACTIC));

    // std::vector<std::string> skipLinePatterns {"global","image","fk5","fk4","galactic"};
    std::vector<std::string> skipLinePatterns { "global", "#" };
    std::vector<std::string> endDataReadPatterns { "#" };

    wcsType = eUNKNOWN_CS;
    bool foundWCS = false;

    while (std::getline(in, parsedline)) {
        line_counter++;

        // Check file
        if (!in.good())
            break;

        // Check first character
        std::string field = "";
        std::istringstream parsedline_stream(parsedline);
        parsedline_stream >> field;
        char first_char = *(field.c_str());
        if (first_char == '#' || first_char == '*' || first_char == '/' || first_char == '\n'
            || first_char == '\t' || first_char == ' ') { // skip line
            continue;
        }

        // Search for pattern and skip if present
        bool skipLine = false;
        for (size_t i = 0; i < skipLinePatterns.size(); i++) {
            bool hasPattern = CodeUtils::HasPatternInString(field, skipLinePatterns[i]);
            if (hasPattern) {
                skipLine = true;
                break;
            }
        }
        if (skipLine) {
            continue;
        }

        // Search for WCS line. If found skip line
        bool wcsLine = false;
        for (auto const &it : wcsTypeMap) {
            bool hasPattern = CodeUtils::HasPatternInString(parsedline, it.first);
            if (hasPattern) {
                wcsType = it.second;
                foundWCS = true;
                wcsLine = true;
            }
        }
        if (wcsLine) {
            continue;
        }

        // Parse data until '#'
        std::istringstream ss(parsedline);
        std::vector<std::string> fields;
        std::vector<std::string> fields_metadata;
        bool readMetadata = false;

        while (ss >> field) {
            // Reached end data?
            char first_char = *(field.c_str());
            if (first_char == '#') {
                readMetadata = true;
                break;
            }
            if (first_char == '\n') {
                break;
            }

            // Save data
            fields.push_back(field);

        } // end loop data

        // Parse metadata
        if (readMetadata) {
            while (ss >> field) {
                // Reached end line
                char first_char = *(field.c_str());
                if (first_char == '*' || first_char == '/' || first_char == '\n'
                    || first_char == '\t' || first_char == ' ') {
                    break;
                }

                // Save data
                fields_metadata.push_back(field);

            } // end loop data
        } // close if

        // Append data
        data.push_back(fields);
        metadata.push_back(fields_metadata);

        // Exit at the end
        if (!in.good())
            break;

    } // close while

    // Close file
    in.close();

    // Check if WCS type was found
    if (!foundWCS) {
        std::cerr << "Cannot find and parse WCS type in region file (check if given WCS is "
                     "supported)!"
                  << std::endl;
        return -1;
    }

    return 0;
}
