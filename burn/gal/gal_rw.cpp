#include "gal.h"
// Galaxy Force HW - Read/Write module

unsigned char GalButton[8]={0,0,0,0,0,0,0,0};
unsigned char GalInput=0,GalGear=0,GalLastGear=0;
unsigned char GalAnaVal[GAL_ANALOG_COUNT]; // Converted analog values
unsigned char GalAnalog[GAL_ANALOG_COUNT]={0x80,0x80,0x80,0x80,0x80,0x80};

void GalInputPrepare()
{
  switch (GalGame&0xf0)
  {
    case 0x10: // Galaxy Force
      GalAnaVal[1]=(unsigned char)(0x100-GalAnaVal[1]);
    break;
    case 0x20: // Power Drift
      GalAnaVal[5]=(unsigned char)(0x30+(GalAnaVal[5]*0xa/0x10));
    break;
    case 0x40: // G-Loc
    case 0x50: // Strike fighter
      GalAnaVal[3]=(unsigned char)(0x100-GalAnaVal[3]);
    break;
  }
}

void GalInputMemory()
{
  unsigned char *pm=NULL; int i=0;
  switch (GalGame&0xf0)
  {
    case 0x20: case 0x40: pm=GalRam0C+0xff22; break; // Power Drift/G-Loc
    default: pm=GalRam0C+0x00ca; break; // Galaxy Force/Rail Chase
  }
  if (pm==NULL) return;
  for (i=0;i<GAL_ANALOG_COUNT;i++) pm[i^1]=GalAnaVal[i];
}

// Input at 100000 in all cpus
static INLINE unsigned char GalInpRead(unsigned int a)
{
  a&=0xffff;
  if (a==0x3) return (unsigned char)~GalInput;
  if (a==0xb) return (unsigned char)~GalDip[1];
  if (a==0xd) return (unsigned char)~GalDip[0];
  return 0xff;
}

static INLINE void GalInpWrite(unsigned int a,unsigned char d)
{
  (void)d;
  a&=0xffff;
}

// Cpu M -------------------------------
unsigned char GalMReadByte(unsigned int a)
{
  unsigned char d=0;
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) { d=CalcReadByte(0,a); goto End; }// math chip
  if (ab==0x10) { d=GalInpRead(a); goto End; } // Input

  End: return d;
}

static void GalMWriteByte(unsigned int a,unsigned char d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x10) { GalInpWrite(a,d); return; } // Input
  if (ab==0x19) { BsysPalWriteByte(a,d); return; } // palette write

}

static unsigned short GalMReadWord(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) return CalcRead(0,a); // math chip
  SEK_DEF_READ_WORD(a)
}

static void GalMWriteWord(unsigned int a,unsigned short d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) { CalcWrite(0,a,d); return; } // math chip
  if (ab==0x19) { BsysPalWriteWord(a,d); return; } // palette write
  SEK_DEF_WRITE_WORD(a,d)
}
// Cpu X -------------------------------
static unsigned char GalXReadByte(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) return CalcReadByte(1,a); // math chip
  if (ab==0x10) return GalInpRead(a);
  return 0;
}

static void GalXWriteByte(unsigned int a,unsigned char d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x10) { GalInpWrite(a,d); return; }
}

static unsigned short GalXReadWord(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) return CalcRead(1,a); // math chip
  SEK_DEF_READ_WORD(a)
}

static void GalXWriteWord(unsigned int a,unsigned short d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) { CalcWrite(1,a,d); return; } // math chip
  SEK_DEF_WRITE_WORD(a,d)
}

// Cpu Y -------------------------------
static unsigned char GalYReadByte(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) return CalcReadByte(2,a); // math chip
  if (ab==0x10) return GalInpRead(a);
  return 0;
}

static void GalYWriteByte(unsigned int a,unsigned char d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x10) { GalInpWrite(a,d); return; }
  if (a==0x82001) { GalSndCode(d); return; }
}

static unsigned short GalYReadWord(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) return CalcRead(2,a); // math chip
  SEK_DEF_READ_WORD(a)
}

static void GalYWriteWord(unsigned int a,unsigned short d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x08) { CalcWrite(2,a,d); return; } // math chip
  SEK_DEF_WRITE_WORD(a,d)
}

int GalCpuInit()
{
  SekInit(3); // Allocate 3 68000s - M,X,Y

  // Map in memory:
  // ----------------- Cpu M (0) ------------------------
  SekOpen(0);
  SekMemory(GalRomM   ,0x000000,0x03FFFF,SM_ROM); // Cpu M Rom
  SekMemory(GalRam0C  ,0x0C0000,0x0CFFFF,SM_RAM);
  SekMemory(GalRamM180,0x180000,0x1807FF,SM_RAM);
  SekMemory(GalRamM188,0x188000,0x1887FF,SM_RAM);
  SekMemory(GalRamM19 ,0x190000,0x197FFF,SM_ROM); // Read-only: write goes through palette handler
  SekMemory(GalRamMFF ,0xFF0000,0xFFFFFF,SM_RAM);
  SekExt[0].ReadByte =GalMReadByte;
  SekExt[0].WriteByte=GalMWriteByte;
  SekExt[0].ReadWord =GalMReadWord;
  SekExt[0].WriteWord=GalMWriteWord;
  SekClose();
  // ----------------- Cpu X (1) ------------------------
  SekOpen(1);
  SekMemory(GalRomX  ,0x000000,0x03FFFF,SM_ROM); // Cpu X Rom
  SekMemory(GalRam0C ,0x0C0000,0x0CFFFF,SM_RAM);
  SekMemory(GalRamX18,0x180000,0x18FFFF,SM_RAM);
  SekMemory(GalRamXFF,0xFF0000,0xFFFFFF,SM_RAM);
  SekExt[1].ReadByte =GalXReadByte;
  SekExt[1].WriteByte=GalXWriteByte;
  SekExt[1].ReadWord =GalXReadWord;
  SekExt[1].WriteWord=GalXWriteWord;
  SekClose();
  // ----------------- Cpu Y (2) ---------------------
  SekOpen(2);
  SekMemory(GalRomY  ,0x000000,0x07FFFF,SM_ROM); // Cpu Y Rom
  SekMemory(GalRam0C ,0x0C0000,0x0CFFFF,SM_RAM);
  SekMemory(GalRamYFF,0xFF0000,0xFFFFFF,SM_RAM);
  SekExt[2].ReadByte =GalYReadByte;
  SekExt[2].WriteByte=GalYWriteByte;
  SekExt[2].ReadWord =GalYReadWord;
  SekExt[2].WriteWord=GalYWriteWord;
  SekClose();
  // ------------------------------------------------
  return 0;
}
