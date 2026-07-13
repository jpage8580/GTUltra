base             = 4096
zpbase           = 252
SIDBASE          = 54272
SID2BASE         = 54304
SOUNDSUPPORT     = 0
VOLSUPPORT       = 0
BUFFEREDWRITES   = 0
ZPGHOSTREGS      = 0
FIXEDPARAMS      = 0
SIMPLEPULSE      = 0
PULSEOPTIMIZATION = 0
REALTIMEOPTIMIZATION = 0
NOAUTHORINFO     = 0
ZPPLAYSID        = 0
NOEFFECTS        = 0
NOGATE           = 0
NOFILTER         = 0
NOFILTERMOD      = 0
NOPULSE          = 0
NOPULSEMOD       = 0
NOWAVEDELAY      = 1
NOWAVECMD        = 1
NOREPEAT         = 1
NOTRANS          = 0
NOPORTAMENTO     = 0
NOTONEPORTA      = 0
NOVIB            = 0
NOINSTRVIB       = 0
NOSETAD          = 1
NOSETSR          = 0
NOSETWAVE        = 1
NOSETWAVEPTR     = 1
NOSETPULSEPTR    = 1
NOSETFILTPTR     = 0
NOSETFILTCTRL    = 1
NOSETFILTCUTOFF  = 0
NOSETMASTERVOL   = 1
NOFUNKTEMPO      = 0
NOGLOBALTEMPO    = 0
NOCHANNELTEMPO   = 1
NOFIRSTWAVECMD   = 1
NOCALCULATEDSPEED = 1
NONORMALSPEED    = 0
NOZEROSPEED      = 0
NUMCHANNELS      = 6
NUMSONGS         = 1
FIRSTNOTE        = 0
FIRSTNOHRINSTR   = 12
FIRSTLEGATOINSTR = 13
NUMHRINSTR       = 11
NUMNOHRINSTR     = 1
NUMLEGATOINSTR   = 2
ADPARAM          = 255
SRPARAM          = 0
DEFAULTTEMPO     = 11
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

mt_freqtbllo:
                .BYTE ($17,$27,$39,$4b,$5f,$74,$8a,$a1,$ba,$d4,$f0,$0e,$2d,$4e,$71,$96)
                .BYTE ($be,$e8,$14,$43,$74,$a9,$e1,$1c,$5a,$9c,$e2,$2d,$7c,$cf,$28,$85)
                .BYTE ($e8,$52,$c1,$37,$b4,$39,$c5,$5a,$f7,$9e,$4f,$0a,$d1,$a3,$82,$6e)
                .BYTE ($68,$71,$8a,$b3,$ee,$3c,$9e,$15,$a2,$46,$04,$dc,$d0,$e2,$14,$67)
                .BYTE ($dd,$79,$3c,$29,$44,$8d,$08,$b8,$a1,$c5,$28,$cd,$ba,$f1,$78,$53)
                .BYTE ($87,$1a,$10,$71,$42,$89,$4f,$9b,$74,$e2,$f0,$a6,$0e,$33,$20,$ff)
mt_freqtblhi:
                .BYTE ($01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$02,$02,$02,$02,$02)
                .BYTE ($02,$02,$03,$03,$03,$03,$03,$04,$04,$04,$04,$05,$05,$05,$06,$06)
                .BYTE ($06,$07,$07,$08,$08,$09,$09,$0a,$0a,$0b,$0c,$0d,$0d,$0e,$0f,$10)
                .BYTE ($11,$12,$13,$14,$15,$17,$18,$1a,$1b,$1d,$1f,$20,$22,$24,$27,$29)
                .BYTE ($2b,$2e,$31,$34,$37,$3a,$3e,$41,$45,$49,$4e,$52,$57,$5c,$62,$68)
                .BYTE ($6e,$75,$7c,$83,$8b,$93,$9c,$a5,$af,$b9,$c4,$d0,$dd,$ea,$f8,$ff)
mt_songtbllo:
                .BYTE (mt_song0 % 256)
                .BYTE (mt_song1 % 256)
                .BYTE (mt_song2 % 256)
                .BYTE (mt_song3 % 256)
                .BYTE (mt_song4 % 256)
                .BYTE (mt_song5 % 256)
mt_songtblhi:
                .BYTE (mt_song0 / 256)
                .BYTE (mt_song1 / 256)
                .BYTE (mt_song2 / 256)
                .BYTE (mt_song3 / 256)
                .BYTE (mt_song4 / 256)
                .BYTE (mt_song5 / 256)
mt_patttbllo:
                .BYTE (mt_patt0 % 256)
                .BYTE (mt_patt1 % 256)
                .BYTE (mt_patt2 % 256)
                .BYTE (mt_patt3 % 256)
                .BYTE (mt_patt4 % 256)
                .BYTE (mt_patt5 % 256)
                .BYTE (mt_patt6 % 256)
                .BYTE (mt_patt7 % 256)
                .BYTE (mt_patt8 % 256)
                .BYTE (mt_patt9 % 256)
                .BYTE (mt_patt10 % 256)
                .BYTE (mt_patt11 % 256)
                .BYTE (mt_patt12 % 256)
                .BYTE (mt_patt13 % 256)
                .BYTE (mt_patt14 % 256)
                .BYTE (mt_patt15 % 256)
                .BYTE (mt_patt16 % 256)
                .BYTE (mt_patt17 % 256)
                .BYTE (mt_patt18 % 256)
                .BYTE (mt_patt19 % 256)
                .BYTE (mt_patt20 % 256)
                .BYTE (mt_patt21 % 256)
                .BYTE (mt_patt22 % 256)
                .BYTE (mt_patt23 % 256)
                .BYTE (mt_patt24 % 256)
                .BYTE (mt_patt25 % 256)
                .BYTE (mt_patt26 % 256)
                .BYTE (mt_patt27 % 256)
                .BYTE (mt_patt28 % 256)
                .BYTE (mt_patt29 % 256)
                .BYTE (mt_patt30 % 256)
                .BYTE (mt_patt31 % 256)
                .BYTE (mt_patt32 % 256)
                .BYTE (mt_patt33 % 256)
                .BYTE (mt_patt34 % 256)
                .BYTE (mt_patt35 % 256)
                .BYTE (mt_patt36 % 256)
                .BYTE (mt_patt37 % 256)
                .BYTE (mt_patt38 % 256)
                .BYTE (mt_patt39 % 256)
                .BYTE (mt_patt40 % 256)
                .BYTE (mt_patt41 % 256)
                .BYTE (mt_patt42 % 256)
                .BYTE (mt_patt43 % 256)
                .BYTE (mt_patt44 % 256)
                .BYTE (mt_patt45 % 256)
                .BYTE (mt_patt46 % 256)
                .BYTE (mt_patt47 % 256)
                .BYTE (mt_patt48 % 256)
                .BYTE (mt_patt49 % 256)
                .BYTE (mt_patt50 % 256)
                .BYTE (mt_patt51 % 256)
