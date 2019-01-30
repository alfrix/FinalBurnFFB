#include "out.h"
// Out Run - Memory

static unsigned char *Mem=NULL,*MemEnd=NULL;
unsigned char *OutRom1,*OutRom2,*OutZ80;
unsigned char *OutRam06,*OutRam10,*OutRam12,*OutRam13;
unsigned char *OutRam26,*OutRam28,*OutRam29;
static unsigned char AnalogSelect=0;

// This routine is called first to determine how much memory is needed (MemEnd-(unsigned char *)0),
// and then afterwards to set up all the pointers
static int MemIndex()
{
  unsigned char *Next; Next=Mem;
  OutRom1 =Next; Next+=0x040000; // [118/133 117/132]
  OutRom2 =Next; Next+=0x040000; // [ 58/ 76  57/ 75]
  OutZ80  =Next; Next+=0x010000; // z80 memory
  OutRam06=Next; Next+=0x008000; 
  OutRam10=Next; Next+=0x011000; // tile [63/65], text [64/62]
  OutRam12=Next; Next+=0x002000; // palette [95/92]
  OutRam13=Next; Next+=0x000800; // sprites
  OutRam28=Next; Next+=0x001000; // Ground data
  OutRam29=Next; Next+=0x000004; // Road on/off flags
  OutRam26=Next; Next+=0x008000; // 54/72
  Next+=4; // Padding
  MemEnd=Next;
  return 0;
}

// Scan ram
int OutScan(int Action,int *pMin)
{
  struct BurnArea ba;
  memset(&ba,0,sizeof(ba));
  if ((Action&4)==0) return 0;

  // Scan volatile ram
  if (pMin!=NULL) *pMin=0x050100; // Return minimum compatible version

  ba.Data=OutRam06; ba.nLen=0x008000; ba.szName="OutRam06"; BurnAcb(&ba);
  ba.Data=OutRam10; ba.nLen=0x011000; ba.szName="OutRam10"; BurnAcb(&ba);
  ba.Data=OutRam12; ba.nLen=0x002000; ba.szName="OutRam12"; BurnAcb(&ba);
  ba.Data=OutRam13; ba.nLen=0x000800; ba.szName="OutRam13"; BurnAcb(&ba);
  ba.Data=OutRam28; ba.nLen=0x001000; ba.szName="OutRam28"; BurnAcb(&ba);
  ba.Data=OutRam29; ba.nLen=0x000004; ba.szName="OutRam29"; BurnAcb(&ba);
  ba.Data=OutRam26; ba.nLen=0x008000; ba.szName="OutRam26"; BurnAcb(&ba);

  SCAN_VAR(OutButton[4]) // Scan gear state
  SCAN_VAR(AnalogSelect) // Scan input select

  SekScan(Action&3); // scan 68000 states
  BsysZScan(Action&3); // Scan Z80
  PcmScan(); // Scan PCM chip
  return 0;
}

static int Out1ResetCallback()
{
  // Reset instruction on cpu 1
  int Last; Last=nSekActive;
  SekClose();
  SekOpen(1); SekReset(); SekClose(); // Reset cpu 2
  SekOpen(Last);
  return 0;
}

void OutScode(int d)
{
  BsysSndCode=(unsigned char)d;
  ZetNmi();
}

static unsigned char Out1ReadByte(unsigned int a)
{
  a&=0xffffff;

  if (a<=0x03ffff) { return OutRom1[a^1]; }
  if (a>=0x060000 && a<=0x067fff) { return OutRam06[(a^1)-0x060000]; }
  if (a>=0x260000 && a<=0x267fff) { return OutRam26[(a^1)-0x260000]; }

  if (a==0x140011) return (unsigned char)(~OutComb);   // Buttons
  if (a==0x140015) return (unsigned char)(~OutDip[0]); // Dip A
  if (a==0x140017) return (unsigned char)(~OutDip[1]); // Dip B
  if (a==0x140031)
  {
    switch (AnalogSelect)
    {
      case 0x0: return OutAxis[0];
      case 0x4: return OutAxis[1];
      case 0x8: return OutAxis[2];
    }
    return 0;
  }
  return 0;
}

static void Out1WriteByte(unsigned int a,unsigned char d)
{
  int ab=(a>>16)&0xff;

  if (ab==0x12) { BsysPalWriteByte(a,d); return; } // palette write
  a&=0xffffff;

  if (a>=0x060000 && a<=0x067fff) { OutRam06[(a^1)-0x060000]=d; return; }
  if (a>=0x260000 && a<=0x267fff) { OutRam26[(a^1)-0x260000]=d; return; }

  if (a==0x140031) { AnalogSelect=d; return; }
  if (a==0xffff07) { OutScode(d); return; }
}

static void Out1WriteWord(unsigned int a,unsigned short d)
{
  int ab=(a>>16)&0xff;
  if (ab==0x12) BsysPalWriteWord(a,d); // palette write
  SEK_DEF_WRITE_WORD(a,d)
}

static unsigned char Out2ReadByte(unsigned int a)
{
  a&=0xffffff;
  if (a<=0x03ffff) { return OutRom2[a^1]; }
  if (a>=0x060000 && a<=0x067fff) { return OutRam26[(a^1)-0x060000]; }
  return 0;
}

static void Out2WriteByte(unsigned int a,unsigned char d)
{
  (void)d;
  a&=0xffffff;

  if (a>=0x060000 && a<=0x067fff) { OutRam26[(a^1)-0x060000]=d; return; }
}

int OutMemInit()
{
  int nLen=0;

  // Find out how much memory is needed
  Mem=NULL; MemIndex();
  nLen=MemEnd-(unsigned char *)0;
  Mem=(unsigned char *)malloc(nLen); if (Mem==NULL)  return 1;
  memset(Mem,0,nLen); // blank all memory
  MemIndex(); // Index the allocated memory

  SekInit(2); // Allocate 2 68000s
  SekExt[0].ResetCallback=Out1ResetCallback; // Get cpu 1 reset requests

  // Map in memory:
  // ----------------- Cpu 1 ------------------------
  SekOpen(0);
  SekMemory(OutRom1 ,0x000000,0x03ffff,SM_FETCH); // cpu 1 Rom
  SekMemory(OutRam10,0x100000,0x110fff,SM_RAM);
  SekMemory(OutRam12,0x120000,0x121fff,SM_ROM); // palette ram (read-only - write goes through handler)
  SekMemory(OutRam13,0x130000,0x1307ff,SM_RAM);
  SekMemory(OutRom2 ,0x200000,0x23ffff,SM_ROM); // cpu 2 Rom (visible from cpu 1)

  SekExt[0].ReadByte =Out1ReadByte;
  SekExt[0].WriteWord=Out1WriteWord;
  SekExt[0].WriteByte=Out1WriteByte;
  SekClose();
  // ------------------------------------------------

  // ----------------- Cpu 2 ------------------------
  SekOpen(1);
  SekMemory(OutRom2 ,0x000000,0x03ffff,SM_FETCH); // cpu 2 Rom
  SekMemory(OutRam28,0x080000,0x080fff,SM_RAM);
  SekMemory(OutRam29,0x090000,0x090000,SM_RAM);
  SekExt[1].ReadByte =Out2ReadByte;
  SekExt[1].WriteByte=Out2WriteByte;
  SekClose();
  // ------------------------------------------------

  return 0;
}

int OutMemExit()
{
  SekExit(); // Deallocate 68000s
  // Deallocate all used memory
  if (Mem!=NULL) free(Mem);  Mem=NULL;
  return 0;
}
