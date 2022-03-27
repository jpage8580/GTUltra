#ifndef GMIDI_H
#define GMIDI_H

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct {
		unsigned char *message;
		int size;
	}MIDI_MESSAGE;

	int initMidi();
	int checkForMidiInput(MIDI_MESSAGE *m);

#ifdef __cplusplus
}
#endif


#endif

