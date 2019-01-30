// DirectSound module
#include "app.h"
#define DIRECTSOUND_VERSION  0x0300  // Only need version from DirectX 3
#include <dsound.h>

// Sound is split into a series of 'segs', one seg for each frame
// (typically 367 samples long).
// The Loop buffer is a multiple of this seg length.

static IDirectSound *pDS=NULL;   // DirectSound interface
static IDirectSoundBuffer *pdsbPrim=NULL; // Primary buffer
static IDirectSoundBuffer *pdsbLoop=NULL; // (Secondary) Loop buffer
int nDSoundSamRate=22050;        // sample rate
int nDSoundSegCount=6;    // Segs in the pdsbLoop buffer
static int cbLoopLen=0;          // Loop length (in bytes) calculated

int nDSoundFps=600;              // Application fps * 10
int nDSoundSegLen=0;             // Seg length in samples (calculated from Rate/Fps)
short *DSoundNextSound=NULL;     // The next sound seg we will add to the sample loop
unsigned char bDSoundOkay=0;     // True if DSound was initted okay
unsigned char bDSoundPlaying=0;  // True if the Loop buffer is playing

static int BlankSound()
{
  void *pData=NULL,*pData2=NULL; DWORD cbLen=0,cbLen2=0;
  int nRet=0;
  // Lock the Loop buffer
  nRet=pdsbLoop->Lock(0,cbLoopLen,&pData,&cbLen,
    &pData2,&cbLen2,0);
  if (nRet<0) return 1;
  if (pData!=NULL) memset(pData,0,cbLen);

  // Unlock (2nd 0 is because we wrote nothing to second part)
  nRet=pdsbLoop->Unlock(pData,cbLen,pData2,0);

  // Also blank the DSoundNextSound buffer
  if (DSoundNextSound!=NULL) memset(DSoundNextSound,0,nDSoundSegLen<<2);
  return 0;
}

#define WRAP_INC(x) { x++; if (x>=nDSoundSegCount) x=0; }

static int nDSoundNextSeg=0; // We have filled the sound in the loop up to the beginning of 'nNextSeg'

static int DSoundGetNextSoundFiller(int bDraw)
{
  (void)bDraw;
  if (DSoundNextSound==NULL) return 1;
  memset(DSoundNextSound,0,nDSoundSegLen<<2); // Write silence into the buffer
  return 0;
}

int (*DSoundGetNextSound) (int bDraw) = DSoundGetNextSoundFiller; // Callback used to request more sound

// This function checks the DSound loop, and if necessary gets some more sound
// and a picture.
int DSoundCheck()
{
  int nPlaySeg=0,nFollowingSeg=0;
  DWORD nPlay=0,nWrite=0;
  int nRet=0; int nRetVal=0;
  if (pdsbLoop==NULL) { nRetVal=1; goto End; }
  // We should do nothing until nPlay has left nDSoundNextSeg
  nRet=pdsbLoop->GetCurrentPosition(&nPlay,&nWrite);

  nPlaySeg=nPlay/(nDSoundSegLen<<2);
  
  if (nPlaySeg>nDSoundSegCount-1) nPlaySeg=nDSoundSegCount-1;
  if (nPlaySeg<0) nPlaySeg=0; // important to ensure nPlaySeg clipped for below
  
  if (nDSoundNextSeg==nPlaySeg)
  {
    Sleep(2); // Don't need to do anything for a bit
    goto End;
  }

  // work out which seg we will fill next
  nFollowingSeg=nDSoundNextSeg; WRAP_INC(nFollowingSeg)
  while (nDSoundNextSeg!=nPlaySeg)
  {
    void *pData=NULL,*pData2=NULL; DWORD cbLen=0,cbLen2=0;
    int bDraw;
    // fill nNextSeg
    // Lock the relevant seg of the loop buffer
    nRet=pdsbLoop->Lock(nDSoundNextSeg*(nDSoundSegLen<<2),nDSoundSegLen<<2,&pData,&cbLen,&pData2,&cbLen2,0);

    if (nRet>=0 && pData!=NULL)
    {
      // Locked the seg okay - write the sound we calculated last time
      memcpy(pData,DSoundNextSound,nDSoundSegLen<<2);
    }
    // Unlock (2nd 0 is because we wrote nothing to second part)
    if (nRet>=0) pdsbLoop->Unlock(pData,cbLen,pData2,0); 

    bDraw=(nFollowingSeg==nPlaySeg); // If this is the last seg of sound, flag bDraw (to draw the graphics)

    DSoundGetNextSound(bDraw); // get more sound into DSoundNextSound

    if (bAppBassFilter) DspDo(DSoundNextSound,nDSoundSegLen);
    nDSoundNextSeg=nFollowingSeg;
    WRAP_INC(nFollowingSeg)
  }
End:
  return nRetVal;
}

