
/*/////////////////////////////////////////////////////////////////////
//                                                                   //
//  TXZM.C -- zmodem protocol driver (formerly ZMP)                  //
//                                                                   //
//    (c) 1991-94, Mike Dumdei, 6 Holly Lane, Texarkana TX, 75503    //
//                                                                   //
//////////////////////////////////////////////////////////////////// */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>
#include <ctype.h>
#include <io.h>
#include <dos.h>
#include <bios.h>

#if defined (__TURBOC__)
  #include <dir.h>
  #define ChDrive(d) setdisk((d)-1)
  #pragma warn -par
  #ifdef _RTLENTRY              // BC4 or greater
      #pragma option -x- -RT-
      void _ExceptInit(void) {} // disable exception handling
  #endif
#elif defined (__ZTC__)
  #include <direct.h>
  #define ChDrive(d) _chdrive(d)
#elif defined (__MSC__)
  int ndrives;
  #define ChDrive(d) _dos_setdrive((d), &ndrives)
#endif

#define ALT_C       0x2e00
#define ALT_D       0x2000
#define ALT_E       0x1200
#define ALT_H       0x2300
#define ALT_L       0x2600
#define ALT_P       0x1900
#define ALT_R       0x1300
#define ALT_S       0x1f00
#define ALT_X       0x2d00
#define FK1         0x3b00
#define PGUP        0x4900
#define PGDN        0x5100
#define ALT_EQU     0x8300
#define X_ESC       0x011b

void AddToList(char *subdir);
char *CaptureBaudRate(void);
char *ConvertSecs(long secs);
int Dial(void);
void DrawBox(int row, int col, int nrows, int ncols, int color, int style);
void DspZmodemScrn(void);
char *ExpandSubDirs(char *fnames);
int FileTransfer(void);
int GetNumber(void);
long getHunds(void);
long getSeconds(void);
void HelpFunc(void);
void InitDefaults(void);
void MiniTermMode(void);
void ProcCmdLine(int argc, char * *argv);
int prompt(char *buf, int maxlen);
void ReadDialDir(void);
int RecurseSubDirs(char *sd);
int RepeatDial(void);
void SetFIFOLevels(int rxFIFOlevel, int txFIFOlevel);
int SetParams(char *newparams);
void usage(void);
void vDisplay(int row, int col, char *format, ...);
int waitfor(int ticks, ...);
void ZMsg(int type, ...);

extern int  _C_ TestDesqView(void);
extern int  _C_ DV_VideoSeg(int);
extern void _C_ DV_TimeSlice(void);

/*/////////////////////////////////////////////////////////
//  Configuration structure.  Structure is used to allow //
//  the defaults to be changed modifying the EXE.        //
//  "TXZMCFG:" is the tag to search for to find the start//
//  of the structure in the EXE.                         //
//////////////////////////////////////////////////////// */
struct PROTCONFIG
{
    char    tag[8], DLPath[80], ULPath[80];
    long    LocBaud, MinFifoBaud;
    int     ComBase, IRQ, Vctr;
    int     h_VBufSize, h_BufSize, b_VBufSize, b_BufSize;
    int     TxWindow, ZExistOpts, XYExistOpts;
    int     FifoTxLvl, FifoRxLvl;
    char    IgnCarrier, MsrFlow, KeepTime, EscCtl, OvlyIO;
    char    Color[10], Mono[10];
} cfg =
{
    "TXZMCFG:", "", "",
    0L, 1L,
    0x3f8, IRQ4, VCTR4,
    2048, 0, 20508, 20480,
    0, 1, 2,
    8, 8,
    0, 0, 1, 0, 1,
    {  WHT|BLU_B,  H_GRN|BLU_B,  H_RED|BLU_B,    YLW|BLU_B,
       GRY|BLU_B,    YLW|BLU_B,  H_RED|BLU_B,  H_MAG|BLU_B,
       CYN,              CYN_B  },
    { WHT, WHT, RVRS, H_WHT, WHT, H_WHT, H_WHT, H_WHT, WHT, WHT_B }
};

