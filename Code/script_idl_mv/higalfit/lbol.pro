function lbol,w,f,d

; interpolation between data points is done in logarithmic space to allow 
; straight lines (the SED is like that) in the log-log plot. interpolation 
; on a finer grid is done and then data are transformed back in linear space
; where the trapezoidal interpolation is then done

; w is lambda in um
; f is flux jy
; d is dist in parsec

;convert flux to W/cm2/um
fw=1.e-15*f*.2997/(w^2.)
lw=alog10(w)
lfw=alog10(fw)
;1000 points resampling
lw1=(findgen(1000)*((max(lw)-min(lw))/1000.))+min(lw)
lfw1=interpol(lfw,lw,lw1)

w1=10.^lw1
fw1=10.^lfw1
jy=fw1/1.e-15/.3*(w1^2.)
;integrate over whole range
fint=0.
for i=0,n_elements(w1)-2 do fint=fint+((fw1(i)+fw1(i+1))*(w1(i+1)-w1(i))/2.)

;fint=int_tabulated(w,fw,/double,/sort)
; integrate longword of 350um
;qc0=where(w ge 350.)
;fc0=int_tabulated(w(qc0),fw(qc0))
; compute lbol
l=fint*4.*!pi*d*3.d18/3.8d26*d*3.d18   ;lsol

;c0ratio=fc0/fint
;print,'Lsubmm/Lbol = ',c0ratio

return,l
end



