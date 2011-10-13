int textatt=7;
union REGS r;

#define Scrolls

void ScrollUp() {
	r.h.ah=6;
	r.h.al=1;
	r.h.bh=textatt;
	r.h.ch=top_scroll-1;
	r.h.cl=0;
	r.h.dh=bottom_scroll-1;
	r.h.dl=79;
	int86(0x10,&r,&r);
}

void ScrollDown() {
	r.h.ah=7;
	r.h.al=1;
	r.h.bh=textatt;
	r.h.ch=top_scroll-1;
	r.h.cl=0;
	r.h.dh=bottom_scroll-1;
	r.h.dl=80;
	int86(0x10,&r,&r);
}

void locate(int x, int y) {
	r.h.ah=2;
	r.h.bh=0;
	r.h.dh=y+top_scroll-2;
	r.h.dl=x-1;
	int86(0x10,&r,&r);
}

int findx() {
	r.h.ah=3;
	r.h.bh=0;
	int86(0x10,&r,&r);
	return r.h.dl+1;
}

int findy() {
	r.h.ah=3;
	r.h.bh=0;
	int86(0x10,&r,&r);
	return r.h.dh+1;
}

void clrs() {
	r.h.ah=6;
	r.h.al=bottom_scroll;
	r.h.bh=7;
	r.h.ch=top_scroll-1;
	r.h.cl=0;
	r.h.dh=bottom_scroll-1;
	r.h.dl=79;
	int86(0x10,&r,&r);
	locate(1,top_scroll);
}

void wrch(char c, int num) {
	int x,y;
	switch(c) {
		case '\n':
			y=findy();
			if(y>=bottom_scroll) ScrollUp();
			else locate(findx(),y+1);
			break;
		case '\r':
			locate(1,findy());
			break;
		case '\b':
			if((x=findx())>1) locate(x-1,findy());
			break;
		default:
			r.h.ah=9;
			r.h.bh=0;
			r.h.bl=textatt;
			r.h.al=c;
			r.x.cx=num;
			int86(0x10,&r,&r);
			x=findx();
			if(x==80) { wrch('\n',1); wrch('\r',1);}
			else {locate(x+1,findy());}
	}
}

void clreol_() {
	wrch(' ',80-findy());
}

void clreos_() {
	wrch(' ',80-findx());
	r.h.ah=7;
	r.h.al=bottom_scroll;
	r.h.bh=textatt;
	r.h.ch=findy()-1;
	r.h.cl=0;
	r.h.dh=bottom_scroll-1;
	r.h.dl=80;
	int86(0x10,&r,&r);
}

#define textattr(c) textatt=c
#define clreol      clreol_
#define clreos      clreos_
#define putch(c)    wrch(c,1)
#define gotoxy(x,y) locate(x,y)
#define wherex      findx
#define wherey      findy
#define window(a,b,c,d) top_scroll=b; bottom_scroll=d
#define clrscr      clrs
