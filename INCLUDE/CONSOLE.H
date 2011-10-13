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
	u.h.ah=9;
	u.h.al=c;
	u.h.bh=0;
	u.h.bl=curra;
	u.x.cx=1;
	int86(0x10,&u,&u);
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