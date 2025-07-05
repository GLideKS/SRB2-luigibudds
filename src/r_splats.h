// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2025 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_splats.h
/// \brief Flat sprites & splats effects

#ifndef __R_SPLATS_H__
#define __R_SPLATS_H__

#include "r_defs.h"

struct vissprite_s;

// ==========================================================================
// DEFINITIONS
// ==========================================================================

struct rastery_s
{
	INT16 minx, maxx; // for each raster line starting at line 0
};

typedef struct floorsplat_s
{
	UINT16 *pic;
	fixed_t xscale, yscale;
	angle_t angle;
	pslope_t *slope;

	vector3_t verts[4]; // (x,y,z) as viewed from above on map
	fixed_t x, y, z; // position

	INT32 miny, maxy;
	INT32 minx, maxx;
	struct rastery_s rastertab[MAXVIDHEIGHT];
} floorsplat_t;

boolean R_AddFloorSplat(struct vissprite_s *spr);
void R_DrawFloorSplat(struct vissprite_s *spr);

#endif /*__R_SPLATS_H__*/
