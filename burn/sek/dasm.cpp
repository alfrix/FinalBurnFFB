// DisAsm module
#include "../burnint.h"

extern "C" int m68k_disassemble(char* str_buff, int pc);

// Disassemble an area
int DasmArea(int Pc,int Before,int After)
{
  int i=0;
  for (i=Pc-Before;i<Pc+After; )
  {
    int Did=0; int j;
    char Buffer[128]="";

    Did=m68k_disassemble(Buffer,i);

    dprintf("%s %6x %-40.40s ",
      i==Pc?"*****":"     ",
      i,Buffer);

    dprintf ("[");
    for (j=0;j<Did;j++) dprintf ("%.2x ",SekFetch(i+j));
    dprintf ("]\n");

    i+=Did;
  }

  return 0;
}

// d68k.c callbacks
extern "C" unsigned int m68k_read_memory_8  (unsigned int a) { return SekFetch(a); }
extern "C" unsigned int m68k_read_memory_16 (unsigned int a) { return (SekFetch(a)<<8)|SekFetch(a+1); }
extern "C" unsigned int m68k_read_memory_32 (unsigned int a) { return (SekFetch(a)<<24)|(SekFetch(a+1)<<16)|(SekFetch(a+2)<<8)|SekFetch(a+3); }
