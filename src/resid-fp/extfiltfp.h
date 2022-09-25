//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581 SID emulator engine.
//  Copyright (C) 2004  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------

#ifndef __EXTFILTFP_H__
#define __EXTFILTFP_H__

#include <math.h>

#include "siddefsfp.h"

// ----------------------------------------------------------------------------
// The audio output stage in a Commodore 64 consists of two STC networks,
// a low-pass filter with 3-dB frequency 16kHz followed by a high-pass
// filter with 3-dB frequency 16Hz (the latter provided an audio equipment
// input impedance of 1kOhm).
// The STC networks are connected with a BJT supposedly meant to act as
// a unity gain buffer, which is not really how it works. A more elaborate
// model would include the BJT, however DC circuit analysis yields BJT
// base-emitter and emitter-base impedances sufficiently low to produce
// additional low-pass and high-pass 3dB-frequencies in the order of hundreds
// of kHz. This calls for a sampling frequency of several MHz, which is far
// too high for practical use.
// ----------------------------------------------------------------------------
class ExternalFilterFP
{
public:
  ExternalFilterFP();

  void set_clock_frequency(float);

  inline void clock(float Vi,int LR);
  void reset();

  // Audio output (20 bits).
  inline float output(int LR);

  inline float outputL();
  inline float outputR();

private:
  inline void nuke_denormals();

  // State of filters.
  // JP - 2 for stereo
  float Vlp[2]; // lowpass
  float Vhp[2]; // highpass

  // Cutoff frequencies.
  float w0lp;
  float w0hp;

friend class SIDFP;
};

// ----------------------------------------------------------------------------
// SID clocking - 1 cycle.
// JP added LR index
// ----------------------------------------------------------------------------
inline
void ExternalFilterFP::clock(float Vi,int LR)
{
	float *vLptr = &Vlp[LR];
	float *vHptr = &Vhp[LR];


  float dVlp = w0lp * (Vi - *vLptr);
  float dVhp = w0hp * (*vLptr - *vHptr);
  *vLptr += dVlp;
  *vHptr += dVhp;
}

// ----------------------------------------------------------------------------
// Audio output (19.5 bits).
// ----------------------------------------------------------------------------
inline
float ExternalFilterFP::output(int LR)
{
  return Vlp[LR] - Vhp[LR];
}

inline
float ExternalFilterFP::outputL()
{
	return Vlp[0] - Vhp[0];
}

inline
float ExternalFilterFP::outputR()
{
	return Vlp[1] - Vhp[1];
}

inline
void ExternalFilterFP::nuke_denormals()
{
	for (int i = 0;i < 2;i++)
	{
		if (Vhp[i] > -1e-12f && Vhp[i] < 1e-12f)
			Vhp[i] = 0;
		if (Vlp[i] > -1e-12f && Vlp[i] < 1e-12f)
			Vlp[i] = 0;
	}
}

#endif // not __EXTFILTFP_H__