/*/////////////////////////////////////////////////////////
//  Screen data structure                                //
//////////////////////////////////////////////////////// */
typedef struct
{
    int row, col, color, count;
    char *text;
} SCREENDATA;

/*/////////////////////////////////////////////////////////
//  Dial directory structure                             //
//////////////////////////////////////////////////////// */
static struct DIAL_DIRECTORY
{
    char *num;
    char *name;
} DialDir[10];

/*/////////////////////////////////////////////////////////
//  Global variables                                     //
//////////////////////////////////////////////////////// */
ASYNC port;                 /* ASYNC port structure */
int combase, irq, vctr;     /* port address, IRQ number, & vector */
int openmask = 0;           /* mask for forcing no FIFOs, no MSR intrpts */
long LocBaud = 0L;          /* CPU to modem/device baud rate */
char params[12] = "";       /* port parameters */
char lockedbaud[12] = "";   /* locked baud parameter */
char fnames[256] = "";      /* list of files to send if sending */
char minsecs[10];           /* seconds to min:secs buffer */
char *color;                /* pointer to list of colors */
int txtcolor;               /* color of most screen message output */
char buf[256];              /* general purpose buffer */
int goodargs = 0;           /* got an 'r' or an 's' on command line */
char *node = NULL;          /* for bbs use: LASTUSER.BBS format file name */
int miniterm = 0;           /* mini-terminal mode selected */
char OvlyIO;                /* overlay disk and serial I/O flag */
char *flist;                /* used when expanding subdirectories */
char mask[14];              /* used when expanding subdirectories */
int plen;                   /* used when expanding subdirectories */
int tryDV = 0;              /* cmdline switch to test for DesqView */
int checkcarrier = 0;       /* check for carrier during 'waitfor' flag */
int stripmask = 0xff;       /* strip high bit mask for miniterm mode */
char phone[40] = "";        /* phone number to dial */
char DialPrefix[16] = "ATDT"; /* default dial prefix */
char *OverrideName = NULL;  /* cmdline rx filename used if not NULL */
char OverrideFlag = '\0';   /* if '/', above specifies complete pathname */

long PrevSecs = -1L;        /* used by getSeconds to handle midnight */
long PrevHunds = -1L;       /* used by getHunds to handle midnight */
long tfBytes, tCPS;         /* total bytes transferred, average CPS */
int tFiles;                 /* number of files transferred */

/* variables & dummy function to disable display of file transfer screen */
int qf = 0;
void DoNothing(int r, int c, int a, char *s) {}
void (*DspMsgAt[2])(int, int, int, char *) = { d_msgat, DoNothing };


/*/////////////////////////////////////////////////////////
//  DrawBox                                              //
//:dra////////////////////////////////////////////////// */
void DrawBox(int row, int col, int nrows, int ncols, int color, int style)
{
    static char boxchars[][6] =
    {
        'Ú', '¿', 'À', 'Ù', 'Ä', '³',
        'É', '»', 'È', '¼', 'Í', 'º',
        'Õ', '¸', 'Ô', '¾', 'Í', '³',
        'Ö', '·', 'Ó', '½', 'Ä', 'º'
    };
    char *box = boxchars[style];
    int temp = v_color;

    v_color = color;
    --ncols;
    scrlupat(row, col, row + nrows - 1, col + ncols, nrows);
    --nrows;
    ++col, --ncols, --ncols;
    d_nchat(row, col, box[4], color, ncols, 1);
    d_nchat(row + nrows, col, box[4], color, ncols, 1);
    d_nchat(row, col, box[5], color, nrows, 0);
    d_nchat(row, col + ncols, box[5], color, nrows, 0);
    d_chat(row, col, box[0]);
    d_chat(row, col + ncols, box[1]);
    d_chat(row + nrows, col, box[2]);
    d_chat(row + nrows, col + ncols, box[3]);
    v_color = temp;
}

