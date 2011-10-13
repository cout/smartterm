#define PGUP        0x4900        /* Grey Page Up key */
#define PGDN        0x5100        /* Grey Page Down key */
#define HOME        0x4700        /* Greyu Home key */
#define END         0x4F00        /* Grey End key */
#define INS         0x5200        /* Grey Insert Key */
#define ESC         0x1B          /* ASCII ESCape character */
#define ESCAPE      0x11B         /* Keyboard ESCape scan code */
#define DEL         0x7F          /* ASCII DELete character */
#define BKSP        0xE08         /* Keyboard BacKSPace scan code */
#define F1          0x3B00        /* Keyboard Function key 1 scan code */
#define F2          0x3C00        /* Keyboard Function key 2 scan code */
#define F3          0x3D00        /* Keyboard Function key 3 scan code */
#define F4          0x3E00        /* Keyboard Function key 4 scan code */
#define F5          0x3F00        /* Keyboard Function key 5 scan code */
#define F6          0x4000        /* Keyboard Function key 6 scan code */
#define F7          0x4100        /* Keyboard Function key 7 scan code */
#define F8          0x4200        /* Keyboard Function key 8 scan code */
#define F9          0x4300        /* Keyboard Function key 9 scan code */
#define F10         0x4400        /* Keyboard Function key 10 scan code */
#define UP          0x4800        /* Keyboard Up Arrow scan code */
#define DOWN        0x5000        /* Keyboard Down Arrow scan code */
#define LEFT        0x4B00        /* Keyboard Left Arrow scan code */
#define RIGHT       0x4D00        /* Keyboard Right Arrow scan code */
#define K7          0x4737        /* Keyboard Numeric 7 scan code */
#define K8          0x4838        /* Keyboard Numeric 8 scan code */
#define K9          0x4939        /* Keyboard Numeric 9 scan code */
#define KDASH       0x372A        /* Keyboard Numeric Asteric scan code */
#define K4          0x4B34        /* Keyboard Numeric 4 scan code */
#define K5          0x4C35        /* Keyboard Numeric 5 scan code */
#define K6          0x4D36        /* Keyboard Numeric 6 scan code */
#define KCOMA       0x4A2D        /* Keyboard Numeric Dash(minus) scan code */
#define K1          0x4F31        /* Keyboard Numeric 1 scan code */
#define K2          0x5032        /* Keyboard Numeric 2 scan code */
#define K3          0x5133        /* Keyboard Numeric 3 scan code */
#define KENTR       0x4E2B        /* Keyboard Numeric + (plus) scan code */
#define K0          0x5230        /* Keyboard Numeric 0 scan code */
#define KDOT        0x532E        /* Keyboard Numeric Period scan code */
#define ALTQ        0x1000	  /* Alt-Q */
#define ALTR        0x1300
#define ALTA        0x1E00
#define ALTZ        0x2C00
#define ALTX        0x2D00
#define ALTC        0x2E00
#define ALTT        0x1400
#define ALTO        0x1800

/*****************************************************************************/
/* function prototypes                                                       */

void         Keyinit( void );     /* Initialize the keyboard system */
int          ConChk( void );      /* Check the keyboard for keystrokes */
unsigned int GetKey( void );      /* Retrieve a scan code from keyboard */
int          DoKey( void );       /* Transmit a key sequence */

static void SendBksp();
static void TransKey( unsigned ); /* Translate a keystroke to a sequence */
static int TransNumKey(unsigned); /* Translate Numeric Keypad keystroke */
static int TransApplKey(unsigned);/* Translate Application Keypad keystroke */
static void SendBksp( void );     /* Transmit the backspace character */
void   SetKeyPad( int );          /* Set the keypad to APPLICATION, NUMERIC */
void   SetKey( int );             /* Set the cursor key mode */




/*****************************************************************************/
/* Global variables                                                          */

unsigned char backspace;          /* Backspace/Delete translation flag */
unsigned char keyclick;           /* Keyclick mode on/off */
unsigned char applkeypad;         /* Current state of keypad */

#define ttoc writecomm

/*****************************************************************************/
/* External variables                                                        */


/*****************************************************************************/
/* Local Static data                                                         */

static char cursorkey = '[';      /* Sequence character in cursor key */
static union REGS regs;           /* Registers for int86 call */

/*****************************************************************************/
/*****************************************************************************/

/*  G E T K E Y  --  Return a keyboard scan code */

unsigned int GetKey() {
    register unsigned int scancode;

    scancode = bioskey(0);        /* Get a keystroke, waits if none ready */

    if (keyclick) {               /* If keyclick flag is set */
        sound(250);                  /* Turn on low frequency sound */
        delay(5);                    /* Wait a short time period */
        nosound();                   /* Turn off the sound */
    }
    return(scancode);             /* Return the retrieved scancode */
}

/*  D O K E Y  --  Retrieve and interpret a keystroke */

DoKey() {
    unsigned scancode;

    scancode = GetKey();          /* Get a keystroke, waits if none ready */

    if ((scancode == F10)||(scancode==ALTX))  /* Check for EXIT character */
       return(-1);
    else                                /* Else translate the pressed key */
       TransKey(scancode);

    return(0);                                          /* return success */
}


/* T R A N S K E Y  -- translate a scancode into a keystroke sequence */

