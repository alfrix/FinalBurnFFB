// Z80 (Zed Eight-Ty) Interface
#include "burnint.h"

unsigned char (*ZetRead)(unsigned short a)=NULL;         // Mem read
void (*ZetWrite)(unsigned short a,unsigned char d)=NULL; // Mem Write
unsigned char (*ZetIn)(unsigned short a)=NULL;           // Port read
void (*ZetOut)(unsigned short a,unsigned char d)=NULL;   // Port Write

#ifdef EMU_DOZE
extern "C" unsigned char __cdecl DozeRead(unsigned short a) { return ZetRead(a); }
extern "C" void __cdecl DozeWrite(unsigned short a, unsigned char d) { ZetWrite(a,d); }
extern "C" unsigned char __cdecl DozeIn(unsigned short a) { return ZetIn(a); }
extern "C" void __cdecl DozeOut(unsigned short a, unsigned char d) { ZetOut(a,d); }

int nDozeRunTotal=0; // While inside ZetRun(), the total cycles we should be doing
#endif

int ZetInit()
{
  if (ZetRead ==NULL) return 1;
  if (ZetWrite==NULL) return 1;
  if (ZetIn   ==NULL) return 1;
  if (ZetOut  ==NULL) return 1;
  return 0;
}

// This function will make an area callback ZetRead/ZetWrite
// Must be called between MemBegin and End
int ZetMemCallback(int nStart,int nEnd,int nMode)
{
#ifdef EMU_DOZE
  int i;
  nStart>>=8; nEnd+=0xff; nEnd>>=8;

  // Leave the section out of the memory map, so the Doze* callback with be used
  for (i=nStart;i<nEnd;i++)
  {
         if (nMode==0) DozeMemRead [i]=NULL;
    else if (nMode==1) DozeMemWrite[i]=NULL;
    else if (nMode==2) DozeMemFetch[i]=NULL;
  }
  return 0;
#endif

}

int ZetMemEnd()
{
  return 0;
}

int ZetExit()
{
  return 0;
}


int ZetMapArea(int nStart,int nEnd,int nMode,unsigned char *Mem)
{
#ifdef EMU_DOZE
  int i,s,e;
  s=nStart>>8; e=(nEnd+0xff)>>8;

  // Put this section in the memory map, giving the offset from Z80 memory to PC memory
  for (i=s;i<e;i++)
  {
         if (nMode==0) DozeMemRead [i]=(unsigned int)(Mem-nStart);
    else if (nMode==1) DozeMemWrite[i]=(unsigned int)(Mem-nStart);
    else if (nMode==2) DozeMemFetch[i]=(unsigned int)(Mem-nStart);
  }
  return 0;
#endif
}

int ZetReset()
{
#ifdef EMU_DOZE
  DozeReset();
#endif
  return 0;
}

int ZetPc()
{
#ifdef EMU_DOZE
  return Doze.pc;
#endif
}

int ZetScan(int nAction)
{
#ifdef EMU_DOZE
  (void)nAction;
  SCAN_VAR(Doze)
  return 0;
#endif
}
