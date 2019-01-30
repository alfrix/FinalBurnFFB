#include "burnint.h"
// TMNT - Input

unsigned char TmntAoo[0x10]; // values to return from A0000
unsigned char TmntCoin[4],TmntStart[4];
static unsigned char *pPlay[4]={TmntAoo+1,TmntAoo+2,TmntAoo+3,TmntAoo+0x0a};

int TmntInpMake()
{
  int i=0;
  memset(&TmntAoo,0,sizeof(TmntAoo));

  for (i=0;i<4;i++)
  {
    if (TmntCoin [i]) TmntAoo[0]|=0x01<<i;
    if (TmntStart[i]) *(pPlay[i])|=0x20;
  }

  return 0;
}
