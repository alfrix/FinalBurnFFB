// Z80 (Zed Eight-Ty) Interface
#define EMU_DOZE // Use my 'Doze' Assembler Z80 emulator

#ifdef EMU_DOZE
#include "doze.h"
#endif

extern unsigned char (*ZetRead)(unsigned short a);         // Mem read
extern void (*ZetWrite)(unsigned short a,unsigned char d); // Mem Write
extern unsigned char (*ZetIn)(unsigned short a);           // Port read
extern void (*ZetOut)(unsigned short a,unsigned char d);   // Port Write

int ZetInit();
int ZetMemCallback(int nStart,int nEnd,int nMode);
int ZetMemEnd();
int ZetExit();
int ZetMapArea(int nStart,int nEnd,int nMode,unsigned char *Mem);
int ZetReset();
int ZetPc();
int ZetScan(int nAction);

#ifdef EMU_DOZE
extern int nDozeRunTotal; // While inside ZetRun(), the total cycles we should be doing
#endif

static INLINE int ZetMidCycles() // Cycles executed so far (valid during ZetRun only)
{
#ifdef EMU_DOZE
  return nDozeRunTotal-nDozeCycles;
#endif
}

static INLINE void ZetRaiseIrq(int nVal)
{
#ifdef EMU_DOZE
  nDozeInterrupt=nVal;
#endif
}

static INLINE void ZetLowerIrq()
{
#ifdef EMU_DOZE
  nDozeInterrupt=-1;
#endif
}

static INLINE void ZetNmi()
{
#ifdef EMU_DOZE
  DozeNmi();
#endif
}

static INLINE int ZetRun(int Cycles)
{
#ifdef EMU_DOZE
  int nDid=0;
  nDozeRunTotal=Cycles;
  nDozeCycles=Cycles;

  DozeRun(); nDid=ZetMidCycles();

  nDozeCycles=0;
  nDozeRunTotal=0;
  return nDid;
#endif
}
