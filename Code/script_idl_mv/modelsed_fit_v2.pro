function modelsed_fit_v2,wavelength,flux,dflux,fluxflag,w_mod,fmod,dmin,dmax,nstep,sed_weights=sed_weights

;print,sed_weights
par=replicate({d:0.d0, chi2:0.d0},1)

; convert model flux in Jy and interpolate at observations wavelength
d_ref=1000.   ; reference distance for SED models is 1 kpc
; exclude NaNs

qnotnan=where(finite(fmod) eq 1 or fmod gt 0)
wavelength1=wavelength(qnotnan)
flux1=flux(qnotnan)
dflux1=dflux(qnotnan)
fluxflag1=fluxflag(qnotnan)
fmod=fmod(qnotnan)


;establishes weights by assigning equal weights to mid-IR, far-IR and sub-mm
flag=fix(fluxflag1)
ul_flag=intarr(n_elements(flag))
ul_flag(*)=0
ll_flag=intarr(n_elements(flag))
ll_flag(*)=0
dyd=fltarr(n_elements(flag))
;w1=sqrt(3./7.)
;w2=sqrt(3./7.)
;w3=sqrt(3./7.)

if (keyword_set(sed_weights) eq 0) then sed_weights=[3./7.,1./7.,3./7.] 
renorm=total(sed_weights)
w1=sqrt(sed_weights(0)/renorm)
w2=sqrt(sed_weights(1)/renorm)
w3=sqrt(sed_weights(2)/renorm)
qmir=where(wavelength1 lt 25,nqmir)
qfir=where(wavelength1 ge 25 and wavelength1 le 250,nqfir)
qmm=where(wavelength1 gt 250,nqmm)
if (nqmir gt 0) then begin
  q1=where(flag(qmir) eq 1,nq1,complement=q1neg,ncomplement=nq1neg)
  if (nq1 gt 0) then dyd(qmir(q1))=sqrt(nq1)/w1
  if (nq1neg gt 0) then begin
    dyd(qmir(q1neg))=9999.  ;i.e. it's an upper/lower limit
    ul_flag(qmir(q1neg))=1
  endif
endif
if (nqfir gt 0) then begin
  q2=where(flag(qfir) eq 1,nq2,complement=q2neg,ncomplement=nq2neg)
  if (nq2 gt 0) then dyd(qfir(q2))=sqrt(nq2)/w2
  if (nq2neg gt 0) then begin
    dyd(qfir(q2neg))=9999.   ;i.e. it's an upper limit
    ul_flag(qfir(q2neg))=1
  endif
endif
if (nqmm gt 0) then begin
  q3=where(flag(qmm) eq 1,nq3,complement=q3neg,ncomplement=nq3neg)
  if (nq3 gt 0) then dyd(qmm(q3))=sqrt(nq3)/w3
  if (nq3neg gt 0) then begin
    dyd(qmm(q3neg))=9999.   ;i.e. it's an upper limit
    ul_flag(qmm(q3neg))=1
  endif
endif

; now normalize dyd to the minimum value
; so that we artificially increase the uncertainties of the fluxes where the SED-based weight is lower
dyd=dyd/min(dyd)
;print,'DF/F= ',dflux1/flux1
dyd=dyd*dflux1/flux1

; now dyd has to be interpreted as a relative uncertainty otherwise the same assigned weight will have more or less importance depending on the level of flux and flux difference between model and observation. I then multiply the "weight" by the flux value
dyd=dyd*flux1

;nstep=10
;dist_arr=10.^(alog10(dmin)+findgen(nstep)*(alog10(dmax/dmin)/nstep))  
dist_arr=dmin+findgen(nstep)*(dmax-dmin)/nstep

upplim_good=intarr(nstep)
chi2=fltarr(nstep)
invalid_chi2=-999
chi2(*)=invalid_chi2
q_ul=where(ul_flag eq 1,nq_ul)

if (nq_ul ge 1) then begin
  for i=0,nstep-1 do begin
    if ( total(flux1(q_ul)-fmod(q_ul)*((d_ref/dist_arr(i))^2.)) eq total(abs(flux1(q_ul)-fmod(q_ul)*((d_ref/dist_arr(i))^2.))) ) then upplim_good(i)=1 ; if the sum of the source-model fluxes is equal to its absolute value then it means that no upper limit is violated 
  endfor
  qgoodul=where(upplim_good eq 1,nqgoodul)
endif else begin
  qgoodul=indgen(n_elements(upplim_good))
  nqgoodul=n_elements(upplim_good)
endelse

if (nqgoodul gt 0) then begin
  for j=0,nqgoodul-1 do begin
    chi2tmp=total((((fmod*((d_ref/dist_arr(qgoodul(j)))^2.))-flux1)^2)/(dyd^2))

;stop

;    chi2tmp=total((((fmod*((d_ref/dist_arr(qgoodul(j)))^2.))-flux1)^2))
    chi2(qgoodul(j))=chi2tmp
  endfor
  qvalid=where(chi2 ne invalid_chi2)
  qmin=where(chi2(qvalid) eq min(chi2(qvalid)))
  par.d=dist_arr(qvalid(qmin(0)))  ; put qmin(0) because I found there may be two occasions with the minimum chi2
  par.chi2=chi2(qvalid(qmin(0)))
endif else begin
  par.d=-999
  par.chi2=invalid_chi2
endelse

return,par

end
