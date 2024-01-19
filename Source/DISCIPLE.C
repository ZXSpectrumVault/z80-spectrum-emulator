/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 3.04, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                     Gerton Lunter, 10/8/96
*/

/*
 This program MUST be compiled using the LARGE model.

 Modifications to fix corruption of squeezed files
 Ian Cull. 21/10/92.
 (See lines marked 'IANC')
 Modifications to correctly translate SNAP files
 Ian Cull. 8/11/92.
 (See lines marked 'IANC2')
 Modifications to add autocopy
 Ian Cull. 9/11/92.
 Modifications to read 720K 80 track double density 5.25'' diskettes,
 Hugh McLenaghan, 31/3/93
 Made obsolete by emulation of the DISCiPLE and +D also by Hugh McLenaghan
*/


#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <bios.h>
#include <memory.h>

unsigned char   secbuf[512];
int             secbufp;
struct diskinfo_t disk_read;


char            name[80];
int             drive,buf_track,buf_head,bad_track,curblock,printmes,eightytracks=1;
unsigned char   *buffer,*trackbuf,*pagebuf[8];
unsigned char   *mbyte=NULL,oldmbyte;
const char      interleave[]={1,6,2,7,3,8,4,9,5,10};
unsigned int    bufferp;

/* IANC2: header data changed to correct for bugs in SNAP file conversion
   and to add comments */

const unsigned char header[]={
        1,      /* A and F, fetched from stacked data in code */
        234,235,/* BC */
        236,237,/* HL */
        2,      /* PC, fetched from stacked data in code */
        4,      /* SP, 6 more than in SNAP file to account for stacked data */
        239,    /* I (IANC2: bug fix here) */
        5,      /* R */
        3,      /* bitmap - R bit 7, border colour, squeeze flag, etc */
        232,233,/* DE */
        226,227,/* BC' */
        224,225,/* DE' */
        228,229,/* HL' */
        231,230,/* AF' (IANC2: bug fix here) */
        220,221,/* IY */
        222,223,/* IX */
        0       /* end of data */
        };

void    leessector(int,int,int);
void    getmbyte(void);
void    clr(void);
void    toets(void);
void    addextension(char*,char*);
int     fwritebuf(unsigned int,unsigned int,FILE*);
void    listfiles(void);
void    compress(unsigned char*,unsigned char **,unsigned int);
int     readpiece(unsigned int*,int*,int*,int*,int*,FILE*,int);
int     copyfile(int,FILE*);
void    autocopy(char);


void leessector(track,head,sector)
int track,head,sector;
{
    int i;
    if ((buf_track!=track)||(buf_head!=head)||(bad_track)) {
        char far *drivparm;
        unsigned result;
        int counter;
        if ((buf_track!=track)||(buf_head!=head)) bad_track=0;
        drivparm=*((char**)(120L));     /* This only works in LARGE model */
        drivparm[4]=0x16;
        if (bad_track)
            i=9;
        else
            i=0;
        do {
            counter=10;
            do {
               disk_read.drive = drive;
               disk_read.head = head;
               disk_read.track = track;
               disk_read.nsectors = 1;
               if (bad_track) {
                   disk_read.sector = sector+1;
                   disk_read.buffer = trackbuf+512*sector;
               } else {
                   disk_read.sector = interleave[i];
                   disk_read.buffer = trackbuf+512*(interleave[i]-1);
               }
               result=_bios_disk ( _DISK_READ, &disk_read);
               if (result<=1) counter=0;
            } while (--counter>0);
            if (!counter) {
               if (bad_track) {
                   puts ("\n\nError reading disciple/+D diskette!  Exiting.");
                   *mbyte=oldmbyte;
                   exit(1);
               } else {
                    bad_track=1;
                    i=8;
               }
            }
        } while (++i<10);
    }
    buf_track=track;
    buf_head=head;
    memcpy(secbuf,trackbuf+512*sector,512);
}


