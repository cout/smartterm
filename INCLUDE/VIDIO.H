#define NORMAL        0x7         /* Normal video attribute */
#define BOLD          0x8         /* Bold video attribute */
#define UNDERLINED    0xA         /* Underlined video attribute */
#define REVERSE       0x70        /* Reverse video attribute */
#define SCREEN        0x10        /* BIOS video interrupt number */
#define RETRACE       0x3da       /* Video Retrace port address for CGA */
#define ASCII         0           /* ASCII character set */
#define UK            1           /* UK character set */
#define SPECIAL       2           /* Special character set, graphics chars */

/****************************************************************************/
/* Function prototypes                                                      */

#define putch ChrWrite
#define gotoxy SetCurs

void VidInit( void );             /* Initialize the video system */
void SetVattr( unsigned char );   /* Set the video attribute */
void ChrWrite( unsigned char );   /* Write character to the screen */
void SetScroll(int , int );       /* Set the scrolling region */
void ScrollDown( void );          /* Move down a row scrolling if necessary */
void ScrollUp( void );            /* Move up a row scrolling if necessary */
void IndexDown( void );           /* Scroll the screen down */
void IndexUp( void );             /* Scroll the screen up */
void SetCurs(int , int );         /* Set the cursor to absolute coordinates */
void PosCurs( void );             /* Position the cursor to cursx,cursy */
void ClearScreen( void );         /* Clear the terminal screen */
void ClearEOS( void );            /* Clear from cursor to end of screen */
void ClearBOS( void );            /* Clear from cursor to top of screen */
void ClearEOL( void );            /* Clear from cursor to end of line */
void ClearBOL( void );            /* Clear from cursor to start of line */
void ClearBox( unsigned char,     /* Clear a box on the video screen */
   unsigned char, unsigned char,
   unsigned char, unsigned char);
void StartScreen( int, int );     /* Start a screen access */
void EndScreen( void );           /* End a screen access */
void WriteOneChar( unsigned char, /* Write one character to the screen */
     int, int );
int  VTprintf( int, int, int,     /* Printf for Emulator to row and column */
     char *, ... );                  /* in regular or reverse video */
void SaveScreen( void );          /* Save contents of video memory */
void RestoreScreen( void );       /* Restore contents of video memory */

static void interrupt10( unsigned,/* Issue a BIOS video interrupt */
  unsigned , unsigned , unsigned );


/****************************************************************************/
/* Global Data                                                              */

unsigned char retracemode = 0;    /* Flag indicating No Video refresh wait */
unsigned char forecolor;          /* Foreground color */
unsigned char backcolor;          /* Background color */

unsigned char vidmode;            /* Screen video mode */

/****************************************************************************/
/* External variables                                                       */

unsigned originmode;       /* Origin mode, relative or absolute */
unsigned insertmode;       /* Insert mode, off or on */
unsigned autowrap;         /* Automatic wrap mode, off or on */
unsigned newline;          /* Newline mode, off or on,  GLOBAL data!*/
unsigned cursorvisible;    /* Cursor visibility, on or hidden */
unsigned reversebackground;/* Reverse background attribute, on or off*/
unsigned screenwid;        /* Absolute screen width */

char modeline[80];           /* Text of mode line */

/***************************************************************************/
/* Local static data                                                       */

static unsigned char screentop;   /* Absolute top of screen */
static unsigned char screenbot;   /* Absolute bottom of screen */
static unsigned char scrolltop;   /* Top row of scrolling region */
static unsigned char scrollbot;   /* Bottom row of scrolling region */

static int cursx;                 /* X cursor position */
static int cursy;                 /* Y cursor position */

static unsigned scroff;           /* Screen memory offset */
static unsigned scrseg;           /* Screen memory segment */
static unsigned scrchars;         /* Number of chars written to video memory */
static unsigned char tvmode;      /* Flag to indicate control program present*/
char far *screen;                 /* Pointer to video screen */
static unsigned savebp;           /* Static storage for BP through int 0x10 */

static unsigned char video_state; /* State of video, reversed or normal */
static unsigned char scbattr;     /* Video attribute of empty video cell */
static unsigned char curattr;     /* Video attribute of displayable chars */
static unsigned char baseattr;    /* Base attribute for video attributes */
static unsigned char extrattr;    /* Extra attribute for video attributes */

