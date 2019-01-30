#include "burnint.h"
// PSound (CPS1 sound) Mixing 

int bPsmOkay=0; // 1 if the module is okay
static short *WaveBuf=NULL;
static short *AdBuf=NULL;

int PsmInit()
{
  int nRet=0; int nMemLen=0;
  bPsmOkay=0; // not ok yet
  if (nBurnSoundRate<=0) return 1;

  nRet=YM2151Init(1,3579540,nBurnSoundRate); // Init FM sound chip
  if (nRet!=0) return 1;

  // Allocate a buffer for the intermediate sound (between YM2151 and pBurnSoundOut)
  nMemLen=nBurnSoundLen*2*sizeof(short);
  WaveBuf=(short *)malloc(nMemLen);
  if (WaveBuf==NULL) { PsmExit(); return 1; }
  memset(WaveBuf,0,nMemLen); // init to silence

  // Init ADPCM
  nPsaRate=nBurnSoundRate;
  nRet=PsaInit(); if (nRet!=0) { PsmExit(); return 1; }

  // Allocate a buffer for the ADPCM
  nMemLen=nBurnSoundLen*sizeof(short);
  AdBuf=(short *)malloc(nMemLen);
  if (AdBuf==NULL) { PsmExit(); return 1; }
  memset(AdBuf,0,nMemLen); // init to silence

  bPsmOkay=1; // ok
  return 0;
}

int PsmExit()
{
  bPsmOkay=0; // not ok
  if (AdBuf  !=NULL) free(AdBuf);    AdBuf=NULL;
  PsaExit();
  nPsaRate=0;

  if (WaveBuf!=NULL) free(WaveBuf);  WaveBuf=NULL;
  YM2151Shutdown();  // Exit FM sound chip
  return 0;
}

// Render a section of sound
// e.g. 0x200 to 0x400 for second half of frame
int PsmSect(int nStart,int nEnd)
{
  short *Buf[2];
  int p1,p2,i;
  if (bPsmOkay==0) return 1;  if (pBurnSoundOut==NULL) return 1;
  if (nEnd<=nStart) return 0;

  // Copy this portion of YM2151 sound into our WaveBuf buffer
  p1=nStart*nBurnSoundLen; p1>>=10;
  p2=nEnd  *nBurnSoundLen; p2>>=10;
  Buf[0]=WaveBuf+p1;
  Buf[1]=WaveBuf+nBurnSoundLen+p1;

  // Render Fm
  YM2151UpdateOne(0,Buf,p2-p1);

  // Render ADPCM
  PsaRender(AdBuf+p1,p2-p1);

  // Interleve into our buffer
  for (i=p1;i<p2;i++)
  {
    int l,r;
    l=WaveBuf[i];
    r=WaveBuf[nBurnSoundLen+i];
    l+=AdBuf[i]; r+=AdBuf[i];
    l>>=1; r>>=1;

#define CLIP(x) if (x<-0x8000) x=-0x8000;  if (x>0x7fff) x=0x7fff;
    CLIP(l) CLIP(r)
    pBurnSoundOut[(i<<1)+0]=(short)l;
    pBurnSoundOut[(i<<1)+1]=(short)r;
  }

  return 0;
}
