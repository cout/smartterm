/* Character color attribute bit values:
 * 7        6        5        4        3        2        1        0
 * blink    red      green    blue     intense  red      green    blue
 */

/* Defines for character attributes */
#define ATTR_BLINK       0x80   /* 10000000 */
#define ATTR_INTENSE     0x08   /* 00001000 */

/* Background Colors */
#define ATTR_BG_BLACK    0x00   /* 00000000 */
#define ATTR_BG_BLUE     0x10   /* 00010000 */
#define ATTR_BG_GREEN    0x20   /* 00100000 */
#define ATTR_BG_CYAN     0x30   /* 00110000 */
#define ATTR_BG_RED      0x40   /* 01000000 */
#define ATTR_BG_MAGENTA  0x50   /* 01010000 */
#define ATTR_BG_YELLOW   0x60   /* 01100000 */
#define ATTR_BG_WHITE    0x70   /* 01110000 */
 /* #define ATTR_BG_BROWN    0x60   /* 01100000 */
 /* #define ATTR_BG_GREY     0x70   /* 01110000 */

/* Foreground Colors */
#define ATTR_FG_BLACK    0x00   /* 00000000 */
#define ATTR_FG_BLUE     0x01   /* 00000001 */
#define ATTR_FG_GREEN    0x02   /* 00000010 */
#define ATTR_FG_CYAN     0x03   /* 00000011 */
#define ATTR_FG_RED      0x04   /* 00000100 */
#define ATTR_FG_MAGENTA  0x05   /* 00000101 */
#define ATTR_FG_YELLOW   0x06   /* 00000110 */
#define ATTR_FG_WHITE    0x07   /* 00000111 */

/* Useful Masks */
#define ATTR_BG_MASK     0x70   /* 01110000 */
#define ATTR_FG_MASK3    0x07   /* 00000111 */
#define ATTR_FG_MASK4    0x0f   /* 00001111 */
