#include "sys.h"

// Calc chip used in After Burner and Galaxy Force hardware
struct CalcChip Calc[3]; // Up to three maths chips

int CalcScan()
{
  SCAN_VAR(Calc)
  return 0;
}