/*/////////////////////////////////////////////////////////
//  DspZmodemScrn                                        //
//:dsp////////////////////////////////////////////////// */
void DspZmodemScrn(void)
{
    static SCREENDATA zscrn[] =
    {
        {  1,  6, 2,   0, " Zmodem File " },
        {  2, 71, 0, -20, "³" },
        { 15,  5, 0,  64, "Ä" },
        {  2, 73, 0,   0, "F T" },
        {  3, 73, 4, -19, "±" },
        {  3, 75, 4, -19, "±" },
        {  3,  5, 0,   0, "File name :" },
        {  5,  5, 0,   0, "Estimated time :" },
        {  6,  5, 0,   0, "Elapsed time   :" },
        {  7,  5, 0,   0, "File CPS rate  :" },
        {  5, 37, 0,   0, "File position      :" },
        {  6, 37, 0,   0, "Expected file size :" },
        {  7, 37, 0,   0, "Beginning offset   :" },
        {  9, 51, 0,   0, "ÄÄÄ Hdr Data ÄÄÄ" },
        { 10, 23, 0,   0, "Header Name" },
        { 10, 40, 0,   0, "Type" },
        { 10, 52, 0,   0, "Hex" },
        { 10, 61, 0,   0, "Decimal" },
        { 11,  5, 0,   0, "Last hdr recvd :" },
        { 12,  5, 0,   0, "Last hdr sent  :" },
        { 14,  5, 0,   0, "Crc :" },
        { 14, 37, 0,   0, "Flow :" },
        { 17,  5, 0,   0, "Total files queued :" },
        { 18,  5, 0,   0, "Total bytes queued :" },
        { 17, 42, 0,   0, "Estimated time   :" },
        { 18, 42, 0,   0, "Accumulated time :" },
        { 20,  5, 0,   0, "Files transferred  :" },
        { 21,  5, 0,   0, "Bytes transferred  :" },
        { 21, 42, 0,   0, "Average CPS rate :" },
        { -1, -1, 0,   0, "" }
    };
    register SCREENDATA *sd;

    if (qf != 0)
        return;
    v_color = WHT;
    cls();
    v_color = txtcolor;
    DrawBox(1, 1, 22, 78, color[0], 0);
    for (sd = zscrn; sd->row >= 0; ++sd)
    {
        if (sd->count == 0)
            d_msgat(sd->row, sd->col, color[sd->color], sd->text);
        else if (sd->count > 0)
        {
            d_nchat(sd->row, sd->col, *sd->text, color[sd->color],
             sd->count, 1);
        }
        else
        {
            d_nchat(sd->row, sd->col, *sd->text, color[sd->color],
             -sd->count, 0);
        }
    }
    d_msgat(1, 19, color[2], (TFlag.F.Receiving) ? "Receive " : "Send ");
    loc(23, 0);
}

/*/////////////////////////////////////////////////////////
//  ExpandSubDirs                                        //
//:exp////////////////////////////////////////////////// */
char *ExpandSubDirs(char *fnames)
{
    DF fbuf;
    char *p1, *lstptr, *wrkname, *savdir, *orgdir;
    int i, dosub;

    orgdir = malloc(_MAX_PATH), getcwd(orgdir, _MAX_PATH);
    wrkname = malloc(_MAX_PATH), savdir = malloc(_MAX_PATH);
    plen = 1, lstptr = SkipSpaces(fnames), flist = calloc(1, 1);
    while (1)
    {
        if (lstptr != fnames)
        {
            if (wrkname[1] == ':')
                chdir(savdir);
            ChDrive(toupper(*orgdir) - 'A' + 1);
            chdir(orgdir);
        }
        if (*lstptr == '\0')
        {
            free(wrkname), free(orgdir), free(savdir);
            return(flist);
        }
        p1 = lstptr, i = SkipChars(lstptr) - lstptr, dosub = 0;
        lstptr = (SkipSpaces(SkipChars(lstptr)));
        if (*p1 == '(' && p1[i - 1] == ')')
            ++p1, dosub = 2;
        strncpy(wrkname, p1, i), wrkname[i - dosub] = '\0';
        if (wrkname[1] == ':')
        {
            ChDrive(toupper(*wrkname) - 'A' + 1);
            getcwd(savdir, _MAX_PATH);
        }
        p1 = GetNameExt(wrkname);
        strcpy(mask, "*.*");
        if (*p1)
        {
            fbuf.attrib = 0;
            if (!strchr(p1, '*') && !strchr(p1, '?'))
                DosFindFirst(wrkname, -1, &fbuf);
            if (fbuf.attrib & _A_SUBDIR)
                p1 = strchr(p1, '\0');
            else
            {
                strupr(strncpy(mask, p1, 12));
                p1[0] = mask[12] = '\0';
            }
        }
        if (p1 != wrkname && *(--p1) != ':')
        {
            if (p1 != wrkname && *p1 == '\\' && *(p1 - 1) != ':')
                *p1 = '\0';
            if (chdir(wrkname) != 0)
                continue;
        }
        getcwd(wrkname, _MAX_PATH);
        if (dosub == 0)
            AddToList(wrkname);
        else
            RecurseSubDirs(wrkname);
    }
}