static void TransKey( unsigned key ) {


    switch(key) {                 /* Evaluate this keyboard scan code */
       case BKSP:                 /* Backspace pressed */
            SendBksp();
            break;
       case F1:                   /* Function key 1 pressed */
           ttoc( ESC );
           ttoc( 'P' );
           break;
       case F2:                   /* Function key 2 pressed */
           ttoc( ESC );
           ttoc( 'Q' );
           break;
       case F3:                   /* Function key 3 pressed */
           ttoc( ESC );
           ttoc( 'R' );
           break;
       case F4:                   /* Function key 4 pressed */
           ttoc( ESC );
           ttoc( 'S' );
           break;
       case UP:                   /* Up Arrow pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('A');
           break;
       case DOWN:                 /* Down Arrow pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('B');
           break;
       case RIGHT:                /* Right Arrow pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('C');
           break;
       case LEFT:                 /* Left Arrow pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('D');
           break;
       case PGUP:                 /* Page Up pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('5');
	   ttoc('~');
           break;
       case PGDN:                 /* Page Down pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('6');
	   ttoc('~');
           break;
       case HOME:                 /* Home pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('1');
	   ttoc('~');
           break;
       case END:                 /* Delete pressed */
           ttoc(ESC);
           ttoc(cursorkey);
	   ttoc('4');
	   ttoc('~');
           break;
       case INS:                 /* Insert pressed */
           ttoc(ESC);
           ttoc(cursorkey);
           ttoc('2');
	   ttoc('~');
           break;
       case ALTC:
           cls();
	   break;
       case ALTO:
	   SaveScreen();
	   window(1,0,80,28);
	   cls();
	   system("");
	   RestoreScreen();
	   break;
       case ALTR:
           SaveScreen();
           DispLog();
           RestoreScreen();
	   break;

       default:                   /* No translation yet, check numeric pad */
           if ( (TransNumKey(key) == 0 ) && (TransApplKey(key) == 0) )
              ttoc( (char)key );  /* Still no translation, transmit char */
           break;
    }
}

/* T R A N S N U M K E Y  --  Try and translate key from the Numeric Keypad */

static TransNumKey(register unsigned key) {

   if (applkeypad != 0)           /* If keypad is not in NUMERIC mode */
      return(0);                    /* then no translation here possible */

   switch ( key ) {
      case K7:                    /* Numeric 7 pressed */
         ttoc('7');
         return(1);
      case K8:                    /* Numeric 8 pressed */
         ttoc('8');
         return(1);
      case K9:                    /* Numeric 9 pressed */
         ttoc('9');
         return(1);
      case KDASH:                 /* Numeric Minus pressed */
         ttoc('-');
         return(1);
      case K4:                    /* Numeric 4 pressed */
         ttoc('4');
         return(1);
      case K5:                    /* Numeric 5 pressed */
         ttoc('5');
         return(1);
      case K6:                    /* Numeric 6 pressed */
         ttoc('6');
         return(1);
      case KCOMA:                 /* Numeric Comma pressed */
         ttoc(',');
         return(1);
      case K1:                    /* Numeric 1 pressed */
         ttoc('1');
         return(1);
      case K2:                    /* Numeric 2 pressed */
         ttoc('2');
         return(1);
      case K3:                    /* Numeric 3 pressed */
         ttoc('3');
         return(1);
      case K0:                    /* Numeric 0 pressed */
         ttoc('0');
         return(1);
      case KDOT:                  /* Numeric Period pressed */
         ttoc('.');
         return(1);
      case KENTR:                 /* Numeric Enter pressed */
         ttoc(13);
         return(1);
      default:
	 break;
   }
   return(0);                     /* No translation */
}

/* T R A N S A P P L K E Y  --  Try and translate key from Application Keypad*/

static TransApplKey(register unsigned key) {

   if (applkeypad != 1)           /* If keypad is not APPLICATION mode */
      return(0);                    /* then no translation here possible */

   switch ( key )  {
       case K7:                   /* Application key 7 pressed*/
           ttoc(ESC);
           ttoc('O');
           ttoc('w');
           return(1);
       case K8:                   /* Application key 8 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('x');
           return(1);
       case K9:                   /* Application key 9 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('y');
           return(1);
       case KDASH:                /* Application key minus pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('m');
           return(1);
       case K4:                   /* Application key 4 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('t');
           return(1);
       case K5:                   /* Application key 5 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('u');
           return(1);
       case K6:                   /* Application key 6 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('v');
           return(1);
       case KCOMA:                /* Application key Comma pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('l');
           return(1);
       case K1:                   /* Application key 1 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('q');
           return(1);
       case K2:                   /* Application key 2 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('r');
           return(1);
       case K3:                   /* Application key 3 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('s');
           return(1);
       case K0:                   /* Application key 0 pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('p');
           return(1);
       case KDOT:                 /* Application key Dot pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('n');
           return(1);
       case KENTR:                /* Application key Enter pressed */
           ttoc(ESC);
           ttoc('O');
           ttoc('M');
           return(1);
       default:
	   break;
   }
   return(0);                     /* No translation */
}

/* S E T K E Y P A D -- Set the keypad translation */

void SetKeyPad( int mode ) {

   if (mode)                      /* If a mode set is requested */
      applkeypad = 1;               /* set the keypad to APPLICATION */
   else                           /* Else */
      applkeypad = 0;               /* set the keypad to NUMERIC mode*/
}

/* S E T C U R S O R K E Y -- Set the cursior key mode */

void SetCursorKey( mode ) {       /* This establishes the second character */
                                  /* of the cursor keys escape sequence */
   if (mode)                      /* If cursor key mode is requested */
      cursorkey = 'O';              /* then set cursor key to 'O' */
   else                           /* Else */
      cursorkey = '[';              /* use a bracket for escape sequences */
}

