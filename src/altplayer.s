;-------------------------------------------------------------------------------
; GoatTracker V2.74 stereo alternative SID write order playroutine
;
; NOTE: This playroutine source code does not fall under the GPL license!
; Use it, or song binaries created from it freely for any purpose, commercial
; or noncommercial.
;
; NOTE 2: This code is in the format of Magnus Lind's assembler from Exomizer.
; Does not directly compile on DASM etc.
;-------------------------------------------------------------------------------

        ;Defines will be inserted by the relocator here

mt_temp1        = zpbase+0
mt_temp2        = zpbase+1

        ;Defines for the music data
        ;Patterndata notes

ENDPATT         = $00
INS             = $00
FX              = $40
FXONLY          = $50
NOTE            = $60
REST            = $bd
KEYOFF          = $be
KEYON           = $bf
FIRSTPACKEDREST = $c0
PACKEDREST      = $00

        ;Effects

DONOTHING       = $00
PORTAUP         = $01
PORTADOWN       = $02
TONEPORTA       = $03
VIBRATO         = $04
SETAD           = $05
SETSR           = $06
SETWAVE         = $07
SETWAVEPTR      = $08
SETPULSEPTR     = $09
SETFILTPTR      = $0a
SETFILTCTRL     = $0b
SETFILTCUTOFF   = $0c
SETMASTERVOL    = $0d
SETFUNKTEMPO    = $0e
SETTEMPO        = $0f

        ;Orderlist commands

REPEAT          = $d0
TRANSDOWN       = $e0
TRANS           = $f0
TRANSUP         = $f0
LOOPSONG        = $ff

        ;Wave,pulse,filttable comands

LOOPWAVE        = $ff
LOOPPULSE       = $ff
LOOPFILT        = $ff
SETPULSE        = $80
SETFILTER       = $80
SETCUTOFF       = $00

                .ORG (base)

        ;Jump table

                jmp mt_init
                jmp mt_play
              .IF (SOUNDSUPPORT != 0)
                jmp mt_playsfx
              .ENDIF
              .IF (VOLSUPPORT != 0)
                jmp mt_setmastervol
              .ENDIF

        ;Author info

              .IF (NOAUTHORINFO == 0)

authorinfopos   = base + $20
checkpos1:
              .IF ((authorinfopos - checkpos1) > 15)
mt_tick0jumptbl:
                .BYTE (mt_tick0_0 % 256)
                .BYTE (mt_tick0_12 % 256)
                .BYTE (mt_tick0_12 % 256)
                .BYTE (mt_tick0_34 % 256)
                .BYTE (mt_tick0_34 % 256)
                .BYTE (mt_tick0_5 % 256)
                .BYTE (mt_tick0_6 % 256)
                .BYTE (mt_tick0_7 % 256)
                .BYTE (mt_tick0_8 % 256)
                .BYTE (mt_tick0_9 % 256)
                .BYTE (mt_tick0_a % 256)
                .BYTE (mt_tick0_b % 256)
                .BYTE (mt_tick0_c % 256)
                .BYTE (mt_tick0_d % 256)
                .BYTE (mt_tick0_e % 256)
                .BYTE (mt_tick0_f % 256)
              .ENDIF

checkpos2:
              .IF ((authorinfopos - checkpos2) > 4)
mt_effectjumptbl:
                .BYTE (mt_effect_0 % 256)
                .BYTE (mt_effect_12 % 256)
                .BYTE (mt_effect_12 % 256)
                .BYTE (mt_effect_3 % 256)
                .BYTE (mt_effect_4 % 256)
              .ENDIF

checkpos3:
              .IF ((authorinfopos - checkpos3) > 1)
mt_funktempotbl:
                .BYTE (8,5)
              .ENDIF

        ;This is pretty stupid way of filling left-out space, but .ORG
        ;seemed to bug

checkpos4:
              .IF ((authorinfopos - checkpos4) > 0) .BYTE (0) .ENDIF
checkpos5:
              .IF ((authorinfopos - checkpos5) > 0) .BYTE (0) .ENDIF
checkpos6:
              .IF ((authorinfopos - checkpos6) > 0) .BYTE (0) .ENDIF

mt_author:

                .BYTE (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
              .ENDIF

        ;0 Instrument vibrato

mt_tick0_0:
              .IF (NOEFFECTS == 0)
              .IF (NOINSTRVIB == 0)
                lda mt_insvibparam-1,y
                jmp mt_tick0_34
              .ELSE
              .IF (NOVIB == 0)
                lda #$00
                jmp mt_tick0_34
              .ENDIF
              .ENDIF
              .ENDIF

        ;1,2 Portamentos


mt_tick0_12:
              .IF (NOVIB == 0)
                tay
                lda #$00
                sta mt_chnvibtime,x
                tya
              .ENDIF

        ;3,4 Toneportamento, Vibrato

mt_tick0_34:
              .IF (NOEFFECTS == 0)
              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0) || (NOVIB == 0))
                sta mt_chnparam,x
                lda mt_chnnewfx,x
                sta mt_chnfx,x
              .ENDIF
                rts
              .ENDIF

        ;5 Set AD

mt_tick0_5:
              .IF (NOSETAD == 0)
                sta mt_chnad,x
                rts
              .ENDIF

        ;6 Set Sustain/Release

mt_tick0_6:
              .IF (NOSETSR == 0)
                sta mt_chnsr,x
                rts
              .ENDIF

        ;7 Set waveform

mt_tick0_7:
              .IF (NOSETWAVE == 0)
                sta mt_chnwave,x
                rts
              .ENDIF

        ;8 Set wavepointer

mt_tick0_8:
              .IF (NOSETWAVEPTR == 0)
                sta mt_chnwaveptr,x
              .IF (NOWAVEDELAY == 0)
                lda #$00                        ;Make sure possible delayed
                sta mt_chnwavetime,x            ;waveform execution goes
              .ENDIF                            ;correctly
                rts
              .ENDIF

        ;9 Set pulsepointer

mt_tick0_9:
              .IF (NOSETPULSEPTR == 0)
                sta mt_chnpulseptr,x
                lda #$00                        ;Reset pulse step duration
                sta mt_chnpulsetime,x
                rts
              .ENDIF

        ;a Set filtpointer

mt_tick0_a:     
              .IF (NOSETFILTPTR == 0)
                cpx #21
                bcs mt_tick0_a_sid2
              .IF (NOFILTERMOD == 0)
                ldy #$00
                sty mt_filttime+1
              .ENDIF
mt_tick0_a_step:
                sta mt_filtstep+1
                rts
              .ENDIF

