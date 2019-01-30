// CPS Tiles (header)

// Draw a nxn tile
// pCtvLine, pTile, nTileAdd are defined
// CU_FLIPX is 1 to flip the tile horizontally
// CU_CARE  is 1 to clip output based on nCtvRollX/Y
// CU_ROWS  is 1 to shift output based on CpstRowShift
// CU_SIZE  is 8, 16 or 32
// CU_PMSK  is 1 to mask pixels based on CpstPmsk (1=draw)
// CU_BPP is 1 2 3 4 bytes per pixel

#ifndef CU_FLIPX
#error CU_FLIPX wasn't defined
#endif
#ifndef CU_CARE
#error CU_CARE wasn't defined
#endif
#ifndef CU_ROWS
#error CU_ROWS wasn't defined
#endif
#ifndef CU_SIZE
#error CU_SIZE wasn't defined
#endif
#ifndef CU_PMSK
#error CU_PMSK wasn't defined
#endif
#ifndef CU_BPP
#error CU_BPP wasn't defined
#endif

{ int y; unsigned int *ctp; unsigned int nBlank=0xffffffff;
#if CU_ROWS==1
  short *Rows=CpstRowShift;
#endif
#if CU_PMSK==1
  unsigned int pmsk=CpstPmsk;
#endif

  ctp=CpstPal;

for (y=0;y<CU_SIZE; y++,pCtvLine+=nBurnPitch,pCtvTile+=nCtvTileAdd

#if CU_ROWS==1
     ,Rows++
#endif

)
{
  unsigned int b;      // Eight bit-packed pixels (msb) AAAABBBB CCCCDDDD EEEEFFFF GGGGHHHH (lsb)
  unsigned char *pPix; // Pointer to output bitmap
#if CU_CARE==1
  unsigned int rx=nCtvRollX; // Copy of nCtvRollX
#endif

#if CU_CARE==1
  if (nCtvRollY&0x20004000) { nCtvRollY+=0x7fff; continue; } else nCtvRollY+=0x7fff; // okay to plot line
#endif

  // Point to the line to draw
  pPix=pCtvLine;
#if CU_ROWS==1
  pPix+=Rows[0]*nBurnBpp;
#if CU_CARE==1
  rx+=Rows[0]*0x7fff;
#endif
#endif

// Make macros for plotting c and advancing pPix by one pixel
#if   CU_BPP==1
#define PLOT_ADV *pPix++=(unsigned char)c;
#define ADV pPix++;
#elif CU_BPP==2
#define PLOT_ADV { *(unsigned short *)pPix=(unsigned short)c; pPix+=2; }
#define ADV pPix+=2;
#elif CU_BPP==3
#define PLOT_ADV { *pPix++=(unsigned char)c; *pPix++=(unsigned char)(c>>8); *pPix++=(unsigned char)(c>>16); }
#define ADV pPix+=3;
#elif CU_BPP==4
#define PLOT_ADV { *(unsigned int *)pPix=c; pPix+=4; }
#define ADV pPix+=4;
#else
#error Unsupported CU_BPP
#endif

// Make macros for plotting the next pixel from 'b' (= 8 packed pixels)
// or skipping the pixel.
#if CU_FLIPX==0
#define SKIP { ADV b<<=4; }

#if CU_PMSK==0
#define PIX { if (b&0xf0000000) { unsigned int c; c=ctp[(b>>28)^15]; PLOT_ADV b<<=4; } else SKIP }
#else
#define PIX { unsigned int c; c=(b>>28)^15; if (pmsk&(1<<c)) { c=ctp[c]; PLOT_ADV b<<=4; } else SKIP }
#endif

#else
#define SKIP { ADV b>>=4; }

#if CU_PMSK==0
#define PIX { if (b&0x0000000f) { unsigned int c; c=ctp[(b &15)^15]; PLOT_ADV b>>=4; } else SKIP }
#else
#define PIX { unsigned int c; c=(b &15)^15;  if (pmsk&(1<<c)) { c=ctp[c]; PLOT_ADV b>>=4; } else SKIP }
#endif

#endif

#define EIGHT(x) x x x x x x x x

#if CU_CARE==1

// If we need to clip left or right, check nCtvRollX before plotting
#define DO_PIX if (rx&0x20004000) SKIP else PIX  rx+=0x7fff;
#else
// Always plot
#define DO_PIX PIX
#endif

#define TAKE_B nBlank&=b; b=~b; EIGHT(DO_PIX)

#if   CU_SIZE==8
 // 8x8 tiles
  b=*((unsigned int *)(pCtvTile+0)); TAKE_B
#elif CU_SIZE==16
 // 16x16 tiles
#if CU_FLIPX==0
  b=*((unsigned int *)(pCtvTile+0)); TAKE_B
  b=*((unsigned int *)(pCtvTile+4)); TAKE_B
#else
  b=*((unsigned int *)(pCtvTile+4)); TAKE_B
  b=*((unsigned int *)(pCtvTile+0)); TAKE_B
#endif

#elif CU_SIZE==32
 // 16x16 tiles
#if CU_FLIPX==0
  b=*((unsigned int *)(pCtvTile+ 0)); TAKE_B
  b=*((unsigned int *)(pCtvTile+ 4)); TAKE_B
  b=*((unsigned int *)(pCtvTile+ 8)); TAKE_B
  b=*((unsigned int *)(pCtvTile+12)); TAKE_B
#else
  b=*((unsigned int *)(pCtvTile+12)); TAKE_B
  b=*((unsigned int *)(pCtvTile+ 8)); TAKE_B
  b=*((unsigned int *)(pCtvTile+ 4)); TAKE_B
  b=*((unsigned int *)(pCtvTile+ 0)); TAKE_B
#endif

#else
#error Unsupported CU_SIZE
#endif

#undef TAKE_B
#undef DO_PIX
#undef EIGHT
#undef PIX
#undef SKIP

#undef ADV
#undef PLOT_ADV
}

  return nBlank==0xffffffff;
}
