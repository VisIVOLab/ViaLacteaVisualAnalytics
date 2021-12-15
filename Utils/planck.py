#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Nov 10 10:38:47 2021

@author: smordini


function planck,wave,temp
;+
; NAME:
;       PLANCK()   
; PURPOSE: 
;       To calculate the Planck function in units of ergs/cm2/s/A  
;
; CALLING SEQUENCE: 
;       bbflux = PLANCK( wave, temp) 
;
; INPUT PARAMETERS: 
;       WAVE   Scalar or vector giving the wavelength(s) in **Angstroms**
;               at which the Planck function is to be evaluated.
;       TEMP   Scalar giving the temperature of the planck function in degree K
;
; OUTPUT PARAMETERS:
;       BBFLUX - Scalar or vector giving the blackbody flux (i.e. !pi*Intensity)
;               in erg/cm^2/s/A in at the specified wavelength points.
;
; EXAMPLES:
;       To calculate the blackbody flux at 30,000 K every 100 Angstroms between
;       2000A and 2900 A
;   
;       IDL> wave = 2000 + findgen(10)*100
;       IDL> bbflux = planck(wave,30000)
;
;       If a star with a blackbody spectrum has a radius R, and distance,d, then
;       the flux at Earth in erg/cm^2/s/A will be bbflux*R^2/d^2
; PROCEDURE:
;       The wavelength data are converted to cm, and the Planck function
;       is calculated for each wavelength point. See Allen (1973), Astrophysical
;       Quantities, section 44 for more information.
;
; NOTES:
;       See the procedure planck_radiance.pro in 
;       ftp://origin.ssec.wisc.edu/pub/paulv/idl/Radiance/planck_radiance.pro
;       for computation of Planck radiance given wavenumber in cm-1 or  
;       wavelength in microns 
; MODIFICATION HISTORY:
;       Adapted from the IUE RDAF               August, 1989
;       Converted to IDL V5.0   W. Landsman   September 1997
;       Improve precision of constants    W. Landsman  January 2002
;-
"""
import math
import numpy as np

def planck(wavelength, temp):
    
    try:
        len(wavelength)
    except:
        wavelength=[wavelength]
    try:
        len(temp)
    except:
        pass
    else:
        print('Only one temperature allowed, not list')
        return
    if len(wavelength)<1:
        print('Enter at least one wavelength in Angstrom.')
        return

    
    wave=[ wl*1e-8 for wl in wavelength]
    c1 =  3.7417749e-5 #2*pi*h*c*c with constatns in cgs units (TO BE CHECKED?)
    c2 =  1.4387687    # h*c/k
    value = [c2/wl/temp for wl in wave]
    # print(value)
    bbflux=[]
    test_precision=math.log(np.finfo(float).max)
    # print(test_precision)
    for val,wl in zip (value, wave):   
        if val<test_precision:
            flux=c1/(wl**5*(math.expm1(val)))
            # print(flux)
            bbflux.append(flux*1e-8)
        else:
            bbflux.append(0)
    if len(bbflux)==1:
        bbflux=bbflux[0]
    
    
    
    return bbflux

