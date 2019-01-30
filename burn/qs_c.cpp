#include "burnint.h"
// QSound - emulator for the actual QSound Chip

int nQscRate=0;

struct QChan
{
  unsigned char bKey;  // 1 if channel is playing

  unsigned int nStart; // Start of sample 16.12
  unsigned int nEnd;   // End of sample   16.12

  unsigned int nPos;   // Current position within the bank 16.12
  unsigned int nLoop;  // Loop offset from end

  unsigned int nVol;
  unsigned short nBank; // Bank we are currently playing a sample from
  char *PlayBank;      // Direct pointer
  unsigned short nPitch;
  unsigned int nAdvance;
  int nSide[2];

  unsigned char bDidKeyOn; // used for display
};

static struct QChan QChan[16];

static void MapBank(struct QChan *pc)
{
  unsigned int nBank;
  // Banks are 0x10000 samples long
  nBank=(pc->nBank&0xff)<<16;
  // Confirm whole bank is in range:
  // If bank is out of range use bank 0 instead
  if ((nBank+0x10000) > nCpsQSamLen) nBank=0;
  pc->PlayBank=CpsQSam+nBank;
}

static void CalcAdvance(struct QChan *pc)
{
  if (nQscRate) pc->nAdvance=pc->nPitch*24100/nQscRate;
}

int QscReset()
{
  int i=0;
  memset(QChan,0,sizeof(QChan));
  // Point all to bank 0
  for (i=0;i<16;i++) QChan[i].PlayBank=CpsQSam;
  return 0;
}

int QscScan(int nAction)
{
  int i=0;

  SCAN_VAR(QChan)
  if (nAction&2)
  {
    // Update bank pointers with new banks, and recalc nAdvance
    for (i=0;i<16;i++) { MapBank(QChan+i); CalcAdvance(QChan+i); }
  }
  return 0;
}

int QscDispInfo(int *pbKey,unsigned int *pnVol,int *pbDidKeyOn,int i)
{
  *pbKey=QChan[i].bKey;
  *pnVol=QChan[i].nVol;
  *pbDidKeyOn=QChan[i].bDidKeyOn;
  QChan[i].bDidKeyOn=0; // reset the flag for did key on since last call
  return 0;
}

void QscWrite(int a,int d)
{
  int nChanNum,r; struct QChan *pc;

  if (a>=0x90)
  {
    // unknown
    return;
  }
  if (a>=0x80)
  {
    int nPan;
    // Panning for channel
    nChanNum=a&15;
    pc=QChan+nChanNum; // Find channel
    nPan=d-0x110; // nPan = 0x00 to 0x20 now
    if (nPan<0x00) nPan=0x00;
    if (nPan>0x20) nPan=0x20;
    pc->nSide[0]=0x08+0x20-nPan;
    pc->nSide[1]=0x08+nPan;
    return;
  }

  // Get channel and register number
  nChanNum=a>>3; r=a&7;
  if (r==0) nChanNum++; // very strange!
  nChanNum&=15;

  pc=QChan+nChanNum; // Find channel

  if (r==0) { pc->nBank=(unsigned short)d; MapBank(pc); }
  if (r==1) { pc->nStart=d<<12; return; }  
  if (r==2)
  {
    if (d==0) { pc->bKey=0; return; } // Key off

    pc->nPitch=(unsigned short)d;
    CalcAdvance(pc);

    if (pc->bKey) return; // Key already on
    // Key on

    // Start playing the sample
    pc->nPos=pc->nStart;
    pc->bKey=1;
    pc->bDidKeyOn=1; // used for display

    return;
  }

  if (r==4) { pc->nLoop=d<<12; return; } // Loop offset
  if (r==5) { pc->nEnd= d<<12; return; }
  if (r==6)
  {
    pc->nVol=d;
    if (d==0) pc->bKey=0;
    return;
  }
}

static INLINE void QscChan(int nChanNum,int *ps)
{
  struct QChan *pc; int nOff,nSam;
  int nSam2,nFrac;

  pc=QChan+nChanNum;

  if (pc->bKey==0) return;

  // Check if we've reached the end of the sample
  // (-0x1000 = don't let interpolation run into next sample)
  if (pc->nPos>=pc->nEnd-0x1000)
  {
    if (pc->nLoop>0)
    {
      // loop back
      pc->nPos-=pc->nLoop;
    }
    else
    {
      pc->bKey=0;
      return;
    }
  }

  // Find the offset within the bank
  nOff=pc->nPos>>12; nOff&=0xffff;

  nSam =pc->PlayBank[nOff];
 
  // Linearly interpolate:
  nSam2=pc->PlayBank[(nOff+1)&0xffff];
  nFrac=pc->nPos&0xfff;
  nSam=nSam*(0x1000-nFrac) + nSam2*nFrac;
  nSam>>=13;

  nSam*=pc->nVol; nSam>>=3;

  // Find out stereo
  ps[0]+=(nSam*pc->nSide[0])/0x28;
  ps[1]+=(nSam*pc->nSide[1])/0x28;

  // Increment pointer
  pc->nPos+=pc->nAdvance;
}

// Write nLen*2 16-bit samples (stereo interleved) into Dest
int QscUpdate(short *Dest,int nLen)
{
  int i=0,c=0; short *pd=NULL;

  // Go through output buffer
  for (i=0,pd=Dest; i<nLen; i++,pd+=2)
  {
    int s[2];
    // Start with silence
    s[0]=0; s[1]=0;

    // Add in all channels
    for (c=0;c<16;c++) QscChan(c,s);

#define CLIP(x) if (x<-0x8000) x=-0x8000;  if (x>0x7fff) x=0x7fff;
    CLIP(s[0]) CLIP(s[1])
    // Insert into sound
    pd[0]=(short)s[0]; pd[1]=(short)s[1];
  }

  return 0;
}
