#define writecomm

#include <dos.h>
#include <stdio.h>
#include "keyio.h"

main() {
	int c;
top:
	if((c=GetKey())==F10) return;
	printf("%X\n",c);
	goto top;
}

static void SendBksp() {return;}
