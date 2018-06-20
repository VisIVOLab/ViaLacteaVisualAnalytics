function pad0_num,num
; num must be a string that contains a number. If the number ends with a floating point without a trailing 0 then this routine will add the 0. This is needed for a bug of the VIALACTEA TAP query service that is screwed up by numbers ending with floating points

q=strpos(num,'.')
if (q eq strlen(num)-1) then num0=num+'0' else num0=num

return,num0
end
