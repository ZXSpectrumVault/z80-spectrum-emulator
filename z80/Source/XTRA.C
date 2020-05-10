/*
Recommended compiler/linker: BCC v4.02, TLINK v5.1
You ought to get no warnings except three 'possibly incorrect assignment' ones.
*/


#define xwmax 34
#define ywmax 16
#define xwind 1
#define ywind 3

#define NULL ((unsigned char*)0L)
#define word unsigned int
#define byte unsigned char

#define swap(a,b) {int swp=a;a=b;b=swp;}
#define min(a,b) ((a)<(b)?(a):(b))
#define upc(a) (((a)>='a')&&((a)<='z') ? a&0xDF : a)

#define _tapeinmode 2

struct regs {
	  unsigned int	  rfa,rbc,rhl,rpc,rsp;
	  unsigned char   ri,rr,rr7;
	  unsigned int	  rde,rbca,rdea,rhla,rafa,riy,rix;
	  unsigned char   iff,iff2,imode;
};

struct whereami {
	  int x,y,firstlin;
};

char		Double,rr_emul,flash,ldrflag,gmod,silence,eofil,joymode,winkey,
issue2,syncmod,regist,modflg,tapeif,rommod,romstate,
debugflg;
int 		tapeinmode,tapeoutmode,tapemode,tapeinh,tapeouth,tapewrap,
    tapeeof,taperacc,tapeim2,tapeom2,taperain,taperaout,debugadr;
extern char far zxptype,disctype,discinhibit,pdhandle;	 /* HMc */
extern char far DISCD1Type; 				/* HMc */
extern char far DISCD1Drive;				/* HMc */
extern char far DISCD2Type; 				/* HMc */
extern char far DISCD2Drive;				/* HMc */
extern char far DISCD1FileName[12]; 		/* HMc */
extern char far DISCD2FileName[12]; 		/* HMc */
extern unsigned int far tstates,tstatehi,lasttlo,lastthi,srate,vocdatapoint;
extern unsigned int count200hz;
char    pestmode=0;
extern unsigned int far
    vocdatalen,voctoread,voctoreadhi,vochandle,vocsec,vochsec,nohcrmem;
extern unsigned char far
    outlog,outloghandle,vocplay,vocerror,usercoloremu,tapewindow,m128_on,blaster;
extern unsigned int far outlogtable[];
extern int far udef_joy[5];
extern int far udef_keys[5];
extern int far udef_joy_table[5];
extern char tape_pause;
unsigned int startadr=16384,blocklength=6912,blocktype=1;	/* see extra() */
char		loadpause,voc_msg;
unsigned char *taperabuf;
unsigned char rabuf[50];
int 		rscount,rsoutcount,tapeio;
unsigned int mhandles[8];
unsigned char writprot[8];
int 		innum,outnum;
char		inmode,outmode;
char        xcurs,ycurs,xwmax2,ywmax2,updvid,clrkbd,gkfl_count,screenbagger;
char		attribute;
char		inputfile[129],outputfile[129],prognaam[129],tapein[129],tapeout[129];
char        sprognaam[129];
char		progpath[129];
char		scrfile[129];
char		vocfile[129];
char        outlogfile[129];
char		cartfiles[8][129];
unsigned char *buffer=NULL;
const char  z80sna[]="Z80SNASLT";
const char  taptap[]="TAP";
const char  mdrmdr[]="MDR";
const char  scrscr[]="SCR";
const char  vocvoc[]="VOC";
const char  datdat[]="DAT";
const char	vgamessage[]="If you can read this, use option -xv";
const char	helpfile[]="LAYOUT.SCR";
const int	ynalist[]={'A'};
const int	ynlist[]={'J','Y','N',27,0};
const int	mainmenu[]={2,'M','B','R','L','S','T','C','H','X','O','Q',
'b','c','d','f','g','h','i',27,0};
const int	changelist[]={'L','H','R','D','C','J','2','V','S','I','O','K',27,0};
const int   tapelist[]={'P','S','I','O','F','R','W','M','B','D','X','g',27,0};
const int	mdrvlist[]={'W','I','O','8','7','6','5','4','3','2','1'};
const int   extralist[]={'D','S','L','R','N','M','B','O',27,0};
const int	extra2list[]={'1','2','I','B','Z',27,0};   /* HMc */
const int	fivelist[]={13,10,327,335,328,336,'M','T','7','6','5'};
const int	fourlist[]={'4'};
const int	threelist[]={'3'};
const int	twolist[]={'2','1',27,0};
const int	ldrwlist[]={'C','L','D','R','X',27,0};
struct regs 	*regpointer;
struct whereami scrstate,loadstate,redirectstate,tapestate,mdrvstate,vocstate;
char			*specpointer,*prog,window[xwmax][ywmax],screen[xwmax][ywmax];
char			firsttime=1,cfcleared=0,tomenu;
char			defdir[65],dir1[65],tapdir[65],rtapdir[65],rs232dir[65],
mdrdir[65],scrdir[65],vocdir[65];

int far 		entry(unsigned char action_code);
void			first_func(void);
void			init(int);
int 			mswitch(int,char*,int*);
void			printmes(void);
int 			tswitch(char **);
void			prw(char ch);
void			prstr(char *strng);
void			prcenter(int y, char *strng);
void			prwind(void);
void			unhilite(void);
void			vernachelflop(void);
void			intzero(void);
void			clearwindow(void);
void			prat(int x, int y, char *strng);
void			prat_file(int x,int y, char *strng);
void			prgetal(unsigned int,int);
void			prhexgetal(unsigned int);
void			help(void);
void			altf1(void);
void			regmsg(void);
void            discmsg(void);
void            load_prog(void);
int 			editstring(char *strng);
unsigned int	hex(char *);
unsigned int    dechex(char *);
unsigned int    number(char *);
word            unpack2(byte*,byte*,word,word);
void			save_prog(void);
void            tape(char);
void			mdrv(void);
void			change(void);
void			joystick(void);
void			pruserdefjoy(int j);
void			to_dos(int);
void			display_inst(void);
void			printinst(char x, char y, char z);
int 			addz80(char *naam,char *extensie);
void            substz80(char *, char *);
void			change_input(void);
void			change_output(void);
void			change_menu(char *inout,char *mode,int *num, char *name);
void			flushoutputC(int handle);
int 			suretoquit(void);
int 			getkeyfromlist (const int *keylist);
int 			di_halt(void);
int 			in_zero(void);
int 			zero(int adres);
void			saveloadblock(int type);
void			clearkeybuffer(void);
void			clrlin(int);
void			qsort(int,int);
int 			choose(char,char*,struct whereami*);
int 			real_mode(void);
char			fileorder(char*,char*);
void			browsetapefile(void);
void			voc_error(void);
char			extra(void);
void			extra2(void);					/* HMc */
extern void far 		printchar(int x, int y, char ch);
extern void far 		update_video(int);
extern void far 		init_emulator(void);
extern void far 		install_all(void);
extern void far 		exit_emulator(void);
extern void far 		clearkeyboard(void);
extern unsigned int far open_file(char *name, char access_code);
extern unsigned int far creat_file(char *name);
extern int far			close_file(int handle);
extern int far			read_file(int handle, char *pointer, int bytes);
extern int far			write_file(int handle, char *pointer, int bytes);
extern int far			commit(int handle);
extern int far			lseek(int,unsigned int*,unsigned int*,char);
extern int far			getkey(void);
extern int far			shell(void);
extern int far			shell_flushbuf(void);
extern int far			flushoutput(void);
extern void far 		beep(void);
extern void far 		clrinp(void);
extern void far 		clroutp(void);
extern int far			spacebar(void);
extern int far			findfirst(char,char*,unsigned char*);
extern int far			findnext(unsigned char*);
extern int far			findfirst_ra(unsigned char*);
extern int far			findnext_ra(unsigned char*);
extern int far			inpakken(void);
extern int far			chdir(char*);
extern void far 		getdir(char*);
extern int far			hoesnel(void);
extern void far 		speed_convert(char*);
extern void far 		maakinstelbits(void);
extern int far			loadblock(void);
extern int far			saveblock(void);
extern void far 		getissue2default(int);
extern int far			testfordrive(int);
extern int far			loaddiskblock(unsigned int,unsigned int,int);
extern int far			savediskblock(unsigned int,unsigned int,unsigned int,int,int,unsigned char);
int 					savediskblock_1(unsigned int,unsigned int,unsigned int,int,int,unsigned char);
extern void far 		update_regs_save(void);
extern unsigned int far memalloc(unsigned int);
extern void             memfree(unsigned int);
extern void far 		pauze(void);
extern void far 		flush_mdrv_buffer(void);
extern void far 		press_break(void);
extern void far 		mdrv_reset(void);
extern int far			load_z80_file(int,int);
extern int far			save_z80_file(int);
extern void far 		init_emulmode(int,int);
extern void far 		set_nmireset(int);
extern void far 		select_page5(void);
extern void far 		reset_page5(void);
extern void far 		delete_tapblock(int handle);
extern unsigned long far voc_position(void);
extern int far			windvoc(unsigned int);
extern void far         outlog_flushbuf(void);

#define SHELL 0
#define QUIT 1

void first_func(void) {}

void init(int options)
{
	  int i,j;
	  unsigned int mem;
	  ywmax2=ywmax;
      xwmax2=xwmax;
	  for (i=0;i<xwmax;i++) for (j=0;j<ywmax;j++) window [i][j]=' ';
	  screenbagger=1;
	  xcurs=0;
	  ycurs=0;
	  attribute=0;
	  tomenu=0;
	  gkfl_count=0;
	  if ((!options)&&(buffer==NULL)) {
		  mem=memalloc(1024);
		  if (mem) buffer=(unsigned char *)(((unsigned long)mem)<<16); else {
			  clearwindow();
			  prat (1,2,"ERROR - Not enough memory!");
              prat (1,3,"If you're not already using it,");
              prat (1,4,"using switch -xt at startup will");
              prat (1,5,"help you out.");
              prat (1,7,"Press any key to quit.");
			  prwind();
			  do {} while (getkey());
			  do {} while (!getkey());
			  to_dos(QUIT);
		  }
	  }
	  if (firsttime) {
		  firsttime=0;
		  getdir(defdir);
		  getdir(dir1);
		  getdir(rs232dir);
		  getdir(tapdir);
		  getdir(rtapdir);
		  getdir(mdrdir);
		  getdir(scrdir);
		  getdir(vocdir);
		  vocstate.x=0;
		  vocstate.y=0;
		  vocstate.firstlin=0;
		  loadstate.x=0;
		  loadstate.y=0;
		  loadstate.firstlin=0;
		  redirectstate.x=0;
		  redirectstate.y=0;
		  redirectstate.firstlin=0;
		  tapestate.x=0;
		  tapestate.y=0;
		  tapestate.firstlin=0;
		  mdrvstate.x=0;
		  mdrvstate.y=0;
		  mdrvstate.firstlin=0;
		  scrstate.x=0;
		  scrstate.y=0;
		  scrstate.firstlin=0;
		  tapeinmode=tapeoutmode=tapeim2=tapeom2=tapeinh=tapeouth=tapemode=
		  tapeeof=taperain=taperaout=0;
		  tapewrap=taperacc=1;
		  taperabuf=NULL;
		  if (!cfcleared) for (i=0;i<8;i++) {
				   cartfiles[i][0]=0;
				   mhandles[i]=0;
			   }
		  cfcleared=1;
	  }
	  if ((tapemode)&&(tapeouth==0)) tapemode=0;
}

int mswitch(int mdrv,char *name,int *handles)	  /* called from assembly */
{
	  int i;
	  unsigned int j,k;
	  if (!cfcleared) for (i=0;i<8;i++) {
			   cartfiles[i][0]=0;
			   handles[i]=0;
		   }
	  cfcleared=1;
	  if (handles[mdrv]) close_file(handles[mdrv]);
	  for (i=0;i<129;i++) cartfiles[mdrv][i]=name[i];
	  addz80(cartfiles[mdrv],"MDR");
	  chdir(mdrdir);
	  i=open_file(cartfiles[mdrv],2);
	  chdir(defdir);
	  if (i==-1) return(0);
	  handles[mdrv]=i;
	  j=137922L & 0xFFFFL;
	  k=137922L >> 16;
	  lseek(i,&j,&k,0);
	  writprot[mdrv]=0;
	  read_file(i,&(writprot[mdrv]),1);
	  return(1);
}

void printmes() 								/* called from assembly */
{
	  int i;
	  for (i=0;vgamessage[i];i++) printchar(i+2,30,vgamessage[i]);
}


int tswitch(char **cmdlineptr)					/* called from assembly */
{
	  #define singlefile \
	  {if (taperacc) {taperacc=0;tapeim2=tapeinmode;tapeom2=tapeoutmode;\
	  tapeinmode=tapeoutmode=0;}}
	  char name[128],ch,ch2;
	  int i;
	  init(1);
	  switch (ch2=((*cmdlineptr)[0]&0xDF)) {
		  case 'X':
			   tapewindow=!tapewindow;
			   (*cmdlineptr)++;
			   break;
		  case 'M':
			   singlefile
			   tapemode=1;
			   (*cmdlineptr)++;
			   break;
		  case 'E':
			   singlefile
			   tapemode=2;
			   (*cmdlineptr)++;
			   break;
		  case 'W':
			   singlefile
			   tapewrap=!tapewrap;
			   (*cmdlineptr)++;
			   break;
          case 'P':
               tape_pause=!tape_pause;
               break;
		  case 'I':
		  case 'O':
		  case 'D':
		  case 'S':
		  case 'V':
		  case 'N':
			   do {
				   (*cmdlineptr)++;
				   i=(*cmdlineptr)[0];
			   } while ((i==32)||(i==9));
			   if (i==0x0d) return(1);		   /* error on cmd line */
			   i=0;
			   do {
				   name[i++]=(*cmdlineptr)[0];
				   ch=(++(*cmdlineptr))[0];
			   } while ((ch!=32)&&(ch!='/')&&(ch!=9)&&(ch!=0x0d));
			   name[i]=0;
			   switch (ch2) {
				   case 'D':
						if (chdir(name)) return(6); 	/* error while changing dir */
						getdir(rtapdir);
						chdir(defdir);
						tapeinmode=tapeoutmode=1;
						break;
				   case 'S':
						if (chdir(name)) return(6);
						getdir(tapdir);
						chdir(defdir);
						break;
				   case 'V':
                        return(5);
				   default:
						addz80(name,"TAP");
						singlefile
						if (ch2=='I') {
							if (tapeinh) close_file(tapeinh);
							chdir(tapdir);
							tapeinh=open_file(name,_tapeinmode);
							chdir(defdir);
							if (tapeinh==-1) return(2); 	/* file not found */
							for (i=0;i<129;i++) tapein[i]=name[i];
							tapeinmode=1;
						} else {
							if (tapeouth) close_file(tapeouth);
							chdir(tapdir);
							tapeouth=open_file(name,0);
							if (tapeouth!=-1) {
								close_file(tapeouth);
								tapeouth=open_file(name,2);
								i=0;
								lseek(tapeouth,&(unsigned int)i,&(unsigned int)i,2);
							} else {
								tapeouth=creat_file(name);
							}
							chdir(defdir);
							if (tapeouth==-1) return (3);	/* error opening file for output */
							for (i=0;i<129;i++) tapeout[i]=name[i];
							tapeoutmode=1;
						}
			   }
			   break;
		  default:
			   return (1);	   /* error on cmd line */
	  }
	  return (0);  /* OK */
}

int xswitch(char **cmdlineptr)					/* called from assembly */
{
	  char name[128],ch,ch2=**cmdlineptr;
	  int i;
	  init(1);
	  do {
		  (*cmdlineptr)++;
		  i=(*cmdlineptr)[0];
	  } while ((i==32)||(i==9));
	  if (i==0x0d) return(1);		  /* error on cmd line */
	  i=0;
	  do {
		  name[i++]=(*cmdlineptr)[0];
		  ch=(++(*cmdlineptr))[0];
	  } while ((ch!=32)&&(ch!='/')&&(ch!=9)&&(ch!=0x0d));
	  name[i]=0;
	  switch (ch2&0xDF) {
		  case 'M':
			   if (chdir(name)) return(6);	   /* error while changing dir */
			   getdir(mdrdir);
			   chdir(defdir);
			   break;
		  case 'P':
			   if (chdir(name)) return(6);
			   getdir(dir1);
			   chdir(defdir);
			   break;
		  default:
			   return (1);
	  }
	  return(0);
}


