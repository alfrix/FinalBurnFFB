#include "burnint.h"
#include "math.h"
// PSound ADPCM

// low 3 bits add to the step value:
static int StepShift[8]={-1,-1,-1,-1,2,4,6,8};
// 4 bits combined with current step value = delta
static int DeltaLookup[49*16];

int nPsaRate=0;
int nPsaSpeed=0;

struct PsaChan
{
  unsigned int nPos;
  unsigned int nEnd;
  short v;
  short nStep;
  char nVol;
  int nOutput; // v * volume
};

static struct PsaChan PsaChan[4];
static int nTotal=0,nLastTot=0; // Mix total, and last total value
static int nFracSam=0;
// 1111 ABCD (1 if channel is playing)
unsigned char nPsaStatus=0;
static unsigned char nLastCmd=0; // First part of two byte command

static char VolTable[16];

static int DeltaLookupMake()
{
  static int nCalced=0;
  int i,j; int nStep;
  if (nCalced) return 0; // already done

  memset(&DeltaLookup,0,sizeof(DeltaLookup));
  for (i=0;i<49;i++)
  {
    nStep=(int) ( pow(1.1, (double)i) * 16.0 );
    for (j=0;j<16;j++)
    {
      int s;
      s=((j&7)<<1)+1;
      s=nStep*s;
      if (j&8) s=-s;
      s/=16;

      DeltaLookup[(i<<4)|j]=s;
    }
  }
  nCalced=1;
  return 0;
}

int PsaInit()
{
  float fVol; int i;
  PsaExit();
  DeltaLookupMake();
  // Volume table
  memset(&VolTable,0,sizeof(VolTable));
  fVol=80.0f;
  for (i=0;i<16;i++) { VolTable[i]=(signed char)fVol; fVol*=0.70794578f; }

  return 0;
}

int PsaExit()
{
  memset(&PsaChan,0,sizeof(PsaChan));
  nFracSam=0; nPsaStatus=0xf0; nLastCmd=0;
  nTotal=0; nLastTot=0;
  return 0;
}

static INLINE void PsaNextValue(int c)
{
  unsigned int n; int nDelta; struct PsaChan *pc;
  c&=3; pc=PsaChan+c;
  if (pc->nPos>=pc->nEnd)
  {
    // not playing/reached end of sample
    pc->v=0;  nPsaStatus&=~(1<<c);
    return;
  }

  n=(pc->nPos>>1);

  if (n>=nCpsAdLen) { pc->nPos=0; return; }

  n=CpsAd[n];  if ((pc->nPos&1)==0) n>>=4;
  n&=15;
  pc->nPos++;

  // Clip step value
  if (pc->nStep< 0) pc->nStep= 0;
  if (pc->nStep>48) pc->nStep=48;

  // Work out delta
  nDelta=DeltaLookup[(pc->nStep<<4) | n];

  // Calculate new value
  pc->v=(short)((pc->v*63)/64); // centralise
  pc->v=(short)(pc->v+nDelta);

  pc->nOutput=pc->v*pc->nVol;

  // Calculate new step value
  pc->nStep=(short)(pc->nStep+StepShift[n&7]);
}

static INLINE void NextAdpcm()
{
  int j;
  // Store last total for interpolation
  nLastTot=nTotal;
  while (nFracSam>=nPsaRate)
  {
    for (j=0;j<4;j++) { PsaNextValue(j); }
    nFracSam-=nPsaRate;
  }

  // Mix channels into a total output
  nTotal=0;  for (j=0;j<4;j++) { nTotal+=PsaChan[j].nOutput; }
  nTotal>>=2;
  // Clip
  if (nTotal> 0x7ff0) nTotal= 0x7ff0;
  if (nTotal<-0x7ff0) nTotal=-0x7ff0;
}

// Render a section of sound
int PsaRender(short *Dest,int nLen)
{
  int i=0; short *pd;
  if (nLen<=0) return 0;
  if (nPsaRate<=0) return 1;

  for (i=0,pd=Dest; i<nLen; i++,pd++,nFracSam+=nPsaSpeed)
  {
    int v;
    // See if we need to get the next ADPCM sample for each channel
    if (nFracSam>=nPsaRate) NextAdpcm();

    // Interpolate between last and current totals
    v=nLastTot*(nPsaRate-nFracSam) + nTotal*nFracSam;  v/=nPsaRate;
    *pd=(short)v;
  }

  return 0;
}

// Play a sample on an ADPCM channel
static INLINE void PsaPlay(int nChan,unsigned int nSam)
{
  struct PsaChan *pc; int c=-1; unsigned int a;
       if (nChan&0x10) c=0;
  else if (nChan&0x20) c=1;
  else if (nChan&0x40) c=2;
  else if (nChan&0x80) c=3;
  if (c<0) return;
  nSam&=0x7f;

  pc=PsaChan+c;

  // Read sample offset from the ADPCM table
  nSam<<=3;
  if (nSam+3>nCpsAdLen) return;
  a =CpsAd[nSam+0]; a<<=8;
  a|=CpsAd[nSam+1]; a<<=8;
  a|=CpsAd[nSam+2]; a<<=1;
  pc->nPos=a;

  a =CpsAd[nSam+3]; a<<=8;
  a|=CpsAd[nSam+4]; a<<=8;
  a|=CpsAd[nSam+5]; a<<=1;
  pc->nEnd=a;

  pc->v=0; pc->nStep=0;
  pc->nVol=VolTable[nChan&15];
  pc->nOutput=0;

  nPsaStatus|=1<<c; // Status = playing
}


// Write ADPCM
void PsaWrite(unsigned char d)
{
  if (nLastCmd) { PsaPlay(d,nLastCmd); nLastCmd=0; return; }
  if (d&0x80) { nLastCmd=d; return; } // First part of two-byte command

  // Stop channels
  if (d&0x08) { PsaChan[0].nEnd=0; nPsaStatus&=~1; }
  if (d&0x10) { PsaChan[1].nEnd=0; nPsaStatus&=~2; }
  if (d&0x20) { PsaChan[2].nEnd=0; nPsaStatus&=~4; }
  if (d&0x40) { PsaChan[3].nEnd=0; nPsaStatus&=~8; }
}
