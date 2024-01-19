/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 3.04, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                     Gerton Lunter, 10/8/96
*/

/* Tap2Voc - Spectrum emulator utility - 22/7/93, G.A. Lunter */


#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>


#define byte unsigned char
#define up 0xF0
#define down 0x10


void subext(char*,char*);


int taphandle,vochandle;
const byte vocheader[]={"Creative Voice File\032\032\000\012\001\051\021"};
const byte pause[]={"\003\003\000\000\000\020\014"};
const byte endmarker[]={"\000"};
int blockcount;
unsigned int blocklen,ipointer,opointer;
byte *blockbuf,*outbuf;
char filename[80];
int i,j,makeraw;
long len;


void subext(filnam,ext)
char *filnam,*ext;
{
    int i,j;
    for (i=j=0;filnam[i];i++) {
        filename[i]=filnam[i];
        if (filnam[i]=='.') j=i; else if (filnam[i]=='\\') j=0;
    }
    if (!j) j=i;
    if (j) for (i=0;ext[i];i++) filename[j++]=ext[i];
    filename[j]=0;
}


void main(argc,argv)
int argc;
char **argv;
{
    puts ("\nTAP2VOC - Converts .TAPs to .VOC Creative Voice files - v3.0");
    puts ("          This program may not be distributed.");
    puts ("          It is part of the registered Spectrum Emulator package.\n");
    if ((argc==1)||(argc>4)) {
       puts ("\nUsage: TAP2VOC  [-r]  inputfile[.TAP]  [outputfile[.VOC]]\n\n");
       puts ("This program converts blocks in a .TAP file to the corresponding sounds.");
       puts ("Using a sample file player, a .TAP file can be played back into a real");
       puts ("Spectrum again.  The .VOC file produced may also be fed back to the");
       puts ("emulator itself.\n");
       puts ("If you use -r, TAP2VOC will produce a raw sample file (extension .RAW).\n");
       puts ("The sample file created uses a sample frequency of 4090 Hz\n");
       exit(1);
    }
    blockbuf=(byte*)malloc(0xFF00);
    outbuf=(byte*)malloc(0xFF00);
    if (outbuf==NULL) {
       puts ("\007Sorry --- not enough memory.");
       exit(1);
    }
    i=1;
    makeraw=0;
    if ((argv[1][0]=='-')&&((argv[1][1]&0xDF)=='R')) {
       i++;
       makeraw=1;
    }
    if (makeraw) puts ("Making raw sample file.");
    subext(argv[i],".TAP");
    taphandle=open(filename,O_RDONLY|O_BINARY);
    if (taphandle==-1) {
       printf ("\007File %s could not be opened for reading.\n",filename);
       exit(1);
    }
    if (read(taphandle,&blocklen,2)!=2) {
       puts ("\007Empty .TAP file!  Won't create empty .VOC file!");
       exit(1);
    }
    if (!makeraw)
       subext(argv[argc-1],".VOC");
    else
        subext(argv[argc-1],".RAW");
    vochandle=open(filename,O_RDWR|O_CREAT|O_BINARY);
    if (vochandle==-1) vochandle=open(filename,O_RDWR|O_TRUNC|O_BINARY);
    if (vochandle==-1) {
       printf ("\007File %s could not be created.\n",filename);
       close(taphandle);
       exit(1);
    }
    if (!makeraw) if (write(vochandle,vocheader,0x1a)!=0x1a) {
       write_err:
       printf ("\007Error while writing %s.\n",filename);
       errexit:
       close(vochandle);
       close(taphandle);
       exit(1);
    }
    blockcount=0;
    len=0;
    do {
       if (blockcount++>0) {
          if (makeraw) {
             for (i=0;i<8192;i++) outbuf[i]=0;
             if (write(vochandle,outbuf,8192)!=8192) goto write_err;
             opointer=0;
          } else {
            if (write(vochandle,pause,7)!=7) goto write_err;
          }
          len+=4096;
       }
       if (read(taphandle,blockbuf,blocklen)!=blocklen) {
          printf ("\007Error in .TAP file reading block %d",blockcount);
          goto errexit;
       }
       printf ("Converting block %d...",blockcount);
       if (blockbuf[0]&0x80) {
          j=1612;               /* leader of data block (flag=FF) */
          len+=8060;
       } else {
           j=4032;              /* leader of header (flag=00) */
           len+=20160;
       }
       if (makeraw) {
          opointer=0;
       } else {
            outbuf[0]=1;             /* sound data */
            outbuf[4]=0x0c;          /* sample rate: once per 855 T states */
            outbuf[5]=0;             /* compression type: none */
            opointer=6;
       }
       for (i=0;i<j;i++) {
           outbuf[5*i+opointer]=outbuf[5*i+opointer+1]=outbuf[5*i+opointer+2]=down;
           outbuf[5*i+opointer+3]=outbuf[5*i+opointer+4]=up;
        }
        opointer+=5*j;
        outbuf[opointer++]=down;
        outbuf[opointer++]=up;   /* sync pulse */
        ipointer=0;
        while (blocklen) {
            i=blockbuf[ipointer++];
            blocklen--;
            j=8;
            len+=16;
            while (j--) {
                outbuf[opointer++]=down;
                if (i&128) outbuf[opointer++]=down;
                outbuf[opointer++]=up;
                if (i&128) {
                   outbuf[opointer++]=up;
                   len+=2;
                }
                i<<=1;
            }
            if ((opointer>0xF000)&&(blocklen>100)) {
                if (!makeraw) {
                   outbuf[1]=(opointer-4) & 0xFF;
                   outbuf[2]=(opointer-4) >> 8;
                   outbuf[3]=0;
                }
                if (write(vochandle,outbuf,opointer)!=opointer) {
                   printf ("\n");
                   goto write_err;
                }
                if (makeraw) {
                   opointer=0;
                } else {
                  outbuf[0]=2;        /* sound continue */
                  opointer=4;
                }
            }
        }
        outbuf[opointer++]=down;
        outbuf[opointer++]=down;
        outbuf[opointer++]=down;
        printf ("Done.\n");
        if (!makeraw) {
           outbuf[1]=(opointer-4) & 0xFF;
           outbuf[2]=(opointer-4) >> 8;
           outbuf[3]=0;
        }
        if (write(vochandle,outbuf,opointer)!=opointer) goto write_err;
        i=read(taphandle,&blocklen,2);
    } while (i==2);
    write(vochandle,endmarker,1);
    close (taphandle);
    if (close (vochandle)) goto write_err;
    printf ("\nFile successfully converted.\nLength sample file: %3ld min %02ld sec\n",len/245880L,(len/4098)%60);
    return;
}