mt_patttblhi:
                .BYTE (mt_patt0 / 256)
                .BYTE (mt_patt1 / 256)
                .BYTE (mt_patt2 / 256)
                .BYTE (mt_patt3 / 256)
                .BYTE (mt_patt4 / 256)
                .BYTE (mt_patt5 / 256)
                .BYTE (mt_patt6 / 256)
                .BYTE (mt_patt7 / 256)
                .BYTE (mt_patt8 / 256)
                .BYTE (mt_patt9 / 256)
                .BYTE (mt_patt10 / 256)
                .BYTE (mt_patt11 / 256)
                .BYTE (mt_patt12 / 256)
                .BYTE (mt_patt13 / 256)
                .BYTE (mt_patt14 / 256)
                .BYTE (mt_patt15 / 256)
                .BYTE (mt_patt16 / 256)
                .BYTE (mt_patt17 / 256)
                .BYTE (mt_patt18 / 256)
                .BYTE (mt_patt19 / 256)
                .BYTE (mt_patt20 / 256)
                .BYTE (mt_patt21 / 256)
                .BYTE (mt_patt22 / 256)
                .BYTE (mt_patt23 / 256)
                .BYTE (mt_patt24 / 256)
                .BYTE (mt_patt25 / 256)
                .BYTE (mt_patt26 / 256)
                .BYTE (mt_patt27 / 256)
                .BYTE (mt_patt28 / 256)
                .BYTE (mt_patt29 / 256)
                .BYTE (mt_patt30 / 256)
                .BYTE (mt_patt31 / 256)
                .BYTE (mt_patt32 / 256)
                .BYTE (mt_patt33 / 256)
                .BYTE (mt_patt34 / 256)
                .BYTE (mt_patt35 / 256)
                .BYTE (mt_patt36 / 256)
                .BYTE (mt_patt37 / 256)
                .BYTE (mt_patt38 / 256)
                .BYTE (mt_patt39 / 256)
                .BYTE (mt_patt40 / 256)
                .BYTE (mt_patt41 / 256)
                .BYTE (mt_patt42 / 256)
                .BYTE (mt_patt43 / 256)
                .BYTE (mt_patt44 / 256)
                .BYTE (mt_patt45 / 256)
                .BYTE (mt_patt46 / 256)
                .BYTE (mt_patt47 / 256)
                .BYTE (mt_patt48 / 256)
                .BYTE (mt_patt49 / 256)
                .BYTE (mt_patt50 / 256)
                .BYTE (mt_patt51 / 256)
mt_insad:
                .BYTE ($40,$20,$0f,$0f,$03,$04,$0e,$0e,$02,$02,$03,$00)
mt_inssr:
                .BYTE ($a7,$a8,$f9,$f6,$e4,$84,$e8,$e8,$a8,$d8,$d6,$a2)
mt_inswaveptr:
                .BYTE ($36,$01,$06,$10,$1e,$04,$28,$19,$26,$04,$31,$23)
mt_inspulseptr:
                .BYTE ($1e,$01,$06,$06,$00,$09,$06,$06,$13,$17,$1b,$0f)
mt_insfiltptr:
                .BYTE ($0f,$06,$00,$0b,$0b,$00,$17,$01,$00,$00,$00,$00)
mt_insvibparam:
                .BYTE ($00,$02,$00,$00,$00,$01,$00,$00,$00,$00,$00,$00)
mt_insvibdelay:
                .BYTE ($02,$02,$00,$00,$00,$02,$00,$00,$00,$00,$00,$00)
mt_insgatetimer:
                .BYTE ($01,$02,$02,$02,$02,$02,$02,$02,$02,$02,$02,$01)
mt_insfirstwave:
                .BYTE ($09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09)
mt_wavetbl:
                .BYTE ($81)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($ff)
                .BYTE ($09)
                .BYTE ($81)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($10)
                .BYTE ($10)
                .BYTE ($10)
                .BYTE ($ff)
                .BYTE ($09)
                .BYTE ($81)
                .BYTE ($81)
                .BYTE ($81)
                .BYTE ($81)
                .BYTE ($80)
                .BYTE ($50)
                .BYTE ($10)
                .BYTE ($ff)
                .BYTE ($41)
                .BYTE ($81)
                .BYTE ($41)
                .BYTE ($81)
                .BYTE ($ff)
                .BYTE ($09)
                .BYTE ($81)
                .BYTE ($13)
                .BYTE ($80)
                .BYTE ($ff)
                .BYTE ($81)
                .BYTE ($80)
                .BYTE ($ff)
                .BYTE ($21)
                .BYTE ($ff)
                .BYTE ($09)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($40)
                .BYTE ($ff)
                .BYTE ($09)
                .BYTE ($41)
                .BYTE ($41)
                .BYTE ($31)
                .BYTE ($ff)
                .BYTE ($81)
                .BYTE ($61)
                .BYTE ($61)
                .BYTE ($61)
                .BYTE ($61)
                .BYTE ($ff)
mt_notetbl:
                .BYTE ($51)
                .BYTE ($8c)
                .BYTE ($80)
                .BYTE ($80)
                .BYTE ($00)
                .BYTE ($80)
                .BYTE ($5f)
                .BYTE ($20)
                .BYTE ($2f)
                .BYTE ($23)
                .BYTE ($13)
                .BYTE ($1a)
                .BYTE ($1a)
                .BYTE ($80)
                .BYTE ($00)
                .BYTE ($80)
                .BYTE ($5f)
                .BYTE ($29)
                .BYTE ($22)
                .BYTE ($5f)
                .BYTE ($5d)
                .BYTE ($27)
                .BYTE ($20)
                .BYTE ($14)
                .BYTE ($2f)
                .BYTE ($51)
                .BYTE ($40)
                .BYTE ($51)
                .BYTE ($19)
                .BYTE ($80)
                .BYTE ($2f)
                .BYTE ($a2)
                .BYTE ($1a)
                .BYTE ($00)
                .BYTE ($5f)
                .BYTE ($5f)
                .BYTE ($00)
                .BYTE ($80)
                .BYTE ($00)
                .BYTE ($80)
                .BYTE ($2f)
                .BYTE ($20)
                .BYTE ($2f)
                .BYTE ($23)
                .BYTE ($13)
                .BYTE ($1a)
                .BYTE ($1a)
                .BYTE ($00)
                .BYTE ($80)
                .BYTE ($8c)
                .BYTE ($81)
                .BYTE ($80)
                .BYTE ($00)
                .BYTE ($51)
                .BYTE ($20)
                .BYTE ($29)
                .BYTE ($20)
                .BYTE ($80)
                .BYTE ($00)
