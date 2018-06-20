function vialactea_tap_sedfit,w,f,fflag,distance,prefilter_thresh,sed_weights=sed_weights,local=local,use_wave=use_wave

DB_schema='vlkb_compactsources'
DB_table='sed_models'
user='root'
db_server=''
phys_par=strupcase(['clump_mass','clump_upper_age'])

jy2mjy=1000.

; w is the array of SED wavelengths in microns
; f is the array of the fluxes in Jy (df contains the uncertainties, if defined)
; distance is in parsec
; prefilter_thresh is the threshold for prefiltering the model grids at the given USE_WAVE wavelength

; this is an updated version with respect to FIT_SED because it only uses fixed photometric bands and extract them already at the stage of initial model selection 
; instead of re-extracting each and every pre-selected model a second time. Indeed, most of the computing time was being wasted for the MYSQL calls
d_ref=1000.
ref_wave=[3.6,4.5,5.8,8.0,24.0,70.,100.,160.,250.,350.,500.]
col_names=['I1','I2','I3','I4','M1','PACS1','PACS2','PACS3','SPIR1','SPIR2','SPIR3']

fac_resc=(distance/d_ref)^2.
; now builds the string for mysql query to dump either the full model or the discrete sed points corresponding to fixed wavelengths
  
par_str=''
par_str_arr=strarr(n_elements(w))
ret_par_str=''
phys_str=''
phys_par_arr=strarr(n_elements(phys_par))
ret_phys_par=''  
for k=0,n_elements(w)-1 do begin
  qw=where(ref_wave-0.5 lt w(k) and ref_wave+0.5 gt w(k)) 
  par_str=par_str+col_names(qw(0))+','
  ret_par_str=ret_par_str+col_names(qw(0))+','
  par_str_arr(k)=strlowcase(col_names(qw(0)))
endfor
par_str=strupcase(strmid(par_str,0,strlen(par_str)-1))
ret_par_str=strlowcase(strmid(ret_par_str,0,strlen(ret_par_str)-1))

for k=0,n_elements(phys_par)-1 do begin
  phys_str=phys_str+phys_par(k)+','
  ret_phys_par=ret_phys_par+phys_par(k)+','
  phys_par_arr(k)=strlowcase(phys_par(k))
endfor
phys_str=strupcase(strmid(phys_str,0,strlen(phys_str)-1))
ret_phys_par=strlowcase(strmid(ret_phys_par,0,strlen(ret_phys_par)-1))

if (keyword_set(use_wave)) then begin
  qband=where(w eq use_wave)
  if (qband eq -1) then begin
    print,'Reference wavelength required not found in data file.....EXIT'
    return,-1
  endif
  qrefband=where(ref_wave eq use_wave)
  nreq_par=1+n_elements(phys_par_arr)+n_elements(par_str_arr)
    
  dqlcmd="select id,"+phys_str+","+par_str+" from "+db_schema+"."+db_table+" where ("+col_names(qrefband(0))+">'"+tostring(f(qband(0))*jy2mjy*fac_resc*(1-prefilter_thresh))+"') and ("+col_names(qrefband(0))+" <'"+tostring(f(qband(0))*jy2mjy*fac_resc*(1+prefilter_thresh))+"')" ;fluxes are mutliplied by 1000 because model fluxes are in milliJy  
    
  curlcmd='curl --user vialactea:ia2vlkb -L --data "REQUEST=doQuery&VERSION=1.0&LANG=ADQL&FORMAT=csv&QUERY='
  
  curl_endpoint='" "http://palantir19.oats.inaf.it:8080/vlkb/sync?"'
  
  spawn,curlcmd+dqlcmd+curl_endpoint, data

  openw,unit1,'sed_returned.dat', /get_lun
  for i=0,n_elements(data)-1 do printf,unit1,data(i)
  free_lun, unit1
    
;  models=read_votable8('sed_returned.dat')
  dmodels=read_csv('sed_returned.dat', header=new_tags, count=nlines)
  if (nlines le 1) then return,-1
  old_tags=tag_names(dmodels)
  models=rename_tags(dmodels,old_tags,new_tags)
  
endif else begin
; compute object luminosity from observed SED
  lum=lbol(w,f,distance)
; rescaling factor has to stay at 1 if luminosity is used
  fac_resc=1.
  z=execute('mysqlquery,lun," select MODEL_NAME, INCL, THETA,'+par_str+' from YSO_OBJECTS_2 where (LTOT_OBS>'+string(lum*fac_resc*(1-prefilter_thresh))+') and (LTOT_OBS <'+string(lum*fac_resc*(1+prefilter_thresh))+');",id,incl,theta,'+ret_par_str)
endelse


n_sel=n_elements(models.id)
par=replicate({id:0L, clump_mass:0.d0, clump_age:0.d0, d:0.d0, chi2:0.d0, wmod:ptr_new(), fmod:ptr_new()},n_sel)


;MODIFICARE PER METTERE IL CHI2 NEL FILE DI OUTPUT
openr,unit1,'sed_returned.dat', /get_lun
openw,unit2,'sedfit_output.dat', /get_lun

s=' '
readf,unit1,s
printf,unit2,s+',CHI2,DIST'

; now starts fitting to the subset of models
for i=0L,n_sel-1 do begin
    print,'Doing model ',models.id(i),' clump mass= ',models.clump_mass(i),'...',n_sel-i,' to go'
      w_mod=w
      fjy_mod=dblarr(n_elements(w))
      for k=0,n_elements(w)-1 do z=execute('fjy_mod(k)=double(models.'+par_str_arr(k)+'(i))/1000.') ;divide by 1000 because model fluxes are in milliJy
      fmod=fjy_mod
    p=modelsed_fit(w,f,fflag,w_mod,fmod,distance*(1-sqrt(prefilter_thresh)),distance*(1+sqrt(prefilter_thresh)),5,sed_weights=sed_weights)
    par(i).id=models.id(i)
    par(i).clump_mass=models.clump_mass(i)
    par(i).clump_age=models.clump_upper_age(i)
    par(i).d=p.d
    par(i).chi2=p.chi2
    par(i).wmod=ptr_new(w_mod)
    par(i).fmod=ptr_new(fmod)

	readf,unit1,s
	printf,unit2,s+','+tostring(p.chi2)+','+tostring(p.d)
endfor

free_lun,unit1,unit2

return,par

end
