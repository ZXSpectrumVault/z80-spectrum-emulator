// SoundDev.cpp

// General sound functions that can be applied to derived sound classes
// (eg SbDevice)

// Written by Christopher M. Box (1993)

#include <conio.h>
#include <io.h>
#include <process.h>
#include <values.h>

#include "sndclass.h"

#define DEFAULTRECORDLEN 30000000L      // 30 megabytes

// Forward function definitions

static void bar_display(char val);
static void bar_display(int val);

// Function: setsizes
// Given a buffer size ('bufsize') and remaining file length ('len'), this
// determines lengths for the lower and upper half-buffers. It is used four
// times in the file_dma function, so it is brought out here as an inline
// function.

inline void SoundDevice::setsizes(unsigned &bufsize, long &len,
                                    unsigned &lo_sz, unsigned &hi_sz) {
	len -= (lo_sz = (bufsize < len) ? bufsize : (unsigned) len);
	len -= (hi_sz = (bufsize < len) ? bufsize : (unsigned) len);
}

// Function: file_dma
// Handles the running of buffered DMA from a sound device to a disk file,
// or in the opposite direction. 'handle' is the handle of a file opened
// with _open(), 'lobuf' is a pointer to the bottom of the single DMA buffer,
// 'bufsize' is the size of it (up to a maximum of 64K), 'len' is the desired
// length of file to play (or zero for the full length) and 'dir' sets the
// direction (recording or playback).

void SoundDevice::file_dma(int handle, byte far *lobuf, unsigned bufsize,
                                                        long len, byte dir) {

	reset();
	byte far *hibuf = lobuf + bufsize;  // Calculate pos of high half-buffer
	unsigned lo_sz, hi_sz, cur_sz;
	dt_min=65535U;                 // Initialise disk time statistics
	dt_max=0;
	if (dir == RECORD) {
        int c=0;
        unsigned int outputlen;
        monitor_input(lobuf,1000);
        reset();
		if (len == 0) len = DEFAULTRECORDLEN;
		setsizes(bufsize,len,lo_sz,hi_sz);
        record_lo_again:
		buf_dma_start(lobuf,lo_sz+hi_sz,RECORD);
		while (1) {
			if (lo_sz == 0) break;
            while (dma_addr() == 0);
            // Wait for the transfer to start to avoid confusing buf_dma_lo().
            if (c=buf_dma_lo(lo_sz)) { // if terminated by keypress
				lo_sz = dma_addr();
				Dprint(("Saving up to position %u.\r\n",lo_sz));
			}
            outputlen = process_buffer(lobuf,outputbuffer,lo_sz,get_rate());
            if (disk_io(handle,outputbuffer,outputlen,RECORD)) return;
            if (c) {
                if (c=='p') {
                    monitor_input(lobuf,1000);
                    len += hi_sz;       // We should also include part of lo_sz
                    setsizes(bufsize,len,lo_sz,hi_sz);
                    c=0;
                    goto record_lo_again;
                } else {
                    hi_sz=0;
                }
            }
			if (hi_sz == 0) break;
			cur_sz = hi_sz;
			setsizes(bufsize,len,lo_sz,hi_sz);
            while (dma_addr() <= bufsize);
            if (c=buf_dma_hi(cur_sz,lo_sz+hi_sz)) {
				cur_sz = dma_addr() - bufsize;
				Dprint(("Saving hibuf, length %u.\r\n",cur_sz));
			}
            outputlen = process_buffer(hibuf,outputbuffer,cur_sz,get_rate());
            if (disk_io(handle,outputbuffer,outputlen,RECORD)) return;
            if (c) {
                if (c=='p') {
                    monitor_input(lobuf,1000);
                    c=0;
                    goto record_lo_again;
                } else {
                    lo_sz=0;
                }
            }
		}
	} else {                                         // Play function
		if (len == 0) len = filelength(handle);
		setsizes(bufsize,len,lo_sz,hi_sz);
		if (lo_sz == 0) {
			cprintf("Error: zero length.\r\n");
			exit(-1);
		}
		if (_read(handle,lobuf,lo_sz) != lo_sz) {    // Fill first buffer
			cprintf("File read error.\r\n");
			exit(-1);
		}
		buf_dma_start(lobuf,lo_sz+hi_sz,PLAY);
		while (dma_addr() == 0);               // Wait for it to start
		while (1) {                            // Main buffer loop - forever
			if (hi_sz == 0) {
				buf_dma_lo(lo_sz);
				break;
			} else {
				if (disk_io(handle,hibuf,hi_sz,PLAY)) return;
				if (buf_dma_lo(lo_sz)) break;
			}
			if (len == 0) {
				buf_dma_hi(hi_sz, 0);
				break;
			} else {
				setsizes(bufsize,len,lo_sz,hi_sz);
				if (disk_io(handle,lobuf,lo_sz,PLAY)) return;
				if (buf_dma_hi(bufsize, lo_sz+hi_sz)) break;
			}
		}
	}
	// At the end print out min and max times per disk access,
	// in terms of both bytes and milliseconds.
//    unsigned bytes_per_ms =
//        (unsigned) (((long) get_rate() * get_width() + 500) / 1000);
//    cprintf("Disk stats: min %u (%ums), max %u (%ums).\r\n",
//    dt_min,
//    dt_min / bytes_per_ms,
//    dt_max,
//    dt_max / bytes_per_ms);
}