/*/////////////////////////////////////////////////////////
//  FileTransfer - file send / receive caller            //
//:fil////////////////////////////////////////////////// */
int FileTransfer(void)
{
    FILE *fh;
    int rval, temp = v_color;
    long efficiency = 0L;

    if (miniterm)
        pushscrn(0, 0, 24, 80);
    DspZmodemScrn();
    if (node != NULL)           /* BBS support -- Maximus */
    {
        if (qf == 0 && (fh = fopen(node, "rb")) != NULL)
        {
            memset(buf, 0, sizeof(buf));
            fread(buf, 1, sizeof(buf), fh);
            fclose(fh);
            buf[32] = buf[68] = '\0';
            d_msgat(23, 1, WHT, &buf[0]);
            d_msgat(23, 40, WHT, &buf[36]);
        }
    }
    SetFIFOLevels(cfg.FifoRxLvl, cfg.FifoTxLvl);
    ZMsg(M_RESET);
    if (TFlag.F.Receiving)
        rval = ZmodemRecv(&port);
    else
    {
        rval = ZmodemSend(&port, flist);
        free(flist);
    }
    if (ConnectBaud)
        efficiency = uldiv(ulmul(tCPS, 1000L), ConnectBaud);
    sprintf(buf, "\r\nCPS: %ld (%d files, %ld bytes)  Efficiency %ld%%",
     tCPS, tFiles, tfBytes, efficiency);
    if (qf == 0)
    {
        loc(23,1), v_color = WHT;
        d_str(&buf[2]);
    }
    tdelay(4);
    if (node != NULL && async_carrier(&port) && tFiles)
    {                           /* send other end result report */
        async_txblk(&port, buf, strlen(buf));
        async_txblk(&port, "\r\n", 2);
        while (!async_txempty(&port))
            ;
    }
    async_rxflush(&port);

    if (miniterm)
    {
        SetFIFOLevels(1, 1);
        d_str(", press ENTER...");
        KBREAD;
        popscrn();
        d_str(buf);
    }
    v_color = temp;
    return (rval);
}

/*/////////////////////////////////////////////////////////
//  InitDefaults - init protocol global variables        //
//:ini////////////////////////////////////////////////// */
void InitDefaults(void)
{
    combase = cfg.ComBase, irq = cfg.IRQ, vctr = cfg.Vctr;
    LocBaud = cfg.LocBaud;
    OvlyIO = cfg.OvlyIO;
    TFlag.F.ExistOpts = cfg.ZExistOpts;
    TFlag.F.IgnCarrier = cfg.IgnCarrier;
    TFlag.F.KeepTime = cfg.KeepTime;
    TFlag.F.EscCtl = cfg.EscCtl;
    TxWindow = cfg.TxWindow;
}

/*/////////////////////////////////////////////////////////
//  Prompt - prompt for input string                     //
//:pro////////////////////////////////////////////////// */
int prompt(char *buf, int maxlen)
{
    char *p1 = buf;
    int ch = 0;

    while (ch != '\r' && ch != '\x1b')
    {
        if ((ch = (KBREAD & 0xff)) == '\r')
            continue;
        if (ch == '\b')
        {
            if (p1 > buf)
                d_ch((char)ch), --p1;
        }
        else if (p1 >= &buf[maxlen])
            d_ch('\a');
        else if (ch != '\x1b' && isprint(ch))
            d_ch((char)ch), *p1++ = ch;
    }
    d_ch('\n');
    if (ch == '\x1b')
        p1 = buf;
    *p1 = '\0';
    return (*buf);
}

