#include "burnint.h"
// TMNT - Tiles

static unsigned int nPageBase[4]={0,0,0,0};
static short nScrollX[3]={12*8,0,0},nScrollY[3]={2*8,0,0};

static INLINE void RawTile(unsigned char *Dest,unsigned int nTile,unsigned int nPal)
{
  unsigned int *ps; unsigned char *pl; int y;
  unsigned int *Pal;
  nPal<<=4; nPal&=0x3f0; Pal=TmntPal+nPal;
  nTile<<=5;  if (nTile>=0x300000-0x20) return;

  ps=(unsigned int *)(TmntTile+nTile); pl=Dest;

  for (y=0;y<8; y++,ps++,pl+=nBurnPitch)
  {
    unsigned int s; int x; unsigned char *pPix;
    s=*ps;

    for (pPix=pl,x=0; x<8; x++,pPix+=nBurnBpp,s<<=4)
    {
      unsigned char p;
      p=(unsigned char)((s>>28)&15);
      if (p) PutPix(pPix,Pal[p]);
    }
  }
}

static INLINE void OneTile(unsigned char *Dest,unsigned int nTile,int nLayer)
{
  unsigned char PalBase[3]={0,0x20,0x28};
  int nPal; int p;
  // Get palette
  nPal=(nTile>>13)&7; nPal+=PalBase[nLayer];

  // Get tile page (0-3)
  p=(nTile>>10)&3; nTile=((nTile&0x1000)>>2) | (nTile&0x03ff);
  if (p) nTile+=nPageBase[p];

  RawTile(Dest,nTile,nPal);
}

// Draw Fix/A/B layers
int TmntTileLayer(int nLayer)
{
  int y=0; unsigned char *pl;
  for ( y=0,pl=pBurnDraw; y<28; y++,pl+=nBurnPitch<<3 )
  {
    int x=0; unsigned char *pt;
    for ( x=0,pt=pl; x<36; x++,pt+=nBurnBpp<<3 )
    {
      int sx,sy; unsigned char *pSrc;
      int nTile;
      
      pSrc=TmntRam10+(nLayer<<13);
      sx=x+(nScrollX[nLayer]>>3); sy=y+(nScrollY[nLayer]>>3);
      sx&=63; sy&=31; sx+=2;
      pSrc+=(sy<<7)+(sx<<1);
      
      nTile=*((unsigned short *)pSrc);
      OneTile(pt,nTile,nLayer);
    }
  }

  return 0;
}

int TmntTileDraw()
{
  int nLayer;
  unsigned char da,db;
  short s;

  // Get the two tile page decoders
  da=TmntRam10[0x6b00^1];
  db=TmntRam10[0x6e00^1];

  // Find the tile page base for each
                              nPageBase[1]=(da&0xf0)<<7;
  nPageBase[2]=(db&0x0f)<<11; nPageBase[3]=(db&0xf0)<<7;

  // Get the scroll-x values
  s =TmntRam10[0x6402^1]; s<<=8;
  s|=TmntRam10[0x6400^1]; s+=12*8; nScrollX[1]=s;

  s =TmntRam10[0x6403^1]; s<<=8;
  s|=TmntRam10[0x6401^1]; s+=12*8; nScrollX[2]=s;

  // Get the scroll-y values
  s=TmntRam10[0x6018^1]; s+=2*8; nScrollY[1]=s;
  s=TmntRam10[0x6019^1]; s+=2*8; nScrollY[2]=s;

  for (nLayer=2;nLayer>=0;nLayer--) TmntTileLayer(nLayer);
  return 0;
}
