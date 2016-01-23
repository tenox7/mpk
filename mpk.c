/*________________________________________
 |                                        |
 | mime pack - v1.0                       |
 | Copyright (c) 2005 by Antoni Sawicki   |
 |________________________________________|
 |
 |
 |
 |
*/

#define BOUNDARY "8<--cut--8<--here--8<--"


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

FILE *fi, *fo;

//
// base64 stuff
//
#define TRUE 1
#define FALSE 0
#define LINELEN 72
#define MAXINLINE 256 


typedef unsigned char byte;

static byte     iobuf[MAXINLINE];
static int      iolen = 0;
static int      iocp = MAXINLINE;
static int      ateof = FALSE;
static byte     dtable[256];
static int      linelength = 0;
static char     eol[] = "\r\n";
static int      errcheck = TRUE;

static int inbuf(void);
static int inchar(void);
static void ochar(int);
static void encode(void);


int main(int argc, char **argv) {
	int fn;
	char inp[1024];
	struct stat fs;

	if(argc<2) {
		fprintf(stderr, "Usage: %s filename ...\n", argv[0]);
		exit(1);
	}

	fo=stdout;

	//
	// mime header
	//
	fprintf(stdout, 
		"MIME-Version: 1.0\n"
		"Content-Type: multipart/mixed; boundary=\"%s\"\n"
		"\n"
		"This is a MIME encoded message.\n"
		"\n",
	BOUNDARY);

	//
	// handle input
	//
	for(fn=1; fn<argc; fn++) {

		//
		// text message from stdin
		//
		if(strcmp("-m", argv[fn])==0) {
			fprintf(stdout, "--%s\nContent-Type: text/plain\nContent-Disposition: inline\n\n", BOUNDARY);
			while(fgets(inp, 990, stdin))
				fprintf(stdout, "%s", inp);
			fprintf(stdout, "\n--%s\n", BOUNDARY);
		} 
		// attachment files
		else if(stat(argv[fn], &fs)==0 && !(fs.st_mode & S_IFDIR)) {
			if(fi=fopen(argv[fn], "r")) {
				fprintf(stderr, "reading %d:%s\n", fn, argv[fn]);
				encode();


				fclose(fi);
			}
		}

	}

	return 0;
}


//
// base64.c by John Walker 
//

static int inbuf(void) {
	int             l;

	if (ateof) {
		return FALSE;
	}
	l = fread(iobuf, 1, MAXINLINE, fi);
	if (l <= 0) {
		if (ferror(fi)) {
			exit(1);
		}
		ateof = TRUE;
		return FALSE;
	}
	iolen = l;
	iocp = 0;
	return TRUE;
}

static int inchar(void) {
	if (iocp >= iolen) {
		if (!inbuf()) {
			return EOF;
		}
	}
	return iobuf[iocp++];
}

static void ochar(int c) {
	if (linelength >= LINELEN) {
		if (fputs(eol, fo) == EOF) {
			exit(1);
		}
		linelength = 0;
	}
	if (putc(((byte) c), fo) == EOF) {
		exit(1);
	}
	linelength++;
}

static void encode(void)
{
	int i, hiteof = FALSE;

	for (i = 0; i < 9; i++) {
		dtable[i] = 'A' + i;
		dtable[i + 9] = 'J' + i;
		dtable[26 + i] = 'a' + i;
		dtable[26 + i + 9] = 'j' + i;
	}
	for (i = 0; i < 8; i++) {
		dtable[i + 18] = 'S' + i;
		dtable[26 + i + 18] = 's' + i;
	}
	for (i = 0; i < 10; i++) {
		dtable[52 + i] = '0' + i;
	}
	dtable[62] = '+';
	dtable[63] = '/';

	while (!hiteof) {
		byte            igroup[3], ogroup[4];
		int             c, n;

		igroup[0] = igroup[1] = igroup[2] = 0;
		for (n = 0; n < 3; n++) {
			c = inchar();
			if (c == EOF) {
				hiteof = TRUE;
				break;
			}
			igroup[n] = (byte) c;
		}
		if (n > 0) {
			ogroup[0] = dtable[igroup[0] >> 2];
			ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
			ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
			ogroup[3] = dtable[igroup[2] & 0x3F];

			if (n < 3) {
				ogroup[3] = '=';
				if (n < 2) {
					ogroup[2] = '=';
				}
			}
			for (i = 0; i < 4; i++) {
				ochar(ogroup[i]);
			}
		}
	}
	if (fputs(eol, fo) == EOF) {
		exit(1);
	}
}



// vim:ts=4:sw=4
