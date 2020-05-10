// SbDevice.cpp

// Function definitions for the SbDevice class. Allows a program to make
// use of the Soundblaster's DMA recording/playback modes.

// Written by Christopher M. Box (1993).
// Some functions were based on Soundblaster Freedom Project code.

#include <conio.h>
#include <process.h>
#include <dos.h>

#include "sndclass.h"

// Define maximum sampling speeds for SB low-speed mode
#define MAX_LO_PLAY 22222
#define MAX_LO_REC 12048
// Define maximum wait when writing a command to the SB
#define CMD_TIMEOUT 500000UL

// Command array used to store SB commands
static byte sb_cmd_data[5];
static volatile byte sb_cmd_len;

// Constructor - calls ASM SB detection routine and initialises variables

SbDevice::SbDevice(void) {
	Dprint(("Soundblaster initialisation.\r\n"));
	if (dsp_reset()) {
		cprintf("Soundblaster not found.\r\n");
		exists = 0;
	} else {
        exists = 1;
		init_irq();	        // Install interrupt handler
		sb_size = 0;
	}
}

// Destructor - switches off SB

SbDevice::~SbDevice(void) {
	if (exists) {
		Dprint(("Destruct sb device.\r\n"));
		deinit_irq();
		voice(0);          // Turn off voice output
		dsp_reset();       // Reset SB
	}
	prevent_dma(SbDMAchan);
}

// Function: set_rate
// Set the sampling rate, subject to the SB's granular speed-setting ability.
// Stores the resulting rate in the 'rate' variable (this is usually near
// to, but not the same as 'new_rate'). Automatically enables high speed
// mode if necessary, but it needs to know the intended data direction
// ('dir') to do this.

void SbDevice::set_rate(unsigned new_rate, byte dir) {
	byte tc;              // Time constant

	if (!exists) return;
	tc = (byte) (256 - ((1000000L + new_rate/2)/new_rate));
	rate = (unsigned) (1000000L / (256 - tc));
	hi_speed = (rate > (dir == PLAY ? MAX_LO_PLAY : MAX_LO_REC));
	Dprint(("Time constant %i. Hispeed %i.\r\n",(int)tc,hi_speed));
	dsp_cmd(TIME_CONSTANT);         // Command byte for sample rate
	dsp_cmd(tc);                    // Sample rate time constant
}

// Function: dsp_cmd
// Send a command byte ('cmd') to the SB, after waiting for the busy flag to
// clear. If the SB locks up and never clears the busy flag, it prints
// an error message and exits.

void SbDevice::dsp_cmd(byte cmd) {
	unsigned long wait = 0;

	while (inportb(DSP_WRITE_STATUS) & 0x80) {
		if (++wait > CMD_TIMEOUT) {
			cprintf("Timeout while waiting to write command to SB.\r\n");
			exit(1);
		}
	}
	outportb(DSP_WRITE_DATA,cmd);
	Dprint(("Waited %lu to write %x.\r\n",wait,(int)cmd));
}

// Function: voice
// Enables or disables the SB's voice output according to 'state'.

void SbDevice::voice(int state) {
	dsp_cmd((state) ? SPEAKER_ON : SPEAKER_OFF);
}

// Function: buf_dma_start
// Starts buffered DMA to/from the SB. 'buffer' points to the start of
// the buffer area, and 'buflen' holds the length of the buffer (both halves)
// in bytes. 'dir' sets the direction.

void SbDevice::buf_dma_start(byte far *buffer, unsigned buflen, byte dir) {
	byte im, tm;             // Interrupt masks

	if (!exists) {
		cprintf("Fatal error: no SB exists.\r\n");
		exit(-1);
	}

	im = inportb(0x21);
	tm = ~(1 << SbIRQ);
	outportb(0x21,im & tm);      // Enable SB interrupt
	enable();

	// First ensure that the channel is inactive before setting it up
	if (prevent_dma(SbDMAchan)) {
		cprintf("DMA: %s\r\n", dma_errlist[dma_errno]);
		exit(1);
	}

	// Next prepare the DMA controller
	if (dma_setup(SbDMAchan,buffer,buflen-1,dir)) {
		cprintf("DMA setup: %s\r\n", dma_errlist[dma_errno]);
		exit(1);
	}
	direction = dir;

	// Setup Soundblaster for transfer
	set_rate(rate, direction);
	voice(dir == PLAY);
	sb_size = 0;            // SB card forgets last buffer size so remind it
	set_sb_cmds(buflen);    // Work out the commands to send
	int i=0;
	while (i < sb_cmd_len) dsp_cmd(sb_cmd_data[i++]);     // And send them

	Dprint(("Buffered DMA started - length %u\r\n",buflen));
}

