// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2024 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  screen.h
/// \brief Handles multiple resolutions

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "command.h"

#if defined (_WIN32) && !defined (__CYGWIN__)
#define RPC_NO_WINDOWS_H
#include <windows.h>
#define DNWH HWND
#else
#define DNWH void * // unused in DOS version
#endif

// quickhack for V_Init()... to be cleaned up
#ifdef NOPOSTPROCESSING
#define NUMSCREENS 2
#else
#define NUMSCREENS 5
#endif

// used now as a maximum video mode size for extra vesa modes.

// we try to re-allocate a minimum of buffers for stability of the memory,
// so all the small-enough tables based on screen size, are allocated once
// and for all at the maximum size.
#define MAXVIDWIDTH 1920 // don't set this too high because actually
#define MAXVIDHEIGHT 1200 // lots of tables are allocated with the MAX size.
#define BASEVIDWIDTH 320 // NEVER CHANGE THIS! This is the original
#define BASEVIDHEIGHT 200 // resolution of the graphics.

// global video state
typedef struct viddef_s
{
	INT32 modenum; // vidmode num indexes videomodes list

	UINT8 *buffer; // invisible screens buffer
	size_t rowbytes; // bytes per scanline of the VIDEO mode
	INT32 width; // PIXELS per scanline
	INT32 height;
	union { // don't need numpages for OpenGL, so we can use it for fullscreen/windowed mode
		INT32 numpages; // always 1, page flipping todo
		INT32 windowed; // windowed or fullscren mode?
	} u;
	INT32 recalc; // if true, recalc vid-based stuff
	UINT8 *direct; // linear frame buffer, or vga base mem.
	INT32 dup; // scale 1, 2, 3 value for menus & overlays
	INT32/*fixed_t*/ fdup; // same as dup, but exact value when aspect ratio isn't 320/200
	INT32 bpp; // BYTES per pixel: 1 = 256color

	INT32 baseratio; // Used to get the correct value for lighting walls

	// for Win32 version
	DNWH WndParent; // handle of the application's window
	UINT8 smalldup; // factor for a little bit of scaling
	UINT8 meddup; // factor for moderate, but not full, scaling
#ifdef HWRENDER
	INT32/*fixed_t*/ fsmalldup;
	INT32/*fixed_t*/ fmeddup;
	INT32 glstate;
#endif
} viddef_t;

enum
{
	VID_GL_LIBRARY_NOTLOADED  = 0,
	VID_GL_LIBRARY_LOADED     = 1,
	VID_GL_LIBRARY_ERROR      = -1,
};

// ---------------------------------------------
// color mode dependent drawer function pointers
// ---------------------------------------------

#define BASEDRAWFUNC 0

enum
{
	COLDRAWFUNC_BASE = BASEDRAWFUNC,
	COLDRAWFUNC_FUZZY,
	COLDRAWFUNC_TRANS,
	COLDRAWFUNC_SHADE,
	COLDRAWFUNC_SHADOWED,
	COLDRAWFUNC_TRANSTRANS,
	COLDRAWFUNC_CLAMPED,
	COLDRAWFUNC_CLAMPEDTRANS,
	COLDRAWFUNC_TWOSMULTIPATCH,
	COLDRAWFUNC_TWOSMULTIPATCHTRANS,
	COLDRAWFUNC_FOG,

	COLDRAWFUNC_MAX
};

extern void (*colfunc)(void);
extern void (*colfuncs[COLDRAWFUNC_MAX])(void);

enum
{
	SPANDRAWFUNC_BASE = BASEDRAWFUNC,
	SPANDRAWFUNC_TRANS,
	SPANDRAWFUNC_TILTED,
	SPANDRAWFUNC_TILTEDTRANS,

	SPANDRAWFUNC_SPLAT,
	SPANDRAWFUNC_TRANSSPLAT,
	SPANDRAWFUNC_TILTEDSPLAT,
	SPANDRAWFUNC_TILTEDTRANSSPLAT,

	SPANDRAWFUNC_SPRITE,
	SPANDRAWFUNC_TRANSSPRITE,
	SPANDRAWFUNC_TILTEDSPRITE,
	SPANDRAWFUNC_TILTEDTRANSSPRITE,

	SPANDRAWFUNC_WATER,
	SPANDRAWFUNC_TILTEDWATER,

	SPANDRAWFUNC_SOLID,
	SPANDRAWFUNC_TRANSSOLID,
	SPANDRAWFUNC_TILTEDSOLID,
	SPANDRAWFUNC_TILTEDTRANSSOLID,
	SPANDRAWFUNC_WATERSOLID,
	SPANDRAWFUNC_TILTEDWATERSOLID,

	SPANDRAWFUNC_FOG,
	SPANDRAWFUNC_TILTEDFOG,

	SPANDRAWFUNC_MAX
};

extern void (*spanfunc)(void);
extern void (*spanfuncs[SPANDRAWFUNC_MAX])(void);
extern void (*spanfuncs_npo2[SPANDRAWFUNC_MAX])(void);

// ----------------
// screen variables
// ----------------
extern viddef_t vid;
extern INT32 setmodeneeded; // mode number to set if needed, or 0
extern UINT8 setrenderneeded;

extern double averageFPS;

void SCR_ChangeRenderer(void);

extern CV_PossibleValue_t cv_renderer_t[];

extern INT32 scr_bpp;

extern consvar_t cv_scr_width, cv_scr_height, cv_scr_width_w, cv_scr_height_w, cv_scr_depth, cv_fullscreen;
extern consvar_t cv_renderer;
// wait for page flipping to end or not
extern consvar_t cv_vidwait;
extern consvar_t cv_timescale;

// Initialize the screen
void SCR_Startup(void);

// Change video mode, only at the start of a refresh.
void SCR_SetMode(void);

// Set drawer functions for Software
void SCR_SetDrawFuncs(void);

// Recalc screen size dependent stuff
void SCR_Recalc(void);

// Check parms once at startup
void SCR_CheckDefaultMode(void);

// Set the mode number which is saved in the config
void SCR_SetDefaultMode(void);

void SCR_CalculateFPS(void);

FUNCMATH boolean SCR_IsAspectCorrect(INT32 width, INT32 height);

// move out to main code for consistency
void SCR_DisplayTicRate(void);
void SCR_ClosedCaptions(void);
void SCR_DisplayLocalPing(void);
void SCR_DisplayMarathonInfo(void);
#undef DNWH
#endif //__SCREEN_H__
