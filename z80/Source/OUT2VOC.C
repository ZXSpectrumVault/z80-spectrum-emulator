/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 3.04, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                     Gerton Lunter, 10/8/96
*/

/* Out2Voc - Spectrum emulator utility - 27/10/94, G.A. Lunter */
/* Version 3.03 finished on 20/12/94 */


#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <sys/stat.h>
#include <math.h>


#define byte unsigned char
#define word unsigned int
#define queuelen0 64
#define queuelen1 8
#define oversamplerate 16
#define fudgefactor0 (256/257.4441)     /* oversamplerate/signal */
#define fudgefactor1 (256/252.7799)
#define freqmargin 0.95
#define ampfac 8192                     /* sweep of mic/ear signal in sineintegral */
#define phasesteps 8192
#define logps 13                        /* 2log of phasesteps */

const float fourier[logps]={1,1/2.,1/2.5,1/3.,1/3.5,1/4.,1/4.5,1/5.,1/5.5,1/6.,1/6.5,1/7.,1/7.5};
      /* fourier coeffs of AY signal; lots of high harmonics */

void subext(char*,char*);
void process_ayout(void);
float regulated_phasestep(float);


float fouriersum[logps];        /* fouriersum[i] = sum(j>=i) abs(fourier[j]) */
int outhandle,vochandle;
const byte vocheader[]={"Creative Voice File\032\032\000\012\001\051\021"};
const byte endmarker[]={"\000"};
char filename[80];
int digital;
word lastout,iovalue;
word lastvalue,value,port;
word curt,time,lastt,tpersample,tperblock;
float aphase,bphase,cphase,noisegen,ephase;       /* channel a,b,c; noise; envelope */
float aphstep,bphstep,cphstep,afactor,sqrt1minusa,ephstep;
int minT;
int aamp,bamp,camp,eamp,eadd,e2amp,e2add;     /* envelope stuff */
int adivfactor,bdivfactor,cdivfactor;
int asample,bsample,csample;
int amparr[16];
word curreg;
byte reg[16];
word mask,earamp,micamp,inactive,raw,qnd,silence,ayamp,warning;
word queue[queuelen0];
int queuelen;
int quietblock;
int newvalue;
long quietblocks;
float sine,tempsine,oldsine,signal,fudgefactor,r,ph;
word blocklen,ipointer,opointer,qpointer,samplerate,seconds,twohundreds;
byte *blockbuf,*outbuf;
int *sineintegral,*aysine[logps+1];
float *ayfloatsine;
long outcount;
int i,j,k;


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


float regulated_phasestep(float phase)
/* Find good rational approximation to phasestep, to reduce noise */
{
    #define maxrelerr 0.003     /* +/- approx. 1/10th of half note value */
    int i=0,j;
    do {
       i++;
       j=floor(i/phase+0.5);
    } while (fabs(1-i/(j*phase))>maxrelerr);
    return(i/(float)j);
}



