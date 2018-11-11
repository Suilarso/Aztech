;******************************************************************************/
; dali_infod_default.asm                                                                                                                                 
;                                                                            
; Chris Sterzik                                                                
; Texas Instruments                                                          
; Built with CCS Version: 4.0.1.01000                                       
;                                                                            
; Revision 1.0    Baseline                                         
;******************************************************************************/

; THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
; REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
; INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
; FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
; COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.
; TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET
; POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY
; INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR
; YOUR USE OF THE PROGRAM.
;
; IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
; CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY
; THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED
; OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT
; OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.
; EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF
; REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS
; OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF
; USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S
; AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF
; YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS
; (U.S.$500).
;
; Unless otherwise stated, the Program written and copyrighted
; by Texas Instruments is distributed as "freeware".  You may,
; only under TI's copyright in the Program, use and modify the
; Program without any charge or restriction.  You may
; distribute to third parties, provided that you transfer a
; copy of this license to the third party and the third party
; agrees to these terms by its first use of the Program. You
; must reproduce the copyright notice and any other legend of
; ownership on each copy or partial copy, of the Program.
;
; You acknowledge and agree that the Program contains
; copyrighted material, trade secrets and other TI proprietary
; information and is protected by copyright laws,
; international copyright treaties, and trade secret laws, as
; well as other intellectual property laws.  To protect TI's
; rights in the Program, you agree not to decompile, reverse
; engineer, disassemble or otherwise translate any object code
; versions of the Program to a human-readable form.  You agree
; that in no event will you alter, remove or destroy any
; copyright notice included in the Program.  TI reserves all
; rights not specifically granted under this license. Except
; as specifically provided herein, nothing in this agreement
; shall be construed as conferring by implication, estoppel,
; or otherwise, upon you, any license or other right under any
; TI patents, copyrights or trade secrets.

            .cdecls C,LIST, "global_var.h"
            .cdecls C,LIST, "msp430g2332.h"

            .sect   ".infoD"   ;0x1000
            .byte   0xC0   ;0-Timer const MSB
            .byte   0xFF   ;1-TBD
            .byte   0xEE   ;2-TBD

            .sect   ".infoC"   ;0x1040
            .byte   0x00  ;0
            .byte   0x01  ;1
            .byte   0x02  ;2
            .byte   0x03  ;3
            .byte   0x04  ;4
            .byte   0x05  ;5
            .byte   0x06  ;6
            .byte   0x07  ;7
            .byte   0x08  ;8
            .byte   0x09  ;9
            .byte   0x00  ;10
            .byte   0x00  ;11
            .byte   0x00  ;12
            .byte   0x00  ;13
            .byte   0x00  ;14
            .byte   0x00  ;15
            .byte   0x00  ;16
            .byte   0x0A  ;17
            .byte   0x0B  ;18
            .byte   0x0C  ;19
            .byte   0x0D  ;20
            .byte   0x0E  ;21
            .byte   0x0F  ;22
            .byte   0x00  ;23
            .byte   0x00  ;24
            .byte   0x00  ;25
            .byte   0x00  ;26
            .byte   0x00  ;27
            .byte   0x00  ;28
            .byte   0x00  ;29
            .byte   0x00  ;30
            .byte   0x00  ;31

            .sect   ".infoB"   ;0x1080
            .byte   0xC3   ;0-Timer const MSB
            .byte   0x50   ;1-Timer const LSB
            .byte   0x06   ;2-P1SEL
            .byte   0xDB   ;3-P1DIR
            .byte   0x00   ;4-P2SEL
            .byte   0x00   ;5-P2DIR
            .byte   0x00   ;6-P3SEL
            .byte   0x00   ;7-P3DIR
            .byte   0xC0   ;8-ADC
            .byte   0xFF   ;9-TBD
            .byte   0xEE   ;10-TBD
            .byte   0x32   ;11-Start EEPROM: 22024738-001
            .byte   0x32   ;12-
            .byte   0x30   ;13-
            .byte   0x32   ;14-
            .byte   0x34   ;15-
            .byte   0x37   ;16-
            .byte   0x33   ;17-
            .byte   0x38   ;18-
            .byte   0x2D   ;19-
            .byte   0x30   ;20-
            .byte   0x30   ;21-
            .byte   0x31   ;22-End of EEPROM data
            .byte   0x01   ;23-Firmware location
            .byte   0x00   ;24-Last digit of product ID
            .byte   0x01   ;25-Release code
            .byte   0x01   ;26-Version code
            .end
