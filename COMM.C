/*
 * A generic ANSI interpreter by Saeger  (aka: Jason Farrell)
 * (c) 1995, Saeger
 */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <ctype.h>
#include <string.h>
#include <conio.h>
#include <alloc.h>

int top_scroll=1, bottom_scroll=28;
char color_flag=1;

/*#define putch(c) directvideo=0; putch(c); directvideo=1
  #define cputs(s) directvideo=0; cputs(s); directvideo=1*/

#include "include\set28.h"
#include "include\direvid.h"
#include "include\vttio.h"
#include "include\log.h"
#include "include\keyio.h"
#include "include\ansicode.h"
#include "include\clrattr.h"
#include "include\serial.h"

#define rch readcomm
#define setup initcomport

#define SCREENBOTTOM 28

#define NUM_LINES   28
#define MAX_PARAMS  10

enum { FALSE, TRUE };
enum { progname, ansifile };

unsigned char Delete_Flag=0;
char Prompts[20][80];
char pctr=0, NL_Flag=0;
/*char TempStr[80];*/

static void SendBksp() {
	if((Delete_Flag==255)||(color_flag==0)) {
		ttoc(127);
	} else {
		ttoc(8);
	}
}

ctrl_brk_handler() {return;}

/*#define getline(fp) while(fgetc(fp)!='\n')*/
#define getline(fp) fscanf(fp,"%s")

