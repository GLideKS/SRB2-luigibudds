// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2025 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_splats.c
/// \brief Floor splats

#include "r_draw.h"
#include "r_fps.h"
#include "r_main.h"
#include "r_splats.h"
#include "r_things.h"
#include "r_bsp.h"
#include "p_local.h"
#include "p_slopes.h"
#include "w_wad.h"
#include "z_zone.h"

#define NEARZ 1.0

// ==========================================================================
//                                                               FLOOR SPLATS
// ==========================================================================

static struct rasteryfix_s
{
	fixed_t minx, maxx; // for each raster line starting at line 0
} rastertab[MAXVIDHEIGHT];

static void R_RasterizeFloorSplat(floorsplat_t *pSplat, vector2_t *verts, int start, int end, boolean backface);

static void RasterizeSegment(INT32 x1, INT32 y1, INT32 x2, INT32 y2)
{
	fixed_t x, dx;
	INT32 count;

	if (y1 == y2)
		return;

	if (y2 > y1)
	{
		count = (y2 - y1) + 1;
		dx = FixedDiv((x2 - x1) << FRACBITS, count << FRACBITS);
		x = x1 << FRACBITS;

		while (count-- > 0)
		{
			if (x > rastertab[y1].maxx)
				rastertab[y1].maxx = x;
			x += dx;
			y1++;
		}
	}
	else
	{
		count = (y1 - y2) + 1;
		dx = FixedDiv((x1 - x2) << FRACBITS, count << FRACBITS);
		x = x2 << FRACBITS;

		while (count-- > 0)
		{
			if (x < rastertab[y2].minx)
				rastertab[y2].minx = x;
			x += dx;
			y2++;
		}
	}
}

static void ProjectTriangle(dvector3_t *transformed, floorsplat_t *pSplat)
{
	vector2_t v2d[3];

	boolean backface = false;

	for (unsigned i = 0; i < 3; i++)
	{
		// Do not project the triangle if there are vertices behind the camera
		if (transformed[i].y < NEARZ)
			return;

		if (transformed[i].z > 0)
			backface = true;

#if 1
		// note: y from view above of map, is distance far away
		double xscale = FixedToDouble(projection) / transformed[i].y;
		double yscale = -FixedToDouble(projectiony) / transformed[i].y;

		// projection
		v2d[i].x = (int)(FixedToDouble(centerxfrac) + (transformed[i].x * xscale));
		v2d[i].y = (int)(FixedToDouble(centeryfrac) + (transformed[i].z * yscale));
#else
		// note: y from view above of map, is distance far away
		fixed_t xscale = FixedDiv(projection, transformed[i].y);
		fixed_t yscale = -FixedDiv(projectiony, transformed[i].y);

		// projection
		v2d[i].x = (centerxfrac + FixedMul(transformed[i].x, xscale))>>FRACBITS;
		v2d[i].y = (centeryfrac + FixedMul(transformed[i].z, yscale))>>FRACBITS;
#endif
	}

	R_RasterizeFloorSplat(pSplat, v2d, 0, 2, backface);
}

static void ClipTransform(dvector3_t *front, dvector3_t *behind)
{
#if 1
	double ratio = (NEARZ - front->y) / (front->y - behind->y);
	behind->x = front->x + ((front->x - behind->x) / ratio);
	behind->z = front->z + ((front->z - behind->z) / ratio);
#else
	fixed_t ratio = FixedDiv(NEARZ - front->y, front->y - behind->y);
	behind->x = front->x - FixedDiv(front->x - behind->x, ratio);
	behind->z = front->z - FixedDiv(front->z - behind->z, ratio);
#endif
	behind->y = NEARZ;
}