/*/////////////////////////////////////////////////////////
//  RecurseSubDirs                                       //
//:rec////////////////////////////////////////////////// */
int RecurseSubDirs(char *sd)
{
    DF fbuf;
    static char subdir[_MAX_PATH];
    int mask = (TFlag.F.SendHid) ? _A_SUBDIR|_A_HIDDEN : _A_SUBDIR;

    if (chdir(sd) != 0)
        return 0;
    getcwd(subdir, _MAX_PATH);
    AddToList(subdir);
    if (!DosFindFirst("*.*", mask, &fbuf))
    {
        do {
            if (fbuf.attrib & _A_SUBDIR && fbuf.name[0] != '.')
                RecurseSubDirs(fbuf.name);
        } while (!DosFindNext(&fbuf));
    }
    chdir("..");
    getcwd(subdir, _MAX_PATH);
    return 1;
}

/*///////////////////////////////////////////////
//  vDisplay                                   //
//:vdi//////////////////////////////////////// */
void vDisplay(int row, int col, char *format, ...)
{
    va_list arg_ptr;
    char lbuf[80];

    va_start(arg_ptr, format);
    vsprintf(lbuf, format, arg_ptr);
    DspMsgAt[qf](row, col, txtcolor, lbuf);
}

/*--------------------------------------------+
|  zmodem message types (defined in ZMDOS.H)  |
+---------------------------------------------/
#define M_RHDR      0       // received header
#define M_SHDR      1       // sent header
#define M_BLK0      2       // block 0 data processed (name, size, etc.)
#define M_CLASH     3       // file name clash occurred (use ExistOpts)
#define M_FILE      4       // start of transfer, FilePos = 1st position
#define M_EOF       5       // end of transfer (1 file)
#define M_DATA      6       // sent or received file data packet
#define M_FLOW      7       // change in XOFF or CTS flow status
#define M_IDLE      8       // waiting for character or for tx to empty
#define M_RESET     9       // reset to 'first file' condition */

