#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Dec 14 13:41:47 2021

@author: smordini

main file for sed fitting

"""

import sys
import sedfitgrid_engine_thick_vialactea as sedfit_thick
import sedfitgrid_engine_thin_vialactea as sedfit_thin


try:
    import numpy as np
except:
    print("ModuleNotFoundError: missing Numpy package")

try:
    from scipy.interpolate import interp1d
except:
    print("ModuleNotFoundError: missing Scipy package")


val = sys.argv[1]
val = val.lower()


if "thin" in val:
    wavelength = eval(sys.argv[2])
    flux = eval(sys.argv[3])
    massrange = eval(sys.argv[4])
    temprange = eval(sys.argv[5])
    betarange = eval(sys.argv[6])
    dist = eval(sys.argv[7])
    sref = eval(sys.argv[8])
    errorbars = eval(sys.argv[9])
    ulimit = eval(sys.argv[10])
    printfile = sys.argv[11]

    wl, flusso = sedfit_thin.sedfitgrid_engine_thin_vialactea(
        wavelength,
        flux,
        massrange,
        temprange,
        betarange,
        dist,
        sref,
        errorbars=errorbars,
        ulimit=ulimit,
        printfile=printfile,
    )
elif "thick" in val:
    wavelength = eval(sys.argv[2])
    flux = eval(sys.argv[3])
    sizz = eval(sys.argv[4])
    temprange = eval(sys.argv[5])
    betarange = eval(sys.argv[6])
    l0range = eval(sys.argv[7])
    sfactrange = eval(sys.argv[8])
    dist = eval(sys.argv[9])
    sref = eval(sys.argv[10])
    errorbars = eval(sys.argv[11])
    ulimit = eval(sys.argv[12])
    printfile = sys.argv[13]
    wl, flusso = sedfit_thick.sedfitgrid_engine_thick_vialactea(
        wavelength,
        flux,
        sizz,
        temprange,
        betarange,
        l0range,
        sfactrange,
        dist,
        sref,
        errorbars=errorbars,
        ulimit=ulimit,
        printfile=printfile,
    )

else:
    print("No valid sed fit option selected")
