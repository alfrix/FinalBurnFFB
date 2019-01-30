#include <stdio.h>

// Create the ctv.h header file
// which includes all combinations of the cps tile drawing functions

int main()
{
  int CuPmsk=0;
  int CuBpp=0;
  int CuSize=0;
  int CuRows=0;
  int CuCare=0;
  int CuFlipX=0;

  for (CuBpp=1; CuBpp<=4; CuBpp++)
  {
    printf ("#define CU_BPP   (%d)\n\n",CuBpp);
    for (CuPmsk=0; CuPmsk<2; CuPmsk++)
    {
      printf ("#define CU_PMSK  (%d)\n\n",CuPmsk);
      for (CuSize=8; CuSize<=32; CuSize<<=1)
      {
        printf ("#define CU_SIZE  (%d)\n\n",CuSize);
        for (CuRows=0; CuRows<2; CuRows++)
        {
          printf ("#define CU_ROWS  (%d)\n\n",CuRows);
          for (CuCare=0; CuCare<2; CuCare++)
          {
            printf ("#define CU_CARE  (%d)\n",CuCare);
            for (CuFlipX=0; CuFlipX<2; CuFlipX++)
            {
              printf ("#define CU_FLIPX (%d)\n",CuFlipX);
          
              printf ("static int CtvDo");
              printf ("%d_",CuBpp);
              printf (CuPmsk?"p":"_");
              printf ("%.2d",CuSize);
              printf (CuRows?"r":"_");
              printf (CuCare?"c":"_");
              printf (CuFlipX?"f":"_");
              printf ("()\n");
              printf ("#include \"ctv_do.h\"\n");
          
              printf ("#undef  CU_FLIPX\n");
            }
            printf ("#undef  CU_CARE\n\n");
          }
          printf ("#undef  CU_ROWS\n\n");
        }
        printf ("#undef  CU_SIZE\n\n");
      }
      printf ("#undef  CU_PMSK\n\n");
    }
    printf ("#undef  CU_BPP\n\n");
  }


  printf ("\n\n");

  printf ("// Filler function\n");
  printf ("static int CtvDoNull() { return 0; }\n\n\n\n");

  for (CuBpp=1; CuBpp<=4; CuBpp++)
  {
    int i=0;

    printf ("// Lookup table for %d bpp\n",CuBpp);
    printf ("static CtvDoFn CtvDo%d[0x40]={\n",CuBpp);
    for (i=0;i<0x40;i++)
    {
      int s=0; // Tile size
      
      s=(i&24)+8;
      if (s!=8 && s!=16 && s!=32) { printf ("CtvDoNull    ",s); goto End; }

      printf ("CtvDo");
      printf ("%d_",CuBpp);
      printf (i&0x20?"p":"_");
      printf ("%.2d",s);
      printf (i&4?"r":"_");
      printf (i&2?"c":"_");
      printf (i&1?"f":"_");
      
    End:
      printf (",");
      if (((i+1)&3)==0) printf("\n");
    }
    printf ("};\n");
  }

  printf ("\n\n");
  printf ("// Current BPP:\n");
  printf ("CtvDoFn CtvDoX[0x40];\n");
  printf ("\n\n");

  return 0;
}
