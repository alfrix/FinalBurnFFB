// Out Run -----------------------------------------
#include "../sys16/sys.h"

// out_draw.cpp
int OutDrawDo();

// out_init.cpp
int OutInit();
int OutExit();

// out_mem.cpp
extern unsigned char *OutRom1,*OutRom2,*OutZ80;
extern unsigned char *OutRam06,*OutRam10,*OutRam12,*OutRam13;
extern unsigned char *OutRam26,*OutRam28,*OutRam29;
int OutScan(int Action,int *pMin);
void OutScode(int d);
int OutMemInit();
int OutMemExit();

// out_run.cpp
extern unsigned char OutReset;
extern unsigned char OutButton[];
extern unsigned char OutComb; // Above compiled into a byte
extern unsigned char OutAxis[];
extern unsigned char OutDip[];
extern unsigned char OutGear;
int OutRunReset();
int OutFrame();

// out_gr.cpp
extern unsigned char *OutGrBmap; // Ground birmap length 0x20000 bytes
extern unsigned char *OutGrRam;  // Ground ram length 0x1000 bytes
extern unsigned char *OutGrFlag; // Flag ram length 2 bytes
int OutGrDraw();
