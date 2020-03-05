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
#include <stdlib.h>
#include <mem.h>
#include <alloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>

struct z80header {
    char areg,freg;
    int bcreg,hlreg,pcreg,spreg;
    char ireg,rreg;
    unsigned char flagbyte;
    int dereg,bcareg,deareg,hlareg;
    char aareg,fareg;
    int iyreg,ixreg;
    char iff1,iff2,imode;

    int length,pcreg2;
    char hmode,outstate,if1paged,flagbyte2,bytefffd;
    char soundreg[16];
};

struct blockheader {
       unsigned int length;
       char page;
};


struct tapeh {
       char type;
       char fnam[10];
       int length;
       char b0,b1,b2,b3;
};

const unsigned char loadline1[]={0xd2,0,0,1,0xfd,0x7f,0x3e,0x8f,0x2e,0,0x3b,0xed,0x69,0x80,0x06,0xbf,
      0xed,0x79,6,0xff,0xe1,0x38,0xf3,0x3e,0,0xd3,0xfe,0xc1,0xd1,0xc3,0xe0,0x56};
const unsigned char loadline2[]={0xd9,1,0,0,0x11,0,0,0x21,0,0,0xdd,0x21,0,0,0xfd,0xe1,
      0x3e,0,0xed,0x4f,0xf1,8,0xf1,0x31,0,0,0xed,0x56,0xfb,0xc3,0,0};

char filename[128];
int fin,fout;
struct z80header header;
struct blockheader block;
struct tapeh tapehdr;
unsigned char *specmem[8],*temp;

extern const char far LOADER;

int main(int,char**);
int unpack(unsigned char*,unsigned char*,unsigned int);
int pack(unsigned char*);
void subext(char*,char*);
void writetap(unsigned char*,unsigned char,unsigned int);



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


int unpack(inp,outp,size)
unsigned char *inp,*outp;
unsigned int size;
{
    register unsigned int incount=0,outcount=0;
    unsigned int i;
    char j;
    do {
       if ((inp[0]==0xED)&&(inp[1]==0xED)) {
            i=inp[2];
            j=inp[3];
            inp+=4;
            incount+=4;
            outcount+=i;
            for (;i!=0;i--) *(outp++)=j;
       } else {
         *(outp++)=*(inp++);
         incount++;
         outcount++;
       }
    } while (outcount<size);
    if (outcount!=size) incount=0;
    return (incount);
}


int pack(inp)
unsigned char *inp;
{
    unsigned *hash,i,j,j0,j00,thishv,input,output,output0,bits;
    unsigned bestpos,bestlen;
    register unsigned tmp;
    #define hv(k) (tmp=*(unsigned*)(inp+k),16384+(((tmp>>6)^tmp)&4095))
    #define putbit(b) {if ((bits+=(bits+(b)))&0x100) \
            {temp[output0]=bits;output0=output++;bits=1;}}
    #define putbits(x,n) {unsigned p,q=((x)<<(15-n)); \
            for (p=n;p;p--) putbit((q<<=1)>>15)}
    #define marker 0xFFFF

/*    memcpy(temp,inp,16384);
    return(16384);              */

    hash=(unsigned*)(temp+16384);
    for (i=0;i<16384+4096;i++) hash[i]=marker;
    input=0;
    output=1;
    output0=0;
    bits=1;

    while ((input<16384-256)&&(output<16384-256)) {
          j=j0=hash[j00=thishv=hv(input)];      /* search the hash table */
          bestlen=0;
          while ((j0!=marker)&&(bestlen<1023)) {
                j=j0;
                do {
                    if ((inp[input]==inp[j])&&
                        (inp[input+1]==inp[j+1])) {
                       for (i=2;(input+i<16384-256)&&(i<1023)&&(inp[input+i]==inp[j+i]);i++) {}
                       if (i>=bestlen) {
                          bestlen=i;
                          bestpos=j;
                       }
                       j+=i;
                    } else
                      j++;
                } while (((hv(j)==thishv)||(hv(j+1)==thishv))&&(j<input)&&(bestlen<1023));
                j0=hash[j00=j0];
          }

          if ((j==marker)||(input-j>=2)) hash[j00]=input;     /* update hash table */

          if ((bestlen>2)||((bestlen==2)&&(input-bestpos<256))) {
             putbit(0);
             if ((bestlen<16)&&(input-bestpos<256)) {   /* store result */
                putbit(1);
                putbits(input-bestpos,8);
                putbits(bestlen,4);
             } else {
               putbit(0);
               putbits(input-bestpos,14);
               putbits(bestlen,10);
             }
             input+=bestlen;
          } else {
            temp[output++]=inp[input++];
            putbit(1);
          }
    }

    do {bits<<=1;} while (!(bits&0x100));
    temp[output0]=bits;
    if (output<16384+256) {
       memcpy(temp+output,inp+input,256);
       return(output+256);
    } else {
      memcpy(temp,inp,16384);
      return(16384);
    }
}



