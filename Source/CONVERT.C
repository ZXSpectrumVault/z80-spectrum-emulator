/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 3.04, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                     Gerton Lunter, 10/8/96
*/

/* Convert -- utility bij de Spectrum Emulator */

#include <stdio.h>
#include <math.h>
#include <alloc.h>
#include <dos.h>
#include <string.h>

typedef unsigned char byte;

#define inb inb_p
#define outb outb_p
#define findkeyword FINDKEYWORD
#define listchar LISTCHAR

char	*inb_p,*outb_p,mode;
char	line[1024],nextline[1024],outline[4096],fnam[80];
int n,b,xinp,inpt,xoutp,linec,bufsize,linen,byteswritten;
FILE	*inf,*outf,*outf2;
const char  gif_header[]={'G','I','F','8','7','a',0,1,192,0,0xB3,0,0,
                 0,0,0, 0,0,162, 223,0,0, 231,0,182, 0,215,0, 0,215,215, 210,210,0, 202,202,202,
                 0,0,0, 0,0,174, 243,0,0, 255,0,223, 0,243,0, 0,255,255, 255,255,0, 255,255,255,
                 ',',0,0,0,0,0,1,192,0,0,4};
#define ghlen 72
const char  pcx_header[128]={10,3,1,1,0,0,0,0,0xFF,0,0xBF,0,0,1,0xC0,0,
/*                 0,0,0, 0,0,0xaa, 0,0xaa,0, 0,0xaa,0xaa, 0xaa,0,0, 0xaa,0,0xaa, 0xaa,0xaa,0, 0xaa,0xaa,0xaa,
                 0x55,0x55,0x55, 0x55,0x55,0xff, 0x55,0xff,0x55, 0x55,0xff,0xff, 0xff,0x55,0x55, 0xff,0x55,0xff, 0xff,0xff,0x55, 0xff,0xff,0xff, */
                 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
                 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
                 0,4,32};
#define phlen 128

const char	gif_tail[]={0,';'};
#define gtlen 2

int 	main(int argc,char **argv);
int	readline(void);
int	convertline(void);
int     writeline(void);
int	transfer(char **in,char **out);
int	getbyte(void);
int	writebyte(char byt);
int	flushit(void);
void	helpscreen(void);
void	insertint(char *string,unsigned int num,char sign);
void	insertfloat(char *string,unsigned char exponent,unsigned long mantisse);
void	insert(char *string,byte b1,byte b2,byte b3,byte b4,byte b5);
void    findkeyword (char **xinp,int *k);
void	writlin (void);
void	PROCESS_SCREEN(char *screen, char *buf);
int	LZW (char *table, char *output, char *data, int handle);
int PCX (char *outputb,char *data,int handle);
int	EXPAND (char *data);
int	clmode2 (int);
int	listchar (char **in,char **out);

void helpscreen()
{
	puts ("Syntax:  CONVERT <inmode> <outmode> inputfile [outputfile]\n");
    puts ("Inmode:   A: Ascii --- pure text");
    puts ("          B: Bytes --- pure code or screen data");
    puts ("          S: Save *\"b\" format --- code, screen data or BASIC\n");
    puts ("Outmode:  C: Ascii, with CR's as line-breaks (Spectrum standard) [.TXT]");
    puts ("          L: Ascii, with CR/LF (PC standard) [.TXT]");
    puts ("          B: Bytes --- pure code or screen data [.BIN]");
    puts ("          S: Save *\"b\" format --- 'bytes' (code) [.SAV]");
    puts ("          P: Save *\"b\" format --- 'program' (BASIC) [.SAV]");
    puts ("          G: GIF picture format (Graphics Interchange Format) [.GIF]");
    puts ("          X: PCX picture format [.PCX]\n");
    puts ("Examples:  CONVERT b x screen.scr screen.pcx");
    puts ("           CONVERT a l output output.txt\n");
    puts ("If no output filename is given, the input filename is taken with one of above");
    puts ("default extensions, depending on the outputmode.");
}

int getbyte()
{
    if (xinp==inpt) {
        xinp=0;
		inpt=fread (inb,1,bufsize,inf);
		if (inpt==0) return (-1); /* eof */
	}
    if (xinp>=bufsize) xinp=0;
    return ((int)((unsigned char)inb[xinp++]));
}

int flushit()
{
    if (xoutp==0) return (1);
    if (fwrite(outb,1,xoutp,outf)!=xoutp) return (0);
    xoutp=0;
	return (1);
}

