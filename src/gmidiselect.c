//
// GOATTRACKER ULTRA MIDI Port Select
//

#define GMIDISELECT_C

#include "goattrk2.h"
#include "gmidiselect.h"

void displayMIDISelectWindow()
{
	int lastclick = 0;
	int exitMIDISelect = 1;

	int portCount = getPortCount();

	int initialPort = selectedMIDIPort;

	while (exitMIDISelect)
	{
//		int cc = cursorcolortable[cursorflash];
		if (cursorflashdelay >= 6)
		{
			cursorflashdelay %= 6;
			cursorflash++;
			cursorflash &= 3;
		}

		fliptoscreen();
		getkey();
		if (lastclick)
			lastclick--;

		if (win_quitted)
			return;

		int boxColor = 0xe;
		int boxWidth = 64;
		int boxHeight = 20;
		int boxX = 3;
		int boxY = 3;

		if ((mouseb) && (!prevmouseb))
		{
			lastclick = DOUBLECLICKDELAY;

		}
		if (mouseb)
		{


			if ((mousey < boxY) || (mousey >= boxY + boxHeight) || (mousex < boxX) || (mousex >= boxX + boxWidth))
			{
				if ((!prevmouseb) && (lastclick))
					exitMIDISelect = 0;
			}

		}

		switch (rawkey)
		{
		case KEY_ESC:
			selectedMIDIPort = initialPort;
			exitMIDISelect = 0;
			break;

		case KEY_ENTER:
			exitMIDISelect = 0;
			break;

		case KEY_UP:
			if (selectedMIDIPort)
				selectedMIDIPort--;
			break;
		case KEY_DOWN:
			if (selectedMIDIPort < 9)
				selectedMIDIPort++;
			break;
		}



		for (int c = 0;c < boxHeight;c++)
		{
			printblank(boxX, boxY + c, boxWidth);
		}

		drawbox(boxX, boxY, boxColor, boxWidth, boxHeight);


		printtext(boxX + 2, boxY + 3, getColor(boxColor, 0), "Port     Name");

		printtext(boxX + 2, boxY + 17, getColor(boxColor, 0), "Up/Down to highlight.");	// Enter to select.Esc to cancel");
		printtext(boxX + 2, boxY + 18, getColor(boxColor, 0), "Quit and restart GTUltra for changes to take effect");
		for (int i = 0;i < 10;i++)
		{
			int color = CNORMAL;
			if (selectedMIDIPort == i)
				color = CEDIT;
			if (i <= portCount)
			{
				char *p = getPortName(i);
				if (p != NULL)
				{
					sprintf(textbuffer, "%2d.      %s", i, p);
					printtext(boxX + 2, boxY + 5 + i, getColor(color, 0), textbuffer);
					free(p);
				}
				else
				{
					sprintf(textbuffer, "%2d.      (no name)", i);
					printtext(boxX + 2, boxY + 5 + i, getColor(color, 0), textbuffer);
				}

			}
			else
			{
				sprintf(textbuffer, "%2d.", i);
				printtext(boxX + 2, boxY + 5 + i, getColor(color, 0), textbuffer);
			}
		}

		printtext(boxX +25, 4, getColor(0, boxColor), "MIDI Port Select");

		fliptoscreen();

	}
}