mt_pulsetimetbl:
                .BYTE ($da)
                .BYTE ($01)
                .BYTE ($da)
                .BYTE ($4a)
                .BYTE ($ff)
                .BYTE ($88)
                .BYTE ($88)
                .BYTE ($ff)
                .BYTE ($83)
                .BYTE ($1f)
                .BYTE ($81)
                .BYTE ($10)
                .BYTE ($12)
                .BYTE ($ff)
                .BYTE ($80)
                .BYTE ($30)
                .BYTE ($30)
                .BYTE ($ff)
                .BYTE ($8d)
                .BYTE ($03)
                .BYTE ($02)
                .BYTE ($ff)
                .BYTE ($d8)
                .BYTE ($2f)
                .BYTE ($0f)
                .BYTE ($ff)
                .BYTE ($da)
                .BYTE ($02)
                .BYTE ($ff)
                .BYTE ($88)
                .BYTE ($00)
                .BYTE ($23)
                .BYTE ($ff)
mt_pulsespdtbl:
                .BYTE ($10)
                .BYTE ($40)
                .BYTE ($0a)
                .BYTE ($0f)
                .BYTE ($04)
                .BYTE ($88)
                .BYTE ($88)
                .BYTE ($00)
                .BYTE ($00)
                .BYTE ($e4)
                .BYTE ($02)
                .BYTE ($7f)
                .BYTE ($80)
                .BYTE ($0c)
                .BYTE ($00)
                .BYTE ($40)
                .BYTE ($c0)
                .BYTE ($10)
                .BYTE ($d0)
                .BYTE ($20)
                .BYTE ($02)
                .BYTE ($14)
                .BYTE ($00)
                .BYTE ($2d)
                .BYTE ($43)
                .BYTE ($18)
                .BYTE ($0d)
                .BYTE ($30)
                .BYTE ($00)
                .BYTE ($00)
                .BYTE ($23)
                .BYTE ($f1)
                .BYTE ($1e)
mt_filttimetbl:
                .BYTE ($88)
                .BYTE ($02)
                .BYTE ($b0)
                .BYTE ($00)
                .BYTE ($ff)
                .BYTE ($98)
                .BYTE ($02)
                .BYTE ($01)
                .BYTE ($00)
                .BYTE ($ff)
                .BYTE ($a0)
                .BYTE ($02)
                .BYTE ($02)
                .BYTE ($ff)
                .BYTE ($88)
                .BYTE ($00)
                .BYTE ($02)
                .BYTE ($04)
                .BYTE ($00)
                .BYTE ($01)
                .BYTE ($00)
                .BYTE ($ff)
                .BYTE ($98)
                .BYTE ($00)
                .BYTE ($01)
                .BYTE ($00)
                .BYTE ($01)
                .BYTE ($00)
                .BYTE ($ff)
mt_filtspdtbl:
                .BYTE ($f5)
                .BYTE ($d2)
                .BYTE ($f5)
                .BYTE ($32)
                .BYTE ($00)
                .BYTE ($f7)
                .BYTE ($81)
                .BYTE ($15)
                .BYTE ($10)
                .BYTE ($00)
                .BYTE ($f5)
                .BYTE ($02)
                .BYTE ($02)
                .BYTE ($00)
                .BYTE ($f7)
                .BYTE ($18)
                .BYTE ($12)
                .BYTE ($12)
                .BYTE ($0a)
                .BYTE ($12)
                .BYTE ($04)
                .BYTE ($00)
                .BYTE ($f2)
                .BYTE ($32)
                .BYTE ($03)
                .BYTE ($03)
                .BYTE ($03)
                .BYTE ($02)
                .BYTE ($00)
                .BYTE ($00)
mt_speedlefttbl:
                .BYTE ($03)
                .BYTE ($02)
                .BYTE ($01)
                .BYTE ($06)
                .BYTE ($04)
                .BYTE ($00)
                .BYTE ($04)
                .BYTE ($57)
                .BYTE ($00)
mt_speedrighttbl:
                .BYTE ($20)
                .BYTE ($2a)
                .BYTE ($00)
                .BYTE ($30)
                .BYTE ($a0)
                .BYTE ($30)
                .BYTE ($07)
                .BYTE ($57)
mt_song0:
                .BYTE ($f1,$00,$00,$00,$00,$00,$00,$00,$00,$00,$09,$0b,$0c,$0e,$11,$0e)
                .BYTE ($12,$17,$1d,$1f,$1d,$21,$1d,$1f,$1d,$23,$1d,$1f,$1d,$21,$1d,$1f)
                .BYTE ($1d,$21,$02,$0a,$02,$0d,$02,$0a,$02,$0d,$1d,$1f,$1d,$21,$1d,$1f)
                .BYTE ($1d,$21,$03,$03,$03,$03,$33,$33,$33,$33,$33,$ff,$00)
mt_song1:
                .BYTE ($f1,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$0f,$0f,$0f)
                .BYTE ($13,$18,$01,$01,$01,$01,$01,$01,$01,$24,$26,$26,$26,$2d,$26,$26)
                .BYTE ($26,$2d,$1d,$1f,$1d,$21,$1d,$1f,$1d,$21,$31,$31,$31,$31,$31,$31)
                .BYTE ($31,$31,$01,$01,$04,$04,$33,$33,$33,$33,$33,$ff,$00)
mt_song2:
                .BYTE ($f1,$02,$02,$02,$02,$02,$02,$02,$02,$02,$0a,$02,$0d,$02,$0a,$02)
                .BYTE ($14,$19,$04,$04,$04,$04,$04,$04,$04,$07,$27,$27,$27,$2e,$27,$27)
                .BYTE ($27,$2e,$04,$04,$04,$04,$30,$04,$04,$04,$32,$32,$32,$32,$32,$32)
                .BYTE ($32,$32,$02,$0a,$02,$0d,$33,$33,$33,$33,$33,$ff,$00)
