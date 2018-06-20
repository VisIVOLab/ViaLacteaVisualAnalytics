pro sedbank_thin_large,lambda,mmin,mmax,nm,tmin,tmax,nt,bmin,bmax,nb,kref,lambdaref,dist,mass,temp,beta,bank,mlin=mlin,m2=m2
;Written by Davide Elia, 2012-2015

Rgd = 1. ; il fattore 100 � gi� contenuto in k0
mmin=float(mmin)
tmin=float(tmin)
bmin=float(bmin)

if nm gt 1 then begin
  if keyword_set(m2) then nm=nm/2
    mstep=(mmax^0.2-mmin^0.2)/(nm-1)
    amass=mmin^0.2+mstep*findgen(nm)
    mass=amass^5.
  if keyword_set(mlin) then begin
    mstep=(mmax-mmin)/(nm-1)
    mass=mmin+mstep*findgen(nm)
  endif
endif else mass=mmin

massg=mass*1.98892e33 ;g
distcm=dist*3.09d18

if nt gt 1 then begin
  tstep=(tmax-tmin)/(nt-1)
  temp=tmin+tstep*findgen(nt)
endif else temp=tmin

if nb gt 1 then begin
  bstep=(bmax-bmin)/(nb-1)
  beta=bmin+bstep*findgen(nb)
endif else beta=bmax


lam=lambda  ; um
alam=lambda*1.d4    ; lambda in Angstrom

nlam=n_elements(lam)
bank=dblarr(nm,nt,nb,nlam)
;param=dblarr(nl,nt,nb,ns,4)

conv=1/2.99792e-13*lam^2.


for im=0,nm-1 do begin
for it=0,nt-1 do begin
for ibb=0,nb-1 do begin

bank[im,it,ibb,*]=massg[im]*kref/distcm^2*planck(alam,temp[it])/!pi*conv*(lambdaref/lam)^beta[ibb]


endfor
endfor
endfor


end