void prw(ch)
char	ch;
{
	  if (ch==13) {
		  xcurs=1;
		  if (ycurs<ywmax) ycurs++;
	  } else {
		  window [xcurs++][ycurs]=(ch|attribute);
          if (xcurs>=xwmax2) xcurs=0;
	  }
}

void prstr(strng)
char	*strng;
{
	  int i;
	  for (i=0;strng[i];i++) prw(strng[i]);
}

void prat(x,y,strng)
char	*strng;
int 	x,y;
{
      xcurs=x % xwmax2;
	  ycurs=y % ywmax2;
	  prstr(strng);
}

void prcenter(y,strng)
char *strng;
int y;
{
	  for (xcurs=0;strng[xcurs];xcurs++);
      xcurs=(xwmax2-xcurs)/2;
	  ycurs=y % ywmax2;
	  prstr(strng);
}

void prat_file(x,y,strng)
char *strng;
int x,y;
{
	  int p;
	  for (p=0;strng[p];p++) {}
      if (p+x<xwmax2) p=0; else {
		  prat (x,y,"...");
		  x+=3;
          p-=xwmax2-x-1;
	  }
	  prat (x,y,strng+p);
}


void prgetal(x,leadzero)
unsigned int x;
int leadzero;
{
	  int y=10000;
	  char c,f=0;
	  do {
		  for (c='0';x>=y;x-=y) c++;
		  if (f|=((c!='0')||(y==leadzero))) prw(c);
	  } while (y/=10);
}


void prhexgetal(x)
unsigned int x;
{
	  char c,y=16;
	  do {
		  c=(x>>(y-=4))&15;
		  prw(c>9 ? c+'7' : c+'0');
	  } while (y);
}


void clrlin(line)
int line;
{
	  int x;
      for (x=1;x<xwmax2-1;x++) window[x][line]=(32|attribute);
}

void unhilite(void)
{
	  int x,y;
      for (x=0;x<xwmax2;x++) for (y=0;y<ywmax2;y++) window[x][y]&=0x7f;
}

void clearwindow(void)
{
	  int x,y;
	  char c;
      for (x=0;x<xwmax2;x++) {
		  for (y=0;y<ywmax2;y++) {
			  c=32;
			  if ((y==0)||(y==ywmax2-1)) c=2;
              if ((x==0)||(x==xwmax2-1))
				   if (c!=2)
						c=4;
				   else {
					   if (x==0) {
						   if (y==0)
								c=1;
						   else
								c=5;
					   } else if (y==0)
							c=3;
					   else
							c=6;
				   }
			  window[x][y]=c;
		  }
	  }
}

void prwind(void)
{
	  int i,j;
      for (i=0;i<xwmax2;i++) {
		  for (j=0;j<ywmax2;j++) {
			  if ((screen[i][j]!=window[i][j])||(screenbagger))
				   printchar(i+xwind,j+ywind,screen[i][j]=window[i][j]);
		  }
	  }
	  screenbagger=0;
}

int suretoquit(void)
{
	  int x;
	  clearwindow();
	  prat (1,1,"Quit the ZX Spectrum Emulator");
	  prcenter (7,"Are you sure? (Y/N)");
	  prwind();
	  x=getkeyfromlist(ynlist);
	  return ((x=='J')||(x=='Y'));
}


int editstring(strng)
char	*strng;
{
	  #define nochange 1
	  #define ctrlenter 2
	  int x=xcurs,ch=1,totlen=0x80;
      unsigned char reglen=xwmax2-xcurs-1,len,temp,xx,erase;
	  if (strng[0]=='\005') {
		  totlen=5;
		  len=0;
	  } else for (len=0;(len<=totlen)&&(strng[len]!=0);len++) {}
		   strng[len]=0;
	  erase=len;
	  do {
		  xcurs=x;
		  if (len<reglen) temp=0; else {
			  temp=4+len-reglen;
			  prstr("...");
		  } while (strng[temp]!=0) prw(strng[temp++]);
		  xx=xcurs;
		  prw('_');
          while (xcurs<xwmax2-1) prw(' ');
		  prwind();
		  if (ch==0) ch=13; else
			   ch=getkey();
		  if (ch==0) ch=1;
		  switch (ch) {
			  case 13:
			  case 10:
			  case 1:
				   break;
			  case 8:
				   if (len!=0) strng[--len]=0; else beep();
				   erase=0;
				   break;
			  case 27:
			  case 256+68:
				   if ((ch==27)&&(erase)) {
					   erase=len=strng[0]=0;
					   ch=' ';
				   } else {
					   if (ch==256+68) tomenu=1;
					   ch=len=strng[1]=0;
					   strng[0]=27;
				   }
				   break;
			  default:
				   if ( (len<totlen)&&(ch>31)&&(ch<0x7B) ) {
					   if (erase) len=0;
					   strng[len++]=ch;
					   strng[len]=0;
				   } else if (ch!=1) beep();
				   erase=0;
		  }
	  } while ((ch!=13)&&(ch!=10));
	  xcurs=xx;
	  prw(' ');
	  prwind();
	  if (erase) erase=nochange;
	  if (ch==10) erase|=ctrlenter;
	  return(erase);
}

unsigned int hex(string)
char *string;
{
	  unsigned int h=0,i=0,c;
	  while (c=(unsigned char)string[i]) {
		  h<<=4;
		  if ((c-='0')>48) c-=32;
		  if (c>16) c-=7;
		  if (c>15) return(0);
		  h+=c;
		  i++;
	  }
	  return(h);
}


unsigned int number(string)
char *string;
{
      unsigned int i=0,c;
      long h=0;
	  while (c=(unsigned char)string[i]) {
          h*=10;
          if (c>'9') return(0);
          if (c<'0') return(0);
          h+=c-'0';
		  i++;
          if (h>0xffff) return(0);
	  }
      return((unsigned int)h);
}

unsigned int dechex(string)
char *string;
{
      if ((string[0]=='$')||(string[0]=='#'))
        return(hex(string+1));
      else
        return(number(string));
}


word unpack2(inp,outp,size,maxout)
// unpacks 'size' input bytes, but stores at most 'maxout' bytes
byte *inp,*outp;
word size;
word maxout;
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
            if (outcount>maxout) return(maxout);
            for (;i!=0;i--) *(outp++)=j;
       } else {
         incount++;
         outcount++;
         if (outcount>maxout) return(maxout);
         *(outp++)=*(inp++);
       }
    } while (incount<size);
    if (incount!=size) outcount=0;
    return (outcount);
}



char fileorder(fil1,fil2)
char *fil1,*fil2;
{
	  char t;
	  if ((*fil1&0x40)<(*fil2&0x40)) return(1);
	  if ((*fil1&0x40)>(*fil2&0x40)) return(0);
	  if ((*fil1)>(*fil2)) return(1);
	  if ((*fil1)<(*fil2)) return(0);
	  for (t=1;*(fil1+t);t++) {
		  if ((*(fil1+t))<(*(fil2+t))) return(1);
		  if ((*(fil1+t))>(*(fil2+t))) return(0);
	  }
	  return(1);
}

void qsort(p,q)
int p,q;
{
	  #define attr_drive 0x40
	  int r,s,y;
	  unsigned char c;
	  if (p+1<q) {
		  s=(p+q-1)/2;
		  for (y=0;y<16;y++) {
			  c=buffer[16*s+y];
			  buffer[16*s+y]=buffer[16*p+y];
			  buffer[16*p+y]=c;
		  }
		  r=p;
		  s=q-1;
		  while (r!=s) {
			  if (fileorder(buffer+16*(r+1),buffer+16*p)) r++; else
			  if (fileorder(buffer+16*p,buffer+16*s)) s--; else {
				  for (y=0;y<16;y++) {
					  c=buffer[16*(r+1)+y];
					  buffer[16*(r+1)+y]=buffer[16*s+y];
					  buffer[16*s+y]=c;
				  }
				  r++;
				  s--;
			  }
		  }
		  for (y=0;y<16;y++) {
			  c=buffer[16*r+y];
			  buffer[16*r+y]=buffer[16*p+y];
			  buffer[16*p+y]=c;
		  }
		  qsort(p,r);
		  qsort(r+1,q);
	  }
}

int choose(onlyz80,filename,wobinich)
char onlyz80;
char *filename;
struct whereami* wobinich;
{
	  /* returns 1 if left with CTRL ENTER */
	  #define attr_normal 0x21
	  #define attr_subdir 0x10
	  #define attr_drive 0x40
	  #define kar(z) (*(buffer+16*x+(z)))
	  const char *ext;
	  int x,y,result,n;
	  int xx,yy;
	  int firstlin;
	  char *curnam,*bufp1;
	  char fast;
	  char ended;
	  switch (onlyz80) {
		  case 0:
			   chdir(rs232dir);
			   break;
		  case 1:
			   ext=&(z80sna[0]);
			   chdir(dir1);
			   break;
		  case 2:
			   ext=&(taptap[0]);
			   chdir(tapdir);
			   break;
		  case 3:
			   ext=&(mdrmdr[0]);
			   chdir(mdrdir);
			   break;
		  case 4:
			   ext=&(scrscr[0]);
			   chdir(scrdir);
			   break;
		  case 5:
			   ext=&(taptap[0]);
			   chdir(rtapdir);
			   break;
		  case 6:
			   ext=&(vocvoc[0]);
			   chdir(vocdir);
			   break;
		  case 7:
			   chdir(scrdir);
			   break;
		  case 8:
			   ext=&(datdat[0]);
			   chdir(dir1);
			   break;
	  }
	  for (y=3;y<=ywmax2-2;y++) clrlin(y);
	  if (chdir(filename)!=0) {
		  return(0);
	  }
	  do {
		  x=y=0;
		  while (y<26) if (testfordrive(y++)) {
				   kar(0)=attr_drive;
				   kar(1)='[';
				   kar(2)=64+y;
				   kar(3)=':';
				   kar(4)=']';
				   kar(5)=0;
				   x++;
			   }
		  result=findfirst(attr_normal+attr_subdir,"*.*",buffer+16*x);
		  while ((result==0)&&(x<1000)) {
              if (!(kar(0)&=attr_subdir)) {
                for (y=1;kar(y);y++)
                    if ((kar(y)>='A')&&(kar(y)<='Z'))
                        kar(y) |= 0x20;
              }
			  kar(0)|=1;
			  y=1;
			  while ((kar(y)!=0)&&(kar(y)!='.')) y++;
              if (!((kar(1)=='.')&&(kar(2)==0))) {
                int k;
                if ((onlyz80==0)||(onlyz80==7)||(kar(0)&attr_subdir)) x++; else {
                    n=0;
                    k=0;
                    while (ext[k]) {
                        if ((kar(y)=='.')&&
                            (upc(kar(y+1))==ext[k])&&
                            (upc(kar(y+2))==ext[k+1])&&
                            (upc(kar(y+3))==ext[k+2])) n=1;
                        k+=3;
                    }
                    if (n) x++;
                }
              }
//              if (  ( ((kar(y)=='.')&&(kar(y+1)==ext[0])&&(kar(y+2)==ext[1])&&(kar(y+3)==ext[2]))||
//                      ((kar(y)=='.')&&(kar(y+1)==ext[3])&&(kar(y+2)==ext[4])&&(kar(y+3)==ext[5]))||
//                      (onlyz80==0)||
//                      (onlyz80==7)||
//                      (kar(0)&attr_subdir) )
//                &&(!((kar(1)=='.')&&(kar(2)==0))))
//                   x++;
			  result=findnext(buffer+16*x);
		  }
		  if (!result) {
			  beep();
			  for (y=3;y<=ywmax2-3;y++) clrlin(y);
			  prat (1,4,"Only first 1000 files shown.");
			  prat (1,5,"Press any key...");
			  prwind();
			  clrlin(4);
			  clrlin(5);
			  clearkeybuffer();
			  do {} while (!getkey());
			  do {} while (getkey());
		  }
		  kar(0)=0;
		  if (x==0) {
			  kar(0)=attr_subdir+1;
			  kar(1)='.';
			  kar(2)=0;
			  kar(16)=0;
			  x++;
		  }
		  qsort(0,x);
		  n=x;
		  firstlin=wobinich->firstlin;
		  x=wobinich->x;
		  y=wobinich->y;
		  if ((firstlin+x+2*y>n)||(filename[0])) firstlin=x=y=0;
		  fast=0;
		  do {
			  if (firstlin!=0) prat (1,3,"^"); else prat (1,3," ");
			  ended=0;
			  for (n=0;n<2*(ywmax2-6);n++) {
				  xx=(n%2)*15+1;
				  yy=(n/2)+4;
				  bufp1=(char*)buffer+32*firstlin+16*n;
				  if (2*y+x==n) {
					  attribute=0x80;
					  curnam=bufp1;
				  }
				  if (!(*bufp1)) ended=1;
				  prat (xx+1,yy,"             ");
				  if (!ended) {
					  prat (xx+1,yy,bufp1+1);
					  if ((*bufp1)&attr_subdir) prw('\\');
				  }
				  attribute=0;
			  }
			  if ((!ended)&&(*(bufp1+16)))
			  prat (1,ywmax2-2,"\007"); else prat(1,ywmax2-2," ");
			  prwind();
			  clearkeybuffer();
			  do {n=getkey();} while (!n);
			  switch (n) {
				  case 337:   /* pgdn */
					   fast=0;
					   for (xx=0;xx<ywmax2-6;xx++) {
						   if ((*(curnam+16))&&(*(curnam+32))) {
							   curnam+=32;
							   firstlin++;
						   } else {
							   if (y) {y--;firstlin++;}
						   }
					   }
					   break;
				  case 329:   /* pgup */
					   fast=0;
					   for (xx=0;xx<ywmax2-6;xx++) {
						   if (firstlin) firstlin--; else
								if (y) y--;
					   }
					   break;
				  case 327:   /* home */
					   fast=0;
					   x=y=firstlin=0;
					   break;
				  case 335:   /* end */
					   fast=0;
					   while (*(curnam+=16)) {
						   x^=1;
						   if (!x) y++;
						   if (y>=ywmax2-6) {
							   y--;
							   firstlin++;
						   }
					   }
					   break;
				  case 328:   /* up */
					   fast=0;
					   if (firstlin+y>0) {
						   y--;
						   if (y<0) {
							   y=0;
							   firstlin--;
						   }
					   }
					   break;
				  case 336:   /* down */
					   fast=0;
					   if ((*(curnam+16))&&(*(curnam+32))) {
						   y++;
						   if (y>=ywmax2-6) {
							   y--;
							   firstlin++;
						   }
					   } else if ((*(curnam+16))&&(x==1)) {
						   if (y==ywmax2-7) {
							   y--;
							   firstlin++;
						   }
					   }
					   break;
				  case 333:   /* right */
					   fast=0;
					   if (*(curnam+16)) {
						   x^=1;
						   if (!x) y++;
						   if (y>=ywmax2-6) {
							   y--;
							   firstlin++;
						   }
					   }
					   break;
				  case 331:   /* left */
					   fast=0;
					   if (firstlin+x+y>0) {
						   x^=1;
						   if (x) {
							   y--;
							   if (y<0) {
								   y=0;
								   firstlin--;
							   }
						   }
					   }
					   break;
				  case 13:
				  case 10:
				  case 27:
				  case 256+68:
					   unhilite();
					   prwind();
					   break;
				  default:
					   if (((n>='a')&&(n<='z'))||((n>='A')&&(n<='Z'))) {
						   n&=0xDF;
						   first_letter:
						   if (!fast) {
							   x=y=firstlin=0;
							   curnam=buffer;
						   }
						   if (fast==8) break;
						   fast++;
						   while ((*(curnam+16))&&
							 ((*curnam&(attr_subdir+attr_drive))||
                             (upc(*(curnam+fast))<n))) {
							   x^=1;
							   if (!x) y++;
							   if (y>=ywmax2-6) {
								   y--;
								   firstlin++;
							   }
							   curnam+=16;
						   }
                           if (upc(*(curnam+fast))!=n) if (fast==1) fast=0; else {
									fast=0;
									goto first_letter;
								}
					   } else {
						   beep();
						   fast=0;
					   }
			  }
		  } while ((n!=13)&&(n!=10)&&(n!=27)&&(n!=256+68));
		  if ((*(curnam)&(attr_subdir+attr_drive))&&((n==13)||(n==10))) {
			  wobinich->x=0;
			  wobinich->y=0;
			  wobinich->firstlin=0;
			  if (*(curnam)&attr_drive) {
				  *(curnam+4)=0;
				  xx=chdir(curnam+2);
			  } else
				   xx=chdir(curnam+1);
			  if (xx==-1) {
				  filename[0]=27;
				  filename[1]=0;
				  for (y=3;y<=ywmax2-3;y++) clrlin(y);
				  prat (1,3,"Error during MsDos CHDIR.");
				  prat (1,4,"Press any key.");
				  prwind();
				  clearkeybuffer();
				  do {n=getkey();} while (!n);
				  return(0);
			  }
		  }
	  } while ((*(curnam)&(attr_subdir+attr_drive))&&((n==13)||(n==10)));
	  if ((n==13)||(n==10)) {
		  for (xx=0;curnam[xx];xx++) filename[xx]=curnam[xx+1];
		  switch (onlyz80) {
			  case 0:getdir(rs232dir);
				   break;
			  case 1:getdir(dir1);
				   break;
			  case 2:getdir(tapdir);
				   break;
			  case 3:getdir(mdrdir);
				   break;
			  case 4:getdir(scrdir);
				   break;
			  case 5:getdir(rtapdir);
				   break;
			  case 6:getdir(vocdir);
				   break;
			  case 7:getdir(scrdir);
				   break;
			  case 8:getdir(dir1);
				   break;
		  }
	  } else {
		  filename[0]=27;
		  filename[1]=0;
		  if (n==256+68) tomenu=1;
		  if (onlyz80==5) getdir(rtapdir);
	  }
	  wobinich->x=x;
	  wobinich->y=y;
	  wobinich->firstlin=firstlin;
	  return(n==10);
}

