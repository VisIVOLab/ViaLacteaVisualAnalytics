pro newtest
lambda=[250,350,500]                           ; in micron
flux= [7.52693,4.67851,1.28158]                ; in Jy
eflux=[0.169653,0.246719,0.337756,0.266316]    ; in Jy
;lambda=[70.,160,250,350,500]                  ; in micron
;flux=[244.963,238.151,492.615,236.352,37.521] ; in Jy
;eflux=[27.166,54.749,106.240,37.521,8.985]    ; in Jy
sizz=20.                                      ; observed FWHM in arcsec
dist=1000.                                    ; in pc
mrange=[10.,5000.,100]                        ; in M_Sun ; only for opt. thin
trange=[7.,40.,50]                            ; in K; for both opt. thin and thick
brange=[2.000,2.000,1]                        ; for both opt. thin and thick
l0range=[10.,300,100]                         ; in micron; only for opt. thick
sfactrange=[1.000,1.000,1]                    ; range of variability of the source size with respect to the observed
                                              ; one (sizz); only for opt. thick
sref=[0.1,300]                                ; kref in cm^2 g-1, lref (micron) 
ul=[1,0,0,0,0]                                ; upper limit switch on/off; 0: regular flux, 1: upper limit 

thinfile='./thinfit.csv'    ; output file containing the best-fitting greybody in csv format
thickfile='./thickfit.csv'

sedfitgrid_engine_thin_vialactea,lambda,flux,mrange,trange,brange,dist,sref,lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars=eflux,ulimit=ul,printfile=thinfile ; /silent
;sedfitgrid_engine_thick_vialactea,lambda,flux,sizz,trange,brange,l0range,sfactrange,dist,sref,lambdagb,flussogb,mtk,ttk,btk,l0,sizesec,ltk,dmtk,dtk,dl0,errorbars=eflux,ulimit=ul,printfile=thickfile ; /silent


end
