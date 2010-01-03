#
# makefile for Notes API sample program RSIMPLE
# Windows 32-bit version using Microsoft 32-bit Visual C++ compiler 
# and linker.
#
# This makefile assumes that the INCLUDE and LIB environment variables
# are set up to point at the Notes and C "include" and "lib" directories.

# Standard Windows 32-bit make definitions
!include <ntwin32.mak>

# The name of the program.
#


all: jndcalx.exe

PROGNAME = jndcalx

cpuflags = /Zp

# Dependencies

$(PROGNAME).EXE: $(PROGNAME).OBJ
$(PROGNAME).OBJ: $(PROGNAME).C

$(PROGNAME).OBJ: $(PROGNAME).C
# Compilation command.  

.C.OBJ:
    $(cc) $(cdebug) $(cflags) $(cpuflags) /DNT $(cvars) $*.c

# Link command.
           
.OBJ.EXE:
    $(link) $(linkdebug) $(conflags) -out:$@ $** $(conlibs) \
    	notes.lib user32.lib
