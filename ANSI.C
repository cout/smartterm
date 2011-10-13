/*
 * A generic ANSI interpreter by Saeger  (aka: Jason Farrell)
 * (c) 1995, Saeger
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <ctype.h>

#include "ansicode.h"
#include "clrattr.h"

#define NUM_LINES   25     /* change to 50 for 80x50 ANSI's */
#define MAX_PARAMS  10

enum { FALSE, TRUE };
enum { progname, ansifile };


main (int argc, char **argv) {
   FILE *fp;
   union REGS regs;

   unsigned char c;
   char param[MAX_PARAMS];
   char p, dig;
   char i;

   unsigned char curr_clr = 0;

   char curr_x = wherex (), curr_y = wherey ();

   char saved_cursor_x;
   char saved_cursor_y;

   char line_wrapping = TRUE;
   char more_params;
   char at_least_one_digit;
   char first_param_implied;

   if (line_wrapping) { line_wrapping = line_wrapping; } /* satisfy compiler */

   if (argc == 1) {
      printf ("Must specify filename[.ans]\n");
      exit (1);
   }

   if (! (fp = fopen (argv[ansifile], "r"))) {
      printf ("Error opening '%s'\n", argv[ansifile]);
      exit (1);
   }


   while (! feof (fp)) {
      c = getc (fp);

      /* Handle escape sequence */
      if (c == ESC) {
	 c = getc (fp);    /* grab the left bracket */

         more_params = TRUE;
         first_param_implied = FALSE;
         p = 0;

         while (more_params == TRUE) {
            at_least_one_digit = FALSE;

            for (dig = 0; (isdigit (c = getc (fp))) && (dig < 3); dig++) {
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

         }

	 /* Handle specific escape sequences */
         switch (c) {
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
               if (param[0] == 2)
                  clrscr ();
               break;

            case ERASE_TO_EOL:
               clreol ();
               break;

            case SET_GRAPHICS_MODE:
	       /* specific changes can occur in any order */

               for (i = 0; i < p; i++) {
                  if ((param[i] <= 8) && (param[i] >= 0)) {
		     /* Change text attributes */
                     switch (param[i]) {
                        case ALL_ATTRIBUTES_OFF:
                           curr_clr &= ~ATTR_INTENSE;
                           curr_clr &= ~ATTR_BLINK;
                           curr_clr &= ~ATTR_BG_MASK;
                           break;

                        case BOLD_ON:
                           curr_clr |= ATTR_INTENSE;
                           break;

                        case UNDERSCORE:
			   /* ? monochrome only... but what bits?*/
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

            case SET_MODE:
               if (param[0] <= 6 || (param[0] >= 8 && param[0] <= 16)) {
                  regs.h.ah = 0;
                  regs.h.al = param[0];
                  int86 (0x10, &regs, &regs);
               }
               else if (param[0] == 7) {
                  line_wrapping = TRUE;
               }
               break;


            case SET_KEYBOARD_STRINGS:
               break;

            default:
               break;

	 } /* end escape switch */

         /*
         printf ("\n--------> '%c' <--------- ", c);
         for (i = 0; i < p; i++)
            printf ("\nparam[%d] = %d", i, param[i]);
         printf ("\n-------------------------");
         */

      } /* end if main escape sequence handler */
      else {
	 /* otherwise output character using current color */
         if (c == '\n')
            cprintf ("\n\r");
         else
            cprintf ("%c", c);

      }
   } /* end while !feof */

   cprintf ("\r");
   textattr(7);
   /*clreol ();*/
   /*cprintf ("\n");*/

   return 0;
} /* end main */


