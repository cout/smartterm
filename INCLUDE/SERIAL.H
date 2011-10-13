/* ---------- serial.h ---------------
 * Serial Port Definitions
 */
extern int ticker, COMPORT;
extern char *nextin, *nextout;
/* ----------- serial prototypes ----------- */
void initcomport(void);
int readcomm(void);
int writecomm(int);
void clear_serial_queue(void);
/* -------- timer prototypes --------- */
void sleep(unsigned);
int set_timer(unsigned);
void intercept_timer(void);
void restore_timer(void);
void restore_serialint(void);
/* ----------------- macros ------------------- */
#define comstat() (inp(LINESTATUS))
#define input_char_ready() (nextin!=nextout)
#define timed_out() (ticker==0)
#define set_timer(secs) ticker=secs*182/10+1
#define XON  17
#define XOFF 19
/* ---------------- serial port addresses ----------------- */
/* - 8250 UART base port address:  COM1 = 3f8, COM2 = 2f8 - */
#define BASEPORT    (0x3f8-((COMPORT-1)<<8))
#define TXDATA       BASEPORT      /* transmit data         */
#define RXDATA       BASEPORT      /* receive data          */
#define DIVLSB       BASEPORT      /* baud rate divisor lsb */
#define DIVMSB      (BASEPORT+1)   /* baud rate divisor msb */
#define INTENABLE   (BASEPORT+1)   /* interrupt enable      */
#define INTIDENT    (BASEPORT+2)   /* interrupt ident'n     */
#define LINECTL     (BASEPORT+3)   /* line control          */
#define MODEMCTL    (BASEPORT+4)   /* modem control         */
#define LINESTATUS  (BASEPORT+5)   /* line status           */
#define MODEMSTATUS (BASEPORT+6)   /* modem status          */
 /* --------------- serial interrupt stuff ------------------ */
#define IRQ     (4-(COMPORT-1))     /* 0-7 = IRQ0-IRQ7       */
#define COMINT  (12-(COMPORT-1))    /* interrupt vector 12/11*/
#define COMIRQ  (~(1 << IRQ))
#define PIC01   0x21 /*8259 Programmable Interrupt Controller*/
#define PIC00   0x20 /* "      "              "        "     */
#define EOI     0x20 /* End of Interrupt command             */
#define TIMER   0x1c /* PC timer interrupt vector            */
 /* --------------- line status register values ------------- */
#define XMIT_DATA_READY    0x20
 /* ------------ modem control register values -------------- */
#define DTR   1
#define RTS   2
#define OUT2  8
 /* ----------- interrupt enable register signals ------------ */
#define DATAREADY 1
 /* ------------- serial input interrupt buffer -------------- */
#define BUFSIZE 16768
#define SAFETYLEVEL (BUFSIZE/4)
#define THRESHOLD (SAFETYLEVEL*3)
#ifndef true
#define true 1
#define false 0
#endif

#if COMPILER == MSOFT
#define _dos_getvect getvect
#define _dos_setvect setvect
#endif

char recvbuff[BUFSIZE];
char *nextin = recvbuff;
char *nextout = recvbuff;
int buffer_count;
int COMPORT  = 1;    /* COM1 or COM2                  */
int PARITY   = 0;    /* 0 = none, 1 = odd, 2 = even   */
int STOPBITS = 1;    /* 1 or 2                        */
int WORDLEN  = 8;    /* 7 or 8                        */
unsigned long BAUD     = 38400; /* 110,150,300,600,1200,2400     */
int TIMEOUT  = 10;   /* number of seconds to time out */
int xonxoff_enabled = true;
static int waiting_for_XON;
static int waiting_to_send_XON;
int ticker;

/* ----- the com port initialization parameter byte ------ */
static union    {
    struct {
        unsigned wordlen  : 2;
        unsigned stopbits : 1;
        unsigned parity   : 3;
        unsigned brk      : 1;
        unsigned divlatch : 1;
    } initbits;
    char initchar;
} initcom;
static void (interrupt far *oldtimer)(void);
static void interrupt far newtimer(void);
static void (interrupt far *oldcomint)(void);
static void interrupt far newcomint(void);

