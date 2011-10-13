int cursx=1,cursy=1,curra=7;

void SetCursor(unsigned char x, unsigned char y) {
	union REGS u;
	u.h.ah=2;
	u.h.bh=0;
	u.h.dh=x-1;
	u.h.dl=y-1;
	int86(0x10,&u,&u);
	cursx=x; cursy=y;
}

void PutCharacter(char c) {
	union REGS u;
	u.h.ah=0x13;
	u.h.al=1;
	u.h.bh=0;
	u.h.bl=curra;
	u.x.cx=1;
	u.h.dh=cursx-1;
	u.h.dl=cursy-1;
	int86(0x10,&u,&u);
	u.h.ah=3;
	u.h.bh=0;
	int86(0x10,&u,&u);
	cursx=u.h.dh+1;
	cursy=u.h.dl+1;
}

void PutString(char *s) {
	int j;
	char *c;
	while((*c=s)!='\0') PutCharacter(*c);
}

#define putch(c) PutCharacter(c)
#define wherex() cursx
#define wherey() cursy
#define cputs(s) PutString(s)
#define gotoxy(x,y) SetCursor(x,y)