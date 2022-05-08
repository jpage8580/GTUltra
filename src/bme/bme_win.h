// BME windows, input & timing module header file

int win_openwindow(unsigned xsize, unsigned ysize, char *appname, char *icon, int enableAntiAlias);
void win_closewindow(void);
void win_messagebox(char *string);
void win_checkmessages(void);
int win_getspeed(int framerate);
void win_setmousemode(int mode);
void win_enableKeyRepeat(void);
void win_disableKeyRepeat(void);

extern float xmouseScale;
extern float ymouseScale;

extern int win_windowinitted;
extern int win_quitted;
extern int win_fullscreen;
extern unsigned char win_keytable[SDL_NUM_SCANCODES];
extern unsigned char win_keystate[SDL_NUM_SCANCODES];
extern unsigned char win_asciikey;
extern unsigned win_virtualkey;
extern unsigned win_mousexpos;
extern unsigned win_mouseypos;
extern unsigned win_mousexrel;
extern unsigned win_mouseyrel;
extern unsigned win_mousebuttons;
extern int win_mousemode;
extern SDL_Joystick *joy[MAX_JOYSTICKS];
extern Sint16 joyx[MAX_JOYSTICKS];
extern Sint16 joyy[MAX_JOYSTICKS];
extern Uint32 joybuttons[MAX_JOYSTICKS];
extern SDL_Window *win_window;
