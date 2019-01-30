{
  unsigned char *pLine;
// This is included several times to create rendering function for each color depth

  BmapY=StartBmapY,pLine=pBurnDraw+bss.Top*nBurnPitch+StartPix;

  for (y=0; y<bss.Height; y++,BmapY+=bss.IncY, pLine+=nBurnPitch)
  {
    unsigned char *pPix,*pEnd; // Pointer to the output bitmap pixel/the end of the line
    int BmapX,BmapLine,nRealY;

    // Find which line we are on (output bitmap)
    nRealY=bss.Top+y;

    BmapLine=bss.Base+(BmapY>>9)*(bss.Width>>1); // Point to fractional sprite line

    // Clip to sprite data length
    if (BmapLine<0) continue;
    if (BmapLine>=BsysSprDataLen) continue;

    pPix=pLine; // Find line on screen
    pEnd=pLine+PixLen; // Find end

    for (BmapX=StartBmapX; pPix<pEnd;
         
#if BSS_DEPTH==4
         pPix+=4,
#elif BSS_DEPTH==3
         pPix+=3,
#elif BSS_DEPTH==2
         pPix+=2,
#else
         pPix++,
#endif
         
         BmapX+=bss.IncX)
    {
      int Pos,c;
      // Find this pixel out of the whole Bitmap memory block
      
      Pos=BmapLine+(BmapX>>10); 
      c=BsysSprData[Pos];  if ((BmapX&0x200)==0) c>>=4;  c&=15;

      if (bss.Trans&(1<<c)) continue; // Skip if transparent

      c=bss.pPal[c];
#if BSS_DEPTH==4
      *((unsigned int *)pPix)=c;
#elif BSS_DEPTH==3
      pPix[0]=(unsigned char) c;
      pPix[1]=(unsigned char)(c>>8);
      pPix[2]=(unsigned char)(c>>16);
#elif BSS_DEPTH==2
      *((unsigned short *)pPix)=(unsigned short)c;
#else
      *pPix=(unsigned char)c;
#endif

    }
  }
}