mt_song3:
                .BYTE ($f1,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03)
                .BYTE ($15,$1a,$1e,$1e,$1e,$1e,$1e,$1e,$1e,$03,$28,$1e,$1e,$1e,$28,$1e)
                .BYTE ($1e,$1e,$1e,$1e,$1e,$1e,$1e,$1e,$1e,$1e,$28,$1e,$1e,$1e,$28,$1e)
                .BYTE ($1e,$1e,$28,$1e,$1e,$1e,$33,$33,$33,$33,$33,$ff,$00)
mt_song4:
                .BYTE ($f1,$04,$04,$04,$04,$04,$04,$04,$04,$08,$08,$08,$08,$10,$10,$10)
                .BYTE ($16,$1b,$04,$04,$20,$20,$20,$20,$22,$25,$29,$2b,$29,$2b,$29,$2b)
                .BYTE ($29,$2b,$29,$2b,$29,$2b,$29,$2b,$29,$2b,$29,$2b,$29,$2b,$29,$2b)
                .BYTE ($29,$2b,$29,$2b,$04,$04,$33,$33,$33,$33,$33,$ff,$00)
mt_song5:
                .BYTE ($f1,$04,$04,$04,$04,$05,$06,$05,$07,$02,$0a,$02,$0d,$02,$0a,$02)
                .BYTE ($0a,$1c,$04,$04,$04,$04,$02,$0a,$02,$0d,$2a,$2c,$2a,$2f,$2a,$2c)
                .BYTE ($2a,$2f,$02,$0a,$02,$0d,$02,$0a,$02,$0d,$02,$0a,$02,$0d,$02,$0a)
                .BYTE ($02,$0d,$02,$0a,$02,$0d,$33,$33,$33,$33,$33,$ff,$00)
mt_patt0:
                .BYTE ($01,$40,$7a,$bd,$5c,$02,$50,$e8,$5c,$03,$50,$fe,$5c,$02,$50,$ee)
                .BYTE ($5c,$03,$50,$fe,$5c,$04,$50,$fe,$5c,$03,$50,$fe,$00)
mt_patt1:
                .BYTE ($4c,$20,$be,$50,$fe,$5c,$10,$50,$5c,$04,$50,$fe,$5c,$02,$50,$fc)
                .BYTE ($5c,$10,$50,$5c,$04,$50,$fe,$5c,$03,$50,$f4,$5c,$10,$50,$5c,$04)
                .BYTE ($50,$fc,$5c,$10,$50,$5c,$02,$50,$f8,$5c,$10,$50,$5c,$0f,$5c,$03)
                .BYTE ($5c,$0a,$5c,$05,$5c,$07,$5c,$08,$00)
mt_patt2:
                .BYTE ($06,$40,$78,$bd,$84,$bd,$86,$bd,$86,$bd,$7a,$bd,$84,$bd,$56,$a3)
                .BYTE ($50,$86,$fd,$7a,$bd,$84,$bd,$78,$bd,$78,$bd,$84,$bd,$84,$bd,$78)
                .BYTE ($bd,$7a,$bd,$84,$bd,$86,$bd,$86,$bd,$7a,$bd,$84,$bd,$56,$a3,$50)
                .BYTE ($86,$fd,$7a,$bd,$84,$bd,$78,$bd,$78,$bd,$84,$bd,$84,$bd,$78,$bd)
                .BYTE ($00)
mt_patt3:
                .BYTE ($05,$4e,$07,$7a,$50,$4c,$f8,$62,$50,$bd,$5c,$20,$4c,$0a,$62,$5c)
                .BYTE ($10,$50,$5c,$01,$46,$aa,$62,$50,$4c,$20,$62,$56,$aa,$5c,$10,$50)
                .BYTE ($46,$fa,$62,$5c,$e2,$50,$5c,$10,$40,$62,$5c,$02,$56,$66,$5c,$20)
                .BYTE ($42,$02,$62,$52,$06,$54,$01,$54,$02,$56,$a4,$50,$fe,$4e,$07,$62)
                .BYTE ($50,$4c,$f8,$62,$50,$bd,$5c,$20,$4c,$0a,$62,$5c,$10,$50,$5c,$01)
                .BYTE ($46,$aa,$62,$50,$4c,$20,$62,$56,$aa,$5c,$10,$50,$46,$fa,$62,$5c)
                .BYTE ($e2,$50,$5c,$10,$40,$62,$5c,$02,$56,$6f,$5c,$20,$4c,$50,$62,$50)
                .BYTE ($46,$aa,$7a,$5c,$55,$5c,$02,$56,$40,$56,$a5,$5c,$00,$00)
mt_patt4:
                .BYTE ($40,$be,$c1,$00)
mt_patt5:
                .BYTE ($06,$40,$7a,$bd,$46,$22,$84,$50,$86,$bd,$46,$22,$86,$50,$7a,$bd)
                .BYTE ($46,$22,$84,$50,$56,$a3,$50,$86,$fd,$46,$22,$7a,$50,$84,$bd,$46)
                .BYTE ($22,$78,$50,$78,$bd,$46,$22,$84,$50,$84,$bd,$46,$22,$78,$50,$7a)
                .BYTE ($bd,$46,$22,$84,$50,$46,$22,$86,$50,$86,$bd,$46,$22,$7a,$50,$84)
                .BYTE ($bd,$56,$a3,$50,$86,$fd,$7a,$bd,$46,$22,$84,$50,$78,$bd,$46,$22)
                .BYTE ($78,$50,$84,$bd,$84,$bd,$78,$bd,$00)
mt_patt6:
                .BYTE ($06,$40,$7a,$bd,$46,$22,$84,$50,$86,$bd,$46,$22,$86,$50,$7a,$bd)
                .BYTE ($46,$22,$84,$50,$56,$a3,$50,$86,$fd,$46,$22,$7a,$50,$84,$bd,$46)
                .BYTE ($22,$78,$50,$78,$bd,$46,$22,$84,$50,$84,$bd,$46,$22,$78,$50,$7a)
                .BYTE ($bd,$46,$22,$84,$50,$46,$11,$86,$50,$7a,$bd,$46,$22,$7a,$50,$7a)
                .BYTE ($bd,$90,$bd,$46,$22,$84,$50,$84,$bd,$90,$bd,$84,$bd,$90,$bd,$7a)
                .BYTE ($fd,$92,$bd,$86,$bd,$00)
