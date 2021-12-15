#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Nov 10 10:40:49 2021

@author: smordini

# version for VIALACTEA, it doesn't use a settings file, but accepts the initial ranges as input variables
#Written by Davide Elia, 2012-2015

; wavelength in um, flux in Jy, dist in pc
; massrange,temprange,betarange: ; 3-element vectors, cotaining min, max and number of steps
; sref: 2-element vector, containing opacity at lambda-ref (cm^2_g^-1), and lambda_ref itself (micron).


"""
# from datetime import datetime
import numpy as np
# np.set_printoptions(threshold=np.inf)
import sedbank_thin_large as sed_fit
from scipy.interpolate import interp1d
import lbol
import planck
# import math 
# import matplotlib.pyplot as plt
# import pandas as pd


def sedfitgrid_engine_thin_vialactea(wavelength,flux,massrange,temprange,betarange,dist,sref,errorbars=0,ulimit=0,silent=0,printfile=0):
    

    dvirt=1000.

    smass=massrange  
    stemp=temprange
    sbeta=betarange

    if massrange[2] < 5:
        deg1=0
    else:
        deg1=1
    if temprange[2] < 5:
        deg2=0 
    else:
        deg2=1
    if betarange[2] < 5:
        deg3=0
    else:
        deg3=1
    deg=deg1+deg2+deg3

    pccm=3.086e+18
    kref=sref[0]
    lambdaref=sref[1]
    

    lambdagb=np.arange(5, 2000)


    conv=1/2.99792458e-13*lambdagb**2.
    # with open('sed_fit_log.txt', 'a') as ext_f:
    #     ext_f.write('## conv(len='+str(len(conv))+'): \n')
    #     for i in range(len(conv)):
    #         ext_f.write("%10.8E"%(conv[i]))
    #         ext_f.write('\n')
    # ext_f.close()
    distcm=dist*pccm
    # with open('sed_fit_log.txt', 'a') as ext_f:
    #     ext_f.write('## distcm: '+str(distcm)+'\n')
    # ext_f.close()
    m1=0
    m2=1
    t1=0
    t2=1
    b1=0
    b2=1

    npos=3

    chi0=1e8
    rchi=1e8
    count=0

    lmask=np.zeros(len(wavelength))
    try:
        len(flux)
        wf=[]
        i=0
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
        
    # with open('sed_fit_log.txt', 'a') as ext_f:
    #     ext_f.write('## lamsk(len='+str(len(lmask))+'): ')
    #     ext_f.write(str(lmask)+'\n')
    if silent==0:
        print('------------------------------------------------------')
        print('flux: '+str(flux))
    #         ext_f.write('------------------------------------------------------\n')
    #         ext_f.write('flux: '+str(flux)+'\n')
        
        
    # ext_f.close()


    maxmass=smass[m2]


    mass,temp,beta,bank=sed_fit.sedbank_thin_large(wavelength, massrange[m1],massrange[m2],massrange[2],temprange[t1], temprange[t2],temprange[2],betarange[b1],betarange[b2], betarange[2], sref[0], sref[1], dvirt)
    # with open('sed_fit_log.txt', 'a') as ext_f:
    #     ext_f.write('## mass array (len'+str(len(mass))+'):\n'+str(mass)+'\n')
    #     ext_f.write('## temp array (len'+str(len(temp))+'):\n'+str(temp)+'\n')
    #     ext_f.write('## beta array (len'+str(len(beta))+'):\n'+str(beta)+'\n')
    #     ext_f.write('## bank array (len'+str(bank.shape)+'):\n'+str(bank)+'\n')
    # ext_f.close()
    sb=bank.shape
    cflux=np.zeros((sb[0]*sb[1]*sb[2],sb[3]))
    mask=np.zeros((sb[0]*sb[1]*sb[2],sb[3]))

    if np.count_nonzero(errorbars)!=0:
        eflux=np.zeros((sb[0]*sb[1]*sb[2],sb[3]))
        
    
    for tt in range(sb[3]):
        cflux[:,tt]=flux[tt]
        mask[:,tt]=lmask[tt]
        if np.count_nonzero(errorbars)!=0:
            eflux[:,tt]=errorbars[tt]
    # with open('sed_fit_log.txt', 'a') as ext_f:
    #     ext_f.write('## cflux (len'+str(cflux.shape)+'):\n'+str(cflux)+'\n')
    #     ext_f.write('## mask array (len'+str(cflux.shape)+'):\n'+str(mask)+'\n')
    #     ext_f.write('## eflux array (len'+str(cflux.shape)+'):\n'+str(eflux)+'\n')
    # ext_f.close()
    m1=0
    m2=len(mass)-1
    t1=0
    t2=len(temp)-1
    b1=0
    b2=len(beta)-1
    while rchi>1.01 and count<=10:
        mass,temp,beta,bank=sed_fit.sedbank_thin_large(wavelength, mass[m1],mass[m2],massrange[2],temp[t1], temp[t2],temprange[2],beta[b1],beta[b2], betarange[2], sref[0], sref[1], dvirt)
        # with open('sed_fit_log.txt', 'a') as ext_f:
        #     ext_f.write('## mass array (len'+str(len(mass))+'):\n'+str(mass)+'\n')
        #     ext_f.write('## temp array (len'+str(len(temp))+'):\n'+str(temp)+'\n')
        #     ext_f.write('## beta array (len'+str(len(beta))+'):\n'+str(beta)+'\n')
        #     ext_f.write('## bank array (len'+str(bank.shape)+'):\n'+str(bank)+'\n')
        # ext_f.close()        
        smass=mass
        sbeta=beta
        stemp=temp
        sb=bank.shape
        ctemp=[250. , 100 ,  50 ,  40 , 30  , 20  , 19  , 18  , 17  , 16  , 15  , 14  , 13  , 12  , 11  , 10  ,  9  ,  8  ,  7   ,  6   ,   5   ]
        c70 = [1.005,0.989,0.982,0.992,1.034,1.224,1.269,1.325,1.396,1.488,1.607,1.768,1.992,2.317,2.816,3.645,5.175,8.497,17.815,58.391,456.837] 
        # c100= [1.023,1.007,0.985,0.980,0.982,1.036,1.051,1.069,1.093,1.123,1.162,1.213,1.282,1.377,1.512,1.711,2.024,2.554, 3.552, 5.774, 12.259] 
        c160= [1.062,1.042,1.010,0.995,0.976,0.963,0.964,0.967,0.972,0.979,0.990,1.005,1.028,1.061,1.110,1.184,1.300,1.491, 1.833, 2.528,  4.278]

        if 70 in wavelength:
            w70=wavelength.index(70)
            interpfunc=interp1d(ctemp,c70,kind='quadratic')
            ic70=interpfunc(stemp)
            corr70=np.zeros((sb[0],sb[1],sb[2]))
            z=0
            for z in range(sb[1]):
                corr70[:,z,:]=ic70[z]
            bank[:,:,:,w70]=bank[:,:,:,w70]*corr70
            # with open('sed_fit_log.txt', 'a') as ext_f:
    
            #     ext_f.write('## ic70 (len'+str(len(ic70))+'):\n'+str(ic70)+'\n')
            #     ext_f.write('## bank array (len'+str(bank.shape)+'):\n'+str(bank[:,:,:,w70])+'\n')
            # ext_f.close()        
   
        if 160 in wavelength:
            w160=wavelength.index(160)
            interpfunc=interp1d(ctemp,c160,kind='quadratic')
            ic160=interpfunc(stemp)
            corr160=np.zeros((sb[0],sb[1],sb[2]))
            z=0
            for z in range(sb[1]):
                corr160[:,z,:]=ic160[z]
            bank[:,:,:,w160]=bank[:,:,:,w160]*corr160
            # with open('sed_fit_log.txt', 'a') as ext_f:
    
            #     ext_f.write('## ic160 (len'+str(len(ic160))+'):\n'+str(ic70)+'\n')
            #     ext_f.write('## bank array (len'+str(bank.shape)+'):\n'+str(bank[:,:,:,w160])+'\n')
            # ext_f.close()  


        rbank=bank.reshape((sb[0]*sb[1]*sb[2],sb[3]))
        # with open('sed_fit_log.txt', 'a') as ext_f:

        #     ext_f.write('## rbank array (len'+str(rbank.shape)+'):\n'+str(rbank)+'\n')
        # ext_f.close()  

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
            chi=(bank*mask-cflux)**2/(rbank*mask+1e-7)
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
            mmm=chi[wf].min   
            wmm=chi.index(mmm)
            wm=wf[wmm]
        else:
            mmm=chi.min()
            wt=np.where(chi==mmm)
            wx=[list(t) for t in wt]
            wm=[x[0] for x in wx]

        remchi=mmm/max(1, len(wavelength)-deg-1)
        # with open('sed_fit_log.txt', 'a') as ext_f: 
        #     ext_f.write('### mmm= '+str(mmm)+'\n')
        #     ext_f.write('### wm= '+str(wm)+'\n')
        #     ext_f.write('### remchi= '+str(remchi)+'\n')
        # ext_f.close()            
        if betarange[2]==1:
            nw=0
            nt=0
            nm=0
            i=0
            for nw in range(sb[3]):
                for nm in range(sb[0]):
                    for nt in range(sb[1]):
                        if i==wm[0]:
                            wm3=[nm,nt,0,nw]
                            i=i+1
                        else:
                            i=i+1
        else:
            nw=0
            nt=0
            nb=0
            nm=0
            i=0
            for nw in range(sb[3]):
                for nb in range(sb[2]):
                    
                    for nm in range(sb[0]):
                        for nt in range(sb[1]):
                            if i==wm[0]:
                                wm3=[nm,nt,0,nw]
                                i=i+1
                            else:
                                i=i+1

        m1=max([0,wm3[0]-npos])
        m2=min([sb[1]-1,wm3[0]+npos])
        t1=max([0,wm3[1]-npos])
        t2=min([sb[0]-1,wm3[1]+npos])
        b1=max([0,wm3[2]-npos])

        if betarange[2]!=1:
            
            b2=min([sb[2]-1,wm3[2]+npos])
        else:
            b2=0
        fmass=smass[wm3[0]]
        ftemp=stemp[wm3[1]]
        fbeta=sbeta[wm3[2]]
        # with open('sed_fit_log.txt', 'a') as ext_f: 
        #     ext_f.write('### wm3= '+str(wm3)+'\n')
        #     ext_f.write('### m1= '+str(m1)+', m2='+str(m2)+'\n')
        #     ext_f.write('### t1= '+str(m1)+', t2='+str(t2)+'\n')
        #     ext_f.write('### b1= '+str(m1)+', b2='+str(b2)+'\n')
        #     ext_f.write('### fmass= '+str(fmass)+'\n')
        #     ext_f.write('### ftemp= '+str(ftemp)+'\n')
        #     ext_f.write('### fbeta= '+str(fbeta)+'\n')
        # ext_f.close()
        if abs(fmass-maxmass)/fmass < 1e-6:
            print,'azz',maxmass,fmass
            maxmass=2*maxmass
            smass[m1]=4*smass[m1]
            smass[m2]=maxmass
            count=count+1
            # with open('sed_fit_log.txt', 'a') as ext_f: 
            #     ext_f.write('azz '+ str(maxmass)+', '+str(fmass)+'\n')
            #     ext_f.write('changing smass: smass[m1]=4*smass[m1]='+str(smass[m1])+'\n')
            #     ext_f.write('smass[m2]=maxmass='+str(maxmass)+'\n')
            # ext_f.close()
            continue
    
    
        if silent== 0:
            print(fmass,ftemp,fbeta,mmm)
            print(smass[m1],smass[m2],stemp[t1],stemp[t2],sbeta[b1],sbeta[b2])


        # cchi=total(((rbank[wm,wf]*mask[wm,wf]-cflux[wm,wf])/(rbank[wm,wf]*mask[wm,wf]))**2).sum(2)#/c     ???;stima "personalizzata del chi2 per confrontarlo con l'altro metodo
        rchi=chi0/mmm
        # with open('sed_fit_log.txt', 'a') as ext_f: 
        #     ext_f.write('rchi=chi0/mmm= '+ str(rchi)+'\n')

        # ext_f.close()        
        count=count+1
        if count==1:
            rchi=1e8
        chi0=mmm
        
        
    fmass=fmass*(dist/dvirt)**2
    # with open('sed_fit_log.txt', 'a') as ext_f: 
    #     ext_f.write('Outside of while cycle\n')
    #     ext_f.write('## fmass= '+str(fmass)+'\n')
    # ext_f.close()  
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
                        wm4=[nm,nt,0,nw]
                        i=i+1
                    else:
                        i=i+1


    ssmass=list(smass)
    sstemp=list(stemp)
    dmass1=abs(fmass-ssmass[wm4[0]])
    dmass2=abs(fmass-ssmass[wm4[0]])
    dtemp1=abs(ftemp-sstemp[wm4[1]])
    dtemp2=abs(ftemp-sstemp[wm4[1]])

    dmass=max([dmass1,dmass2])
    dtemp=max([dtemp1,dtemp2])

    if silent==0:
        print('.....')
        print('FIT results:  Mass: '+str(fmass)+'     Temp: '+str(ftemp)+'     Beta: '+str(fbeta)+'     iterations:'+str(count))

    fmassg=fmass*1.98892e33

    flussogb=[float(fmassg*kref/distcm**2*planck.planck(1e4*lm,ftemp)/np.pi*cv*(lambdaref/lm)**fbeta) for lm,cv in zip(lambdagb,conv)]
    
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
        string=str(fmass)+','+str(dmass)+','+str(ftemp)+','+str(dtemp)+','+str(fbeta)+','+str(lum)
        file.write(string)
    return lambdabol,flussobol



