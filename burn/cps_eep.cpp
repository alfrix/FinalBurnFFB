#include "burnint.h"
// CPS - EEPROM

static char Cmd[32*8]="";
static int nCmdPos=0;

static unsigned short CpsEep[0x40];
static int nReadWord=0,nReadBit=-1;
static int nAfterRead=0;

int CpsEepromBlank()
{
  memset(CpsEep,0,sizeof(CpsEep));
  return 0;
}

int CpsEepromReset()
{
  Cmd[0]=0;
  nCmdPos=0;
  nReadWord=0; nReadBit=-1; nAfterRead=0;
  return 0;
}

int CpsEepromScan()
{
  struct BurnArea ba;
  memset(&ba,0,sizeof(ba));

  ba.Data=CpsEep; ba.nLen=sizeof(CpsEep);
  ba.szName="EEPROM";
  BurnAcb(&ba);
  return 0;
}

static INLINE void AddBit(int b)
{
  Cmd[nCmdPos]=b?'1':'0';
  nCmdPos++;
  if (nCmdPos>sizeof(Cmd)-1) nCmdPos=sizeof(Cmd)-1; // Clip to end
  Cmd[nCmdPos]=0; // zero terminate
}

static unsigned int BinToVal(char *s,int nLen)
{
  int v=0; char *End=s+nLen;

  do { v<<=1; if (*s++ == '1') v|=1; } while (s<End);

  return v;
}

void CpsEepromWrite(int b)
{
  if (nAfterRead==1)
  { 
    // Write to eeprom after a bit was read
    nAfterRead=0;
    // go to next bit
    nReadBit++; if (nReadBit>=16) nReadBit=-1; // finished reading this word
  }


  AddBit(b);

  if (nCmdPos==1)
  {
    if (Cmd[0]=='1') { nCmdPos=0; goto End; } // Ignore single 1s?
  }
  if (nCmdPos==2)
  {
    if (Cmd[0]=='0' && Cmd[1]=='0') { nCmdPos=1; goto End; } // Ignore double 0s?
  }

  // Check for unlock
  if (nCmdPos==10)
  {
    if (memcmp("0100110000",Cmd,10)==0)
    {
      nCmdPos=0; goto End;
    }
  }

  // Check for erase
  if (nCmdPos==4+6)
  {
    if (memcmp("0111",Cmd,4)==0)
    {
      unsigned int a;
      a=BinToVal(Cmd+4  ,6);
      CpsEep[a&0x3f]=0;
      nCmdPos=0; goto End;
    }
  }

  // Check for write
  if (nCmdPos==4+6+16)
  {
    if (memcmp("0101",Cmd,4)==0)
    {
      unsigned int a,d;
      a=BinToVal(Cmd+4  ,6);
      d=BinToVal(Cmd+4+6,16);
      CpsEep[a&0x3f]=(unsigned short)d;
      nCmdPos=0; goto End;
    }
  }

  // Check for read
  if (nCmdPos==4+6)
  {
    if (memcmp("0110",Cmd,4)==0)
    {
      unsigned int a;
      a=BinToVal(Cmd+4  ,6);
      nReadWord=CpsEep[a&0x3f]; nReadBit=0; // start reading
      nCmdPos=0; goto End;
    }
  }

#undef CHK_CMD

End:
  Cmd[nCmdPos]=0; // zero terminate
  return;
}

int CpsEepromRead()
{
  int b;
  if (nReadBit<0) return 1; // not reading

  b=nReadWord;
  b>>=(15-nReadBit); b&=1;
  nAfterRead=1;
  return b;
}
