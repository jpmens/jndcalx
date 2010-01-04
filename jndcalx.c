/*
 *	   _           _           _            
 *	  (_)_ __   __| | ___ __ _| |_  __  ___ 
 *	  | | '_ \ / _` |/ __/ _` | \ \/ / / __|
 *	  | | | | | (_| | (_| (_| | |>  < | (__ 
 *	 _/ |_| |_|\__,_|\___\__,_|_/_/\_(_)___|
 *	|__/                                    
 *
 * jndcalx (C)2009/2010 by Jan-Piet Mens
 */

#include "jndcalx.h"

#define CALXVERSION "0.1"

FILE *fpout;
char *progname;

static void usage()
{
	fprintf(stderr, "Usage: %s [-h] [-o output]  dbname\n",
		progname);
	exit(EX_USAGE);
}

int main(int argc, char **argv)
{
	DBHANDLE dbh; 
	FORMULAHANDLE formula_handle;
	WORD wdc;
	STATUS error = NOERROR;
	char *dbname, *formula;
	int c;
	extern int optind;
	extern char *optarg;
	
	progname = *argv;

	fpout = stdout;

	while ((c = getopt(argc, argv, "-ho:")) != EOF) {
		switch (c) {
			case 'h':
				usage();
				break;
			case 'o':
				if ((fpout = fopen(optarg, "w")) == NULL) {
					perror(optarg);
					exit(2);
				}
				break;
			default:
				usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage();
		exit(2);
	}

	dbname = argv[0];

        if (error = NotesInitExtended (argc, argv)) {
                printf("Unable to initialize Notes.\n");
                return (1);
        }


	if (error = NSFDbOpen (dbname, &dbh)) {
		return(notes_error(error, "Can't open database"));
	}

	fprintf(fpout, "BEGIN:VCALENDAR\n");
	fprintf(fpout, "VERSION:2.0\n");
	fprintf(fpout, "PRODID:-//fupps.com/calx//NONSGML v%s//EN\n", CALXVERSION);

	/* Compile the selection formula. */

	formula = "((Form = \"Appointment\") & (@IF(@Date(StartDate) > @Now; 1; 0) = 1))";
	formula = "((Form = \"Appointment\") | (Form = \"Task\"))";

	if (error = NSFFormulaCompile (
		NULL,               /* name of formula (none) */
		(WORD) 0,           /* length of name */
		formula,            /* the ASCII formula */
		(WORD) strlen(formula),    /* length of ASCII formula */
		&formula_handle,    /* handle to compiled formula */
		&wdc,               /* compiled formula length (don't care) */
		&wdc,               /* return code from compile (don't care) */
		&wdc, &wdc, &wdc, &wdc)) /* compile error info (don't care) */

	{
		NSFDbClose (dbh);
		return(notes_error(error, "Can't compile formula"));
	}


	if (error = NSFSearch (dbh, formula_handle,
		NULL,           /* title of view in selection formula */
		0,              /* search flags */
		NOTE_CLASS_DATA,/* note class to find */
		NULL,           /* starting date (unused) */
		procall,   /* call for each note found */
                &dbh,     /* argument to procall */
                NULL))          /* returned ending date (unused) */
	{
		NSFDbClose (dbh);
		return(notes_error(error, "Can't NSFSearch"));
		LAPI_RETURN (ERR(error));
	}
	

	OSMemFree (formula_handle);

	fprintf(fpout, "END:VCALENDAR\n");

	if (error = NSFDbClose (dbh))
		LAPI_RETURN (ERR(error));
	
	NotesTerm();
	LAPI_RETURN (NOERROR);
}

STATUS LNPUBLIC procall(void far *dbh,
            SEARCH_MATCH far *pSearchMatch,
            ITEM_TABLE far *summary_info)
{
	SEARCH_MATCH SearchMatch;
	NOTEHANDLE   nh;
	STATUS       error;
	char form[128];
	long seqnr;
	OID        MainOID;
	

	memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

	/* Skip this note if it does not really match the search criteria
	 * (it is now deleted or modified). This is not necessary for
	 * full searches, but is shown here in case a starting date was
	 * used in the search.
	 */

	if (!(SearchMatch.SERetFlags & SE_FMATCH))
		return (NOERROR);

	if (error = NSFNoteOpen(*(DBHANDLE far *)dbh,
		SearchMatch.ID.NoteID, /* note ID */
		0,                      /* open flags */
		&nh)) {
			return (ERR(error));
	}

	NSFNoteGetInfo(nh, _NOTE_OID, &MainOID);


	strcpy(form, domfield(nh, "Form"));

	if (!strcmp(form, "Appointment")) {

		fprintf(fpout, "BEGIN:VEVENT\n");
	
		/*
		 * STARTDATETIME: TYPE_TIME for single 
		 * STARTDATETIME: TYPE_TOME_RANGE for multiple
		 */


		out_time(nh, "DTSTART", "startdatetime");
		out_time(nh, "DTEND", "enddatetime");
		timerange(nh, "startdatetime");
			out_time(nh, "DTSTAMP", "startdatetime"); // huh?
		out(nh, "SUMMARY", "subject", 1);
		out(nh, "LOCATION", "location", 1);
		out_rich(nh, "DESCRIPTION", "body");
		out_tlist(nh, "CATEGORIES", "categories");
		fprintf(fpout, "UID:%X-%X-calx-generated\n", MainOID.File, MainOID.Note);
	
		// updatesequence contains # of updates if set.

		if ((seqnr = domlong(nh, "updateseq", -1L)) != -1L)
			fprintf(fpout, "SEQUENCE:%ld\n", seqnr);

		fprintf(fpout, "END:VEVENT\n");

	} else if (!strcmp(form, "Task")) {

		char *p;
		int prio;

		if ((p = domfield(nh, "importance")) == NULL)
			p = "99";
		
		prio = (int)atol(p);

		prio = (prio == 3) ? 9 : prio;
		prio = (prio == 2) ? 5 : prio;
		prio = (prio == 1) ? 1 : prio;
		prio = (prio == 99) ? 0 : prio;


		/* iCal:  low=9, med=5, high=1, none=0 */
		/* Lotus: low=3, med=2, high=1, none=99 */

		fprintf(fpout, "BEGIN:VTODO\n");
		out_time(nh, "DTSTART", "originalmodtime");
		out(nh, "SUMMARY", "subject", 1);
		out_tlist(nh, "CATEGORIES", "categories");
		out_time(nh, "DTSTAMP", "originalmodtime");
		// fprintf(fpout, "STATUS:NEEDS-ACTION\n");
		fprintf(fpout, "UID:%X-%X-calx-generated\n", MainOID.File, MainOID.Note);
		out_rich(nh, "DESCRIPTION", "body");
		fprintf(fpout, "PRIORITY:%d\n", prio);

		if (NSFItemIsPresent(nh, "CompletedDateTime", strlen("CompletedDateTime")) == TRUE) {
			fprintf(fpout, "STATUS:COMPLETED\n");
			out_time(nh, "COMPLETED", "CompletedDateTime");
		}
		fprintf(fpout, "SEQUENCE:2\n");
		fprintf(fpout, "END:VTODO\n");
	}

	if (error = NSFNoteClose (nh))
		return (ERR(error));

	return (NOERROR);
}

void encode(FILE *fp, char *buf)
{
	char *bp = buf;
	unsigned int c;

	for (bp = buf; bp && *bp; bp++) {
		c = *bp & 0x00FF;

		switch (c) {
			case '<': fprintf(fp, "&lt;"); break;
			case '>': fprintf(fp, "&gt;"); break;
			case '&': fprintf(fp, "&amp;"); break;
			default: fputc(c, fp);
		}
	}
}


/*
 * If `utf8' is true, convert to UTF-8, otherwise ASCII
 */

void out(NOTEHANDLE nh, char *tag, char *dominofieldname, int utf8)
{
	BOOL found;
	WORD len = 0, outlen = 0;
	char lmbcs[4096], *bp;
	char converted[4096];
	char *tmp, *tp;
	long n, nl = 0;

	found = NSFItemIsPresent(nh, dominofieldname,
                (WORD) strlen(dominofieldname));

	if (!found)
		return;

	len = NSFItemGetText(nh,
			dominofieldname, lmbcs, sizeof(lmbcs) - 1);

	if (len < 1) {
		return;
	}

	fprintf(fpout, "%s:", tag);
	if (len > 0) {
		// Count "newlines" (i.e. zero-bytes)
		for (bp = lmbcs, n = 0; n < len; n++, bp++) {
			if (*bp == '\0')
				++nl;
		}

		tmp = (char *)malloc(strlen(lmbcs) + (nl * 3) + 1);
		if (tmp == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(1);
		}

		/* copy into tmp, expanding newlines */
		for (bp = lmbcs, tp = tmp, n = 0; n < len; n++, bp++) {
			if (*bp == '\0') {	// zero is newline
				*tp++ = ' ';
				*tp++ = '$';
				*tp++ = ' ';
			} else {
				*tp++ = *bp;
			}
		}
		*tp = '\0';

		outlen = OSTranslate(
				(utf8) ? OS_TRANSLATE_LMBCS_TO_UTF8 : OS_TRANSLATE_LMBCS_TO_ASCII,
				tmp, (WORD)strlen(tmp),
				converted, (WORD)sizeof(converted));

		encode(fpout, converted);
		fprintf(fpout, "\n");
	}
}

char *domfield(NOTEHANDLE nh, char *dominofieldname)
{
	BOOL found;
	WORD len = 0;
	static char buf[2048];

	found = NSFItemIsPresent(nh, dominofieldname,
                (WORD) strlen(dominofieldname));

	if (found) {
		len = NSFItemGetText(nh, dominofieldname, buf, sizeof(buf) - 1);
	}

	return (len > 0 ? buf : "");
}

void domtimefield(NOTEHANDLE nh, char *dominofieldname)
{
	TIMEDATE td;
	char timedate_str[MAXALPHATIMEDATE + 1];
	STATUS error_status;
	WORD text_len;
	TFMT td_format;

	if (NSFItemGetTime(nh, dominofieldname, &td)) {


		TIME ti;
		
		ti.GM = td;
		ti.zone = ti.dst = 0;
		if (TimeGMToLocalZone(&ti) == TRUE) {
			puts("Can't convert time!!!!!");
		}

		printf("%04d-%02d-%02d XXXXXXXXXXXXXXX %02d:%02d:%02d\n", 
				ti.year, ti.month, ti.day, 
				ti.hour, ti.minute, ti.second);




		/* Spec Time/Date string format for convert function        */
		td_format.Date = TDFMT_FULL4;   /* show year, month, and day */
		td_format.Time = TTFMT_FULL;   /* show hour, minute, and sec*/
		td_format.Zone = TZFMT_ALWAYS; /* TZ- show on at all times  */

		td_format.Structure = TSFMT_DATE;
		td_format.Structure = TSFMT_DATETIME; /* req'd. Overall Time Date structure    */
		error_status = ConvertTIMEDATEToText(
					NULL,
					&td_format,
					&td,
				       	timedate_str,
					MAXALPHATIMEDATE,
				       	&text_len);
		timedate_str[text_len] = '\0'; /* provide NULL terminator */
		printf("%s: %s\n", "TIME-xxx", timedate_str);
	}
}

void out_time(NOTEHANDLE nh, char *tag, char *dominofieldname)
{
	TIMEDATE td;

	if (NSFItemGetTime(nh, dominofieldname, &td)) {

		TIME ti;
		
		ti.GM = td;
		ti.zone = ti.dst = 0;
		if (TimeGMToLocalZone(&ti) == TRUE) {
			puts("Can't convert time!!!!!");
		}

		fprintf(fpout, "%s:%04d%02d%02dT%02d%02d%02dZ\n", 
				tag,
				ti.year, ti.month, ti.day, 
				ti.hour, ti.minute, ti.second);
	}
}

void out_rich(NOTEHANDLE nh, char *tag, char *dominofieldname)
{
	STATUS error;
	WORD outlen;
	BLOCKID blockid, field_block;
	DWORD field_length, text_length;
	WORD field_type;
	HANDLE text_buffer;
	char converted[4096];
	char *text_ptr, field_text[BUFSIZE + 1];


	/* Look for the RICH_TEXT field within the note. This function tells us
whether the field is there, and if present, its location (BLOCKID)
within Domino and Notes' memory. Check the return code for "field not found" versus
a real error. */

	error = NSFItemInfo (
            nh,
            dominofieldname,
            (WORD)strlen(dominofieldname),
            &blockid,            /* full field (return) */
            &field_type,        /* field type (return) */
            &field_block,        /* field contents (return) */
            &field_length);        /* field length (return) */

	if (ERR(error) != NOERROR)
		return;

	// printf("type == 0x%X, length == %ld\n", field_type, field_length);

/* Extract only the text from the rich-text field into an ASCII string. */

	if (error = ConvertItemToText (
                field_block,    /* BLOCKID of field */
                field_length,    /* length of field */
                "\\n ",        /* line separator for output */
                60,        /* line length in output */
                &text_buffer,    /* output buffer */
                &text_length,    /* output length */
                TRUE))        /* strip tabs */

        {
            return;
        }

/* Lock the memory allocated for the text buffer. Cast the resulting
pointer to the type we need. */

        text_ptr = (char *) OSLockObject (text_buffer);

/* Move the text from the text buffer into a character array,
truncate if necessary to fit the character array, and append a null
to it. (We do this so that we now have a regular C text string.) */

        text_length = (text_length > BUFSIZE ? BUFSIZE : text_length);
        memcpy (field_text, text_ptr, (short) text_length);
        field_text[text_length] = '\0';

/* Print the text of the RICH_TEXT field. */

	// fprintf(fpout, "%s:%s\n", tag, field_text);

	outlen = OSTranslate(
			OS_TRANSLATE_LMBCS_TO_UTF8,
			field_text, (WORD)strlen(field_text),
			converted, (WORD)sizeof(converted));
	fprintf(fpout, "%s:%s\n", tag, converted);

/* Unlock and free the text buffer. */

        OSUnlockObject (text_buffer);
        OSMemFree (text_buffer);

}

/*
 * Output Text List (textlist)
 */

void out_tlist(NOTEHANDLE nh, char *tag, char *dominofieldname)
{
	WORD nents, n, outlen;
	DWORD tlen;
	char converted[4096];
	char ttxt[BUFSIZE + 1];

	if ((nents = NSFItemGetTextListEntries(nh, dominofieldname)) > 0) {

		fprintf(fpout, "%s:", tag);

		for (n = 0; n < nents; n++) {
			tlen = NSFItemGetTextListEntry(nh,
					dominofieldname,
					n,
					ttxt,
					BUFSIZE);
			ttxt[tlen] = '\0';

			outlen = OSTranslate(
					OS_TRANSLATE_LMBCS_TO_UTF8,
					ttxt, (WORD)strlen(ttxt),
					converted, (WORD)sizeof(converted));
			fprintf(fpout, "%s%c", converted, 
					(n < (nents - 1)) ? ',' : '\n');

		}
	}
}

long domlong(NOTEHANDLE nh, char *dominofieldname, long dflt)
{
	if (NSFItemIsPresent(nh, dominofieldname, (WORD) strlen(dominofieldname))) {
		return (NSFItemGetLong(nh, dominofieldname, dflt));
	}

	return (dflt);
}

int notes_error(STATUS err, char *txt)
{
	char str[2048];

	OSLoadString(NULL, ERR(err), str, sizeof(str)-1);

	fprintf(stderr, "Notes: %s: err=0x%X [%s]\n",
			txt, err, str);
	LAPI_RETURN (ERR(err));
}

void LNPUBLIC  GetTDString( TIMEDATE * ptdModified, char * szTimedate )
{
    WORD            wLen;

    ConvertTIMEDATEToText( NULL, NULL,
                            ptdModified,
                            szTimedate,
                            MAXALPHATIMEDATE,
                            &wLen );
    szTimedate[wLen] = '\0';
}

void sprint_td(TIMEDATE td, char *buf)
{
	TIME ti;
	
	ti.GM = td;
	ti.zone = ti.dst = 0;
	if (TimeGMToLocalZone(&ti) == TRUE) {
		puts("Can't convert time!!!!!");
	}

	sprintf(buf, "%04d%02d%02dT%02d%02d%02dZ", 
			ti.year, ti.month, ti.day, 
			ti.hour, ti.minute, ti.second);
}

void dumptimerange(char *data, WORD len)
{
	RANGE range;
	TIMEDATE td;
	TIMEDATE_PAIR tdp;
	USHORT usListEntry, usRangeEntry;
	char szTimedate[MAXALPHATIMEDATE+1], szUpperTD[MAXALPHATIMEDATE+1], buf[BUFSIZ];

	/* Get the RANGE from the location specified by pData */
	memcpy( (char*)&range, data, sizeof(RANGE) );

    /* Advance pData to point to the first TIMEDATE in the array */
    data += sizeof(RANGE);

    /* First print the list entries, then the range entries.*/

    for (usListEntry = 0; usListEntry < range.ListEntries; usListEntry++) {
        memcpy( (char*)&td, data, sizeof(TIMEDATE) );
        data += sizeof(TIMEDATE);

	sprint_td(td, buf);
	fprintf(fpout, "%s/%s", buf, buf);
	if (usListEntry < (range.ListEntries - 1)) {
		fprintf(fpout, "\n ,");
	}


        //GetTDString( &td, szTimedate );
	//printf("stimed---> %s\n", szTimedate);

    }
    fprintf(fpout, "\n");

/*
    for (usRangeEntry = 0; usRangeEntry < range.RangeEntries; usRangeEntry++) {
        memcpy( (char*)&tdp, data, sizeof(TIMEDATE_PAIR) );
        data += sizeof(TIMEDATE_PAIR);
        GetTDString( &tdp.Lower, szTimedate );
        GetTDString( &tdp.Upper, szUpperTD );

		printf("---> szTimedate: %s\n", szTimedate);
		printf("---> szUpperTD:  %s\n", szUpperTD);
    }
    */
}

void timerange(NOTEHANDLE nh, char *domfield)
{
	BLOCKID itembid, valbid;
	WORD datatype;
	DWORD valuelen;
	STATUS err;
	char *ptr;

	if ((err = NSFItemInfo(nh, domfield, strlen(domfield),
		   &itembid, &datatype, &valbid, &valuelen)) != NOERROR) {
		notes_error(err, "Can't get NSFItemInfo");
		return;
	}

	switch (datatype) {
		// case TYPE_TIME:
		// 	puts("	! Got Time!"); break;
		case TYPE_TIME_RANGE:
			if ((ptr = OSLockBlock(char, valbid)) != NULL) {
				fprintf(fpout, "TRANSP:OPAQUE\n");
				fprintf(fpout, "RDATE;VALUE=PERIOD:");
				dumptimerange(ptr + sizeof(WORD), valuelen);
				OSUnlockBlock(valbid);
			}
			break;
	}
}