mt_patt7:
                .BYTE ($06,$40,$78,$bd,$46,$22,$84,$50,$86,$bd,$46,$22,$86,$50,$7a,$bd)
                .BYTE ($46,$22,$84,$50,$56,$a3,$50,$86,$fd,$46,$22,$7a,$50,$84,$bd,$46)
                .BYTE ($22,$78,$50,$78,$bd,$46,$22,$84,$50,$84,$bd,$46,$22,$78,$50,$7a)
                .BYTE ($bd,$56,$22,$50,$92,$54,$04,$54,$05,$bd,$46,$22,$86,$50,$84,$bd)
                .BYTE ($90,$54,$05,$54,$04,$54,$05,$40,$84,$bd,$84,$bd,$46,$22,$84,$50)
                .BYTE ($90,$bd,$92,$54,$05,$fa,$00)
mt_patt8:
                .BYTE ($4c,$02,$be,$50,$fe,$5c,$20,$50,$5c,$04,$50,$fe,$5c,$12,$50,$bd)
                .BYTE ($5c,$02,$50,$bd,$5c,$20,$50,$5c,$04,$50,$5c,$0a,$50,$5c,$03,$50)
                .BYTE ($5c,$20,$50,$5c,$02,$50,$f8,$5c,$30,$50,$5c,$04,$50,$5c,$10,$50)
                .BYTE ($fe,$5c,$20,$50,$5c,$02,$50,$5c,$a0,$50,$fe,$5c,$02,$50,$fc,$5c)
                .BYTE ($0f,$5c,$03,$5c,$0a,$5c,$05,$5c,$07,$5c,$08,$00)
mt_patt9:
                .BYTE ($50,$bd,$5c,$03,$50,$e8,$5c,$04,$50,$ea,$5c,$05,$50,$fe,$5c,$04)
                .BYTE ($50,$fe,$5c,$06,$50,$fe,$00)
mt_patt10:
                .BYTE ($06,$40,$7a,$bd,$84,$bd,$86,$bd,$86,$bd,$7a,$bd,$84,$bd,$56,$a3)
                .BYTE ($50,$86,$fd,$7a,$bd,$84,$bd,$78,$bd,$78,$bd,$84,$bd,$84,$bd,$78)
                .BYTE ($bd,$7a,$fd,$86,$bd,$7a,$bd,$7a,$bd,$7a,$bd,$90,$bd,$84,$bd,$84)
                .BYTE ($bd,$90,$bd,$84,$bd,$90,$bd,$7a,$fd,$92,$bd,$86,$bd,$00)
mt_patt11:
                .BYTE ($50,$bd,$5c,$05,$50,$e8,$5c,$06,$50,$ea,$5c,$06,$50,$fe,$5c,$07)
                .BYTE ($50,$fe,$5c,$08,$50,$fe,$00)
mt_patt12:
                .BYTE ($50,$bd,$5c,$08,$50,$e8,$5c,$09,$50,$ea,$5c,$0a,$50,$fe,$5c,$0b)
                .BYTE ($50,$fe,$5c,$0c,$50,$fe,$00)
mt_patt13:
                .BYTE ($06,$40,$84,$bd,$84,$bd,$86,$bd,$86,$bd,$7a,$bd,$84,$bd,$56,$a3)
                .BYTE ($50,$86,$fd,$7a,$bd,$84,$bd,$78,$bd,$78,$bd,$84,$bd,$84,$bd,$78)
                .BYTE ($bd,$7a,$fd,$92,$54,$04,$fe,$40,$86,$bd,$84,$bd,$90,$54,$04,$fe)
                .BYTE ($40,$84,$bd,$84,$bd,$84,$bd,$90,$bd,$92,$54,$05,$fa,$00)
mt_patt14:
                .BYTE ($01,$40,$7a,$bd,$5c,$05,$50,$fc,$5c,$06,$50,$fc,$5c,$07,$50,$fd)
                .BYTE ($5c,$08,$50,$fd,$5c,$09,$50,$fe,$5c,$0a,$50,$bd,$5c,$0b,$50,$bd)
                .BYTE ($5c,$0e,$50,$f8,$5c,$0b,$50,$bd,$5c,$0a,$50,$bd,$5c,$09,$50,$5c)
                .BYTE ($08,$50,$5c,$07,$50,$7a,$f9,$00)
mt_patt15:
                .BYTE ($50,$c1,$00)
mt_patt16:
                .BYTE ($03,$40,$7a,$f5,$7a,$ed,$7a,$f5,$7a,$f5,$7a,$f9,$00)
mt_patt17:
                .BYTE ($01,$40,$7a,$bd,$5c,$09,$50,$f6,$5c,$0a,$50,$fd,$5c,$0b,$50,$fe)
                .BYTE ($5c,$0d,$50,$bd,$5c,$0e,$50,$bd,$5c,$0f,$50,$5c,$10,$50,$bd,$5c)
                .BYTE ($14,$50,$fb,$5c,$15,$5c,$16,$5c,$17,$5c,$18,$5c,$19,$5c,$1a,$5c)
                .BYTE ($20,$5c,$23,$5c,$2a,$5c,$37,$5c,$30,$5c,$3a,$5c,$20,$02,$40,$78)
                .BYTE ($5c,$02,$5c,$0a,$5c,$09,$5c,$08,$5c,$07,$5c,$06,$5c,$05,$5c,$04)
                .BYTE ($5c,$03,$00)
mt_patt18:
                .BYTE ($01,$40,$7a,$bd,$5c,$02,$5c,$03,$5c,$04,$5c,$05,$5c,$06,$5c,$07)
                .BYTE ($5c,$08,$5c,$09,$5c,$0a,$5c,$0b,$5c,$0c,$5c,$0d,$5c,$0e,$5c,$0f)
                .BYTE ($5c,$10,$5c,$14,$5c,$17,$5c,$18,$5c,$19,$5c,$1a,$5c,$20,$5c,$30)
                .BYTE ($5c,$29,$5c,$24,$5c,$23,$5c,$22,$5c,$21,$5c,$1d,$5c,$1b,$5c,$1c)
                .BYTE ($5c,$1a,$50,$f6,$5c,$35,$50,$fe,$5c,$20,$50,$5c,$20,$50,$5c,$20)
                .BYTE ($50,$5c,$20,$50,$02,$7a,$fd,$03,$4c,$05,$86,$50,$fe,$00)
