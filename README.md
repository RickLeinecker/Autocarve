Authors: Derrick Greenspan, Richard Leinecker, and the University of Central Florida


To compile: run make

The program expects a file called "FormatSpecs.txt"

FormatSpects.txt is structured like so:
Header=HEADERINFO (append h to the end of it to indicate it is a hex value, and not a character)
Extension=NAMEOFEXTENSION (e.g.: png) 
(optional):
Validation=VALIDATION (valid ranges for the header - to prevent false positives)

Usage: AutoCarve followed by filename to carve. 
