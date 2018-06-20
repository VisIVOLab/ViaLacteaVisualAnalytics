;+
; NAME:
;  RENAME_TAGS()
;
; PURPOSE:
;  Rename tags in a structure
;
; CALLING SEQUENCE:
;  newstruct = rename_tags(struct, oldtagnames, newtagnames)
;
; INPUTS:
;  struct: The original structure. May be an array.
;  oldtagnames: names of tags to change
;  newtagnames: new names for tags
;
; OUTPUTS:
;  A new structure with tags renamed.
;
; OPTIONAL INPUTS:
;  verbose: set to greater than 0 for verbose mode.  Each changed tag will
;    be printed.
;
; MODIFICATION HISTORY:
;  Early 2006: Erin Sheldon, NYU
;-
;
;
;
;  Copyright (C) 2005  Erin Sheldon.  erin dot sheldon at gmail dot com
;
;    This program is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 2 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
;
;

FUNCTION rename_tags, struct, oldtags_in, newtags_in, verbose=verbose

  IF n_params() LT 3 THEN BEGIN 
      print,'-Syntax: newstruct = rename_tags(struct, oldtags, newtags, verbose=)'
      return, -1
  ENDIF 

  if n_elements(verbose) eq 0 then verbose=0

  oldtags = strupcase(oldtags_in)
  newtags = strupcase(newtags_in)

  nst = n_elements(struct)
  nold = n_elements(oldtags)
  nnew = n_elements(newtags)

  IF nold NE nnew THEN BEGIN 
      message,'# of old tags ne # of new tags',/inf
      return, -1
  ENDIF 

  tags = tag_names(struct)
  ntags = n_elements(tags)

  ;; create new structure
  FOR i=0L, ntags-1 DO BEGIN 

      w=where(oldtags EQ tags[i], nw)
      
      IF nw NE 0 THEN BEGIN 
          taguse = newtags[w[0]]
          if verbose gt 0 then begin
              print,'Changing tag "'+tags[i]+'" to "'+taguse+'"'
          endif
      ENDIF ELSE BEGIN 
          taguse = tags[i]
      ENDELSE 
      
      IF i EQ 0 THEN BEGIN 
          newst = create_struct(taguse, struct[0].(i))
      ENDIF ELSE BEGIN 
          newst = create_struct(newst, taguse, struct[0].(i))
      ENDELSE 

  ENDFOR 
  
  ;; copy in the values
  newstruct = replicate(newst, nst)
  FOR i=0L, ntags-1 DO BEGIN 
      newstruct.(i) = struct.(i)
  ENDFOR 

  return, newstruct
END 