mt_tick0_a_sid2:
              .IF (NOSETFILTPTR == 0)
              .IF (NOFILTERMOD == 0)
                ldy #$00
                sty mt_filttime_sid2+1
              .ENDIF
mt_tick0_a_step_sid2:
                sta mt_filtstep_sid2+1
                rts
              .ENDIF

        ;b Set filtcontrol (channels & resonance)

mt_tick0_b:
              .IF (NOSETFILTCTRL == 0)
                cpx #21
                bcs mt_tick0_b_sid2
                sta mt_filtctrl+1
                cmp #$00
              .IF (NOSETFILTPTR == 0)
                beq mt_tick0_a_step          ;If 0, stop also step-programming
              .ELSE
                bne mt_tick0_b_noset
                sta mt_filtstep+1
mt_tick0_b_noset:
              .ENDIF
                rts
              .ENDIF

mt_tick0_b_sid2:
              .IF (NOSETFILTCTRL == 0)
                sta mt_filtctrl_sid2+1
                cmp #$00
              .IF (NOSETFILTPTR == 0)
                beq mt_tick0_a_step_sid2          ;If 0, stop also step-programming
              .ELSE
                bne mt_tick0_b_noset_sid2
                sta mt_filtstep_sid2+1
mt_tick0_b_noset_sid2:
              .ENDIF
                rts
              .ENDIF


        ;c Set cutoff

mt_tick0_c:
              .IF (NOSETFILTCUTOFF == 0)
                cpx #21
                bcs mt_tick0_c_sid2
                sta mt_filtcutoff+1
                rts
              .ENDIF

mt_tick0_c_sid2:
              .IF (NOSETFILTCUTOFF == 0)
                sta mt_filtcutoff_sid2+1
                rts
              .ENDIF

        ;d Set mastervolume / timing mark

mt_tick0_d:
              .IF (NOSETMASTERVOL == 0)
              .IF (NOAUTHORINFO == 0)
                cmp #$10
                bcs mt_tick0_d_timing
              .ENDIF
mt_setmastervol:
                sta mt_masterfader+1
                rts
              .IF (NOAUTHORINFO == 0)
mt_tick0_d_timing:
                sta mt_author+31
                rts
              .ENDIF
              .ENDIF

        ;e Funktempo

mt_tick0_e:
              .IF (NOFUNKTEMPO == 0)
                tay
                lda mt_speedlefttbl-1,y
                sta mt_funktempotbl
                lda mt_speedrighttbl-1,y
                sta mt_funktempotbl+1
                lda #$00
              .IF (NOCHANNELTEMPO == 0)
                beq mt_tick0_f_setglobaltempo
              .ENDIF
              .ENDIF

        ;f Set Tempo

mt_tick0_f:
              .IF ((NOCHANNELTEMPO == 0) && (NOGLOBALTEMPO == 0))
                bmi mt_tick0_f_setchantempo     ;Channel or global tempo?
              .ENDIF
mt_tick0_f_setglobaltempo:
              .IF (NOGLOBALTEMPO == 0)
                sta mt_chntempo
                sta mt_chntempo+7
                sta mt_chntempo+14
                sta mt_chntempo+21
                sta mt_chntempo+28
                sta mt_chntempo+35                
                rts
              .ENDIF
mt_tick0_f_setchantempo:
              .IF (NOCHANNELTEMPO == 0)
                and #$7f
                sta mt_chntempo,x
                rts
              .ENDIF

        ;Continuous effect code

        ;0 Instrument vibrato

              .IF (NOINSTRVIB == 0)
mt_effect_0_delay:
                dec mt_chnvibdelay,x
mt_effect_0_donothing:
                jmp mt_done
mt_effect_0:    beq mt_effect_0_donothing         ;Speed 0 = no vibrato at all
                lda mt_chnvibdelay,x
                bne mt_effect_0_delay
              .ELSE
mt_effect_0:
mt_effect_0_donothing:
                jmp mt_done
              .ENDIF

        ;4 Vibrato

mt_effect_4:
              .IF (NOVIB == 0)
              .IF (NOCALCULATEDSPEED == 0)
                lda mt_speedlefttbl-1,y
              .IF (NONORMALSPEED == 0)
                bmi mt_effect_4_nohibyteclear
                ldy #$00                        ;Clear speed highbyte
                sty <mt_temp2
              .ENDIF
mt_effect_4_nohibyteclear:
                and #$7f
                sta mt_effect_4_speedcmp+1
              .ELSE
                lda #$00                        ;Clear speed highbyte
                sta <mt_temp2
              .ENDIF
                lda mt_chnvibtime,x
                bmi mt_effect_4_nodir
              .IF (NOCALCULATEDSPEED != 0)
                cmp mt_speedlefttbl-1,y
              .ELSE
mt_effect_4_speedcmp:
                cmp #$00
              .ENDIF
                bcc mt_effect_4_nodir2
                beq mt_effect_4_nodir
                eor #$ff
mt_effect_4_nodir:
                clc
mt_effect_4_nodir2:
                adc #$02
mt_vibdone:
                sta mt_chnvibtime,x
                lsr
                bcc mt_freqadd
                bcs mt_freqsub
              .ENDIF

        ;1,2,3 Portamentos

mt_effect_3:
              .IF (NOTONEPORTA == 0)
                tya
                beq mt_effect_3_found           ;Speed $00 = tie note
              .ENDIF
mt_effect_12:
              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0))
              .IF (NOCALCULATEDSPEED != 0)
                lda mt_speedlefttbl-1,y
                sta <mt_temp2
              .ENDIF
              .ENDIF
              .IF (NOPORTAMENTO == 0)

              .IF (NOWAVECMD != 0)
                lda mt_chnfx,x
              .ELSE
mt_effectnum:
                lda #$00
              .ENDIF
                cmp #$02
                bcc mt_freqadd
                beq mt_freqsub
              .ELSE
              .IF (NOTONEPORTA == 0)
                sec
              .ENDIF
              .ENDIF
              .IF (NOTONEPORTA == 0)
                ldy mt_chnnote,y
                lda mt_chnfreqlo,x              ;Calculate offset to the
                sbc mt_freqtbllo-FIRSTNOTE,y    ;right frequency
                pha
                lda mt_chnfreqhi,x
                sbc mt_freqtblhi-FIRSTNOTE,y
                tay
                pla
                bcs mt_effect_3_down            ;If positive, have to go down

mt_effect_3_up:
                adc <mt_temp1                   ;Add speed to offset
                tya                             ;If changes sign, we're done
                adc <mt_temp2
                bpl mt_effect_3_found
              .ENDIF


              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0) || (NOVIB == 0))