static void TransformTriangle(vector3_t *triangle, vissprite_t *spr, floorsplat_t *pSplat)
{
	fixed_t x = spr->mobj->x - pSplat->x;
	fixed_t y = spr->mobj->y - pSplat->y;
	angle_t angle = spr->viewpoint.angle >> ANGLETOFINESHIFT;
	fixed_t ca = FINECOSINE(angle);
	fixed_t sa = FINESINE(angle);

	dvector3_t transformed[3];

	unsigned numfront = 0;
	unsigned numbehind = 0;
	unsigned front_idx[3];
	unsigned behind_idx[3];

	for (unsigned i = 0; i < 3; i++)
	{
		vector3_t *v3d = &triangle[i];

		// transform the origin point
		fixed_t tr_x = v3d->x - spr->viewpoint.x;
		fixed_t tr_y = v3d->y - spr->viewpoint.y;

		// rotation around vertical y axis
		transformed[i].x = FixedToDouble(FixedMul(tr_x, sa) - FixedMul(tr_y, ca));
		transformed[i].y = FixedToDouble(FixedMul(tr_x, ca) + FixedMul(tr_y, sa));
		transformed[i].z = FixedToDouble(v3d->z - spr->viewpoint.z);

		// Store which vertices are behind or in front of the camera, for clipping
		if (transformed[i].y < NEARZ)
		{
			behind_idx[numbehind] = i;
			numbehind++;
		}
		else
		{
			front_idx[numfront] = i;
			numfront++;
		}
	}

	// All vertices are behind the camera
	if (numbehind == 3)
	{
		return;
	}
	// An entire edge is behind the camera
	else if (numbehind == 2)
	{
		// Clip vertices
		for (unsigned i = 0; i < 2; i++)
		{
			ClipTransform(&transformed[front_idx[0]], &transformed[behind_idx[i]]);
		}
	}
	// Only one vertex is behind the camera
	else if (numbehind == 1)
	{
		// Make a new triangle and clip it
		dvector3_t transformed2[3];

		transformed2[0] = transformed[front_idx[1]];

		for (unsigned i = 0; i < 2; i++)
		{
			transformed2[i + 1] = transformed[behind_idx[0]];

			ClipTransform(&transformed[front_idx[i]], &transformed2[i + 1]);
		}

		// Project the new triangle
		ProjectTriangle(transformed2, pSplat);

		// Modify the first triangle so that it shares a vertex with the second one
		transformed[behind_idx[0]] = transformed2[2];
	}

	// Project the triangle
	ProjectTriangle(transformed, pSplat);
}