void process_ayout(void)                /* uses global variables... */
{
    int i;
          if ((port|0x3FFC)==0xFFFD) {
             curreg=iovalue & 15;
          }
          if ((port|0x3FFC)==0xBFFD) {
             reg[curreg]=iovalue;
             switch (curreg) {
             case 0:
             case 1:
                  i=(*(int*)(reg))&4095;
                  if (i<minT) i=minT;
                  r=aphstep=regulated_phasestep((110830.0/samplerate)/i);
                  asample=0;
                  /* Find out which sample to use, so that it does not contain
                     frequency components too high for the current sampling rate,
                     and so that the high frequency components make at least a 1 bit
                     contribution to the actual signal at this amplitude, to reduce
                     unnecessary noise */
                  while ((asample<logps)&&(r<0.5)&&(4*(ampfac*fouriersum[asample])>=adivfactor)) {
                     r*=2;
                     asample++;
                  }
                  break;
             case 2:
             case 3:
                  i=(*(int*)(reg+2))&4095;
                  if (i<minT) i=minT;
                  r=bphstep=regulated_phasestep((110830.0/samplerate)/i);
                  bsample=0;
                  while ((bsample<logps)&&(r<0.5)&&(4*(ampfac*fouriersum[bsample])>=bdivfactor)) {
                     r*=2;
                     bsample++;
                  }
                  break;
             case 4:
             case 5:
                  i=(*(int*)(reg+4))&4095;
                  if (i<minT) i=minT;
                  r=cphstep=regulated_phasestep((110830.0/samplerate)/i);
                  csample=0;
                  while ((csample<logps)&&(r<0.5)&&(4*(ampfac*fouriersum[csample])>=cdivfactor)) {
                     r*=2;
                     csample++;
                  }
                  break;
             case 6:
                  i=reg[6]&31;
                  if (!i) i=1;
                  afactor=1-((110830.0/samplerate)/i)/3;
                  if (afactor<0) afactor=0;
                  sqrt1minusa=sqrt(1-afactor);
                  break;
             case 7:
                  if (reg[7]&1) aamp=0; else aamp=reg[8];
                  if (reg[7]&2) bamp=0; else bamp=reg[9];
                  if (reg[7]&4) camp=0; else camp=reg[10];
                  break;
             case 8:
                  if (reg[7]&1) aamp=0; else aamp=reg[8];
                  break;
             case 9:
                  if (reg[7]&2) bamp=0; else bamp=reg[9];
                  break;
             case 10:
                  if (reg[7]&4) camp=0; else camp=reg[10];
                  break;
             case 11:
             case 12:
                  i=(*(unsigned int*)(reg+11));
                  if (i<minT) i=minT;
                  ephstep=(110830.0/samplerate)/i;
                  break;
             case 13:
                  reg[13]&=0x0F;
                  if (reg[13]&4) eamp=0; else eamp=15;
                  eadd=1-(eamp&2);       /* 15->-1, 0->1 */
                  if (!(reg[13]&8))
                     e2amp=e2add=0;
                  else {
                       if ((reg[13]^(reg[13]>>1)^(reg[13]>>2))&1)
                          e2amp=0;
                       else
                           e2amp=15;
                       e2add=1-(e2amp&2);
                  }
             default:
                  break;
             }
          }
}