int zero(adres)
int 	adres;
{
	  return ((((int*)(0L))[adres/2])==0);
}

void vernachelflop()
{
	  char *p;
	  int h;
	  p=*((char**)(prog+0x2a));
	  p=(char*)( ((long)p)&0xFFFF0000L );
	  while ((p[0]+p[1])!=0) p++;
	  h=creat_file(p+4);
	  write_file(h,(char*)0,0xFFFF);
}

void intzero()
{
	  void (*f)(void);
	  f=*((void**)((long*)(0L)));
	  (*f)();
}

int addz80(naam,extensie)
char	*naam,*extensie;
{
      int i,j=1;
	  for (i=0;naam[i];i++) j=(j||(naam[i]=='\\'))&&(naam[i]!='.');
      if (j) {
		  naam[i]='.';
		  naam[i+1]=extensie[0];
		  naam[i+2]=extensie[1];
		  naam[i+3]=extensie[2];
		  naam[i+4]=0;
	  }
	  return(i);
}

void substz80(naam,extensie)
char	*naam,*extensie;
{
      int i,j=-1;
      for (i=0;naam[i];i++) {
        if (naam[i]=='.') j=i;
        if (naam[i]=='\\') j=-1;
      }
      if (j==-1) j=i;
      naam[j++]='.';
      naam[j++]=extensie[0];
      naam[j++]=extensie[1];
      naam[j++]=extensie[2];
      naam[j]=0;
}


void regmsg()
{
	  clearwindow();
	  prat (1,4, "Sorry -- this option is");
	  prat (1,5, "available to registered users");
	  prat (1,6, "only. Please refer to the manual");
	  prat (1,7, "for registration details.");
	  prat (26,ywmax2-3,"key..");         /* HMc MOD */
	  prwind();
	  do {} while (!getkey());
	  return;
}

void discmsg()
{
      clearwindow();
      prat (1,4,"Your registered copy does not");
      prat (1,5,"emulate the DISCiPLE and +D");
      prat (1,6,"interfaces.");
      prat (26,ywmax2-3,"key..");
      prwind();
      do {} while (!getkey());
      return;
}

int getkeyfromlist (keylist)
const int		*keylist;
{
	  int i,ch;
	  do {
		  if (gkfl_count) {
			  pauze();
			  gkfl_count++;
		  }
		  ch=getkey();
		  if ((ch>='a')&&(ch<='z')) ch&=0xDF;
		  if (ch==256+68) {
			  tomenu=1;
			  return(27);
		  }
		  if ((ch>=256+59)&&(ch<=256+68)) ch='a'+ch-256-59;
		  for (i=0;keylist[i]&&(keylist[i]!=ch);i++);
		  if (!keylist[i]&&ch) beep();
	  } while (!keylist[i]);
	  return (ch);
}

void clearkeybuffer(void)
{
	  do {} while (getkey());
}

void printinst(x,y,z)
char	x,y,z;
{
	  if (z) prat (x,y,"ON "); else prat (x,y,"off");
}

void flushoutputC(handle)
int handle;
{
	  if ((!flushoutput())+(!close_file(handle))) {
		  clearwindow();
		  prat (1,1,"Error while writing to the");
		  prat (2,2,"RS232 channel");
		  prat (1,4,"key...");
		  while (getkey()==0) {}
	  }
}

void change_menu(inout,mode,num,name)
char	*inout,*mode,*name;
int 	*num;
{
	  int i,j,k,inp;
	  char tempnaam[129];
	  inp=(inout[6]=='i');
	  setupscr:
	  clearwindow();
	  prat (1,1,"Change");
	  prat (8,1,inout);
	  prat (1,3,"Current setting:");
	  switch (*mode) {
		  case 0: {prat (2,4,"NONE");
		  break;}
		  case 1: {prat (2,4,"LPT");
			  prw(*num+'1');
		  break;}
		  case 2: {prat (2,4,"COM");
			  prw(*num+'1');
		  break;}
		  case 3: {i=0;
			  xcurs=2;ycurs=4;
              j=xwmax2-10;
			  if (eofil&&inp) j+=2;
			  while ((name[i]!=0)&&(i<j)) prw(name[i++]);
			  xcurs+=j-i;
			  if (inp&&eofil)
				   prstr ("<EOF>");
			  else {
				  prw('(');
				  if (inp)
					   prgetal(rscount,1);
				  else
					   prgetal(rsoutcount,1);
			  prw(')');}
		  break;}
	  }
	  prat (1,6,"C] COMn");
	  if (!inp) {
		  prat (1,7,"L] LPTn");
		  prat (1,8,"D] Disk");
		  if ((*mode>=1)&&(*mode<=2)) prat (1,9,"X] Clear output buffer"); else
			   if (*mode==3) prat(1,9,"X] Close outputfile");
	  } else {
		  prat (1,7,"D] Disk");
		  if (*mode==3) prat (1,8,"R] Reset disk input"); else
			   if (*mode!=0) prat (1,8,"X] Clear input buffer");
	  }
	  prwind();
	  do {
		  i=getkeyfromlist(ldrwlist);
		  for (j=6;j<10;j++) {
			  ycurs=j;
			  xcurs=1;
              for (k=1;k<xwmax2-2;k++) prw(' ');
		  }
		  if (i=='C') {
			  prat (1,6,"Enter COM-port:");
			  prat (1,7,">_");
			  prwind();
			  i=getkeyfromlist(fourlist)-'1';
			  if (i==27-'1') goto setupscr;
			  xcurs--;
			  prw (i+'1');
			  prwind();
			  if (zero(2*i+0x400)) {
				  prat (1,9,"COM-poort not found..");
				  prwind();
				  do {} while (!getkey());
				  goto setupscr;
			  }
			  if ((*mode==3)&&(!inp)) flushoutputC(*num);
			  *mode=2;
			  *num=i;
			  if (inp) clrinp(); else clroutp();
			  goto setupscr;
		  } else if ((i=='L')&&(!inp)) {
			  prat (1,6,"Enter LPT port number:");
			  prat (1,7,">_");
			  prwind();
			  i=getkeyfromlist(fourlist)-'1';
			  if (i==27-'1') goto setupscr;
			  xcurs--;
			  prw (i+'1');
			  prwind();
			  if (zero(2*i+0x408)) {
				  prat (1,9,"LPT-port not found..");
				  prwind();
				  do {} while (!getkey());
				  goto setupscr;
			  }
			  if ((*mode==3)&&(!inp)) flushoutputC(*num);
			  *mode=1;
			  *num=i;
			  if (inp) clrinp(); else clroutp();
			  goto setupscr;
		  } else if (i=='D') {
			  prat (1,6,"Enter filename:");
			  xcurs=1;ycurs=7;
			  prwind();
			  for (i=0;i<129;i++) tempnaam[i]=name[i];
			  if (*mode!=3) tempnaam[0]=0;
			  i=*mode;
			  j=*num;
			  *mode=0;
			  editstring (tempnaam);
			  if (tempnaam[0]==27) {
				  *mode=i;
				  goto setupscr;
			  }
			  if (inp) {
				  choose(0,tempnaam,&redirectstate);
			  }
			  if (tempnaam[0]==27) {
				  *mode=i;
				  goto setupscr;
			  }
			  if (inp) {
				  if (i==3) close_file(*num);
				  *num=open_file(tempnaam,0);
				  eofil=0;
				  rscount=0;
				  clrinp();
			  } else {
				  chdir(rs232dir);
				  *num=open_file(tempnaam,0);
				  if (*num!=-1) {
					  close_file(*num);
					  prat (1,9,"File exists - overwrite? (Y/N)");
					  prwind();
					  k=0xDF & getkeyfromlist (ynlist);
					  if ( (k=='N')||(k==27) ) {
						  *mode=i;
						  *num=j;
						  goto setupscr;
					  }
					  prat (1,9,"                              ");
					  prwind();
				  }
				  if (i==3) flushoutputC(j);
				  *num=creat_file(tempnaam);
				  clroutp();
				  rsoutcount=0;
			  }
			  if (*num==-1) {
				  if (!inp)
					   prat (1,9,"Error while opening");
				  else
					   prat (1,9,"File not found.");
				  prwind();
				  do {} while (!getkey());
			  } else *mode=3;
			  for (i=0;i<129;i++) name[i]=tempnaam[i];
			  goto setupscr;
		  } else if (i=='R') {
			  if ((*mode!=3)||(!inp)) beep(); else {
				  close_file(*num);
				  chdir(rs232dir);
				  *num=open_file(name,0);
				  clrinp();
				  if (*num==-1) {
					  prat (1,9,"Error while resetting");
					  *mode=0;
					  prwind();
					  do {} while (!getkey());
					  goto setupscr;
				  }
				  eofil=0;
				  rscount=0;
				  prat (1,6,"Disk input reset. Key..");
				  prwind();
				  do {} while (!getkey());
				  goto setupscr;
			  }
		  } else if (i=='X') {
			  if ((*mode!=0)&&(*mode!=3)) {
				  if (inp) {
					  clrinp();
					  prat (1,6,"Input buffer cleared. Key..");
				  } else {
					  clroutp();
					  prat (1,6,"Output buffer cleared. Key..");
				  }
				  prwind();
				  do {} while (!getkey());
				  goto setupscr;
			  } else if ((*mode==3)&&(!inp)) {
				  flushoutputC(*num);
				  *mode=0;
				  prat (1,6,"Outputfile closed. Key..");
				  prwind();
				  do {} while (!getkey());
				  goto setupscr;
			  } else beep();
		  }
	  } while ((i!=27)&&(i!=0));
	  install_all();
}

void change_input()
{
	  change_menu("RS232 input channel",&inmode,&innum,inputfile);
}

void change_output()
{
	  change_menu("RS232 output channel",&outmode,&outnum,outputfile);
}

void pruserdefjoy(int j)
{
	  if (j==5) {
		  pruserdefjoy(0);
		  prw(',');
		  pruserdefjoy(1);
		  prw(',');
		  pruserdefjoy(3);
		  prw(',');
		  pruserdefjoy(2);
		  prw(',');
		  pruserdefjoy(4);
	  } else {
		  int far *key=&(udef_keys[j]);
		  if (*key=='[') prstr("<Caps>"); else
		  if (*key==']') prstr("<Sym>"); else
		  if (*key=='/') prstr("<Ent>"); else
		  if (*key=='\\') prstr("<Spc>"); else
			   prw(*key);
	  }
}

void joystick(void)
{
	  int i;
	  joymode=(joymode+1)%4;
	  ywmax2=5+(joymode==2);;
	  clearwindow();
	  prat (1,2,"Joystick emulation:");
	  switch (joymode) {
		  case 0: prat(21,2,"Cursor"); break;
		  case 1: prat(21,2,"Kempston"); break;
		  case 2:
			   prat(21,2,"User defined");
			   prat(3,3,"(Keys: ");
			   pruserdefjoy(5);
			   prstr(")");
			   break;
		  case 3: prat(21,2,"Sinclair 2"); break;
	  }
	  prwind();
      i=10;
	  while (i>0) {
		  if (getkey()) break;
		  pauze();
		  i--;
	  }
}