static unsigned char att_reverse;          /* Reverse attribute bits */
static unsigned char att_normal;           /* Normal attribute bits */
static unsigned char att_low_mask  = 0x6;  /* Low attribute mask */
static unsigned char att_underline = 0x1;  /* Underlined attribute bit */
static unsigned char att_intensity = 0x8;  /* Bold attribute bit */
static unsigned char att_blink     = 0x80; /* Blinking attribute bit */


static unsigned columns;          /* Columns on logical terminal screen */
static unsigned lines;            /* Lines on logical terminal screen */

static char special_chars[32] = { /* Special characters */
   32,4,176,9,12,13,10,248,241,18,11,217,191,218,192,197,
   196,196,196,196,196,195,180,193,194,179,243,242,227,216,156,7
   } ;

static int G0=ASCII;

static struct SaveCursorStruct {  /* Structure to save cursor description */
   int cursx;                        /* X cursor position, column */
   int cursy;                        /* Y cursor position, row */
   int *GL;                          /* pointer to mapped character set */
   int G0;                           /* character set G0 */
   int G1;                           /* character set G1 */
   int mode;                         /* origin mode */
   } save = { 1, 1, &G0, ASCII, ASCII , 0 } ;

                                  /* Pointer to memory allocated to hold */
static char far *screen_save = NULL; /* the contents of video memory */

/****************************************************************************/
/****************************************************************************/


/* V I D I N I T  -- Initialize the video system */

void VidInit( ) {

    char far *bptr;
                                  /* Obtain video information from BIOS */
    _AH = 0x0F;                   /* Use function F of interrupt 10 */
    savebp = _BP;                 /* Precautionary save of register BP */
    geninterrupt( 0x10 );         /* Issue BIOS video interrupt */
    _BP = savebp;                 /* Restore saved BP register */
    vidmode = _AL;                /* Save the video mode */
    columns = _AH;                /* Save the number of columns */
    lines = 25;                   /* Lines = 25, (sorry no 43,50 lines */
    screenbot = lines - 1;        /* Bottom of screen is 24 */
    screentop = 1;                /* Top of screen is line 1 */

    tvmode = 0;                   /* Assume no control program present */


                                  /* First determine if snow is a problem */
       if (vidmode != 7)          /* Assume video adapter is snowy if */
            retracemode = 1;      /* it is not a MonoChrome */

                                  /* First query Video BIOS to see if */
        _AX = 0x1A00;             /* VGA is present, no "snow" problem on VGA*/
        savebp = _BP;             /* Precautionary save of register BP */
        geninterrupt( 0x10 );     /* Issue BIOS video interrupt */
        _BP = savebp;             /* Restore saved BP register */
        if (_AL == 0x1A)          /* If VGA is detected */
            retracemode = 0;         /* No snow protection needed */
        else {                    /* Else look for an EGA */
            _CL = 0xC;               /* Test the Video BIOS to see if */
            _BX = 0xFF10;            /*  an EGA can be detected */
            _AX = 0x1200;            /*  EGA's don't have "snow" problem either*/
            savebp = _BP;            /* Precautionary save of register BP */
            geninterrupt( 0x10 );    /* Issue BIOS video interrupt */
            _BP = savebp;            /* Restore saved BP register */
            if (_CL < 0xC) {         /* If EGA is detected */
                bptr = MK_FP(0x40,0x87);/* Check BIOS data to see if the */
                if ( (*bptr & 0x8) == 0)/* EGA is the active adapter */
                    retracemode = 0;    /* No snow protection required */
            }
        }

                                  /* Determine the default screen attributes*/
        _AH = 0x8;                /* Issue function 8 or interrupt 10 */
        _BH = 0x0;                /* for page 0 */
        savebp = _BP;             /* Precautionary save of register BP */
        geninterrupt( 0x10 );     /* Get video attribute at cursor pos */
        _BP = savebp;             /* Restore saved BP register */
        scbattr = _AH;            /* Save this attribute */

        forecolor = scbattr & 0xf;/* Save values for determined colors */
        backcolor = scbattr >> 4;

    if (screen_save == NULL) {    /* If first time to be initialized */
                                  /* Attempt to allocate screen mem */
       if ( (screen_save = malloc( lines * columns * 2 )) == NULL)
     	exit(1);
    }

}