/* -------- initialize the com port ----------- */
void initcomport(void)
{
    initcom.initbits.parity   = PARITY == 2 ? 3 : PARITY;
    initcom.initbits.stopbits = STOPBITS-1;
    initcom.initbits.wordlen  = WORDLEN-5;
    initcom.initbits.brk      = 0;
    initcom.initbits.divlatch = 1;
    outp(LINECTL, initcom.initchar);
    outp(DIVLSB, (char) ((115200L/BAUD) & 255));
    outp(DIVMSB, (char) ((115200L/BAUD) >> 8));
    initcom.initbits.divlatch = 0;
    outp(LINECTL, initcom.initchar);
/* ------ hook serial interrupt vector --------- */
    if (oldcomint == NULL)
        oldcomint = getvect(COMINT);
    setvect(COMINT, newcomint);
    outp(MODEMCTL, (inp(MODEMCTL) | DTR | RTS | OUT2));
    outp(PIC01, (inp(PIC01) & COMIRQ));
    outp(INTENABLE, DATAREADY);
    outp(PIC00, EOI);
/* ----- flush any old interrupts ------ */
    inp(RXDATA);
    inp(INTIDENT);
    inp(LINESTATUS);
    inp(MODEMSTATUS);
}

/* ------ restore the serial interrupt vector ---------- */
void restore_serialint(void)
{
    if (oldcomint)
        setvect(COMINT, oldcomint);
}

/* ------- clear the serial input buffer --------- */
void clear_serial_queue(void)
{
    nextin = nextout = recvbuff;
    buffer_count = 0;
}

/* ---- serial input interrupt service routine ------- */
static void interrupt far newcomint(void)
{
    int c;
    outp(PIC00,EOI);
    if (nextin == recvbuff+BUFSIZE)
        nextin = recvbuff;           /* circular buffer */
    c = inp(RXDATA);              /* read the input  */
    if (xonxoff_enabled)
        if (c == XOFF)               /* test XON        */
            waiting_for_XON = 1;
        else if (c == XON)           /* test XOFF       */
            waiting_for_XON = 0;
    if (!xonxoff_enabled || (c != XON && c != XOFF))    {
        *nextin++ = (char) c;        /* put char in buff*/
        buffer_count++;
    }
    if (xonxoff_enabled && !waiting_to_send_XON &&
            buffer_count > THRESHOLD)    {
        while ((inp(LINESTATUS) & XMIT_DATA_READY) == 0)
            ;
        outp(TXDATA, XOFF);          /* send XOFF        */
        waiting_to_send_XON = 1;
    }
}

int end_serial(void) {
	restore_serialint();
	exit(0);
}

/* ---- read a character from the input buffer ----- */
int readcomm(void)
{
    int old_min=-1;
    struct time curr_time;
    char time_buf[10];
    char ampm;
    register int j;
    set_timer(TIMEOUT);
    while (!input_char_ready()) {
	if(kbhit())
	    if(DoKey()==-1) end_serial();
	gettime(&curr_time);
	if(old_min!=curr_time.ti_min) {
	    if(curr_time.ti_hour>11) {
		ampm='p';
		curr_time.ti_hour-=12;
	    } else {
		ampm='a';
	    }
	    sprintf(time_buf," %2d:%02d%c ",curr_time.ti_hour,
		curr_time.ti_min, ampm);
	    if(color_flag==1) ScreenMemory=MK_FP(0xb800,142);
	    else ScreenMemory=MK_FP(0xb000,138);
	    for(j=0;j<8;j++) {
		*ScreenMemory++=time_buf[j];
		*ScreenMemory++=26;
	    }
	    old_min=curr_time.ti_min;
    }
	if (timed_out())
	    return false;
	}
    if (nextout == recvbuff+BUFSIZE)
        nextout = recvbuff;
    --buffer_count;
    if (waiting_to_send_XON && buffer_count < SAFETYLEVEL) {
        waiting_to_send_XON = 0;
        writecomm(XON);
    }
    return *nextout++;
}

/* ---- write a character to the comm port ----- */
int writecomm(int c)
{
    while (waiting_for_XON)
        ;
    set_timer(TIMEOUT);
    while ((inp(LINESTATUS) & XMIT_DATA_READY) == 0)
        if (timed_out())
            return false;
    outp(TXDATA, c);
    return true;
}

/* ---- intercept the timer interrupt vector ----- */
void intercept_timer(void)
{
    if (oldtimer == NULL)    {
        oldtimer = getvect(TIMER);
        setvect(TIMER, newtimer);
    }
}

/* ---------- sleep for n seconds ------------ */
void sleep(unsigned secs)
{
    set_timer(secs);
    while (!timed_out())
        ;
}

/* ---- restore timer interrupt vector ------- */
void restore_timer()
{
    if (oldtimer)
        setvect(TIMER, oldtimer);
}

/* ------ ISR to count timer ticks ------- */
static void interrupt far newtimer()
{
    (*oldtimer)();
    if (ticker)
        --ticker;
}
