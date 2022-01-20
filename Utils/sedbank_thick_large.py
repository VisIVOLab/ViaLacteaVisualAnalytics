#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Nov 10 10:39:49 2021

@author: smordini

pro sedbank_thick_large,lam,lmin,lmax,nl,tmin,tmax,nt,bmin,bmax,nb,smin,smax,ns,lam0,temp,beta,siz,bank
;Written by Davide Elia, 2012-2015
; lam=lambda in um
"""
import numpy as np
import planck

def sedbank_thick_large(wavelength,lmin,lmax,nl,tmin,tmax,nt,bmin,bmax,nb,smin,smax,ns):

    if nl>1:
        lam0=np.linspace(lmin,lmax,nl)

    elif lmin!=lmax:
            lam0=[lmin, lmax]
    else:
        lam0=lmin

    if nt>1:
        temp=np.linspace(tmin,tmax,nt)
    else:
        temp=[tmin]

    if nb>1:
        beta=np.linspace(bmin,bmax,nb)
    else:
        beta=bmax
    try:
        len(beta)
    except:
        beta=[beta]
    if ns>1:
        psize=np.linspae(smin,smax,ns)
    else:
        psize=[smin]

    lam_a=[l*1e4 for l in wavelength]  #lamnda in Angstrom
    size=[(ps/206265.)**2.*np.pi/4. for ps in psize]



    nlam=len(wavelength)
    bank=np.zeros((nl,nt,nb,ns,nlam))


    conv=[1/2.99792458e-13*wl**2 for wl in wavelength]
    if nb>1 and ns>1:
        for il in range(nl):
            for it in range(nt):
                for ibb in range(nb):
                    for iss in range(ns):
                        for wl in range(nlam):
                            tau=(lam0[il]/wavelength[wl])**beta[ibb]
                            bank[il,it,ibb,iss,wl]=psize[iss]*planck.planck(lam_a[wl],temp[it])/np.pi*conv[wl]*(1- np.exp(-tau))
    elif nb==1 and ns>1:
        for il in range(nl):
            for it in range(nt):
                for iss in range(ns):
                    for wl in range(nlam):
                        tau=(lam0[il]/wavelength[wl])**beta[0]
                        bank[il,it,0,iss,wl]=psize[iss]*planck.planck(lam_a[wl],temp[it])/np.pi*conv[wl]*(1- np.exp(-tau))
    elif nb>1 and ns==1:
        for il in range(nl):
            for it in range(nt):
                for ibb in range(nb):
                        for wl in range(nlam):
                            tau=(lam0[il]/wavelength[wl])**beta[ibb]
                            bank[il,it,ibb,0,wl]=size[0]*planck.planck(lam_a[wl],temp[it])/np.pi*conv[wl]*(1- np.exp2(-tau))
    else:
        for il in range(nl):
            for it in range(nt):
                for wl in range(nlam):
                    tau=(lam0[il]/wavelength[wl])**beta[0]
                    bank[il,it,0,0,wl]=size[0]*planck.planck(lam_a[wl],temp[it])/np.pi*conv[wl]*(1- np.exp(-tau))

    return lam0,temp,beta,size,bank


wavelength=[500,350,22,12] 
lmin=10
lmax=300
nl=100
tmin=7
tmax=40
nt=50
bmin=2
bmax=2
nb=1
smin=1
smax=1
ns=1
t_lam0,t_temp,t_beta,size,t_bank=sedbank_thick_large(wavelength,lmin,lmax,nl,tmin,tmax,nt,bmin,bmax,nb,smin,smax,ns)