/* S E T V A T T R  --  Set the video attribute */

void SetVattr(unsigned char attr) {
	curattr=attr;
}


/* C H R W R I T E  -- Write a character to a row and column of the screen */

void ChrWrite( unsigned char chr ) {
    unsigned char c[2], attr[2];
    int row, ws;

    /* NOTE:  Inserting a character using this technique is *very* slow      */
    /* for snowy CGA systems                                                 */
    if (insertmode) {             /* If insert mode, scoot rest of line over */
       StartScreen(cursy,cursx-1);/* Start direct video memory access        */

       c[0] = *screen;            /* Save character at current position      */
       attr[0] = *(screen + 1);   /* Save attribute at current position      */
       ws = 1;
       for (row = cursx; row < columns; row ++) {
 
           c[ws] = *(screen + 2); /* Save character at next position        */
           attr[ws] = *(screen + 3); /* Save attribute at next position     */
           ws ^= 1;               /* Flop save char,attribute array index   */
           *(screen + 2) = c[ws];  /* Write saved character and attribute   */
           *(screen + 3) = attr[ws];
           screen += 2;           /* Increment to next character position   */
           scrchars++;            /* Increment the number of chars written  */
       }
       EndScreen();               /* Update screen in control programs      */
    }

    if (cursx > screenwid) {      /* If trying to go beyond the screen width */
       if (autowrap) {               /* when autowrap is on */
          ScrollUp();                /* scroll the screen up */
          SetCurs(1,0);              /* set cursor to column 1 of next line */
       }
       else
          cursx = screenwid;         /* else put the cursor on right margin */
    }
 
    WriteOneChar(chr, cursy, cursx - 1);

    ++cursx;                      /* Increment the cursor X position */
    PosCurs();                    /* Move the cursor to the new position */

}


/* S E T S C R O L L  -- Establish the scrolling boundaries */

void SetScroll( register int top, register int bottom) {

    if (top == 0)                 /* If the top scroll boundary is 0 */
       top = 1;                     /* interpret this as the top screen row */
    if (bottom == 0)              /* If the bottom scroll boundary is 0 */
       bottom = screenbot;          /* interpret this as bottom screen row */
 
                                  /* Set scrolling region if valid coords */
    if (top > 0 && top <= screenbot && bottom >= top && bottom <= screenbot){
        scrolltop = top;            /* save top boundary */
        scrollbot = bottom;         /* save bottom boundary */
        SetCurs(1,1);               /* this also homes the cursor */
    }
}




/* S C R O L L D O W N  -- Move up a row scrolling if necessary */

void ScrollDown( )
{

   if (cursy == scrolltop)        /* If on the top of the scrolling region */
      IndexDown();                  /* scroll the rest of the region down */
   else {                         /* Else */
      --cursy;                      /* just decrement cursor Y position */
      PosCurs();                    /* and request the reposition */
   }
}

 
/* S C R O L L U P  -- Move down a row scrolling if necessary */

void ScrollUp( )
{

   if (cursy == scrollbot)        /* If on the bottom of the scrolling region*/
      IndexUp();                    /* scroll the rest of the region down */
   else {                         /* Else */
      ++cursy;                      /* just increment the cursor Y position */
      PosCurs();                    /* and request the reposition */
   }
}


/* I N D E X D O W N  -- Scroll the screen down */

void IndexDown( )
{
  register unsigned attr;

  attr = scbattr << 8;            /* Get the attribute for new line */
                                  /* Call the BIOS to scroll the region */
  interrupt10( 0x0701,attr,scrolltop << 8,(scrollbot << 8) | (columns - 1) );
  PosCurs();                      /* Position the cursor */
}

/* I N D E X U P  -- Scroll the screen up */

void IndexUp( )
{
  register unsigned attr;

  attr = scbattr << 8;            /* Get the attribute for new line */
                                  /* Call the BIOS to scroll the region */
  interrupt10( 0x0601,attr,scrolltop << 8,(scrollbot << 8) | (columns - 1));
  PosCurs();                      /* Position the cursor */
}
 

