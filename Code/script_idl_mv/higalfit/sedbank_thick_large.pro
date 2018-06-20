pro sedbank_thick_large,lam,lmin,lmax,nl,tmin,tmax,nt,bmin,bmax,nb,smin,smax,ns,lam0,temp,beta,siz,bank
;Written by Davide Elia, 2012-2015
; lam=lambda in um

;print,systime()
; k0 = 0.16 ; cm^2 g^-1  ; Olmi
 ;nu0 = 3e14/250 ; Hz  ; Olmi

 lmin=float(lmin)
 tmin=float(tmin)
 bmin=float(bmin)
 smin=float(smin)


if nl gt 1 then begin
  lstep=(lmax-lmin)/(nl-1)
  lam0=lmin+lstep*findgen(nl)
endif else lam0=lmin

if nt gt 1 then begin
  tstep=(tmax-tmin)/(nt-1)
  temp=tmin+tstep*findgen(nt)
endif else temp=tmin

if nb gt 1 then begin
  bstep=(bmax-bmin)/(nb-1)
  beta=bmin+bstep*findgen(nb)
endif else beta=bmax

if ns gt 1 then begin
  sstep=(smax-smin)/(ns-1)
  psiz=smin+sstep*findgen(ns)
endif else psiz=smin
siz=(psiz/206265.)^2.*!pi/4.

alam=lam*1.d4    ; lambda in Angstrom

nlam=n_elements(lam)
bank=dblarr(nl,nt,nb,ns,nlam)
;param=dblarr(nl,nt,nb,ns,4)

conv=1/2.99792e-13*lam^2.


for il=0,nl-1 do begin
for it=0,nt-1 do begin
for ibb=0,nb-1 do begin
for is=0,ns-1 do begin
tau=(lam0[il]/lam)^beta[ibb]
bank[il,it,ibb,is,*]=siz[is]*planck(alam,temp[it])/!pi*conv*(1- exp(-tau))
;param[il,it,ibb,is,*]=[lam0[il],temp[it],beta[ibb],siz[is]]
endfor
endfor
endfor
endfor


end
