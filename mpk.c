/*___________________________________________
 |                                           |
 | MPK - MIME Pack - v1.2                    |
 | - - - - - - - - - - - - - - - - - - - - - |
 | Copyright (c) 2005 by Antoni Sawicki      |
 | http://www.tenox.tc/out#mpk               |
 | - - - - - - - - - - - - - - - - - - - - - |
 | Uses base64 encoding routine 1.3          |
 | Copyright (c) 2001 by John Walker         |
 | http://www.fourmilab.ch/webtools/base64/  |
 |___________________________________________|
 |__________________________________________/
 |
 | This version of mpk is distributed under
 | terms and conditions of GPL.
 |
 | Note: on some system you need to include
 | libgen by adding -lgen or an equivalent.
 |
*/

#define BOUNDARY "==mpk==mpk==mpk==mpk==mpk=="
#define VERSION "1.2"


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

FILE *fi;

/*
** base64 stuff
*/
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

static int inbuf(void);
static int inchar(void);
static void ochar(int);
static void encode(void);


int main(int argc, char **argv) {
    int fn, bout;
    char inp[1024];
    struct stat fs;

    if(argc<2) {
        fprintf(stderr, "mpk version %s\n\n"
                        "Usage: mpk [-m] filename ...\n\n"
                        "mpk ecapsulates files in to a MIME 1.0 stream on stdout.\n"
                        "If -m flag is specified, a 7-bit ascii text message will\n"
                        "also be included from stdin.\n\n", VERSION);
        exit(1);
    }

    bout=0;

    /*
    ** mime header
    */
    fprintf(stdout, 
        "MIME-Version: 1.0\n"
        "Content-Type: multipart/mixed; boundary=\"%s\"\n"
        "\n"
        "This is a MIME encoded message.\n"
        "\n",
    BOUNDARY);

    /*
    ** handle input
    */
    for(fn=1; fn<argc; fn++) {

        /*
        ** text message from stdin
        */
        if(strcmp("-m", argv[fn])==0) {
            bout=1;
            fprintf(stdout, 
                "--%s\n"
                "Content-Type: text/plain\n"
                "\n",
            BOUNDARY);
            while(fgets(inp, 990, stdin))
                fprintf(stdout, "%s", inp);

            fprintf(stdout, "\n");
        } 

        /*
        ** attachment files
        */ 
        else if(stat(argv[fn], &fs)==0 && !(fs.st_mode & S_IFDIR)) {
            if(fi=fopen(argv[fn], "r")) {
                bout=1;
                fprintf(stdout, 
                    "--%s\n"
                    "Content-Type: application/octet-stream; name=\"%s\"\n"
                    "Content-Transfer-Encoding: base64\n"
                    "Content-Disposition: attachment; filename=\"%s\"\n"
                    "\n",
                BOUNDARY, basename(argv[fn]), basename(argv[fn]));

                encode();

                fprintf(stdout, "\n");
                fclose(fi);
            }
        }
    }

    /*
    ** last part
    */
    if(bout)
        fprintf(stdout, 
            "--%s--\n",
        BOUNDARY);

    return 0;
}


/*
** base64.c by John Walker 
*/

static int inbuf(void) {
    int             l;

    if (ateof) {
        return FALSE;
    }
    l = fread(iobuf, 1, MAXINLINE, fi);
    if (l <= 0) {
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
        fprintf(stdout, "\n");
        linelength = 0;
    }
    putc(((byte) c), stdout);
    linelength++;
}

static void encode(void) {
    int i, hiteof = FALSE;

    ateof = FALSE;
    iolen = 0;
    iocp = MAXINLINE;
    linelength = 0;

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
    fprintf(stdout, "\n");
}
