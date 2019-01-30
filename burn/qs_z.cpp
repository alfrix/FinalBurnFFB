#include "burnint.h"
// QSound - Z80

static unsigned char QsndZIn(unsigned short a) { (void)a; return 0; }
static void QsndZOut(unsigned short a,unsigned char d) { (void)a; (void)d; }

static int nQsndZBank=0;
// Map in the memory for the current 0x8000-0xc000 music bank
static int QsndZBankMap()
{
  unsigned int nOff; unsigned char *Bank;
  nOff=nQsndZBank<<14;
  nOff+=0x8000;

  if (nOff+0x4000>nCpsZRomLen) nOff=0; // End of bank is out of range
  Bank=CpsZRom+nOff;

  // Read and fetch the bank
  ZetMapArea(0x8000,0xbfff,0,Bank);
  ZetMapArea(0x8000,0xbfff,2,Bank);
  return 0;
}

static unsigned char QscCmd[2]={0,0};

static void QsndZWrite(unsigned short a,unsigned char d)
{
  if (a==0xd000) { QscCmd[0]=d; return; }
  if (a==0xd001) { QscCmd[1]=d; return; }
  if (a==0xd002) { QscWrite(d,(QscCmd[0]<<8) | QscCmd[1]); return; }
  if (a==0xd003)
  {
    int nNewBank; nNewBank=d&0x0f;
    if (nQsndZBank!=nNewBank)
    {
      nQsndZBank=nNewBank;
      QsndZBankMap();
    }
  }
}

static unsigned char QsndZRead(unsigned short a)
{
  if (a==0xd007) return 0x80; // return ready all the time
  return 0;
}

int QsndZInit()
{
  if (nCpsZRomLen<0x8000) return 1; // Not enough Z80 Data
  if (CpsZRom==NULL) return 1;

  ZetRead=QsndZRead; ZetWrite=QsndZWrite;
  ZetIn=QsndZIn; ZetOut=QsndZOut;
  ZetInit();

  // Read and fetch first 0x8000 of Rom
  ZetMapArea(0x0000,0x7fff,0,CpsZRom);
  ZetMapArea(0x0000,0x7fff,2,CpsZRom);

  // Map first Bank of Rom
  nQsndZBank=0;
  QsndZBankMap();

  ZetMapArea(0xc000,0xcfff,0,CpsZRamC0);
  ZetMapArea(0xc000,0xcfff,1,CpsZRamC0);
  ZetMapArea(0xc000,0xcfff,2,CpsZRamC0);

  ZetMemCallback(0xd000,0xefff,0);
  ZetMemCallback(0xd000,0xefff,1);
  ZetMapArea    (0xd000,0xefff,2,CpsZRom); // If it tries to fetch this area

  ZetMapArea(0xf000,0xffff,0,CpsZRamF0);
  ZetMapArea(0xf000,0xffff,1,CpsZRamF0);
  ZetMapArea(0xf000,0xffff,2,CpsZRamF0);

  ZetMemEnd();

  return 0;
}

int QsndZExit()
{
  ZetExit();
  ZetIn=NULL; ZetOut=NULL;
  ZetRead=NULL; ZetWrite=NULL;
  return 0;
}

// Scan the current QSound z80 state
int QsndZScan(int nAction)
{
  ZetScan(nAction); // Scan Z80
  SCAN_VAR(nQsndZBank)
  if (nAction&2) QsndZBankMap(); // If write, bank could have changed
  return 0;
}
