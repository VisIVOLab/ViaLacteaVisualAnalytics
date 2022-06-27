#include "fits_simimg.hpp"

#include <fitsio.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

using namespace std;

string get_errstatus(int status)
{
    char err_text[32]; // CFITSIO doc: max 30 char error text
    fits_get_errstatus(status, err_text);
    return string { err_text };
}

fits_simimg::fits_simimg(string fits_filename) : fptr { nullptr }
{
    int status = 0;

    if (fits_open_image(&fptr, fits_filename.c_str(), READWRITE, &status)) {
        throw std::runtime_error(get_errstatus(status) + " : " + fits_filename);
    }

    // FIXME assert that ctype is parsec -> only then cdelt/dist is valid

    int nfound = 0;
    if (fits_read_keys_dbl(fptr, "CDELT", 1, 2, cdelt_vals, &nfound, &status) || (nfound != 2)) {
        throw std::invalid_argument(string("Error: reading CDELT 1 2 : ") + get_errstatus(status));
    }
}

void fits_simimg::update_card(string keyname, string keyvalue)
{
    int status = 0;
    char card[FLEN_CARD], oldcard[FLEN_CARD], newcard[FLEN_CARD];
    char oldvalue[FLEN_VALUE], comment[FLEN_COMMENT];
    int keytype;

    if (fits_read_card(fptr, keyname.c_str(), card, &status)) {
        throw invalid_argument(string("Keys to update must exist but read card ") + string(card)
                               + " yields error : " + get_errstatus(status));
    }

    strcpy(oldcard, card);

    /* check if this is a protected keyword that must not be changed */
    if (*card && fits_get_keyclass(card) == TYP_STRUC_KEY) {
        throw invalid_argument(string("Protected keyword cannot be modified: ") + string(card));
    } else {
        /* get the comment string */
        if (*card)
            fits_parse_value(card, oldvalue, comment, &status);

        /* construct template for new keyword */
        strcpy(newcard, keyname.c_str()); /* copy keyword name */
        strcat(newcard, " = "); /* '=' value delimiter */
        strcat(newcard, keyvalue.c_str()); /* new value */
        if (*comment) {
            strcat(newcard, " / "); /* comment delimiter */
            strcat(newcard, comment); /* append the comment */
        }

        /* reformat the keyword string to conform to FITS rules */
        fits_parse_template(newcard, card, &keytype, &status);

        if (string(card) == string(oldcard)) {
            throw invalid_argument(string("Card has already the expected value: ") + string(card));
        }

        /* overwrite the keyword with the new value */
        fits_update_card(fptr, keyname.c_str(), card, &status);
    }

    if (status)
        throw runtime_error(string("update_card failed: ") + get_errstatus(status));
}

void fits_simimg::update_header(const double cdelt_deg[2], const double crval_deg[2])
{
    map<string, string> keys_const_value {
        { "CTYPE1", "GLON-CAR" },
        { "CTYPE2", "GLAT-CAR" },
        { "CUNIT1", "deg" },
        { "CUNIT2", "deg" },
    };

    for (map<string, string>::iterator it = keys_const_value.begin(); it != keys_const_value.end();
         ++it)
        update_card(it->first, it->second);

    map<string, string> keys_calc_value {
        { "CRVAL1", to_string(crval_deg[0]) },
        { "CRVAL2", to_string(crval_deg[1]) },
        { "CDELT1", to_string(cdelt_deg[0]) },
        { "CDELT2", to_string(cdelt_deg[1]) },
    };

    for (map<string, string>::iterator it = keys_calc_value.begin(); it != keys_calc_value.end();
         ++it)
        update_card(it->first, it->second);
}