mt_freqadd:
                lda mt_chnfreqlo,x
                adc <mt_temp1
                sta mt_chnfreqlo,x
                lda mt_chnfreqhi,x
                adc <mt_temp2
                jmp mt_storefreqhi
              .ENDIF

              .IF (NOTONEPORTA == 0)
mt_effect_3_down:
                sbc <mt_temp1                   ;Subtract speed from offset
                tya                             ;If changes sign, we're done
                sbc <mt_temp2
                bmi mt_effect_3_found
              .ENDIF

              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0) || (NOVIB == 0))
mt_freqsub:
                lda mt_chnfreqlo,x
                sbc <mt_temp1
                sta mt_chnfreqlo,x
                lda mt_chnfreqhi,x
                sbc <mt_temp2
                jmp mt_storefreqhi
              .ENDIF

mt_effect_3_found:
              .IF (NOTONEPORTA == 0)
              .IF (NOCALCULATEDSPEED == 0)
                lda mt_chnnote,x
                jmp mt_wavenoteabs
              .ELSE
                ldy mt_chnnote,y
                jmp mt_wavenote
              .ENDIF
              .ENDIF

        ;Init routine

mt_init:
              .IF (NUMSONGS > 1)
                asl
                sta mt_init+6
                asl
                adc #$00
              .ENDIF
                sta mt_initsongnum+1
                rts

        ;Play soundeffect -routine

              .IF (SOUNDSUPPORT != 0)
        ;Sound FX init routine

mt_playsfx:     sta mt_playsfxlo+1
                sty mt_playsfxhi+1
                lda mt_chnsfx,x                   ;Need a priority check?
                beq mt_playsfxok
                tya                               ;Check address highbyte
                cmp mt_chnsfxhi,x
                bcc mt_playsfxskip                ;Lower than current -> skip
                bne mt_playsfxok                  ;Higher than current -> OK
                lda mt_playsfxlo+1                ;Check address lowbyte
                cmp mt_chnsfxlo,x
                bcc mt_playsfxskip                ;Lower than current -> skip
mt_playsfxok:   lda #$01
                sta mt_chnsfx,x
mt_playsfxlo:   lda #$00
                sta mt_chnsfxlo,x
mt_playsfxhi:   lda #$00
                sta mt_chnsfxhi,x
mt_playsfxskip: rts
              .ENDIF

        ;Set mastervolume -routine

              .IF ((VOLSUPPORT != 0) && (NOSETMASTERVOL != 0))
mt_setmastervol:
                sta mt_masterfader+1
                rts
              .ENDIF

        ;Playroutine

mt_play:        ldx #$00                        ;Channel index

        ;Song initialization

mt_initsongnum:
                ldy #$00
                bmi mt_filtstep
                txa
                ldx #NUMCHANNELS * 14 - 1
mt_resetloop:
                sta mt_chnsongptr,x             ;Reset sequencer + voice
                dex                             ;variables on all channels
                bpl mt_resetloop
                sta SIDBASE+$15                       ;Reset filter cutoff lowbyte
                sta SID2BASE+$15
                sta mt_filtctrl+1             ;Switch filter off & reset
                sta mt_filtctrl_sid2+1
              .IF (NOFILTER == 0)
                sta mt_filtstep+1             ;step-programming
                sta mt_filtstep_sid2+1
              .ENDIF
                stx mt_initsongnum+1          ;Reset initflag
                tax
                jsr mt_initchn
                ldx #$07
                jsr mt_initchn
                ldx #$0e
                jsr mt_initchn
                ldx #$15
                jsr mt_initchn
                ldx #$1c
                jsr mt_initchn
                ldx #$23
mt_initchn:
              .IF (NUMSONGS > 1)
                tya
                iny
                sta mt_chnsongnum,x             ;Store index to songtable
              .ENDIF
mt_defaulttempo:
                lda #DEFAULTTEMPO               ;Set default tempo
                sta mt_chntempo,x
                lda #$01
                sta mt_chncounter,x             ;Reset counter
                sta mt_chninstr,x               ;Reset instrument
                jmp mt_loadregswave

        ;Filter execution

mt_filtstep:
              .IF (NOFILTER == 0)
                ldy #$00                        ;See if filter stopped
                beq mt_filtdone
              .IF (NOFILTERMOD == 0)
mt_filttime:
                lda #$00                        ;See if time left for mod.
                bne mt_filtmod                  ;step
              .ENDIF
mt_newfiltstep:
                lda mt_filttimetbl-1,y          ;$80-> = set filt parameters
                beq mt_setcutoff                ;$00 = set cutoff
              .IF (NOFILTERMOD == 0)
                bpl mt_newfiltmod
              .ENDIF
mt_setfilt:
                asl                             ;Set passband
                sta mt_filttype+1
                lda mt_filtspdtbl-1,y           ;Set resonance/channel
                sta mt_filtctrl+1
                lda mt_filttimetbl,y            ;Check for cutoff setting
                bne mt_nextfiltstep2            ;following immediately
mt_setcutoff2:
                iny
mt_setcutoff:
                lda mt_filtspdtbl-1,y           ;Take cutoff value
                sta mt_filtcutoff+1
              .IF (NOFILTERMOD == 0)
                jmp mt_nextfiltstep
mt_newfiltmod:
                sta mt_filttime+1               ;$01-$7f = new modulation step
mt_filtmod:   
                lda mt_filtspdtbl-1,y           ;Take filt speed
                clc
                adc mt_filtcutoff+1
                sta mt_filtcutoff+1
                dec mt_filttime+1
                bne mt_storecutoff
              .ENDIF
mt_nextfiltstep:
                lda mt_filttimetbl,y           ;Jump in filttable?
mt_nextfiltstep2:
                cmp #LOOPFILT
                iny
                tya
                bcc mt_nofiltjump
                lda mt_filtspdtbl-1,y          ;Take jump point
mt_nofiltjump:
                sta mt_filtstep+1
mt_filtdone:
mt_filtcutoff:
                lda #$00
mt_storecutoff:
                sta SIDBASE+$16
              .ENDIF
mt_filtctrl:
                lda #$00
                sta SIDBASE+$17
mt_filttype:
                lda #$00
mt_masterfader:
                ora #$0f                        ;Master volume fader
                sta SIDBASE+$18

        ;Second filter execution

mt_filtstep_sid2:
              .IF (NOFILTER == 0)
                ldy #$00                        ;See if filter stopped
                beq mt_filtdone_sid2
              .IF (NOFILTERMOD == 0)
mt_filttime_sid2:
                lda #$00                        ;See if time left for mod.
                bne mt_filtmod_sid2                  ;step
              .ENDIF