boolean R_AddFloorSplat(vissprite_t *spr)
{
	floorsplat_t *splat = NULL;
	mobj_t *mobj = spr->mobj;
	fixed_t tr_x, tr_y;

	vector2_t rotated[4];

	fixed_t x, y;
	fixed_t w, h;
	angle_t angle, splatangle;
	fixed_t ca, sa;
	fixed_t xscale, yscale;
	fixed_t xoffset, yoffset;
	fixed_t leftoffset, topoffset;
	fixed_t splatscale;
	INT32 i;

	boolean hflip = (spr->xiscale < 0);
	boolean vflip = (spr->cut & SC_VFLIP);
	UINT8 flipflags = 0;

	renderflags_t renderflags = spr->renderflags;

	if (hflip)
		flipflags |= PICFLAGS_XFLIP;
	if (vflip)
		flipflags |= PICFLAGS_YFLIP;

	if (!mobj || P_MobjWasRemoved(mobj))
		return false;

	Patch_GenerateFlat(spr->patch, flipflags);
	void *pic = spr->patch->flats[flipflags];
	if (pic == NULL)
		return false;

	splat = &spr->splat;
	splat->pic = pic;

	splatscale = mobj->scale;

	if (mobj->skin && ((skin_t *)mobj->skin)->flags & SF_HIRES)
		splatscale = FixedMul(splatscale, ((skin_t *)mobj->skin)->highresscale);

	if (spr->rotateflags & SRF_3D || renderflags & RF_NOSPLATBILLBOARD)
		splatangle = spr->centerangle;
	else
		splatangle = spr->viewpoint.angle;

	if (!(spr->cut & SC_ISROTATED))
		splatangle += mobj->spriteroll;

	splat->angle = -splatangle;
	splat->angle += ANGLE_90;

	topoffset = spr->spriteyoffset;
	leftoffset = spr->spritexoffset;
	if (hflip)
		leftoffset = ((spr->patch->width * FRACUNIT) - leftoffset);

	xscale = spr->spritexscale;
	yscale = spr->spriteyscale;

	splat->xscale = FixedMul(splatscale, xscale);
	splat->yscale = FixedMul(splatscale, yscale);

	xoffset = FixedMul(leftoffset, splat->xscale);
	yoffset = FixedMul(topoffset, splat->yscale);

	x = spr->gx;
	y = spr->gy;
	w = spr->patch->width * splat->xscale;
	h = spr->patch->height * splat->yscale;

	splat->x = x;
	splat->y = y;
	splat->z = spr->pz;
	splat->slope = NULL;

	// Set positions

	// 3--2
	// |  |
	// 0--1

	splat->verts[0].x = w - xoffset;
	splat->verts[0].y = yoffset;

	splat->verts[1].x = -xoffset;
	splat->verts[1].y = yoffset;

	splat->verts[2].x = -xoffset;
	splat->verts[2].y = -h + yoffset;

	splat->verts[3].x = w - xoffset;
	splat->verts[3].y = -h + yoffset;

	angle = -splat->angle>>ANGLETOFINESHIFT;
	ca = FINECOSINE(angle);
	sa = FINESINE(angle);

	// Rotate
	for (i = 0; i < 4; i++)
	{
		rotated[i].x = FixedMul(splat->verts[i].x, ca) - FixedMul(splat->verts[i].y, sa);
		rotated[i].y = FixedMul(splat->verts[i].x, sa) + FixedMul(splat->verts[i].y, ca);
	}

	if (renderflags & (RF_SLOPESPLAT | RF_OBJECTSLOPESPLAT))
	{
		pslope_t *standingslope = mobj->standingslope; // The slope that the object is standing on.

		// The slope that was defined for the sprite.
		if (renderflags & RF_SLOPESPLAT)
			splat->slope = mobj->floorspriteslope;

		if (standingslope && (renderflags & RF_OBJECTSLOPESPLAT))
			splat->slope = standingslope;
	}

	// Translate
	for (i = 0; i < 4; i++)
	{
		tr_x = rotated[i].x + x;
		tr_y = rotated[i].y + y;

		if (splat->slope)
			splat->verts[i].z = P_GetSlopeZAt(splat->slope, tr_x, tr_y);
		else
			splat->verts[i].z = splat->z;

		splat->verts[i].x = tr_x;
		splat->verts[i].y = tr_y;
	}

	// prepare rastertab
	for (i = 0; i < viewheight; i++)
	{
		rastertab[i].minx = INT32_MAX;
		rastertab[i].maxx = INT32_MIN;
	}

	splat->minx = INT32_MAX;
	splat->maxx = INT32_MIN;
	splat->miny = viewheight + 1;
	splat->maxy = 0;

	// The polygon is split into two triangles, so that clipping it later becomes trivial.
	vector3_t v3d[3];
	v3d[0] = spr->splat.verts[0];
	v3d[1] = spr->splat.verts[1];
	v3d[2] = spr->splat.verts[2];
	TransformTriangle(v3d, spr, &spr->splat);

	v3d[0] = spr->splat.verts[0];
	v3d[1] = spr->splat.verts[2];
	v3d[2] = spr->splat.verts[3];
	TransformTriangle(v3d, spr, &spr->splat);

	// Store splat bounds
	for (i = 0; i < viewheight; i++)
	{
		splat->rastertab[i].minx = rastertab[i].minx >> FRACBITS;
		splat->rastertab[i].maxx = rastertab[i].maxx >> FRACBITS;

		if (splat->rastertab[i].minx < splat->minx)
			splat->minx = splat->rastertab[i].minx;
		if (splat->rastertab[i].maxx > splat->maxx)
			splat->maxx = splat->rastertab[i].maxx;
	}

	if (splat->minx < 0)
		splat->minx = 0;
	if (splat->maxx >= viewwidth)
		splat->maxx = viewwidth - 1;
	if (splat->miny < 0)
		splat->miny = 0;

	if (splat->minx >= splat->maxx)
		return false;
	if (splat->miny >= splat->maxy)
		return false;

	return true;
}