void main(argc,argv)
int argc;
char **argv;
{
    puts ("\nOUT2VOC - Converts .OUT log file of music or SAVE-data to sample file - v3.03");
    puts ("          This program may not be distributed.");
    puts ("          It is part of the registered Spectrum Emulator package.\n");
    if (argc==1) {
       puts ("Usage: OUT2VOC  [switches ...]  inputfile[.OUT]  [outputfile[.VOC|.RAW]]");
       puts ("\nSwitches: -f rate    to set the sample rate (default 10000 Hz)");
       puts ("          -d         for a digital sample file");
       puts ("          -m, -e     listen to MIC/EAR output resp. (port FE)");
       puts ("          -a         listen to AY-3-8912 soundchip (ports BFFD and FFFD)");
       puts ("          -i f[.Z80] use initial soundchip settings from .Z80 snapshot 'f'");
       puts ("          -r         make a raw sample file instead of a .VOC file");
       puts ("          -q         for quicker conversion of beeper sounds");
       puts ("          -s         skip silences of longer than 1 second");
       puts ("\n  OUT2VOC converts OUTs to the internal Spectrum beeper and to the 128 sound-");
       puts ("  chip to a sound sample file, for converting music or SAVE-data.");
       puts ("þ By default, OUT2VOC listens to the EAR output.  To convert save-data, use");
       puts ("  -m; for '128 music, use -a.  Use -e -a for beeper and AY sounds to be mixed.");
       puts ("þ To get the correct result when converting '128 music, it is sometimes");
       puts ("  necessary to know initial values of AY registers.  Use -i for this.");
       puts ("þ For save-data, use -d.  The resulting sample files can be compressed much");
       puts ("  better.  Don't use this switch for music.  No difference for '128 music.");
       exit(1);
    }
    for (i=0;i<logps+1;i++) aysine[i]=(int*)malloc(sizeof(int)*(phasesteps));
    ayfloatsine=(float*)malloc(sizeof(float)*phasesteps);
    sineintegral=(int*)malloc(sizeof(int)*oversamplerate*queuelen0);
    blockbuf=(byte*)malloc(16000);
    outbuf=(byte*)malloc(0x8000);
    fouriersum[logps-1]=abs(fourier[logps-1]);
    for (i=logps-2;i>=0;i--) fouriersum[i]=fouriersum[i+1]+fabs(fourier[i]);
    if (outbuf==NULL) {
       puts ("\007Sorry --- not enough memory.");
       exit(1);
    }
    samplerate=10000;
    minT=12;
    /* minimum value of timing registers in order for phase step to be <1 */
    fudgefactor=fudgefactor0;
    queuelen=queuelen0;
    digital=0;
    raw=0;
    qnd=0;
    silence=0;
    mask=0x00;
    earamp=0;
    micamp=0;
    ayamp=0;
    /* initialising AY */
    aphase=bphase=cphase=ephase=0;
    aphstep=bphstep=cphstep=ephstep=0;
    afactor=1;
    sqrt1minusa=0;
    noisegen=0;
    aamp=bamp=camp=0;
    eamp=eadd=e2amp=e2add=0;
    adivfactor=bdivfactor=cdivfactor=30000;  /* effectively infinity */
    asample=bsample=csample=0;
    for (i=0;i<16;i++) reg[i]=0;
    reg[7]=0xff;
    curreg=0;
    /* end of AY initialisation */
    i=1;
    while ((i<argc)&&(argv[i][0]=='-')) {
        if ((argv[i][1]&0xDF)=='F') {
           long temp;
           if (argv[i][2]) temp=(unsigned)atol(argv[i]+2); else
                           temp=(unsigned)atol(argv[++i]);
           if (temp<4000) {
              printf ("\007Minimum sample rate is 4000 Hz\n");
              exit(1);
           }
           if (temp>50000L) {
              printf ("\007Maximum sample rate is 50000 Hz\n");
              exit(1);
           }
           samplerate=(unsigned)temp;
        }
        else if ((argv[i][1]&0xDF)=='I') {
           int z80handle;
           word dummy;
           if (argv[i][2])
              subext(argv[i]+2,".Z80");
           else
               subext(argv[++i],".Z80");
           printf ("(Loading soundchip register values from %s...)\n",filename);
           z80handle=open(filename,O_RDONLY|O_BINARY);
           lseek(z80handle,6,SEEK_SET);
           if (read(z80handle,&dummy,2)!=2) {
              rerr:
              printf ("Error while opening file %s.",filename);
              exit(1);
           }
           if (dummy!=0) {
              verr:
              printf ("File %s either corrupt as a .Z80 snapshot file, or old version.\n",filename);
              exit(1);
           }
           lseek(z80handle,30,SEEK_SET);
           if (read(z80handle,&dummy,2)!=2) goto rerr;
           if ((dummy!=23)&&(dummy!=54)&&(dummy!=21)) goto verr;
           lseek(z80handle,39,SEEK_SET);
           for (j=0;j<14;j++)              /* get AY registers */
               if (read(z80handle,&reg[j],1)!=1) goto rerr;
           lseek(z80handle,38,SEEK_SET);
           read(z80handle,&curreg,1);
           curreg&=15;
           close(z80handle);
        }
        else if ((argv[i][1]&0xDF)=='D') digital=1;
        else if ((argv[i][1]&0xDF)=='R') raw=1;
        else if ((argv[i][1]&0xDF)=='Q') qnd=1;
        else if ((argv[i][1]&0xDF)=='S') silence=1;
        else if ((argv[i][1]&0xDF)=='E') {
           mask|=0x10;
        }
        else if ((argv[i][1]&0xDF)=='M') {
           mask|=0x08;
        }
        else if ((argv[i][1]&0xDF)=='A') {
           ayamp=4;
        }
        else {
            printf ("\007Unrecognized switch on command line.\n");
            exit(1);
        }
        i++;
    }
    if (i==argc) {
       printf ("\007Need .OUT file name!");
       exit(1);
    }
    printf ("Sample rate: %u Hz.\n",samplerate);
    if (digital) printf ("Making a 'digital' .VOC file.\n");
    if (raw) printf ("Making a raw sample file.\n");
    if (silence) printf ("Skipping long silences.\n");
    if (qnd) {
       printf ("Converting quick but dirty.\n");
       fudgefactor=fudgefactor1;
       queuelen=queuelen1;
    }
    if (mask&0x10) printf ("Listening to EAR output.\n");
    if (mask&0x08) printf ("Listening to MIC output.\n");
    if (ayamp) printf ("Listening to AY-3-8912 soundchip output.\n");
    if (!ayamp) {
       if (mask==0) mask=0x10;
       if (mask==0x10) earamp=4;
       if (mask==0x08) micamp=4;
       if (mask==0x18) {
          earamp=3;
          micamp=1;
       }
    } else {
      mask&=0x10;
      if (mask==0x10) {
         earamp=1;
         ayamp=3;
      }
    }
    inactive=0x8080-(ampfac/2)*(earamp+micamp);     /* 0x8080 to prevent small-signal blow-up */
    minT=(int)ceil(110830.0/samplerate);
    for (j=0;j<14;j++) {             /* process AY registers with correct sample rate */
        port=0xFFFD;
        iovalue=j;
        process_ayout();
        port=0xBFFD;
        iovalue=reg[j];
        process_ayout();
    }
    printf ("\n");
    subext(argv[i],".OUT");
    outhandle=open(filename,O_RDONLY|O_BINARY);
    if (outhandle==-1) {
       printf ("\007File %s could not be opened for reading.\n",filename);
       exit(1);
    }
    if (!raw) subext(argv[argc-1],".VOC"); else subext(argv[argc-1],".RAW");
    vochandle=open(filename,O_RDWR|O_CREAT|O_TRUNC|O_BINARY,S_IREAD|S_IWRITE);
    if (vochandle==-1) {
       printf ("\007File %s could not be created.\n",filename);
       close(outhandle);
       exit(1);
    }
    if (!raw) if (write(vochandle,vocheader,0x1a)!=0x1a) {
       write_err:
       printf ("\007Error while writing %s.\n",filename);
       errexit:
       close(vochandle);
       close(outhandle);
       exit(1);
    }
    printf ("Opened file %s for output.\n",filename);
    printf ("\nInitialising... ");
    tpersample=(int)(3494400L/samplerate);
    oldsine=0;
    signal=0;
    if (!digital) {
      for (i=-(oversamplerate/2)*queuelen;i<(oversamplerate/2)*queuelen;i++)
      for (j=0;j<4;j++) {
        if ((i==0)&&(j==0)) sine=freqmargin; else {
           sine=freqmargin*(M_PI*i + (M_PI/4)*j)/oversamplerate;
           sine=freqmargin*sin(sine)/sine;
        }
        if ((j%2)==0)
           tempsine=sine;
        else {
             signal=signal+(oldsine+4*tempsine+sine)/12;     /* simpson */
             oldsine=sine;
             if ((i>=-(oversamplerate/2)*queuelen)&&(j==3))
                sineintegral[i+(oversamplerate/2)*queuelen]=
                  ampfac*((fudgefactor*signal)/oversamplerate);
        }
      }
    }

    if (ayamp) {

       for (i=0;i<phasesteps;i++) {
           ayfloatsine[i]=0.0;
           aysine[0][i]=0;
       }
       for (j=1;j<=logps;j++) {
           for (i=0;i<phasesteps;i++) {
               ayfloatsine[i]+=fourier[j-1]*sin(((2*M_PI*j)*i)/phasesteps);
               aysine[j][i]=floor(0.5+2*ampfac*ayfloatsine[i]);
           }
       }
       for (i=1;i<16;i++) amparr[i]=floor((4*3*exp(log(2)*(15-i)/2.0)/ayamp)+0.49);
       amparr[0]=30000;    /* effectively infinity */
       /* 2 amplitude's=factor 2 (3 dB). 4=2(sweep/amp)*2 with aysine, 2 should be 3 (channels) */

    }

    printf ("\nProcessing... 0:00.0 s");
    lastout=0x08;
    lastvalue=0;
    lastt=0;
    time=0;
    seconds=0;
    twohundreds=0;
    outcount=0;
    tperblock=17727;
    for (i=0;i<queuelen;i++) queue[i]=inactive;
    qpointer=0;
    ipointer=0;
    warning=0;
    quietblocks=0;
    blocklen=read(outhandle,blockbuf,16000);
    if ((blocklen%5)!=0) {
       lengtherr:
       printf ("\n\nError in .OUT file: length not a multiple of 5\n");
       close(outhandle);
       close(vochandle);
       exit(1);
    }
    if (raw) opointer=0; else {
       opointer=6;
       outbuf[0]=1;        /* sound data */
       outbuf[4]=256-(int)(1000000L/samplerate);
       outbuf[5]=0;        /* compression type: none */
    }
    while (blocklen>0) {
          curt=*(word*)(blockbuf+ipointer);
          if (curt==0xFFFF) {
             if ((quietblock)&&(silence))
                quietblocks++;
             else {
                  if (quietblocks>200) {
                     quietblocks-=200;
                     printf ("\nSkipped: %d:%02d.%d s\n",(int)(quietblocks/12000),
                                                        (int)((quietblocks/200)%60),
                                                        (int)((quietblocks%200)/20) );
                  }
                  quietblocks=0;
             }
             quietblock=1;
             iovalue=lastout;
             port=0xfe;
             tperblock=*(word*)(blockbuf+ipointer+2);
             time+=tperblock;
             if ((tperblock!=17472)&&(tperblock!=17727)&&((warning&8)==0)) {
                warning|=8;
                puts ("\nWARNING: 1/200s time skip is neither 17472 (48K) nor 17727 (128K) T states");
             }
             curt=0;
             twohundreds++;
             if ((twohundreds%20)==0) {
                if (twohundreds==200) {
                   twohundreds=0;
                   seconds++;
                }
                printf ("\rProcessing... %d:%02d.%d s",seconds/60,seconds%60,twohundreds/20);
             }
          } else if (curt==0xFFFE) {
            if ((warning&16)==0) {
               puts ("\nWARNING: OUT log file contains program trace.  Cannot be converted to sound");
               puts ("sample due to lack of timing information.\n");
               warning|=16;
            }
          } else {
             outcount++;
             port=*(word*)(blockbuf+ipointer+2);
             iovalue=(int)blockbuf[ipointer+4];
             if ((port&1)&&((warning&4)==0)) {
                /* warning for weird out */
                warning|=4;
             }
             if (curt>tperblock) port=0xFF;
          }
          if (curt>tperblock) {
             if ((warning&1)==0) {
                printf ("\n\nWARNING: High times in 1/200s block encountered.  OUT log file corrupt.\n");
                warning|=1;
             }
             curt=tperblock;
          }
          if (time+curt<lastt) {
             if ((warning&2)==0) {
                printf ("\n\nWARNING: OUTs in wrong order encountered.  OUT log file corrupt.\n");
                warning|=2;
             }
             curt=lastt-time;
          }
          ipointer+=5;
          blocklen-=5;

          #define ayvoice(s,p,d) \
            (((word)(aysine[s][floor(p*phasesteps)]+d*(((word)0x8000)/d)+(d>>1)))/d)-((word)0x8000)/d

          #define minimum(a,b,c) \
             (a<b ? (a<c?a:c) : (b<c?b:c) )

          #define noisedivfactor \
             minimum((reg[7]&0x8  ? 30000 : adivfactor), \
                     (reg[7]&0x10 ? 30000 : bdivfactor), \
                     (reg[7]&0x20 ? 30000 : cdivfactor))

          #define ayvalue ayvoice(asample,aphase,adivfactor) + \
                          ayvoice(bsample,bphase,bdivfactor) + \
                          ayvoice(csample,cphase,cdivfactor) + \
                          2*ampfac*noisegen/(noisedivfactor);

          while (time+curt-lastt>=tpersample) {
             if (quietblocks<200) {
                newvalue=inactive+lastvalue*ampfac;
                if (ayamp) {
                   noisegen=afactor*noisegen+
                            (sqrt1minusa*(random(1000)-500))/500;
                }
                if (digital) {
                   if (ayamp) newvalue+=ayvalue;   /* same place */
                   outbuf[opointer++]=newvalue>>8;
                } else {
                  outbuf[opointer++]=(queue[qpointer]>>8);
                  queue[qpointer]=newvalue;
                  if (ayamp)
                     queue[(qpointer+(queuelen/2))%queuelen]+=ayvalue;    /* further up */
                  qpointer=(qpointer+1)%queuelen;
                }
             }
             lastt+=tpersample;
             if (ayamp) {
                aphase+=aphstep;
                if (aphase>1) { /* change amps only at 0 crossing */
                   aphase-=1;
                   adivfactor=amparr[aamp&0x10 ? eamp : aamp&0x0F];
                   r=aphstep;
                   asample=0;
                   while ((asample<logps)&&(r<0.5)&&(4*(ampfac*fouriersum[asample])>=adivfactor)) {
                      r*=2;
                      asample++;
                   }
                   if ((asample)&&(adivfactor<30000)) quietblock=0;
                }
                bphase+=bphstep;
                if (bphase>1) {
                   bphase-=1;
                   bdivfactor=amparr[bamp&0x10 ? eamp : bamp&0x0F];
                   r=bphstep;
                   bsample=0;
                   while ((asample<logps)&&(r<0.5)&&(4*(ampfac*fouriersum[bsample])>=bdivfactor)) {
                      r*=2;
                      bsample++;
                   }
                   if ((bsample)&&(bdivfactor<30000)) quietblock=0;
                }
                cphase+=cphstep;
                if (cphase>1) {
                   cphase-=1;
                   cdivfactor=amparr[camp&0x10 ? eamp : camp&0x0F];
                   r=cphstep;
                   csample=0;
                   while ((csample<logps)&&(r<0.5)&&(4*(ampfac*fouriersum[csample])>=cdivfactor)) {
                      r*=2;
                      csample++;
                   }
                   if ((csample)&&(cdivfactor<30000)) quietblock=0;
                }
                ephase+=ephstep;
                if (ephase>1) {
                   ephase-=1;
                   eamp+=eadd;
                   if (eamp&0xF0) {
                      eamp=e2amp;
                      eadd=e2add;
                      if ((reg[13]==10)||(reg[13]==14)) {
                         e2amp=15-e2amp;
                         e2add=-e2add;
                      }
                   }
                }
             }
          }
          while ((lastt>=tperblock)&&(time>=tperblock)) {
                lastt-=tperblock;
                time-=tperblock;
          }
          process_ayout();  /* updates global variables for AY emulation */
          if (((iovalue&mask)!=lastout)&&((port&1)==0)) {
             quietblock=0;
             value=0;
             iovalue&=mask;
             if (!(iovalue&0x08)) value+=micamp;
             if (iovalue&0x10) value+=earamp;
             if (!digital) {
                j=(int)((oversamplerate*(long)(time+curt-lastt))/(long)tpersample);
                j=oversamplerate-j-1;
                if (j<0) printf ("%d\n",j);
                if (j>=oversamplerate) printf ("%d\n",j);
                for (i=0;i<queuelen;i++)
                    queue[(i+qpointer)%queuelen]+=(value-lastvalue)*sineintegral[oversamplerate*i+j];
             }
             lastvalue=value;
             lastout=iovalue;
          }
          if (opointer>0x7800) {
              if (!raw) {
                 outbuf[1]=(opointer-4) & 0xFF;
                 outbuf[2]=(opointer-4) >> 8;
                 outbuf[3]=0;
              }
              if (write(vochandle,outbuf,opointer)!=opointer) {
                 printf ("\n");
                 goto write_err;
              }
              if (raw)
                 opointer=0;
              else {
                   outbuf[0]=2;        /* sound continue */
                   opointer=4;
              }
          }
          if (blocklen==0) {
              ipointer=0;
              blocklen=read(outhandle,blockbuf,16000);
              if ((blocklen%5)!=0) goto lengtherr;
          }
    }
    if (!digital)
       for (i=0;i<queuelen;i++) outbuf[opointer++]=(queue[(i+qpointer)%queuelen]>>8);
    if (!raw) {
       outbuf[1]=(opointer-4) & 0xFF;
       outbuf[2]=(opointer-4) >> 8;
       outbuf[3]=0;
    }
    if (write(vochandle,outbuf,opointer)!=opointer) {
       printf ("\n");
       goto write_err;
    }
    if (!raw) write(vochandle,endmarker,1);
    close (outhandle);
    if (quietblocks>200) {
       quietblocks-=200;
       printf ("\nSkipped: %d:%02d.%d s\n",(int)(quietblocks/12000),
                                          (int)((quietblocks/200)%60),
                                          (int)((quietblocks%200)/20) );
    }
    if (close (vochandle)) goto write_err;
    printf ("\n\nDone.   (processed %ld OUTs)\n",outcount);
    return;
}