mt_newfiltstep_sid2:
                lda mt_filttimetbl-1,y          ;$80-> = set filt parameters
                beq mt_setcutoff_sid2                ;$00 = set cutoff
              .IF (NOFILTERMOD == 0)
                bpl mt_newfiltmod_sid2
              .ENDIF
mt_setfilt_sid2:
                asl                             ;Set passband
                sta mt_filttype_sid2+1
                lda mt_filtspdtbl-1,y           ;Set resonance/channel
                sta mt_filtctrl_sid2+1
                lda mt_filttimetbl,y            ;Check for cutoff setting
                bne mt_nextfiltstep2_sid2            ;following immediately
mt_setcutoff2_sid2:
                iny
mt_setcutoff_sid2:
                lda mt_filtspdtbl-1,y           ;Take cutoff value
                sta mt_filtcutoff_sid2+1
              .IF (NOFILTERMOD == 0)
                jmp mt_nextfiltstep_sid2
mt_newfiltmod_sid2:
                sta mt_filttime_sid2+1               ;$01-$7f = new modulation step
mt_filtmod_sid2:
                lda mt_filtspdtbl-1,y           ;Take filt speed
                clc
                adc mt_filtcutoff_sid2+1
                sta mt_filtcutoff_sid2+1
                dec mt_filttime_sid2+1
                bne mt_storecutoff_sid2
              .ENDIF
mt_nextfiltstep_sid2:
                lda mt_filttimetbl,y           ;Jump in filttable?
mt_nextfiltstep2_sid2:
                cmp #LOOPFILT
                iny
                tya
                bcc mt_nofiltjump_sid2
                lda mt_filtspdtbl-1,y          ;Take jump point
mt_nofiltjump_sid2:
                sta mt_filtstep_sid2+1
mt_filtdone_sid2:
mt_filtcutoff_sid2:
                lda #$00
mt_storecutoff_sid2:
                sta SID2BASE+$16
              .ENDIF
mt_filtctrl_sid2:
                lda #$00
                sta SID2BASE+$17
mt_filttype_sid2:
                lda #$00
                ora mt_masterfader+1           ;Master volume fader
                sta SID2BASE+$18

        ;Channel execution

                jsr mt_execchn
                ldx #$07
                jsr mt_execchn
                ldx #$0e
                jsr mt_execchn
                ldx #$15
                jsr mt_execchn
                ldx #$1c
                jsr mt_execchn
                ldx #$23

mt_execchn:
                dec mt_chncounter,x               ;See if tick 0
                beq mt_tick0

        ;Ticks 1-n

mt_notick0:
                bpl mt_effects
                lda mt_chntempo,x               ;Reload tempo if negative

              .IF (NOFUNKTEMPO == 0)
                cmp #$02
                bcs mt_nofunktempo              ;Funktempo: bounce between
                tay                             ;funktable indexes 0,1
                eor #$01
                sta mt_chntempo,x
                lda mt_funktempotbl,y
                sbc #$00
              .ENDIF

mt_nofunktempo:
                sta mt_chncounter,x
mt_effects:
                jmp mt_waveexec

        ;Sequencer repeat

mt_repeat:
              .IF (NOREPEAT == 0)
                sbc #REPEAT
                inc mt_chnrepeat,x
                cmp mt_chnrepeat,x
                bne mt_nonewpatt
mt_repeatdone:
                lda #$00
                sta mt_chnrepeat,x
                beq mt_repeatdone2
              .ENDIF

        ;Tick 0

mt_tick0:
              .IF (NOEFFECTS == 0)
                ldy mt_chnnewfx,y               ;Setup tick 0 FX jumps
                lda mt_tick0jumptbl,y
                sta mt_tick0jump1+1
                sta mt_tick0jump2+1
              .ENDIF

        ;Sequencer advance

mt_checknewpatt:
                lda mt_chnpattptr,x             ;Fetch next pattern?
                bne mt_nonewpatt
mt_sequencer:
                ldy mt_chnsongnum,y
                lda mt_songtbllo,y              ;Get address of sequence
                sta <mt_temp1
                lda mt_songtblhi,y
                sta <mt_temp2
                ldy mt_chnsongptr,y
                lda (mt_temp1),y                ;Get pattern from sequence
                cmp #LOOPSONG                   ;Check for loop
                bcc mt_noloop
                iny
                lda (mt_temp1),y
                tay
                lda (mt_temp1),y
mt_noloop:
              .IF (NOTRANS == 0)
                cmp #TRANSDOWN                  ;Check for transpose
                bcc mt_notrans
                sbc #TRANS
                sta mt_chntrans,x
                iny
                lda (mt_temp1),y
              .ENDIF
mt_notrans:
              .IF (NOREPEAT == 0)
                cmp #REPEAT                     ;Check for repeat
                bcs mt_repeat
              .ENDIF
                sta mt_chnpattnum,x             ;Store pattern number
mt_repeatdone2:
                iny
                tya
                sta mt_chnsongptr,x             ;Store songposition

        ;New note start

mt_nonewpatt:
                ldy mt_chninstr,y
              .IF (FIXEDPARAMS == 0)
                lda mt_insgatetimer-1,y
                sta mt_chngatetimer,x
              .ENDIF
                lda mt_chnnewnote,x             ;Test new note init flag
                beq mt_nonewnoteinit
mt_newnoteinit:
                sec
                sbc #NOTE
                sta mt_chnnote,x
                lda #$00
              .IF (NOEFFECTS == 0)
              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0) || (NOVIB == 0))
                sta mt_chnfx,x                  ;Reset effect
              .ENDIF
              .ENDIF
                sta mt_chnnewnote,x             ;Reset newnote action
              .IF (NOINSTRVIB == 0)
                lda mt_insvibdelay-1,y          ;Load instrument vibrato
                sta mt_chnvibdelay,x
              .IF (NOEFFECTS == 0)
                lda mt_insvibparam-1,y
                sta mt_chnparam,x
              .ENDIF
              .ENDIF
              .IF (NOTONEPORTA == 0)
                lda mt_chnnewfx,x               ;If toneportamento, skip
                cmp #TONEPORTA                  ;most of note init
                beq mt_nonewnoteinit
              .ENDIF

              .IF (FIXEDPARAMS == 0)
                lda mt_insfirstwave-1,y         ;Load first frame waveform
              .IF (NOFIRSTWAVECMD == 0)
                beq mt_skipwave
                cmp #$fe
                bcs mt_skipwave2                ;Skip waveform but load gate
              .ENDIF
              .ELSE
                lda #FIRSTWAVEPARAM
              .ENDIF
                sta mt_chnwave,x
              .IF ((NUMLEGATOINSTR > 0) || (NOFIRSTWAVECMD == 0))
                lda #$ff
