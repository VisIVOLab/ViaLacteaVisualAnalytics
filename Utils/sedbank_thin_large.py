#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Nov 10 10:39:59 2021

@author: smordini
mmin,mmax,nm= minimum mass, maximum mass, mass step
tmin,tmax,nt= minimum temperature, maximum temperature, temperature step
bmin,bmax,nb= minimum beta, maximum beta, beta step
kref= opacity at reference wavelength (cm^2_g^-1)
lambdaref= reference wavelength
"""
import numpy as np
import planck

def sedbank_thin_large(wavelength,mmin,mmax,nm,tmin,tmax,nt,bmin,bmax,nb,kref,lambdaref,dist,m2=0):


    if nm>1:
        if m2!=0:
            nm=int(nm/2)
        mstep=(mmax**0.2-mmin**0.2)/(nm-1)
        amass=mmin**0.2+mstep*np.arange(nm)
        mass=amass**5.
        # mass=np.linspace(mmin,mmax,nm)

    elif mmin!=mmax:
            mass=[mmin, mmax]
    else:
        mass=mmin
    try:
        len(mass)
        massg=[ma*1.98892e33 for ma in mass]
    except:
        massg=[mass*1.98892e33 ]
    distcm=dist*3.08567758128e18
    if nt>1:
        temp=np.linspace(tmin,tmax,nt)
    else:
        temp=tmin
    try:
        len(temp)
    except:
        temp=[temp]
    if nb>1:
        beta=np.linspace(bmin,bmax,nb)
    else:
        beta=bmax
    try:
        len(beta)
    except:
        beta=[beta]



    wl_a=[wave*1e4 for wave in wavelength]
    nlam=len(wavelength)
    bank=np.zeros((nm,nt,nb,nlam))


    conv=[1/2.99792458e-13*wl**2 for wl in wavelength]


    im=0
    it=0
    ibb=0
    wl=0
    if nb>1:
        for im in range(len(mass)):
            for it in range(len(temp)):
                for ibb in range(len(beta)):
                    for wl in range(len(wavelength)):
                        bank[im,it,ibb,wl]=massg[im]*kref/distcm**2*planck.planck(wl_a[wl],temp[it])/np.pi*conv[wl]*((lambdaref/wavelength[wl]))**beta[ibb]
    else:
        for im in range(len(mass)):
            for it in range(len(temp)):
                for wl in range(len(wavelength)):
                    bank[im,it,ibb,wl]=massg[im]*kref/distcm**2*planck.planck(wl_a[wl],temp[it])/np.pi*conv[wl]*((lambdaref/wavelength[wl]))**beta[ibb]
    return mass,temp,beta,bank


mass,temp,beta,bank=sedbank_thin_large([1100,350,250,160,70],10,5000,100,7,40,50,2,2,1,0.1,300,100)

