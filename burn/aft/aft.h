
// After Burner ----------------------------------
#include "../sys16/sys.h"

// aft_gr.cpp
extern unsigned char *AftGrBmap; // Ground bitmap length 0x40000 bytes
int AftGrDraw();

// aft_mem.cpp
extern unsigned char *AftRom1,*AftRom2;
extern unsigned char *AftRam0C,*AftRam10,*AftRam12,*AftRam14,*AftRam16;
extern unsigned char *AftRam29,*AftRam2E,*AftRamFF;
int AftMemInit();
int AftMemExit();

// aft_run.cpp
extern unsigned char AftGame;
extern unsigned char AftButton[10];
extern unsigned char AftInput[6];
extern unsigned char AftReset;
int AftInit();
int AftExit();
int AftFrame();
int AftScan(int nAction,int *pnMin);
unsigned char *AftBackdropDecode(const unsigned char *base);

// aft_rw.cpp
extern unsigned char AftInputSel;
int AftCpuInit();

// aft_spr.cpp
extern unsigned char *AftSprSrc; // Original sprite table
int AftSprDraw();

// d_aft2.cpp
int Aft2InputInfo(struct BurnInputInfo *pii,unsigned int i);