int writebyte(byt)
char byt;
{
    outb[xoutp++]=byt;
    if (xoutp>=bufsize) if (!flushit()) return (0);
	return (1);
}

int writeline()
{
	int x,y;
	for (y=4;outline[y]!=13;y++) if (outline[y]==14) y+=5;
	for (x=0;x<y;x++) if (!writebyte(outline[x])) return (0);
	return (writebyte(13));
}

int readline()
{
	int b,p=0,pa,c,k,q;
	char newline[1024],*keylist;
	if (nextline[0]!=0) {
		for (p=0;nextline[p]!=0;p++) line[p]=nextline[p];
		line[p]=nextline[0]=0;
	} else {
		do {
			b=getbyte();
			if (b==-1) return (0);
		}
		while (((b==0x0D)||(b==0x0A)||(b==0x00)||(b==0x1A)));
		do {
			line[p++]=b;
			line[p]=0;
			b=getbyte();
		} while ( (b!=-1)&&(b!=10)&&(b!=13)&&(b!=0)&&(b!=26)&&(p<1023) );
	}
	if (p==1023) return (0);
	if (p==80) do {
		pa=newline[0]=0;
		do {
			b=getbyte();
			if (b==-1) {
				newline[0]=0;
				goto nostart;
			}
		}
		while (((b==0x0D)||(b==0x0A)||(b==0x00)||(b==0x1A)));
		do {
			newline[pa++]=b;
			newline[pa]=0;
			b=getbyte();
		} while ( (b!=-1)&&(b!=10)&&(b!=13)&&(b!=0)&&(b!=26)&&(pa<1023) );
		if (pa==1023) return (0);
		for (b=0;newline[b]==0x20;b++){}
		if (b>3) goto nostart;
		c=0;q=b;
		while ((newline[b]>='0')&&(newline[b]<='9')) {
			if (c>3276) goto nostart;
			c=10*c+newline[b++]-'0';
		}
		if ((c<=linen)||(b==q)) goto nostart;
		for (;newline[b]==0x20;b++){}
		if ((newline[b]<='A')||(newline[b]>='Z')) goto nostart;
		keylist=newline+b;
		findkeyword(&keylist,&k);
		if ((k>=206)&&(k<=255)) {
			for (b=0;newline[b]!=0;b++) nextline[b]=newline[b];
			nextline[b]=0;
			goto exitreadline;
		}
nostart:
		for (b=0;newline[b]!=0;b++,p++) {
			line[p]=newline[b];
			if (p>=1020) return (0);
		}
		line[p]=0;
	} while (b==80);
exitreadline:
	for (b=0;line[b]==0x20;b++) {}
	for (c=0;c<=p-b;c++) line[c]=line[c+b];
	line[c]=0;
	if (p>1020) return (0);
	return (1);
}

int transfer(in,out)
char **in,**out;
{
    char *xinp;
	int k;
	if (**in=='"') {
		**out='"';
		do {
			*(++(*out))=*(++(*in));
		} while ((**in!='"')&&(**in!='\0'));
		if (**in=='\0') return (0);
		++(*in);
		++(*out);
		return (1);
	} else if (**in==' ') {
		++(*in);
		return (1);
	} else if ((**in=='\0')||(**in==0x0D)||(**in==0x0A)) return (1);
    xinp=*in;
    findkeyword(&xinp,&k);
	if (k!=0) {
		*((*out)++)=k;
        *in=xinp;
		if (k==234) {
			(*in)++;
			while (**in!='\0') *((*out)++)=*((*in)++);
		}
		return (1);
	}
	*((*out)++)=*((*in)++);
	return (1);
}

void insertint(string,num,sign)
char *string;
unsigned int num;
char sign;
{
	insert (string,0,(0xFF*(sign<0)),num & 0xFF,num >> 8,0);
}

void insertfloat(string,exponent,mantisse)
char *string;
unsigned char exponent;
unsigned long mantisse;
{
	insert (string,exponent,mantisse >> 24,mantisse >> 16,mantisse >> 8,mantisse);
}

void insert (string,b1,b2,b3,b4,b5)
char *string;
unsigned char b1,b2,b3,b4,b5;
{
	int i,j;
	for (i=0;string[i]!=13;i++) {};
	for (j=i+6;j>=6;j--) string[j]=string[j-6];
	string[0]=14;
	string[1]=b1;
	string[2]=b2;
	string[3]=b3;
	string[4]=b4;
	string[5]=b5;
}