mt_skipwave2:
                sta mt_chngate,x                ;Reset gateflag
              .ELSE
                inc mt_chngate,x
              .ENDIF
mt_skipwave:

              .IF (NOPULSE == 0)
                lda mt_inspulseptr-1,y          ;Load pulseptr (if nonzero)
                beq mt_skippulse
                sta mt_chnpulseptr,x
              .IF (NOPULSEMOD == 0)
                lda #$00                        ;Reset pulse step duration
                sta mt_chnpulsetime,x
              .ENDIF
              .ENDIF
mt_skippulse:
              .IF (NOFILTER == 0)
                cpx #21
                lda mt_insfiltptr-1,y         ;Load filtptr (if nonzero)
                beq mt_skipfilt
                bcs mt_loadfilt_sid2
mt_loadfilt:
                sta mt_filtstep+1
              .IF (NOFILTERMOD == 0)
                lda #$00
                sta mt_filttime+1
              .ENDIF
                bcc mt_skipfilt
mt_loadfilt_sid2:
                sta mt_filtstep_sid2+1
              .IF (NOFILTERMOD == 0)
                lda #$00
                sta mt_filttime_sid2+1
              .ENDIF
              .ENDIF
mt_skipfilt:

                lda mt_inswaveptr-1,y           ;Load waveptr
                sta mt_chnwaveptr,x
                
				lda mt_insad-1,y                ;Load Attack/Decay
                sta mt_chnad,x
                lda mt_inssr-1,y                ;Load Sustain/Release
                sta mt_chnsr,x

              .IF (NOEFFECTS == 0)
                lda mt_chnnewparam,x            ;Execute tick 0 FX after
mt_tick0jump1:
                jsr mt_tick0_0                  ;newnote init
              .ENDIF
                jmp mt_loadregs

              .IF (NOWAVECMD == 0)
mt_wavecmd:
                jmp mt_execwavecmd
              .ENDIF

        ;Tick 0 effect execution

mt_nonewnoteinit:
              .IF (NOEFFECTS == 0)
                lda mt_chnnewparam,x            ;No new note init: exec tick 0
mt_tick0jump2:
                jsr mt_tick0_0                  ;FX, and wavetable afterwards
              .ENDIF

        ;Wavetable execution

mt_waveexec:
                ldy mt_chnwaveptr,y
                beq mt_wavedone
                lda mt_wavetbl-1,y
              .IF (NOWAVEDELAY == 0)
                cmp #$10                        ;0-15 used as delay
                bcs mt_nowavedelay              ;+ no wave change
                cmp mt_chnwavetime,x
                beq mt_nowavechange
                inc mt_chnwavetime,x
                bne mt_wavedone
mt_nowavedelay:
                sbc #$10
              .ELSE
                beq mt_nowavechange
              .ENDIF
              .IF (NOWAVECMD == 0)
                cmp #$e0
                bcs mt_nowavechange
              .ENDIF
                sta mt_chnwave,x
mt_nowavechange:
                lda mt_wavetbl,y
                cmp #LOOPWAVE                  ;Check for wavetable jump
                iny
                tya
                bcc mt_nowavejump
              .IF (NOWAVECMD != 0)
                clc
              .ENDIF
                lda mt_notetbl-1,y
mt_nowavejump:
                sta mt_chnwaveptr,x
              .IF (NOWAVEDELAY == 0)
                lda #$00
                sta mt_chnwavetime,x
              .ENDIF

              .IF (NOWAVECMD == 0)
                lda mt_wavetbl-2,y
                cmp #$e0
                bcs mt_wavecmd
              .ENDIF

                lda mt_notetbl-2,y

              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0) || (NOVIB == 0))
                bne mt_wavefreq                 ;No frequency-change?

        ;No frequency-change / continuous effect execution

mt_wavedone:
              .IF (REALTIMEOPTIMIZATION != 0)
                lda mt_chncounter,x             ;No continuous effects on tick0
              .IF (PULSEOPTIMIZATION != 0)
                beq mt_gatetimer
              .ELSE
                beq mt_done
              .ENDIF
              .ENDIF
              .IF (NOEFFECTS == 0)
                ldy mt_chnfx,y
              .IF (NOWAVECMD == 0)
              .IF (.DEFINED(mt_effectnum))
                sty mt_effectnum+1
              .ENDIF
              .ENDIF
                lda mt_effectjumptbl,y
                sta mt_effectjump+1
                ldy mt_chnparam,y
              .ELSE
                ldy mt_chninstr,y
                lda mt_insvibparam-1,y
                tay
              .ENDIF
mt_setspeedparam:
              .IF (NOCALCULATEDSPEED != 0)
                lda mt_speedrighttbl-1,y
                sta <mt_temp1
              .ELSE
              .IF (NONORMALSPEED == 0)
                lda mt_speedlefttbl-1,y
                bmi mt_calculatedspeed
mt_normalspeed:
                sta <mt_temp2
                lda mt_speedrighttbl-1,y
                sta <mt_temp1
                jmp mt_effectjump
              .ELSE
              .IF (NOZEROSPEED == 0)
                bne mt_calculatedspeed
mt_zerospeed:
                sty <mt_temp1
                sty <mt_temp2
                beq mt_effectjump
              .ENDIF
              .ENDIF
mt_calculatedspeed:
                lda mt_speedrighttbl-1,y
                sta mt_cscount+1
                sty mt_csresty+1
                ldy mt_chnlastnote,y
                lda mt_freqtbllo+1-FIRSTNOTE,y
                sec
                sbc mt_freqtbllo-FIRSTNOTE,y
                sta <mt_temp1
                lda mt_freqtblhi+1-FIRSTNOTE,y
                sbc mt_freqtblhi-FIRSTNOTE,y
mt_cscount:     ldy #$00
                beq mt_csresty
mt_csloop:      lsr
                ror <mt_temp1
                dey
                bne mt_csloop
mt_csresty:     ldy #$00
                sta <mt_temp2
              .ENDIF
mt_effectjump:
                jmp mt_effect_0
              .ELSE
                beq mt_wavedone
              .ENDIF
              
        ;Setting note frequency

mt_wavefreq:
                bpl mt_wavenoteabs
                adc mt_chnnote,x
                and #$7f
mt_wavenoteabs:
              .IF (NOCALCULATEDSPEED == 0)
                sta mt_chnlastnote,x
              .ENDIF
                tay
