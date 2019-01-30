
#define DIRECTDRAW_VERSION 0x0300 // Use this DirectX version
#include <ddraw.h>

extern IDirectDraw2 *DtoDD; // DirectDraw interface
extern IDirectDrawSurface *DtoPrim; // Primary surface
extern IDirectDrawSurface *DtoBack; // Back buffer surface
int DtoClear(IDirectDrawSurface *Surf);
int DtoPrimClear();

// dtos.cpp
extern IDirectDrawSurface *pddsDtos; // The screen surface
int DtosInit();
int DtosExit();
int DtosFrame();
int DtosPaint(int bValidate);
