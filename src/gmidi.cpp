//
// GOATTRACKER ULTRA MIDI
//

#define GMIDI_C

#define __WINDOWS__
//#define __LINUX__

#include <iostream>
#include <cstdlib>
#include <signal.h>

#ifdef __WINDOWS__
#include <windows.h>
#endif

#include "RtMidi.h"

#include "gmidi.h"


bool done;
static void finish(int ignore) { done = true; }
RtMidiIn *midiin;

bool portOpen = false;

int initMidi()
{
	midiin = new RtMidiIn();
	portOpen = false;

	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if (nPorts > 0)
	{
		midiin->openPort(0);
		// Don't ignore sysex, timing, or active sensing messages.
		midiin->ignoreTypes(false, false, false);
		portOpen = true;
	}

	return nPorts;

}

char unsigned msg[4096];

int test = 0;
int checkForMidiInput(MIDI_MESSAGE *m)
{
	m->size = 0;
	if (portOpen)
	{
		int nDevices = midiin->getPortCount();
		if (nDevices)
		{
		
			std::vector<unsigned char> message;
			int nBytes, i;
			double stamp;

			stamp = midiin->getMessage(&message);
			m->message = (unsigned char*)&msg;
			m->size = message.size();

			for (int i = 0;i < m->size;i++)
			{
				msg[i] = (unsigned char)message[i];
			}
		}
		else
		{
			portOpen = false;
			midiin->closePort();
		}
		return nDevices;	// nDevices;
	}
	else
	{
		midiin = new RtMidiIn();

		unsigned int nPorts = midiin->getPortCount();
		if (nPorts > 0)
		{
			midiin->openPort(0);
			// Don't ignore sysex, timing, or active sensing messages.
			midiin->ignoreTypes(false, false, false);
			return 1;
		}
		return test;
	}
	return 999;

}