void change(void)
{
	  int ch;
	  char snelheid[6];
	  do {
          int x,tal=10000,flg=0;
		  clearwindow();
		  prat (1,1,"Change settings");
		  prat (1,3,"I] Redirect RS232 Input");
		  prat (1,4,"O] Redirect RS232 Output");
		  prat (1,5,"R]-register emulation:");
		  prat (1,6,"L]dir emulation:");
		  prat (1,7,"H]i-res color emulation:");
		  prat (1,8,"2] Issue-2 emulation:");
		  prat (1,9,"S]ound:");
		  prat (1,10,"D]ouble irpt freqency:");
		  prat (1,11,"V]ideo synchr. mode:");
		  prat (1,12,"J]oystick-emulation:");
		  prat (1,13,"C]hange rel. speed:");
		  if (joymode==2) {
			  prat(1,14,"K]ey controls: ");
			  pruserdefjoy(5);
		  }
		  printinst(25,5,rr_emul);
		  printinst(25,6,ldrflag);
		  printinst(25,7,usercoloremu);
		  printinst(25,8,issue2);
		  printinst(25,9,!silence);
		  printinst(25,10,Double);
		  if (Double||(syncmod==2)) prat(25,11,"Normal"); else
		  if (syncmod==3) prat(25,11,"Low   "); else
			   prat(25,11,"High  ");
		  switch (joymode) {
			  case 0:
				   prat(25,12,"Cursor  ");
				   break;
			  case 2:
				   prat(25,12,"User Def");
				   break;
			  case 3:
				   prat(25,12,"Sinclr.2");
				   break;
			  case 1:
				   prat(25,12,"Kempston");
		  }
		  x=hoesnel();
          prat(25,13,"       ");
		  xcurs=25;
		  while (tal) {
			  if (x<tal) {
				  if (flg) prw('0');
			  } else {
				  prw ('0'+x/tal);
				  x%=tal;
				  flg=1;
			  }
			  tal/=10;
			  if (tal==1) prw('.');
			  if (tal==10) flg=1;
		  }
		  prw ('%');
		  prwind();

		  ch=getkeyfromlist (changelist);
		  switch (ch) {
			  case 'R':
				   rr_emul=!rr_emul;
				   install_all();  /* try to get speed correct again */
				   break;
			  case 'L':
				   ldrflag=!ldrflag;
				   break;
			  case 'H':
				   if (gmod==5) {
                        if (nohcrmem) {
                            clearwindow();
                            prat (1,3,"No Hi-color emulation");
                            prat (1,4,"buffers allocated!  Remove");
                            prat (1,5,"switch -xu from Z80.INI or");
                            prat (1,6,"command line switches.  Key...");
                            prwind();
                            do {} while (!getkey());
                        } else {
                            usercoloremu=!usercoloremu;
                            install_all();
                        }
				   } else {
					   clearwindow();
					   prcenter (3,"Hi-res color emulation only");
					   prcenter (4,"works in VGA mode.  Key..");
					   prwind();
					   do {} while (!getkey());
				   }
				   break;
			  case 'D':
				   Double=!Double;
				   break;
			  case 'S':
				   silence=!silence;
				   break;
			  case 'J':
				   joymode=(joymode+1)%4;
				   break;
			  case '2':
				   issue2=!issue2;
				   break;
			  case 'V':
				   syncmod=1+(syncmod+1)%3;
				   break;
			  case 'I':
				   change_input();
				   break;
			  case 'O':
				   change_output();
				   break;
			  case 'C':
                   regmsg();
                   break;
			  case 'K':
				   if (joymode!=2) beep(); else {
					   int i,j,k;
					   int far *tab;
					   int udefjoy_loc[5],udefkeys_loc[5];
					   clearwindow();
					   prat (1,2,"Press key that should");
					   prat (1,3,"correspond to:-");
					   prat (1,5,"Left:  ");
					   prat (1,6,"Right: ");
					   pruserdefjoy(1);
					   prat (1,7,"Up:    ");
                       pruserdefjoy(3);
					   prat (1,8,"Down:  ");
                       pruserdefjoy(2);
					   prat (1,9,"Fire:  ");
                       pruserdefjoy(4);
					   prat (1,13,"[ = shift");
					   prat (1,14,"] = symbol shift");
					   i=0;
					   do {
						   prat(8,i+5,"_     ");
						   prwind();
						   do {
							   ch=getkey();
							   if ((ch>'A')&&(ch<='Z')) ch+=32;
							   if (ch==13) ch='/';
							   if (ch==' ') ch='\\';
							   j=-1;
							   do {
								   j++;
							   } while ((j<40)&&(udef_joy_table[2*j+1]!=ch));
						   } while ((j==40)&&(ch!=27));
						   k=i;
						   if (k==2) k=3; else if (k==3) k=2;
						   udefjoy_loc[k]=udef_joy_table[2*j];
						   udefkeys_loc[k]=ch;
						   xcurs=8;
						   swap(udefkeys_loc[k],udef_keys[k]);
						   pruserdefjoy(k);
						   swap(udefkeys_loc[k],udef_keys[k]);
						   prwind();
						   i++;
					   } while ((i<5)&&(ch!=27));
					   if (ch!=27) for (i=0;i<5;i++) {
								udef_joy[i]=udefjoy_loc[i];
								udef_keys[i]=udefkeys_loc[i];
							}
				   }
				   ch=0;
				   break;
		  }
	  } while (ch!=27);
	  install_all();
}

void to_dos(mode)
int mode;
{
	  int ch,x,y;
	  clearwindow();
	  flush_mdrv_buffer();
	  if (mode==QUIT) {
		  for (x=0;x<8;x++) if (mhandles[x]) close_file(mhandles[x]);
		  if (tapeinh) close_file(tapeinh);
		  if (tapeouth) {
			  if (!close_file(tapeouth)) {
				  prat (1,1,"Error while closing tapefile");
				  prwind();
				  do {} while (!getkey());
			  }
		  }
		  exit_emulator();
	  } else {
		  if (!shell_flushbuf()) {
			  prat (1,1,"Error while writing.");
			  prat (1,3,"Writing/closing RS232 output-");
			  prat (2,4,"file failed.");
			  prat (1,6,"key..");
			  prwind();
			  while (getkey()==0) {}
		  }
			   ch=shell();
		  screenbagger=1;
		  if (ch!=0) {
			  clearwindow();
			  prat (1,1,"Error executing command shell:");
			  if (ch==1) {
				  prat (1,3,"Not enough memory, or");
				  prat (1,4,"command shell not found");
			  } else {
				  prat (1,3,"No COMSPEC variabele in");
				  prat (2,4,"environment");
			  }
			  prat (1,6,"key..");
			  prwind();
			  while (getkey()==0) {}
		  }
	  }
}

void altf1(void)
{
	  int i,j;
	  for (i=0;progpath[i];i++) {}
	  for (j=0;helpfile[j];j++) progpath[i+j]=helpfile[j];
	  progpath[i+j]=0;
	  j=open_file(progpath,0);
	  progpath[i]=0;
	  if (j==-1) {
		  clearwindow();
		  prat (1,2,"File LAYOUT.SCR not found!");
		  prat (1,4,"(press any key)");
		  prwind();
		  do {} while (getkey());
		  do {} while (!getkey());
		  return;
	  }
	  select_page5();
	  for (i=0;i<6912;i++) buffer[i]=specpointer[i+16384];
	  read_file(j,specpointer+16384,6912);
	  close_file(j);
	  reset_page5();
	  update_video(0);
	  screenbagger=1;
	  select_page5();
	  for (i=0;i<6912;i++) specpointer[i+16384]=buffer[i];
	  reset_page5();
	  do {} while (getkey());
	  do {} while (!getkey());
}


void help()
{
	  int ch;
	  clearwindow();
	  prcenter (1,"Sinclair ZX Spectrum Emulator");
	  prcenter (2,"Help page");
	  prat (1,4,"ATL-F1  Keyboard lay-out");
	  prat (1,5,"F2      Save .Z80 snapshot");
	  prat (1,6,"F3      Load .Z80 snapshot");
	  prat (1,7,"F4      Change settings");
	  prat (1,8,"ALT-F4  Change joystick setting");
	  prat (1,9,"F5      Generate an NMI");
	  prat (1,10,"F6      Real mode");
      prat (1,11,"(ALT)F7 Tape options");
	  prat (1,12,"F8      Microdrive / MGT options");
	  prat (1,13,"F9      Select hardware");
	  prat (1,14,"F10     Main menu");
	  prwind();
	  while ((ch=getkey())==0) {}
	  if ((ch==256+68)||(ch==27)) goto helpklaar;
	  clearwindow();
	  prcenter (1,"Sinclair ZX Spectrum Emulator");
	  prcenter (2,"Help page");
	  prat (1,4,"ALT, CTRL   Symbol Shift");
	  prat (1,5,"ALT-F5      Reset");
	  prat (1,6,"Numlock     Shifted cursorkeys");
	  prat (1,7,"\010,\011,^,\007     Joystick");
	  prat (1,8,"TAB,5,0     Fire");
	  prat (1,9,"ESC         Edit");
	  prat (1,10,"CTRL-BRK    Quit");
	  prwind();
	  while ((ch=getkey())==0) {}
	  if ((ch==256+68)||(ch==27)) goto helpklaar;
	  clearwindow();
	  prcenter (2,"'Z80'");
	  prcenter (4, "the Sinclair ZX Spectrum 48/128");
	  prcenter (5,"Emulator");
      prcenter (7,"Version 3.04");
      prcenter (8,"August 1996");
      prcenter(10,"(unregistered copy)");
	  prcenter (13,"Written by Gerton Lunter");
	  prwind();
	  while ((ch=getkey())==0) {}
	  helpklaar:
	  tomenu=(ch==256+68);
}

void mdrv(void)
{
	  char tempnaam[129];
	  char ch,curmdrv=0;
	  unsigned int i,j,handle;
	  mdrvagain:
	  clearwindow();
	  prat (1,1,"Microdrive Interface settings");
	  prat (1,3,"1-8: Select microdrive");
	  prat (1,4,"I:   Insert cartridge");
	  prat (1,5,"O:   Take cartridge out");
	  prat (1,6,"W:   Write protection:");
	  if (mhandles[curmdrv]) {
		  if (writprot[curmdrv]) prat(28,6,"ON"); else prat(28,6,"off");
	  } else prat(28,6,"-");
	  prat (1,8,"Current microdrive: ");
	  prw (curmdrv+'1');
	  prat (1,10,"Current cartridge file:");
	  if (mhandles[curmdrv])
		   prat_file(2,11,cartfiles[curmdrv]);
	  else
		   prat(2,11,"none");
	  prwind();
	  ch=getkeyfromlist(mdrvlist);
	  if ((ch>='1')&&(ch<='8')) {
		  curmdrv=ch-'1';
		  goto mdrvagain;
	  }
	  switch (ch) {
		  case 'I':
			   if (mhandles[curmdrv]) {
				   beep();
				   goto mdrvagain;
			   }
			   for (i=0;i<129;i++) tempnaam[i]=cartfiles[curmdrv][i];
			   xcurs=2;
			   ycurs=11;
			   editstring (tempnaam);
			   if (tempnaam[0]==27) break;
			   prat (1,1,"Select cartridge file for mdrv ");
			   prw(curmdrv+'1');
			   choose (3,tempnaam,&mdrvstate);
			   if (tempnaam[0]==27) break;
			   addz80(tempnaam,"MDR");
			   handle=open_file(tempnaam,2);
			   if (handle==0xffff) {
				   clearwindow();
				   prat (1,2,"File not found.");
				   prat (1,4,"Do you want to create an");
				   prat (1,5,"empty cartridge? (Y/N)");
				   prwind();
				   ch=getkeyfromlist(ynlist);
				   if ((ch=='N')||(ch==27)) goto mdrvagain;
				   prat (18,5,"  Y  ");
				   prat (1,7,"  Creating cartridge file...");
				   prwind();
				   handle=creat_file(tempnaam);
				   if (handle==0xffff) {
					   m_c_err:
					   close_file(handle);
					   prat (1,7,"Error creating file.       ");
					   prwind();
					   do {} while (!getkey());
					   goto mdrvagain;
				   }
				   for (i=0;i<13792;i++) buffer[i]=0xFF;   /* 254*0x21f = 137922 */
				   buffer[13792]=0;
				   for (i=0;i<10;i++) if (!write_file(handle,buffer,13792)) goto m_c_err;
				   write_file(handle,buffer+13790,3);
				   if (!close_file(handle)) goto m_c_err;
				   handle=open_file(tempnaam,2);
				   prat (1,7,"UNFORMATTED cartridge created.");
				   prwind();
				   do {} while (!getkey());
			   }
			   for (i=0;i<129;i++) cartfiles[curmdrv][i]=tempnaam[i];
			   i=137922L & 0xFFFFL;
			   j=137922L >> 16;
			   lseek(handle,&i,&j,0);
			   writprot[curmdrv]=0;    /* default: no write protect */
			   read_file(handle,&(writprot[curmdrv]),1);
			   mhandles[curmdrv]=handle;
			   goto write_wpbyte;
		  case 'O':
			   if (!mhandles[curmdrv]) {
				   beep();
			   } else {
				   if (!close_file(mhandles[curmdrv])) {
					   clearwindow();
					   prat (1,2,"Error closing cartridge file...");
					   prwind();
					   do {} while (!getkey());
				   }
				   mhandles[curmdrv]=0;
				   mdrv_reset();
			   }
			   break;
		  case 'W':
			   if (mhandles[curmdrv]) {
				   writprot[curmdrv]=!writprot[curmdrv];
				   write_wpbyte:
				   i=137922L & 0xFFFFL;
				   j=137922L >> 16;
				   lseek(mhandles[curmdrv],&i,&j,0);
				   write_file(mhandles[curmdrv],&(writprot[curmdrv]),1);
				   write_file(mhandles[curmdrv],writprot,0);	 /* chop! */
			   } else
					beep();
			   break;
	  }
	  if (ch!=27) goto mdrvagain;
	  return;
}

void browsetapefile(void)
{
	  unsigned int plow,phigh,i,j,length;
	  int teller,position,firstlin=0,ypos=0,n;
	  unsigned char *bufp1;
	  browse_again:
	  plow=phigh=teller=0;
	  position=-1;
	  clearwindow();
	  prat (3,1,"Position the tape pointer:");
	  prat (3,2,"(press DEL to delete block)");
	  lseek(tapeinh,&plow,&phigh,1);
	  i=j=0;
	  lseek(tapeinh,&i,&j,0);
	  do {
		  bufp1=buffer+32*teller;
		  i=j=0;
		  lseek(tapeinh,&i,&j,1);
		  if (read_file(tapeinh,(char*)&length,2)!=2) {
			  bufp1[0]=0xff;
			  length=0xffff;
		  } else {
			  bufp1[0]=0xfe;
			  if (length==19) read_file(tapeinh,bufp1,12);
			  if ((length!=19)||(bufp1[0]!=0)) {
				  bufp1[1]=(length & 0xff);
				  bufp1[2]=(length >> 8);
			  }
			  bufp1[13]=(i & 0xff);
			  bufp1[14]=(i >> 8);
			  bufp1[15]=(j & 0xff);
			  bufp1[16]=(j >> 8);
			  if ((i==plow)&&(j==phigh)) position=teller;
			  if (i+length+2<i) j++;
			  i+=length+2;
			  lseek(tapeinh,&i,&j,0);
			  teller++;
		  }
	  } while ((length!=0xffff)&&(teller<500));
	  lseek(tapeinh,&plow,&phigh,0);
	  if (teller==0) {
		  beep();
		  return;
	  }
	  if ((position!=-1)&&(ypos+firstlin==0)) {
		  firstlin=position;
		  for (i=0;(i<3)&&firstlin;i++) {
			  firstlin--;
			  ypos++;
		  }
	  }
	  do {
		  if (firstlin!=0) prat (1,3,"^"); else prat (1,3," ");
		  if (ywmax-6+firstlin<teller) prat(1,ywmax-2,"\007"); else prat(1,ywmax-2," ");
		  for (i=0;i<ywmax2-6;i++) {
			  if (i+firstlin==position) prat(1,i+4,">"); else prat(1,i+4," ");
			  if (i==ypos) {
				  attribute=0x80;
			  }
			  bufp1=buffer+32*(firstlin+i);
			  if (firstlin+i>=teller) {
				  xcurs=3;
				  ycurs=i+4;
			  } else {
				  switch(bufp1[0]) {
					  case 0:
						   switch(bufp1[1]) {
							   case 0:
									prat(3,i+4,"Program: ");
									break;
							   case 1:
									prat(3,i+4,"Nmbr Arr: ");
									break;
							   case 2:
									prat(3,i+4,"Char Arr: ");
									break;
							   default:
									prat(3,i+4,"Bytes: ");
						   }
						   for (j=2;j<12;j++) {
							   prw( (bufp1[j]>=32)&&(bufp1[j]<128)?bufp1[j]:32+128 );
						   }
						   break;
					  default:
						   prat(3,i+4,"   Datablock (");
						   j=bufp1[1]+(bufp1[2]<<8);
						   if (j<2) prgetal(0,1); else prgetal(j-2,1);
						   prw(')');
						   break;
				  }
			  }
			  do {prw(' ');} while (xcurs<xwmax-3);
			  attribute=0;
		  }
		  prwind();
		  clearkeybuffer();
		  do {n=getkey();} while (!n);
		  switch (n) {
			  case 337:   /* pgdn */
				   firstlin+=(ywmax2-6);
				   if (firstlin+ypos>=teller) ypos=teller-1-firstlin;
				   if (ypos<0) {
					   firstlin+=ypos;
					   ypos=0;
				   }
				   break;
			  case 329:   /* pgup */
				   firstlin-=(ywmax2-6);
				   if (firstlin<0) {
					   ypos+=firstlin;
					   firstlin=0;
				   }
				   if (ypos<0) ypos=0;
				   break;
			  case 327:   /* home */
				   home:
				   ypos=firstlin=0;
				   break;
			  case 335:   /* end */
				   end:
				   ypos=ywmax2-7;
				   firstlin=teller-ypos-1;
				   if (firstlin<0) {
					   firstlin=0;
					   ypos=teller-1;
				   }
				   break;
			  case 328:   /* up */
				   if (firstlin+ypos>0) {
					   ypos--;
					   if (ypos<0) {
						   ypos=0;
						   firstlin--;
					   }
				   }
				   break;
			  case 336:   /* down */
				   if (ypos+firstlin<teller-1) {
					   ypos++;
					   if (ypos>=ywmax2-6) {
						   ypos--;
						   firstlin++;
					   }
				   }
				   break;
			  case 13:
			  case 27:
			  case 256+68:
				   unhilite();
				   prwind();
				   break;
			  case 46:			  /* . or DEL */
			  case 339:
				   prat (3,1," ARE YOU SURE YOU WANT TO ");
				   prat (3,2," DELETE THAT BLOCK? (Y/N)  ");
				   prwind();
				   do {} while (getkey());
				   do {i=getkey();} while (i==0);
				   if ((i & 0xDF)!='Y') {
					   browse_cont:
					   prat (3,1,"Position the tape pointer:");
					   prat (3,2,"(press DEL to delete block)");
					   prwind();
					   break;
				   }
				   prat (1,1,"Deleting block from tapfile...");
				   clrlin(2);
				   prwind();
				   bufp1=buffer+32*(ypos+firstlin);
				   i=bufp1[13]+(bufp1[14]<<8);
				   j=bufp1[15]+(bufp1[16]<<8);
				   lseek(tapeinh,&i,&j,0);
				   delete_tapblock(tapeinh);
				   lseek(tapeinh,&i,&j,0);
				   if (firstlin+ypos>=teller-1) {
					   if (firstlin+ypos>0) {
						   ypos--;
						   if (ypos<0) {
							   ypos=0;
							   firstlin--;
						   }
						   bufp1-=32;
						   i=bufp1[13]+(bufp1[14]<<8);
						   j=bufp1[15]+(bufp1[16]<<8);
						   lseek(tapeinh,&i,&j,0);
					   } else {
						   i=j=0;
						   lseek(tapeinh,&i,&j,0);
						   return;
					   }
				   }
				   goto browse_again;
			  default:
				   beep();
		  }
	  } while ((n!=13)&&(n!=27)&&(n!=256+68));
	  if (n!=13) return;
	  tapeeof=0;
	  bufp1=buffer+32*(ypos+firstlin);
	  i=bufp1[13]+(bufp1[14]<<8);
	  j=bufp1[15]+(bufp1[16]<<8);
	  lseek(tapeinh,&i,&j,0);
	  return;
}

