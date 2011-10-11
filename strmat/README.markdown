Based on STRMAT 1.5 from Gusfield et al at UC Davis.

Modifications
-------------

Removed a lot of code
Removed alphabets and simply store strings of CHAR_TYPE.
Added file io to read binary files in string of byte
Replace inefficient edge (substring) and node storage with external maps. See peter_global 
