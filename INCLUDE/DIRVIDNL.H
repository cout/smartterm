int textatt=7;
union REGS r;
unsigned char far *ScreenMemory;
unsigned char far *log, *maxlog, *baselog;
char mintraverse=0;
int locx=1, locy=1, rlocy=1;
char buffer[4000];
int temp_top, temp_bot;
int temp_x, temp_y;

#define Scrolls

void SetPage(char page) {
	r.h.ah=5;
	r.h.al=page;
	int86(0x10,&r,&r);
}

void ScrollUp() {
	r.h.ah=6;
	r.h.al=1;
	r.h.bh=textatt;
	r.h.ch=top_scroll-1;
	r.h.cl=0;
	r.h.dh=bottom_scroll;
	r.h.dl=79;
	int86(0x10,&r,&r);
}

void ScrollDown() {
	r.h.ah=7;
	r.h.al=1;
	r.h.bh=textatt;
	r.h.ch=top_scroll-1;
	r.h.cl=0;
	r.h.dh=bottom_scroll;
	r.h.dl=80;
	int86(0x10,&r,&r);
}

void locate(int x, int y) {
	if(x>80) x=80;
	if(y>bottom_scroll) y=bottom_scroll;
	if(x<1) x=1;
	r.h.ah=2;
	r.h.bh=0;
	rlocy=r.h.dh=y+top_scroll-2;
	if(r.h.dh>bottom_scroll) r.h.dh=bottom_scroll;
	r.h.dl=x-1;
	int86(0x10,&r,&r);
	locx=x; locy=y;
}

void SaveScreen() {
	int j;
	if(color_flag==1) ScreenMemory=MK_FP(0xb800,0);
	else ScreenMemory=MK_FP(0xb000,0);
	for(j=0;j<4480;j++) buffer[j]=*ScreenMemory++;
	temp_top=top_scroll; temp_bot=bottom_scroll;
	temp_x=locx; temp_y=locy;
}

void RestoreScreen() {
	int j;
	if(color_flag==1) ScreenMemory=MK_FP(0xb800,0);
	else ScreenMemory=MK_FP(0xb000,0);
	for(j=0;j<4480;j++) *ScreenMemory++=buffer[j];
	top_scroll=temp_top; bottom_scroll=temp_bot;
	locate(temp_x, temp_y);
}

int findx() {
	return locx;
}

int findy() {
	return locy;
}

void clrs() {
	r.h.ah=6;
	r.h.al=bottom_scroll;
	r.h.bh=7;
	r.h.ch=top_scroll-1;
	r.h.cl=0;
	r.h.dh=bottom_scroll;
	r.h.dl=79;
	int86(0x10,&r,&r);
	locate(1,top_scroll-1);
}

void wrch(char c, int num) {
	register int j;
	if(color_flag==1) ScreenMemory=
		MK_FP(0xb800,2*(locx-1)+160*rlocy);
	else ScreenMemory=
		MK_FP(0xb000,2*(locx-1)+160*rlocy);
	for(j=1;j<=num;j++) {
		*ScreenMemory++=*log++=c;
		*ScreenMemory++=*log++=textatt;
		if(log==maxlog) maxlog++;
		if(baselog+64000<=maxlog) {
			maxlog=baselog;
			mintraverse=1;
		}
		if(baselog+64000==log) log=baselog;
	}
}

void wch(char c) {
	register int x,y;
	switch(c) {
		case '\n':
			if(locy>=bottom_scroll) ScrollUp();
			else locate(locx,locy+1);
			log+=80;
			if(log>maxlog) maxlog=log;
			if(baselog+64000==maxlog) {
				maxlog=baselog;
				mintraverse=1;
			}
			if(baselog+64000==log) log=baselog;
			break;
		case '\r':
			locate(1,locy);
			break;
		case '\b':
			if((x=locx)>1) {locate(x-1,locy); log--;}
			break;
		default:
			if(color_flag==1) ScreenMemory=
				MK_FP(0xb800,2*(locx-1)+160*rlocy);
			else ScreenMemory=
				MK_FP(0xb000,2*(locx-1)+160*rlocy);
			*ScreenMemory++=*log++=c;
			*ScreenMemory++=*log++=textatt;
			if(log>maxlog) maxlog=log;
			if(baselog+64000==maxlog) {
				maxlog=baselog;
				mintraverse=1;
			}
			if(baselog+64000==log) log=baselog;
			if(locx>=80) { wch('\n'); wch('\r');}
			else {locate(locx+1,locy);}
	}
}

void clreol_() {
	wrch(' ',80-locx);
}

void clreos_() {
	register int x;
	x=locx;
	wrch(' ',80-locx);
	r.h.ah=7;
	r.h.al=bottom_scroll;
	r.h.bh=7;
	r.h.ch=locy;
	r.h.cl=0;
	r.h.dh=bottom_scroll-1;
	r.h.dl=80;
	int86(0x10,&r,&r);
	locx=x;
}

void putstr(char *str) {
	while(*str) wch(*str++);
}

#define cputs       putstr
#define textattr(c) textatt=c
#define clreol      clreol_
#define clreos      clreos_
#define putch(c)    wch(c)
#define gotoxy(x,y) locate(x,y)
#define wherex      findx
#define wherey      findy
#define window(a,b,c,d) top_scroll=b+1; bottom_scroll=d
#define clrscr      clrs
