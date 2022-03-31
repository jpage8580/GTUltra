//
// BME (Blasphemous Multimedia Engine) windows & timing module
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include "bme_main.h"
#include "bme_gfx.h"
#include "bme_mou.h"
#include "bme_io.h"
#include "bme_err.h"
#include "bme_cfg.h"

SDL_Joystick *joy[MAX_JOYSTICKS] = { NULL };
Sint16 joyx[MAX_JOYSTICKS];
Sint16 joyy[MAX_JOYSTICKS];
Uint32 joybuttons[MAX_JOYSTICKS];
SDL_Window *win_window = NULL;

// Prototypes

int win_openwindow(unsigned xsize, unsigned ysize, char *appname, char *icon);
void win_closewindow(void);
void win_messagebox(char *string);
void win_checkmessages(void);
int win_getspeed(int framerate);
void win_setmousemode(int mode);

// Global variables

int win_fullscreen = 0; // By default windowed
int win_windowinitted = 0;
int win_quitted = 0;
unsigned char win_keytable[SDL_NUM_SCANCODES] = { 0 };
unsigned char win_asciikey = 0;
unsigned win_virtualkey = 0;
unsigned win_mousexpos = 0;
unsigned win_mouseypos = 0;
unsigned win_mousexrel = 0;
unsigned win_mouseyrel = 0;
unsigned win_mousebuttons = 0;
int win_mouseWheelY = 0;

int win_mousemode = MOUSE_FULLSCREEN_HIDDEN;
unsigned char win_keystate[SDL_NUM_SCANCODES] = { 0 };

// Static variables

static int win_lasttime = 0;
static int win_currenttime = 0;
static int win_framecounter = 0;
static int win_activateclick = 0;

int win_openwindow(unsigned xsize, unsigned ysize, char *appname, char *icon)
{
	Uint32 flags = win_fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

	if (!win_windowinitted)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
		{
			return BME_ERROR;
		}
		atexit(SDL_Quit);
		win_windowinitted = 1;
	}

	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	//SDL_EnableUNICODE(1);
	win_window = SDL_CreateWindow(appname,
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								xsize, ysize,
								flags);
	return BME_OK;
}

int enableState = -1;
void win_enableKeyRepeat(void)
{
	if (enableState != 1)
	{
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
		enableState = 1;
	}
}

void win_disableKeyRepeat(void)
{
	if (enableState != 0)
	{
		SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
		enableState = 0;
	}
}

void win_closewindow(void)
{
    SDL_DestroyWindow(win_window);
}

void win_messagebox(char *string)
{
	return;
}

int win_getspeed(int framerate)
{
	// Note: here 1/10000th of a second accuracy is used (although
	// timer resolution is only millisecond) for minimizing
	// inaccuracy in frame duration calculation -> smoother screen
	// update

	int frametime = 10000 / framerate;
	int frames = 0;

	while (!frames)
	{
		win_checkmessages();

		win_lasttime = win_currenttime;
		win_currenttime = SDL_GetTicks();

		win_framecounter += (win_currenttime - win_lasttime) * 10;
		frames = win_framecounter / frametime;
		win_framecounter -= frames * frametime;

		if (!frames) SDL_Delay((frametime - win_framecounter) / 10);
	}

	return frames;
}

// This is the "message pump". Called by following functions:
// win_getspeed();
// kbd_waitkey();
//
// It is recommended to be called in any long loop where those two functions
// are not called.

void win_checkmessages(void)
{
	SDL_Event event;
	unsigned keynum;

	win_activateclick = 0;

	SDL_PumpEvents();

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_JOYBUTTONDOWN:
			joybuttons[event.jbutton.which] |= 1 << event.jbutton.button;
			break;

		case SDL_JOYBUTTONUP:
			joybuttons[event.jbutton.which] &= ~(1 << event.jbutton.button);
			break;

		case SDL_JOYAXISMOTION:
			switch (event.jaxis.axis)
			{
			case 0:
				joyx[event.jaxis.which] = event.jaxis.value;
				break;

			case 1:
				joyy[event.jaxis.which] = event.jaxis.value;
				break;
			}
			break;

		case SDL_MOUSEMOTION:
			win_mousexpos = event.motion.x;
			win_mouseypos = event.motion.y;
			win_mousexrel += event.motion.xrel;
			win_mouseyrel += event.motion.yrel;
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				win_mousebuttons |= MOUSEB_LEFT;
				break;

			case SDL_BUTTON_MIDDLE:
				win_mousebuttons |= MOUSEB_MIDDLE;
				break;

			case SDL_BUTTON_RIGHT:
				win_mousebuttons |= MOUSEB_RIGHT;
				break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				win_mousebuttons &= ~MOUSEB_LEFT;
				break;

			case SDL_BUTTON_MIDDLE:
				win_mousebuttons &= ~MOUSEB_MIDDLE;
				break;

			case SDL_BUTTON_RIGHT:
				win_mousebuttons &= ~MOUSEB_RIGHT;
				break;
			}
			break;

		case SDL_QUIT:
			win_quitted = 1;
			break;

		case SDL_TEXTINPUT:
			win_asciikey = event.text.text[0];
			break;

		case SDL_KEYDOWN:
			win_virtualkey = event.key.keysym.sym;
			keynum = event.key.keysym.scancode;
			if (keynum < SDL_NUM_SCANCODES)
			{
				win_keytable[keynum] = 1;
				win_keystate[keynum] = 1;
				if ((keynum == SDL_SCANCODE_RETURN) && ((win_keystate[SDL_SCANCODE_LALT]) || (win_keystate[SDL_SCANCODE_RALT])))
				{
					win_fullscreen ^= 1;
					gfx_reinit();
				}
			}
			break;

		case SDL_KEYUP:
			keynum = event.key.keysym.scancode;
			if (keynum < SDL_NUM_SCANCODES)
			{
				win_keytable[keynum] = 0;
				win_keystate[keynum] = 0;
			}
			break;

		case SDL_WINDOWEVENT_RESIZED:
		//case SDL_VIDEOEXPOSE:
			gfx_redraw = 1;
			break;
		}
	}
}

void win_setmousemode(int mode)
{
	win_mousemode = mode;

	switch (mode)
	{
	case MOUSE_ALWAYS_VISIBLE:
		SDL_ShowCursor(SDL_ENABLE);
		break;

	case MOUSE_FULLSCREEN_HIDDEN:
		if (gfx_fullscreen)
		{
			SDL_ShowCursor(SDL_DISABLE);
		}
		else
		{
			SDL_ShowCursor(SDL_ENABLE);
		}
		break;

	case MOUSE_ALWAYS_HIDDEN:
		SDL_ShowCursor(SDL_DISABLE);
		break;
	}
}