void getmbyte(void)
{
    int i=10;
    if (mbyte!=NULL) *mbyte=oldmbyte;
    _bios_disk(_DISK_RESET,&disk_read);
    do {
        disk_read.drive=drive;
        disk_read.head=0;
        disk_read.track=0;
        disk_read.nsectors=1;
        disk_read.sector=1;
        disk_read.buffer=trackbuf;
    } while ((--i>0)&&(_bios_disk(_DISK_READ,&disk_read)>1));
    if (!i) {
       puts ("ERROR - drive not responding.  Exiting.");
       exit(1);
    }
    mbyte=(unsigned char*)(0x490L+drive);
    oldmbyte=*mbyte;
    if (eightytracks)
       *mbyte=oldmbyte&0xDF;   /* disable double stepping for eg 720K 5.25'': Hugh McLenaghan */
    else
        *mbyte=oldmbyte|0x20;  /* enable double stepping for 40 track disks */
}



void clr()
{
    clrscr();
    printf ("DISCIPLE - disk read/conversion utility - (c) 1994 G.A. Lunter - version 3.0\n");
    printf ("           Bug fixes and enhancements by Ian Cull and Hugh McLenaghan.\n");
    printf ("           This program may not be distributed.\n");
    printf ("           It is part of the registered Spectrum Emulator package.\n\n");
}

void toets()
{
    printf ("Press any key.\n");
    while (kbhit()) getch();
    while (!kbhit()) {}
    getch();
}

int fwritebuf(position,length,bestand)
unsigned int position,length;
FILE *bestand;
{
    if ((bufferp+length>50000L)||(length==0)) {
        if (fwrite(buffer,bufferp,1,bestand)!=1) return(0);
        bufferp=0;
    }
    memcpy(buffer+bufferp,secbuf+position,length);
    bufferp+=length;
    return(1);
}

void listfiles(void)
{
    int n,k,m=0;
    int offset;
    buf_track=-1;
    printf ("Directory of DISCiPLE/+D disk in drive %c:\n\n",'A'+drive);
    puts ("Nr Type    Name                        Nr Type    Name");
    puts ("------------------------------------------------------------------------------");
    for (n=0;n<80;n++) {
        if (!(n%2)) leessector(n/20,0,(n/2)%10);
        offset=256*(n%2);
        if ((secbuf[offset]==0) && (secbuf[offset+1]==0))
            n = 999; /* Stop early if rest of directory unused */
        if (secbuf[offset]!=0) {
            printf ("%2d ",n+1);
            switch (secbuf[offset])
            {
                case 1:printf("Program ");break;
                case 2:printf("NmbArr  ");break;
                case 3:printf("CharArr ");break;
                case 4:printf("Code    ");break;
                case 5:printf("48 Snap ");break;
                case 7:printf("ScrSnap ");break;
                case 9:printf("128Snap ");break;
                default:printf(" -----  ");
            }
            for (k=1;k<=10;k++) {
                putch(secbuf[offset+k]);
            }
            m++;
            switch (secbuf[offset]) {
            case 1:
                k=*(int*)(secbuf+offset+218);
                if (k!=-1) printf (" LINE %-5u      ",k);
                    else goto blankpad;
                break;
            case 4:
                k=*(int*)(secbuf+offset+214);
                printf (" CODE %5u,",k);
                k=*(int*)(secbuf+offset+212);
                printf ("%-5u",k);
                break;
            default:
                blankpad:
                printf ("                 ");
            }
        if (m%2) printf(" ");else printf("\n");
        }
    }
    printf("\n\n");
    toets();
}

