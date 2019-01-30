// Calc functions

static INLINE void CalcMult0(unsigned short *pm)
{
  int x;
  // Do the function of 0000: [0]*[2] -> [4].long
  x=(short)pm[0]*(short)pm[1];
  pm[2]=(unsigned short)(x>>16);
  pm[3]=(unsigned short)(x&0xffff);
}

static INLINE void CalcDiv4(unsigned short *pm,unsigned int a)
{
  int f,g,r;
  // Do the function of 4000 (changes depending on which address is hit)

  f=(pm[0]<<16)|pm[1];
  if ((a&6)==6) { g=(pm[2]<<16)|pm[3]; } // Divide by 32-bit value
  else          { g=(short)pm[2]; }      // Divide by 16-bit value

  if (a&8)
  {
    if (g!=0) r=f/g; else r=0x7fffffff;
    pm[4]=(unsigned short)(r>>16);
    pm[5]=(unsigned short)r;
  }
  else
  {
    if (g!=0) r=f/g; else r=0x7fff;
    pm[4]=(unsigned short)r;
    if (g!=0) r=f%g; else r=0x7fff;
    pm[5]=(unsigned short)r;
  }
}

static INLINE void CalcDo8(unsigned short *pm)
{
  int x,f,g,h,z;
  // Do the function of 8000

  f=(short)pm[0];
  g=(short)pm[1];
  h=(short)pm[2];

  // this function is unknown - only used on helicopters? (0E8) and level 5 (2E8) ??
  // also used to attract the plane to the refueller before level 3

  z=0;
  if (f>g) { if (h<0) z=-1; if (h>0) z=+1; } // Used mainly in After Burner II
  else     { if (h<f) z=-1; if (h>g) z=+1; } // Used mainly in Thunder Blade

  x=z;
  pm[3]=(unsigned short)(x&0xffff);
}
