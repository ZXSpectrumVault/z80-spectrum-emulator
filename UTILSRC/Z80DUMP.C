/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 2.01, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                       Gerton Lunter, 3/5/93
*/

#include <stdio.h>

unsigned char buffer[55];
char ver,i;
FILE *fin;

int main(int,char**);

int main(teller,str)
int teller;
char **str;
{
    printf ("Z80DUMP - .Z80 file inspection utility - (c) 1993 G.A. Lunter - version 2.01\n");
    printf ("          This program may not be distributed.\n");
    printf ("          It is part of the registered Spectrum Emulator package.\n\n\n");
    if (teller!=2) {
	puts ("Syntax: Z80DUMP <file.z80>");
	return(1);
    }
    fin=fopen(str[1],"rb");
    if (fin==NULL) {
	for (i=0;str[1][i];i++) {}
	str[1][i++]='.';
	str[1][i++]='Z';
	str[1][i++]='8';
	str[1][i++]='0';
	str[1][i]=0;
	fin=fopen(str[1],"rb");
	str[1][i-4]=0;
    }
    if (fin==NULL) {
	printf ("File %s not found.\n",str[1]);
	return(1);
    }
    if (fread(buffer,1,55,fin)!=55) {
	printf ("Error while reading file %s.\n",str[1]);
	return(1);
    }
    ver=0;
    printf ("Header of file %s\n",str[1]);
    if ((buffer[6]==0)&&(buffer[7]==0)) {
	ver=1;
	buffer[6]=buffer[32];
	buffer[7]=buffer[33];
	if (((buffer[30]==21)||(buffer[30]==23))&&(buffer[31]==0))
		printf ("(Version 2.0 .Z80 file)\n\n");
	else { printf ("\nUnsupported version of .Z80 file, or .Z80 file corrupt\n\n");
	       return(1);
	}
     } else {
	printf ("(Version 1.45 (or earlier) .Z80 file)\n\n");
    }
    if ((unsigned char)buffer[12]==0xff) buffer[12]=1;
    if (ver==0) {
	printf ("Datacompression              ");
	if (buffer[12]&32)
	printf ("Active\n");
	else
	printf ("No compression\n");
	printf ("SamRom                       ");
	if (buffer[12]&16)
	printf ("Switched on\n");
	else
	printf ("Switched off\n");
    } else {
	printf ("Selected hardware            ");
	switch (buffer[34]) {
	    case 0: printf ("Normal Spectrum\n");
		    break;
	    case 1: printf ("Normal Spectrum + If.1\n");
		    printf("Interface 1 ROM              ");
		    if(buffer[36]) printf ("paged\n"); else printf ("inactive\n");
		    break;
	    case 2: printf ("Normal Spectrum + If.1 + SamRam\n");
		    printf("Interface 1 ROM              ");
		    if(buffer[36]) printf ("paged\n"); else printf ("inactive\n");
		    printf ("State of OUT-port 31         ");
		    for (i=0;i<8;i++) {
			printf ("%d",2*i+((buffer[35]>>i)&1));
			if (i!=7) printf (","); else printf ("\n");
		    }
		    break;
	    case 3:
	    case 4:printf ("Spectrum 128");
		    if (buffer[34]==4) {
			printf (" + If.1");
			printf("\nInterface 1 ROM              ");
			if(buffer[36]) printf ("paged"); else printf ("inactive");
		    }
		    printf ("\nState of OUT-port 7ffd       #%02x\n",buffer[35]);
		    printf ("State of OUT-port fffd       #%02x\n",buffer[38]);
		    printf ("Sound chip registers         ");
		    for (i=0;i<buffer[30]-7;i++) {	/* 14 or 16 */
			printf ("%02x",buffer[39+i]);
			if (i==buffer[30]-8) printf ("\n"); else printf (",");
		    }
		    break;
	}
	printf ("R-register emulation         ");
	if (buffer[37]&1) printf ("On\n"); else printf ("Off\n");
	printf ("Ldir emulation               ");
	if (buffer[37]&2) printf ("On\n"); else printf ("Off\n");
    }
    printf ("Issue 2 emulation            ");
    if (buffer[29]&4) printf ("On\n"); else printf ("Off\n");
    printf ("Double interrupt frequency   ");
    if (buffer[29]&8) printf ("On\n"); else printf ("Off\n");
    printf ("Video synchronisation        ");
    switch (buffer[29]&48) {
	case 16:printf ("High\n");
		break;
	case 48:printf ("Low\n");
		break;
	default:printf ("Normal\n");
    }
    printf ("Joystick emulation           ");
    switch (buffer[29]&192) {
	case 0:printf ("Cursor\n");
		break;
	case 64:printf ("Kempston\n");
		break;
	case 128:printf ("Sinclair 1\n");
		break;
	case 192:printf ("Sinclair 2\n");
    }
    printf ("Border colour                %d\n\n",(buffer[12]&14)>>1);
    printf ("Interrupt status      IM%d, ",buffer[29]&3);
    if (buffer[27]) printf ("EI"); else printf ("DI");
    printf (" (IFF2 = ");
    if (buffer[28]) printf ("EI)\n"); else printf ("DI)\n");
    printf ("Interrupt register    #%02x\n",buffer[10]);
    printf ("Refresh register      #%02x (bit 8 is %d)\n",(buffer[11]&0x7f)|((buffer[12]<<7)&0x80),buffer[12]&1);
    #define wordbuf(n) *((unsigned int*)(buffer+n))
    printf ("Program counter       #%04x                Stack pointer         #%04x\n",wordbuf(6),wordbuf(8));
    printf ("Register A, F         #%02x, #%02x             Register A', F'       #%02x, #%02x\n",buffer[0],buffer[1],buffer[21],buffer[22]);
    printf ("Register BC           #%04x                Register BC'          #%04x\n",wordbuf(2),wordbuf(15));
    printf ("Register DE           #%04x                Register DE'          #%04x\n",wordbuf(13),wordbuf(17));
    printf ("Register HL           #%04x                Register HL'          #%04x\n",wordbuf(4),wordbuf(19));
    printf ("Register IX           #%04x\n",wordbuf(25));
    printf ("Register IY           #%04x\n\n",wordbuf(23));
    return(0);
}