mt_wavenote:
              .IF (NOVIB == 0)
                lda #$00                        ;Reset vibrato phase
                sta mt_chnvibtime,x
              .ENDIF
                lda mt_freqtbllo-FIRSTNOTE,y
                sta mt_chnfreqlo,x
                lda mt_freqtblhi-FIRSTNOTE,y
mt_storefreqhi:
                sta mt_chnfreqhi,x


        ;Check for new note fetch

              .IF ((NOTONEPORTA != 0) && (NOPORTAMENTO != 0) && (NOVIB != 0))
mt_wavedone:
              .ENDIF
mt_done:
              .IF (PULSEOPTIMIZATION != 0)
                lda mt_chncounter,x             ;Check for gateoff timer
mt_gatetimer:
              .IF (FIXEDPARAMS == 0)
                cmp mt_chngatetimer,x
              .ELSE
                cmp #GATETIMERPARAM
              .ENDIF

                beq mt_getnewnote               ;Fetch new notes if equal
              .ENDIF

        ;Pulse execution
              .IF (NOPULSE == 0)
mt_pulseexec:
                ldy mt_chnpulseptr,y            ;See if pulse stopped
                beq mt_pulseskip
              .IF (PULSEOPTIMIZATION != 0)
                ora mt_chnpattptr,x             ;Skip when sequencer executed
                beq mt_pulseskip
              .ENDIF
              .IF (NOPULSEMOD == 0)
                lda mt_chnpulsetime,x           ;Pulse step counter time left?
                bne mt_pulsemod
              .ENDIF
mt_newpulsestep:
                lda mt_pulsetimetbl-1,y         ;Set pulse, or new modulation
              .IF (NOPULSEMOD == 0)
                bpl mt_newpulsemod              ;step?
              .ENDIF
mt_setpulse:
              .IF (SIMPLEPULSE == 0)
                sta mt_chnpulsehi,x             ;Highbyte
              .ENDIF
                lda mt_pulsespdtbl-1,y          ;Lowbyte
                sta mt_chnpulselo,x
              .IF (NOPULSEMOD == 0)
                jmp mt_nextpulsestep
mt_newpulsemod:
                sta mt_chnpulsetime,x
mt_pulsemod:
              .IF (SIMPLEPULSE == 0)
                lda mt_pulsespdtbl-1,y          ;Take pulse speed
                clc
                bpl mt_pulseup
                dec mt_chnpulsehi,x
mt_pulseup:
                adc mt_chnpulselo,x             ;Add pulse lowbyte
                sta mt_chnpulselo,x
                bcc mt_pulsenotover
                inc mt_chnpulsehi,x

mt_pulsenotover:
              .ELSE
                lda mt_chnpulselo,x
                clc
                adc mt_pulsespdtbl-1,y
                adc #$00
                sta mt_chnpulselo,x

              .ENDIF
                dec mt_chnpulsetime,x
                bne mt_pulsedone2
              .ENDIF

mt_nextpulsestep:
                lda mt_pulsetimetbl,y           ;Jump in pulsetable?
                cmp #LOOPPULSE
                iny
                tya
                bcc mt_nopulsejump
                lda mt_pulsespdtbl-1,y          ;Take jump point
mt_nopulsejump:
                sta mt_chnpulseptr,x
mt_pulsedone:
mt_pulsedone2:
mt_pulseskip:
              .ENDIF

              .IF (PULSEOPTIMIZATION == 0)
                lda mt_chncounter,x             ;Check for gateoff timer
mt_gatetimer:
              .IF (FIXEDPARAMS == 0)
                cmp mt_chngatetimer,x
              .ELSE
                cmp #GATETIMERPARAM
              .ENDIF

                beq mt_getnewnote               ;Fetch new notes if equal
              .ENDIF

                jmp mt_loadregs

        ;New note fetch

mt_getnewnote:
                ldy mt_chnpattnum,y
                lda mt_patttbllo,y
                sta <mt_temp1
                lda mt_patttblhi,y
                sta <mt_temp2
                ldy mt_chnpattptr,y
                lda (mt_temp1),y
                cmp #FX
                bcc mt_instr                    ;Instr. change
              .IF (NOEFFECTS == 0)
                cmp #NOTE
                bcc mt_fx                       ;FX
              .ENDIF
                cmp #FIRSTPACKEDREST
                bcc mt_note                     ;Note only

        ;Packed rest handling

mt_packedrest:
                lda mt_chnpackedrest,x
                bne mt_packedrestnonew
                lda (mt_temp1),y
mt_packedrestnonew:
                adc #$00
                sta mt_chnpackedrest,x
                beq mt_rest
                bne mt_loadregs

        ;Instrument change

mt_instr:
                sta mt_chninstr,x               ;Instrument change, followed
                iny
                lda (mt_temp1),y                ;by either FX or note

              .IF (NOEFFECTS == 0)
                cmp #NOTE
                bcs mt_note

        ;Effect change

mt_fx:
                cmp #FXONLY                     ;Note follows?
                and #$0f
                sta mt_chnnewfx,x
                beq mt_fx_noparam               ;Effect 0 - no param.
                iny
                lda (mt_temp1),y
                sta mt_chnnewparam,x
mt_fx_noparam:
                bcs mt_rest
mt_fx_getnote:
                iny
                lda (mt_temp1),y
              .ENDIF

        ;Note handling

mt_note:
                cmp #REST                   ;Rest or gateoff/on?
              .IF (NOGATE == 0)
                bcc mt_normalnote
              .ENDIF
                beq mt_rest
mt_gate:
              .IF (NOGATE == 0)
                ora #$f0
                bne mt_setgate
              .ENDIF

        ;Prepare for note start; perform hardrestart

mt_normalnote:
              .IF (NOTRANS == 0)
                adc mt_chntrans,x
              .ENDIF
                sta mt_chnnewnote,x
              .IF (NOTONEPORTA == 0)
                lda mt_chnnewfx,x           ;If toneportamento, no gateoff
                cmp #TONEPORTA
                beq mt_rest
              .ENDIF
              .IF (((NUMHRINSTR > 0) && (NUMNOHRINSTR > 0)) || (NUMLEGATOINSTR > 0))
                lda mt_chninstr,x
                cmp #FIRSTNOHRINSTR         ;Instrument order:
              .IF (NUMLEGATOINSTR > 0)
                bcs mt_nohr_legato          ;With HR - no HR - legato
              .ELSE
                bcs mt_skiphr
              .ENDIF
              .ENDIF
              .IF (NUMHRINSTR > 0)
                lda #ADPARAM                ;Hard restart
                sta mt_chnad,x
                lda #SRPARAM
                sta mt_chnsr,x
              .ENDIF
mt_skiphr:
                lda #$fe
