/* Escape deliminator */
#define ESC 0x1B

/* ANSI escape sequences */
#define CURSOR_POSITION       'H'
#define CURSOR_POSITION_ALT   'f'     /* equivalent to 'H' */
#define CURSOR_UP             'A'
#define CURSOR_DOWN           'B'
#define CURSOR_FORWARD        'C'
#define CURSOR_BACKWARD       'D'
#define SAVE_CURSOR_POS       's'
#define RESTORE_CURSOR_POS    'u'
#define ERASE_DISPLAY         'J'     /* 2J */
#define ERASE_TO_EOL          'K'
#define SET_GRAPHICS_MODE     'm'
#define SET_MODE              'h'
#define RESET_MODE            'l'
#define SET_KEYBOARD_STRINGS  'p'

/* Text attributes */
#define ALL_ATTRIBUTES_OFF 0
#define BOLD_ON            1
#define UNDERSCORE         4
#define BLINK_ON           5
#define REVERSE_VIDEO_ON   7
#define CONCEALED_ON       8

/* Foreground colors */
#define FG_BLACK    30
#define FG_RED      31
#define FG_GREEN    32
#define FG_YELLOW   33
#define FG_BLUE     34
#define FG_MAGENTA  35
#define FG_CYAN     36
#define FG_WHITE    37

/* Background colors */
#define BG_BLACK    40
#define BG_RED      41
#define BG_GREEN    42
#define BG_YELLOW   43
#define BG_BLUE     44
#define BG_MAGENTA  45
#define BG_CYAN     46
#define BG_WHITE    47
