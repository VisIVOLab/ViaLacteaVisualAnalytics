pro sedfitgrid_engine_thick_vialactea,lambda,flux,sizz,temprange,betarange,l0range,sfactrange,dist,sref,lambdagb,flussogb,mass,ftemp,fbeta,fl0,sizesec,lum,dmass,dtemp,dlam0,errorbars=errorbars,ulimit=ulimit,silent=silent,printfile=printfile
; version for VIALACTEA, it doesn't use a settings file, but accepts the initial ranges as input variables
;Written by Davide Elia, 2012-2015

; lambda in um, size in arcsec, flux in Jy, size in arcsec, dist in pc
; temprange,betarange,l0range,sfactrange: ; 3-element vectors, cotaining min, max and number of steps
; sref: 2-element vector, containing opacity at lambda-ref (cm^2_g^-1), and lambda_ref itself (micron).

stemp=temprange
sbeta=betarange
sl0=l0range
sfact=sfactrange

if temprange[2]  lt 5 then deg1=0 else deg1=1
if betarange[2]  lt 5 then deg2=0 else deg2=1
if sfactrange[2] lt 5 then deg3=0 else deg3=1
if l0range[2]    lt 5 then deg4=0 else deg4=1
deg=deg1+deg2+deg3+deg4

pccm=3.09d18
kref=sref[0]
lambdaref=sref[1]

lambdagb=5.+findgen(1995)

conv=1/2.99792e-13*lambdagb^2.

ssize=sfact*sizz

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

lmask=fltarr(n_elements(lambda))
wf=where(flux,c)
lmask[wf]=1.

if keyword_set(ulimit) gt 0 then begin
  wu=where(ulimit ne 0,cu)
  if cu ne 0 then lmask[wu]=0.
endif

if keyword_set(silent) eq 0 then begin
print,''
print,'------------------------------------------------------'
print,'flux= ',flux
endif


sedbank_thick_large,lambda,sl0[m1],sl0[m2],l0range[2],stemp[t1],stemp[t2],temprange[2],sbeta[b1],sbeta[b2],betarange[2],ssize[s1],ssize[s2],sfact[2],ssl0,sstemp,ssbeta,sssize,bank
sb=size(bank)
cflux=fltarr(sb[1]*sb[2]*sb[3]*sb[4],sb[5])
mask=fltarr(sb[1]*sb[2]*sb[3]*sb[4],sb[5])
if keyword_set(errorbars) ne 0 then eflux=fltarr(sb[1]*sb[2]*sb[3]*sb[4],sb[5])

for tt=0,sb[5]-1 do begin
  cflux[*,tt]=flux[tt]
  mask[*,tt]=lmask[tt]
  if keyword_set(errorbars) ne 0 then eflux[*,tt]=errorbars[tt]
endfor


while (rchi gt 1.01 and count le 10) do begin

sedbank_thick_large,lambda,sl0[m1],sl0[m2],l0range[2],stemp[t1],stemp[t2],temprange[2],sbeta[b1],sbeta[b2],betarange[2],ssize[s1],ssize[s2],sfact[2],ssl0,sstemp,ssbeta,sssize,bank
sssize=206265.*sqrt(sssize*4./!pi)

sl0=ssl0
sbeta=ssbeta
stemp=sstemp
ssize=sssize
sb=size(bank)

ctemp=[250. , 100 ,  50 ,  40 , 30  , 20  , 19  , 18  , 17  , 16  , 15  , 14  , 13  , 12  , 11  , 10  ,  9  ,  8  ,  7   ,  6   ,   5   ]
c70 = [1.005,0.989,0.982,0.992,1.034,1.224,1.269,1.325,1.396,1.488,1.607,1.768,1.992,2.317,2.816,3.645,5.175,8.497,17.815,58.391,456.837] 
c100= [1.023,1.007,0.985,0.980,0.982,1.036,1.051,1.069,1.093,1.123,1.162,1.213,1.282,1.377,1.512,1.711,2.024,2.554, 3.552, 5.774, 12.259] 
c160= [1.062,1.042,1.010,0.995,0.976,0.963,0.964,0.967,0.972,0.979,0.990,1.005,1.028,1.061,1.110,1.184,1.300,1.491, 1.833, 2.528,  4.278]