void tape(char mode)
{
	  char tempnaam[129];
	  char menumode=vochandle;
	  char ch;
	  int i,voc_winded=0,wspd=0,loopcnt;
	  unsigned int p,q,voc_pos,voc_len;
	  unsigned long t;
      if (mode) {
         // ALT-F7: insert .TAP file for reading
         if (taperacc) {
            taperacc=0;
            swap(tapeinmode,tapeim2);
            swap(tapeoutmode,tapeom2);
            taperabuf=NULL;
            if (taperain) close_file(taperain);
            taperain=tapeeof=0;
        }
        if (vochandle) {
            close_file(vochandle);
            menumode=vochandle=vocplay=voc_winded=0;
        }
      }
      do {
		  tapeagain:
		  clearwindow();
		  prat (1,1,"Tape Interface settings");
		  if (menumode) {					  /* VOC file submenu */
			  prat (1,3,"P: VOC file to play:");
			  if (vochandle) prat_file(3,4,vocfile); else prat(3,4,"none");
			  prat (1,6,"S: Recorder status:");
			  if (vochandle) {
				  if (vocplay)
					   prat (3,7,"Playing");
				  else
					   if (loadpause)
					   prat (3,7,"Paused until LOAD \"\"");
				  else
					   prat (3,7,"Paused");
				  prat (1,9,"R: Rewind");
				  prat (1,10,"F: Fast Forward");
				  if (!voc_winded) {
					  t=voc_position();
					  voc_pos=t/100;
				  } else
					   t=voc_pos*100;
				  prat (1,12,"Position:  ");
				  prgetal (t/6000,1);
				  prw(':');
				  t=t%6000;
				  prgetal (t/100,10);
				  prw('.');
				  prgetal(t%100,10);
			  } else
				   prat (3,7,"Off");
			  prwind();
			  gkfl_count=1;
			  ch=getkeyfromlist(tapelist);
			  if (gkfl_count>10) wspd=0;
			  gkfl_count=0;
			  switch (ch) {
				  case 'P':
					   xcurs=3;ycurs=4;
					   prwind();
					   for (i=0;i<129;i++) tempnaam[i]=vocfile[i];
					   editstring (tempnaam);
					   if (tempnaam[0]==27) break;
					   prat (1,1,"Select VOC file to play:");
					   choose (6,tempnaam,&vocstate);
					   if (tempnaam[0]==27) break;
					   addz80(tempnaam,"VOC");
					   if (vochandle) close_file(vochandle);
					   vocplay=0;
					   voc_winded=0;
					   vochandle=open_file(tempnaam,_tapeinmode);
					   if ((int)vochandle==-1) {
						   clearwindow();
						   vochandle=0;
						   prat (1,3,"File not found...");
						   prwind();
						   do {} while (getkey());
						   do {} while (!getkey());
						   goto tapeagain;
					   }
					   for (i=0;i<129;i++) vocfile[i]=tempnaam[i];
					   read_file((int)vochandle,tempnaam,0x1a); 	   /* skip header */
					   vocsec=0;
					   vochsec=0;
					   srate=100;
					   vocdatapoint=0;
					   vocdatalen=0;
					   voctoread=0;
					   voctoreadhi=0;
					   vocplay=0;
					   loadpause=1;
					   voc_msg=1;
					   break;
				  case 'S':
					   if (vochandle) {
						   if (vocplay) {
							   vocplay=0;
							   loadpause=0;
						   } else if (!loadpause) {
							   loadpause=1;
						   } else {
							   loadpause=0;
							   vocplay=0xff;
							   lasttlo=tstates;
							   lastthi=tstatehi;
						   }
					   } else beep();
					   break;
				  case 'R':
				  case 'F':
					   wspd++;
					   if (vochandle) {
						   for (p=0;p<=wspd/30;p++) {
							   if (!voc_winded) {
								   voc_winded=1;
								   prat (20,12,"Please wait..");
								   prwind();
								   i=windvoc(65535L);
								   if (i==-1) {
									   vocerror:
									   clearwindow();
									   prat (1,1,"ERROR reading VOC-file!");
									   prat (1,3,"Key...");
									   prwind();
									   do {} while (getkey());
									   do {} while (!getkey());
									   close_file(vochandle);
									   vocplay=0;
									   goto tapeagain;
								   }
								   voc_len=vocsec;
							   }
							   if (ch=='F') {
								   if (voc_pos<voc_len) voc_pos++;
							   } else if (voc_pos) voc_pos--;
						   }
					   } else beep();
					   break;
				  case 'g':
					   menumode=0;
				  case 27:
					   if (voc_winded) {
						   i=windvoc(voc_pos);
						   if (i==-1) goto vocerror;
						   if (i==1) {
							   close_file(vochandle);
							   vocplay=0;
						   }
					   }
					   break;
				  default:
					   beep();
			  }
		  } else {							  /* Indentation is wrong, I know! */
			  prat (1,3,"P: Play .VOC file");
			  prat (1,5,"S: Single .TAP file mode:");
			  if (taperacc) {
				  prat(28,5,"No");
				  prat (1,6,"D: Tapefile dir:");
				  prat_file(3,7,rtapdir);
				  prat (1,9,"I: Input from:");
				  if (tapeinmode) prat(28,9,"disk"); else {
					  if (blaster) prat(28,9,"S.B."); else {
						  prat(28,9,"LPT");
						  prw(tapeif+'0');
					  }
				  }
				  prat (1,10,"O: Output to:");
				  if (tapeoutmode) prat(28,10,"disk"); else {
					  prat(28,10,"LPT");
					  prw(tapeif+'0');
				  }
                  prat (1,11,"X: Pause after loading:");
                  if (tape_pause) prat(28,11,"YES"); else prat(28,11,"no");
			  } else {
				  prat(28,5,"Yes");
				  prat (1,6,"R: Inputfile:");
				  if (tapeinh) prat_file(16,6,tapein); else prat(16,6,"none");
				  prat (1,7,"W: Outputfile:");
				  if (tapeouth) prat_file(16,7,tapeout); else prat(16,7,"none");
				  prat (1,8,"B: Browse through inputfile");
				  prat (1,10,"I: Input from:");
				  if (tapeinmode) prat(28,10,"disk"); else {
					  if (blaster) prat(28,10,"SB ADC"); else {
						  prat(28,10,"LPT");
						  prw(tapeif+'0');
					  }
				  }
				  prat (1,11,"O: Output to:");
				  if (tapeoutmode) prat(28,11,"disk"); else {
					  prat(28,11,"LPT");
					  prw(tapeif+'0');
				  }
				  prat (1,12,"M: Mirror input to disk: ");
				  if (tapemode==1) prat(28,12,"ON"); else
					   if (tapemode==2) prat(28,12,"EXACT");
				  else prat(28,12,"off");
                  prat (1,13,"X: Pause after loading:");
                  if (tape_pause) prat(28,13,"YES"); else prat(28,13,"no");
                  /* prat (1,12,"T: Continuous tape:");
						  if (tapewrap) prat(28,12,"ON"); else prat(28,12,"off"); */
			  }
			  prwind();
              if (mode) {
                mode=0;
                ch='R';
              } else
                  ch=getkeyfromlist(tapelist);
			  switch (ch) {
				  case 'P':
				  case 'g':
                       regmsg();
                       goto tapeagain;
				  case 'S':
					   if (taperacc) taperacc=0; else taperacc=-1;
					   swap(tapeinmode,tapeim2);
					   swap(tapeoutmode,tapeom2);
					   taperabuf=NULL;
					   if (taperain) close_file(taperain);
					   taperain=tapeeof=0;
					   break;
				  case 'D':
					   if (taperacc==0) {
						   beep();
						   break;
					   }
					   xcurs=3;ycurs=7;
					   prwind();
					   for (i=0;i<129;i++) tempnaam[i]=rtapdir[i];
					   if (editstring (tempnaam)&nochange) {
						   prat (1,1,"Select tapefile directory:");
						   choose (5,tempnaam,&tapestate);
					   } else {
						   chdir(rtapdir);
						   if (!chdir(tempnaam)) {
							   getdir(rtapdir);
						   }
					   }
					   taperabuf=NULL;
					   tapeinmode=tapeoutmode=1;
					   tapeeof=0;
					   break;
				  case 'B':
					   if (taperacc) {
						   beep();
						   break;
					   }
					   if (!tapeinh)
							beep();
					   else
							browsetapefile();
					   break;
				  case 'I':
					   tapeinmode=!tapeinmode;
					   break;
				  case 'O':
					   tapeoutmode=!tapeoutmode;
					   break;
                  case 'X':
                       tape_pause=!tape_pause;
                       break;
				  case 'R':
					   if (taperacc) {
						   beep();
						   break;
					   }
                       if (tapeinh) {
                            close_file(tapeinh);
                            tapeinh=0;
                            goto tapeagain;
                       }
					   xcurs=16;ycurs=6;
					   prwind();
					   for (i=0;i<129;i++) tempnaam[i]=tapein[i];
					   editstring (tempnaam);
					   if (tempnaam[0]==27) break;
					   prat (1,1,"Select tape file for input:");
					   choose (2,tempnaam,&tapestate);
					   if (tempnaam[0]==27) break;
					   addz80(tempnaam,"TAP");
					   if (tapeinh) close_file(tapeinh);
					   tapeinh=open_file(tempnaam,_tapeinmode);
					   if (tapeinh==-1) {
						   clearwindow();
						   tapeinh=open_file(tempnaam,0);
						   if (tapeinh==-1) prat(1,2,"File not found..."); else {
							   close_file(tapeinh);
							   prat(1,2,"File is read-only...");
						   }
						   prwind();
						   do {} while (!getkey());
						   tapeinh=tapeinmode=0;
						   goto tapeagain;
					   }
					   tapeeof=0;
					   for (i=0;i<129;i++) tapein[i]=tempnaam[i];
					   tapeinmode=1;
					   break;
				  case 'W':
					   if (taperacc) {
						   beep();
						   break;
					   }
                       if (tapeouth) {
                            close_file(tapeouth);
                            tapeouth=0;
                            goto tapeagain;
                       }
					   xcurs=16;ycurs=7;
					   prwind();
					   for (i=0;i<129;i++) tempnaam[i]=tapeout[i];
					   editstring (tempnaam);
					   if (tempnaam[0]==27) break;
					   if (tapeouth) {
						   if (!close_file(tapeouth)) {
							   clearwindow();
							   prat (1,3,"Error during closing outputfile");
							   prat (1,5,"Press any key...");
							   do {} while (getkey());
							   do {} while (!getkey());
							   tapeouth=tapeoutmode=0;
							   goto tapeagain;
						   }
					   }
					   chdir(tapdir);
					   addz80(tempnaam,"TAP");
					   tapeouth=open_file(tempnaam,0);
					   if (tapeouth!=-1) {
						   close_file(tapeouth);
						   clearwindow();
						   prat (1,2,"File exists!  Press:");
						   prat (2,4,"Y - to overwrite it,");
						   prat (2,5,"A - to append to existing file,");
						   prat (2,6,"N - to exit.");
						   prwind();
						   i=0xDF & getkeyfromlist (ynalist);
						   if ( (i=='N')||(i==27) ) {
							   tapeouth=tapeoutmode=0;
							   goto tapeagain;
						   }
						   if (i=='A') {
							   tapeouth=open_file(tempnaam,2);
							   i=0;
							   lseek(tapeouth,&(unsigned int)i,&(unsigned int)i,2);
						   } else {
							   tapeouth=creat_file(tempnaam);
						   }
					   } else tapeouth=creat_file(tempnaam);
					   if (tapeouth==-1) {
						   clearwindow();
						   prat (1,2,"Error while opening...");
						   prwind();
						   do {} while (!getkey());
						   tapeouth=tapeoutmode=0;
						   goto tapeagain;
					   }
					   for (i=0;i<129;i++) tapeout[i]=tempnaam[i];
					   tapeoutmode=1;
					   break;
				  case 'M':
					   if (taperacc) {
						   beep();
						   break;
					   }
					   tapemode=(tapemode+1)%3;
					   break;
				  case 'T':
					   beep();
					   /* tapewrap=!tapewrap;
							   tapeeof=0; */
					   break;
				  case 27:
					   break;
				  default:
					   beep();
			  }
		  } 					  /* Wrong indentation, see above */
	  } while (ch!=27);
	  if (taperacc) return;
	  clearwindow();
	  prat (1,2,"WARNING:");
	  if (tapeinmode && (!tapeinh)) {
		  prat (1,4,"No inputfile specified.");
		  goto taperr;
	  }
	  if ((tapeoutmode||tapemode)&&(!tapeouth)) {
		  prat (1,4,"No outputfile specified.");
		  goto taperr;
	  }
	  return;
	  taperr:
	  prat (1,6,"Press any key to return to");
	  prat (1,7,"tape-menu.");
	  prwind();
	  do {} while (!getkey());
	  goto tapeagain;
}

void save_prog(void)
{
	  int ch;
	  unsigned int handle,totlen;
      sprognaam[0]=0;
	  clearwindow();
	  prat (1,1,"Save Spectrum snapshot");
	  prat (1,3,"Enter name:");
	  xcurs=1;ycurs=4;
      editstring(sprognaam);
      if ((sprognaam[0]==0)||(sprognaam[0]==27)) return;    /* lege regel of ESC */
      substz80 (sprognaam,"Z80");
	  chdir(dir1);
      handle=open_file(sprognaam,0);                   /* 0=read only */
	  if (handle!=0xFFFF) {
		  close_file(handle);
		  prat (1,6,"File exists - overwrite? (Y/N)");
		  prwind();
		  ch=0xDF & getkeyfromlist (ynlist);
		  if ( (ch=='N')||(ch==27) ) return;
		  prat (1,6,"                              ");
		  prwind();
	  }
      handle=creat_file(sprognaam);
	  if (handle==0xFFFF) {
		  prat (1,6,"Error in filename or");
		  prat (1,7,"Drive Write Protected - key..");
		  prwind();
		  while (getkey()==0) {}
		  return;
	  }
	  maakinstelbits();
	  if (save_z80_file(handle)) {
		  close_file(handle);
		  goto display_error;
	  }
	  if (!close_file(handle)) {
		  display_error:
		  prat (1,6,"Disk full - key..");
		  prwind();
		  while (getkey()==0) {}
	  }
		   regpointer->imode&=0x03;
}