/* S E T C U R S -- Set absolute cursor position on the logical screen */

void SetCurs(register int col, register int row)
{
   if (col == 0)                  /* If called with X coordinate = zero */
      col = cursx;                  /* then default to current coordinate */
   if (row == 0)                  /* If called with Y coordinate = zero */
      row = cursy;                  /* then default to current coordinate */

   if (originmode) {              /* If origin mode is relative */
      row += (scrolltop - 1);     /* adjust the row */
     if (row < scrolltop || row > scrollbot)
          return;                 /* Can not position cursor out of scroll */
                                  /* region in relative cursor mode */
   }
                                  /* Can only position the cursor if it lies */
                                  /* within the logical screen limits */
   if (col <= screenwid && row <= screenbot ) {
       cursx = col;               /* Set the X cursor coordinate, column */
       cursy = row;               /* Set the Y cursor coordinate, row */
       PosCurs();                 /* Request the physical positioning */
   }
}


/* S E T R E L C U R S -- Set relative curs pos on the logical screen */

void SetRelCurs(register int col, register int row)
{

   if (col == 0)                  /* If called with X coordinate = zero */
      col = cursx;                  /* then default to current X coordinate */
   else                           /* Else */
      col = cursx + col;            /* add col value to X cursor position */
                                    /* Note:  col can be negative */

   if (row == 0)                  /* If called with Y coordinate = zero */
      row = cursy;                  /* then default to current Y coordinate */
   else                           /* Else */
      row = cursy + row;            /* add row value to Y cursor position */
                                    /* Note:  row can be negative */

   if (originmode) {               /* If origin mode is relative */
      row += (scrolltop - 1);      /* adjust the row */
      if (row < scrolltop || row > scrollbot)
          return;                  /* Can not position cursor out of scroll */
   }                               /* region in relative cursor mode */

                                   /* Can only position the cursor if it lies */
                                   /* within the logical screen limits */
   if (col > 0 && col <= screenwid && row > 0 && row <= screenbot){
       cursy = row;                /* Set the X cursor coordinate, column */
       cursx = col;                /* Set the Y cursor coordinate, row */
       PosCurs();                  /* Request the physical positioning */
   }
}

/* P O S C U R S -- Position the cursor on the physical screen */

void PosCurs()
{
   register int col = cursx;
   register int row = cursy;

   if (col <= columns)            /* Check validity of requested column */
      ;                             /* ok if column is within screen bounds */
   else
      col = columns;                /* else put cursor on the right bound */

   if (row <= lines)              /* Check validity of requested row */
      ;                             /* ok if row is within screen bounds */
   else
      row = lines;                  /* else put cursor on the bottom */

    if (cursorvisible)            /* Only position the cursor if its visible */
       interrupt10(0x0200,0,0,(row << 8) | --col );
}



/* C L E A R S C R E E N -- Clear the screen setting it to a normal attr */

void ClearScreen()
{
    ClearBox(1, screentop, columns, screenbot, scbattr);
}
 
/* C L E A R E O S -- Clear from the cursor to the end of screen */

void ClearEOS()
{
    ClearEOL();                   /* First clear to the End of the Line */
    if (cursy < screenbot)        /* Then clear every line below it */
       clearbox(1,cursy + 1,columns, screenbot, scbattr);
}

/* C L E A R B O S -- Clear from the cursor to the beggining of screen */

void ClearBOS()
{
    ClearBOL();                   /* First clear to the Beginning of the Line */
    if (cursy > screentop)        /* Then clear every line above it */
       ClearBox(1,screentop,columns, cursy - 1, scbattr);
}
 
/* C L E A R E O L -- Clear to the end of the current line */

void ClearEOL()
{
   ClearBox(cursx, cursy, columns, cursy, scbattr );
}


/* C L E A R B O L -- Clear to the beginning of the current line */

void ClearBOL()
{
   ClearBOX(1, cursy, cursx, cursy, scbattr );
}


/* C L E A R B O X -- Clear a window on the screen with the specified attr */

void ClearBox( unsigned char left, unsigned char top,
unsigned char right, unsigned char bottom, unsigned char attr )
{
                                  /* Use BIOS scroll window function to clear*/
    interrupt10( 0x0600,attr << 8,  /* a window to a specified attribute */
       (top << 8) | (--left),(bottom << 8) | --right);
}