/*/////////////////////////////////////////////////////////
//  ZMsg -- zmodem message handler                       //
//:zms///////////////////////////////////////////////:z: */
void ZMsg(int type, ...)
{
    static char *HdrType[] =
    {
        "Garbage Count", "Long Packet",   "Garbled Packet",   "Bad Crc",
        "Timed Out",     "Unknown Hdr",   "Sync Error",       "Memory Error",
        "File Error",    "Lost Carrier",  "Remote Abort",     "Local Abort",
        "ZRQINIT",       "ZRINIT",        "ZSINIT",           "ZACK",
        "ZFILE",         "ZSKIP",         "ZNAK",             "ZABORT",
        "ZFIN",          "ZRPOS",         "ZDATA",            "ZEOF",
        "ZFERR",         "ZCRC",          "ZCHALLENGE",       "ZCOMPL",
        "ZCAN",          "ZFREECNT",      "ZCOMMAND",         "ZSTDERR"
    };
    static char *HdrStyle[] = { "ZBIN", "ZHEX", "ZBIN32", "" };
    static char *CrcStyle[] = { "ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW" };
    static char MsgMask[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    static long fStartSec, tStartSec = -1L, fStartHunds, tStartHunds = -1L;
    static long fCPS, fBytes, tBytes;
    static long fBarInc, tBarInc, fBarBytes, tBarBytes;
    static long fSize, fStartPos, updBytes, lastPos, orgTBytes;
    static long lastSec, temp, flowhalts;
    static int tBarRow, fBarRow;
    static char gotBlock, crcType = ZCRCG, firstfile = 1;

    FILE *flog;
    va_list argptr;
    int i, j, row, hdr;
    long secs;
    char *p1;

    if (MsgMask[type] == '\0')
        return;
    switch (type)
    {
      case M_RHDR:              /* received header */
      case M_SHDR:              /* sent header */
        row = (type == M_RHDR) ? 11 : 12;
        va_start(argptr, type);
        if ((hdr = va_arg(argptr, int)) < 0)
            ZHdr.Data = 0L;
        i = hdr + 12;
        j = va_arg(argptr, int) - 'A';
        if (j < 0 || j > 3)
            j = 3;
        vDisplay(row, 22, "%-17s%-9s%08lX%11ld", HdrType[i], HdrStyle[j],
         ZHdr.Data, ZHdr.Data);
        break;
      case M_CLASH:             /* file name clash occurred */
        if (TFlag.F.ExistOpts != 2)
            break;  /* not new name */
      case M_BLK0:              /* sent or received block 0 */
        if (OverrideName != NULL && TFlag.F.Receiving && type != M_CLASH)
        {
            if (OverrideFlag == '/')
                strcpy(PathName, OverrideName);
            else
                strcpy(NameExt, GetNameExt(OverrideName));
        }
        i = sprintf(buf, "%-52s", PathName) - 52;
        DspMsgAt[qf](3, 17, color[3], &(strupr(buf))[i]);
        if (type == M_CLASH)
            break;
        v_color = txtcolor;
        flowhalts = 0L;
        if (qf == 0)
        {
            scrlupat(5, 22, 6, 32, 0);
            scrlupat(5, 58, 7, 68, 0);
        }
        fSize = (TFlag.F.Receiving) ? RxdFileSize : FileSize;
        vDisplay(6, 58, "%-8ld", fSize);
        fStartHunds = getHunds();
        fStartSec = uldiv(fStartHunds, 100L);
        if (firstfile)
        {
            firstfile = tFiles = 0;
            tBytes = tBarBytes = 0L;
            lastSec = tStartSec = fStartSec;
            tStartHunds = fStartHunds;
            tBarRow = 21;
            if ((tBarInc = uldiv(TotalBytes, 19L)) == 0)
                ++tBarInc;
            orgTBytes = TotalBytes;
            if ((tCPS = uldiv(ulmul(ConnectBaud, 955L), 10000L)) == 0L)
                ++tCPS;
            vDisplay(17, 61, "%-8s",
             ConvertSecs(uldiv(TotalBytes, tCPS) + (TotalFiles >> 2)));
        }
        if ((fBarInc = uldiv(fSize, 19L)) == 0)
            ++fBarInc;
        fBarBytes = 0L, fBarRow = 21;
        if (qf == 0)
            d_nchat(3, 73, '±', color[4], 19, 0);
        if (orgTBytes != 0L)
        {
            j = 3 + (int)((orgTBytes & 0xffff0000L) ?
             uldiv(TotalBytes, uldiv(orgTBytes, 19L)) :
             uldiv(ulmul(TotalBytes, 19L), orgTBytes));
            while (j < tBarRow && tBarRow > 2)
                DspMsgAt[qf](tBarRow--, 75, color[6], "±");
            vDisplay(17, 26, "%-4d", TotalFiles - 1);
            vDisplay(18, 26, "%-8ld", TotalBytes - fSize);
        }
        else
        {
            tBarInc = fBarInc, tBarBytes = 0L, tBarRow = 21;
            if (qf == 0)
                d_nchat(3, 75, '±', color[4], 19, 0);
        }
        break;
      case M_EOF:               /* end of file transfer */
        va_start(argptr, type);
        i = va_arg(argptr, int);
        if (i == ZEOF || i == ZRINIT)
        {
            vDisplay(20, 26, "%d", ++tFiles);
            tfBytes = tBytes;
            if (((p1 = getenv("TXZMLOG")) != NULL)
             && ((flog = fopen(p1, "at")) != NULL))
            {
                sprintf(buf,
                 "%c %6ld %5ld bps %4ld cps %3u errors %5ld %4d %s %d\n",
                 TFlag.F.Receiving ? 'Z' : 'z', FilePos, ConnectBaud, fCPS,
                 ErrCnt, flowhalts, BlkLen, NameExt, (SerNbr) ? SerNbr : -1);
                fputs(buf, flog);
                fclose(flog);
            }
        }
        if (orgTBytes != 0L)
        {
            temp = TotalBytes - fSize;
            j = 3 + (int)((orgTBytes & 0xffff0000L) ?
             uldiv(temp, uldiv(orgTBytes, 19L)) :
             uldiv(ulmul(temp, 19L), orgTBytes));
            while (j < tBarRow && tBarRow > 2)
                DspMsgAt[qf](tBarRow--, 75, color[6], "±");
        }
        crcType = ZCRCG;
        break;
      case M_FILE:              /* start of transfer, 1st FilePos set */
        lastPos = fStartPos = FilePos;
        fBytes = 0L;
        vDisplay(5, 22, "%-8s", ConvertSecs(uldiv(fSize - fStartPos, tCPS)));
        vDisplay(7, 58, "%-8ld", fStartPos);
        if (fStartPos == 0L)
            break;
        updBytes = fStartPos;
      case M_DATA:              /* sent or received file data packet */
        if (type != M_FILE)
        {
            gotBlock = 1;
            updBytes = FilePos - lastPos;
            lastPos = FilePos;
            fBytes += updBytes, tBytes += updBytes;
            vDisplay(5, 58, "%-8ld", FilePos);
            vDisplay(21, 26, "%-8ld", tBytes);
            va_start(argptr, type);
            vDisplay(14, 11, "%s-%s", (BinHdr == ZBIN32) ? "32" : "16",
             CrcStyle[(crcType = (char)va_arg(argptr, int)) - ZCRCE]);
        }
        fBarBytes += updBytes;
        tBarBytes += updBytes;
        while (fBarBytes >= fBarInc && fBarRow > 2)
            fBarBytes -= fBarInc, DspMsgAt[qf](fBarRow--, 73, color[5], "±");
        while (tBarBytes >= tBarInc && tBarRow > 2)
            tBarBytes -= tBarInc, DspMsgAt[qf](tBarRow--, 75, color[6], "±");
        break;
      case M_FLOW:
        va_start(argptr, type);
        switch (i = va_arg(argptr, int))
        {
          case 0:           /* XOFF cleared (XON received) */
          case 1:           /* XOFF received */
            DspMsgAt[qf](14, 44, color[3], (i) ? "XOFF" : "    ");
            break;
          case 2:           /* CTS signal raised */
          case 3:           /* CTS signal lowered */
            DspMsgAt[qf](14, 49, color[3], (i == 3) ? "CTS" : "   ");
            break;
          case 4:           /* port error */
            DspMsgAt[qf](14, 55, color[3]|BLNK, "PORT RESET");
            tdelay(18);
            DspMsgAt[qf](14, 55, color[3], "          ");
        }
        flowhalts += (i & 1);
        break;
      case M_IDLE:
        if (firstfile || ((secs = getSeconds()) == fStartSec))
            break;
        if (gotBlock || crcType == ZCRCE)
        {
            gotBlock = 0;
            if (fBytes & 0xffff0000L)
                fCPS = uldiv(fBytes, secs - fStartSec);
            else
                fCPS = uldiv(ulmul(fBytes, 100L), getHunds() - fStartHunds);
            if (tBytes & 0xffff0000L)
                tCPS = uldiv(tBytes, secs - tStartSec);
            else
                tCPS = uldiv(ulmul(tBytes, 100L), getHunds() - tStartHunds);
            if (tCPS == 0)
                ++tCPS;
            vDisplay(7, 22, "%-6ld", fCPS);
            vDisplay(21, 61, "%-6ld", tCPS);
        }
        if (secs != lastSec)
        {
            lastSec = secs;
            DspMsgAt[qf](6, 22, txtcolor, ConvertSecs(secs - fStartSec));
            DspMsgAt[qf](18, 61, txtcolor, ConvertSecs(secs - tStartSec));
        }
        if (tryDV)
            DV_TimeSlice();
        break;
      case M_RESET:
        if (cfg.MsrFlow && !async_cts(&port))
            DspMsgAt[qf](14, 49, color[3], "CTS");
        crcType = ZCRCG, firstfile = 1;
        tStartSec = PrevSecs = PrevHunds = -1L;
        break;
    }
}