void load_prog(void)
{
	  unsigned int handle;
      int filelen,p,q,nochnge=0,tryfirst=0;
	  if (prognaam[0]) tryfirst=2;
	  load_again:
	  clearwindow();
	  prat (1,1,"Load Spectrum snapshot");
	  if (!tryfirst) {
		  prat (1,3,"Enter name:");
		  prat (1,ywmax2-2,"(CTRL-ENTER = keep settings)");
		  xcurs=1;ycurs=4;
		  nochnge=(editstring(prognaam)&ctrlenter);
		  if (prognaam[0]==27) return;
		  nochnge|=choose(1,prognaam,&loadstate);
		  if (prognaam[0]==27) return;
	  }
      substz80 (prognaam,"SLT");
	  handle=open_file(prognaam,0); 	  /* 0 = read only */
	  if (handle==0xFFFF) {
          substz80 (prognaam,"SNA");
		  handle=open_file(prognaam,0);
      }
      if (handle==0xFFFF) {
          substz80 (prognaam,"Z80");
          handle=open_file(prognaam,0);
      }
      if (handle==0xFFFF) {
          if (tryfirst) {
              tryfirst--;
              chdir(dir1);
              goto load_again;
          }
          clearwindow();
          prat (1,3,"File not found - key..");
          prwind();
          while (getkey()==0) {}
          goto load_again;
	  }
	  for (p=0;(prognaam[p+1]&&(prognaam[p]!='.'));p++) {}
      if ((prognaam[p+2]&0xDF)=='N') p=1; else p=0;
	  if (nochnge) p|=2;
	  p=load_z80_file(p,handle);
	  if (p) {
          update_video(8);
          screenbagger=1;
          regpointer->rpc=0;
		  clearwindow();
		  prat (1,3,"Error in .Z80/.SNA file");
		  prat (1,4,"Press any key...");
		  prwind();
		  while (getkey()==0) {}
		  if (handle!=0xFFFF) close_file(handle);
		  return;
	  }
      p=regpointer->imode;
      if (!nochnge) {
          issue2=!!(p&4);
          Double=!!(p&8);
          syncmod=((p&48)>>4);
          if (!syncmod) syncmod=2;
          joymode=((p&192)>>6);
      }
      regpointer->imode&=3;
      init_emulator();
      install_all();
      //
      // now look whether loading screen has to be shown...
      //
      if (handle!=0xFFFF) {
          struct sltheader {
              word type;
              word level;
              long length;
          } slt;
          long scrpos=0,scrlen,curpos;
          word poslo,poshi,startposlo,startposhi;
          int scrtoshow=1,scrcounter,borclr;
          if (read_file(handle,buffer,3)!=3) goto noscreen;
          if ((buffer[0]!='S')||(buffer[1]!='L')||(buffer[2]!='T')) goto noscreen;
          startposlo=startposhi=0;
          lseek(handle,&startposlo,&startposhi,1);
          do {
              scrcounter=scrtoshow;
              scrlen=curpos=0;
              do {
                  if (read_file(handle,(void*)&slt,8)!=8) goto noscreen;
                  if (slt.type==3) {
                     scrcounter--;
                     if (scrcounter==0) {
                         scrpos=curpos;
                         scrlen=slt.length;
                         borclr=slt.level;
                     }
                  }
                  curpos += slt.length;
              } while (slt.type);
              if (scrlen) {
                poslo=scrpos & 0xFFFFL;
                poshi=(scrpos>>16);
                lseek(handle,&poslo,&poshi,1);
                if (read_file(handle,buffer+16384-scrlen,scrlen)==scrlen) {
                  select_page5();
                  unpack2(buffer+16384-scrlen,buffer,scrlen,6912);
                  for (p=0;p<6912;p++) swap(buffer[p],specpointer[p+16384]);
                  reset_page5();
                  update_video(borclr & 7);
                  screenbagger=1;
                  select_page5();
                  for (p=0;p<6912;p++) specpointer[p+16384]=buffer[p];
                  reset_page5();
                  do {} while (getkey());
                  do {} while (!getkey());
                  lseek(handle,&startposlo,&startposhi,0);
                }
              }
              scrtoshow++;
          } while (scrcounter<=0);
      }
      noscreen:
      update_video(8);
      screenbagger=1;
      if (handle!=0xFFFF) close_file(handle);
}

int open_tapfile(char *name)
{
	  char fname[13];
	  int i,j=0,k=0;
	  char ch;
	  fname[0]='t';
	  for (i=0;i<10;i++) {
		  ch=name[i];
		  if ( ((ch>='0')&&(ch<='9')) || ((ch>='a')&&(ch<='z')) ||
			((ch>='A')&&(ch<='Z')) ) fname[j++]=ch;
	  }
	  if (j==0) j=1;
	  fname[j]=0;
	  do {
		  addz80(fname,"TAP");
		  i=open_file(fname,0);
		  if (i!=-1) {
			  close_file(i);
			  i=j;
			  if (i>6) i=6;
			  if ((fname[i]>='0')&&(fname[i]<='9')&&
				(fname[i+1]>='0')&&(fname[i+1]<='9')) {
				  fname[i+1]++;
				  if (fname[i+1]>'9') {
					  fname[i+1]='0';
					  fname[i]++;
					  if (fname[i]>'9') fname[i]='0';
				  }
			  } else {
				  fname[i]=fname[i+1]='0';
			  }
			  fname[i+2]=0;
		  }
	  } while ((i!=-1)&&(k++<100));
	  if (k>=100) return(-1);
	  return(creat_file(fname));
}


int savediskblock_1(a1,a2,a3,handle,a5,a6)
unsigned int a1,a2,a3;
int handle,a5;
unsigned char a6;
{
	  unsigned int i1=0,i2=0;
	  lseek(handle,&i1,&i2,2);
	  return(savediskblock(a1,a2,a3,handle,a5,a6));
}


void saveloadblock(type)
int type;
{
	  unsigned int x,y,start,length,afpair,oldrafa;
	  int diskmode,blocklength,reopen=0;
	  slb_again:
	  ywmax2=11;
	  clearwindow();
	  if (type) {
		  diskmode=tapeoutmode;
		  tapeeof=0;
	  } else {
		  diskmode=tapeinmode;
		  if (tapeeof) diskmode=0;
	  }
	  if ((loadpause||vocplay)&&(!type)) {	  /* VOC playback mode? */
		  if (loadpause) {
			  lasttlo=tstates;
			  lastthi=tstatehi;
			  loadpause=0;
			  vocplay=0xff;
		  }
          return_immediately:
          if (type) regpointer->rpc=0x4D5; else
               regpointer->rpc=0x56B;
          return;
	  }
      if ((outlog)&&(type))                   /* SAVE and logging OUTs? */
          goto return_immediately;
      if (!diskmode) {
		  prat(1,3,"Sorry -- tape support is");
		  prat(1,4,"available to registered users");
		  prat(1,5,"only. Please refer to the");
		  prat(1,6,"manual for registration");
		  prat(1,7,"details.");
		  prat (26,ywmax2-3,"key..");
		  prwind();
		  do {} while (!getkey());
          goto return_immediately;
	  }
	  if (spacebar()) {
		  press_break();
		  regpointer->rfa &= 0xBEFF;   /* nc & nz */
		  regpointer->rpc=0x5e2;
		  clrkbd=updvid=0;
		  return;
	  }
	  start=(regpointer->rix)+type;
	  length=(regpointer->rde)-type;
	  afpair=oldrafa=regpointer->rafa;
	  if ((rommod==2)&&(romstate==2)&&(!type)) {/* See routine at #0567 SamRam */
		  afpair=((afpair&0xbfff)|((afpair&0x8000)>>1));
		  regpointer->rafa=afpair;
	  }
	  if (winkey && (!diskmode)) {
		  prat (3,2,"No physical tape support in");
		  prat (3,3,"Windows Compatibility mode!");
		  prat (8,5,"(press any key)");
		  prwind();
		  do {} while (getkey());
		  do {} while (!getkey());
		  goto return_immediately;
	  }
	  if ((!taperacc)||(!diskmode)) {
		  if (!type) prat(1,1,"LOADing block from "); else
			   prat(1,1,"SAVEing block to ");
		  if (diskmode) prstr("disk..."); else prstr("tape...");
		  prat (1,3,"Start:  ");
		  prgetal(start,1);
		  prat (1,4,"Length: ");
		  prgetal(length,1);
		  prat (1,5,"Type:   ");
		  x=(afpair & 0xFF);
		  if ((!!(afpair & 0x4000))&&(!type)) {
			  prstr("?");
			  x=1;
		  } else prgetal (x,1);
		  switch (x) {
			  case 0:prstr("  (header)");
				   break;
			  case 255:prstr("  (datablock)");
				   break;
			  default:
				   prstr ("  (unknown)");
		  }
		  if (diskmode) {
			  prat (1,8,"Press space to <BREAK>");
		  } else {
			  prat (1,7,"Press F6 for real mode,");
			  prat (1,8,"any other key to <BREAK>");
		  }
		  if (tapewindow) prwind();
	  }
	  if (!diskmode) {			  /* tape */
		  if (!type)
			   x=loadblock();
		  else
			   x=saveblock();
		  if (x) {				  /* f6 */
			  modflg=1;
			  if (type) regpointer->rpc=0x4D5; else
				   regpointer->rpc=0x56B;
			  regpointer->rafa=oldrafa;
			  do {} while (getkey());
			  update_video(8);
			  screenbagger=1;
			  init(0);
			  real_mode();
			  return;
		  }
	  }
	  if (diskmode) {
		  if (taperacc) {
			  updvid=0;
			  if (!type) {	  /* load, non single-file-mode */
				  if (taperain) {
					  if (loaddiskblock(start,length,taperain)==1) { /* eof? */
						  close_file(taperain);
						  taperain=0;
					  }
				  }
				  if (!taperain) {	  /* first block */
					  if (taperabuf) {
						  taperain=findnext_ra(taperabuf);
						  if (!taperain) taperabuf=NULL;
					  }
					  if (!taperabuf) {   /* do a findfirst */
						  taperabuf=rabuf;
						  chdir(rtapdir);
						  taperain=findfirst_ra(taperabuf);
						  if (!taperain) {	  /* not a single .tap file found */
                              tapeeof=1;      /* switch to loading from lpt/sb */
							  updvid=1;
							  goto slb_again;
						  }
					  }
					  if (loaddiskblock(start,length,taperain)!=0) {
                          close_file(taperain);     /* error, or eof at 1st block */
						  taperain=0;
					  }
				  }
			  } else {		  /* save, random access */
				  if ((afpair&0xFF)==0) {	  /* header */
					  if (taperaout) close_file(taperaout);
					  chdir(rtapdir);
					  taperaout=open_tapfile(&specpointer[start+1]);
					  if (taperaout==-1) {
						  ra_wrierr:
						  prat (1,3,"Error writing .TAP file in");
						  raerr:
						  prat (1,4,"Multiple File mode - key..");
						  prwind();
						  do {} while (getkey());
						  do {} while (!getkey());
						  updvid=1;
						  goto return_immediately;
					  }
					  if(savediskblock_1(start,length,afpair,taperaout,2,0))
						   goto ra_wrierr;
					  update_regs_save();
				  } else {
					  if (!taperaout) {   /* headerless */
						  taperaout=open_tapfile("hdrles00  ");
						  if (taperaout==-1) goto ra_wrierr;
					  }
					  if(savediskblock_1(start,length,afpair,taperaout,2,0)) goto ra_wrierr;
					  close_file(taperaout);
					  update_regs_save();
					  taperaout=0;
				  }
			  }
			  if (getkey()) {	  /* any keypress generates a break */
				  do {} while (getkey());
				  regpointer->rfa &= 0xBEFF;   /* nc & nz */
				  press_break();
				  clrkbd=0;
			  }
			  return;
		  }
			   /* At this point, random-access disk i/o is finished & has returned */
		  if (!type)
			   x=loaddiskblock(start,length,tapeinh);
		  else {
			  x=savediskblock_1(start,length,afpair,tapeouth,2,0);
			  update_regs_save();
			  reopen=1;
		  }
	  }

	  if (x==1) { /* eof, only for disk input */
		  if (!tapewrap) {
			  tapeeof=1;
			  goto slb_again;
		  }
		  close_file(tapeinh);
		  chdir(tapdir);
		  tapeinh=open_file(tapein,_tapeinmode);
		  if (tapeinh==-1) {
			  tapeinerr:
			  tapeinh=0;
			  diskmode=0;
			  tapeinmode=0;
			  prat(1,7,"Error reading tapefile.");
			  prat(1,8,"Press any key...       ");
			  prwind();
			  do {} while (getkey());
			  do {} while (!getkey());
			  goto slb_again;
		  }
		  x=loaddiskblock(start,length,tapeinh);
		  if (x==1) {
			  close_file(tapeinh);
			  goto tapeinerr;
		  }
	  }

	  if (diskmode) {
		  do {y=getkey();} while ((y!=0)&&(y!=32));
		  while (getkey()!=0) {}
		  if (y) {
			  regpointer->rfa &= 0xBEFF;   /* nc & nz */
			  press_break();
			  clrkbd=0;
		  }
	  }

	  if (x) goto tapediskerr;

      /* here tape and disk in/out come together again */

	  if ((!type)&&tapemode) {
		  prat(1,1,"Copying block to disk...  ");
		  /* If tapemode = 1 (ON)
             - Do not copy blocks with checksum errors
             - Do not copy 0 byte blocks (ticks in leader)
             - Do copy blocks shorter than DE bytes
             If tapemode = 2 (EXACT)
             - Copy all blocks, together with original checksum loaded from tape
          */
		  if (tapemode==1) {
			  if ((afpair & 0x4000)) {	 /* z */
				  afpair=specpointer[start];
				  start++;
				  length--;
			  }
			  if (regpointer->rde) length-=((regpointer->rde)+1);
				   /* If length wasn't right but checksum is 0, then save with one
							  byte less (checksum), it'll be computed correctly upon saving */
			  if ((length<0xFFFE)&&(length!=0)) {
				  if (!((regpointer->rhl)&0xff00)) {
					  prwind();
					  x=savediskblock_1(start,length,afpair,tapeouth,2,0);
					  reopen=1;
				  } else {
					  clearwindow();
					  prat (1,3,"    Error loading block -");
					  prat (1,4,"  block NOT copied to disk");
					  prat (1,6," (press any key to continue)");
					  prwind();
					  beep();
					  do {} while (getkey());
					  do {} while (!getkey());
					  x=0;
				  }
			  }
		  } else {
			  if (regpointer->rde) length-=(regpointer->rde);
			  if (length==0) savediskblock_1(0,0,0,tapeouth,0,0); else {
				  if ((afpair & 0x4000)) {	 /* z */
					  afpair=specpointer[start];
					  start++;
					  length--;
				  }
				  if (regpointer->rde)
					   savediskblock_1(start,length,afpair,tapeouth,1,0);
				  else
					   savediskblock_1(start,length,afpair,tapeouth,2,((regpointer->rhl)&0xff00)>>8);
				  reopen=1;
			  }
		  }
	  }
	  if (x) {
		  tapediskerr:
		  clearwindow();
		  prat (1,2,"Error during disk I/O...");
		  if (!type) {
			  close_file(tapeinh);
			  tapeinmode=tapeinh=0;
		  } else {
			  close_file(tapeouth);
			  tapeoutmode=tapeouth=0;
		  }
		  prwind();
		  do {} while (!getkey());
	  } else if (diskmode) {
		  if (tapewindow) {
			  prwind();
              for (length=0;length<5;length++) {
				  pauze();
				  if (x=getkey()) break;
			  }
		  }
		  if (x==32) {
			  press_break();
			  regpointer->rfa &= 0xBEFF;   /* nc & nz */
			  clrkbd=0;
		  }
	  }

	  if (reopen) {
		  if (commit(tapeouth)) goto tapediskerr;
		  if (tapeinh) {
			  x=y=0;
			  lseek(tapeinh,&x,&y,1);
			  close_file(tapeinh);
			  chdir(tapdir);
			  tapeinh=open_file(tapein,_tapeinmode);
			  if (tapeinh==-1) {
				  clearwindow();
				  prat (1,2,"Error re-opening inputfile");
				  prwind();
				  do {} while (!getkey());
				  tapeinh=tapeinmode=0;
			  } else lseek(tapeinh,&x,&y,0);
		  }
	  }
      if (tape_pause) {
          int ch;
          screenbagger=1;
          update_video(0);
          xwmax2=8;
          ywmax2=3;
          clearwindow();
          prat(1,1,"Key...");
          prwind();
          do {} while (getkey());
          do {} while (!(ch=getkey()));
          screenbagger=1;
          if ((ch>=256+59)&&(ch<=256+68)) tomenu='a'+ch-256-59;
      }
}


