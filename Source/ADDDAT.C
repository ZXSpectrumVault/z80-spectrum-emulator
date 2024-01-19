/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 3.04, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                     Gerton Lunter, 10/8/96
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define word unsigned int
#define byte unsigned char

struct header {
    byte ra,rf;                 /* version 1 stuff */
    word rbc,rhl,rpc,rsp;
    byte ri,rr,rr_bit7;
    word rde,rbca,rdea,rhla;
    byte raa,rfa;
    word riy,rix;
    byte iff,iff2,imode;
    word length;                /* version 2 stuff */
    word rpc2;
    byte rommod;
    byte hstate;
    byte if1flg;
    byte mflag;
    byte sreg;
    byte sstate[16];
    word tstates;               /* version 3 stuff */
    byte viertel;
    byte zero;
    byte discflg;
    byte m128flg;
    byte rompage1,rompage2;
    word udef_joy[5];
    word udef_keys[5];
    byte disctype;
    byte discinhibit;
    byte discinhibitflg;
} buffer;

struct blockheader {
    word length;
    char page;
} blockhdr;

struct sltheader {
    word type;
    word level;
    long length;
} slt;

word i;
int level;
byte *buf1,*buf2;
FILE *fin,*fout,*fext;
char finname[128],foutname[128];
long datsize,pos;
word dataddsize;
int change,infile,outfile,deletelev,deletescr,border,scrcount,scrdowncount;
long blockoffset,blocklen;
long zero=0;


int main(int,char**);
word compress(byte*,byte*,word);
word unpack2(byte*,byte*,word);
void writeerror(void);


void writeerror()
{
    printf ("Error writing to file!  (Disk full?)\n");
    exit(1);
}

#define fw(buf,n,f) if (fwrite(buf,1,n,f)!=n) writeerror();

word compress(byte *source,byte *dest,word length)
{
    byte tel=0,byt,k;
    word len=0;
    do {
        if (tel==0) {
            byt=*(source++);
            tel=1;
            length--;
        } else if ((tel!=255)&&(byt==*source)&&(length!=0)) {
            source++;
            tel++;
            length--;
        } else if ((tel>=5)||((byt==0xED)&&(tel>=2))) {
            *(dest++)=0xED;
            *(dest++)=0xED;
            *(dest++)=tel;
            *(dest++)=byt;
            len+=4;
            tel=0;
        } else {
            for (k=0;k<tel;k++) *(dest++)=byt;
            len+=tel;
            tel=0;
            if ((byt==0xED)&&(length!=0)) {
                *(dest++)=*(source++);
                length--;
                len++;
            }
        }
    } while ((length!=0)||(tel!=0));
    return(len);
}


word unpack2(inp,outp,size)
// unpacks 'size' input bytes
byte *inp,*outp;
word size;
{
    register word incount=0,outcount=0;
    word i;
    byte j;
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
    } while (incount<size);
    if (incount!=size) outcount=0;
    return (outcount);
}


void copyblock(FILE *fin, FILE *fout, long len, char *buf)
{
    int i,j;
    i=j=0;
    while (len&&(i==j)) {
          if (len>16384)
             i=16384;
          else
              i=len;
          len-=i;
          j=fread(buf,1,i,fin);
          fw(buf,j,fout);
    }
}


void openz80file(FILE **fin, char *name, char *mode)
{
    int i;
    finname[0]=0;
    strcat(finname,name);
    *fin=fopen(finname,mode);
    if (*fin==NULL) {
       for (i=0;finname[i];i++);
       strcat(finname,".Z80");
       *fin=fopen(finname,mode);
       if (*fin==NULL) {
          finname[i]=0;
          strcat(finname,".SLT");
          *fin=fopen(finname,mode);
       }
    }
    if (*fin==NULL) {
	   printf ("File %s not found.\n",finname);
       exit(1);
    }
}



int skipinitialrubbish(FILE *fin, FILE *fout)
// returns 0 if Z80 file has no SLT block (fin -> eof)
// returns 3 if Z80 file has got SLT block (fin -> 'S','L','T' id.)
// copies Z80 file if fout!=0
{
    word i;
    if (fread(&buffer,1,30,fin)!=30) {
       errfile:
       printf ("Error while reading file %s.\n",finname);
       exit(1);
    }
    if (fout) fw(&buffer,30,fout);
    if (buffer.rpc!=0) {
        printf ("Error: Can only handle .Z80 files of version 2.0 or higher\n");
        exit(1);
    }
    if (fread(&(buffer.length),1,2,fin)!=2) goto errfile;
    if (fout) fw(&(buffer.length),2,fout);
    if (buffer.length > sizeof(buffer)-32) {
       printf ("ERROR!  Can only handle .Z80 files up to v3.04 (ie. secondary header\n54 bytes)\n");
       exit(1);
    }
    fread(&(buffer.rpc2),1,buffer.length,fin);
    if (fout) fw(&(buffer.rpc2),buffer.length,fout);
    do {
        i=fread(&blockhdr,1,3,fin);
        if (fout) fw(&blockhdr,i,fout);
        if ((i==3)&&(blockhdr.length!=0)) {
           if (fout)
              copyblock(fin,fout,blockhdr.length,buf1);
           else
               fseek(fin,blockhdr.length,SEEK_CUR);
           i=0xffff;
        }
    } while (i==0xffff);
    return(i);
}


