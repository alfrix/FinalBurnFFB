#include "../burnint.h"
// CPS2 - Driver

unsigned char CpiAllThree[4]={0,0,0,0};

// Inputs common to all Cps2 Fighter Style games (3 x Punch,3 x Kick)
struct BurnInputInfo CpsFsi[31]=
{
  {"P1 Coin"       , 0, CpsInp020+4, "p1 coin"},
  {"P1 Start"      , 0, CpsInp020+0, "p1 start"},
// 2
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
// 6
  {"P1 Low Punch"  , 0, CpsInp001+4, "p1 fire 4"},
  {"P1 Mid Punch"  , 0, CpsInp001+5, "p1 fire 5"},
  {"P1 High Punch" , 0, CpsInp001+6, "p1 fire 6"},
  {"P1 Low Kick"   , 0, CpsInp011+0, "p1 fire 1"},
  {"P1 Mid Kick"   , 0, CpsInp011+1, "p1 fire 2"},
  {"P1 High Kick"  , 0, CpsInp011+2, "p1 fire 3"},
  {"P1 All Punches", 0, CpiAllThree+0 , ""},
  {"P1 All Kicks"  , 0, CpiAllThree+1 , ""},

  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
// 16
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
// 20
  {"P2 Low Punch"  , 0, CpsInp000+4, "p2 fire 4"},
  {"P2 Mid Punch"  , 0, CpsInp000+5, "p2 fire 5"},
  {"P2 High Punch" , 0, CpsInp000+6, "p2 fire 6"},
  {"P2 Low Kick"   , 0, CpsInp011+4, "p2 fire 1"},
  {"P2 Mid Kick"   , 0, CpsInp011+5, "p2 fire 2"},
  {"P2 High Kick"  , 0, CpsInp020+6, "p2 fire 3"},
  {"P2 All Punches", 0, CpiAllThree+2 , ""},
  {"P2 All Kicks"  , 0, CpiAllThree+3 , ""},

// 28
  {"Reset"         , 0, &CpsReset  , "reset"},
  {"Diagnostic"    , 0, CpsInp021+1, "diag"},
  {"Service"       , 0, CpsInp021+2, "service"},
// 31
};

int CpsFsAllThree()
{
  // Process all three punches and kicks
  if (CpiAllThree[0]) { CpsFsi[ 6].pVal[0]=1; CpsFsi[ 7].pVal[0]=1; CpsFsi[ 8].pVal[0]=1; }
  if (CpiAllThree[1]) { CpsFsi[ 9].pVal[0]=1; CpsFsi[10].pVal[0]=1; CpsFsi[11].pVal[0]=1; }
  if (CpiAllThree[2]) { CpsFsi[20].pVal[0]=1; CpsFsi[21].pVal[0]=1; CpsFsi[22].pVal[0]=1; }
  if (CpiAllThree[3]) { CpsFsi[23].pVal[0]=1; CpsFsi[24].pVal[0]=1; CpsFsi[25].pVal[0]=1; }
  return 0;
}

int CpsFsStopOpposite()
{
  // Stop opposite directions being pressed at the same time:
  ONLY_ONE(001,0,1) ONLY_ONE(001,2,3) // Player 1
  ONLY_ONE(000,0,1) ONLY_ONE(000,2,3) // Player 2
  return 0;
}