mt_patt19:
                .BYTE ($50,$e1,$5c,$02,$fc,$5c,$03,$5c,$04,$5c,$05,$5c,$06,$5c,$07,$5c)
                .BYTE ($08,$5c,$07,$5c,$06,$5c,$05,$5c,$04,$5c,$03,$5c,$02,$fe,$50,$fc)
                .BYTE ($08,$4a,$17,$7a,$5c,$31,$4c,$33,$be,$5c,$4a,$4c,$5a,$bf,$5c,$53)
                .BYTE ($5c,$13,$bd,$00)
mt_patt20:
                .BYTE ($06,$40,$7a,$bd,$84,$bd,$86,$bd,$86,$bd,$7a,$bd,$84,$bd,$56,$a3)
                .BYTE ($50,$86,$fd,$7a,$bd,$84,$bd,$78,$bd,$78,$bd,$84,$bd,$84,$bd,$78)
                .BYTE ($bd,$7a,$bd,$84,$bd,$86,$bd,$7a,$bd,$7a,$bd,$7a,$bd,$90,$bd,$84)
                .BYTE ($bd,$84,$bd,$90,$bd,$84,$bd,$90,$bd,$08,$4a,$17,$7a,$5c,$31,$4c)
                .BYTE ($33,$be,$5c,$4a,$4c,$5a,$bf,$5c,$53,$5c,$13,$bd,$00)
mt_patt21:
                .BYTE ($05,$4e,$07,$7a,$50,$4c,$f8,$62,$50,$bd,$5c,$20,$4c,$0a,$62,$5c)
                .BYTE ($10,$50,$5c,$01,$46,$aa,$62,$50,$4c,$20,$62,$56,$aa,$5c,$10,$50)
                .BYTE ($46,$fa,$62,$5c,$e2,$50,$5c,$10,$40,$62,$5c,$02,$56,$66,$5c,$20)
                .BYTE ($42,$02,$62,$52,$06,$54,$01,$54,$02,$56,$a4,$50,$fe,$4c,$e2,$62)
                .BYTE ($bd,$62,$fd,$62,$bd,$5c,$e3,$bd,$62,$5c,$a3,$62,$5c,$63,$fe,$62)
                .BYTE ($bd,$50,$5c,$10,$40,$62,$5c,$02,$50,$5e,$07,$4c,$50,$62,$50,$46)
                .BYTE ($aa,$7a,$5c,$55,$5c,$02,$56,$40,$56,$a5,$5c,$00,$00)
mt_patt22:
                .BYTE ($03,$4e,$07,$7a,$50,$fe,$7a,$f9,$7a,$fd,$7a,$f9,$7a,$f9,$7a,$ed)
                .BYTE ($5e,$08,$50,$fe,$7a,$5e,$07,$40,$be,$bd,$4e,$07,$7a,$50,$be,$bd)
                .BYTE ($00)
mt_patt23:
                .BYTE ($50,$fd,$00)
mt_patt24:
                .BYTE ($50,$fd,$00)
mt_patt25:
                .BYTE ($5c,$45,$be,$fe,$00)
mt_patt26:
                .BYTE ($50,$fd,$00)
mt_patt27:
                .BYTE ($03,$40,$84,$fd,$00)
mt_patt28:
                .BYTE ($50,$fd,$00)
mt_patt29:
                .BYTE ($01,$4e,$07,$7a,$50,$fe,$02,$86,$bd,$56,$00,$50,$7a,$bd,$84,$bd)
                .BYTE ($78,$bd,$86,$fd,$7a,$bd,$84,$fd,$01,$78,$f9,$7a,$bd,$be,$bd,$02)
                .BYTE ($86,$bd,$56,$00,$50,$01,$7a,$bd,$02,$84,$fd,$86,$fd,$01,$6e,$bd)
                .BYTE ($02,$84,$fd,$01,$6c,$fd,$02,$78,$fd,$00)
mt_patt30:
                .BYTE ($05,$4e,$07,$62,$50,$4c,$f8,$62,$50,$bd,$5c,$20,$4c,$0a,$62,$5c)
                .BYTE ($10,$08,$40,$90,$be,$56,$aa,$50,$05,$4c,$20,$62,$56,$aa,$5c,$10)
                .BYTE ($50,$46,$fa,$62,$5c,$e2,$50,$5c,$10,$40,$62,$5c,$02,$56,$66,$5c)
                .BYTE ($20,$08,$40,$84,$be,$56,$aa,$50,$05,$46,$a4,$62,$50,$fe,$4e,$07)
                .BYTE ($62,$50,$4c,$f8,$62,$50,$bd,$5c,$20,$4c,$0a,$62,$5c,$10,$08,$40)
                .BYTE ($90,$be,$fe,$05,$4c,$20,$62,$56,$aa,$5c,$10,$50,$46,$fa,$62,$5c)
                .BYTE ($e2,$50,$5c,$10,$40,$62,$5c,$02,$56,$6f,$5c,$20,$08,$40,$90,$be)
                .BYTE ($fe,$05,$62,$5c,$55,$5c,$02,$56,$40,$00)
mt_patt31:
                .BYTE ($01,$4e,$07,$7a,$50,$fe,$02,$86,$bd,$56,$00,$50,$7a,$bd,$84,$bd)
                .BYTE ($6c,$bd,$86,$fd,$7a,$bd,$84,$fd,$01,$78,$f9,$7a,$fd,$02,$86,$fd)
                .BYTE ($01,$7a,$bd,$02,$43,$00,$7a,$50,$44,$04,$84,$fd,$01,$40,$78,$fd)
                .BYTE ($02,$84,$fd,$01,$7a,$fd,$02,$86,$fd,$00)
mt_patt32:
                .BYTE ($5e,$07,$50,$fe,$0c,$6e,$f9,$6e,$bd,$6e,$fc,$be,$6e,$f9,$6e,$be)
                .BYTE ($fe,$5e,$07,$50,$fe,$6e,$f9,$6e,$bd,$6e,$fb,$75,$f9,$6e,$fd,$00)
mt_patt33:
                .BYTE ($01,$4e,$07,$7a,$50,$fe,$02,$86,$bd,$56,$00,$50,$7a,$bd,$84,$bd)
                .BYTE ($6c,$bd,$86,$fd,$7a,$bd,$84,$fd,$01,$78,$f9,$7a,$fd,$02,$86,$fd)
                .BYTE ($01,$7a,$bd,$02,$43,$00,$7a,$50,$84,$54,$04,$fe,$01,$40,$78,$fd)
                .BYTE ($02,$84,$fd,$86,$54,$05,$fa,$00)
