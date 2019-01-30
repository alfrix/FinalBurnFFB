// Sound DSP code (filtering through convolution on the sound output)
#include "app.h"

// (Note - this old code, and convolution is a very slow way of filtering:
//  use a IIR filter!)

// This is the convolution function
static int nDspFnLen=48;
static short DspFn[  475]=
{ 5491,9479,3505, 702,1714,1173,1187,1052, 971, 889, 819, 756, 702, 655, 614, 577,
   545, 516, 490, 467, 445, 426, 408, 391, 376, 361, 347, 335, 322, 311, 300, 289,
   279, 270, 260, 252, 243, 235, 227, 220, 213, 206, 199, 193, 187, 181, 176, 170,
   165, 160, 155, 150, 146, 142, 138, 134, 130, 126, 122, 119, 116, 113, 109, 106,
   104, 101,  98,  96,  93,  91,  88,  86,  84,  82,  80,  78,  76,  74,  73,  71,
    69,  68,  66,  65,  63,  62,  60,  59,  58,  57,  55,  54,  53,  52,  51,  50,
    49,  48,  47,  46,  46,  45,  44,  43,  42,  42,  41,  40,  39,  39,  38,  37,
    37,  36,  36,  35,  35,  34,  33,  33,  32,  32,  31,  31,  31,  30,  30,  29,
    29,  28,  28,  28,  27,  27,  27,  26,  26,  25,  25,  25,  24,  24,  24,  24,
    23,  23,  23,  22,  22,  22,  22,  21,  21,  21,  21,  20,  20,  20,  20,  19,
    19,  19,  19,  18,  18,  18,  18,  18,  17,  17,  17,  17,  17,  16,  16,  16,
    16,  16,  16,  15,  15,  15,  15,  15,  15,  14,  14,  14,  14,  14,  14,  13,
    13,  13,  13,  13,  13,  13,  12,  12,  12,  12,  12,  12,  12,  12,  11,  11,
    11,  11,  11,  11,  11,  11,  10,  10,  10,  10,  10,  10,  10,  10,  10,   9,
     9,   9,   9,   9,   9,   9,   9,   9,   9,   8,   8,   8,   8,   8,   8,   8,
     8,   8,   8,   8,   8,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
     7,   7,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
     6,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
     5,   5,   5,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
     4,   4,   4,   4,   4,   4,   4,   4,   4,   3,   3,   3,   3,   3,   3,   3,
     3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
     3,   3,   3,   3,   3,   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,   2,
     2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
     2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
     2,   2,   2,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1
};



// This is the history of the WaveForm (used for applying the convolution function)
static short *History=NULL,*pHist=NULL;
static int nHistoryIndex=0;
// Add to the history buffer, wrapping around
static INLINE void AddToHistory(short nLeft,short nRight)
{
  *pHist++=nLeft; *pHist++=nRight; nHistoryIndex++;
  if (nHistoryIndex>=nDspFnLen) { nHistoryIndex=0; pHist=History; } // wrap around to the beginning
}
// Get from the history buffer, wrapping around
static INLINE void GetFromHistory(short *pnLeft,short *pnRight)
{
  *pnLeft=*pHist++; *pnRight=*pHist++; nHistoryIndex++;
  if (nHistoryIndex>=nDspFnLen) { nHistoryIndex=0; pHist=History; } // wrap around to the beginning
}

int DspDo(short *Wave,int nCount)
{
  int i,j;
  if (History==NULL) return 1;
  for (i=0;i<nCount;i++,Wave+=2)
  {
    __int64 nConvLeft=0,nConvRight=0;
    short *pDsp;
    AddToHistory(Wave[0],Wave[1]);
    // Now go all the way around the history buffer, convoluting the history
    // and arriving back where we started
    for (j=0,pDsp=DspFn; j<nDspFnLen; j++,pDsp++) // +2 for 22050
    {
      short nLeft,nRight;
      GetFromHistory(&nLeft,&nRight);
      nConvLeft +=((int)nLeft *(*pDsp));
      nConvRight+=((int)nRight*(*pDsp));
    }
    // Finally average them and put them back in the buffer
    nConvLeft>>=14; nConvRight>>=14;
    // Clip left and right output to +-16 bits
#define CLIP_LR(x) if (x>0x7fff) x=0x7fff; if (x<-0x8000) x=-0x8000;
    CLIP_LR(nConvLeft) CLIP_LR(nConvRight)
    Wave[0]=(short)nConvLeft; Wave[1]=(short)nConvRight;
  }
  return 0;
}

int DspInit()
{
  // Create the wave history buffer
  History=(short *)malloc(nDspFnLen*2*sizeof(short));
  if (History==NULL) return 1;
  memset(History,0,nDspFnLen*2*sizeof(short));
  // Start at the beginning of the History buffer
  nHistoryIndex=0; pHist=History;
  return 0;
}

int DspExit()
{
  if (History!=NULL) free(History); History=NULL;
  return 0;
}