/* S A V E C U R S O R  --  Save the cursor description into memory */

void SaveCursor() {

   save.cursx = cursx;              /* Save the X cursor position */
   save.cursy = cursy;              /* Save the Y cursor position */
   save.G0 = G0;                    /* Save G0 character set */
   save.mode = originmode;          /* Also save the origin mode */
}


/* R E S T O R E C U R S O R  --  Restore the cursor description from memory */

void RestoreCursor() {

   cursx = save.cursx;              /* Restore the saved X cursor position */
   cursy = save.cursy;              /* Restore the saved Y cursor position */
   G0 = save.G0;                    /* Restore the saved G0 character set */
   originmode = save.mode;          /* Also restore the saved origin mode */
   PosCurs();                       /* Then reposition the cursor */
}


/* S E T C U R S O R V I S I B I L I T Y -- Show/Hide the cursor */

void SetCursorVisibility( int mode ) {

   if (mode) {                    /* If invisible cursor is specified, then */
      cursorvisible = 1;            /* the cursor will not appear on the */
      SetCurs(0,0);                 /* terminal screen */
   }
   else {                         /* Else the cursor will be shown at the */
      cursorvisible = 0;            /* current cursor position */
      interrupt10(0x0200,0,0,lines << 8 );
   }
}


/* S T A R T S C R E E N -- Start a direct screen addressing "transaction" */

void StartScreen( register int row, register int col ) {
    unsigned vscroff, vscrseg;

    if ( vidmode == 7 ) {         /* If in Monochrome video mode     */
        scrseg = 0xb000;            /* Assume video segment of B000 */
    }
    else {                        /* If other mode then assumme */
        scrseg = 0xb800;            /* video memory segment is B800 */
    }

    scroff =((row*columns)+col)*2;/* Calculate offset from beginning of */
                                    /* screen memory */
    scrchars = 0;                 /* Initialize count of characters written */

    savebp = _BP;                 /* Save reg BP, in case old BIOS trashes it */
                                  /* Query the address of video memory for */
    _ES = scrseg;                 /* this process under Windows, DesQview, ... */
    _DI = 0;                      /* ES:DI = assumed screen address */
    _AH = 0xFE;                   /* Issue Video interrupt function FE */
    geninterrupt(0x10);           /*  in order to check for control program */
    vscroff = _DI;                /* ES:DI now holds actual address of screen*/
    vscrseg = _ES;
    _BP = savebp;                 /* Restore register BP */

    tvmode = 0;                   /* Assume no control program */
    if (vscroff != 0 || vscrseg != scrseg) {
        scrseg = vscrseg;           /* If address of screen is different from*/
        scroff += vscroff;          /* assumed address then a control program*/
        tvmode = 1;                 /* is present and has remapped the screen*/
    }
    else if (retracemode == 0)
        ;                         /* No refresh wait for these situations */
    else {
                                  /* Wait till screen is refreshing */
        while( (inportb(RETRACE) & 0x8) != 0 )
            ;                     /* Wait till refresh is through */
        while( (inportb(RETRACE) & 0x8) == 0 )
            ;
        outportb(0x3d8,0x25);     /* Turn off the screen refresh */
    }
                                  /* Construct a far pointer to video memory*/
    (void far *)screen = MK_FP(scrseg,scroff);

}


/* E N D S C R E E N -- End a direct screen addressing "transaction" */

void EndScreen( ) {
                                  /* Values to turn the screen back on */
    static unsigned char modeset[] = /* after video refresh is turned off */
        { 0x2C,0x28,0x2D,0x29,0x2A,0x2E,0x1E,0x29,
    } ;
                                  /* If control program video update required*/
    if (tvmode == 1 && scrchars > 0) {
        _ES = scrseg;             /* Point ES:DI to beginning of video memory*/
        _DI = scroff;             /*  to update */
        _CX = scrchars;           /* CX holds the number of chars to update */
        _AH = 0xFF;               /* Use function FF of BIOS video interrupt */
        geninterrupt(0x10);       /*  to copy video buffer to screen */
    }
    else if (retracemode == 0)
        ;                         /* Situations screen was not turned off */
    else {                        /* Else if screen is off turn it back on */
        outportb(0x3d8, modeset[vidmode]);
    }
}


