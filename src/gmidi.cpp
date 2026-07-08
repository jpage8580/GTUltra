//
// GOATTRACKER ULTRA MIDI
//

#define GMIDI_C

#include <iostream>
#include <cstdlib>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "RtMidi.h"

#include "gmidi.h"
#include "gmidiselect.h"


bool done;
//static void finish(int ignore) { done = true; }
RtMidiIn *midiin;

bool portOpen = false;

int initMidi(int midiPort)
{
	midiin = new RtMidiIn();
	portOpen = false;

	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if (nPorts == 0)
		return 0;

	if (nPorts <= (unsigned int)midiPort)
		midiPort = 0;

	midiin->openPort(midiPort);
	// Don't ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes(false, false, false);
	portOpen = true;


	return midiPort;

}

char unsigned msg[4096];

int test = 0;
int checkForMidiInput(MIDI_MESSAGE *m,int midiPort)
{
	m->size = 0;
	if (portOpen)
	{
		int nDevices = midiin->getPortCount();
		if (nDevices)
		{

			std::vector<unsigned char> message;
			//int nBytes, i;
			//double stamp;

			midiin->getMessage(&message);
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
			midiin->openPort(midiPort);
			// Don't ignore sysex, timing, or active sensing messages.
			midiin->ignoreTypes(false, false, false);
			return 1;
		}
		return test;
	}
	return 999;

}

unsigned int getPortCount()
{
	return  midiin->getPortCount();
}

char* getPortName(int portNumber)
{
	std::string str;
	try {
		str = midiin->getPortName(portNumber);
	}
	catch (RtMidiError &error) {
		return NULL;
	}

	char * writable = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), writable);
	writable[str.size()] = '\0'; // don't forget the terminating 0
	return writable;
}


