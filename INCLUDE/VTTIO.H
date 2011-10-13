char tabs[80];

void cls() {
	textattr(7);
	clrscr();
	if(!color_flag) color28();
	/*window(0,0,80,28);*/
}

void DoTab() {
	register int i;

	for(i=wherex()+1;i<=80;i++) {
		if(tabs[i]==1) {
			gotoxy(i,wherey());
			return;
		}
	}
}

static void VTBell() {
	sound(880);
	delay(125);
	nosound();
}

#ifndef Scrolls
void ScrollUp() {
	movetext(1,top_scroll+1,80,bottom_scroll,1,top_scroll);
}

void ScrollDown() {
	movetext(1,top_scroll,80,bottom_scroll-1,1,top_scroll+1);
}
#endif

void SetTabStop() {
	tabs[wherex()]=1;
}

void ClearTabStop() {
	tabs[wherex()]=0;
}

void ClearAllTabs() {
	int j;
	for(j=1;j<=80;j++){
		tabs[j]=0;
		if(((j-1)%8)==0) tabs[j]=1;
	}
}
