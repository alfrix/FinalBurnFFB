#include "sys.h"
// draw sprites

unsigned char *BsysSprData=NULL; // Sprite bitmap data
int BsysSprDataLen=0; // Length of above

struct BsysSprite bss; // Sprite to draw

int BsysSprDraw()
{
  int y,BmapY;
  int ScrWidth,StartPix,PixLen,StartBmapX,StartBmapY;
  if (bss.IncX<=0) return 1;
  if (bss.IncY==0) return 1;

  ScrWidth=(bss.ClipWidth<<9)/bss.IncX; // Work out how wide the sprite will be on the screen
  if (bss.FlipX) { bss.Base+=bss.Width>>1; bss.IncX=-bss.IncX; } // reverse the scan direction
  if (bss.DrawToLeft)
  {
    // Modify so we can draw it left to right
    if (bss.FlipX) bss.Base-=bss.Width>>1;
    else            bss.Base+=bss.Width>>1;
    bss.Left-=ScrWidth-1;
    bss.IncX=-bss.IncX;
  }
  // ------- Sprite is now a simple left to right one -------------

  if (bss.DrawToTop)
  {
    int nDataHeight;
    nDataHeight=(bss.Height*bss.IncY)>>9; // Work out how high the sprite is in memory
    if (nDataHeight>0) nDataHeight--; else nDataHeight++;
    // Modify so we can draw it top to bottom
    bss.Top-=bss.Height;
    bss.Base+=(bss.Width>>1)*nDataHeight;
    bss.IncY=-bss.IncY;
  }

  // ------- Sprite is now a simple top to bottom one -------------

  // Clip horizontally
  if (bss.Left<0)
  {
    // Adjust to clip to left edge
    StartBmapX=-bss.Left*bss.IncX; // nBmapX starts further along
    ScrWidth+=bss.Left;             // Width is now smaller
    StartPix=0; bss.Left=0;         // Pix now starts on the left edge
    
  }
  else
  {
    // Adjust to start in the middle of the screen
    StartBmapX=0;                    // nBmapX starts at zero
    StartPix=bss.Left*nBurnBpp; // Pix starts in the middle of the screen
  }

  // Now clip sprite width to the right side
  if (ScrWidth>320-bss.Left) ScrWidth=320-bss.Left;
  // Find the byte count of the width
  PixLen=ScrWidth*nBurnBpp;

  // Clip vertically
  if (bss.Top<0)
  {
    // Adjust to clip to top edge
    StartBmapY=-bss.Top*bss.IncY; // nBmapY starts further along
    bss.Height+=bss.Top;           // Height is now smaller
    bss.Top=0;                     // Pix now starts on the top edge
  }
  else
  {
    StartBmapY=0;
  }

  // Now clip sprite to the bottom
  if (bss.Height>224-bss.Top) bss.Height=224-bss.Top;

// Include function for each color depth
if (nBurnBpp==2)
#define BSS_DEPTH (2)
#include "sys_spr.h"
#undef BSS_DEPTH

else if (nBurnBpp==3)
#define BSS_DEPTH (3)
#include "sys_spr.h"
#undef BSS_DEPTH

else if (nBurnBpp==4)
#define BSS_DEPTH (4)
#include "sys_spr.h"
#undef BSS_DEPTH

else
#define BSS_DEPTH (1)
#include "sys_spr.h"
#undef BSS_DEPTH
  return 0;
}
