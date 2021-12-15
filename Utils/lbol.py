#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Nov  9 11:10:34 2021

@author: smordini


"""

import numpy as np
from scipy import interpolate

def lbol(wavelength,flux,dist):
#function lbol,w,f,d

# interpolation between data points is done in logarithmic space to allow 
# straight lines (the SED is like that) in the log-log plot. interpolation 
# on a finer grid is done and then data are transformed back in linear space
# where the trapezoidal interpolation is then done

# w is lambda in um
# f is flux jy
# d is dist in parsec

#convert flux to W/cm2/um

    fw=[1.e-15*f*.2997/(w**2.) for f,w in zip(flux,wavelength)]
    lw=[np.log10(w) for w in wavelength]
    lfw=[np.log10(f) for f in fw]
    #1000 points resampling
    
    lw1=(np.arange(1000)*((max(lw)-min(lw))/1000.))+min(lw)
    interpfunc = interpolate.interp1d(lw,lfw, kind='linear')
    lfw1=interpfunc(lw1)
    w1=[10**l for l in lw1]
    fw1=[10**l for l in lfw1]
    jy=[f/1.e-15/.3*(w**2.) for f,w in zip (fw1,w1)]
#    ;integrate over whole range
    fint=0.
    # for i=0,n_elements(w1)-2 do fint=fint+((fw1(i)+fw1(i+1))*(w1(i+1)-w1(i))/2.):
    for i in range(len(w1)-2):
        fint=fint+((fw1[i]+fw1[(i+1)])*(w1[(i+1)]-w1[(i)])/2.)
    
#    ;fint=int_tabulated(w,fw,/double,/sort)
    # ; integrate longword of 350um
    # ;qc0=where(w ge 350.)
    # ;fc0=int_tabulated(w(qc0),fw(qc0))
    # ; compute lbol
    # l=fint*4.*math.pi*d*3.e18/3.8e26*d*3.e18   ;lsol
    lum=fint*4*np.pi*dist*3.086e18/3.827e26*dist*3.086e18   #lsol
    # ;c0ratio=fc0/fint
    # ;print,'Lsubmm/Lbol = ',c0ratio
    
    return lum