// Function: set_sb_cmds
// Private function to work out the necessary commands to start an SB
// transfer, and store these in an array. 'buflen' tells it the total
// number of bytes to play/record.

#define STORE(x) sb_cmd_data[sb_cmd_len++] = (x)

void SbDevice::set_sb_cmds(unsigned buflen) {
	sb_cmd_len = 0;
	unsigned bl = buflen - 1;
	if (hi_speed) {              // Different commands in hispeed mode
		if (buflen != sb_size) {  // Only need to set the length if it hasn't
			STORE(SET_HS_SIZE);   // been used before
			STORE(bl & 0xff);
			STORE(bl >> 8);
		}
		STORE((direction == PLAY) ? HS_DAC : HS_ADC);
	} else {
		STORE((direction == PLAY) ? DMA_8_BIT_DAC : DMA_ADC);
		STORE(bl & 0xff);       // Always set the length in low-speed mode
		STORE(bl >> 8);
	}
	sb_size = buflen;
	Dprint(("%i commands stored.\r\n", (int)sb_cmd_len));
}

// Function: process_keys
// Called by buf_dma_lo and buf_dma_hi whenever a key has been pressed
// (kbhit() is true) while waiting for the DMA to reach the end
// of a buffer. It defines the actions to be taken, depending on the key.
// Returns 0 if the calling function should continue, otherwise it returns
// the character that was pressed.

int SbDevice::process_keys(void) {
	int c;
	c = getch();
//    if (c == 'p') {          // Key 'p' means pause
//        halt();
//        cprintf ("Paused... press any key to continue.\r");
//        getch();
//        cprintf ("Continuing...                       \r\n");
//        cont();
//    } else {                 // Anything else stops the SB DMA
		if (dsp_reset()) cprintf("Bad dsp reset.");
		Dprint(("Terminated.\r\n"));
		return(c);
//    }
//    return 0;
}

// Function: buf_dma_lo
// Called when the foreground routine has finished its task and wishes
// to wait for the DMA to complete the low half-buffer. It checks for overrun
// (DMA already into the high buffer) and keyboard activity. It returns zero
// when the DMA has completed, and non-zero if a key has been pressed. The
// single argument, 'len', defines the length of the low buffer.

int SbDevice::buf_dma_lo(unsigned len) {
	if (len > sb_size) {
		cprintf("Bad length.");
		exit(1);
	}
	register unsigned ad = dma_addr();
	Dprint(("Lo addr = %X. ",ad));
	// Current address needs to be between 0 and len, otherwise
	// something has gone wrong.
	if (ad > len) {
        cprintf("*WARNING* : Overrun - skipping (hi) buffer.  Gap in sample!\r\n");
		while ((ad = dma_addr()) > len);			// Skip high buffer
	}
	Dprint(("\r\n"));
	// If ad==0 then either the CPU is very fast, or the transfer has already
	// finished and auto-initialised. We assume the second case. The first
	// only occurs at the start, and is dealt with in file_dma().
	int c;
	while ((ad=dma_addr()) < len && ad) {
		// Terminate waiting loop if either:
		// 1. DMA current address reaches 'len' or more
		// 2. Current address is zero (after an auto-initialise)
		// Meanwhile, check for keypresses
		if (kbhit()) {
			if ((c=process_keys()) != 0) return(c);
		}
	}
	// Now set up variables for high buffer.
	lo_buf_sz = len;
	if (sb_size == len) {
		Dprint(("Finished buffered DMA.\r\n"));
	}
	return 0;
}

// Function: buf_dma_hi
// This is the corresponding high-buffer function. An extra argument is
// required ('next_buflen') which defines the size of the buffer length to
// be used after the high buffer has completed. This is usually the same
// as the current length, or perhaps zero.

