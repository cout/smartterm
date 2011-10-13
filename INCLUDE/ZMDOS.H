
/*/////////////////////////////////////////////////////////////////////
//                                                                   //
//  ZMDOS.H -- zmodem protocol header file                           //
//                                                                   //
//    Mike Dumdei, 6 Holly Lane, Texarkana TX, 75503                 //
//                                                                   //
//////////////////////////////////////////////////////////////////// */
#ifdef __cplusplus
  extern "C" {
#endif

/*/////////////////////////////////////////////////////////
//                                                       //
//  general                                              //
//                                                       //
//////////////////////////////////////////////////////// */
typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;

/*/////////////////////////////////////////////////////////
//                                                       //
//  compiler specific                                    //
//                                                       //
//////////////////////////////////////////////////////// */
#ifndef __TURBOC__
  #ifndef __ZTC__
    #define __MSC__
  #endif
#endif

#define MAX_PATH            148
#define KBHIT               _bios_keybrd(1)
#define KBREAD              _bios_keybrd(0)
#define Upd16Crc(crc,c)     ((Crc16Tbl[(crc) >> 8]) ^ (((crc) << 8) ^ (c)))

/*--------------------------------------------+
|  DOS findfirst/findnext structure           |
+--------------------------------------------*/
typedef struct
{
    char reserved[21];
    char attrib;
    unsigned time;
    unsigned date;
    long size;
    char name[13];
} DF;

#ifdef __TURBOC__
  #if (__TURBOC__ < 0x400)
    #define _A_NORMAL           0
    #define _A_RDONLY           FA_RDONLY
    #define _A_SUBDIR           FA_DIREC
    #define _A_HIDDEN           FA_HIDDEN
    #define _A_SYSTEM           FA_SYSTEM
    #define _A_VOLID            FA_LABEL
  #endif
#endif

#ifndef _MAX_PATH
 #define _MAX_PATH 148
#endif

/*/////////////////////////////////////////////////////////
//                                                       //
//  selected definitions from Chuck Forsberg's PD zmodem //
//                                                       //
//////////////////////////////////////////////////////// */
/*--------------------------------------------+
|  limits                                     |
+--------------------------------------------*/
#define ZATTNLEN    32        /* max length of attention string */
#define MAXPACKET 1024        /* maximum size of a packet */

/*--------------------------------------------+
|  special character definitions              |
+--------------------------------------------*/
#define ZPAD        '*'       /* pad char - begins zmodem frames */
#define ZDLE      '\x18'      /* zmodem escape - aka CAN, Ctrl-X */
#define ZBIN        'A'       /* binary frame indicator (CRC-16) */
#define ZHEX        'B'       /* hex frame indicator */
#define ZBIN32      'C'       /* binary frame indicator (CRC-32) */
#define XON       '\x11'      /* XON character */
 /*++ zdle sequences ++*/
#define ZCRCE       'h'       /* CRC next, frame ends, hdr packet follows */
#define ZCRCG       'i'       /* CRC next, frame continues nonstop */
#define ZCRCQ       'j'       /* CRC next, frame continues, ZACK expected */
#define ZCRCW       'k'       /* CRC next, ZACK expected, end of frame */
#define ZRUB0       'l'       /* translate to DEL '\x7f' */
#define ZRUB1       'm'       /* translate to DEL '\xff' */
#define GOTCRCE (ZCRCE|256)   /* ZDLE-ZCRCE received */
#define GOTCRCG (ZCRCG|256)   /* ZDLE-ZCRCG received */
#define GOTCRCQ (ZCRCQ|256)   /* ZDLE-ZCRCQ received */
#define GOTCRCW (ZCRCW|256)   /* ZDLE-ZCRCW received */
#define GOTCAN     (256)      /* got 5 ZDLEs */

/*--------------------------------------------+
|  zmodem frame types                         |
+--------------------------------------------*/
#define ZRQINIT      0        /* request receiver init */
#define ZRINIT       1        /* receiver init */
#define ZSINIT       2        /* sender init sequence (optional) */
#define ZACK         3        /* zmodem ACK */
#define ZFILE        4        /* file name from sender */
#define ZSKIP        5        /* to sender: skip this file */
#define ZNAK         6        /* zmodem NAK */
#define ZABORT       7        /* abort batch transfers */
#define ZFIN         8        /* finish session */
#define ZRPOS        9        /* resume data trans at this position */
#define ZDATA       10        /* data packet(s) follow */
#define ZEOF        11        /* end of file */
#define ZFERR       12        /* disk I/O error detected */
#define ZCRC        13        /* request for file CRC and response */
#define ZCHALLENGE  14        /* receiver's challenge */
#define ZCOMPL      15        /* request is complete */
#define ZCAN        16        /* other end canned session with CAN*5 */
#define ZFREECNT    17        /* request for free bytes on filesystem */
#define ZCOMMAND    18        /* command from sending program */
#define ZSTDERR     19        /* output to standard error, data follows */

/*--------------------------------------------+
|  Bit Masks for ZRINIT ZF0 flags             |
+--------------------------------------------*/
#define CANFDX     0x01       /* receiver can send and receive true FDX */
#define CANOVIO    0x02       /* receiver can receive during disk I/O */
#define CANBRK     0x04       /* receiver can send a break signal */
#define CANCRY     0x08       /* receiver can decrypt */
#define CANLZW     0x10       /* receiver can uncompress */
#define CANFC32    0x20       /* receiver can use 32 bit Frame Check */
#define ESCCTL     0x40       /* receiver expects ctl chars to be escaped */
#define ESC8       0x80       /* receiver expects 8th bit to be escaped */

/*--------------------------------------------+
|  Bit Masks for ZSINIT ZF0 flags             |
+--------------------------------------------*/
#define TESCCTL    0x40       /* tx expects ctl chars to be escaped */
#define TESC8      0x80       /* tx expects 8th bit to be escaped */

/*--------------------------------------------+
|  Bit masks for ZFILE header flags           |
+--------------------------------------------*/
 /*++ ZF0 flag, conversion options ++*/
#define ZCBIN        1      /* binary transfer - inhibit conversion */
#define ZCNL         2      /* convert NL to local end of line convention */
#define ZCRESUM      3      /* resume interrupted file transfer */
 /*++ ZF1 flag, management options ++*/
#define ZMSKNOLOC  0x80     /* skip file if not present at rx */
#define ZMMASK     0x1f     /* mask for the choices below */
#define ZMNEWL       1      /* transfer if source newer or longer */
#define ZMCRC        2      /* transfer if different file CRC or length */
#define ZMAPND       3      /* append contents to existing file (if any) */
#define ZMCLOB       4      /* replace existing file */
#define ZMNEW        5      /* transfer if source newer */
#define ZMDIFF       6      /* transfer if dates or lengths different */
#define ZMPROT       7      /* protect destination file */

/*--------------------------------------------+
|  Error return values                        |
+--------------------------------------------*/
#define GARB_COUNT     (-12)    /* header not found on call to ZRecvHdr */
#define LONG_PKT       (-11)    /* exceeded expected block length */
#define GARBLED_PKT    (-10)    /* unexpected data/invalid ZDLE sequence */
#define BAD_CRC         (-9)    /* CRC incorrect */
#define TIMED_OUT       (-8)    /* timed out on character receive */

#define SYNC_ERR        (-6)    /* couldn't synchronize file positions */
#define MEM_ERR         (-5)    /* memory allocation error */
#define FILE_ERR        (-4)    /* DOS read/write error */
#define LOST_CARRIER    (-3)    /* lost carrier */
#define REM_ABORT       (-2)    /* aborted by remote */
#define LCL_ABORT       (-1)    /* aborted by local operator */

/*/////////////////////////////////////////////////////////
//                                                       //
//  zmodem definitions specific to this implementation   //
//                                                       //
//////////////////////////////////////////////////////// */
/*--------------------------------------------+
|  zmodem message types                       |
+--------------------------------------------*/
#define M_RHDR      0       /* received header */
#define M_SHDR      1       /* sent header */
#define M_BLK0      2       /* block 0 data processed (name, size, etc.) */
#define M_CLASH     3       /* file name clash occurred (use ExistOpts) */
#define M_FILE      4       /* start of transfer, FilePos = 1st position */
#define M_EOF       5       /* end of transfer (1 file) */
#define M_DATA      6       /* sent or received file data packet */
#define M_FLOW      7       /* change in XOFF or CTS flow status */
#define M_IDLE      8       /* waiting for character or for tx to empty */
#define M_RESET     9       /* reset to 'first file' condition */

/*--------------------------------------------+
|  zmodem header (i86 byte order dependent)   |
+--------------------------------------------*/
typedef union
{
    long Data;                  /* generic reference to all bytes */
    long Pos;                   /* reference as a file position value */
    WORD BufSz;                 /* receiver buffer size for ZRINIT */
    BYTE Flg[4];                /* reference to individual flag values */
} ZHEADER;
enum zhdrflag { F3, F2, F1, F0 };

/*--------------------------------------------+
|  bit mapped zmodem program control flags    |
+--------------------------------------------*/
typedef union
{
    int Data;                   /* reference to all bits at once */
    struct                      /* reference to individual fields */
    {
        unsigned ExistOpts : 3; /* 0=skip, 1=resume, 2=makedup, 3=replace,
                                   4=abort transfer */
        unsigned KeepTime  : 1; /* use mod time in block 0 header */
        unsigned EscCtl    : 1; /* escape txd ctl chars even if not req */
        unsigned IgnCarrier: 1; /* ignore absence of carrier */
        unsigned EndOfFile : 1; /* end of file encountered */
        unsigned Receiving : 1; /* receiving if set, sending if reset */

        unsigned KBlocks   : 2; /* 0 = default (128 if X, 1024 if Y) */
                                /* 1 = force 128, 2 = force 1024 */
        unsigned G         : 1; /* 1 = "g" mode, 0 = ACK mode */
        unsigned CkSum     : 1; /* 1 = checksum block check, 0 = crc */
        unsigned FullPath  : 1; /* 1 = send/accept complete path */
        unsigned KeepAtrib : 1; /* 1 = send/accept file attributes */
        unsigned SendHid   : 1; /* 1 = send hidden files */
        unsigned HideSubs  : 1; /* 1 = hide created SUBDIRs with xff or % */
    } F;
} TFLAGS;

/*/////////////////////////////////////////////////////////
//                                                       //
//  async and timer specific functions                   //
//                                                       //
//////////////////////////////////////////////////////// */
#define RxChar()          ((--RxBytesQd < 0) ? LoadRxQueue() : *RxQPtr++)
#define FlushTxBuf()      async_txflush(tPort)
#define FlushRxBuf()      RxBytesQd = 0, async_rxflush(tPort)
#define TxChar(c)         async_tx(tPort, (c))
#define TxBlock(blk, n)   async_txblk(tPort, (blk), (n))
#define RxBufEmpty()      ((RxBytesQd + async_rxcnt(tPort)) == 0)
#define Delay(t)          tdelay(t)

/*/////////////////////////////////////////////////////////
//                                                       //
//  parse file list structure/definitions                //
//                                                       //
//////////////////////////////////////////////////////// */
/*--------------------------------------------+
|  parse file list structure                  |
+--------------------------------------------*/
typedef struct
{
    char *lptr;             /* pointer to current positon in list */
    DF findstruc;           /* dos findfirst/findnext structure */
    char findatrib;         /* search attribute */
    int flags;              /* control flags for ParseFileList function */
} FILELIST;

/*--------------------------------------------+
|  parse file list definitions                |
+--------------------------------------------*/
#define F_NOFILE      0     /* no matching files */
#define F_RDWRITE     1     /* found file, has r/w attribute */
#define F_RDONLY      2     /* found file, has read only attribute */
#define F_SUBDIR      3     /* found subdirectory */
#define F_SYSTEM      4     /* found system or volid type file */
#define F_NOWILD    0x01    /* error if list contains wild cards */
#define F_NOLIST    0x02    /* error if list contains multiple names */
#define F_NOSYS     0x04    /* don't find subdir, hidden, system, or volid */
#define F_UNIQUE    0x08    /* return duplicate name if file exists */
#define F_SLASH     0x10    /* separate pathnames with forward slashes */
#define F_LOWER     0x20    /* return lower case filename */
#define F_FIRST     0x40    /* NZ flag value if no other options used */

/*/////////////////////////////////////////////////////////
//                                                       //
//  global variables                                     //
//                                                       //
//////////////////////////////////////////////////////// */
/*--------------------------------------------+
|  initialized global variables               |
+--------------------------------------------*/
extern char *DfltPath;          /* default download path */
extern long ConnectBaud;        /* set to connect rate if using fixed link */
extern int RxTimeOut;           /* receive char timeout in ticks (1/18 s.) */
extern int DefRxTimeOut;        /* default value for RxTimeOut */
extern int XonTimeOut;          /* max ticks before sending XON for RxChar */
extern int XoffTimeOut;         /* max ticks before override for rx'd XOFF */
extern int TxWindow;            /* max chars in transit, 0 if unlimited */
extern int VBufSize;            /* size of variable disk buffer to use */
extern TFLAGS TFlag;            /* transfer flags (resume,keeptime,ign car) */
extern WORD Crc16Tbl[256];      /* 16 bit CRC table */
extern BYTE ZTable[256];        /* used to speed up ZRxChar & ZTxChar */

 /*++ zrinit local receive buffer size, capability flags ++*/
extern WORD BufSize;            /* 0 if overlaying I/O, else <= VBufSize */
extern BYTE ZFR0;
extern BYTE ZFR1;

 /*++ zsinit flags ++*/
extern BYTE ZFS0;               /* don't need control characters esc'd */

 /*++ zfile management flags ++*/
extern ZHEADER ZFMgt;               /* not requesting any file management */
extern ZHEADER ZFMgtMask;           /* mask for incoming file mgt requests */

/*--------------------------------------------+
|  uninitialized global variables             |
+--------------------------------------------*/
extern ASYNC *tPort;            /* pointer to port structure */
extern char *PathName;          /* full pathname of current file */
extern char *NameExt;           /* name and extension only of current file */
extern long FilePos;            /* current file offset */
extern long FileSize;           /* length of current file */
extern long RxdFileSize;        /* file size from block 0 information */
extern DWORD UnixTime;          /* current file time in Unix format */
extern DWORD RxdUnixTime;       /* file time from block 0 information */
extern WORD DosTime, DosDate;   /* current file time in DOS format */
extern WORD DosAtrib;           /* file attribute of current file */
extern long TotalBytes;         /* total bytes for all files */
extern int TotalFiles;          /* total number of files */
extern WORD SerNbr;             /* serial number of remote program */
extern int BinHdr;              /* ZBIN32 if using CRC32, ZBIN if CRC16 */
extern char rData32;            /* use 32 bit CRC for rx'd data packets */
extern char tData32;            /* use 32 bit CRC for tx'd data packets */
extern int BlkLen;              /* current block length */
extern ZHEADER ZFOpts;          /* option flags received in last ZFILE hdr */
extern ZHEADER ZHdr;            /* I/O buf for zmodem header data */
extern WORD ErrCnt;             /* number of errors on current file */
extern int BytesRxd;            /* # bytes in last data packet received */
extern BYTE *IOBuf;             /* I/O buffer */
extern BYTE RxQueue[128];       /* receive queue */
extern BYTE *RxQPtr;            /* pointer to current RxQueue position */
extern int RxBytesQd;           /* bytes in receive queue */

/*/////////////////////////////////////////////////////////
//                                                       //
//  function declarations                                //
//                                                       //
//////////////////////////////////////////////////////// */
void   _C_ AsyncMode(int mode);
WORD   _C_ Blk16Crc(BYTE *block, int bytes);
int    _C_ BuildYZBlk0(void);
void   _C_ CloseFile(void);
int    _C_ DosFindFirst(char *pathname, int atrib, DF *fstruc);
int    _C_ DosFindNext(DF *fstruc);
long   _C_ DosGetDiskFree(int drive);
int    _C_ DosSetFileTime(int handle, unsigned date, unsigned time);
int    _C_ DosSetFileAttr(char *path, unsigned attrib);
void   _C_ FlushDisk(void);
char * _C_ GetNameExt(char *FullPath);
int    _C_ GetTotalFiles(char *list, int flags);
int    _C_ LoadRxQueue(void);
int    _C_ OpenFile(char *name, char *mode);
int    _C_ ParseFileList(FILELIST *f, int flags, ...);
void   _C_ ParseYZBlk0(char *blk0);
int    _C_ ReadDisk(int bytes);
int    _C_ Seek(long pos);
void   _C_ SendBreak(void);
char * _C_ SkipChars(char *str);
char * _C_ SkipSpaces(char *str);
char * _C_ SwapChars(char *str, char find, char replace);
int    _C_ WaitTxLevel(int level);
int    _C_ WriteDisk(int bytes);
int    _C_ XYInitVars(ASYNC *comport);

DWORD  _C_ DosToUnix(WORD time, WORD date);
void   _C_ UnixToDos(WORD *time, WORD *date, DWORD unixtime);

DWORD  _C_ ulmul(DWORD n1, DWORD n2);
DWORD  _C_ uldiv(DWORD n1, DWORD n2);
DWORD  _C_ ulmod(DWORD n1, DWORD n2);
DWORD  _C_ ulshl(DWORD n1, int shiftval);

int    _C_ ZmodemRecv(ASYNC *comport);
int    _C_ ZmodemSend(ASYNC *comport, char *files);
void   _C_ ZMsg(int type, ...);

#ifdef __cplusplus
  }
#endif