void emulmode(void)
{
	  int ch,curmod=rommod,curtype=disctype;
	  do {
		  tryagain_hwm:
		  clearwindow();
		  prat (1,1,"Select hardware:");
		  prat (1,14,"ENTER=select; CTRL-ENT: no reset");
		  attribute=0x80*(curmod==0);
		  prat (1,3,"1] Spectrum  48");
		  attribute=0x80*(curmod==1);
		  prat (1,4,"2] Spectrum  48 + If.1");
		  attribute=0x80*(curmod==2);
		  prat (1,5,"3] Spectrum  48 + If.1 + SamRam");
		  attribute=0x80*(curmod==3);
		  prat (1,6,"4] Spectrum  48 + M.G.T.");                  /* HMc */
		  attribute=0x80*(curmod==4);
		  prat (1,7,"5] Spectrum 128");                           /* HMc */
		  attribute=0x80*(curmod==5);							  /* HMc */
		  prat (1,8,"6] Spectrum 128 + If.1");                    /* HMc */
		  attribute=0x80*(curmod==6);							  /* HMc */
		  prat (1,9,"7] Spectrum 128 + M.G.T.");                  /* HMc */
		  attribute=0;											  /* HMc */
		  prat (1,11,"T] M.G.T. Type: ");                         /* HMc */
		  if (curtype==0) prat (17,11,"DISCiPLE (Eps)"); else     /* HMc */
		  if (curtype==1) prat (17,11,"DISCiPLE (HP) "); else  /* HMc */
		  if (curtype==9) prat (17,11,"UNIDOS       "); else   /* HMc */
		  if (curtype==16) prat (17,11,"+D           "); else  /* HMc */
			   if (curtype==17) prat (17,11,"+D (HP PCL 3)");       /* HMc */
		  prat (1,12,"M]ultiface 128:");
		  if (m128_on) prat(17,12,"ON "); else prat(17,12,"off");
		  attribute=0;
		  prwind();
		  ch=getkeyfromlist(fivelist);
		  switch (ch) {
			  case 327:   curmod=0;
				   break;
			  case 335:   curmod=6;
				   break;
			  case 328:   curmod--;
				   if (curmod<0) curmod=6;
				   break;
			  case 336:   curmod++;
				   if (curmod>6) curmod=0;
				   break;
              case 'T':
                   regmsg();
                   goto tryagain_hwm;
			  case 'M':
				   m128_on=!m128_on;
				   break;
			  default:	  if ((ch>='1')&&(ch<='7')) curmod=ch-'1';    /* HMc MOD */
		  }
	  } while ((ch!=10)&&(ch!=13)&&(ch!=27));
	  if (ch==27) return;
      if ((curmod==3)||(curmod==6)) {
        regmsg();
        goto tryagain_hwm;
      }
      rommod=curmod;
	  disctype=curtype;
	  init_emulmode(rommod,ch==13);
}


int di_halt(void)
{
	  int ch;
	  ywmax2=10;
	  clearwindow();
	  prat (2,1,"SPECTRUM CRASH by DI/HALT");
	  prat (1,3,"Select:-");
	  prat (1,5,"1] Enable interrupts, continue");
	  prat (1,6,"2] Ignore error and continue");
	  prat (1,7,"3] Go to main menu");
	  prwind();
	  ywmax2=ywmax;
	  screenbagger=1;
	  ch=getkeyfromlist(threelist);
	  switch (ch) {
		  case '1':
			   regpointer->iff=0xFF;
		  case '2':
			   return (0);
	  }
	  return (1);
}

int in_zero(void)
{
	  /* Enter_c restores PC from temp_pc */
	  ywmax2=9;
	  clearwindow();
	  prat (4,1,"SPECTRUM CRASH by IN 0");
	  prat (1,3,"Select:-");
	  prat (1,5,"1] Ignore error and continue");
	  prat (1,6,"2] Go to main menu");
	  prwind();
	  ywmax2=ywmax;
	  screenbagger=1;
	  return (getkeyfromlist(twolist)-'1');
}

int real_mode()
{
	  int ch;
	  ywmax2=9;
	  tomenu=0;
	  updvid=0;
	  clearwindow();
	  if (winkey||vocplay) {
		  if (winkey) {
			  prat (3,2,"You cannot use Real Mode in");
			  prat (3,3,"Windows Compatibility mode!");
		  } else {
			  prat (2,2,"You can't use Real Mode while");
			  prat (2,3,"playing VOC files.");
		  }
		  prwind();
		  do {} while (getkey());
		  do {} while (!getkey());
		  updvid=1;
		  return(0);
	  }
	  modflg=(modflg & 0x80)+1;
	  prat (3,1,"Spectrum Emulator - Real Mode");
	  prat (1,4,"B] Back to normal mode");
	  prat (1,5,"U] Update screen once");
	  prat (1,6,"P] Partial screen update");
	  ch=getkey();
	  if ((ch>='a')&&(ch<='z')) ch&=0xDF;
	  switch (ch) {
		  case 'U':
			   update_video(8);
			   screenbagger=1;
			   if (!(modflg & 0x80)) prwind();
			   break;
		  case 'P':
			   update_video(8);
			   screenbagger=1;
			   if (modflg & 0x80) beep(); else modflg=0x81;
			   break;
		  case 'B':
		  case 27:
			   modflg=0;
			   updvid=1;
			   break;
		  case 0:
			   prwind();
			   break;
		  default:
			   if ((ch>=256+59)&&(ch<=256+68)&&(ch!=256+59+5)) {
				   tomenu=1;
				   update_video(8);
				   screenbagger=1;
				   modflg=0;
				   updvid=1;
				   do {} while (getkey());
				   return (ch+'a'-256-59);
			   } else {
				   beep();
				   modflg&=0x7f;
				   prwind();
				   updvid=!modflg;
			   }
	  }
	  do {} while (getkey());
	  return(0);
}

void voc_error(void)
{
	  ywmax2=7;
	  clearwindow();
	  prat (1,1,"VOC playback status:");
	  switch (vocerror) {
		  case 0:
			   prat (3,3,"Read error!");
			   break;
		  case 1:
			   prat (3,3,"VOC file played.");
			   break;
		  case 2:
			   prat (3,3,"Unsupported block type!");
			   break;
		  case 3:
			   prat (3,3,"Compressed block encountered");
			   break;
	  }
	  prat (1,5,"Key...");
	  if (vocerror!=1) {
		  beep();
		  voc_msg=1;
	  }
	  if (voc_msg) {
		  prwind();
		  do {} while (getkey());
		  do {} while (!getkey());
	  }
	  close_file(vochandle);
	  vochandle=0;
	  vocplay=0;
}


/*************************** HMc Function! *******************************/

void extra2(void)
{
	  char ch2;
	  char d1t,d2t,d1d,d2d;

	  d1t=DISCD1Type;
	  d1d=DISCD1Drive;
	  d2t=DISCD2Type;
	  d2d=DISCD2Drive;

	  extra2_again:
	  do {
		  clearwindow();
		  prat(1,1,"M.G.T. Options");
		  prat(1,4,"1] Drive 1:");
		  if (d1t==1) {
			  prat(19,4,"File");
			  prat(1,5,"F] Filename:");
			  if (DISCD1FileName[0]==' ') prat(14,5,"None"); else
				   prat_file(14,5,DISCD1FileName);
		  } else
		  if (d1t==-1) {
			  prat(23,4,"Floppy ");
			  if (d1d==0) prat(30,4,"A");
			  if (d1d==1) prat(30,4,"B");
		  };
		  prat(1,6,"2] Drive 2:");
		  if (d2t==1) {
			  prat(23,6,"File");
			  prat(1,7,"G] Filename: ");
			  if (DISCD2FileName[0]==' ') prat(14,7,"None"); else
				   prat_file(14,7,DISCD2FileName);
		  } else
		  if (d2t==-1) {
			  prat(23,6,"Floppy ");
			  if (d2d==0) prat(30,6,"A");
			  if (d2d==1) prat(30,6,"B");
		  };
		  if (disctype<16) {							  /* HMc */
			  prat (1,8,"I] Inhibit button:");          /* HMc */
			  if (discinhibit==0) prat (23,8,"Out"); else /* HMc */
				   if (discinhibit==-1) prat (23,8,"In ");  /* HMc */
		  };
			   /*
				   prat (1,10,"Z] ZX Printer = : ");
				   if (zxptype==0) prat(19,10,"HP PCL 3"); else
					  if (zxptype==-1) prat(19,10,"Epson");
				   prat(1,12,"B] Back to previous menu");
				   It's never necessary to change the printer type from EPSON to HP PCL
				   or vv. during execution; setting it once and for all in the .ini file
				   is better.
				   The B] option is absent in other menu's too; ESC is used for backtracking
				   the menu tree
				   */

		  prwind();
		  ch2=getkeyfromlist(extra2list);

		  switch(ch2) {
			  case '1':
				   if (d1t==1) {
					   d1t=-1;
					   d1d=0;
				   } else
				   if (d1t==-1) {
					   d1d=1-d1d;
				   };
				   break;
			  case '2':
				   if (d2t==1) {
					   d2t=-1;
					   d2d=0;
				   } else
				   if (d2t==-1) {
					   d2d=1-d2d;
				   };
				   break;
			  case 'I':
				   if (disctype<16) {
					   if (discinhibit==0) discinhibit=-1; else
							if (discinhibit==-1) discinhibit=0;
				   };
				   break;
			  case 'B':
			  case 27:
				   DISCD1Type=d1t;
				   DISCD1Drive=d1d;
				   DISCD2Type=d2t;
				   DISCD2Drive=d2d;
				   return;
				   /*
						  case 'Z':
						  if (zxptype==0) zxptype=-1; else
						  if (zxptype==-1) zxptype=0;
						  break;
						  See comments above
						  */
		  };

	  } while (1);

}

/************************ END OF HMc Function! **************************/


void multi_level_load(void)
{
	  /* Called when EDFB is executed.	Multi level load format devised by
			 Russell Marks; first implemented in XZX 0.5.2 */
      /* SLT extension to .Z80 files by Damien Burke, James McKay and me */
	  char datname[129];
      unsigned char header[8];
      unsigned int level;
	  int i,j=0,k=0;
	  unsigned int handle;
      unsigned int lopos,hipos,w,levsiz;
      unsigned char *bufr;
      long curpos,levpos;
      regpointer->rfa ^= 0x100;     // CCF, to signal 'failed loading block'
      clearwindow();
	  for (i=0;prognaam[i];i++) {
		  datname[i]=prognaam[i];
		  if (datname[i]=='.') j=i;
		  if (datname[i]=='\\') k=i+1;
	  }
      i=level=(regpointer->rfa&0xFF);
	  if (j-k>7) j=k+7;
      if (i>9) if (j-k>6) j=k+6;
      if (i>99) if (j-k>5) j=k+5;
	  if (j<k) j=k;
      if (i>99) datname[j++]='0'+(i/100);
      i %= 100;
      if (i>9) datname[j++]='0'+(i/10);
      i %= 10;
      datname[j++]='0'+i;
	  datname[j]=0;
	  chdir(dir1);
	  addz80(datname,"DAT");
	  handle=open_file(datname,0);
	  if (handle==0xFFFF) {
          // Level data not found in separate block.  Now try to find
          // .Z80 or .SLT file with additional level data (v3.04)
          chdir(dir1);
          substz80(prognaam,"SLT");
          handle=open_file(prognaam,0);
          substz80(prognaam,"Z80");
          if (handle==0xFFFF) {
            handle=open_file(prognaam,0);
          }
          if (handle==0xFFFF) {
              // Cannot find .DAT file or .Z80/.SLT file; ask for .DAT file
              prat (1,1,"Loading game level ");
              prgetal(level,1);
              prat (1,3," Cannot find file:");
              prat_file(1,4,datname);
              prat (1,ywmax2-3,"Key...");
              askfile:
              prwind();
              do {} while (!getkey());
              levelagain:
              clearwindow();
              prat (1,1,"Loading game level ");
              prgetal(level,1);
              prat(1,3,"Enter name of DAT or Z80/SLT file:");
              xcurs=1;
              ycurs=4;
              editstring(datname);
              if (datname[0]==27) return;
              choose (8,datname,&loadstate);
              if (datname[0]==27) return;
              substz80 (datname,"DAT");
              handle=open_file(datname,0);
              if (handle==0xFFFF) {
                  substz80 (datname,"SLT");
                  handle=open_file(datname,0);
                  if (handle!=0xFFFF) goto loadfromz80;
                  substz80 (datname,"Z80");
                  handle=open_file(datname,0);
                  if (handle!=0xFFFF) goto loadfromz80;
                  clearwindow();
                  prat (1,3,"File not found...");
                  prwind();
                  do {} while (!getkey());
                  goto levelagain;
              }
          } else {
              // Found .Z80 file; now find appropriate level data block
              loadfromz80:
              lopos=6;
              hipos=0;
              clearwindow();
              lseek(handle,&lopos,&hipos,0);      // PC
              read_file(handle,(char*)&w,2);
              i=0;
              if (w!=0) {
                  nodata:
                  close_file(handle);
                  prat(1,3,"Did not find (appropriate) level");
                  prat(1,4,"data in .Z80/.SLT file.  Key..");
                  goto askfile;
              }
              lopos=30;
              hipos=0;
              lseek(handle,&lopos,&hipos,0);      // Length of add. blk
              read_file(handle,(char*)&w,2);
              lopos=w+32;
              hipos=0;
              lseek(handle,&lopos,&hipos,0);      // Now at start of pages
              do {
                  if (read_file(handle,header,3)!=3) goto nodata;
                  lopos=header[0]+(header[1]<<8);
                  hipos=0;
                  lseek(handle,&lopos,&hipos,1);  // skip block
              } while (header[1]||header[2]);
              read_file(handle,header,3);         // read "SLT" identifier
              if ((header[0]!='S')||(header[1]!='L')||(header[2]!='T'))
                  goto nodata;
              levpos=-1;
              curpos=0;
              do {
                  if (read_file(handle,header,8)!=8) goto nodata;
                  if ((header[0]==1)&&(header[1]==0)&&
                      (header[2]==level)&&(header[3]==0)) {
                          levpos=curpos;
                          levsiz=header[4]+(header[5]<<8);
                      }
                  curpos+=(word)header[4]+(word)(header[5]<<8)+
                          (((unsigned long)(header[6]))<<16);
              } while (header[0]||header[1]);
              if (levpos==-1L) goto nodata;
              lopos=(levpos&0xFFFFL);
              hipos=(levpos>>16);
              lseek(handle,&lopos,&hipos,1);
              w=memalloc((levsiz/16)+1);
              if (w)
                  bufr=(unsigned char *)(((unsigned long)w)<<16);
              else {
                  prat (1,3,"Not enough memory to load data!");
                  prat (1,5,"Press any key..");
                  prwind();
                  do {} while (getkey());
                  do {} while (!getkey());
                  close_file(handle);
                  return;
              }
              read_file(handle,bufr,levsiz);
              close_file(handle);
              if (regpointer->rhl>=16384) {    // Don't load in ROM
                  unpack2(bufr,&specpointer[regpointer->rhl],levsiz,
                      -regpointer->rhl);
              }
              memfree(w);
              regpointer->rfa ^= 0x100;     // CCF again, loaded OK
              return;
          }
	  }
      // read normal .DAT file
	  read_file(handle,&(specpointer[regpointer->rhl]),-regpointer->rhl);
      regpointer->rfa ^= 0x100;         // CCF again, loaded OK
	  close_file(handle);
}