int DSoundInit(HWND hWnd)
{
  int nRet=0;
  DSBUFFERDESC dsbd;
  WAVEFORMATEX wfx;

  if (nDSoundSamRate<=0) return 1;

  // Calculate the Seg Length and Loop length
  // (round to nearest sample)
  nDSoundSegLen=(nDSoundSamRate*10+(nDSoundFps>>1))/nDSoundFps;
  cbLoopLen=(nDSoundSegLen*nDSoundSegCount)<<2;

  // Make the format of the sound
  memset(&wfx,0,sizeof(wfx));
  wfx.cbSize=sizeof(wfx);
  wfx.wFormatTag=WAVE_FORMAT_PCM;
  wfx.nChannels=2; // stereo
  wfx.nSamplesPerSec=nDSoundSamRate; // sample rate
  wfx.wBitsPerSample=16; // 16-bit
  wfx.nBlockAlign=4; // bytes per sample
  wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;

  // Create the DirectSound interface
  nRet=DirectSoundCreate(NULL,&pDS,NULL);
  if (nRet<0 || pDS==NULL) return 1;

  // Set the coop level
  nRet=pDS->SetCooperativeLevel(hWnd,DSSCL_PRIORITY);

  // Make the primary sound buffer
  memset(&dsbd,0,sizeof(dsbd));
  dsbd.dwSize=sizeof(dsbd);
  dsbd.dwFlags=DSBCAPS_PRIMARYBUFFER;
  nRet=pDS->CreateSoundBuffer(&dsbd,&pdsbPrim,NULL);
  if (nRet<0 || pdsbPrim==NULL) { DSoundExit(); return 1; }

  {
    // Set the format of the primary sound buffer (not critical if it fails)
    if (nDSoundSamRate<44100) wfx.nSamplesPerSec=44100;
    nRet=pdsbPrim->SetFormat(&wfx);

    wfx.nSamplesPerSec=nDSoundSamRate;
  }

  // Make the loop sound buffer
  memset(&dsbd,0,sizeof(dsbd));
  dsbd.dwSize=sizeof(dsbd);
  // A standard secondary buffer (accurate position, plays in the background, and can notify).
  dsbd.dwFlags=DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
  dsbd.dwBufferBytes=cbLoopLen;
  dsbd.lpwfxFormat=&wfx; // Same format as the primary buffer
  nRet=pDS->CreateSoundBuffer(&dsbd,&pdsbLoop,NULL);
  if (nRet<0 || pdsbLoop==NULL) { DSoundExit(); return 1; }
  
  DSoundNextSound=(short *)malloc(nDSoundSegLen<<2); // The next sound block to put in the stream
  if (DSoundNextSound==NULL) { DSoundExit(); return 1; }

  bDSoundOkay=1; // This module was initted okay
  DspInit();

  return 0;
}

int DSoundPlay()
{
  if (bDSoundOkay==0) return 1;
  BlankSound();
  // Play the looping buffer
  if (pdsbLoop->Play(0,0,DSBPLAY_LOOPING)<0) return 1;
  bDSoundPlaying=1;
  return 0;
}

int DSoundStop()
{
  bDSoundPlaying=0;
  if (bDSoundOkay==0) return 1;
  // Stop the looping buffer
  pdsbLoop->Stop();
  return 0;
}

int DSoundExit()
{
  DspExit();
  bDSoundOkay=0; // This module is no longer okay

  if (DSoundNextSound!=NULL) free(DSoundNextSound); DSoundNextSound=NULL;
  // Release the (Secondary) Loop Sound Buffer
  RELEASE(pdsbLoop)
  // Release the Primary Sound Buffer
  RELEASE(pdsbPrim)
  // Release the DirectSound interface
  RELEASE(pDS)

  return 0;
}