int SbDevice::buf_dma_hi(unsigned len, unsigned next_buflen) {
	len += lo_buf_sz;
    if (len > sb_size) {        // Was: != sb_size (GL)
		cprintf("Bad length.");
		exit(1);
	}
	register unsigned ad = dma_addr();
	Dprint(("Hi addr = %X. ",ad));
	// Address needs to be between buf_size and len
	if (ad < lo_buf_sz) {
        cprintf("*WARNING* : Overrun - skipping (lo) buffer. Gap in sample!\r\n");
		while ((ad = dma_addr()) < lo_buf_sz);	// Skip the low buffer
	}
	Dprint(("\r\n"));
	int c;
	if (next_buflen) {
		// There's another DMA after this so set up an interrupt routine.
		// dma count does not change (assumes buffer len will not get longer)
        int watch_interrupt=0;
		set_sb_cmds(next_buflen);
		setvect(SbIRQ+8, sb_buf_dma_int);    // Select the interrupt handler
		while(sb_cmd_len) {		// Wait for end-of-dma interrupt
			if (kbhit()) {      // (which will set sb_cmd_len back to zero).
				if ((c=process_keys()) != 0) {
					setvect(SbIRQ+8, sb_dummy_int);
					return(c);
				}
			}
            if (!dma_addr())
                if (++watch_interrupt>5) break;
		}
        if (watch_interrupt>5) {
            cprintf ("ERROR: Interrupt not caught - set IRQ to correct value using -i\r\n");
            exit(1);
        }
		Dprint(("Interrupt received.\r\n"));
		setvect(SbIRQ+8, sb_dummy_int);	 // Select the inactive (dummy) handler
	} else {
		// Last block, so don't need to catch the interrupt
		Dprint(("Final high block.\r\n"));
		unsigned ad;
		while ((ad=dma_addr()) < len && ad) {
			if (kbhit() && ((c=process_keys()) != 0)) return(c);
		}
		Dprint(("Finished buffered DMA.\r\n"));
	}
	return 0;
}

// Function: sb_buf_dma_int
// Handler for the "dma-complete" interrupt from the SB. As class member
// functions cannot be interrupt handlers, it is a global function.
// This is the active handler - it sends out the stored list of SB
// commands upon receipt of the interrupt, thus starting a new transfer
// immediately. Note that sb_cmd_len is declared volatile as it is
// decremented by this function, and is used as a signal to the foreground
// routine that the interrupt has occurred.

void far interrupt sb_buf_dma_int(...) {
	inportb(DSP_DATA_AVAIL);      // Acknowledge the interrupt
	outportb(0x20,0x20);          // Send EOI command to int controller
	enable();                     // and allow further interrupts

	byte *data = sb_cmd_data;
	sb_cmd_len++;
	while ((--sb_cmd_len) > 0) {
		while (inportb(DSP_WRITE_STATUS) & 0x80); // Wait for busy flag to clear
		outportb(DSP_WRITE_DATA,*(data++));       // and send out new data
	}
	// sb_cmd_len finishes with 0
}

// Function: sb_dummy_int
// Dummy interrupt handler. Does nothing but acknowledge the interrupt.

void far interrupt sb_dummy_int(...) {
	inportb(DSP_DATA_AVAIL);
	outportb(0x20,0x20);
	enable();
}

// Function: init_irq
// Initialises the interrupts.

void SbDevice::init_irq(void) {
	disable();
	OldIRQ = getvect(0x08 + SbIRQ);       // Save the old vector

	setvect(0x08 + SbIRQ,sb_dummy_int);   // Install the dummy handler
	enable();
}

// Function: deinit_irq
// Removes our interrupt handler and restores the original one.

void SbDevice::deinit_irq(void) {
	byte tm;         // mask

	disable();
	tm = inportb(0x21);
	outportb(0x21, tm | (1 << SbIRQ));
	setvect(0x08 + SbIRQ,OldIRQ);
	enable();
}

// Function: halt
// Temporarily stops sound playback/recording.

void SbDevice::halt(void) {
	if (hi_speed) {               // In high speed mode the only way
		prevent_dma(SbDMAchan);   // is to disable DMA transfers
	} else {
		dsp_cmd(HALT_DMA);        // At low speed we use the "official" way
	}
}

// Function: cont
// Continues sound after a halt().

void SbDevice::cont(void) {
	if (hi_speed) {
		allow_dma(SbDMAchan);
	} else {
		dsp_cmd(CONTINUE_DMA);
	}
}
