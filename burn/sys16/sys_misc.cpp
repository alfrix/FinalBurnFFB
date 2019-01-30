#include "sys.h"
// System 16 - Misc

// This function loads code roms. All even roms should be first, followed by all odd roms.
int BsysLoadCode(unsigned char *Rom,int Start,int Count)
{
  int i=0,Odd=0; int nRet=0;
  Count>>=1; // Half the count, then load all even then all odd

  for (Odd=0;Odd<2;Odd++)
  {
    unsigned char *pLoad; pLoad=Rom+(Odd^1); // ^1 for byteswapped

    for (i=0;i<Count;i++)
    {
      struct BurnRomInfo ri;
      // Load these even/odd bytes
      nRet=BurnLoadRom(pLoad,Start+i,2); if (nRet!=0) return 1;

      // Increment position by the length of the rom * 2
      ri.nLen=0; BurnDrvGetRomInfo(&ri,Start+i); pLoad+=ri.nLen<<1;
    }
    Start+=Count; // Move on to the odd byte roms
  }

  return 0;
}

// Load 3 roms as bitplanes of tile data
int BsysLoadTiles(unsigned char *Tile,int Start)
{
  struct BurnRomInfo ri; unsigned char *TileTmp=NULL;
  int i=0; int nRetVal=0;
  ri.nLen=0; BurnDrvGetRomInfo(&ri,Start); // Find out how big the tile roms are

  // Load tile data (three bitfields)
  TileTmp=(unsigned char *)malloc(ri.nLen); if (TileTmp==NULL) return 1;
  memset(TileTmp,0,ri.nLen);
  for (i=0;i<3;i++)
  {
    int nRet;  nRet=BurnLoadRom(TileTmp,Start+i,1);  if (nRet!=0) nRetVal=1;
    BurnLoadBitField(Tile,TileTmp,i,ri.nLen);
  }
  free(TileTmp); TileTmp=NULL;

  return nRetVal;
}

// Change color 15 to 0, to eliminate the System16 (guide?)lines
int BsysZeroSprites(unsigned char *Spr,int Len)
{
  unsigned char *ps,*pEnd;
  if (Len<=0) return 1;
  pEnd=Spr+Len;
  ps=Spr;

  do
  {
    unsigned char x;
    x=(unsigned char)(~ps[0]);
    if ((x&0xf0)==0) ps[0]&=0x0f;
    if ((x&0x0f)==0) ps[0]&=0xf0;
    ps++;
  }
  while (ps<pEnd);

  return 0;
}

// Decode a 0x200*0x100 1-bit,2-bitplane ground bitmap (0x08000 bytes) into 8-bit (0x20000 bytes)
int BsysDecodeGr(unsigned char *Dest,unsigned char *Src)
{
  unsigned char *pd=Dest;
  unsigned char *ps=Src;
  int x=0,y=0;

  for (y=0;y<0x100;y++)
  {
    int Pixel=0,Last=0;
    for (x=0; x<0x200; x+=8,pd+=8,ps++)
    {
      unsigned int a; int c;
      a=ps[0]|(ps[0x4000]<<8);

      for (c=0;c<8;c++)
      {
        int Val; int Add;
        switch (a&0x8080)
        {
          default:     Val=3; break;
          case 0x0080: Val=2; break;
          case 0x8000: Val=1; break;
          case 0x8080: Val=0; break;
        }
        a<<=1;

        // Get rid of the line down the middle of the road
        pd[c]=(unsigned char)Pixel;
        Add=Val-Last;

             if (Last==3 && Val==0) { Add= 1; }
        else if (Last==0 && Val==3) { Add=-1; }
        else if (Last==2 && Val==0) { Add= 2; }
        else if (Last==0 && Val==2) { Add=-2; }
        Pixel+=Add;

        Last=Val;

      }
    }
  }
  return 0;
}
