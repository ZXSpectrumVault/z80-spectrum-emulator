// readsb.cpp
// Large parts taken from play_rec.cpp by Christopher M. Box (1993)
// Memory allocation code was drawn from the SBF DACDMA program
// This program may be changed/distributed/re-used freely, as long as
//  credits are given in the program and/or the documentation to the
//  original author, Christopher M. Box

#define DMA_BUF_SIZE 32000U
#define ALLOCATE ((2*DMA_BUF_SIZE) + 65536L)



#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <dos.h>
#include <io.h>

#include "sndclass.h"

const byte vocheader[]={"Creative Voice File\032\032\000\012\001\051\021"};
char digital=0;
char oversample=0;
char filter=0;
char play=0;
char raw=0;
int SbIRQ=7;
int SbDMAChan=1;
int FCutOff=3333;               // max freq handled by binary filter div'd by 1.5
long samplesstored=0;
byte far *outputbuffer;

void addextension(char *fnam,char *ext)
{
	 int i=0,f=0;
	 while (fnam[i]) {
		  if (fnam[i]=='\\') f=0;
		  if (fnam[i]=='.') f=i;
		  i++;
	 }
	 if (f) i=f;
	 f=0;
	 do {fnam[i++]=ext[f];} while (ext[f++]);
}

unsigned int process_buffer(byte far *input, byte far *output, unsigned len, unsigned rate)
// Takes as input sample buffer, length, settings of digital, oversample, filter
//  and the routine's own static variables.  Produces in output buffer the data to
//  be written.
{
#define MinWindowSize 5                 // smallest max/min window used
#define MaxWindowSize 31                // largest
#define FFollow 400
	 static byte signal[MaxWindowSize];
	 static byte oversampbuf[8];
     static byte curstate=0;             // to signal: first entry
     static byte curextr=0x80;
	 static int oversampkernel[8];
	 static long alpha;
	 static long average=0;
	 unsigned int outp=0;
	 unsigned int inp=0;
	 int kernellen;
	 int i;
	 long result;
	 byte minb,maxb,b,inpb;
	 float x;


	 kernellen = ((FCutOff/2)+(oversample ? 2*rate : rate)) / FCutOff;
	 kernellen = max(min(kernellen,MaxWindowSize),MinWindowSize);
	 kernellen |= 1;

	 if (!curstate) {
		  for (i=0;i<MaxWindowSize;i++) signal[i]=128;
		  for (i=0;i<8;i++) oversampbuf[i]=128;
		  curstate = 0x20;
		  result=0;
		  for (i=0;i<8;i++) {
				x=(i-3.5)*M_PI;
				result+=256*exp(-0.015*x*x)*(sin(x)/x);
		  }
		  for (i=0;i<8;i++) {
				x=(i-3.5)*M_PI;
				oversampkernel[i]=65536*exp(-0.015*x*x)*(sin(x)/x)/result;
		  }
		  alpha = 256/(1+rate/FFollow);
	 }

	 while (inp<len) {

		  average += alpha * (((long)input[inp]-0x80) - (average+0x80)>>8);

		  if (oversample)
				inpb=oversampbuf[4];
		  else
				inpb=input[inp];

		  if (digital) {
				maxb=minb=inpb;
				for (i=0;i<kernellen-1;i++) {
					 if ((b=signal[i]=signal[i+1])>maxb) maxb=b;
					 if (b<minb) minb=b;
				}
				signal[i]=inpb;
				if ((signal[kernellen/2]==maxb)&&(curstate==0xE0))
					if (maxb>(3*(average>>8)+curextr+0x180)/4) {curstate=0x20;curextr=maxb;}
				if ((signal[kernellen/2]==minb)&&(curstate==0x20))
					if (minb<(3*(average>>8)+curextr+0x180)/4) {curstate=0xE0;curextr=minb;}
				output[outp++]=curstate;
		  } else {
				output[outp++]=inpb;
		  }

		  if (oversample) {
				i=result=0;
				for (i=0;i<7;i++) result+=
                     ((long)oversampkernel[i])*(oversampbuf[i]=oversampbuf[i+1]);
                result+=((long)oversampkernel[7])*(oversampbuf[7]=input[inp]);
                inpb=min(255L,max(0L,result/256));
				if (digital) {
					 maxb=minb=inpb;
					 for (i=0;i<kernellen-1;i++) {
						  if ((b=signal[i]=signal[i+1])>maxb) maxb=b;
						  if (b<minb) minb=b;
					 }
					 signal[i]=inpb;
					 if ((signal[kernellen/2]==maxb)&&(curstate==0xE0))
						if (maxb>(3*(average>>8)+curextr+0x180)/4) {curstate=0x20;curextr=maxb;}
					 if ((signal[kernellen/2]==minb)&&(curstate==0x20))
						if (minb<(3*(average>>8)+curextr+0x180)/4) {curstate=0xE0;curextr=minb;}
					 output[outp++]=curstate;
				} else {
					 output[outp++]=inpb;
				}
		  }

		  inp++;

	 }

	 return(outp);
}