/* W R I T E O N E C H A R -- writes one character to video memory      */
/* NOTE: this function does not shut down video refresh in order       */
/* to avoid snow on CGA, so it is *much* faster than using StartScreen */
/* and an EndScreen transaction for one character screen writes.       */

void WriteOneChar(unsigned char c, register int row, register int col ) {
    unsigned vscroff, vscrseg;

    if ( vidmode == 7 ) {         /* If in Monochrome video mode     */
        scrseg = 0xb000;            /* Assume video segment of B000 */
    }
    else {                        /* If other mode then assumme */
        scrseg = 0xb800;            /* video memory segment is B800 */
    }

    scroff =((row*columns)+col)*2;/* Calculate offset from beginning of */
                                    /* screen memory */

				  /* Construct a far pointer to video memory*/
    (void far *)screen = MK_FP(scrseg,scroff);

    if ( retracemode != 0 ) {    /* If no snow then */
        *screen++ = c;                /* write character into screen memory */
        *screen = curattr;            /* write attribute into screen memory */
    }
    else {                        /* Else if snowy CGA then try and sneak */
        if (inportb(RETRACE) & 0x8) { /* the character and attribute into */
            *screen++ = c;            /* video memory in between video */
            *screen = curattr;        /* refreshes */
        }
        else {
            while (inportb(RETRACE) & 0x8)
                ;
            while (inportb(RETRACE) & 0x1)
                ;
            *screen++ = c;
            *screen = curattr;
        }
    }

}


/* S A V E S C R E E N -- Save the contents of the video screen */

void SaveScreen() {

    StartScreen(0,0);             /* Start the video direct access */
    movedata(scrseg, scroff,      /* Move the video memory to save buffer */
       FP_SEG(screen_save), FP_OFF(screen_save),
       lines * columns * 2 );
    scrchars = 0;                 /* Zero characters were written to screen */
    EndScreen();                  /* End the screen access */
    interrupt10(0x0200,0,0,lines << 8 ); /* Hide the cursor */
}

/* R E S T O R E S C R E E N -- Restore contents of the video screen */

void RestoreScreen() {

    StartScreen(0,0);             /* Start the video direct access */
                                  /* Move the save buffer to video memory */
    movedata(FP_SEG(screen_save), FP_OFF(screen_save),
       scrseg, scroff,
       lines * columns * 2 );
    scrchars = lines * columns;   /* Number of chars written to screen */
    EndScreen();                  /* End the screen access */
    PosCurs();                    /* Reposition the cursor */
}


/* V T P R I N T F -- print a formatted string to the video screen */

int vtprintf( int row, int col, int reverse, char *strformat, ... )
{
    unsigned attr;
    va_list argptr;
    char str[132];
    char *sptr = str;
    int it;

    if (reverse) {                /* If reversed attribute specified */
        attr = att_reverse;
    }
    else {                        /* Else use normal attribute */
        attr = att_normal;
    }

    va_start( argptr, format );   /* Start variable length argument list */
    StartScreen(row,col);         /* Start a screen update */
    scrchars = vsprintf( str, strformat, argptr ); /* Format the string */
    while (*sptr != '\0') {       /* Move the formatted string */
        *screen++ = *sptr++;         /* to video memory */
        *screen++ = attr;
    }
    EndScreen();                  /* End the screen update */
    va_end(argptr);               /* End the va_start */
    return(scrchars);             /* Return number of characters written */
}



/* I N T E R R U P T 1 0  -- Call on the BIOS video software interrupt */

void interrupt10(unsigned ax,unsigned bx,unsigned cx,unsigned dx){
   register unsigned savesi;      /* Force compiler to save contents of */
   register unsigned savedi;      /* SI and DI registers on the stack */
   static unsigned savebp;
 
   savebp = _BP;                  /* Save BP since some BIOS'es may destroy it*/
   _CX = cx;                      /* Load contents of register parameters */
   _DX = dx;
   _BX = bx;
   _AX = ax;
   geninterrupt(0x10);            /* Issue Video interrupt */
   _BP = savebp;                  /* Restore BP register */
}
