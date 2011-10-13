void DispLog(void) {
	char c=0;
	long int curr_loc;
	register int j;
	curr_loc=log-4000;
	cls();
	while(c!=27) {
		if(color_flag==1) ScreenMemory=MK_FP(0xb800,160);
		else ScreenMemory=MK_FP(0xb000,160);
		if(curr_loc<0) {
			if(mintraverse==1) curr_loc+=64000;
			else curr_loc=0;
		}
		if(curr_loc>64000) curr_loc-=64000;
		for(j=0;j<4000;j++) *ScreenMemory++=*(baselog+curr_loc+j);
		if(c==0) { switch(getch()) {
			case 72: curr_loc-=160; break;
			case 80: curr_loc+=160; break;
			case 73: curr_loc-=4000; break;
			case 81: curr_loc+=4000; break;
		}}
		c=getch();
	}
}
