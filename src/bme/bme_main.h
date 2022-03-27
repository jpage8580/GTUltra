// BME main definitions header file

#define GFX_SCANLINES 1
#define GFX_DOUBLESIZE 2
#define GFX_USE1PAGE 0
#define GFX_USE2PAGES 4
#define GFX_USE3PAGES 8
#define GFX_WAITVBLANK 16
#define GFX_FULLSCREEN 32
#define GFX_WINDOW 64
#define GFX_NOSWITCHING 128
#define GFX_USEDIBSECTION 256

#define MOUSE_ALWAYS_VISIBLE 0
#define MOUSE_FULLSCREEN_HIDDEN 1
#define MOUSE_ALWAYS_HIDDEN 2

#define MOUSEB_LEFT 1
#define MOUSEB_RIGHT 2
#define MOUSEB_MIDDLE 4

#define JOY_LEFT 1
#define JOY_RIGHT 2
#define JOY_UP 4
#define JOY_DOWN 8
#define JOY_FIRE1 16
#define JOY_FIRE2 32
#define JOY_FIRE3 64
#define JOY_FIRE4 128

#define LEFT 0
#define MIDDLE 128
#define RIGHT 255

#define B_OFF 0
#define B_SOLID 1
#define B_NOTSOLID 2

#define MONO 0
#define STEREO 1
#define EIGHTBIT 0
#define SIXTEENBIT 2

#define VM_OFF 0
#define VM_ON 1
#define VM_ONESHOT 0
#define VM_LOOP 2
#define VM_16BIT 4