int main(teller,str)
int teller;
char **str;
{
    printf ("ADDDAT - Add/Delete/Extract level/screen data to/from Z80/SLT file - v 3.04\n");
    printf ("         This program may not be distributed.\n");
    printf ("         It is part of the registered Spectrum Emulator package.\n\n\n");
    if (teller<3) {
       help:
       puts ("Syntax:\n");
       puts ("Add level data:      ADDDAT  -a  file[.z80|.slt]  level_number  datafile.DAT");
       puts ("Remove level:        ADDDAT  -d  file[.z80|.slt]  level_number  ");
       puts ("Extract level:       ADDDAT  -x  file[.z80|.slt]  level_number  datafile.DAT");
       puts ("Add loading screen:  ADDDAT  -s  file[.z80|.slt]  screen.SCR    [border colour]");
       puts ("Delete screen:       ADDDAT  -c  file[.z80|.slt]  [screen nmbr]");
       puts ("Extract screen:      ADDDAT  -z  file[.z80|.slt]  screen.SCR    [screen nmbr]");
       puts ("Transfer SLT block:  ADDDAT  -t  file[.z80|.slt]  target[.z80|.slt]");
       puts ("\n-s (add loading screen): If border colour is missing, 0 (black) is assumed.");
       puts ("-c/-z (delete/extract screen): If screen number is absent, 1 (first) is assumed.");
	   return(1);
    }
    if (strlen(str[1])!=2) goto help;
    infile=outfile=change=deletelev=deletescr=0;
    switch (str[1][1]&0xDF) {
        case 'A':
            if (teller!=5) goto help;
            level=atoi(str[3]);
            change=1;
            infile=4;
            break;
        case 'D':
            if (teller!=4) goto help;
            level=atoi(str[3]);
            change=1;
            deletelev=1;
            break;
        case 'X':
            if (teller!=5) goto help;
            level=atoi(str[3]);
            outfile=4;
            break;
        case 'S':
            if ((teller!=4)&&(teller!=5)) goto help;
            if (teller==5) border=atoi(str[4]); else border=0;
            infile=3;
            change=1;
            level=-1;       // signal: screen
            break;
        case 'C':
            if ((teller!=3)&&(teller!=4)) goto help;
            if (teller==4) scrcount=atoi(str[3]); else scrcount=1;
            change=1;
            deletescr=1;
            level=-1;       // signal: screen
            break;
        case 'Z':
            if ((teller!=4)&&(teller!=5)) goto help;
            if (teller==5) scrcount=atoi(str[4]); else scrcount=1;
            outfile=3;
            level=-1;       // signal: screen
            break;
        case 'T':
            openz80file(&fout,str[3],"r+b");
            setbuf(fout,NULL);
            // note: buf1 is not used in this call of skipinitialrubbish
            if (skipinitialrubbish(fout,NULL)==0) {
                // add first half of SLT separator
                fw("\0\0\0",3,fout);
            }
            // The following is necessary for unknown reason...
            // If it is not done, next fwrite to fout starts 0x1fd bytes
            //  beyond current file pointer
            pos=ftell(fout);
            fclose(fout);
            fopen(finname,"r+b");
            fseek(fout,pos,SEEK_SET);
            strcpy(foutname,finname);
            // End of dubious section
            level=-2;       // signal: transfer SLT block
            break;
        default:
            goto help;
    }
    openz80file(&fin,str[2],"rb");
    buf2=(byte*)malloc(16384);
    buf1=(byte*)malloc(50000L);
    if (buf1==NULL) {
        printf ("Not enough memory.\n");
        return(1);
    }
    if (level==-2) {
        i=skipinitialrubbish(fin,NULL);
        if (i==0) {
            printf ("Warning: No SLT block found in source file %s!\n",finname);
            fw ("SLT\0\0\0\0\0\0\0\0",11,fout);
        } else {
            printf ("Copying SLT block from source file %s\n",finname);
            copyblock(fin,fout,999999L,buf2);
        }
        fclose(fin);
        fw(buf1,0,fout);
        fclose(fout);
        return(0);
    }
    if (change) {
        fout=tmpfile();
        if (fout==NULL) {
            printf ("Error: Could not open temporary file\n");
            return(1);
        }
    }
    if (infile) {
        fext=fopen(str[infile],"rb");
        if (fext==NULL) {
            printf ("Error: Could not open file %s for reading.\n",str[infile]);
            return(1);
        }
    }
    if (outfile) {
        fext=fopen(str[outfile],"wb");
        if (fext==NULL) {
            printf ("Error: Could not open file %s for writing.\n",str[outfile]);
            return(1);
        }
    }
    i=skipinitialrubbish(fin,fout);
    datsize=0;
    blockoffset=0;
    blocklen=0;
    if (i==0) {     // There's no SLT block yet
       if (change) {
          fw("\0\0\0SLT",6,fout);
          printf ("Creating SLT block...\n");
       }
    } else {
      fread(&blockhdr,1,3,fin);      // Read and copy rest of "SLT" separator
      if (change) fw(&blockhdr,3,fout);
      fread(&slt,1,8,fin);
      scrdowncount=scrcount;
      while (slt.type) {
          char deletethis;
          deletethis=0;
          // store position if this level is to be deleted
          if ((deletelev)&&(slt.type==1)&&(slt.level==level)) {
            blockoffset=datsize;
            blocklen=slt.length;
            deletethis=1;
          }
          // store position if screen is to be deleted
          if ((deletescr)&&(slt.type==3)) {
            if (!(--scrdowncount)) {
                blockoffset=datsize;
                blocklen=slt.length;
                deletethis=1;
            }
          }
          // store position if screen/level is to be extracted
          // error if level to be added already exists
          if ( ((slt.type==1)&&(slt.level==level)) ||
               ((slt.type==3)&&(level==-1)) ) {
            if (outfile) {
               if ((level!=-1)||(!(--scrdowncount))) {
                   blockoffset=datsize;
                   blocklen=slt.length;
               }
            } else if (infile) {
                if (level!=-1) {
                    printf ("Error: Level %d already exists!\n",level);
                    return(1);
                }
            }
          }
          // add to total length var, copy header block if necessary
          if (slt.type) {
             datsize += slt.length;
             if ((change)&&(!deletethis)) fw(&slt,8,fout);
          }
          fread(&slt,1,8,fin);
      }
    }
    // make header block for new data
    if (infile) {
        dataddsize=fread(buf1+128,1,49152L,fext);
        fclose(fext);
        // if screen
        if (level==-1) {
            if (dataddsize!=6912) {
                printf ("Error: File %s (screen) is not 6912 bytes long!\n",str[infile]);
                return(1);
            }
            printf ("Adding loading screen...\n");
        } else {
          if (dataddsize==0) {
             printf ("Error reading from file %s!\n",str[infile]);
             return(1);
          }
          printf ("Adding level %d data (%u bytes)...\n",level,dataddsize);
        }
        dataddsize=compress(buf1+128,buf1,dataddsize);
        slt.type=(level==-1?3:1);
        slt.level=(level==-1?border:level);
        slt.length=dataddsize;
        fw(&slt,8,fout);
    }
    // write end marker if necessary
    slt.type=0;
    slt.level=0;
    slt.length=0;
    if (change) fw(&slt,8,fout);
    if (change) {
        if (blocklen) {
            if (level==-1)
               printf ("Deleting loading screen...\n");
            else
                printf ("Deleting level %d data...\n",level);
            // copy data before block to be deleted
            copyblock(fin,fout,blockoffset,buf2);
            // skip block to be deleted
            fseek(fin,blocklen,1);
            // copy rest of data
            copyblock(fin,fout,datsize-blockoffset-blocklen,buf2);
        } else {
            if (deletelev || deletescr) {
               printf ("Error: Could not find screen or specified level data to delete!\n");
               return(1);
            }
            // simply copy all data
            copyblock(fin,fout,datsize,buf2);
        }
        // finally, add new data if necessary
        if (infile) {
            fw(buf1,dataddsize,fout);
        }
    } else {
        // now blocksize!=0 if screen/level data is found, 0 otherwise
        if (!blocklen) {
            puts ("Error: Could not find screen or specified level data!");
            return(1);
        }
        fseek(fin,blockoffset,SEEK_CUR);
        fread(&buf1[49900L-blocklen],blocklen,1,fin);
        dataddsize=unpack2(&buf1[49900L-blocklen],buf1,blocklen);
        if ((level==-1)&&(dataddsize!=6912))
            puts ("Warning: screen doesn't expand to 6912 bytes.");
        printf ("Writing %u bytes to file %s...\n",dataddsize,str[outfile]);
        fw(buf1,dataddsize,fext);
        fclose(fext);
    }
    fclose(fin);
    if (change) {
        fw(buf1,0,fout);
        fseek(fout,0,SEEK_SET);
        fin=fopen(finname,"wb");
        copyblock(fout,fin,999999L,buf1);
        fclose(fin);
    }
    return(0);
}



