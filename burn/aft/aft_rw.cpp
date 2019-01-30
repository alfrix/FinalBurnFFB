#include "aft.h"
// After Burner HW - Read/Write module

// Cpu 1 ----------------------------------
unsigned char AftInputSel=0;

static void DrvSndCode(unsigned char Code)
{
  // See if the code should be blocked
  if (BurnScodePass(Code)==0) return;

  BsysSndCode=Code;
  ZetNmi();
  ZetRun(300); // run the z80 to accept the code
}

static unsigned char Aft1ReadByte(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;

  if (a==0x130001)
  {
    if (AftGame==3)
    {
      // Thunder Blade
      if (AftInputSel==4)
      {
        if (AftButton[8]) return 0xff;
        if (AftButton[9]) return 0x00;
        return (unsigned char)(AftInput[5]);
      }
      if (AftInputSel==8) return (unsigned char)(0xc0-(AftInput[4]>>1));
      return (unsigned char)(0x28+((0x100-AftInput[3])*0xb/0x10));
    }
    else
    {
      // After Burner
      if (AftInputSel==4) return (unsigned char)(0xc0-(AftInput[4]>>1));
      if (AftInputSel==8)
      {
        if (AftButton[8]) return 0xff;
        if (AftButton[9]) return 0x00;
        return (unsigned char)(AftInput[5]);
      }
      return (unsigned char)(0x28+(AftInput[3]*0xb/0x10));
    }
  }

  if (a==0x140001) return 0xff; // motor response?
  if (a==0x150001) return (unsigned char)(~AftInput[0]); //input
  if (a==0x150005) return (unsigned char)(~AftInput[1]); //dip 
  if (a==0x150007) return (unsigned char)(~AftInput[2]); //dip
  return 0;
}

static void Aft1WriteByte(unsigned int a,unsigned char d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x12) { BsysPalWriteByte(a,d); return; } // palette write
  if (a==0x130001) { AftInputSel=d; return; } // Input select
  if (a==0xe8017) { DrvSndCode(d); return; } // Send sound code
}

static unsigned short Aft1ReadWord(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;
  // Handle the math chips:
  if (ab==0x0E) return CalcRead(0,a);
  if (ab==0x2E) return CalcRead(1,a);
  SEK_DEF_READ_WORD(a)
}

static void Aft1WriteWord(unsigned int a,unsigned short d)
{
  int ab; ab=(a>>16)&0xff;
  if (ab==0x12) { BsysPalWriteWord(a,d); return; } // palette write
  // Handle the math chips:
  if (ab==0x0E) { CalcWrite(0,a,d); return; }
  if (ab==0x2E) { CalcWrite(1,a,d); return; }
  SEK_DEF_WRITE_WORD(a,d)
}

// Cpu 2 ----------------------------------
static unsigned short Aft2ReadWord(unsigned int a)
{
  int ab; ab=(a>>16)&0xff;
  // Handle the math chips:
  if (ab==0x0E) return CalcRead(1,a);

  SEK_DEF_READ_WORD(a)
}

static void Aft2WriteWord(unsigned int a,unsigned short d)
{
  int ab; ab=(a>>16)&0xff;

  // Handle the math chips:
  if (ab==0x0E) { CalcWrite(1,a,d); return; }
  SEK_DEF_WRITE_WORD(a,d)
}

static unsigned char Aft2ReadByte(unsigned int a)
{
  unsigned char d=0;
  (void)a;
  return d;
}

static void Aft2WriteByte(unsigned int a,unsigned char d)
{
  (void)a; (void)d;
}

static int DrvResetCallback()
{
  // Reset instruction on cpu 1
  int Last=nSekActive;
  SekClose();
  SekOpen(1); SekReset(); SekClose(); // Reset cpu 2
  SekOpen(Last);
  return 0;
}

int AftCpuInit()
{
  SekInit(2); // Allocate 2 68000s
  SekExt[0].ResetCallback=DrvResetCallback; // Get cpu 1 reset requests

  // Map in memory:
  // ----------------- Cpu 1 ------------------------
  SekOpen(0);
  SekMemory(AftRom1 ,0x000000,0x07FFFF,SM_ROM); // cpu 1 Rom
  SekMemory(AftRam0C,0x0C0000,0x0D0FFF,SM_RAM);
  SekMemory(AftRam10,0x100000,0x101FFF,SM_RAM);
  SekMemory(AftRam12,0x120000,0x123FFF,SM_ROM); // palette Ram (read-only - write goes through handler)
  SekMemory(AftRom2 ,0x200000,0x27FFFF,SM_ROM); // cpu 2 Rom (visible from cpu 1)
  SekMemory(AftRam29,0x29C000,0x2A3FFF,SM_RAM); // shared ram with cpu 2 (read/write)
  SekMemory(AftRam2E,0x2EC000,0x2EE001,SM_RAM); // road
  SekMemory(AftRamFF,0xFF8000,0xFFFFFF,SM_RAM);
  SekExt[0].ReadByte =Aft1ReadByte;
  SekExt[0].WriteByte=Aft1WriteByte;
  SekExt[0].ReadWord =Aft1ReadWord;
  SekExt[0].WriteWord=Aft1WriteWord;
  SekClose();
  // ------------------------------------------------

  // ----------------- Cpu 2 ------------------------
  SekOpen(1);
  SekMemory(AftRom2 ,0x000000,0x07FFFF,SM_ROM); // cpu 2 Rom
  SekMemory(AftRom2 ,0x200000,0x27FFFF,SM_ROM); // (mirror)
  SekMemory(AftRam29,0x09C000,0x0A3FFF,SM_RAM); // shared ram with cpu 1 (read/write)
  SekMemory(AftRam29,0x29C000,0x2A3FFF,SM_RAM); // (mirror)
  SekMemory(AftRam2E,0x0EC000,0x0EE001,SM_RAM); // road
  SekMemory(AftRam2E,0x2EC000,0x2EE001,SM_RAM); // (mirror)
  SekExt[1].ReadWord =Aft2ReadWord;
  SekExt[1].WriteWord=Aft2WriteWord;
  SekExt[1].ReadByte =Aft2ReadByte;
  SekExt[1].WriteByte=Aft2WriteByte;
  SekClose();
  // ------------------------------------------------
  return 0;
}