// --------------------------------------------------------------------------
// Rasterize the edges of a floor splat polygon,
// fill the polygon with linear interpolation
// --------------------------------------------------------------------------
static void R_RasterizeFloorSplat(floorsplat_t *pSplat, vector2_t *verts, int start, int end, boolean backface)
{
	// rasterizing
	INT32 miny = viewheight + 1, maxy = 0;
	INT32 x1, ry1, x2, y2, i;
	fixed_t step;

#define RASTERPARAMS(vnum1, vnum2) \
    x1 = verts[vnum1].x; \
    ry1 = verts[vnum1].y; \
    x2 = verts[vnum2].x; \
    y2 = verts[vnum2].y; \
    if (y2 > ry1) \
        step = FixedDiv(x2-x1, y2-ry1+1); \
    else if (y2 == ry1) \
        step = 0; \
    else \
        step = FixedDiv(x2-x1, ry1-y2+1); \
    if (ry1 < 0) { \
        if (step) { \
            x1 <<= FRACBITS; \
            x1 += (-ry1)*step; \
            x1 >>= FRACBITS; \
        } \
        ry1 = 0; \
    } \
    if (ry1 >= viewheight) { \
        if (step) { \
            x1 <<= FRACBITS; \
            x1 -= (viewheight-1-ry1)*step; \
            x1 >>= FRACBITS; \
        } \
        ry1 = viewheight - 1; \
    } \
    if (y2 < 0) { \
        if (step) { \
            x2 <<= FRACBITS; \
            x2 -= (-y2)*step; \
            x2 >>= FRACBITS; \
        } \
        y2 = 0; \
    } \
    if (y2 >= viewheight) { \
        if (step) { \
            x2 <<= FRACBITS; \
            x2 += (viewheight-1-y2)*step; \
            x2 >>= FRACBITS; \
        } \
        y2 = viewheight - 1; \
    } \
    RasterizeSegment(x1, ry1, x2, y2); \
    if (ry1 < miny) \
        miny = ry1; \
    if (ry1 > maxy) \
        maxy = ry1;

	// Rasterize each edge, from the last vertex to the first one
	if (backface)
	{
		for (i = start; i < end; i++)
		{
			RASTERPARAMS(i, i + 1);
		}

		RASTERPARAMS(i, start);
	}
	else
	{
		for (i = end; i > start; i--)
		{
			RASTERPARAMS(i, i - 1);
		}

		RASTERPARAMS(i, end);
	}

#undef RASTERPARAMS

	if (maxy >= viewheight)
		maxy = viewheight-1;

	if (miny < pSplat->miny)
		pSplat->miny = miny;
	if (maxy > pSplat->maxy)
		pSplat->maxy = maxy;
}

