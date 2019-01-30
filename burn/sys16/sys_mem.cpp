#include "sys.h"
// System 16 - Memory

// System 16 Inputs:
static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"   ,0,BsysKey[0]+0,"p1 coin"  },
  {"P1 Start"  ,0,BsysKey[0]+4,"p1 start" },
  {"P1 Fire 1" ,0,BsysKey[1]+1,"p1 fire 1"},
  {"P1 Fire 2" ,0,BsysKey[1]+2,"p1 fire 2"},
  {"P1 Fire 3" ,0,BsysKey[1]+0,"p1 fire 3"},
  {"P1 Down"   ,0,BsysKey[1]+4,"p1 down"  },
  {"P1 Up"     ,0,BsysKey[1]+5,"p1 up"    },
  {"P1 Right"  ,0,BsysKey[1]+6,"p1 right" },
  {"P1 Left"   ,0,BsysKey[1]+7,"p1 left"  },

  {"P2 Coin"   ,0,BsysKey[0]+1,"p2 coin"  },
  {"P2 Start"  ,0,BsysKey[0]+5,"p2 start" },
  {"P2 Fire 1" ,0,BsysKey[3]+1,"p2 fire 1"},
  {"P2 Fire 2" ,0,BsysKey[3]+2,"p2 fire 2"},
  {"P2 Fire 3" ,0,BsysKey[3]+0,"p2 fire 3"},
  {"P2 Down"   ,0,BsysKey[3]+4,"p2 down"  },
  {"P2 Up"     ,0,BsysKey[3]+5,"p2 up"    },
  {"P2 Right"  ,0,BsysKey[3]+6,"p2 right "},
  {"P2 Left"   ,0,BsysKey[3]+7,"p2 left"  },

  {"Reset"     ,0,&BsysReset  ,"reset"    },
  {"Diagnostic",0,BsysKey[0]+2,"diag"     },
  {"Service"   ,0,BsysKey[0]+3,"service"  },

  {"Dip 1"     ,2,BsysDip+0   ,"dip"      },
  {"Dip 2"     ,2,BsysDip+1   ,"dip"      }
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
int BsysInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

// -----------------------------------------------------

static unsigned char BsysReadByte(unsigned int a)
{
  unsigned char d=0xff;
  a&=0xffffff;

  if (a==0xffec96)
  {
    // Golden Axe int routine at 0x3256
    d=0xff; goto End;
  }

  if ((a&0xffc000)==0xffc000) { d=BsysRam[(a^1)&0x3fff]; goto End; }

  if ((a>>16)==0xc4)
  {
    // Input read:
    switch (a&0x3006)
    {
      case 0x1000: d=BsysInp[0]; break;
      case 0x1002: d=BsysInp[1]; break;
      case 0x1004: d=BsysInp[2]; break;
      case 0x1006: d=BsysInp[3]; break;
      case 0x2000: d=BsysDip[1]; break;
      case 0x2002: d=BsysDip[0]; break;
    }
    d=(unsigned char)~d; // Reverse bits
    goto End;
  }

End: return d;
}

static void BsysWriteByte(unsigned int a,unsigned char d)
{
  a&=0xffffff;
  if ((a&0xffc000)==0xffc000) { BsysRam[(a^1)&0x3fff]=d; goto End; }
  if ((a>>16)==BsysPramBank)  { BsysPalWriteByte(a&0xfff,d); goto End; }

End: return;
}

// -----------------------------------------------------

int BsysRunInit()
{
  unsigned int a=0;

  SekInit(1); // Allocate 1 68000

  // Map in memory:
  SekOpen(0);
  SekMemory(BsysRom ,0x000000,BsysRomLen,SM_ROM);
  a=BsysTileBank<<16; SekMemory(BsysTile,a,a+0x010fff,SM_RAM); // Tile Ram
  a=BsysObjBank<<16;  SekMemory(BsysObj ,a,a+0x000fff,SM_RAM); // Obj Ram
  a=BsysPramBank<<16; SekMemory(BsysPram,a,a+0x000fff,SM_ROM); // Pal Ram (read-only, write goes through handler)

  SekExt[0].ReadByte =BsysReadByte;
  SekExt[0].WriteByte=BsysWriteByte;
  
  SekClose();
  
  BsysRunReset();

  return 0;
}

int BsysRunExit()
{
  SekExit();
  return 0;
}
