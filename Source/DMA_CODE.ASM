; DMA_CODE.ASM

                IDEAL
                Model compact

;+---------------------------------------------------------------------------+
;|  IBM-PC(tm) compatible programmer's DMA library                           |
;|  Version 2.0 by Christopher M. Box                                        |
;+---------------------------------------------------------------------------+
;|  Version 1.1 copyright (C) 1992, Heath I Hunnicutt                        |
;+---------------------------------------------------------------------------+
;|  Thanks to: Gary Nemirovsky, Heath I Hunnicutt                            |
;+---------------------------------------------------------------------------+
;|  This document is for free public distribution.  It is unlawful to        |
;|  sell this document, or any work based substantially upon it.             |
;+---------------------------------------------------------------------------+
;| PUBLIC DMA FUNCTIONS                                                      |
;| int dma_setup(int Channel,char far *Buffer,unsigned Length,int Dir)       |
;| int prevent_dma(int Channel)                                              |
;| int allow_dma(int Channel)                                                |
;| unsigned dma_count(int Channel)                                           |
;| unsigned dma_addr(int Channel)                                            |
;+---------------------------------------------------------------------------+
;| PUBLIC DATA                                                               |
;| int dma_errno                                                             |
;| char far *dma_errlist[]                                                   |
;+---------------------------------------------------------------------------+
;| HISTORY:                                                                  |
;|   Ver 1.0 - Initial Release                                               |
;|   Ver 1.1 - Error checking and reporting added to all functions           |
;|             dma_setup(..) should never crash your system now.             |
;|   Ver 1.2 - Changed mode to auto-initialise (Mod. by CMB)                 |
;|   Ver 2.0 - Major changes to deal with 2nd DMA controller (by CMB)        |
;+---------------------------------------------------------------------------+

Status          EQU     08h     ;DMAC status port (read)     \  same port
Command         EQU     08h     ;DMAC command port (write)   /  (read/write)
Request         EQU     09h     ;DMAC channel request (write-only)
DMA_Mask        EQU     0Ah     ;DMAC DMA_Mask (write-only)
Mode            EQU     0Bh     ;DMAC mode (write)
byte_ptr        EQU     0Ch     ;byte pointer flip-flop

addr            EQU  000h  ; per-channel base address
count           EQU  001h  ; per-channel byte count

IFDEF NON_AUTOINIT
read_cmd        EQU     048h    ; read mode
write_cmd       EQU     044h    ; write mode
ELSE
read_cmd        EQU     058h    ; autoinitialising read
write_cmd       EQU     054h    ; auto write
ENDIF

set_cmd         EQU     000h    ; DMA_Mask set (enable dma)
reset_cmd       EQU     004h    ; DMA_Mask reset (disable)

                DATASEG
; dma controller page register table
; this table maps from channel number to the i/o port number of the
; page register for that channel

page_table  DW  00087h  ; channel 0
            DW  00083h  ; channel 1
            DW  00081h  ; channel 2
            DW  00082h  ; channel 3
            DW  0ffffh  ; ch 4 (not used)
            DW  0008Bh  ; ch 5
            DW  00089h  ; ch 6
            DW  0008Ah  ; ch 7

dmac2        DB 0    ; Flag set to non-zero when using the 2nd DMA controller

; "Extra" messages are for future compatability with the Virtual DMA
; specification.
DMA_E0          DB      0
DMA_E1          DB      "Region not in contiguous memory.",0
DMA_E2          DB      "Region crossed a physical alignment boundary.",0
DMA_E3          DB      "Unable to lock pages.",0
DMA_E4          DB      "No buffer available.",0
DMA_E5          DB      "Region too large for buffer.",0
DMA_E6          DB      "Buffer currently in use.",0
DMA_E7          DB      "Invalid memory region.",0
DMA_E8          DB      "Region was not locked.",0
DMA_E9          DB      "Number of physical pages greater than table length.",0
DMA_EA          DB      "Invalid buffer ID.",0
DMA_EB          DB      "Copy out of buffer range.",0
DMA_EC          DB      "Invalid DMA channel number.",0
DMA_ED          DB      "Buffer not word-aligned.",0
_dma_errlist DD DMA_E0, DMA_E1, DMA_E2, DMA_E3, DMA_E4, DMA_E5, DMA_E6, DMA_E7,\
                DMA_E8, DMA_E9, DMA_EA, DMA_EB, DMA_EC, DMA_ED
