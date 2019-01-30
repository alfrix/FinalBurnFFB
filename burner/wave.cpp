// Wave log module
#include "app.h"

FILE *WaveLog=NULL; // wave log file

// Make a wave header so we can write out raw samples after it to make a wave file
static int WaveLogHeaderStart(FILE *Hand,int nSamRate)
{
  unsigned int ua=0x10;
  WORD  wFormatTag=1; // PCM
  WORD  nChannels=0;
  DWORD nSamplesPerSec=0,nAvgBytesPerSec=0;
  WORD  nBlockAlign=0,wBitsPerSample=0;

  nChannels=2; // stereo
  nSamplesPerSec=nSamRate;
  nAvgBytesPerSec=nSamRate*4; // samrate*2*short
  nBlockAlign=4; // 2*short
  wBitsPerSample=16; // 16-bit

  if (Hand==NULL) return 1;
  fwrite("RIFF    WAVEfmt ",1,0x10,Hand); // fill in 0x04 later
  fwrite(&ua,1,sizeof(ua),Hand);
  fwrite(&wFormatTag,1,sizeof(wFormatTag),Hand);
  fwrite(&nChannels,1,sizeof(nChannels),Hand);
  fwrite(&nSamplesPerSec,1,sizeof(nSamplesPerSec),Hand);
  fwrite(&nAvgBytesPerSec,1,sizeof(nAvgBytesPerSec),Hand);
  fwrite(&nBlockAlign,1,sizeof(nBlockAlign),Hand);
  fwrite(&wBitsPerSample,1,sizeof(wBitsPerSample),Hand);
  fwrite("data    ",1,0x08,Hand); // fill in 0x28 later
  return 0;
}

// Fill in the RIFF and data values
static int WaveLogHeaderFillIn(FILE *Hand)
{
  unsigned int nLen=0;
  if (Hand==NULL) return 1;
  fseek(Hand,0,SEEK_END); nLen=ftell(Hand);
  fseek(Hand,0x04,SEEK_SET); nLen-=8;  fwrite(&nLen,1,sizeof(nLen),Hand); // RIFF value
  fseek(Hand,0x28,SEEK_SET); nLen-=44; fwrite(&nLen,1,sizeof(nLen),Hand); // data value
  fseek(Hand,0,SEEK_END);
  return 0;
}

int WaveLogStart(char *szName,int nSamRate)
{
  WaveLogStop(); // make sure old log is closed
  // Set up the sound
  WaveLog=fopen(szName,"wb");
  if (WaveLog==NULL) return 1;
  WaveLogHeaderStart(WaveLog,nSamRate);
  return 0;
}

int WaveLogStop()
{
  if (WaveLog!=NULL) { WaveLogHeaderFillIn(WaveLog); fclose(WaveLog); }
  WaveLog=NULL;
  return 0;
}

