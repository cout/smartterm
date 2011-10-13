#include <dos.h>

#define outpw outport

#define DOTRATE0 25.175E6
#define DOTRATE1 28.322E6

int far *ADDR_6845=(int far *)0x00400063;
union REGS u;

#define SetCRTCReg(r,v) outpw(*ADDR_6845,(v<<8)|r)
#define SetSeqReg(r,v) outpw(0x3c4,(v<<8)|r)

GetCRTCReg(int r) {
	outp(*ADDR_6845,r);
	return inp(*ADDR_6845+1);
}

SetATCReg(int r, int v) {
	u.x.ax=0x1000;
	u.h.bh=v;
	u.h.bl=r;
	int86(0x10,&u,&u);
}

VRWait() {
	register int CRTStatusPort=*ADDR_6845+6;

	while((inp(CRTStatusPort)&8)!=0);
	while((inp(CRTStatusPort)&8)==0);
}

main(){
	set28mode();
}

set28mode() {
	float DotRate;
	float LineRate;
	float FrameRate;
	float HSyncDuration=3.813E-6;
	int CharWidth=8, CharHeight=14;

	unsigned int far *CRT_COLS=(int far *)0x0040004A;
	unsigned int far *CRT_LEN=(int far *)0x0040004C;
	unsigned char far *ROWS=(char far *)0x00400084;

	int Cols=80;
	int HSyncTime;
	int HOverscan=8;
	int HTotal;
	int HAdjust=0;
	int i,j;

	int CRTCval[0x19];

	u.x.ax=3; int86(0x10,&u,&u);
	u.x.ax=0x1101; u.x.bx=0; int86(0x10,&u,&u);
	u.x.ax=0x1110; u.h.bh=14; u.h.bl=0;
	u.x.cx=0; u.x.dx=0; int86(0x10,&u,&u);
	outp(0x3c4,1); i=inp(0x3c5);
	i=(i&0xfe)|((8==8)?1:0);
	VRWait();
	SetSeqReg(0,1); SetSeqReg(1,i); SetSeqReg(0,3);
	j=((CharWidth==8)?0:8); SetATCReg(0x13,j);

	DotRate=(((inp(0x3cc) & 0x0c) == 0) ? DOTRATE0 : DOTRATE1);
	HSyncTime=((DotRate*HSyncDuration)/(float)CharWidth)+0.5;
	HTotal=Cols+HSyncTime+HOverscan;

	for(i=0;i<=0x18;i++) CRTCval[i]=GetCRTCReg(i);

	CRTCval[0]=HTotal-5;
	CRTCval[1]=Cols-1;
	CRTCval[2]=Cols;
	CRTCval[3]=((HTotal-2)&0x1f) | 0x80;
	CRTCval[4]=Cols+HAdjust+4;
	CRTCval[5]=((CRTCval[4]+HSyncTime)&0x1f)|(((HTotal-2)&0x20)<<2);
	CRTCval[0x13]=Cols/2;

	CRTCval[0x11]&=0x7f; SetCRTCReg(0x11,CRTCval[0x11]);
	for(i=0;i<=0x18;i++) SetCRTCReg(i,CRTCval[i]);
	CRTCval[0x11]|=0x80; SetCRTCReg(0x11,CRTCval[0x11]);
	SetATCReg(0x11,1);

	*CRT_COLS=Cols; *CRT_LEN=(((*ROWS+1)*Cols*2)+0xff)&0xff00;

	cprintf("\n\rDisplayed resolution is %d by %d characters",*CRT_COLS,*ROWS+1);
	cprintf("\n\rDisplayed character matrix is %d by %d pixels",CharWidth,CharHeight);
	getch();
}