char extra(void)
{
	  char ch,ch2;
	  char addr[6];
      unsigned int handle,ioadr;
      int i,j;
	  extra_again:
	  do {
		  clearwindow();
		  prat (1,1,"Extra functions:");
		  prat (1,4,"D] Dos shell");
		  prat (1,5,"S] Save screen or memory block");
		  prat (1,6,"L] Load screen or memory block");
		  prat (1,7,"R] Reset Spectrum");
		  prat (1,8,"N] Generate NMI");
          prat (1,9,"O] Log OUTs");
          if (outlog) prstr("  (now ACTIVE)"); else prstr("  (now inactive)");
          prat (1,10,"B] Set/Reset breakpoint (");
		  if (debugflg==(char)0xFE) {
			  prw('#');
			  prhexgetal(debugadr);
		  } else prstr("none");
		  prw (')');
		  prwind();
		  ch=getkeyfromlist(extralist);
		  switch (ch) {
			  case 'R':
				   set_nmireset(0x80);
				   return ('B');
			  case 'N':
				   set_nmireset(0xFF);
				   return ('B');
			  case 'D':
				   chdir(defdir);
				   to_dos(SHELL);
				   getdir(defdir);
				   break;
			  case 'L':
			  case 'S':
				   do {
					   clearwindow();
					   if (ch=='L') prat(1,1,"Load block"); else prat (1,1,"Save block");
					   prat (1,3,"M] Screen or Memory");
					   if (blocktype) prat (24,3,"SCREEN"); else prat(24,3,"MEMORY");
					   prat (1,4,"S] Start address:");
					   xcurs=24;
					   prw ('#');
					   prhexgetal(startadr);
					   prat (1,5,"L] Length:");
					   xcurs=24;
                       if (blocklength) {
                        prw ('#');
                        prhexgetal(blocklength);
                       } else {
                        xcurs=21;
                        prstr ("Entire file");
                       }
					   prat (1,7,"N] Name:");
					   prwind();
					   ch2=(getkey()&0xDF);
					   switch (ch2) {
						   case 'M':
								if (blocktype) blocktype=0; else {
									blocktype=1;
									startadr=16384;
									blocklength=6912;
								}
								break;
						   case 'S':
								blocktype=0;
								addr[0]='\005';
                                prat (24,4,"     ");
                                xcurs=24;
								editstring(addr);
                                startadr=dechex(addr);
								break;
						   case 'L':
								blocktype=0;
								addr[0]='\005';
                                prat (21,5,"        ");
                                xcurs=24;
								editstring(addr);
                                blocklength=dechex(addr);
								break;
						   case 'N':
						   case 0:
								break;
						   case 27:
								goto extra_again;
						   default:
								beep();
					   }
				   } while (ch2!='N');
				   if ((startadr<16384)||(startadr+blocklength<16384)) {
					   prat (1,10,"Warning: ROM (partly) included!");
					   prat (1,11,"ESC=cancel, other=continue...");
					   prwind();
					   beep();
					   do {ch2=getkey();} while (ch2==0);
					   if (ch2==27) goto extra_again;
					   clrlin(10);
					   clrlin(11);
					   prwind();
				   }
				   scrfile[0]=0;
				   xcurs=1;
				   ycurs=8;
				   editstring(scrfile);
				   if (scrfile[0]==27) break;
				   if (ch=='L') {
					   choose(blocktype?4:7,scrfile,&scrstate);
					   if (scrfile[0]==27) goto extra_again;
					   if (blocktype) addz80(scrfile,"SCR");
					   handle=open_file(scrfile,0);
					   if (handle==0xFFFF) {
						   prat (1,6,"File not found - key..");
						   prwind();
						   while (getkey()==0) {}
						   goto extra_again;
					   }
					   if (blocktype) select_page5();
                       if (!blocklength) {
                           if (startadr==0) {
                              blocklength=read_file(handle,specpointer,0x8000);
                              if (blocklength==0) goto err_reading;
                              read_file(handle,specpointer+0x8000,0x8000);
                           } else
                              if (!read_file(handle,specpointer+startadr,-startadr))
                                 goto err_reading;
                       } else if (read_file(handle,&(specpointer[startadr]),blocklength)!=blocklength) {
                           err_reading:
						   prat (1,6,"Error reading file - key..");
						   prwind();
						   while (getkey()==0) {}
					   }
                       close_file(handle);
					   if (blocktype) reset_page5();
					   update_video(8);
					   screenbagger=1;
					   init(0);
				   } else {
					   if (scrfile[0]==0) goto extra_again;
					   if (blocktype) addz80 (scrfile,"SCR");
					   chdir(scrdir);
					   handle=open_file(scrfile,0); 		  /* 0=read only */
					   if (handle!=0xFFFF) {
						   close_file(handle);
						   prat (1,10,"File exists - overwrite? (Y/N)");
						   prwind();
						   ch=0xDF & getkeyfromlist (ynlist);
						   if ( (ch=='N')||(ch==27) ) goto extra_again;
						   prat (1,10,"                              ");
						   prwind();
					   }
					   handle=creat_file(scrfile);
					   if (handle==0xFFFF) {
						   prat (1,10,"Error in filename or");
						   prat (1,11,"Drive Write Protected - key..");
						   prwind();
						   while (getkey()==0) {}
						   goto extra_again;
					   }
					   if (blocktype) select_page5();
					   if (!write_file(handle,&(specpointer[startadr]),blocklength)) {
						   prat (1,10,"Disk full - key..");
					   } else {
						   prat (1,10,"Writing successful - key..");
					   }
					   if (blocktype) reset_page5();
					   prwind();
					   while (getkey()==0) {}
					   close_file(handle);
				   }
				   break;
			  case 'B':
				   clearwindow();
				   prat (1,1,"Set/Reset breakpoint");
				   addr[0]='\005';
                   prat (1,3,"Enter address:  ");
				   editstring(addr);
                   debugadr=dechex(addr);
				   if (debugadr!=0) debugflg=0xFE;
				   break;
              case 'O':
                    logoutagain:
                    clearwindow();
                    prat (1,1,"Log OUTs menu");
                    prat (1,8,"Log file name:");
                    if (outlog) prat_file (16,8,outlogfile); else prat(16,8,"(none)");
                    prat (1,10,"Table of OUT ports logged:");
                    for (i=0;outlogtable[i];i++) {
                        xcurs=1+8*(i%4);
                        ycurs=11+(i/4);
                        prgetal (i+1,1);
                        prw('.');
                        xcurs=4+8*(i%4);
                        prhexgetal (outlogtable[i]);
                    }
                    if (i==0) prat (1,11,"(none)");
                    prat (1,3,"N]ame OUT log file");
                    prat (1,4,"A]dd OUT port");
                    prat (1,5,"D]elete OUT port");
                    prwind();
                    clrlin(3);
                    clrlin(4);
                    clrlin(5);
                    do {
                      ch2=getkey();
                      switch (ch2&0xDF) {
                        case 'N':
                            if (outlog) {
                                outlog_flushbuf();
                                close_file(outloghandle);
                                outlog=0;
                            }
                            prat (1,3,"Enter name:");
                            xcurs=1;
                            ycurs=4;
                            editstring(outlogfile);
                            if (outlogfile[0]==27) goto logoutagain;
                            addz80(outlogfile,"OUT");
                            handle=open_file(outlogfile,0);
                            if (handle!=0xFFFF) {
                                close_file(handle);
                                prat (1,6,"File exists - overwrite? (Y/N)");
                                prwind();
                                ch=0xDF & getkeyfromlist (ynlist);
                                if ( (ch=='N')||(ch==27) ) goto logoutagain;
                                clrlin(6);
                                prwind();
                            }
                            handle=creat_file(outlogfile);
                            if (handle==0xFFFF) {
                                prat (1,6,"Error in filename or");
                                prat (1,7,"Drive Write Protected - key..");
                                clrlin(8);
                                prwind();
                                while (getkey()==0) {}
                                goto logoutagain;
                            }
                            outlog=-1;
                            outloghandle=handle;
                            goto logoutagain;
                        case 'A':
                            if (i==16) {
                                beep();
                                break;
                            }
                            prat (1,3,"Enter port address to add:");
                            addr[0]='\005';
                            prat (1,4,"#");
                            editstring(addr);
                            if (addr[0]==27) goto logoutagain;
                            ioadr=hex(addr);
                            outlogtable[i]=ioadr;
                            goto logoutagain;
                        case 'D':
                            if (i==0) {
                                beep();
                                break;
                            }
                            prat (1,3,"Enter table entry to delete:");
                            addr[0]='\005';
                            xcurs=1;
                            ycurs=4;
                            editstring(addr);
                            if (addr[0]==27) goto logoutagain;
                            j=number(addr);
                            if ((j>=1)&&(j<=i)) {
                                for (;j<=i;j++) outlogtable[j-1]=outlogtable[j];
                            } else beep();
                            goto logoutagain;
                        case 27: goto extra_again;
                        case 0: break;
                        default: beep();
                      }
                    } while (1);
			  default:
				   return (' ');
		  }
	  } while (1);
}

int far entry (action_code)
unsigned char	action_code;
{
	  int ch,i;
	  char flag=1;
	  init(0);
	  updvid=clrkbd=1;
	  if (action_code!=0xF4)
		   while (getkey());
	  switch (action_code) {
		  case 0xFE:			  /* di/halt */
			   modflg=0;
			   if (di_halt()==0) goto return_to_emulator;
			   break;
		  case 0xFD:			  /* in 0 */
			   modflg=0;
			   if (in_zero()==0) goto return_to_emulator;
			   break;
		  case 0xFC:
		  case 0xFB:
			   modflg=0;
			   ywmax2=9;
			   clearwindow();
			   if (action_code==0xFB) prat (1,1,"READ"); else prat (1,1,"WRITE");
			   prat (xcurs,1," ERROR DURING CHANNEL I/O");
			   prat (1,3,"Select:-");
			   prat (1,5,"1] Abort I/O operations");
			   prat (1,6,"2] Retry");
			   prwind();
			   ch=getkeyfromlist(twolist)-'1';
			   if (ch==(27-'1')) ch=0;
			   update_video(8);
			   screenbagger=1;
			   return (ch); 	   /* Do NOT exit the usual way; unusual entry */
		  case 0xFA:
			   if (suretoquit()) {
				   chdir(defdir);
				   to_dos(QUIT);
			   }
			   goto return_to_emulator;
		  case 0xF9:			  /* Startup code */
               for (i=0;prognaam[i]!=0x0D;i++) {};
			   prognaam[i]=0;
			   for (i=0;prognaam[i]==' ';i++) {};
			   if (prognaam[i]) load_prog();
			   goto return_to_emulator;
          case 0xF8:
			   help();
			   goto return_to_emulator;
		  case 0xF7:
			   save_prog();
			   goto return_to_emulator;
		  case 0xF6:
			   prognaam[0]=0;
			   load_prog();
			   goto return_to_emulator;
		  case 0xF5:
			   change();
			   goto return_to_emulator;
		  case 0xF4:
			   ch=real_mode();	   /* updates updvid */
			   flag=2;
			   if (tomenu) {
				   init(0);
				   tomenu=1;
			   }
			   goto return_to_emulator;
          case 0xF3:
		  case 0xF2:
			   saveloadblock(action_code-0xF2);
			   if (modflg) updvid=0;
               if (tomenu) {
                  flag=2;
                  ch=tomenu;
                  init(0);     // normal size window in case of F10 keypress
                  tomenu=1;
               }
               goto return_to_emulator;
          case 0xF1:
               tape(0);
               goto return_to_emulator;
          case 0xF0:
               if ((rommod==3)||(rommod==6))
                    extra2();
			   else
					mdrv();
			   goto return_to_emulator;
		  case 0xEF:
			   /* mdrv_error(); */ /* Well, do YOU think this'll ever happen? */
			   goto mainmenu;
		  case 0xEE:
			   emulmode();
			   goto return_to_emulator;
		  case 0xED:
			   altf1();
			   goto return_to_emulator;
          case 0xEC:
			   ywmax2=5;
			   clearwindow();
			   prat (1,2,"Breakpoint hit at #");
			   prhexgetal(debugadr);
			   prstr("   Key..");
			   prwind();
			   do {} while (getkey());
			   do {} while (!getkey());
			   screenbagger=1;
			   ywmax2=ywmax;
			   goto mainmenu;
		  case 0xEB:
			   voc_error();
			   goto return_to_emulator;
          case 0xEA:
			   joystick();
			   goto return_to_emulator;
          case 0xE9:
			   multi_level_load();
			   goto return_to_emulator;
          case 0xE8:
               tape(1);
               goto return_to_emulator;
		  case 0xE0:
			   vernachelflop();
			   intzero();
		  case 0:
			   goto return_to_emulator;
	  }
	  mainmenu:
	  tomenu=0;
	  do {
		  updvid=1;
		  if (flag==1) {
			  clearwindow();
			  prat (3,1,"Sinclair ZX Spectrum Emulator");
			  prat (1,3,"B - Back to Spectrum");
			  prat (1,4,"L - Load snapshot");
			  prat (1,5,"S - Save snapshot");
			  prat (1,6,"C - Change settings");
			  prat (1,7,"T - Tape");
			  prat (1,8,"M - Microdrive / MGT options");
			  prat (1,9,"H - Select hardware");
			  prat (1,10,"R - Real mode");
			  prat (1,11,"X - Extra functions");
			  prat (1,12,"Q - Quit");
			  prwind();
		  }
		  if (flag!=2) ch=getkeyfromlist (mainmenu); else
		  {
			  flag=1;
			  tomenu=(ch==256+68);
		  }
		  switch (ch) {
			  case 'B':
			  case 2:
				   flag=0;
				   ch='B';
				   break;
			  case 'a':
				   help();
				   ch='B';
				   break;
			  case 'f':
			  case 'R':
				   ch='B';
				   update_video(8);
				   screenbagger=1;
				   init(0);
				   real_mode();
				   if (tomenu) {
					   init(0);
					   tomenu=1;
				   }
				   flag=2;
				   break;
			  case 'C':
			  case 'd':
				   change();
				   break;
			  case 'L':
			  case 'c':
				   prognaam[0]=0;
				   load_prog();
				   init(0);
				   break;
			  case 'S':
			  case 'b':
				   save_prog();
				   break;
			  case 'T':
			  case 'g':
                   tape(0);
				   break;
			  case 'M':
			  case 'h':
				   if ((rommod==3)||(rommod==6))
						extra2();
				   else
						mdrv();
				   break;
			  case 'H':
			  case 'i':
				   emulmode();
				   break;
			  case 'X':
				   ch=extra();
				   if (ch=='B') {
					   flag=0;
					   tomenu=0;
				   }
				   break;
			  case 'Q':
				   if (suretoquit()) {
					   chdir(defdir);
					   to_dos(QUIT);
				   }
				   break;
			  case 27:
				   ch='B';
				   break;
		  }
	  }
	  while (ch!='B');
	  return_to_emulator:
	  if (tomenu) goto mainmenu;
	  install_all();
	  if (updvid) update_video(8);
	  if (clrkbd) clearkeyboard();
	  return (0);
}