#define KEY_BACKSPACE    SDLK_BACKSPACE
#define KEY_CAPSLOCK     SDLK_CAPSLOCK
#define KEY_ENTER        SDLK_RETURN
#define KEY_ESC          SDLK_ESCAPE
#define KEY_ALT          SDLK_LALT
#define KEY_CTRL         SDLK_LCTRL
#define KEY_LEFTCTRL     SDLK_LCTRL
#define KEY_RIGHTALT     SDLK_RALT
#define KEY_RIGHTCTRL    SDLK_RCTRL
#define KEY_LEFTSHIFT    SDLK_LSHIFT
#define KEY_RIGHTSHIFT   SDLK_RSHIFT
#define KEY_NUMLOCK      SDLK_NUMLOCK
#define KEY_SCROLLLOCK   SDLK_SCROLLOCK
#define KEY_SPACE        SDLK_SPACE
#define KEY_TAB          SDLK_TAB
#define KEY_F1           SDLK_F1
#define KEY_F2           SDLK_F2
#define KEY_F3           SDLK_F3
#define KEY_F4           SDLK_F4
#define KEY_F5           SDLK_F5
#define KEY_F6           SDLK_F6
#define KEY_F7           SDLK_F7
#define KEY_F8           SDLK_F8
#define KEY_F9           SDLK_F9
#define KEY_F10          SDLK_F10
#define KEY_F11          SDLK_F11
#define KEY_F12          SDLK_F12
#define KEY_A            SDLK_a
#define KEY_N            SDLK_n
#define KEY_B            SDLK_b
#define KEY_O            SDLK_o
#define KEY_C            SDLK_c
#define KEY_P            SDLK_p
#define KEY_D            SDLK_d
#define KEY_Q            SDLK_q
#define KEY_E            SDLK_e
#define KEY_R            SDLK_r
#define KEY_F            SDLK_f
#define KEY_S            SDLK_s
#define KEY_G            SDLK_g
#define KEY_T            SDLK_t
#define KEY_H            SDLK_h
#define KEY_U            SDLK_u
#define KEY_I            SDLK_i
#define KEY_V            SDLK_v
#define KEY_J            SDLK_j
#define KEY_W            SDLK_w
#define KEY_K            SDLK_k
#define KEY_X            SDLK_x
#define KEY_L            SDLK_l
#define KEY_Y            SDLK_y
#define KEY_M            SDLK_m
#define KEY_Z            SDLK_z
#define KEY_1            SDLK_1
#define KEY_2            SDLK_2
#define KEY_3            SDLK_3
#define KEY_4            SDLK_4
#define KEY_5            SDLK_5
#define KEY_6            SDLK_6
#define KEY_7            SDLK_7
#define KEY_8            SDLK_8
#define KEY_9            SDLK_9
#define KEY_0            SDLK_0
#define KEY_MINUS        SDLK_MINUS
#define KEY_EQUAL        SDLK_EQUALS
#define KEY_BRACKETL     SDLK_LEFTBRACKET
#define KEY_BRACKETR     SDLK_RIGHTBRACKET
#define KEY_SEMICOLON    SDLK_SEMICOLON
#define KEY_APOST1       SDLK_QUOTE
#define KEY_APOST2       SDLK_BACKQUOTE
#define KEY_COMMA        SDLK_COMMA
#define KEY_COLON        SDLK_PERIOD
#define KEY_PERIOD       SDLK_PERIOD
#define KEY_SLASH        SDLK_SLASH
#define KEY_BACKSLASH    SDLK_BACKSLASH
#define KEY_DEL          SDLK_DELETE
#define KEY_DOWN         SDLK_DOWN
#define KEY_END          SDLK_END
#define KEY_HOME         SDLK_HOME
#define KEY_INS          SDLK_INSERT
#define KEY_LEFT         SDLK_LEFT
#define KEY_PGDN         SDLK_PAGEDOWN
#define KEY_PGUP         SDLK_PAGEUP
#define KEY_RIGHT        SDLK_RIGHT
#define KEY_UP           SDLK_UP
#define KEY_WINDOWSL     SDLK_LSUPER
#define KEY_WINDOWSR     SDLK_RSUPER
#define KEY_MENU         SDLK_MENU
#define KEY_PAUSE        SDLK_PAUSE
#define KEY_KPDIVIDE     SDLK_KP_DIVIDE
#define KEY_KPMULTIPLY   SDLK_KP_MULTIPLY
#define KEY_KPPLUS       SDLK_KP_PLUS
#define KEY_KPMINUS      SDLK_KP_MINUS
#define KEY_KP0          SDLK_KP0
#define KEY_KP1          SDLK_KP1
#define KEY_KP2          SDLK_KP2
#define KEY_KP3          SDLK_KP3
#define KEY_KP4          SDLK_KP4
#define KEY_KP5          SDLK_KP5
#define KEY_KP6          SDLK_KP6
#define KEY_KP7          SDLK_KP7
#define KEY_KP8          SDLK_KP8
#define KEY_KP9          SDLK_KP9
#define KEY_KPUP         SDLK_KP8
#define KEY_KPDOWN       SDLK_KP2
#define KEY_KPLEFT       SDLK_KP4
#define KEY_KPRIGHT      SDLK_KP6
#define KEY_KPENTER      SDLK_KP_ENTER
#define KEY_KPEQUALS     SDLK_KP_EQUALS
#define KEY_KPPERIOD     SDLK_KP_PERIOD


typedef struct
{
	Sint8 *start;
	Sint8 *repeat;
	Sint8 *end;
	unsigned char voicemode;
} SAMPLE;

typedef struct
{
	volatile Sint8 *pos;
	Sint8 *repeat;
	Sint8 *end;
	SAMPLE *smp;
	unsigned freq;
	volatile unsigned fractpos;
	int vol;
	int mastervol;
	unsigned panning;
	volatile unsigned voicemode;
} CHANNEL;

typedef struct
{
  unsigned rawcode;
  char *name;
} KEY;

typedef struct
{
  Sint16 xsize;
  Sint16 ysize;
  Sint16 xhot;
  Sint16 yhot;
  Uint32 offset;
} SPRITEHEADER;

typedef struct
{
  Uint32 type;
  Uint32 offset;
} BLOCKHEADER;

typedef struct
{
  Uint8 blocksname[13];
  Uint8 palettename[13];
} MAPHEADER;

typedef struct
{
  Sint32 xsize;
  Sint32 ysize;
  Uint8 xdivisor;
  Uint8 ydivisor;
  Uint8 xwrap;
  Uint8 ywrap;
} LAYERHEADER;

extern int bme_error;
