#include "sys.h"
// After Burner II - Sound

int BsysSndOkay=0; // 1 if the module is initted okay

static short *WaveBuf=NULL;
static int nRenderTo=0; // How much of the current frame we have rendered

int BsysSndInit()
{
  int Ret=0; int nMemLen=0;
  BsysSndOkay=0; // not initted yet
  if (nBurnSoundRate<=0) return 1;

  Ret=YM2151Init(1,4000000,nBurnSoundRate); // Init FM sound chip
  if (Ret!=0) return 1;

  PcmRate=nBurnSoundRate;
  Ret=PcmInit(); // Init PCM sound chip
  if (Ret!=0) { YM2151Shutdown(); return 1; }

  // Allocate a buffer for the intermediate sound (between YM2151 and pBurnSoundOut)
  nMemLen=nBurnSoundLen*2*sizeof(short);
  WaveBuf=(short *)malloc(nMemLen);
  if (WaveBuf==NULL) { PcmExit(); YM2151Shutdown(); return 1; }
  memset(WaveBuf,0,nMemLen); // init to silence

  BsysSndOkay=1; // module initted okay
  return 0;
}

int BsysSndExit()
{
  BsysSndOkay=0; // module not initted
  if (WaveBuf!=NULL) free(WaveBuf);  WaveBuf=NULL;
  PcmExit();         // Exit PCM sound chip

  YM2151Shutdown();  // Exit FM sound chip
  return 0;
}

// Start rendering sound for this frame
int BsysSndFrameStart()
{
  nRenderTo=0; // start rendering at the start of the buffer

  return 0;
}

// Render a frame up to sample 'nPos'
int BsysSndRenderTo(int nPos)
{
  short *Buf[2]={NULL,NULL};
  int i=0;

  if (BsysSndOkay==0 || pBurnSoundOut==NULL) return 1;

  if (nPos<=nRenderTo) return 1;
  if (nPos>nBurnSoundLen) nPos=nBurnSoundLen; // don't go past the end of the buffer

  // Copy this portion of YM2151 sound into our WaveBuf buffer
  Buf[0]=WaveBuf+nRenderTo;
  Buf[1]=WaveBuf+nBurnSoundLen+nRenderTo;

  // Render Fm
  YM2151UpdateOne(0,Buf,nPos-nRenderTo);

  // Interleve into our buffer
  for (i=nRenderTo;i<nPos;i++)
  {
    pBurnSoundOut[(i<<1)+0]=(short)(WaveBuf[i]);
    pBurnSoundOut[(i<<1)+1]=(short)(WaveBuf[nBurnSoundLen+i]);
  }

  // Add PCM sound
  PcmUpdate(pBurnSoundOut+(nRenderTo<<1),nPos-nRenderTo);

  nRenderTo=nPos; // rendered up to here
  return 0;
}

int BsysSndFrameStop()
{
  BsysSndRenderTo(nBurnSoundLen); // Make sure rendered up to the end of the buffer
  return 0;
}