_dma_errno   DW 0

;char near *dma_errlist[]
;int dma_errno
PUBLIC _dma_errlist,_dma_errno

                CODESEG
MACRO zero reg
      xor reg,reg
ENDM zero

MACRO adjust reg          ; Adjust register port for 2nd DMA cont
local no_adjust
      cmp [dmac2], 0
      jz no_adjust
      shl reg,1
      add reg,0C0h
no_adjust:
ENDM adjust

PUBLIC _dma_setup,_prevent_dma,_allow_dma,_dma_count,_dma_addr
;+---------------------------------------------------------------------------+
;| int dma_setup(int Channel,char far *Buffer,unsigned Length,int Dir)       |
;| ------------------------------------------------------------------------- |
;| Channel = 0-7                                                             |
;| Buffer  = Address of data to transfer                                     |
;| Length  = Length of data to transfer                                      |
;| Dir     = Direction to move bytes.  1 == Out to the BUS (TO the card)     |
;|                                     0 == In from the BUS and cards.       |
;| ------------------------------------------------------------------------- |
;| Returns: 0 if no errors (dma_errno == 0)                                  |
;|         -1 if errors occurred (dma_errno set to indicate error.)          |
;+---------------------------------------------------------------------------+
PROC _dma_setup
ARG Channel:WORD,Buffer:DWORD,Len:WORD,Dir:WORD
  push bp
  mov  bp,sp
        push bx cx dx si di
  pushf

        mov  [_dma_errno],0
;Convert seg:ofs Buffer to 20-bit physical address
;Assumes operating in 8086/real-Mode
        mov  bx,[WORD PTR Buffer]
        mov  ax,[WORD PTR Buffer+2]
        mov  cl,4
        rol  ax,cl
        mov  ch,al
        and  al,0F0h
        add  ax,bx
        adc  ch,0
        and  ch,0Fh
        mov  di,ax
; (ch << 16) + di == The physical buffer base.

; Check channel number range
        mov  [dmac2],0
        mov  bx,[Channel]
        cmp  bx,7
        jbe  @@Set_base
        mov  [_dma_errno],0Ch
        mov  ax,-1
        jmp  @@ExitPt
@@Set_base:
        cmp  bx,4
        jb   @@OkChannel
        sub  bx,4
        inc  [dmac2]
@@OkChannel: ; BX contains the adjusted channel number

;Determine which command byte will be written later
        cmp  [WORD PTR Dir],0
        jnz SHORT @@Do_Read
        mov  al,write_cmd
        jmp short @@Do_Mode
@@Do_Read:
        mov  al,read_cmd
@@Do_Mode:
        add  al,bl
        zero ah
        mov  si,ax
        mov  ax,set_cmd    ;allow dma requests
        add  al,bl
        mov  cl,al
;si contains READ/WRITE command for DMA controller
;cl contains confirmation command for DMA controller
        shl  bx,1
;bx == Port # Channel*2

;-------------------------------------------------------------------------
; Calculations have been done ahead of time to minimize time with
; interrupts disabled.
;
; ch:di == physical base address (must be on word boundary for 16 bits)
;
; cl == Confirmation command    (Unmasks the channel)
;
; bx == I/O port Channel*2      (This is where the address is written)
;
; si == Mode command for DMA
;-------------------------------------------------------------------------
        mov  ax,di              ;Let's check the address to see if we
        add  ax,[Len]           ;span a page boundary with our length
        jnc @@BoundaryOk        ;Do we?
        mov  [_dma_errno],2     ; y: Error #2
        mov  ax,-1              ;    Return -1
        jmp @@ExitPt            ;    See ya...