mt_setgate:
                sta mt_chngate,x

        ;Check for end of pattern

mt_rest:
                iny
                lda (mt_temp1),y
                beq mt_endpatt
                tya
mt_endpatt:
                sta mt_chnpattptr,x

        ;Load voice registers

mt_loadregs:
              .IF (SOUNDSUPPORT != 0)
                ldy mt_chnsfx,y
                bne mt_sfxexec
              .ENDIF

mt_loadregswave:
                cpx #21
                bcs mt_loadregswave_sid2

mt_loadregswave_sid1:
                lda mt_chnwave,x
                and mt_chngate,x
                sta SIDBASE+$04,x
                lda mt_chnfreqlo,x
                sta SIDBASE+$00,x
                lda mt_chnfreqhi,x
                sta SIDBASE+$01,x
                lda mt_chnpulselo,x
              .IF (SIMPLEPULSE == 0)
                sta SIDBASE+$02,x
                lda mt_chnpulsehi,x
                sta SIDBASE+$03,x
              .ELSE
                sta SIDBASE+$02,x
                sta SIDBASE+$03,x
              .ENDIF
              .IF (SOUNDSUPPORT != 0)
                lda mt_chnsfx,x
                bne mt_loadskipadsr
              .ENDIF
                lda mt_chnad,x
                sta SIDBASE+$05,x
                lda mt_chnsr,x
                sta SIDBASE+$06,x
mt_loadskipadsr:
                rts


              .IF (NUMLEGATOINSTR > 0)
mt_nohr_legato:
                cmp #FIRSTLEGATOINSTR
                bcc mt_skiphr
                bcs mt_rest
              .ENDIF

mt_loadregswave_sid2:
                lda mt_chnwave,x
                and mt_chngate,x
                sta SID2BASE-21+$04,x
                lda mt_chnfreqlo,x
                sta SID2BASE-21+$00,x
                lda mt_chnfreqhi,x
                sta SID2BASE-21+$01,x
                lda mt_chnpulselo,x
              .IF (SIMPLEPULSE == 0)
                sta SID2BASE-21+$02,x
                lda mt_chnpulsehi,x
                sta SID2BASE-21+$03,x
              .ELSE
                sta SID2BASE-21+$02,x
                sta SID2BASE-21+$03,x
              .ENDIF
              .IF (SOUNDSUPPORT != 0)
                lda mt_chnsfx,x
                bne mt_loadskipadsr_sid2
              .ENDIF
                lda mt_chnad,x
                sta SID2BASE-21+$05,x
                lda mt_chnsr,x
                sta SID2BASE-21+$06,x
mt_loadskipadsr_sid2:
                rts

        ;Sound FX code

              .IF (SOUNDSUPPORT != 0)

mt_sfxexec:     lda mt_chnsfxlo,x
                sta <mt_temp1
                lda mt_chnsfxhi,x
                sta <mt_temp2
                lda #$fe
                sta mt_chngate,x
                lda #$00
                sta mt_chnwaveptr,x
                inc mt_chnsfx,x
                cpx #21
                bcs mt_sfxexec_sid2

                cpy #$02
                beq mt_sfxexec_frame0
                bcs mt_sfxexec_framen
                sta SIDBASE+$05,x                ;Hardrestart before sound FX
                sta SIDBASE+$06,x                ;begins
                jmp mt_loadregswave_sid1
mt_sfxexec_frame0:
                tay
                lda (mt_temp1),y           ;Load ADSR
                sta SIDBASE+$05,x
                iny
                lda (mt_temp1),y
                sta SIDBASE+$06,x
                iny
                lda (mt_temp1),y           ;Load pulse
                sta SIDBASE+$02,x
                sta SIDBASE+$03,x
                lda #$09                   ;Testbit
mt_sfxexec_wavechg:
                sta mt_chnwave,x
                sta SIDBASE+$04,x
mt_sfxexec_done:
                rts
mt_sfxexec_framen:
                lda (mt_temp1),y
                bne mt_sfxexec_noend
mt_sfxexec_end:
                sta mt_chnsfx,x
                beq mt_sfxexec_wavechg
mt_sfxexec_noend:
                tay
                lda mt_freqtbllo-$80,y        ;Get frequency
                sta SIDBASE+$00,x
                lda mt_freqtblhi-$80,y
                sta SIDBASE+$01,x
                ldy mt_chnsfx,y
                lda (mt_temp1),y              ;Then take a look at the next
                beq mt_sfxexec_done           ;byte
                cmp #$82                      ;Is it a waveform or a note?
                bcs mt_sfxexec_done
                inc mt_chnsfx,x
                bcc mt_sfxexec_wavechg

mt_sfxexec_sid2:
                cpy #$02
                beq mt_sfxexec_frame0_sid2
                bcs mt_sfxexec_framen_sid2
                sta SID2BASE-21+$05,x                ;Hardrestart before sound FX
                sta SID2BASE-21+$06,x                ;begins
                jmp mt_loadregswave_sid2
mt_sfxexec_frame0_sid2:
                tay
                lda (mt_temp1),y           ;Load ADSR
                sta SID2BASE-21+$05,x
                iny
                lda (mt_temp1),y
                sta SID2BASE-21+$06,x
                iny
                lda (mt_temp1),y           ;Load pulse
                sta SID2BASE-21+$02,x
                sta SID2BASE-21+$03,x
                lda #$09                   ;Testbit
mt_sfxexec_wavechg_sid2:
                sta mt_chnwave,x
                sta SID2BASE-21+$04,x
mt_sfxexec_done_sid2:
                rts
mt_sfxexec_framen_sid2:
                lda (mt_temp1),y
                bne mt_sfxexec_noend_sid2
mt_sfxexec_end_sid2:
                sta mt_chnsfx,x
                beq mt_sfxexec_wavechg_sid2
mt_sfxexec_noend_sid2:
                tay
                lda mt_freqtbllo-$80,y        ;Get frequency
                sta SID2BASE-21+$00,x
                lda mt_freqtblhi-$80,y
                sta SID2BASE-21+$01,x
                ldy mt_chnsfx,y
                lda (mt_temp1),y              ;Then take a look at the next
                beq mt_sfxexec_done_sid2           ;byte
                cmp #$82                      ;Is it a waveform or a note?
                bcs mt_sfxexec_done_sid2
                inc mt_chnsfx,x
                bcc mt_sfxexec_wavechg_sid2

              .ENDIF

        ;Wavetable command exec

              .IF (NOWAVECMD == 0)
mt_execwavecmd:
                and #$0f
                sta <mt_temp1
                lda mt_notetbl-2,y
                sta <mt_temp2
                ldy <mt_temp1
              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0) || (NOVIB == 0))
                cpy #$05
                bcs mt_execwavetick0