// Function: disk_io
// Takes care of the time-consuming disk reads/writes of file 'handle'
// from/into buffer 'buf', of 'len' bytes in direction 'dir'.
// Automatically measures the time taken in terms of the number of bytes
// advanced by the DMA and updates the min/max times.

int SoundDevice::disk_io(int handle, byte far *buf, unsigned len, byte dir) {
	unsigned old_addr = dma_addr();
	if (dir == RECORD) {
        struct {
            unsigned char type;           // sound data
            unsigned int len;
            unsigned char hilen;          // always below 64K-2 bytes
            unsigned char rate;
            unsigned char compression;    // no compression, 8 bits data
        } vocheader;
        int secs;
        vocheader.type=1;
        vocheader.hilen=0;
        vocheader.len=len+2;
        vocheader.rate=256-((oversample ? 500000L : 1000000L)/get_rate());
        vocheader.compression=0;
        if (!raw) {
            if (_write(handle,&vocheader,sizeof(vocheader)) != sizeof(vocheader)) {
                ferr:
                cprintf("Write error: disk full?\r\n");
                return 1;
            }
        }
        if (_write(handle,buf,len) != len) goto ferr;
        samplesstored += len;
        secs = samplesstored/((oversample ? 2 : 1)*get_rate());
        cprintf ("%2d min %02d sec\r",secs/60,secs%60);
	} else {
		if (_read(handle,buf,len) != len) {
			cprintf("File read error.\r\n");
			return 1;
		}
	}
	unsigned new_addr = dma_addr();
	unsigned disktime = new_addr - old_addr;
	if (new_addr < old_addr) {
        cprintf("Warning: writing took much time; probably gap in recording!\n");
	} else {
		if (disktime && disktime<dt_min) dt_min = disktime;
		if (disktime>dt_max) dt_max = disktime;
		Dprint(("DT = %X. ",disktime));
	}
	return 0;
}

// Function: max
// Defined both for integers and bytes, for use in monitor_input

inline int max(int v1, int v2) {
	return (v1 > v2 ? v1 : v2);
}

inline byte max(byte v1, byte v2) {
	return (v1 > v2 ? v1 : v2);
}

// Function: monitor_input
// Displays input levels by continously reading in DMA for 'blocklen' bytes
// and showing the maximum value within the block as a bar graph from left
// to right. 80 columns represents full scale. The value chosen for
// 'blocklen' affects the responsiveness and accuracy of the bar graph.
// I have included code for both the SB (unsigned mono bytes) data format
// and the CD (signed stereo words) format to demonstrate how both types can
// be used.

void SoundDevice::monitor_input(byte far *dmabuf, unsigned blocklen) {
    static char FirstTime=1;
	if (blocklen > 65000) {
		cprintf("Use a smaller block length.\r\n");
		return;
	}
	blocklen &= ~3;             // Round down to a double-word boundary
	byte maxbyte;
	int maxleft, maxright;
	byte far *bdata;            // Different pointer types for SB (byte)
	int far *idata;             // and CD (signed int)
	byte far *data_end = dmabuf + blocklen;

	_setcursortype(_NOCURSOR);
    if (!FirstTime)
        cprintf ("Paused... press any key to continue recording:\r\n");
    cprintf ("\r\nÚÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄúúúúúúúúúúúúúúúúúú\r\n");
    while (1) {
		buf_dma_start(dmabuf,65500U,RECORD);
		while ((dma_addr() < blocklen) && !kbhit());// Wait for buffer to fill
		halt();                    // Prevent further DMA writes
		reset();
		if (kbhit()) break;
		maxbyte = 0;               // Initialise max variables
		maxleft = maxright = -MAXINT;
		switch(identify()) { // Execute different loops depending on data type
			case SB_ID:
				for (bdata = dmabuf; bdata < data_end; bdata++) {
					maxbyte = max(maxbyte, *bdata);
				}
				break;
			case CD_ID:
				for (idata = (int *) dmabuf; idata < (int *) data_end; idata++) {
					maxleft = max(maxleft, *idata);
					idata++;
					maxright = max(maxright, *idata);
				}
				break;
		}
		switch (identify()) {
			case SB_ID:            // Subtract 0x80 before displaying
				bar_display((char) (maxbyte - 128));
				break;
			case CD_ID:
				bar_display(maxleft);
				bar_display(maxright);
				break;
		}
	}
	getch();           // Remove character from keyboard buffer
	_setcursortype(_NORMALCURSOR);   // Restore cursor
    if (FirstTime) {
        cprintf("\r\n\r\nReading sample...    (Press 'p' to pause, anything else to quit)\r\n");
        FirstTime=0;
    } else {
        cprintf("\r\n\r\nContinuing...\r\n");
    }
	return;
}

// Function: bar_display (signed byte)
// Converts byte (0 to 7F) to int and calls the real bar_display function

static void bar_display(char val) {
	int ival = val * (MAXINT / 127);
	bar_display(ival);
    putch('\r');
}

// Function: bar_display (int)
// Prints a row of asterisks corresponding to the size of 'val'. Always
// completes the line with spaces to eliminate any previous characters.

static void bar_display(int val) {
	if (val < 0) val = 0;
    int bar_len = val / (MAXINT / 79);
	int i;
    for (i = 0; i < 79; i++) {
        putch(i <= bar_len ? 'ş' : ' ');
	}
	return;
}