w70=where(lambda eq 70.,cw70)
if cw70 ne 0 then begin
ic70=interpol(c70,ctemp,stemp,/quadratic)
corr70=fltarr(sb[1],sb[2],sb[3],sb[4])
for z=0,sb[2]-1 do begin
corr70[*,z,*,*]=ic70[z]
endfor
bank[*,*,*,*,w70]=bank[*,*,*,*,w70]*corr70
endif

w160=where(lambda eq 160.,cw160)
if cw160 ne 0 then begin
ic160=interpol(c160,ctemp,stemp,/quadratic)
corr160=fltarr(sb[1],sb[2],sb[3],sb[4])
for z=0,sb[2]-1 do begin
corr160[*,z,*,*]=ic160[z]
endfor
bank[*,*,*,*,w160]=bank[*,*,*,*,w160]*corr160
endif



rbank=reform(bank,sb[1]*sb[2]*sb[3]*sb[4],sb[5])

if keyword_set(errorbars) ne 0 then begin
  chi=total(((rbank*mask-cflux)/(eflux+1e-7))^2,2)
endif else begin
  chi=total((rbank*mask-cflux)^2/(rbank*mask+1e-7),2)
endelse
redchi=chi/max([1,(n_elements(lambda)-deg-1 )])

if keyword_set(ulimit) gt 0 then begin
   if cu gt 1 then mdiff=sum((rbank[*,wu]-cflux[*,wu])/abs(rbank[*,wu]-cflux[*,wu]),1) else mdiff=(rbank[*,wu[0]]-cflux[*,wu[0]])/abs(rbank[*,wu[0]]-cflux[*,wu[0]])
   wf=where(mdiff lt cu,cf)
   mmm=min(chi[wf],wmm)
   wm=wf[wmm]
endif else mmm=min(chi,wm)
remchi=mmm/max([1,(n_elements(lambda)-deg-1 )])

wm3 = ARRAY_INDICES(bank,wm)
wm3 = ARRAY_INDICES(bank,wm)
m1=max([0,wm3[0]-npos])
m2=min([sb[1]-1,wm3[0]+npos])
t1=max([0,wm3[1]-npos])
t2=min([sb[2]-1,wm3[1]+npos])
b1=max([0,wm3[2]-npos])
b2=min([sb[3]-1,wm3[2]+npos])
s1=max([0,wm3[3]-npos])
s2=min([sb[4]-1,wm3[3]+npos])

fl0=sl0[wm3[0]]
ftemp=stemp[wm3[1]]
fbeta=sbeta[wm3[2]]
fsize=(ssize[wm3[3]]/206265.)^2*!pi/4.
if KEYWORD_SET(silent) eq 0 then begin
print,fl0,ftemp,fbeta,206265.*sqrt(fsize*4./!pi),mmm
print,sl0[m1],sl0[m2],stemp[t1],stemp[t2],sbeta[b1],sbeta[b2],ssize[s1],ssize[s2]
endif

cchi=total(((rbank[wm,wf]*mask[wm,wf]-cflux[wm,wf])/(rbank[wm,wf]*mask[wm,wf]))^2,2)/c ;stima "personalizzata del chi2 per confrontarlo con l'altro metodo
rchi=chi0/mmm
count=count+1
if count eq 1 then rchi=1e8
chi0=mmm
endwhile

inc=1
ccc=0
while ccc eq 0  do begin
wwc=where(redchi lt remchi+inc,ccc)
inc=inc+1
endwhile