mt_execwavetickn:
              .IF (.DEFINED(mt_effectnum))
                sty mt_effectnum+1
              .ENDIF
                lda mt_effectjumptbl,y
                sta mt_effectjump+1
                ldy <mt_temp2
                jmp mt_setspeedparam
              .ENDIF
mt_execwavetick0:
                lda mt_tick0jumptbl,y
                sta mt_execwavetick0jump+1
                lda <mt_temp2
mt_execwavetick0jump:
                jsr mt_tick0_0
                jmp mt_done
              .ENDIF

              .IF (NOEFFECTS == 0)
              .IF (!.DEFINED(mt_tick0jumptbl))
mt_tick0jumptbl:
                .BYTE (mt_tick0_0 % 256)
                .BYTE (mt_tick0_12 % 256)
                .BYTE (mt_tick0_12 % 256)
                .BYTE (mt_tick0_34 % 256)
                .BYTE (mt_tick0_34 % 256)
                .BYTE (mt_tick0_5 % 256)
                .BYTE (mt_tick0_6 % 256)
                .BYTE (mt_tick0_7 % 256)
                .BYTE (mt_tick0_8 % 256)
                .BYTE (mt_tick0_9 % 256)
                .BYTE (mt_tick0_a % 256)
                .BYTE (mt_tick0_b % 256)
                .BYTE (mt_tick0_c % 256)
                .BYTE (mt_tick0_d % 256)
                .BYTE (mt_tick0_e % 256)
                .BYTE (mt_tick0_f % 256)
              .ENDIF
              .ENDIF

              .IF (NOEFFECTS == 0)
              .IF (!.DEFINED(mt_effectjumptbl))
              .IF ((NOTONEPORTA == 0) || (NOPORTAMENTO == 0) || (NOVIB == 0))
mt_effectjumptbl:
                .BYTE (mt_effect_0 % 256)
                .BYTE (mt_effect_12 % 256)
                .BYTE (mt_effect_12 % 256)
                .BYTE (mt_effect_3 % 256)
                .BYTE (mt_effect_4 % 256)
              .ENDIF
              .ENDIF
              .ENDIF

              .IF (!.DEFINED(mt_funktempotbl))
              .IF (NOFUNKTEMPO == 0)
mt_funktempotbl:
                .BYTE (8,5)
              .ENDIF
              .ENDIF

              .IF ((NOEFFECTS == 0) || (NOWAVEDELAY == 0) || (NOTRANS == 0) || (NOREPEAT == 0) || (FIXEDPARAMS == 0) || (BUFFEREDWRITES != 0) || (NOCALCULATEDSPEED == 0))

              ;Normal channel variables

mt_chnsongptr:
                .BYTE (0)
mt_chntrans:
                .BYTE (0)
mt_chnrepeat:
                .BYTE (0)
mt_chnpattptr:
                .BYTE (0)
mt_chnpackedrest:
                .BYTE (0)
mt_chnnewfx:
                .BYTE (0)
mt_chnnewparam:
                .BYTE (0)

                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)              


mt_chnfx:
                .BYTE (0)
mt_chnparam:
                .BYTE (0)
mt_chnnewnote:
                .BYTE (0)
mt_chnwaveptr:
                .BYTE (0)
mt_chnwave:
                .BYTE (0)
mt_chnpulseptr:
                .BYTE (0)
mt_chnpulsetime:
                .BYTE (0)

                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)

mt_chnsongnum:
                .BYTE (0)
mt_chnpattnum:
                .BYTE (0)
mt_chntempo:
                .BYTE (0)
mt_chncounter:
                .BYTE (0)
mt_chnnote:
                .BYTE (0)
mt_chninstr:
                .BYTE (1)
mt_chngate:
                .BYTE ($fe)

                .BYTE (1,0,0,0,0,1,$fe)
                .BYTE (2,0,0,0,0,1,$fe)
                .BYTE (3,0,0,0,0,1,$fe)
                .BYTE (4,0,0,0,0,1,$fe)
                .BYTE (5,0,0,0,0,1,$fe)

mt_chnvibtime:
                .BYTE (0)
mt_chnvibdelay:
                .BYTE (0)
mt_chnwavetime:
                .BYTE (0)
mt_chnfreqlo:
                .BYTE (0)
mt_chnfreqhi:
                .BYTE (0)
mt_chnpulselo:
                .BYTE (0)
mt_chnpulsehi:
                .BYTE (0)

                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)


              .IF ((BUFFEREDWRITES != 0) || (FIXEDPARAMS == 0) || (NOCALCULATEDSPEED == 0))
mt_chnad:
                .BYTE (0)
mt_chnsr:
                .BYTE (0)
mt_chnsfx:
                .BYTE (0)
mt_chnsfxlo:
                .BYTE (0)
mt_chnsfxhi:
                .BYTE (0)
mt_chngatetimer:
                .BYTE (0)
mt_chnlastnote:
                .BYTE (0)

                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)

              .ENDIF

              .ELSE

              ;Optimized channel variables

mt_chnsongptr:
                .BYTE (0)
mt_chnpattptr:
                .BYTE (0)
mt_chnpackedrest:
                .BYTE (0)
mt_chnnewnote:
                .BYTE (0)
mt_chnwaveptr:
                .BYTE (0)
mt_chnwave:
                .BYTE (0)
mt_chnpulseptr:
                .BYTE (0)

                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)

mt_chnpulsetime:
                .BYTE (0)
mt_chnpulselo:
                .BYTE (0)
mt_chnpulsehi:
                .BYTE (0)
mt_chnvibtime:
                .BYTE (0)
mt_chnvibdelay:
                .BYTE (0)
mt_chnfreqlo:
                .BYTE (0)
mt_chnfreqhi:
                .BYTE (0)

                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)
                .BYTE (0,0,0,0,0,0,0)

mt_chnsongnum:
                .BYTE (0)
mt_chnpattnum:
                .BYTE (0)
mt_chntempo:
                .BYTE (0)
mt_chncounter:
                .BYTE (0)
mt_chnnote:
                .BYTE (0)
mt_chninstr:
                .BYTE (1)
mt_chngate:
                .BYTE ($fe)

                .BYTE (1,0,0,0,0,1,$fe)
                .BYTE (2,0,0,0,0,1,$fe)
                .BYTE (3,0,0,0,0,1,$fe)
                .BYTE (4,0,0,0,0,1,$fe)
                .BYTE (5,0,0,0,0,1,$fe)

              .ENDIF

        ;Songdata & frequencytable will be inserted by the relocator here