@@BoundaryOk:                   ; n: Continue with action
; Now we shift the address and word count right one bit if in 16 bit mode.
        cmp  [dmac2],0
        jz   @@AddrOk
        shr  ch,1
        rcr  di,1
        shl  ch,1
        jc   @@BadAddr
        rcr  [Len],1            ;Odd byte lengths are rounded down
        jmp  @@AddrOk
@@BadAddr:
        mov  [_dma_errno],13    ;Word-alignment error message
        mov  ax,-1
        jmp  @@ExitPt
@@AddrOk:
        cli                     ;Disable interrupts while mucking with DMA

;The "byte pointer" is also known as the LSB/MSB flip flop.
;By writing any value to it, the DMA controller registers are prepared
;to accept the address and length values LSB first.
        mov  dx,byte_ptr        ;Reset byte pointer Flip/flop
        adjust dx
        out  dx,al              ;All we have to do is write to it

        mov  ax,di              ;ax=LSW of 20-bit address
        mov  dx,bx              ;dx=DMAC Base Address port
        adjust dx
        out  dx,al              ;Store LSB
        mov  al,ah
        out  dx,al              ;Store next byte

;Write length to port (Channel*2 + 1)
        mov  ax,[Len]
        mov  dx,bx
        inc  dx                 ;dx=DMAC Count port
        adjust dx
        out  dx,al              ;Write LSB of Length
        mov  al,ah
        out  dx,al              ;Write MSB

        mov  bx,[Channel]
        shl  bx,1
        mov  dx,[bx + OFFSET page_table]  ;dx=page register port
        mov  al,ch              ;al=Page number
        out  dx,al              ;Store the page

        mov  dx,Mode            ;dx=DMAC mode register
        adjust dx
        mov  ax,si              ;Load pre-calculated mode
        out  dx,al              ;Write it to the DSP

IFNDEF NO_START
        mov  dx,DMA_Mask        ;dx=DMAX DMA_Mask register
        adjust dx
        mov  al,cl              ;al=pre-calulated DMA_Mask value
        out  dx,al              ;Write DMA_Mask (allow dma on this channel)
ENDIF
        mov  ax,0               ;Return with no error

@@ExitPt:                       ;Restore stack and return
        popf
        pop  di si dx cx bx
        pop  bp
        ret
ENDP _dma_setup

;+---------------------------------------------------------------------------+
;| int prevent_dma(int Channel)                                              |
;| ------------------------------------------------------------------------- |
;| Channel = 0-7                                                             |
;|         Prevents DMA requests from Channel by masking bit in DMA_C.       |
;| ------------------------------------------------------------------------- |
;| Returns 0 if Ok, -1 and sets dma_errno on error                           |
;+---------------------------------------------------------------------------+
PROC _prevent_dma
ARG Channel:Word
  push bp
        mov  bp,sp
        mov  ax, reset_cmd      ; Disable DMA requests
Send_ax_mask:                    ; Jump here from _allow_dma
        push dx
        mov  [_dma_errno],0
; Check channel number range
        mov  [dmac2],0
        mov  bx,[Channel]
        cmp  bx,7
        jbe  @@Set_base
        mov  [_dma_errno],0Ch
        mov  ax,-1
        jmp  @@Exit_Pt
@@Set_base:
        cmp  bx,4
        jb   @@OkChannel
        sub  bx,4
        inc  [dmac2]
@@OkChannel:
        mov  dx,DMA_Mask
        adjust dx
        add  ax,bx        ; Add the adjusted channel number
        out  dx,al
        mov  ax,0
@@Exit_Pt:
        pop  dx
        pop  bp
        ret
ENDP _prevent_dma

;+---------------------------------------------------------------------------+
;| int allow_dma(int Channel)                                                |
;| ------------------------------------------------------------------------- |
;| Channel = 0-7                                                             |
;|         Unmasks DMA on the specified channel.                             |
;| ------------------------------------------------------------------------- |
;| Returns 0 if Ok, -1 and sets dma_errno on error                           |
;+---------------------------------------------------------------------------+
PROC _allow_dma
ARG Channel:Word
  push bp
        mov  bp,sp
        mov  ax,set_cmd
        jmp Send_ax_mask
