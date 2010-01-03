#ifndef JNDCALX_H_INCLUDED

#define JNDCALX_H_INCLUDED 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysexits.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <lapicinc.h>
#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <nsfdata.h>
#include <osmem.h>
#include <odserr.h>
#include <ods.h>
#include <osmisc.h>
#include <nsferr.h>
#include <lapiplat.h>

#define BUFSIZE		1024


STATUS LNPUBLIC procall (void far *, SEARCH_MATCH far *, 
                                ITEM_TABLE far *);
void out(NOTEHANDLE nh, char *tag, char *dominofieldname, int utf8);
void out_time(NOTEHANDLE nh, char *tag, char *dominofieldname);
void out_rich(NOTEHANDLE nh, char *tag, char *dominofieldname);
void out_tlist(NOTEHANDLE nh, char *tag, char *dominofieldname);
long domlong(NOTEHANDLE nh, char *dominofieldname, long dflt);
char *domfield(NOTEHANDLE nh, char *dominofieldname);
void domtimefield(NOTEHANDLE nh, char *dominofieldname);
void encode(FILE *fp, char *buf);
void timerange(NOTEHANDLE nh, char *dominofieldname);

int notes_error(STATUS err, char *txt);

#endif
