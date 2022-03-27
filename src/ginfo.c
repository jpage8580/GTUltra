//
// GOATTRACKER ULTRA Info display
//

#define GINFO_C

#include "goattrk2.h"
#include "ginfo.h"

int clearInfoLine = 0;

int lastInfoDisplayed = INFO_CLEAR;
void infoDisplay()
{
	return;
}




char *patternInstructionInfoString[] = {
"(1) Portamento up. Value: $%04X",
"(2) Portamento down. Value: $%04X",
"(3) Tone portamento. Value: $%04X",
"(4) Vibrato Speedtable pointer. Speed: $%02X Depth: $%02X",
"(5) Attack: $%02X Decay: $%02X",
"(6) Sustain: $%02X Release: $%02X",
"(7) Waveform: $%02X",
"(8) Wavetable pointer (0 to stop) Value: $%02X",
"(9) Pulsetable pointer (0 to stop) Value: $%02X",
"(A) Filtertable pointer (0 to stop) Value: $%02X",
"(B) Filter XY. Resonance: $%1X. Channel Enabled Bitmask: $%1X",
"(C) Filter cutoff: $%02X",
"(D) Volume/Marker:  XY $%02X",
"(E) Funk Tempo. Tempo1: $%02X Tempo2: $%02X",
"(F) Global Tempo: $%02X"
};

char *instrumentInfoString[] = {
	"Attack: $%02X Decay: $%02X",
	"Sustain: $%02X Release: $%02X",
	"Wavetable pointer: $%02X",
	"Pulsetable pointer: $%02X (00 = leave untouched)",
	"Filtertable pointer: $%02X (00 = leave untouched)",
	"Vibrato speedtable pointer. Speed: $%02X Depth: $%02X",
	"Vibrato Delay: $%02X ticks until vibrato starts",
	"HR/Gate Delay: $%02X ticks until note start",	// $80=No Hard Restart. $40=No GateOff",
	"1st Frame Wave: $%02X .Usually $09 (gate + testbit)"	//. $00,$FE & $FF = special"
};

char *filterTypeString[] = {
	"No Filter Type",
	"Low Pass",
	"Band Pass",
	"High Pass"
};

char *filterChannelsEnabledString[] = {
	"Active chn: None (0)",
	"Active chn: 1 (1)",
	"Active chn: 2 (2)",
	"Active chn: 1+2 (3)",
	"Active chn: 3 (4)",
	"Active chn: 1+3 (5)",
	"Active chn: 2+3 (6)",
	"Active chn: All (7)"
};

int lastEditWindow = -1;

// Pattern specific
int lastInfoPatternCh = -1;
int lastInfoPattern = -1;
int lastInfoPatternPos = -1;
int lastInfoTablePos = -1;
int lastInfoTableNum = -1;

// Instrument specific
int lastInstrumentNumber = -1;
int lastInstrumentParam = -1;	// example: AD,SR, Vibrato..

int lastTableNumber = -1;
int lastTableIndex = -1;
int lastTableLR = -1;
int infoWaitMS = 0;
int lastMS = 0;
int msDelta = 0;

void displayTableInfo(GTOBJECT *gt)
{
	if (editorInfo.etcolumn == lastTableLR && editorInfo.etnum == lastTableNumber && editorInfo.etpos == lastTableIndex && editorInfo.editmode == lastEditWindow)
		return;

	waveformDisplayInfo.displayOnOff = 0;


	lastEditWindow = editorInfo.editmode;
	lastTableIndex = editorInfo.etpos;
	lastTableLR = editorInfo.etcolumn;
	lastTableNumber = editorInfo.etnum;

	if (editorInfo.etnum == WTBL)
	{
		displayWaveTableInfo(gt);
	}
	else if (editorInfo.etnum == PTBL)
	{
		displayPulseTableInfo(gt);
	}
	else if (editorInfo.etnum == FTBL)
	{
		displayFilterTableInfo(gt);
	}
	else if (editorInfo.etnum == STBL)
	{
		displaySpeedTableInfo(gt);
	}
}