int readpiece(length,sector,track,blocks,chksum,bestand,totblocks)
unsigned int *length;
int *track,*sector,*blocks,*chksum;
int totblocks;
FILE *bestand;
{
    int kant,k,buflen;
    do {
        if (secbufp==0) {
           if (*length==0) return 0;
           kant=0;
           if (*track>=128) {
               kant=1;
               *track-=128;
           }
           if ((*sector<0)||(*sector>10)||
              (*track<0)||(*track>80)) {
              puts ("\n\nError in file-format on disciple/+D diskette. File cannot be read.\n");
              fclose(bestand);
              return 1;
           }
           leessector(*track,kant,*sector-1);
           (*blocks)--;
           curblock++;
           if (printmes)
              printf("Reading diskette: %3u%% complete\r",(100*curblock)/totblocks);
           else
               printf ("%3u%%\b\b\b\b",(100*curblock)/totblocks);
        }
        if (*length>=510-secbufp) buflen=510-secbufp; else buflen=*length;
        for (k=secbufp;k<secbufp+buflen;k++) *chksum^=secbuf[k];
        *length-=buflen;
        if (fwritebuf(secbufp,buflen,bestand)!=1) {
            fclose(bestand);
            return 1;
        }
        secbufp+=buflen;
        if (secbufp>=510) {
           *track=secbuf[510];
           *sector=secbuf[511];
           secbufp=0;
        }
    } while (*length);
    return 0;
}

void compress(unsigned char *source,unsigned char **dest,unsigned int length)
{
    unsigned char tel=0,byte,k;
    do {
        if (tel==0) {
            byte=*(source++);
            tel=1;
            length--;
        } else if ((tel!=255)&&(byte==*source)&&(length!=0)) {
            source++;
            tel++;
            length--;
        } else if ((tel>=5)||((byte==0xED)&&(tel>=2))) {
            *((*dest)++)=0xED;
            *((*dest)++)=0xED;
            *((*dest)++)=tel;
            *((*dest)++)=byte;
            tel=0;
        } else {
            for (k=0;k<tel;k++) *((*dest)++)=byte;
            tel=0;
            if ((byte==0xED)&&(length!=0)) {
                *((*dest)++)=*(source++);
                length--;
            }
        }
    } while ((length!=0)||(tel!=0));
}