main () {
   FILE *fp;
   union REGS regs;

   int ctr;

   char CR_Flag=0,Semicolon_Flag=0;

   char status_str[80];
   char Parity_byte;
   char term_id_str[]="[?1;2c";
   char *termid;
   unsigned char c;
   char param[MAX_PARAMS];
   char p, dig;
   char i;
   char NoCR=0;

   int tempy=0;

   unsigned char curr_clr = 7;

   char saved_cursor_x;
   char saved_cursor_y;

   char line_wrapping = TRUE;
   char more_params;
   char at_least_one_digit;
   char first_param_implied;

   ctrlbrk(ctrl_brk_handler);

   if (line_wrapping) { line_wrapping = line_wrapping; } /* satisfy compiler */

   if((baselog=(unsigned char *)farmalloc(64000))==NULL)
	exit(1);

   textmode(3);
   cls();

   if((fp=fopen("COMM.INI","rt"))==NULL) {
	BAUD=38400;
	PARITY=0;
	STOPBITS=1;
	WORDLEN=8;
   } else {
	getline(fp);
	fscanf(fp,"%u",&WORDLEN);
	getline(fp);
	fscanf(fp,"%u",&STOPBITS);
	getline(fp);
	fscanf(fp,"%u",&PARITY);
	getline(fp);
	fscanf(fp,"%lu",&BAUD);
	getline(fp);
	fscanf(fp,"%u",&COMPORT);
	fclose(fp);
   }
   if((fp=fopen("PROMPTS.INI","rt"))!=NULL) {
	gotoxy(1,2);
	while((!feof(fp))&&(pctr<=20)) {
		fscanf(fp,"%s",Prompts[pctr]);
		printf("Using %s as a delete flag\n",Prompts[pctr++]);
	}
	fclose(fp);
   }

   switch(PARITY) {
	case 0: Parity_byte='N';
	case 1: Parity_byte='E';
	case 2: Parity_byte='O';
   }
   sprintf(status_str, " SMART-TERM 0.36 ³ %6lu-%c ³ COM %.1u ³ VT100/ANSI",
	BAUD, Parity_byte, COMPORT);

   color28();

   window(1,0,80,NUM_LINES);

   locate(1,1);
   textattr(26); wrch('°',80);
   locate(2,1);
   textattr(27); cputs("  SmartTerm");
   textattr(19); cputs(" -- ");
   textattr(26); cputs("The Intelligent Terminal Emulation Program  ");
   locate(1,NUM_LINES);
   textattr(49); clreol();
   cputs(status_str);
   window(1,1,80,NUM_LINES-2);
   locate(1,NUM_LINES-3);

   setup();

   textattr(curr_clr);
   ClearAllTabs();

   log=0; maxlog=0;

   while (1) {
      while((c = rch()) == (unsigned char)-1);

      if((c!='\r')&&(c!='\n')) CR_Flag=0;

      /* Handle escape sequence */
      if (c == ESC) {
	 c = rch();    /* grab the left bracket */
      if(c=='[') {

         more_params = TRUE;
         first_param_implied = FALSE;
         p = 0;

         while (more_params == TRUE) {
            at_least_one_digit = FALSE;

	    for (dig = 0; (isdigit (c = rch())) && (dig < 3); dig++) {
	       at_least_one_digit = TRUE;

	       /* 3 digits at most (255) in a byte size decimal number */
               if (dig == 0) {
                  param[p] = c - '0';
               }
               else if (dig == 1) {
                  param[p] *= 10;
                  param[p] += c - '0';
               }
               else {
                  param[p] *= 100;
                  param[p] += c - '0';
               }
            }

            /* ESC[C     p should = 0
               ESC[6C    p should = 1
               ESC[1;1H  p should = 2
               ESC[;79H  p should = 2 */

	    if (c != '?') {  /* this is apprently an ANSI bug. */

               if ((at_least_one_digit == TRUE) && (c == ';'))
                  p++;

               else if ((!(at_least_one_digit == TRUE)) && (c == ';')) {
                  p++;
		  first_param_implied = TRUE;
               }

               else if (at_least_one_digit) {
                  p++;
                  more_params = FALSE;
	       }

	       else
                  more_params = FALSE;
	       }

		if(c==';') {Semicolon_Flag=1;} else {Semicolon_Flag=0;}


         }

	 /* Handle specific escape sequences */
         switch (c) {
	    case 'g':
		if(param[0]==0) ClearTabStop();
		else if(param[0]==3) ClearAllTabs();
		break;
	    case '\"':
		if(Semicolon_Flag==1) break;   /* I don't know how to handle this */
		c=rch();
		break;
	    case 'r':                         /* Set scrolling region */
		top_scroll=param[0];
		bottom_scroll=param[1];
		if(top_scroll==0) top_scroll=1;
		if(bottom_scroll==0) bottom_scroll=SCREENBOTTOM;
		window(1,top_scroll,80,bottom_scroll);
		break;
	    case 'Z':                         /* Return terminal ID */
	    case 'c':
/*		writecomm(27);
		termid=term_id_str;
		while(*termid!='\0') writecomm(*termid++);*/
		/* This has been disabled until further research
		 * can be done to determine the source of an
		 * apparent bug.
		 */
		break;
	    case CURSOR_POSITION:
	    case CURSOR_POSITION_ALT:
               if (p == 0)
                  gotoxy (1, 1);
               else if (p == 1)
                  gotoxy (1, param[0]);
               else
                  if (first_param_implied)
		     gotoxy (param[1], wherey ());
                  else
                     gotoxy (param[1], param[0]);
               break;

            case CURSOR_UP:
               if (p == 0) {
                  if (wherey () > 1)
                     gotoxy (wherex (), wherey () - 1);
               }
               else {
                  if (param[0] > wherey ())
                     gotoxy (wherex (), 1);
                  else
                     gotoxy (wherex (), wherey () - param[0]);
               }
               break;

            case CURSOR_DOWN:
               if (p == 0) {
                  if (wherey () < NUM_LINES)
                     gotoxy (wherex (), wherey () + 1);
               }
               else {
                  if (param[0] > (NUM_LINES) - wherey ())
                     gotoxy (wherex (), NUM_LINES - 1);
                  else
                     gotoxy (wherex (), wherey () + param[0]);
               }
               break;

            case CURSOR_FORWARD:
               if (p == 0) {
                  if (wherex () < 80)
                     gotoxy (wherex () + 1, wherey ());
               }
               else {
                  if (param[0] > 80 - wherex ())
                     gotoxy (80, wherey ());
                  else
                     gotoxy (wherex () + param[0], wherey ());
               }
               break;

            case CURSOR_BACKWARD:
               if (p == 0) {
                  if (wherex () > 1)
                     gotoxy (wherex () - 1, wherey ());
               }
	       else {
		     if (param[0] > wherex ())
                     gotoxy (0, wherey ());
                  else
                     gotoxy (wherex () - param[0], wherey ());
               }
               break;

            case SAVE_CURSOR_POS:
               saved_cursor_x = wherex ();
               saved_cursor_y = wherey ();
               break;

            case RESTORE_CURSOR_POS:
               gotoxy (saved_cursor_x, saved_cursor_y);
               break;

            case ERASE_DISPLAY:
		if(param[0]==2) cls();
		else clreos();
		break;

            case ERASE_TO_EOL:
	       clreol ();
               break;

            case SET_GRAPHICS_MODE:
	       /* specific changes can occur in any order */

	       if(p==0) {param[0]=ALL_ATTRIBUTES_OFF; p++;}
	       if(first_param_implied==TRUE) param[0]=ALL_ATTRIBUTES_OFF;

	       for (i = 0; i < p; i++) {
                  if ((param[i] <= 8) && (param[i] >= 0)) {
		     /* Change text attributes */
                     switch (param[i]) {
                        case ALL_ATTRIBUTES_OFF:
			   /*curr_clr &= ~ATTR_INTENSE;
                           curr_clr &= ~ATTR_BLINK;
			   curr_clr &= ~ATTR_BG_MASK;*/
			   curr_clr=7;
                           break;

                        case BOLD_ON:
                           curr_clr |= ATTR_INTENSE;
                           break;

                        case UNDERSCORE:
			   if(color_flag) {mono28(); color_flag=0;}
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_BLUE;
			   /*curr_clr |= 1;*/
                           break;

                        case BLINK_ON:
                           curr_clr |= ATTR_BLINK;
                           break;

                        case REVERSE_VIDEO_ON:
			   /* Set background white and foreground black
			    * leaving intense and blink alone. */
                           curr_clr |= ATTR_BG_WHITE;
                           curr_clr &= ~ATTR_FG_MASK3;
                           break;

                        case CONCEALED_ON:
			   /* foreground and background set to black
			      intense remains. */
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr &= ~ATTR_FG_MASK3;
                           break;
                     }
                     textattr (curr_clr);
                  }

                  else if ((param[i] >= 30) && (param[i] <= 37)) {
		     if(color_flag!=1) {color28(); color_flag=1;}
		     /* Change foreground color*/
                     switch (param[i]) {
                        case FG_BLACK:
			   curr_clr &= ~ATTR_FG_MASK3; /* flip off bits 2 - 0 */
			   curr_clr |= ATTR_FG_BLACK;  /* flip on 1's */
                           break;

                        case FG_RED:
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_RED;
                           break;

                        case FG_GREEN:
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_GREEN;
                           break;

                        case FG_YELLOW:
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_YELLOW;
                           break;

                        case FG_BLUE:
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_BLUE;
                           break;

                        case FG_MAGENTA:
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_MAGENTA;
                           break;

                        case FG_CYAN:
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_CYAN;
                           break;

                        case FG_WHITE:
                           curr_clr &= ~ATTR_FG_MASK3;
                           curr_clr |= ATTR_FG_WHITE;
                           break;
                     }
                     textattr (curr_clr);

                  }
                  else if ((param[i] >= 40) && (param[i] <= 47)) {
		     if(color_flag!=1) {color28(); color_flag=1;}
		     /* Change background color */
                     switch (param[i]) {
                        case BG_BLACK:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_BLACK;
                           break;

                        case BG_RED:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_RED;
                           break;

                        case BG_GREEN:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_GREEN;
                           break;

                        case BG_YELLOW:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_YELLOW;
                           break;

                        case BG_BLUE:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_BLUE;
                           break;

                        case BG_MAGENTA:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_MAGENTA;
                           break;

                        case BG_CYAN:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_CYAN;
                           break;

                        case BG_WHITE:
                           curr_clr &= ~ATTR_BG_MASK;
                           curr_clr |= ATTR_BG_WHITE;
                           break;
                     }
                     textattr (curr_clr);

                  }
	       } /* end graphics param for loop */
               textattr (curr_clr);
               break;


            case RESET_MODE:
               if (param[0] == 7)
		  line_wrapping = FALSE;
	       break;

	    case SET_MODE:
	       if (param[0] == 7) {
                  line_wrapping = TRUE;
	       }
	       break;

            case SET_KEYBOARD_STRINGS:
               break;

            default:
               break;

	 } /* end escape switch */

      } else {
	switch(c) {
		case 'D': ScrollUp(); break;
		case 'E': ScrollUp(); break;
		case 'M': ScrollDown(); break;
		case 'H': SetTabStop(); break;
		case 'Z':                         /* Return terminal ID */
			writecomm(27);
			termid=term_id_str;
			while(*termid!='\0') writecomm(*termid++);
			break;
		default: break;
	}


      } /* end if main escape sequence handler */}
      else {
	 if(NoCR) NoCR--;
	 /* otherwise output character using current color */
	 switch(c) {
		case 7: VTBell();
		case 0: break;
		case 11:
		case 12:   cls(); break;
		case 8:
			putch('\b');
			if((NL_Flag!=1)||(color_flag==1)) break;
			putch(' '); putch('\b'); break;
		case '\n':
			if(NoCR) break;
			if(CR_Flag==1) {putch('\n'); break;}
			putch('\n'); putch('\r'); NL_Flag=1; break;
		case '\r':
			if(NoCR) break;
			NL_Flag=1; putch('\r'); CR_Flag=1; break;
		case '\t': DoTab(); break;
		default:
			tempy=wherey();
			if(NL_Flag==1) {
				Delete_Flag=0;
				for(ctr=0;ctr<pctr;ctr++)
					if(Prompts[ctr][0]==c)
						Delete_Flag=ctr+1;
			}
			if((Delete_Flag!=0)&&(Delete_Flag!=255)) {
				if(Prompts[Delete_Flag-1][wherex()-1]!=c) {
					Delete_Flag=0;
				} else {
					if(Prompts[Delete_Flag-1][wherex()]=='\0')
						Delete_Flag=255;
				}
			}
			NL_Flag=0;
			putch(c);
			if(tempy!=wherey()) NoCR=2;
	 }

      }
   } /* continue loop forever */

   return 0;
} /* end main */