int convertline()
{
	char *inlp=line,*outlp=outline;
	int l,i,j;
	double f,g;
	unsigned long mantisse;
	while (*inlp!=0) {
		if (!transfer (&inlp,&outlp)) return (0);
	}
	*outlp=13;
	i=l=0;
	while ((outline[i]>='0')&&(outline[i]<='9')) {
		if (l>3277) return (0);
		l=10*l+outline[i++]-'0';
	}
	if ((linen>=l)||(i==0)) return (0);
	linen=l;
	for (j=0;outline[i]!=13;j++,i++) outline[j]=outline[i];
	i=j+1;
	outline[j]=13;
	for (j=j+4;j>=4;j--) outline[j]=outline[j-4];
	outline[0]=(l>>8);
	outline[1]=l;
	for (i=4;outline[i]!=13;) {
		if (outline[i]==(signed char)206) {
			while (outline[i++]!='(') if ((outline[i]==')')||(outline[i]==13)) return (0);
			while (outline[i]!=')') {
				i++;
				while ( (outline[i]!=',')&&(outline[i]!=')')&&(outline[i]!=13) ) i++;
				if (outline[i]==13) return (0);
				insertint (outline+i,0,1);
				i+=6;
			}
		} else if (outline[i]==(signed char)196) {
			i++;
			l=0;
			while ((outline[i]==' ')||(outline[i]=='1')||(outline[i]=='0')) {
				if (outline[i]!=' ') l+=(l+outline[i]-'0');
				i++;
			}
			insertint (outline+i,(unsigned)l,1);
			i+=6;
		} else if (((outline[i]>='0')&&(outline[i]<='9'))||(outline[i]=='.')) {
			f=0.0;
			j=1;
			while ((outline[i]>='0')&&(outline[i]<='9')) {f=10.0*f+(outline[i++]-'0');}
			if (outline[i]=='.') {
				i++;
				g=.1;
				j=2;		/* 2, flag voor non_integer */
				while ((outline[i]>='0')&&(outline[i]<='9')) {
					f=f+g*(outline[i++]-'0');
					g=g/10;
				}
			}
			if ((outline[i]=='e')||(outline[i]=='E')) {
				i++;
				if (outline[i]=='-') i++;
				if (outline[i]=='+') i++;
				l=0;
				if ((j==1)||(j==-1)) j*=2;
				while ((outline[i]>='0')&&(outline[i]<='9')) {
					l=10*l+outline[i++]-'0';
					if (outline[i-2]=='-') l=-l;
				}
				f=f*pow10(l);
			}
			if ((j==1)&&(f<65535L)) {
				insertint (outline+i,(unsigned int)f,j);
				i+=6;
			}
			else {
				f=frexp(fabs(f),&l);
				mantisse=((unsigned long)ldexp(f,32))&0x7FFFFFFFL;
				if ((l<-127)||(l>127)) {
					l=0x80;
					mantisse=0L;
				}
				l+=0x80;
				insertfloat (outline+i,(unsigned char)l,mantisse);
				i+=6;
			}
		} else if (outline[i]=='"') {
			do {i++;} while (outline[i]!='"');
			i++;
		} else if (outline[i]==(signed char)196) {
			do {i++;} while (outline[i]!=13);
		} else i++;
	}
	for (i=4;outline[i]!=13;i++) if (outline[i]==14) i+=5;
	i-=3;
	outline[2]=(i & 0xFF);
	outline[3]=(i>>8);
	return (1);
}

void writlin()
{
    if (linen!=-1) printf ("\rLine %u:",linen); else printf ("\rFirst line:");
}

int clmode2(mode)
int mode;
{
	int a,b,f=1,p=0;
	char *pin,*pout;
	a=getbyte();
	if ((a==-1)||(a>=64)) {
		inpt=0;
		return (0);
	}
	b=getbyte();
	if (b==-1) goto syntaxerr;
	a=256*a+b;
	b=1000;
	while (b!=0) {
		outline[p]='0'+(a/b);
		if (f && (outline[p]=='0') ) outline[p]=' '; else f=0;
		a%=b;
		b/=10;
		if (b==1) f=0;
		p++;
	}
	b=getbyte();
	if (b==-1) goto syntaxerr;
	b=getbyte();
	if (b==-1) goto syntaxerr;
	b=0;
	do {
		if (b==-1) b=0;
		a=getbyte();
		if (a==-1) {
syntaxerr:
            puts ("Error in inputfile: Format Error");
			return (0);
		}
		if (b==0) {
			if (a==0x0E) b=6;
			else
			if ((a>=16)&&(a<=23)) {
				if (a>=22) b=3; else b=2;
			}
			else
			outline[p++]=a;
		}
	}
	while (((b--)!=0)||(a!=0x0D));
	pin=outline;
	pout=line;
	do {} while ((listchar(&pin,&pout)==0)&&(pout<=line+1024));
	if (mode==1) pout--;
	if (pout>=line+1024) goto syntaxerr;
	pin=line;
	do {
		if (!writebyte(*(pin++))) {
	    puts ("Error during writing outputfile");
			return (0);
		}
	} while (pin<pout);
	return (1);
}

