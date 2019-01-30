#include "burnint.h"
// CPS - Memory

static unsigned char *CpsMem=NULL,*CpsMemEnd=NULL;
unsigned char *CpsRam90=NULL;
unsigned char *CpsZRamC0=NULL,*CpsZRamF0=NULL;
unsigned char *CpsSavePal=NULL,*CpsSaveReg=NULL;
unsigned char *CpsRam708=NULL,*CpsReg=NULL,*CpsFrg=NULL;
unsigned char *CpsRamFF=NULL;

// This routine is called first to determine how much memory is needed
// and then to set up all the pointers.
static int CpsMemIndex()
{
  unsigned char *Next; Next=CpsMem;

    CpsRam90 =Next; Next+=0x030000; // Video Ram
    CpsRamFF =Next; Next+=0x010000; // Work Ram
    CpsReg   =Next; Next+=0x000100; // Registers

  if (Cps==2)
  {
    CpsRam708=Next; Next+=0x008000; // Obj Ram
    CpsZRamC0=Next; Next+=0x001000; // Z80 c000-cfff
    CpsZRamF0=Next; Next+=0x001000; // Z80 f000-ffff
    CpsFrg   =Next; Next+=0x000010; // 'Four' Registers (Registers at 0x400000)
  }

  CpsSavePal =Next;  Next+=0x001000; // Draw Copy of Correct Palette
  CpsSaveReg =Next;  Next+=0x000100; // Draw Copy of Correct registers
  CpsMemEnd  =Next;
  return 0;
}

static int AllocateMemory()
{
  int nLen=0;
  // Find out how much memory is needed
  CpsMem=NULL; CpsMemIndex();
  nLen=CpsMemEnd-(unsigned char *)0;
  CpsMem=(unsigned char *)malloc(nLen); if (CpsMem==NULL) return 1;
  memset(CpsMem,0,nLen); // blank all memory
  CpsMemIndex(); // Index the allocated memory
  return 0;
}

int CpsMemInit()
{
  int nRet=0;
  nRet=AllocateMemory();  if (nRet!=0) return 1;

  SekOpen(0);
  // Map in memory:
  // 68000 Rom (as seen as is, through read)
  SekMemory(CpsRom,0,nCpsRomLen-1 ,SM_READ);

  // 68000 Rom (as seen decrypted, through fetch)
  if (nCpsCodeLen>0)
  {
    // Decoded part (up to nCpsCodeLen)
    SekMemory(CpsCode,0,nCpsCodeLen-1,SM_FETCH);
  }
  if (nCpsRomLen>nCpsCodeLen)
  {
    // The rest (up to nCpsRomLen)
    SekMemory(CpsRom+nCpsCodeLen,nCpsCodeLen,nCpsRomLen-1,SM_FETCH);
  }

  if (Cps==2)
  {
    SekMemory(CpsRam708,0x708000,0x70ffff,SM_RAM); // separate Obj Ram
  }

  SekMemory(CpsRam90 ,0x900000,0x92ffff,SM_RAM); // Gfx Ram
  SekMemory(CpsRamFF ,0xff0000,0xffffff,SM_RAM); // Work Ram
  SekExt[0].ReadByte =CpsReadByte;
  SekExt[0].WriteByte=CpsWriteByte;
  SekExt[0].ReadWord =CpsReadWord;
  SekExt[0].WriteWord=CpsWriteWord;
  SekClose();

  return 0;
}

int CpsMemExit()
{
  // Deallocate all used memory
  if (CpsMem!=NULL) free(CpsMem);  CpsMem=NULL;
  return 0;
}

static int ScanRam()
{
  // scan ram:
  struct BurnArea ba;
  memset(&ba,0,sizeof(ba));

    ba.Data=CpsRam90;  ba.nLen=0x030000; ba.szName="CpsRam90";  BurnAcb(&ba);
    ba.Data=CpsRamFF;  ba.nLen=0x010000; ba.szName="CpsRamFF";  BurnAcb(&ba);
    ba.Data=CpsReg;    ba.nLen=0x000100; ba.szName="CpsReg";    BurnAcb(&ba);

  if (Cps==2)
  {
    ba.Data=CpsRam708; ba.nLen=0x008000; ba.szName="CpsRam708"; BurnAcb(&ba);
    ba.Data=CpsZRamC0; ba.nLen=0x001000; ba.szName="CpsZRamC0"; BurnAcb(&ba);
    ba.Data=CpsZRamF0; ba.nLen=0x001000; ba.szName="CpsZRamF0"; BurnAcb(&ba);
    ba.Data=CpsFrg;    ba.nLen=0x000010; ba.szName="CpsFrg";    BurnAcb(&ba);
  }
  return 0;
}

// Scan the current state of the CPS1/2 machine
int CpsAreaScan(int nAction,int *pnMin)
{
  if (CpsMem==NULL) return 1;

  if (nAction&8) // Scan non-volatile
  {
    if (Cps==2) CpsEepromScan(); // eeprom memory
    if (pnMin!=NULL) *pnMin=0x008400; // Return minimum compatible version
  }

  if (nAction&4) // Scan volatile
  {
    if (pnMin!=NULL) *pnMin=0x010002; // Return minimum compatible version
    ScanRam();
    if (Cps==1) PsndScan(nAction&3); // PSound chips
    if (Cps==2) QsndScan(nAction&3); // QSound chips
    SekScan(nAction&3); // scan 68000 state

    if (nAction&2) CpsRecalcPal=1; // Palette could have changed
  }

  return 0;
}


// CPS2 sound codes in Z80 memory:
int CpsScodePoll()
{
  int Code=0;

  if (Cps!=2) return 1;
  if (CpsZRamC0[0x00f]!=0x00) return 0;

  Code =CpsZRamC0[0x000]<<8;
  Code|=CpsZRamC0[0x001];

  // See if the code should be blocked
  if (BurnScodePass(Code)==0) Code=-1;

  // Assume nothing is being played:
  CpsZRamC0[0x00f]=0xff;

  if (Code>=0)
  {
    CpsZRamC0[0x000]=(unsigned char)(Code>>8);
    CpsZRamC0[0x001]=(unsigned char)(Code&255);
    CpsZRamC0[0x00f]=0x00;
  }
  return 0;
}