void displaySpeedTableInfo(GTOBJECT *gt)
{
	sprintf(infoTextBuffer, "SpeedTable");
}

void displayOrderTableInfo(GTOBJECT *gt)
{
	sprintf(infoTextBuffer, "OrderTable");
}

void displayWaveTableInfo(GTOBJECT *gt)
{

	if ((editorInfo.etcolumn / 2) == 0)
	{
		displayWaveTableLeft(gt, "left");
	}
	else
		displayWaveTableRight(gt);
}



void displayWaveTableRight(GTOBJECT *gt)
{
	int ldata = ltable[WTBL][editorInfo.etpos];
	int rdata = rtable[WTBL][editorInfo.etpos];

	if (ldata >= 0xf0)	// right data is part of the left data $Fx instruction.
	{
		displayWaveTableLeft(gt, "right");
		return;
	}

	if (rdata < 0x60)
		sprintf(infoTextBuffer, "(right): Note offset +$%02X (%d)", rdata, rdata);
	else if (rdata < 0x80)
	{
		int v = 0x80 - rdata; // 0-0x1f

		sprintf(infoTextBuffer, "(right): Note offset -$%02X (-%02d)", v, v);
	}
	else if (rdata == 0x80)
		sprintf(infoTextBuffer, "(right): Keep note unchanged");
	else if (rdata < 0xe0)
	{
		sprintf(infoTextBuffer, "(right): Absolute note ($%02X = %s)", rdata, notenameTableView[rdata - 0x80]);
	}
	else
		sprintf(infoTextBuffer, "(right): Invalid value ($%02X. Max value:$DF)", rdata);	//, notenameTableView[rdata - 0x80]);
}

void displayWaveTableLeft(GTOBJECT *gt, char *leftright)
{
	int ldata = ltable[WTBL][editorInfo.etpos];
	int rdata = rtable[WTBL][editorInfo.etpos];

	if (ldata == 0)
		sprintf(infoTextBuffer, "(%s): 00 = leave waveform unchanged", leftright);
	else if (ldata < 0x10)
		sprintf(infoTextBuffer, "(%s):Delay $%02X (%d) (delay this step for n frames)", leftright, ldata, ldata);
	else if (ldata < 0xe0)
	{
		waveformDisplayInfo.displayOnOff = 1;
		waveformDisplayInfo.value = ldata;
		waveformDisplayInfo.destAddress = &ltable[WTBL][editorInfo.etpos];

		sprintf(infoTextBuffer, "(%s):Waveform $%02X", leftright, ldata);
	}
	else if (ldata < 0xf0)
	{
		waveformDisplayInfo.displayOnOff = 1;
		waveformDisplayInfo.value = ldata-0xe0;
		waveformDisplayInfo.destAddress = &ltable[WTBL][editorInfo.etpos];
		sprintf(infoTextBuffer, "(%s):Inaudible waveform $%02X (converts to %02X)", leftright, ldata, ldata - 0xe0);
	}
	else if (ldata < 0xff)
	{
		int instr = ldata - 0xf0;
		int instrIndex = instr - 1;

		if (instr == 1 || instr == 2 || instr == 3)
		{
			int speed = (ltable[STBL][rdata - 1] << 8) | rtable[STBL][rdata - 1];
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], speed);
		}
		else if (instr == 5 || instr == 6 || instr == 0xb)
		{
			int nybHi = (rdata & 0xf0) >> 4;
			int nybLo = rdata & 0xf;
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], nybHi, nybLo);
		}
		else if (instr == 7 || instr == 8 || instr == 9 || instr == 0xa || instr == 0xc || instr == 0xd)
		{
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], ldata);

			if (instr == 7)
			{
				waveformDisplayInfo.displayOnOff = 1;
				waveformDisplayInfo.value = rdata;
				waveformDisplayInfo.destAddress = &rtable[WTBL][editorInfo.etpos];
			}

		}
		else if (instr == 4 || instr == 0xe)
		{
			int tempo1 = ltable[STBL][rdata - 1];
			int tempo2 = rtable[STBL][rdata - 1];
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], tempo1, tempo2);
		}
	}
	else if (ldata == 0xff)
	{
		if (rdata)
			sprintf(infoTextBuffer, "(%s): Jump to $%02X ($00 = stop)", leftright, rdata);
		else
			sprintf(infoTextBuffer, "(%s): Stop. ($01-$FF = Jump to position)", leftright);
	}
}