int main(argc,argv)
char **argv;
int argc;
{
	char cntlen=0,translat=0,inmod=0,outmod=0,cnt,x;
	char *y;
    puts ("Convert - Conversion utility Spectrum-PC - (c) 1994 G.A. Lunter - version 3.0");
	puts ("          This program may not be distributed.");
	puts ("          It is part of the registered Spectrum Emulator package.\n");
	if (argc<3) {
		if (argc==1) helpscreen(); else
		badcmdlin:
	puts ("Error in command line");
		return (1);
	}
	cnt=1;x=0;
	while (inmod==0) {
		if ((argv[cnt][x]=='/')||(argv[cnt][x]=='-')) x++;
		if (argv[cnt][x]!=0) {
			inmod=outmod;
			outmod=argv[cnt][x]|0x20;
			x++;
		}
		if (argv[cnt][x]==0) {
			x=0;
			cnt++;
		}
	}
	if (x!=0) {
		badmode:
        puts ("Syntax error or unknown option specified");
		return (1);
	}
    if ((inmod!='a')&&(inmod!='b')&&(inmod!='s')) goto badmode;
    if ((outmod!='l')&&(outmod!='c')&&(outmod!='s')&&(outmod!='p')&&(outmod!='g')&&(outmod!='x')&&(outmod!='b')) goto badmode;
	if (cnt>=argc) goto badcmdlin;
	inf=fopen (argv[cnt++],"rb");
	if (inf==0) {
        printf ("Error during opening inputfile %s\n",argv[cnt-1]);
        return (1);
	}
	if (cnt==argc) {
		strcpy (fnam,argv[cnt-1]);
		for (x=0;fnam[x];x++) {}
		for (;(fnam[x]!='.')&&(fnam[x]!='\\')&&(x!=0);x--) {}
		if (fnam[x]!='.') for (;fnam[x];x++) {}
		switch (outmod) {
		case 'c':
		case 'l':y=(".TXT");break;
		case 's':
		case 'p':y=(".SAV");break;
        case 'x':y=(".PCX");break;
		case 'g':y=(".GIF");break;
		case 'b':y=(".BIN");
		}
		strcpy(fnam+x,y);
	} else strcpy(fnam,argv[cnt++]);
	if (cnt!=argc) goto badcmdlin;
	outb_p=(char *)malloc(0x7000);
	inb_p=(char *)malloc(0x7000);
	bufsize=0x7000;
    xinp=inpt=xoutp=nextline[0]=linec=0;
	linen=-1;
	mode=3;
	if (inmod=='s') {
		if (fread (inb_p,1,9,inf)!=9) {
			inferr:
            puts ("Error in inputfile");
			goto err;
		}
	}
	switch (outmod) {
	case 'c':
	case 'l':
		if (outmod=='c') translat=1; else translat=2;
		if (inmod=='b') {
            puts ("Can't convert Bytes to Ascii");
			err:
			fclose (inf);
			fclose (outf);
            return (1);
		}
		if (inmod=='s') {
			if (inb_p[0]!=0) goto inferr;
			mode=1;
		}
		break;
	case 's':
		cntlen=3;
		translat=0;
		if (inmod=='s') {
			caseS:
			if (inb_p[0]==0)
                puts ("Can't convert BASIC to Bytes");
			else
                puts ("No conversion needed");
			goto err;
		}
		break;
	case 'p':
		if (inmod=='a') {
			mode=2;
			cntlen=1;
		} else if (inmod=='b') {
            puts ("Can't convert Bytes to BASIC");
			goto err;
		} else goto caseS;
		break;
	case 'g':
    case 'x':
		if (inmod=='a') {
            puts ("Can't convert Ascii to GIF or PCX");
			goto err;
		}
		if (inmod=='s') {
			if (inb_p[0]!=3) {
                puts ("Can't convert BASIC to GIF or PCX");
				goto err;
			}
		}
		mode=4;
		break;
	case 'b':
		if (inmod=='a') {
            puts ("Can't convert Ascii to Bytes");
			goto err;
		}
		if (inmod=='b') {
            puts ("No conversion needed");
			goto err;
		}
		translat=0;
		break;
	}
	outf=fopen (fnam,"wb");
	if (outf==0) {
        printf ("Error during opening outputfile %s\n",fnam);
		fclose(inf);
        return (1);
	}
	if (cntlen&1) {
		line[0]=0;
		line[3]=0x05;
		line[4]=0x5D;
		line[7]=0xFF;
		line[8]=0xFF;
		if (cntlen&2) {
			line[0]=3;
			line[3]=0xA8;
			line[4]=0x61;
		}
		for (n=0;n<9;n++) if (fputc(line[n],outf)==EOF) {
            puts ("Error during writing to outputfile");
			fclose (inf);
			fclose (outf);
            return (1);
		}
	}
	switch (mode) {
	case 1:
	/******************************************************
	List: Omzetten BASIC in ASCII
	*******************************************************/

		do {} while (clmode2(translat));
		if (inpt!=0) goto exiterr;
		break;

	case 2:
	/************************************************
	Convert: Omzetten ASCII in BASIC
	*************************************************/
		while (1) {
			if (!readline()) {
				if (inpt==0) goto end_of_file;
				flushit();
				writlin();
                puts ("\nUnexpected end of inputfile");
				goto exiterr;
			}
			if (!convertline()) {
				writlin();
                puts ("\nError in inputfile - BASIC syntax error");
				goto exiterr;
			}
			if (!writeline()) goto wderr;
            if ((0x0F & linec++)==0) printf ("\rConverting: Line %u",linen);
		}
	end_of_file:
        printf ("\rConverting: Line %u, finished.\n",linen);
		if (!flushit()) {
	wderr:
            puts ("\nError during writing");
	exiterr:
			flushit();
			fclose(outf);
			fclose(inf);
            return (1);
		}
		break;

	case 3:
	/******************************************************
	Double: Omzetten CR in CR/LF in ASCII
	*******************************************************/
		do {
			n=getbyte();
			if (n!=-1)
				if ((translat!=1)||(n!=0x0A))
					if (!writebyte(n)) goto wderr;
			if (translat==2) while (n==0x0D) {
				n=getbyte();
				if (n!=0x0A) if (!writebyte(0x0A)) goto wderr;
				if (!writebyte(n)) goto wderr;
			}
		} while (n!=-1);
		break;

	case 4:
	/******************************************************
	Graphics: Omzetten Spectrum scherm in .GIF formaat
	*******************************************************/
		free (inb_p);
		free (outb_p);
		if ( ( (outb_p=(char *)malloc(0x8010))==NULL)||( (inb_p=(char *)malloc(0xC080))==NULL) ) {
            puts ("Insufficient memory available.");
			goto exiterr;
		}
        n=fread(outb_p,1,6913,inf);
        if (n!=6912) {
            puts ("Unknown screen format, file should be 6912 bytes long");
			goto exiterr;
        }
        if (outmod=='g') {
            if (fwrite(gif_header,1,ghlen,outf)!=ghlen) goto wderr;
        } else {
            if (fwrite(pcx_header,1,phlen,outf)!=phlen) goto wderr;
        }
		if (fflush(outf)==EOF) goto wderr;
		PROCESS_SCREEN(outb_p,inb_p);
        if (outmod=='g') {
            for (n=0;n<4096;n++) outline[n]=0;
            if (LZW(outb_p,outline,inb_p,(outf-stdin))!=0) goto wderr;
            if (fwrite(gif_tail,1,gtlen,outf)!=gtlen) goto wderr;
        } else {
            PCX(outb_p,inb_p,(outf-stdin));
        }
		break;

    }
    if (!flushit()) goto wderr;
    if (cntlen&1) {
        byteswritten=(int)ftell(outf);
        if (byteswritten==-1) goto wderr;
        byteswritten-=9;
        if (fseek (outf,1L,SEEK_SET)) goto wderr;
        putc (byteswritten & 0xFF,outf);
		putc (byteswritten >> 8,outf);
		if (fseek (outf,5L,SEEK_SET)) goto wderr;
		putc (byteswritten & 0xFF,outf);
		putc (byteswritten >> 8,outf);
		if (fseek (outf,0L,SEEK_END)) goto wderr;
	}
	if (fclose(outf)==EOF) goto wderr;
	fclose (inf);
    return (0);
}