void inc_arg(int &argc, char **&argv) {
	++argv;
	if (--argc < 1) {
        printf("READSB  [switches]  sample_file[.VOC]\n\n");
        printf("-d     Produce filtered digital sample file\n");
        printf("-n     Produce filtered digital sample for normal loading speed\n");
        printf("-2     Two times oversampling\n");
        printf("-s N   Set sample rate to N Hz (default is 10000)\n");
        printf("-i N   Set IRQ to N (default 7)\n");
        printf("-a N   Set I/O address to N (default 0220)\n");
        printf("-c N   Set DMA channel to N (default 1)\n");
        printf("-r     Produce raw sample file\n\n");
        printf("Use -n for samples of normal program tapes.\n");
        printf("Use -d for turbo loaders.\n");
        printf("Some sound cards do not support rates over 12 kHz.\n");
        printf("Using -2 without -d or -n is most often silly.\n");
		exit(1);
	}
}

int main(int argc, char **argv) {
	unsigned sr=10000;         // Default sample rate
    char filename[128];

    printf("READSB v3.04 - reads .VOC sample from SoundBlaster - (c) 1996 Gerton Lunter\n");
    printf("               Based on the PLAY_REC program by Christopher M. Box (1993)\n");
    printf("               This program is public domain.\n\n");

	inc_arg(argc, argv);

    char switched=1;

    while (switched) {
        switched=0;

        if (! strncmp(*argv, "-s", 2)) {
            if (strlen(*argv)==2) {
                inc_arg(argc, argv);
                sr = atoi(*argv);        // Works for unsigned ints too
            } else
                sr = atoi(*argv+2);
            if (sr<4000) sr=4000;
            inc_arg(argc, argv);
            switched=1;
        }

        if (! strcmp(*argv, "-d")) {
            inc_arg(argc, argv);
            digital=1;
            switched=1;
        }

        if (! strcmp(*argv, "-2")) {
            inc_arg(argc, argv);
            oversample=1;
            switched=1;
        }

        if (! strncmp(*argv, "-i", 2)) {
            if (strlen(*argv)==2) {
                inc_arg(argc, argv);
                SbIRQ = atoi(*argv);
            } else
                SbIRQ = atoi(*argv+2);
            inc_arg(argc, argv);
            printf ("SB IRQ set to %d\n",SbIRQ);
            switched=1;
        }

        if (! strncmp(*argv, "-a", 2)) {
            if (strlen(*argv)==2) {
                inc_arg(argc, argv);
                SbIOaddr = ((atoi(*argv)-200)/5)*8 + 0x200;
            } else
                SbIOaddr = ((atoi(*argv+2)-200)/5)*8 + 0x200;
            inc_arg(argc, argv);
            printf ("SB IO address set to 0x%x\n",SbIOaddr);
            switched=1;
        }

        if (! strncmp(*argv, "-c", 2)) {
            if (strlen(*argv)==2) {
                inc_arg(argc, argv);
                SbDMAChan = atoi(*argv);
            } else
                SbDMAChan = atoi(*argv+2);
            inc_arg(argc, argv);
            printf ("SB DMA channel set to %d\n",SbDMAChan);
            switched=1;
        }

        if (! strcmp(*argv, "-p")) {
            inc_arg(argc, argv);
            play=1;
            switched=1;
            printf ("Playing instead of recording...\n");
        }

        if (! strcmp(*argv, "-r")) {
            inc_arg(argc, argv);
            raw=1;
            switched=1;
            printf ("Making a raw sample file.\n");
        }

        if (! strcmp(*argv, "-n")) {
            inc_arg(argc, argv);
            digital=1;
            FCutOff = 2000;
            switched=1;
        }

    }

    if (argc!=1) {
        argc=1;
        inc_arg(argc, argv);
    }

    SbDevice *sdev;
	sdev = new SbDevice;	     // This could also be created statically
    if (! sdev -> install_ok()) {
        printf ("Error: Could not initialise sound blaster!\n");
        exit(1);
    }

// Open/create the file.

    int handle;
    strcpy(filename,*argv);
    if (raw)
        addextension(filename,".RAW");
    else
        addextension(filename,".VOC");
    if (play)
        handle = _open(filename,O_RDONLY);
    else
        handle = _creat(filename, 0);
	if (handle < 0) {
        printf("Error: Could not open sample file '%s'\n",filename);
		exit(1);
	}
    if (!play && !raw) {
        if (_write(handle,vocheader,0x1a) != 0x1a) {
            printf ("Error: Could not write to file; disk full?\n");
            exit(1);
        }
    }

// Obtain an aligned 64K memory buffer for the DMA functions

	byte far *raw = (byte far *) farmalloc(ALLOCATE);
	if (! raw) {
        printf("Error: not enough memory available - an extra %uK needed.\n",
			((unsigned int)(ALLOCATE-farcoreleft()))/1024+1);
		exit(1);
	}
	long physical = ((long)FP_OFF(raw)) + (((long)FP_SEG(raw)) << 4);
	long aligned_physical = (physical+0x0FFFFL) & 0xF0000L;
	byte far *buf = (byte far *)
		MK_FP((unsigned )((aligned_physical >> 4) & 0xFFFF),0);

    outputbuffer = (byte far *) farmalloc(2 * DMA_BUF_SIZE);
    if (!outputbuffer) {
        printf ("Error: not enough memory available.\n");
        exit(1);
    }

    sdev -> set_rate(sr,RECORD);
    printf("Recording sample at %u Hz\n", sdev -> get_rate());
    printf("\nPress any key to start recording:\n");
    sdev -> file_dma(handle,buf,DMA_BUF_SIZE,0,play ? PLAY : RECORD);
    char dummy=0;
    if (!play) {
        _write(handle,&dummy,1);
    }
	_close(handle);
	farfree(raw);
    farfree(outputbuffer);
    delete sdev;
    cprintf("\r\nDone.\r\n");
	return 0;
}
