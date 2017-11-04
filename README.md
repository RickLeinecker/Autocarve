Authors: Richard Leinecker PhD, Derrick Greenspan, and the University of Central Florida


To compile: 
On Linux: Run make
On Windows: Make sure that Mingw32-make is in your PATH
        (on most Windows systems, MinGW will be installed to C:\MinGW
        so run: path=%path%;C:\MinGW\bin\)
        Then run Mingw32-make

The program expects a file called "FormatSpecs.txt"

FormatSpects.txt is structured like so:
Header=HEADERINFO (append h to the end of it to indicate it is a hex value, and not a character)
Extension=NAMEOFEXTENSION (e.g.: png) 
(optional):
Validation=VALIDATION (valid ranges for the header - to prevent false positives)

Usage: AutoCarve followed by filename to carve. Optional params WRITELOG and FINDONLY
