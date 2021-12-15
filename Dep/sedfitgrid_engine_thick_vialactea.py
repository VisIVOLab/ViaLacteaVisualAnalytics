#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Nov 10 10:40:17 2021

@author: smordini

pro sedfitgrid_engine_thick_vialactea,lambda,flux,sizz,temprange,betarange,l0range,sfactrange,dist,sref,lambdagb,flussogb,mass,ftemp,fbeta,fl0,sizesec,lum,dmass,dtemp,dlam0,errorbars=errorbars,ulimit=ulimit,silent=silent,printfile=printfile
; version for VIALACTEA, it doesn't use a settings file, but accepts the initial ranges as input variables
;Written by Davide Elia, 2012-2015

; lambda in um, size in arcsec, flux in Jy, size in arcsec, dist in pc
; temprange,betarange,l0range,sfactrange: ; 3-element vectors, cotaining min, max and number of steps
; sref: 2-element vector, containing opacity at lambda-ref (cm^2_g^-1), and lambda_ref itself (micron).
"""


# from datetime import datetime
import numpy as np
# np.set_printoptions(threshold=np.inf)
import sedbank_thick_large as sed_fit
from scipy.interpolate import interp1d
import lbol
import planck
# import matplotlib.pyplot as plt
# import pandas as pd


def sedfitgrid_engine_thick_vialactea(wavelength,flux,sizz,temprange,betarange,l0range,sfactrange,dist,sref,errorbars=0,ulimit=0,silent=0,printfile=0):

    stemp=temprange
    sbeta=betarange
    sl0=l0range
    sfact=sfactrange
    if temprange[2]<5:
        deg1=0
    else:
        deg1=1
    if betarange[2]<5:
        deg2=0 
    else:
        deg2=1

    if sfactrange[2] <5:
        deg3=0
    else:
        deg3=1
    if l0range[2]<5:
        deg4=0
    else:
        deg4=1
    deg=deg1+deg2+deg3+deg4

    pccm=3.086e18
    kref=sref[0]
    lambdaref=sref[1]
    lambdagb=np.arange(5, 2000)

    conv=1/2.99792458e-13*lambdagb**2.


    ssize=[sf*sizz for sf in sfact]

    m1=0
    m2=1
    t1=0
    t2=1
    b1=0
    b2=1
    s1=0
    s2=1
    npos=3
    
    chi0=1e8
    rchi=1e8
    count=0
    lmask=np.zeros(len(wavelength))
    
    try:
        len(flux)
        wf=[]
        for i in range(len(flux)):
            if flux[i]!=0:
                wf.append(i)
        c=0
        for i in wf:
                lmask[i]=1
                c=c+1
        
    except:
        wf=0
        c=0


 
    
    if np.count_nonzero(ulimit)!=0:
        
        for i in range(len(ulimit)):
            if ulimit[i]!=0:
                lmask[i]=0
    
    if silent==0:
        print('------------------------------------------------------')
        print('flux: '+str(flux))


    wavelength0,temp,beta,size,bank=sed_fit.sedbank_thick_large(wavelength,sl0[m1],sl0[m2],l0range[2],stemp[t1],stemp[t2],temprange[2],sbeta[b1],sbeta[b2],betarange[2],ssize[s1],ssize[s2],sfact[2])#ssl0,sstemp,ssbeta,sssize,bank
    sb=bank.shape
    cflux=np.zeros((sb[0]*sb[1]*sb[2]*sb[3],sb[4]))
    mask=np.zeros((sb[0]*sb[1]*sb[2]*sb[3],sb[4]))
    
    
    if np.count_nonzero(errorbars)!=0:
        eflux=np.zeros((sb[0]*sb[1]*sb[2]*sb[3],sb[4]))


    for tt in range(sb[4]):
        cflux[:,tt]=flux[tt]
        mask[:,tt]=lmask[tt]
        if np.count_nonzero(errorbars)!=0:
            eflux[:,tt]=errorbars[tt]
    

    while rchi> 1.01 and count < 10:    
        wavelength0,temp,beta,size,bank=sed_fit.sedbank_thick_large(wavelength,sl0[m1],sl0[m2],l0range[2],stemp[t1],stemp[t2],temprange[2],sbeta[b1],sbeta[b2],betarange[2],ssize[s1],ssize[s2],sfact[2])#ssl0,sstemp,ssbeta,sssize,bank

        sl0=wavelength0
        sbeta=beta
        stemp=temp
        ssize=[206265.*np.sqrt(sss*4./np.pi) for sss in size]
        sb=bank.shape

        ctemp=[250. , 100 ,  50 ,  40 , 30  , 20  , 19  , 18  , 17  , 16  , 15  , 14  , 13  , 12  , 11  , 10  ,  9  ,  8  ,  7   ,  6   ,   5   ]
        c70 = [1.005,0.989,0.982,0.992,1.034,1.224,1.269,1.325,1.396,1.488,1.607,1.768,1.992,2.317,2.816,3.645,5.175,8.497,17.815,58.391,456.837] 
        c100= [1.023,1.007,0.985,0.980,0.982,1.036,1.051,1.069,1.093,1.123,1.162,1.213,1.282,1.377,1.512,1.711,2.024,2.554, 3.552, 5.774, 12.259] 
        c160= [1.062,1.042,1.010,0.995,0.976,0.963,0.964,0.967,0.972,0.979,0.990,1.005,1.028,1.061,1.110,1.184,1.300,1.491, 1.833, 2.528,  4.278]
    
        if 70 in wavelength:
            w70=wavelength.index(70)
            interpfunc=interp1d(ctemp,c70,kind='quadratic')
            ic70=interpfunc(stemp)
            corr70=np.zeros((sb[0],sb[1],sb[2],sb[3]))
            z=0
            for z in range(sb[1]):
                corr70[:,z,:,:]=ic70[z]
            bank[:,:,:,:,w70]=bank[:,:,:,:,w70]*corr70
            # with open('sed_fit_log.txt', 'a') as ext_f:
    
            #     ext_f.write('## ic70 (len'+str(len(ic70))+'):\n'+str(ic70)+'\n')
            #     ext_f.write('## bank array (len'+str(bank.shape)+'):\n'+str(bank[:,:,:,w70])+'\n')
            # ext_f.close()        
   
        if 160 in wavelength:
            w160=wavelength.index(160)
            interpfunc=interp1d(ctemp,c160,kind='quadratic')
            ic160=interpfunc(stemp)
            corr160=np.zeros((sb[0],sb[1],sb[2],sb[3]))
            z=0
            for z in range(sb[1]):
                corr160[:,z,:]=ic160[z]
            bank[:,:,:,:,w160]=bank[:,:,:,:,w160]*corr160

        rbank=bank.reshape((sb[0]*sb[1]*sb[2]*sb[3],sb[4]))
        if np.count_nonzero(errorbars)!=0:
            chi=((rbank*mask-cflux)/(eflux+1e-7))**2
            # with open('sed_fit_log.txt', 'a') as ext_f:
    
            #     ext_f.write('## chi pre sum (len'+str(chi.shape)+'):\n'+str(chi)+'\n')
            # ext_f.close()             
            chi=chi.sum(1)
            # with open('sed_fit_log.txt', 'a') as ext_f:
            #     ext_f.write('## chi post sum (len'+str(chi.shape)+'):\n'+str(chi)+'\n')    
            # ext_f.close() 
        else:
            chi=(rbank*mask-cflux)**2/(rbank*mask+1e-7)
            # with open('sed_fit_log.txt', 'a') as ext_f:
    
            #     ext_f.write('## chi pre sum (len'+str(chi.shape)+'):\n'+str(chi)+'\n')
            # ext_f.close()             
            chi=chi.sum(1)
            # with open('sed_fit_log.txt', 'a') as ext_f:
            #     ext_f.write('## chi post sum (len'+str(chi.shape)+'):\n'+str(chi)+'\n')    
            # ext_f.close()
        redchi=chi/max(1,len(wavelength)-deg-1)
   

        cu=0
        wu=[]
        
        if np.count_nonzero(ulimit)!=0:
            for i in range(len(ulimit)):
                if ulimit[i]!=0:
                    cu=i
                    wu.append(cu)

            if cu>1:
                mdiff=((rbank[:,wu]-cflux[:,wu])/abs(rbank[:,wu]-cflux[:,wu])).sum(0)
            else:
                mdiff=[(rbank[:,wu[0]]-cflux[:,wu[0]])/abs(rbank[:,wu[0]]-cflux[:,wu[0]])]
            wf=[]
            for i in range(len(mdiff)):
                if mdiff[i]<cu:
                    cf=i
                    wf.append(cf)
            mmm=chi[wf].min()
            wmm=chi.index(mmm)
            wm=wf[wmm]
        else:
            mmm=chi.min()
            wt=np.where(chi==mmm)
            wx=[list(t) for t in wt]
            wm=[x[0] for x in wx]
    
        remchi=mmm/max(1, len(wavelength)-deg-1)


        if betarange[2]==1 and sfact[2]==1:
            nw=0
            nt=0
            nm=0
            i=0
            for nw in range(sb[4]):
                for nm in range(sb[0]):
                    for nt in range(sb[1]):
                        if i==wm[0]:
                            wm3=[nm,nt,0,0,nw]
                            i=i+1
                        else:
                            i=i+1
        elif betarange[2]!=1 and sfact[2]==1:
            for nw in range(sb[4]):
                for nb in range(sb[2]):
                    for nm in range(sb[0]):
                        for nt in range(sb[1]):
                            if i==wm[0]:
                                wm3=[nm,nt,nb,0,nw]
                                i=i+1
                            else:
                                i=i+1
        elif betarange[2]==1 and sfact[2]!=1:
            for nw in range(sb[4]):
                for ns in range(sb[3]):
                    for nm in range(sb[0]):
                        for nt in range(sb[1]):
                            if i==wm[0]:
                                wm3=[nm,nt,0,ns,nw]
                                i=i+1
                            else:
                                i=i+1

        else:
            nw=0
            nt=0
            nb=0
            nm=0
            i=0
            for nw in range(sb[4]):
                for ns in range(sb[3]):
                    for nb in range(sb[2]):
                        for nt in range(sb[1]):
                            for nm in range(sb[0]):
                                if i==wm[0]:
                                    wm3=[nm,nt,nb,ns,nw]
                                    i=i+1
                                else:
                                    i=i+1

        m1=max([0,wm3[0]-npos])
        m2=min([sb[0]-1,wm3[0]+npos])
        t1=max([0,wm3[1]-npos])
        t2=min([sb[1]-1,wm3[1]+npos])
        b1=max([0,wm3[2]-npos])
        b2=min([sb[2]-1,wm3[2]+npos])
        s1=max([0,wm3[3]-npos])
        s2=min([sb[3]-1,wm3[3]+npos])

                
        fl0=sl0[wm3[0]]
        ftemp=stemp[wm3[1]]
        fbeta=sbeta[wm3[2]]
        fsize=(ssize[wm3[3]]/206265.)**2*np.pi/4.
        if silent== 0:
            print(fl0,ftemp,fbeta,206265.*np.sqrt(fsize*4./np.pi),mmm)
            print(sl0[m1],sl0[m2],stemp[t1],stemp[t2],sbeta[b1],sbeta[b2],ssize[s1],ssize[s2])
    
        rchi=chi0/mmm
        count=count+1
        if count ==1:
            rchi=1e8
        chi0=mmm

    inc=1
    ccc=0
    wwc=[]
    while ccc==0:
        for i in range(len(redchi)):
            
            if redchi[i]<remchi+inc:
                
                wwc.append(i)
                ccc=i
        inc=inc+1
    if betarange[2]==1:
        nw=0
        nt=0
        nm=0
        i=0
        for nw in range(sb[3]):
            for nt in range(sb[1]):
                for nm in range(sb[0]):
                    if i==wwc[0]:
                        wm4=[nm,nt,0,0,nw]

                        i=i+1
                    else:
                        i=i+1


    dtemp1=abs(ftemp-stemp[wm4[1]])
    dtemp2=abs(ftemp-stemp[wm4[1]])
    dlam01=abs(fl0-sl0[wm4[0]])
    dlam02=abs(fl0-sl0[wm4[0]])
    dbeta1=abs(fbeta-sbeta[wm4[2]])
    dbeta2=abs(fbeta-sbeta[wm4[2]])
    dsize1=abs(fsize-(ssize[wm4[3]]/206265.)**2*np.pi/4.)
    dsize2=abs(fsize-(ssize[wm4[3]]/206265.)**2*np.pi/4.)
    mll=[sl0[wm4[0]],sl0[wm4[0]]]
    mss=[size[wm4[3]]/206265**2*np.pi/4.,ssize[wm4[3]]/206265**2*np.pi/4.]
    mbb=[sbeta[wm4[2]],sbeta[wm4[2]]]
    
    dmm=np.zeros(8)
    for K0 in [0,1]:
        for K1 in [0,1]:
            for K2 in [0,1]:
                dmm[4*K0+2*K1+K2]=(dist*pccm)**2.*mss[K1]/kref*(mll[K0]/lambdaref)**mbb[K2]


    dmass1=abs(ftemp-sbeta[wm4[2]])
    dmass2=abs(ftemp-sbeta[wm4[2]])
    
    
    dlam0=max([dlam01,dlam02])
    dmass=max([dmass1,dmass2])
    dtemp=max([dtemp1,dtemp2])
    
    
    mass=(dist*pccm)**2.*fsize/kref*(fl0/lambdaref)**fbeta
    mass=mass/1.98892e33
    dmass1=abs(mass-max(dmm)/1.98892e33)
    dmass2=abs(mass-min(dmm)/1.98892e33)
    dmass=max([dmass1,dmass2])
    sizesec=206265.*np.sqrt(fsize*4/np.pi)
    if silent==0:
        print('FIT results:  Mass: '+str(mass)+'     Temp: '+str(ftemp)+'     Beta: '+str(fbeta)+'     lambda_0:'+str(fl0)+'     size:'+str(sizesec)+'     iterations:'+str(count))
    
    tau=(fl0/lambdagb)**fbeta
    flussogb=[fsize*planck.planck(lm*1e4,ftemp)/np.pi*cv*(1- np.exp(-t)) for lm,cv,t in zip(lambdagb,conv,tau)]
    
    qd=[]
    for i in range(len(flussogb)):
        if flussogb[i]>1e-8:
            qd.append(i)

    lambdabol=[float(lambdagb[i]) for i in qd]
    flussobol=[float(flussogb[i]) for i in qd]
    lum=lbol.lbol(lambdabol,flussobol,dist)
    
    if printfile!=0:
        file=open(printfile, 'w')
        for uu in range(len(lambdagb)):
            string=str(lambdagb[uu])+','+str(flussogb[uu])+'\n'
            file.write(string)
        file=open(printfile+'.par', 'w')
        string=str(mass)+','+str(dmass)+','+str(ftemp)+','+str(dtemp)+','+str(fbeta)+','+str(fl0)+','+str(dlam0)+','+str(sizesec)+','+str(lum)
        file.write(string)
    return lambdabol,flussobol
