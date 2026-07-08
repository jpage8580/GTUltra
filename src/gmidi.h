#ifndef GMIDI_H
#define GMIDI_H

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct {
		unsigned char *message;
		int size;
	}MIDI_MESSAGE;

	int initMidi(int midiPort);
	int checkForMidiInput(MIDI_MESSAGE *m, int midiPort);
	unsigned int getPortCount();
	char* getPortName(int portNumber);

#ifdef __cplusplus
}
#endif


#endif