int copyfile(nummer,bestand)
int nummer;
FILE *bestand;
{
    int track,sector,lengte;
    int k,byte,port7ffd,ftype;
    unsigned int stkp;
    unsigned char *pointer;
    unsigned int bytelen;
    int secpointer;
    bufferp=0;
    secbufp=0;
    buf_track=-1;
    curblock=0;
    secpointer=(nummer%2)*256;
    switch (secbuf[secpointer]) {
        case 1:
        case 2:
        case 3:
        case 4: ftype='3';break;            /* normal files */
        case 5: ftype='4';break;            /* 48k snap */
        case 7: ftype='5';break;            /* screen snap, to .SCR */
        case 9: ftype=0;break;              /* 128k snap */
        default:return(1);
    }
    lengte=256*secbuf[secpointer+11];
    lengte+=secbuf[secpointer+12];
    track=secbuf[secpointer+13];
    sector=secbuf[secpointer+14];
    bytelen=256*secbuf[secpointer+213];
    bytelen+=secbuf[secpointer+212]+9;
    if (ftype=='4') {
        if (lengte!=97) goto formaterr;
        bytelen=49152L;
        fwritebuf(0,512,bestand);   /* Only to store data in the buffer */
    } else if (ftype=='5') {
        if (lengte!=14) goto formaterr;
        bytelen=9;
        if (readpiece(&bytelen,&sector,&track,&lengte,&byte,bestand,lengte)!=0) return 1;
        bufferp=0;          /* throw away header */
        bytelen=6912;
    } else if (ftype==0) {
        if (lengte!=258) goto formaterr;
        fwritebuf(0,512,bestand);
    } else {
      k=19;
      fwrite(&k,2,1,bestand);
      k=0;
      fwrite(&k,1,1,bestand);
      secbuf[secpointer+11]=secbuf[secpointer+212];
      secbuf[secpointer+12]=secbuf[secpointer+213];
      switch (--secbuf[secpointer]) {
      case 0:
            secbuf[secpointer+13]=secbuf[secpointer+218];
            secbuf[secpointer+14]=secbuf[secpointer+219];
            secbuf[secpointer+15]=secbuf[secpointer+216];
            secbuf[secpointer+16]=secbuf[secpointer+217];
            break;
      case 1:
      case 2:
            secbuf[secpointer+14]=secbuf[secpointer+216];
            break;
      default:
            secbuf[secpointer+13]=secbuf[secpointer+214];
            secbuf[secpointer+14]=secbuf[secpointer+215];
            secbuf[secpointer]=3;
            break;
      }
      byte=0;
      for (k=0;k<17;k++) byte^=secbuf[secpointer+k];
      secbuf[secpointer+17]=byte;
      fwrite(secbuf+secpointer,18,1,bestand);
      k=bytelen-7;    /* no header, but don't forget checksum & flag */
      fwrite(&k,2,1,bestand);
      k=255;
      fwrite(&k,1,1,bestand);
      k=bytelen-9;
      bytelen=9;
      if (readpiece(&bytelen,&sector,&track,&lengte,&byte,bestand,lengte)!=0) return 1;
      bytelen=k;
      bufferp=0;        /* throw away header */
    }
    if (ftype==0) {
        bufferp=512;
        bytelen=1;
        if (readpiece(&bytelen,&sector,&track,&lengte,&byte,bestand,258)!=0) goto formaterr;
        port7ffd=buffer[512];
        for (k=0;k<8;k++) {
            bytelen=16384;
            bufferp=512;
            if (readpiece(&bytelen,&sector,&track,&lengte,&byte,bestand,258)!=0) goto formaterr;
            memcpy(pagebuf[k],buffer+512,16384);
        }
    } else {
        byte=255;
        if (readpiece(&bytelen,&sector,&track,&lengte,&byte,bestand,lengte)!=0) return 1;
    }

    if (lengte) {
        formaterr:
        puts ("\nError in file-format on disciple/+D diskette.  File cannot be read.\n");
        fclose(bestand);
        return 1;
    }

    if (printmes)
       printf("Reading diskette: 100%% complete\n");
    else
      printf("100%%");
    if (ftype=='3') {
        if (fwritebuf(0,0,bestand)!=1) goto writeerr;
        if (fwrite(&byte,1,1,bestand)!=1) goto writeerr;
        if (fclose(bestand)) {
            writeerr:
            fclose(bestand);
            return 1;
        }
        return 0;
    }
    if (ftype=='5') {
        if (fwritebuf(0,0,bestand)!=1) goto writeerr;
        if (fclose(bestand)) goto writeerr;
        return 0;
    }
    k=0;
    secpointer=256*(nummer%2);
    stkp=*(unsigned int*)(buffer+secpointer+240);
    do {
        #define specbyte(adr,reg) {if (ftype!=0) reg=*(buffer+adr-16384+512); else {\
            if (adr<0x8000) reg=pagebuf[5][adr-0x4000]; else \
            if (adr<0xC000) reg=pagebuf[2][adr-0x8000]; else \
                            reg=pagebuf[port7ffd & 7][adr-0xC000]; }}
        switch (header[k]) {
        case 0:
            break;
        case 1:
            specbyte(stkp+3,byte)
            fwrite(&byte,1,1,bestand);    /* A */
            specbyte(stkp+2,byte)
            fwrite(&byte,1,1,bestand);    /* F, in this order */
            break;
        case 2:
            if (ftype==0) {
                byte=0;
                fwrite(&byte,2,1,bestand);
            } else fwrite(buffer+stkp+4-16384+512,2,1,bestand);
            break;
        case 3:
            /* IANC2: R register bit 7 fetched from correct place */
            specbyte(stkp+1,byte)
            byte=32+((byte&128)>>7);
            fwrite(&byte,1,1,bestand);                      /* Flag byte */
            break;
        case 4:
            stkp+=6;
            fwrite(&stkp,2,1,bestand);                      /* STACK */
            stkp-=6;
            break;
        case 5:
            specbyte(stkp+1,byte)                           /* R */
            fwrite(&byte,1,1,bestand);
            break;
        default:
            fwrite (buffer+secpointer+header[k],1,1,bestand);
        }
    } while(header[k++]);
    specbyte(stkp,byte)
    byte=(!(byte&4))-1;                                     /* Test P/V flag for DI/EI */
    fwrite(&byte,2,1,bestand);                              /* IFF1 and 2 */
    /* IANC2: I register accessed from offset 239, not 231 */
    byte=*(buffer+secpointer+239);                          /* IM0/1/2 */
    if ((byte==0)||(byte==0x3F)) byte=1; else byte=2;
    fwrite(&byte,1,1,bestand);
    if (ftype==0) {
        byte=23;                        /* Length */
        fwrite(&byte,2,1,bestand);
        specbyte(stkp+4,byte)           /* PC */
        fwrite(&byte,1,1,bestand);
        specbyte(stkp+5,byte)
        fwrite(&byte,1,1,bestand);
        byte=3+256*port7ffd;            /* Hardware (128), Port 7ffd */
        fwrite(&byte,2,1,bestand);
        for (byte=k=0;k<19;k++) fwrite(&byte,1,1,bestand);
        for (k=0;k<8;k++) {
            pointer=buffer;
            compress(pagebuf[k],&pointer,16384);
            byte=pointer-buffer;
            fwrite(&byte,2,1,bestand);
            byte=k+3;
            fwrite(&byte,1,1,bestand);
            if (fwrite(buffer,pointer-buffer,1,bestand)!=1) goto writeerr;
        }
        if (fclose(bestand)) goto writeerr;
        return 0;
    }
    pointer=buffer;
    compress(buffer+512,&pointer,49152L);
    *(pointer++)=0;
    *(pointer++)=0xED;
    *(pointer++)=0xED;
    *(pointer++)=0;
    if (fwrite(buffer,pointer-buffer,1,bestand)!=1) goto writeerr;
    if (fclose(bestand)) goto writeerr;
    return 0;
}