ENDP _allow_dma

;+---------------------------------------------------------------------------+
;| int dma_count(Channel)                                                    |
;| ------------------------------------------------------------------------- |
;| Channel = 0-7                                                             |
;| ------------------------------------------------------------------------- |
;| Returns: -1 if DMA transaction completed                                  |
;|         (It returns the number of bytes/words left to transfer)           |
;| dma_errno == 0 if no error, otherwise equals error number                 |
;+---------------------------------------------------------------------------+
PROC _dma_count
ARG Channel:Word
  push bp
        mov  bp,sp
        pushf
        push dx
; Check channel number range
        mov  [dmac2],0
        mov  bx,[Channel]
        cmp  bx,7
        jbe  @@Set_base
        mov  [_dma_errno],0Ch
        mov  ax,-1
        jmp  @@Exit_Pt
@@Set_base:
        cmp  bx,4
        jb   @@OkChannel
        sub  bx,4
        inc  [dmac2]
@@OkChannel:
        mov  dx,bx
        shl  dx,1
        add  dx,count
        adjust dx
        cli
        in   al,dx
        mov  ah,al
        in   al,dx
        xchg al,ah
@@Exit_Pt:
        pop  dx
        popf     ; re-enables interrupts
        pop  bp
        ret
ENDP _dma_count

;+---------------------------------------------------------------------------+
;| unsigned dma_addr(Channel)                                                |
;| ------------------------------------------------------------------------- |
;| Channel = 0-7                                                             |
;| ------------------------------------------------------------------------- |
;| Returns: Current address word of that channel                             |
;| Value must be multiplied by 2 for a 16-bit channel.                       |
;| It is best to start at offset 0, ie on a 64K boundary                     |
;| dma_errno == 0 if no error, otherwise equals error number                 |
;+---------------------------------------------------------------------------+
PROC _dma_addr
ARG Channel:Word
  push bp
        mov  bp,sp
        pushf
        push dx
; Check channel number range
        mov  [dmac2],0
        mov  bx,[Channel]
        cmp  bx,7
        jbe  @@Set_base
        mov  [_dma_errno],0Ch
        mov  ax,-1
        jmp  @@Exit_Pt
@@Set_base:
        cmp  bx,4
        jb   @@OkChannel
        sub  bx,4
        inc  [dmac2]
@@OkChannel:
        mov  dx,bx
        shl  dx,1
        adjust dx
        cli
        in   al,dx
        mov  ah,al
        in   al,dx
        xchg al,ah
@@Exit_Pt:
        pop  dx
        popf
        pop  bp
        ret
ENDP _dma_addr

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; The following routine is not part of the DMA code, but lives
; in this file for convenience.
;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

;-------------------------------
; Reset routine for Soundblaster
;-------------------------------
        DATASEG

_SbIOaddr       dw 0220h

public _SbIOaddr

        CODESEG

public _dsp_reset

PROC _dsp_reset
    mov  al,1
    mov  dx,[_SbIOaddr]
    add  dx,6
    out  dx,al
    in   al,dx    ; Wait 4 microsecs
    in   al,dx    ; Each IO port read takes 1 microsec
    in   al,dx    ; on a standard ISA bus
    in   al,dx
    mov  al,0
    out  dx,al
    add  dx,8      ; DX = Data available port
    mov  cx,100
@@Wait:
    in   al,dx
    test al,80h
    jnz  @@Data_Avail
    loop @@Wait
    jmp  @@Exit_no_sb
@@Data_Avail:
    sub  dx,4    ; DX = Data read port
    in   al,dx
    cmp  al,0AAh
    je   @@Exit_sb
    add  dx,4
    loop @@Wait
@@Exit_no_sb:
    mov  ax,1
    ret
@@Exit_sb:
    mov  ax,0
    ret
ENDP _dsp_reset
END