// Call span drawer for each scan line
static void R_DrawRasterizedFloorSplat(floorsplat_t *pSplat, vissprite_t *vis)
{
	INT32 miny = pSplat->miny, maxy = pSplat->maxy;
	INT32 i;

	fixed_t offsetx = 0, offsety = 0;
	fixed_t planeheight = 0;

	int spanfunctype;

	ds_source = (UINT8 *)pSplat->pic;
	ds_flatwidth = vis->patch->width;
	ds_flatheight = vis->patch->height;

	ds_powersoftwo = ds_solidcolor = ds_fog = false;

	if (R_CheckSolidColorFlat())
		ds_solidcolor = true;
	else if (R_CheckPowersOfTwo())
	{
		R_SetFlatVars(ds_flatwidth * ds_flatheight);
		ds_powersoftwo = true;
	}

	if (pSplat->slope)
	{
		R_SetScaledSlopePlane(pSplat->slope, vis->viewpoint.x, vis->viewpoint.y, vis->viewpoint.z, (INT64)pSplat->xscale, (INT64)pSplat->yscale, -pSplat->verts[0].x, pSplat->verts[0].y, vis->viewpoint.angle, pSplat->angle);
	}
	else if (!ds_solidcolor)
	{
		planeheight = abs(pSplat->z - vis->viewpoint.z);

		if (pSplat->angle)
		{
			// Add the view offset, rotated by the plane angle.
			fixed_t a = -pSplat->verts[0].x + vis->viewpoint.x;
			fixed_t b = -pSplat->verts[0].y + vis->viewpoint.y;
			angle_t angle = (pSplat->angle >> ANGLETOFINESHIFT);
			offsetx = FixedMul(a, FINECOSINE(angle)) - FixedMul(b, FINESINE(angle));
			offsety = -FixedMul(a, FINESINE(angle)) - FixedMul(b, FINECOSINE(angle));
		}
		else
		{
			offsetx = vis->viewpoint.x - pSplat->verts[0].x;
			offsety = pSplat->verts[0].y - vis->viewpoint.y;
		}
	}

	ds_colormap = vis->colormap;
	ds_translation = R_GetTranslationForThing(vis->mobj, vis->color, vis->translation);
	if (ds_translation == NULL)
		ds_translation = colormaps;

	if (vis->extra_colormap)
	{
		if (!ds_colormap)
			ds_colormap = vis->extra_colormap->colormap;
		else
			ds_colormap = &vis->extra_colormap->colormap[ds_colormap - colormaps];
	}

	ds_transmap = vis->transmap;

	// Determine which R_DrawWhatever to use

	// Solid color
	if (ds_solidcolor)
	{
		UINT16 px = *(UINT16 *)ds_source;

		// Uh, it's not visible.
		if (!(px & 0xFF00))
			return;

		// Pixel color is contained in the lower 8 bits (upper 8 are the opacity), so advance the pointer
		ds_source++;

		if (pSplat->slope)
		{
			if (ds_transmap)
				spanfunctype = SPANDRAWFUNC_TILTEDTRANSSOLID;
			else
				spanfunctype = SPANDRAWFUNC_TILTEDSOLID;
		}
		else
		{
			if (ds_transmap)
				spanfunctype = SPANDRAWFUNC_TRANSSOLID;
			else
				spanfunctype = SPANDRAWFUNC_SOLID;
		}
	}
	// Transparent
	else if (ds_transmap)
	{
		if (pSplat->slope)
			spanfunctype = SPANDRAWFUNC_TILTEDTRANSSPRITE;
		else
			spanfunctype = SPANDRAWFUNC_TRANSSPRITE;
	}
	// Opaque
	else
	{
		if (pSplat->slope)
			spanfunctype = SPANDRAWFUNC_TILTEDSPRITE;
		else
			spanfunctype = SPANDRAWFUNC_SPRITE;
	}

	if (ds_powersoftwo || ds_solidcolor)
		spanfunc = spanfuncs[spanfunctype];
	else
		spanfunc = spanfuncs_npo2[spanfunctype];

	for (INT32 y = miny; y <= maxy; y++)
	{
		INT32 x1 = pSplat->rastertab[y].minx;
		INT32 x2 = pSplat->rastertab[y].maxx;

		if (x1 > x2 || x1 == INT16_MIN || x2 == INT16_MAX)
			continue;

		if (x1 < 0)
			x1 = 0;
		if (x2 >= viewwidth)
			x2 = viewwidth - 1;

		if (x1 >= viewwidth || x2 < 0)
			continue;

		// clip left
		while (y >= mfloorclip[x1] || y <= mceilingclip[x1])
		{
			x1++;
			if (x1 >= viewwidth)
				break;
		}

		// clip right
		i = x2;

		while (i > x1)
		{
			if (y >= mfloorclip[i] || y <= mceilingclip[i])
				x2 = i-1;
			i--;
			if (i < 0)
				break;
		}

		if (x2 < x1)
			continue;

		if (!ds_solidcolor && !pSplat->slope)
		{
			fixed_t xstep, ystep;
			fixed_t distance, span;

			angle_t angle = (vis->viewpoint.angle + pSplat->angle)>>ANGLETOFINESHIFT;
			angle_t planecos = FINECOSINE(angle);
			angle_t planesin = FINESINE(angle);

			// [RH] Notice that I dumped the caching scheme used by Doom.
			// It did not offer any appreciable speedup.
			distance = FixedMul(planeheight, yslope[y]);
			span = abs(centery - y);

			if (span) // Don't divide by zero
			{
				xstep = FixedMul(planesin, planeheight) / span;
				ystep = FixedMul(planecos, planeheight) / span;
			}
			else
				xstep = ystep = FRACUNIT;

			ds_xstep = FixedDiv(xstep, pSplat->xscale);
			ds_ystep = FixedDiv(ystep, pSplat->yscale);

			ds_xfrac = FixedDiv(offsetx + FixedMul(planecos, distance) + (x1 - centerx) * xstep, pSplat->xscale);
			ds_yfrac = FixedDiv(offsety - FixedMul(planesin, distance) + (x1 - centerx) * ystep, pSplat->yscale);
		}

		ds_y = y;
		ds_x1 = x1;
		ds_x2 = x2;
		spanfunc();
	}
}

void R_DrawFloorSplat(vissprite_t *spr)
{
	R_DrawRasterizedFloorSplat(&spr->splat, spr);
}