void addextension(fnam,ext)
char *fnam;
char *ext;
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


void autocopy(char mode)
{
    int loop_nr,nr,offset,x,out_x;
    FILE *bestand;
    unsigned char ch,in_name[20],out_name[20];

    buf_track=-1;
    printmes=0;
    for (loop_nr=1;loop_nr<81;loop_nr++)
    {
        nr=loop_nr-1;
        leessector(nr/20,0,(nr/2)%10);
        offset=256*(nr%2);
        if ((secbuf[offset]==0) && (secbuf[offset+1]==0))
            loop_nr = 999; /* Stop early if rest of directory unused */
        ch=secbuf[offset];
        if ((mode && ch && ((ch<=4)||(ch==7)))||(ch==5)||(ch==9)) {
            out_x = 0;
            for (x=1;x<=10;x++)
            {
                ch = secbuf[offset+x];
                in_name[x-1] = ch;
                if ((ch>='a')&&(ch<='z')) ch&=0xDF;
                if (((ch>='0')&&(ch<='9'))||((ch>='A')&&(ch<='Z')))
                   out_name[out_x++]=ch;
            }
            in_name[10] = 0;
            if (out_x>8) out_x=8;
            out_name[out_x]=0;
            if (secbuf[offset]<5) addextension(out_name,".TAP"); else
               if (secbuf[offset]==7) addextension(out_name,".SCR"); else
                  addextension(out_name,".Z80");
            printf ("Transferring %s to %-12s ",in_name,out_name);
            bestand=fopen(out_name,"r");
            if (bestand==NULL)
            {
                bestand=fopen(out_name,"wb");
                if (bestand==NULL)
                    printf(" - Failed\n");
                else
                {
                    if (copyfile(nr,bestand)==0)
                        printf(" - OK\n");
                    else
                        printf(" - Failed\n");
                }
            }
            else
            {
                fclose(bestand);
                printf(" - Exists\n");
            }
        }
    }
    printmes=1;
    toets();
}



