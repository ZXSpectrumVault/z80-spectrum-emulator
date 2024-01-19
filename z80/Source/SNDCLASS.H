// SndClass.h
// Written by Christopher M. Box (1993).

// Contains definitions for sound classes and other globally-defined
// information 

#ifndef SND_CLASSES_H
#define SND_CLASSES_H

// These are constants, but you could of course make them variable and use
// either an auto-detect function or some kind of installation program
//#define SbIOaddr 0x220
//#define SbIRQ 7
//#define SbDMAchan 1
#define SbType 3

extern int SbIOaddr;        // in DMACODE.ASM
extern int SbIRQ;
extern int SbDMAchan;

// Data directions to choose from
enum { RECORD = 0, PLAY };

// SoundDevice identification codes (but only the SB device is included here)
#define SB_ID 1
#define CD_ID 2

// Convenient shorthand to represent a byte
typedef unsigned char byte;

// Dprint is used for including debugging messages, and
// incurs no overhead when DEBUG is not defined.
#ifdef DEBUG
#define Dprint(x) cprintf x
#else
#define Dprint(x)
#endif

// Functions from DMA_CODE.ASM
extern "C" {
int dsp_reset(void);
int prevent_dma(int Channel);
int allow_dma(int Channel);
int dma_setup(int Channel,byte far *Buffer,unsigned Length,int Dir);
unsigned dma_count(int Channel);
unsigned dma_addr(int Channel);
}
extern int dma_errno;
extern char far *dma_errlist[];

void mem_error(void);

// Base sound device class, from SoundDev.cpp

class SoundDevice {
public:
	virtual int identify(void) = 0;
	virtual int install_ok(void) = 0;
	virtual void halt(void) = 0;
	virtual void cont(void) = 0;
	virtual void reset(void) = 0;
	void file_dma(int handle, byte far *buf, unsigned bufsize, long len, byte dir);
	void monitor_input(byte far *dmabuf, unsigned blocklen);
	virtual void buf_dma_start(byte far *buffer, unsigned buflen, byte dir) = 0;
	virtual int buf_dma_lo(unsigned len) = 0;
	virtual int buf_dma_hi(unsigned len, unsigned next_buflen) = 0;
	virtual unsigned dma_addr(void) = 0;
	virtual unsigned get_rate(void) = 0;
	virtual unsigned get_width(void) = 0;
	virtual void set_rate(unsigned rate, byte dir) = 0;
	virtual int get_chan(void) = 0;
private:
	int disk_io(int handle, byte far *buf, unsigned len, byte dir);
	void setsizes(unsigned &bufsize, long &len, unsigned &lo_sz, unsigned &hi_sz);
	unsigned dt_min, dt_max;
};

// Soundblaster functions from SbDevice.cpp

void far interrupt sb_buf_dma_int(...);
void far interrupt sb_dummy_int(...);

class SbDevice : public SoundDevice {
public:
	SbDevice(void);
	~SbDevice(void);
	int identify(void) { return SB_ID; }
	int install_ok(void) { return exists; }
	void halt(void);
	void cont(void);
	void reset(void) { dsp_reset(); }
	void voice(int state);
	void set_rate(unsigned rate, byte dir);
	unsigned get_rate(void) { return rate; }
	unsigned get_width(void) { return 1U; }
	int get_hs(void) { return hi_speed; }
	void buf_dma_start(byte far *buffer, unsigned buflen, byte dir);
	int buf_dma_lo(unsigned len);
	int buf_dma_hi(unsigned len, unsigned next_buflen);
	unsigned dma_addr(void) { return ::dma_addr(SbDMAchan); }
	int get_chan(void) { return SbDMAchan; }
private:
	void dsp_cmd(byte cmd);
	void init_irq(void);
	void deinit_irq(void);
	void set_sb_cmds(unsigned buflen);
	int process_keys(void);

	void interrupt far (*OldIRQ)(...);
	byte exists;
	byte hi_speed;
	unsigned sb_size, lo_buf_sz;
	unsigned rate;
	byte direction;
};

// Defines for SB IO addresses
#define DSP_RESET           SbIOaddr+0x06
#define DSP_READ_DATA       SbIOaddr+0x0A
#define DSP_WRITE_DATA      SbIOaddr+0x0C
#define DSP_WRITE_STATUS    SbIOaddr+0x0C
#define DSP_DATA_AVAIL      SbIOaddr+0x0E

// SB DSP Command codes
#define DIRECT_8_BIT_DAC    0x10
#define DMA_8_BIT_DAC	    0x14
#define DMA_2_BIT_DAC		0x16
#define DMA_2_BIT_REF_DAC   0x17
#define DIRECT_ADC	    	0x20
#define DMA_ADC 	    	0x24
#define TIME_CONSTANT	    0x40
#define DMA_4_BIT_DAC	    0x74
#define DMA_4_BIT_REF_DAC   0x75
#define DMA_26_BIT_DAC	    0x76
#define DMA_26_BIT_REF_DAC  0x77
#define HALT_DMA	    0xD0
#define CONTINUE_DMA	0xD4
#define SPEAKER_ON	    0xD1
#define SPEAKER_OFF	    0xD3
#define DSP_ID		    0xE0
#define DSP_VER 	    0xE1

#define SET_HS_SIZE 0x48
#define HS_DAC 0x91
#define HS_ADC 0x99

#endif

// The rest is included here for convenience

extern byte far *outputbuffer;
extern long samplesstored;
extern char raw;
extern char oversample;
unsigned int process_buffer(byte far*, byte far*, unsigned int, unsigned int);