void writetap(inp,areg,size)
unsigned char *inp;
unsigned char areg;
unsigned int size;
{
    unsigned int chksm=areg,i;
    for (i=0;i<size;i++) chksm^=inp[i];
    i=size+2;
    if (write(fout,&i,2)!=2) goto errwri;
    if (write(fout,&areg,1)!=1) {
       errwri:
       puts ("\n\nError writing to outputfile");
       exit (1);
    }
    if (write(fout,inp,size)!=size) goto errwri;
    if (write(fout,&chksm,1)!=1) goto errwri;
}




int main(int argc, char **argv)
{
    int i,j,compr=1,scr=0;
    puts ("Z802TAP - Converts .Z80 file to .TAP file - (c) 1993 G.A. Lunter - v2.01");
    puts ("          This program may not be distributed.");
    puts ("          It is part of the registered Spectrum Emulator package.\n");
    if (argc<2) {
       usage:
       puts ("\nUsage: Z802TAP [options]  inputfile[.Z80]  [outputfile[.TAP]]\n");
       puts ("Options are:  /u                  Do not compress the blocks");
       puts ("              /s screen[.SCR]     Add loading screen 'screen.SCR'\n");
       puts ("þ The .TAP file created contains the snapshot in a loadable format. It can be");
       puts ("  written to tape using TAP2TAPE and loaded into an ordinary Spectrum (128),");
       puts ("  but can of course also be loaded into the emulator.");
       puts ("þ Z802TAP will not convert SamRam snapshots.\n");
       exit(1);
    }
    for (i=0;i<8;i++) specmem[i]=malloc(16384);
    temp=malloc(60000L);
    if (temp==NULL) {
       puts ("Error - not enough memory.");
       exit (1);
    }
    i=1;
    while ((i<argc)&&((argv[i][0]=='/')||(argv[i][0]=='-'))) {
          if (argv[i][2]) {
             errcmd:
             puts ("\007Error on command line!\n");
             goto usage;
          }
          if ((argv[i][1]&0xDF)=='U')
             compr=0;
          else if ((argv[i][1]&0xDF)=='S') {
               if (scr) goto errcmd;
               i++;
               subext(argv[i],".SCR");
               scr=open(filename,O_RDWR+O_BINARY);
          } else goto errcmd;
          i++;
    }
    if (i==argc) goto errcmd;
    subext(argv[i++],".Z80");
    fin=open(filename,O_RDWR+O_BINARY);
    if (fin==-1) {
       printf ("Error - file %s not found\n",filename);
       exit(1);
    }
    if (i>=argc) i--;
    subext(argv[i++],".TAP");
    fout=_creat(filename,0);
    if (fout==-1) {
       printf ("Error - file %s cannot be created\n",filename);
       exit(1);
    }
    if (i<argc) goto usage;
    if (read(fin,&header,30)!=30) {
       errz80:
       puts ("\n\nError in .Z80 file.");
       exit (1);
    }
    if (header.pcreg!=0) {  /* prior to v2.0 */
       header.hmode=0;
       printf ("Processing: ......\rProcessing: ");
       if ((header.flagbyte==0xFF)||((header.flagbyte&0x20)==0)) {  /* old.. */
          if (read(fin,specmem[5],16384)!=16384) goto errz80;
          printf ("o");
          if (read(fin,specmem[1],16384)!=16384) goto errz80;
          printf ("o");
          if (read(fin,specmem[2],16384)!=16384) goto errz80;
          printf ("o");
       } else {   /* not-so-old */
         i=read(fin,temp,49152L);
         printf ("ooo");
         memmove(temp+59000L-i,temp,i);
         if (unpack(temp+59000L-i,temp,49152L)!=i-4) goto errz80;
         memcpy(specmem[5],temp,16384);
         memcpy(specmem[1],temp+16384,16384);
         memcpy(specmem[2],temp+32768L,16384);
       }
    } else {
      if (read(fin,&header.length,25)!=25) goto errz80;
      if (header.length!=23) goto errz80;
      header.pcreg=header.pcreg2;
      if (header.hmode==2) {
         puts ("\n\nError - Won't convert SamRam snapshots! (Change hardware mode in emulator)");
         exit (1);
      } else if (header.hmode<2)
          printf ("Processing: ......");
      else {
            if (header.outstate&8) printf ("\007WARNING: Program may not load correctly. Try snapping it at a different point.\n\n");
            printf ("Processing: ................");
      }
      printf ("\rProcessing: ");
      if (read(fin,&block,3)!=3) goto errz80;
      do {
         if (read(fin,temp,block.length)!=block.length) goto errz80;
         printf ("o");
         if (unpack(temp,specmem[block.page-3],16384)!=block.length) goto errz80;
      } while (read(fin,&block,3)==3);
    }
    if (header.hmode<3) {
       for (i=0;i<16;i++) header.soundreg[i]=0;
       header.outstate=16;
    }


    write(fout,&LOADER,425);
    tapehdr.type=3;
    for (i=0;i<10;i++) tapehdr.fnam[i]=' ';
    for (i=j=0;filename[i];i++) {
        if (filename[i]=='\\') j=i+1;
    }
    for (i=0;filename[j]&&(filename[j]!='.');j++,i++) tapehdr.fnam[i]=filename[j];
    tapehdr.b2='G';
    tapehdr.b3='L';
    if (scr) {
       tapehdr.fnam[9]='S';
       tapehdr.length=6912;
       tapehdr.b0=11;
       writetap((void*)&tapehdr,0,17);
       if (read(scr,temp,7000)!=6912) {
          close(fout);
          puts ("\n\nError - Screen snapshot not found or not exactly 6912 bytes long!");
          exit(1);
       }
       writetap(temp,0xFF,6912);
    }
    tapehdr.fnam[9]='0';
    for (i=0;i<8;i++) {
        if (((header.hmode>=3)||(i==1)||(i==2))&&(i!=5)) {
           printf ("*\b");
           if (compr)
              j=pack(specmem[i]);
           else
               memcpy(temp,specmem[i],j=16384);
           tapehdr.length=j;
           if (header.hmode>=3)
              tapehdr.b0=i;
           else
               tapehdr.b0=i+7;
           printf ("o");
           writetap((void*)&tapehdr,0,17);
           writetap(temp,0xFF,j);
           tapehdr.fnam[9]++;
        }
    }

    tapehdr.length=16384;
    tapehdr.b0=10;
    tapehdr.b1=header.ireg;
    printf ("o");
    writetap((void*)&tapehdr,0,17);
    memcpy(temp,specmem[5],16384);
    for (i=0;i<32;i++) {
        temp[i+0x15e0]=loadline1[i];
        temp[i+0x16e0]=loadline2[i];
    }
    #define copyword(i,j) temp[i]=j; temp[i+1]=(j>>8);
    temp[0x15e9]=header.outstate;
    temp[0x15f8]=(header.flagbyte>>1)&7;
    copyword(0x16e2,header.bcreg);
    copyword(0x16e5,header.dereg);
    copyword(0x16e8,header.hlreg);
    copyword(0x16ec,header.ixreg);
    temp[0x16f1]=((header.rreg-8)&0x7f)|((header.flagbyte&1)<<7);
    copyword(0x16f8,header.spreg);
    if ((header.imode&3)==2) temp[0x16fb]=0x5e;
    if (header.iff1==0) temp[0x16fc]=0xf3;
    copyword(0x16fe,header.pcreg);
    temp[0x17e4]=0xe0;
    temp[0x17e5]=0x55;
    for (i=0;i<14;i++) temp[0x17e6+i]=header.soundreg[i];
    copyword(0x17f4,header.hlareg);
    copyword(0x17f6,header.bcareg);
    copyword(0x17f8,header.deareg);
    copyword(0x17fa,header.iyreg);
    temp[0x17fc]=header.fareg;
    temp[0x17fd]=header.aareg;
    temp[0x17fe]=header.freg;
    temp[0x17ff]=header.areg;
    writetap(temp,0xFF,16384);
    if (close(fout)) {
       puts ("\n\nError writing to .TAP file");
       exit (1);
    }
    puts (",  Done.");
    return(0);
}