void displayPulseTableInfo(GTOBJECT *gt)
{
	int ldata = ltable[PTBL][editorInfo.etpos];
	int rdata = rtable[PTBL][editorInfo.etpos];

	if (ldata == 0)
		sprintf(infoTextBuffer, "0 (undefined)");
	else if (ldata < 0x80)
	{
		int s = rdata;
		if (s >= 0x80)
		{
			s = 256 - s;
			sprintf(infoTextBuffer, "For $%02X (%d) ticks, pulse - $%02X (%d)", ldata, ldata, rdata, s);
		}
		else
			sprintf(infoTextBuffer, "For $%02X (%d) ticks, pulse + $%02X (%d)", ldata, ldata, rdata, rdata);
	}
	else if (ldata < 0xff)
	{
		int s = rdata + ((ldata & 0xf) << 8);
		sprintf(infoTextBuffer, "PulseTable: Set pulse value $%03X (%d)", s, s);
	}
	else if (ldata == 0xff)
	{
		if (rdata)
			sprintf(infoTextBuffer, "Jump to $%02X ($00 = stop)", rdata);
		else
			sprintf(infoTextBuffer, "Stop. ($01-$FF = Jump to position)");
	}

}


void displayFilterTableInfo(GTOBJECT *gt)
{
	int ldata = ltable[FTBL][editorInfo.etpos];
	int rdata = rtable[FTBL][editorInfo.etpos];

	if (ldata == 0)
		sprintf(infoTextBuffer, "Set cutoff $%02X (%d)", rdata, rdata);
	else if (ldata < 0x80)
	{
		int s = rdata;
		if (s >= 0x80)
		{
			s = 256 - s;
			sprintf(infoTextBuffer, "For $%02X (%d) ticks, cutoff - $%02X (%d)", ldata, ldata, s, s);
		}
		else
			sprintf(infoTextBuffer, "For $%02X (%d) ticks, cutoff + $%02X (%d)", ldata, ldata, rdata, rdata);
	}
	else if (ldata <= 0xf0)
	{

		int filterType = (ldata & 0x70) >> 4;	// remove top bit
		int ft = 0;
		while (filterType != 0)
		{
			ft++;
			filterType >>= 1;
		};

		int resonance = (rdata & 0xf0) >> 4;
		int channelsEnabled = rdata & 0xf;

		sprintf(infoTextBuffer, "%s ($%02X). Resonance: $%02X. %s", filterTypeString[ft], ldata, resonance, filterChannelsEnabledString[channelsEnabled]);

	}
	else if (ldata == 0xff)
	{
		if (rdata)
			sprintf(infoTextBuffer, "Jump to $%02X ($00 = stop)", rdata);
		else
			sprintf(infoTextBuffer, "Stop. ($01-$FF = Jump to position)");
	}

}


