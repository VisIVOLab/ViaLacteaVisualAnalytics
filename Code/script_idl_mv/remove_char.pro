function remove_char,in_str,ch

q=strpos(in_str,ch)
if (q eq -1) then begin
	print,'character ',ch,' not found in string ',in_str
	out_str=in_str
endif else begin
	out_str=strmid(in_str,0,q)+strmid(in_str,q+1,strlen(in_str)-1)
endelse

return,out_str
end