void main(int argc, char *argv[])
{
    int nr;
    char ch;
    FILE *bestand;
    printmes=1;
    trackbuf=(unsigned char*)malloc(5120);
    for (nr=0;nr<8;nr++) pagebuf[nr]=(unsigned char*)malloc(16384);
    buffer=(unsigned char*)malloc(50000L);
    if (buffer==NULL) {
        printf ("Insufficient memory. Need about 200K\n");
        exit(1);
    }
    clr();
    if ((argc==2)&&
        ((!argv[1][2])||((!argv[1][4])&&(argv[1][3]=='0')&&(argv[1][2]=='4')))&&
        (argv[1][1]==':')&&
        (((argv[1][0]-1)&0xDE)==64)) {
            drive = 1- (argv[1][0] & 1);
            if (argv[1][2]=='4') {
               eightytracks=0;
               puts ("Assuming 40 track disks.");
            }
    } else {
        printf("Specify A: or B: as disciple/+D disk drive.\n");
        printf("Use A:40 or B:40 to read 40 track disks (default is 80).\n");
        exit(1);
    }
    do {
        keys:
        do {
            clr();
            puts ("Menu:\n");
            puts ("1] Show directory of files on DISCiPLE/+D disk");
            puts ("2] Copy file from DISCiPLE/+D disk");
            puts ("3] Copy snapshot to .Z80 file");
            puts ("4] Copy screen snapshot to .SCR file");
            puts ("5] Convert all program snapshots");
            puts ("6] Convert all files on the disk");
            puts ("9] Exit to dos\n");
            name[0]=0;
            gets(name);
        } while (((name[0]<'0')||(name[0]>'6'))&&(name[0]!='9'));
        printf("\n");
        ch=name[0]+1;
        switch (ch) {
        case '1':
            drive = 1-drive;
            break;
        case '2':
            clrscr();
            getmbyte();
            listfiles();
            break;
        case '3':
            printf ("Copying DISCiPLE/+D file.\n");
            goto copyfile;
        case '4':
            printf ("Converting DISCiPLE/+D snapshot to .Z80 file.\n");
            goto copyfile;
        case '5':
            printf ("Converting DISCiPLE/+D screen snapshot to .SCR file.\n");
            copyfile:
            printf ("Enter the file number: ");
            nr=0;
            scanf("%d",&nr);
            if ((nr<1)||(nr>80)) {
            puts ("File numbers must be in range 1-80.\n");
                toets();
                goto keys;
            }
            getmbyte();
            nr--;
            buf_track=-1;
            leessector(nr/20,0,(nr/2)%10);
            switch(secbuf[(nr%2)*256]) {
            case 0:printf ("That file does not exist.\n");
                toets();
                goto keys;
            case 1:
            case 2:
            case 3:
            case 4:if (ch!='3') {
                      printf ("Indicated file is a normal data file!  Use menu option 2 to copy this file.\n");
                      toets();
                      goto keys;
                  }
                  break;
            case 5:
            case 9:if (ch!='4') {
                      printf("Indicated file is a snapshot file!  Use menu option 3 to convert it.\n");
                      toets();
                      goto keys;
                  }
                  break;
            case 7:if (ch=='4') {
                      printf("Indicated file is a screen snapshot!  Use menu options 2 or 4 to convert it.\n");
                      toets();
                      goto keys;
                  }
                  break;
            default:printf ("Indicated file is of unknown or unsupported type.\n");
                    toets();
                    goto keys;
            }
            printf ("DISCiPLE/+D file: \"%.10s\"\n",secbuf+(nr%2)*256+1);
            if ((ch=='3')&&(secbuf[(nr%2)*256]!=7)) {
               printf ("Enter the name of the targetfile (.TAP implied):  ");
               scanf("%s",name);
               addextension(name,".TAP");
            } else if ((ch=='5')||(ch=='3')) {
              printf ("Enter the name of the .SCR file for output:  ");
              scanf("%s",name);
              addextension(name,".SCR");
            } else {
              printf ("Enter the name of the .Z80 file for output:  ");
              scanf("%s",name);
              addextension(name,".Z80");
            }
            bestand=fopen(name,"wb");
            if (bestand==NULL) {
               printf ("Error while creating output file %s\n",name);
               toets();
               goto keys;
            }
            nr = copyfile(nr,bestand);
            if (nr==1)
               printf("Error writing to %s\n",name);
               toets();
            break;
        case '6':
        case '7':
             getmbyte();
             autocopy(ch-'6');
             break;
        case '9'+1:
             *mbyte=oldmbyte;
             exit(0);
        }
    } while (1);
}