void displayInstrumentInfo(GTOBJECT *gt)
{

	if (editorInfo.einum == lastInstrumentNumber && editorInfo.eipos == lastInstrumentParam && editorInfo.editmode == lastEditWindow)
		return;

	waveformDisplayInfo.displayOnOff = 0;


	lastInstrumentNumber = editorInfo.einum;
	lastInstrumentParam = editorInfo.eipos;
	lastEditWindow = editorInfo.editmode;

	int param = editorInfo.eipos;

	if (param == 0)
	{
		int nybHi = instr[editorInfo.einum].ad >> 4;
		int nybLo = instr[editorInfo.einum].ad & 0xf;
		sprintf(infoTextBuffer, instrumentInfoString[param], nybHi, nybLo);
	}
	if (param == 1)
	{
		int nybHi = instr[editorInfo.einum].sr >> 4;
		int nybLo = instr[editorInfo.einum].sr & 0xf;
		sprintf(infoTextBuffer, instrumentInfoString[param], nybHi, nybLo);
	}
	if (param == 2)
	{
		sprintf(infoTextBuffer, instrumentInfoString[param], instr[editorInfo.einum].ptr[WTBL]);
	}
	if (param == 3)
	{
		sprintf(infoTextBuffer, instrumentInfoString[param], instr[editorInfo.einum].ptr[PTBL]);
	}
	if (param == 4)
	{
		sprintf(infoTextBuffer, instrumentInfoString[param], instr[editorInfo.einum].ptr[FTBL]);
	}
	if (param == 5)
	{
		int tempo1 = ltable[STBL][instr[editorInfo.einum].ptr[STBL] - 1];
		int tempo2 = rtable[STBL][instr[editorInfo.einum].ptr[STBL] - 1];
		sprintf(infoTextBuffer, instrumentInfoString[param], tempo1, tempo2);
	}
	if (param == 6)
	{
		sprintf(infoTextBuffer, instrumentInfoString[param], instr[editorInfo.einum].vibdelay);
	}
	if (param == 7)
	{
		sprintf(infoTextBuffer, instrumentInfoString[param], instr[editorInfo.einum].gatetimer);
	}
	if (param == 8)
	{
		waveformDisplayInfo.displayOnOff = 1;
		waveformDisplayInfo.value = instr[editorInfo.einum].firstwave;
		waveformDisplayInfo.destAddress = &instr[editorInfo.einum].firstwave;
		sprintf(infoTextBuffer, instrumentInfoString[param], instr[editorInfo.einum].firstwave);
	}
}

void displayPatternInfo(GTOBJECT *gt)
{

	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);


	if (infoWaitMS > 0)
	{
		infoWaitMS -= msDelta;
		if (infoWaitMS > 0)
			return;
	}
	infoWaitMS = 0;

	if (editorInfo.etnum == lastInfoTableNum && editorInfo.etpos == lastInfoTablePos && c2 == lastInfoPatternCh && gt->editorInfo[c2].epnum == lastInfoPattern && editorInfo.eppos == lastInfoPatternPos && editorInfo.editmode == lastEditWindow)
		return;

	waveformDisplayInfo.displayOnOff = 0;


	lastInfoTableNum = editorInfo.etnum;
	lastInfoTablePos = editorInfo.etpos;
	lastInfoPatternCh = c2;
	lastInfoPattern = gt->editorInfo[c2].epnum;
	lastInfoPatternPos = editorInfo.eppos;
	lastEditWindow = editorInfo.editmode;

	if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2])	// instruction not 0?
	{
		int instr = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2];
		int instrIndex = instr - 1;

		int data = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3];

		if (instr == 1 || instr == 2 || instr == 3)
		{
			int speed = (ltable[STBL][data - 1] << 8) | rtable[STBL][data - 1];
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], speed);
		}
		else if (instr == 5 || instr == 6 || instr == 0xb)
		{
			int nybHi = (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] & 0xf0) >> 4;
			int nybLo = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] & 0xf;
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], nybHi, nybLo);
		}
		else if (instr == 7 || instr == 8 || instr == 9 || instr == 0xa || instr == 0xc || instr == 0xd)
		{
			if (instr == 7)
			{
				waveformDisplayInfo.destAddress = &pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3];
				waveformDisplayInfo.displayOnOff = 1;
				waveformDisplayInfo.value = data;
			}
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], data);
		}
		else if (instr == 4 || instr == 0xe)
		{
			int tempo1 = ltable[STBL][data - 1];
			int tempo2 = rtable[STBL][data - 1];
			sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], tempo1, tempo2);
		}
		else if (instr == 0xf)
		{
			if (data < 0x80)
				sprintf(infoTextBuffer, patternInstructionInfoString[instrIndex], data);
			else
				sprintf(infoTextBuffer, "Channel Tempo: %02X", (data - 0x80));
		}
		else
			sprintf(infoTextBuffer, patternInstructionInfoString[instr]);
	}
	else
		sprintf(infoTextBuffer, "                                ");

}