mt_patt34:
                .BYTE ($07,$4e,$07,$78,$50,$fe,$0c,$6e,$fd,$03,$78,$fd,$0c,$6e,$bd,$6e)
                .BYTE ($bd,$07,$78,$fe,$be,$0c,$6e,$fd,$03,$78,$fd,$0c,$6e,$be,$fe,$07)
                .BYTE ($4e,$07,$78,$50,$fe,$0c,$6e,$fd,$03,$78,$fd,$0c,$6e,$bd,$6e,$bd)
                .BYTE ($03,$78,$fd,$0c,$75,$fd,$03,$78,$fd,$0c,$6e,$fd,$00)
mt_patt35:
                .BYTE ($01,$40,$7a,$bd,$5c,$02,$5c,$03,$5c,$04,$5c,$05,$5c,$06,$5c,$07)
                .BYTE ($5c,$08,$5c,$09,$5c,$0a,$5c,$0b,$5c,$0c,$5c,$0d,$5c,$0e,$5c,$0f)
                .BYTE ($5c,$10,$5c,$14,$5c,$17,$5c,$18,$5c,$19,$5c,$1a,$5c,$20,$5c,$30)
                .BYTE ($5c,$29,$5c,$24,$5c,$23,$5c,$22,$5c,$21,$5c,$1d,$5c,$1b,$5c,$1c)
                .BYTE ($40,$be,$e9,$02,$7a,$fd,$03,$4c,$05,$86,$50,$fe,$00)
mt_patt36:
                .BYTE ($50,$e1,$05,$4c,$02,$86,$5c,$a0,$5c,$02,$5c,$a0,$5c,$02,$5c,$a0)
                .BYTE ($5c,$04,$5c,$a0,$5c,$06,$5c,$a0,$5c,$08,$5c,$b0,$5c,$06,$5c,$b0)
                .BYTE ($5c,$04,$5c,$b0,$5c,$02,$5c,$b0,$5c,$02,$5c,$c0,$5c,$01,$5c,$c0)
                .BYTE ($5c,$00,$50,$08,$4a,$17,$7a,$5c,$31,$4c,$33,$be,$5c,$4a,$4c,$5a)
                .BYTE ($bf,$5c,$53,$5c,$13,$bd,$00)
mt_patt37:
                .BYTE ($07,$40,$78,$f9,$78,$f9,$78,$f9,$78,$f9,$78,$ef,$be,$fb,$84,$fd)
                .BYTE ($08,$84,$fd,$00)
mt_patt38:
                .BYTE ($4e,$07,$be,$50,$fe,$09,$92,$bd,$4c,$40,$be,$50,$4c,$d0,$92,$5c)
                .BYTE ($20,$56,$00,$50,$fe,$4c,$30,$92,$50,$be,$fd,$90,$bd,$56,$00,$50)
                .BYTE ($bd,$4c,$40,$90,$4c,$10,$be,$50,$4c,$a0,$92,$50,$4c,$0a,$be,$50)
                .BYTE ($fe,$5c,$10,$50,$92,$bd,$4c,$10,$be,$50,$4c,$30,$92,$50,$56,$00)
                .BYTE ($50,$fe,$92,$bd,$be,$5c,$10,$50,$bd,$90,$bd,$56,$00,$50,$bd,$90)
                .BYTE ($be,$bd,$4c,$10,$92,$50,$be,$bd,$00)
mt_patt39:
                .BYTE ($40,$be,$fd,$0a,$95,$bd,$be,$bd,$97,$bd,$56,$00,$50,$fe,$95,$bd)
                .BYTE ($be,$fd,$8b,$bd,$56,$00,$50,$bd,$8b,$be,$bd,$95,$bd,$be,$fb,$95)
                .BYTE ($bd,$be,$bd,$97,$bd,$56,$00,$50,$fe,$95,$bd,$be,$fd,$8b,$bd,$56)
                .BYTE ($00,$50,$bd,$8b,$be,$bd,$95,$bd,$be,$bd,$00)
mt_patt40:
                .BYTE ($05,$4e,$07,$62,$50,$4c,$f8,$62,$50,$bd,$5c,$20,$4c,$0a,$62,$5c)
                .BYTE ($10,$08,$40,$90,$be,$56,$aa,$50,$05,$4c,$20,$62,$56,$aa,$5c,$10)
                .BYTE ($50,$46,$fa,$62,$5c,$e2,$50,$5c,$10,$40,$62,$5c,$02,$56,$66,$5c)
                .BYTE ($20,$08,$40,$84,$be,$fe,$05,$46,$a4,$62,$50,$fe,$4e,$07,$62,$50)
                .BYTE ($4c,$f8,$62,$50,$bd,$5c,$20,$4c,$0a,$62,$5c,$10,$08,$40,$90,$be)
                .BYTE ($fe,$05,$4c,$20,$62,$56,$aa,$5c,$10,$50,$46,$fa,$62,$5c,$e2,$50)
                .BYTE ($5c,$10,$40,$62,$5c,$02,$56,$6f,$5c,$20,$08,$40,$90,$be,$fe,$05)
                .BYTE ($62,$5c,$55,$5c,$02,$56,$40,$00)
mt_patt41:
                .BYTE ($07,$4e,$07,$7a,$50,$fe,$0c,$4a,$0b,$6e,$50,$fe,$07,$4a,$0b,$7a)
                .BYTE ($50,$fe,$0c,$6e,$bd,$6e,$bd,$07,$7a,$fe,$4a,$0b,$be,$0c,$40,$6e)
                .BYTE ($fd,$07,$4a,$0b,$7a,$50,$fe,$0c,$6e,$be,$fe,$07,$4e,$07,$7a,$50)
                .BYTE ($fe,$0c,$4a,$0b,$6e,$50,$fe,$07,$4a,$0b,$6e,$50,$fe,$0c,$6e,$bd)
                .BYTE ($6e,$bd,$07,$7a,$fd,$0c,$75,$fd,$07,$4a,$0b,$7a,$50,$fe,$0c,$6e)
                .BYTE ($fd,$00)