wm4 = ARRAY_INDICES(bank,wwc)
wm4 = ARRAY_INDICES(bank,wwc)

dtemp1=abs(ftemp-max(stemp[wm4[1,*]]))
dtemp2=abs(ftemp-min(stemp[wm4[1,*]]))
dlam01=abs(fl0-max(sl0[wm4[0,*]]))
dlam02=abs(fl0-min(sl0[wm4[0,*]]))
dbeta1=abs(fbeta-max(sbeta[wm4[2,*]]))
dbeta2=abs(fbeta-min(sbeta[wm4[2,*]]))
dsize1=abs(fsize-max((ssize[wm4[3,*]])/206265.)^2*!pi/4.)
dsize2=abs(fsize-min((ssize[wm4[3,*]])/206265.)^2*!pi/4.)
mll=[min(sl0[wm4[0,*]]),max(sl0[wm4[0,*]])]
mss=([min(ssize[wm4[0,*]]),max(ssize[wm4[0,*]])]/206265.)^2*!pi/4.
mbb=[min(sbeta[wm4[0,*]]),max(sbeta[wm4[0,*]])]

dmm=dblarr(8)
for k0=0,1 do begin
for k1=0,1 do begin
for k2=0,1 do begin
dmm[4*k0+2*k1+k2]=(dist*pccm)^2.*mss[k1]/kref*(mll[k0]/lambdaref)^mbb[k2]
;print,dmm[4*k0+2*k1+k2]
endfor
endfor
endfor
dmass1=abs(ftemp-max(sbeta[wm4[2,*]]))
dmass2=abs(ftemp-min(sbeta[wm4[2,*]]))


dlam0=max([dlam01,dlam02])
dmass=max([dmass1,dmass2])
dtemp=max([dtemp1,dtemp2])

if KEYWORD_SET(silent) eq 0 then begin
print,'.....'
;print,param[wm3[0],wm3[1],wm3[2],wm3[3],*]
endif

;window,1
;plot_oo,lambda[wf],flux[wf],psym=4,yrange=[1e-2,1.3*max(flux[wf])]

;xyouts,0.2,0.9,strtrim(id[i],1),/normal

mass=(dist*pccm)^2.*fsize/kref*(fl0/lambdaref)^fbeta
mass=mass/1.98892e33
dmass1=abs(mass-max(dmm)/1.98892e33)
dmass2=abs(mass-min(dmm)/1.98892e33)
dmass=max([dmass1,dmass2])
sizesec=206265.*sqrt(fsize*4./!pi)
if KEYWORD_SET(silent) eq 0 then begin
  print,'FIT results:  Mass: ',mass,'     Temp: ',ftemp,'     Beta: ',fbeta,'     lambda_0:',fl0,'     size:',sizesec,'     iterations:',count
endif
tau=(fl0/lambdagb)^fbeta
flussogb=fsize*planck(lambdagb*1e4,ftemp)/!pi*conv*(1- exp(-tau))

qd=where(flussogb gt 1e-8)
;oplot,lambdagb[qd],flussogb[qd]
lum=lbol(lambdagb[qd],flussogb[qd],dist)

if keyword_set(printfile) then begin
  openw,lun,printfile,/get_lun
  for uu=0,n_elements(lambdagb)-1 do begin
    str=string([lambdagb[uu],flussogb[uu]])
    printf,lun,strjoin(strtrim(str,2),','),format='(A)'
  endfor
  openw,lun2,printfile+'.par',/get_lun
  cmm=','
  printf,lun2,mass,cmm,dmass,cmm,ftemp,cmm,dtemp,cmm,fbeta,cmm,fl0,cmm,dlam0,cmm,sizesec,cmm,lum,format='(f13.2,A1,f13.2,A1,f7.2,A1,f7.2,A1,F5.2,A1,F7.2,A1,F7.2,A1,f6.1,A1,f13.2)'
  free_lun,lun
  free_lun,lun2
endif

end