mt_patt42:
                .BYTE ($06,$40,$78,$bd,$84,$bd,$0b,$4c,$00,$7a,$bd,$50,$bd,$06,$7a,$bd)
                .BYTE ($0b,$78,$5c,$00,$bd,$50,$06,$86,$fd,$7a,$bd,$0b,$4c,$00,$78,$fe)
                .BYTE ($50,$06,$4c,$00,$78,$50,$84,$bd,$0b,$6e,$5c,$00,$bd,$50,$06,$7a)
                .BYTE ($bd,$84,$bd,$0b,$7a,$5c,$00,$fe,$06,$40,$7a,$bd,$0b,$6c,$5c,$00)
                .BYTE ($5c,$10,$5c,$00,$06,$40,$86,$fd,$7a,$bd,$0b,$6c,$fd,$06,$4c,$00)
                .BYTE ($78,$bd,$84,$bd,$0b,$84,$bd,$78,$50,$00)
mt_patt43:
                .BYTE ($07,$4e,$07,$7a,$50,$fe,$0c,$4a,$0b,$6e,$50,$fe,$07,$4a,$0b,$7a)
                .BYTE ($50,$fe,$0c,$6e,$bd,$6e,$bd,$07,$7a,$fe,$4a,$0b,$be,$0c,$40,$6e)
                .BYTE ($fd,$07,$4a,$0b,$7a,$50,$fe,$0c,$6e,$be,$fe,$07,$4e,$07,$7a,$50)
                .BYTE ($fe,$0c,$4a,$0b,$6e,$50,$fe,$07,$4a,$0b,$6e,$50,$fe,$0c,$6e,$bd)
                .BYTE ($6e,$bd,$07,$7a,$fd,$0c,$75,$fd,$07,$4a,$0b,$7a,$50,$fe,$04,$4a)
                .BYTE ($17,$6e,$50,$be,$bd,$00)
mt_patt44:
                .BYTE ($06,$40,$78,$bd,$84,$bd,$0b,$4c,$00,$7a,$bd,$50,$bd,$06,$7a,$bd)
                .BYTE ($0b,$78,$5c,$00,$bd,$50,$06,$86,$fd,$7a,$bd,$0b,$4c,$00,$78,$fe)
                .BYTE ($50,$06,$4c,$00,$78,$50,$84,$bd,$0b,$6e,$5c,$00,$bd,$50,$06,$7a)
                .BYTE ($bd,$84,$bd,$0b,$7a,$5c,$00,$fe,$06,$40,$7a,$bd,$0b,$6c,$5c,$00)
                .BYTE ($0a,$40,$90,$bd,$84,$bd,$06,$78,$bd,$90,$bd,$78,$bd,$90,$bd,$6e)
                .BYTE ($54,$03,$fe,$40,$92,$bd,$86,$bd,$00)
mt_patt45:
                .BYTE ($40,$be,$fd,$09,$92,$bd,$be,$bd,$92,$bd,$56,$00,$50,$fe,$92,$bd)
                .BYTE ($be,$fd,$90,$bd,$56,$00,$50,$bd,$90,$be,$bd,$4c,$40,$92,$50,$be)
                .BYTE ($fd,$4c,$03,$92,$50,$fc,$4c,$40,$92,$50,$4c,$40,$90,$50,$be,$bd)
                .BYTE ($4c,$40,$92,$50,$be,$fd,$4c,$30,$90,$5c,$10,$4c,$20,$be,$50,$54)
                .BYTE ($05,$bd,$4c,$20,$90,$54,$05,$4c,$10,$90,$54,$05,$fe,$00)
mt_patt46:
                .BYTE ($40,$be,$fd,$0a,$95,$bd,$be,$bd,$97,$bd,$56,$00,$50,$fe,$95,$bd)
                .BYTE ($be,$fd,$8b,$bd,$56,$00,$50,$bd,$8b,$be,$bd,$95,$bd,$be,$fd,$97)
                .BYTE ($fb,$95,$bd,$97,$bd,$be,$bd,$94,$bd,$be,$fd,$94,$bd,$be,$bd,$54)
                .BYTE ($05,$bd,$40,$95,$54,$05,$40,$97,$54,$05,$fe,$00)
mt_patt47:
                .BYTE ($06,$40,$78,$bd,$84,$bd,$0b,$4c,$00,$7a,$bd,$50,$bd,$06,$7a,$bd)
                .BYTE ($0b,$78,$5c,$00,$bd,$50,$06,$86,$fd,$7a,$bd,$0b,$4c,$00,$78,$fe)
                .BYTE ($50,$06,$4c,$00,$78,$50,$84,$bd,$0b,$6e,$5c,$00,$bd,$50,$06,$7a)
                .BYTE ($bd,$84,$bd,$92,$54,$04,$fe,$0b,$40,$6e,$fd,$06,$90,$54,$04,$fe)
                .BYTE ($40,$84,$bd,$0b,$7a,$fd,$06,$90,$bd,$92,$54,$05,$fa,$00)
mt_patt48:
                .BYTE ($50,$c1,$00)
mt_patt49:
                .BYTE ($40,$be,$bd,$09,$92,$bd,$4c,$40,$be,$50,$fe,$4c,$d0,$92,$5c,$20)
                .BYTE ($46,$00,$be,$50,$fe,$4c,$30,$90,$50,$be,$bd,$90,$fe,$56,$00,$50)
                .BYTE ($bd,$4c,$40,$90,$4c,$10,$be,$50,$bd,$5c,$a0,$50,$4c,$0a,$be,$50)
                .BYTE ($fe,$5c,$10,$50,$92,$bd,$4c,$10,$be,$50,$4c,$30,$92,$50,$56,$00)
                .BYTE ($50,$fe,$92,$bd,$be,$5c,$10,$50,$bd,$90,$bd,$56,$00,$50,$bd,$90)
                .BYTE ($be,$bd,$5c,$10,$50,$fe,$00)
mt_patt50:
                .BYTE ($40,$be,$bd,$0a,$95,$bd,$be,$fd,$97,$bd,$46,$00,$be,$50,$fe,$95)
                .BYTE ($bd,$be,$bd,$8b,$fe,$56,$00,$50,$bd,$8b,$be,$fc,$be,$fb,$95,$bd)
                .BYTE ($be,$bd,$97,$bd,$56,$00,$50,$fe,$95,$bd,$be,$fd,$8b,$bd,$56,$00)
                .BYTE ($50,$bd,$8b,$be,$fb,$00)
mt_patt51:
                .BYTE ($40,$be,$c1,$00)
