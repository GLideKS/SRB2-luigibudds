// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2025 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_things.h
/// \brief Rendering of moving objects, sprites

#ifndef __R_THINGS__
#define __R_THINGS__

#include "r_plane.h"
#include "r_patch.h"
#include "r_picformats.h"
#include "r_portal.h"
#include "r_defs.h"
#include "r_skins.h"
#include "r_splats.h"

// --------------
// SPRITE LOADING
// --------------

#define FEETADJUST (4<<FRACBITS) // R_AddSingleSpriteDef

spritenum_t R_GetSpriteNumByName(const char *name);

boolean R_AddSingleSpriteDef(const char *sprname, spritedef_t *spritedef, UINT16 wadnum, UINT16 startlump, UINT16 endlump, boolean longname);

//faB: find sprites in wadfile, replace existing, add new ones
//     (only sprites from namelist are added or replaced)
void R_AddSpriteDefs(UINT16 wadnum);

// ---------------------
// MASKED COLUMN DRAWING
// ---------------------

// vars for R_DrawMaskedColumn
extern INT16 *mfloorclip;
extern INT16 *mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t sprbotscreen;
extern fixed_t windowtop;
extern fixed_t windowbottom;

void R_DrawMaskedColumn(column_t *column, unsigned lengthcol);
void R_DrawFlippedMaskedColumn(column_t *column, unsigned lengthcol);
void R_DrawFlippedPost(UINT8 *source, unsigned length, void (*drawcolfunc)(void));

// ----------------
// SPRITE RENDERING
// ----------------

// Constant arrays used for psprite clipping
//  and initializing clipping.
extern INT16 negonearray[MAXVIDWIDTH];
extern INT16 screenheightarray[MAXVIDWIDTH];

fixed_t R_GetShadowZ(mobj_t *thing, pslope_t **shadowslope);

//SoM: 6/5/2000: Light sprites correctly!
void R_AddSprites(sector_t *sec, INT32 lightlevel);
void R_InitSprites(void);
void R_ClearSprites(void);

UINT8 R_GetBoundingBoxColor(mobj_t *thing);
boolean R_ThingBoundingBoxVisible(mobj_t *thing);

boolean R_ThingVisible (mobj_t *thing);

boolean R_ThingWithinDist (mobj_t *thing,
		fixed_t        draw_dist,
		fixed_t nights_draw_dist);

boolean R_PrecipThingVisible (precipmobj_t *precipthing,
		fixed_t precip_draw_dist);

boolean R_ThingHorizontallyFlipped (mobj_t *thing);
boolean R_ThingVerticallyFlipped (mobj_t *thing);

boolean R_ThingIsPaperSprite (mobj_t *thing);
boolean R_ThingIsFloorSprite (mobj_t *thing);

boolean R_ThingIsFullBright (mobj_t *thing);
boolean R_ThingIsSemiBright (mobj_t *thing);
boolean R_ThingIsFullDark (mobj_t *thing);

boolean R_ThingIsFlashing (mobj_t *thing);

UINT8 *R_GetTranslationForThing(mobj_t *mobj, skincolornum_t color, UINT16 translation);
transnum_t R_GetThingTransTable(fixed_t alpha, transnum_t transmap);

void R_ThingOffsetOverlay (mobj_t *thing, fixed_t *outx, fixed_t *outy);

// --------------
// MASKED DRAWING
// --------------
/** Used to count the amount of masked elements
 * per portal to later group them in separate
 * drawnode lists.
 */
typedef struct
{
	size_t drawsegs[2];
	size_t vissprites[2];
	fixed_t viewx, viewy, viewz;			/**< View z stored at the time of the BSP traversal for the view/portal. Masked sorting/drawing needs it. */
	sector_t* viewsector;
} maskcount_t;

void R_DrawMasked(maskcount_t* masks, INT32 nummasks);

// ----------
// VISSPRITES
// ----------

// number of sprite lumps for spritewidth,offset,topoffset lookup tables
// Fab: this is a hack : should allocate the lookup tables per sprite
#define MAXVISSPRITES 2048 // added 2-2-98 was 128

#define VISSPRITECHUNKBITS 6	// 2^6 = 64 sprites per chunk
#define VISSPRITESPERCHUNK (1 << VISSPRITECHUNKBITS)
#define VISSPRITEINDEXMASK (VISSPRITESPERCHUNK - 1)

typedef enum
{
	// actual cuts
	SC_NONE       = 0,
	SC_TOP        = 1,
	SC_BOTTOM     = 1<<1,
	SC_NOTVISIBLE = 1<<2,
	// other flags
	SC_PRECIP     = 1<<3,
	SC_LINKDRAW   = 1<<4,
	SC_FULLBRIGHT = 1<<5,
	SC_SEMIBRIGHT = 1<<6,
	SC_FULLDARK   = 1<<7,
	SC_VFLIP      = 1<<8,
	SC_ISSCALED   = 1<<9,
	SC_ISROTATED  = 1<<10,
	SC_SHADOW     = 1<<11,
	SC_SHEAR      = 1<<12,
	SC_SPLAT      = 1<<13,
	SC_BBOX       = 1<<14,
	// masks
	SC_CUTMASK    = SC_TOP|SC_BOTTOM|SC_NOTVISIBLE,
	SC_FLAGMASK   = ~SC_CUTMASK
} spritecut_e;

// A vissprite_t is a thing that will be drawn during a refresh,
// i.e. a sprite object that is partly visible.
typedef struct vissprite_s
{
	// Doubly linked list.
	struct vissprite_s *prev;
	struct vissprite_s *next;

	// Bonus linkdraw pointer.
	struct vissprite_s *linkdraw;

	mobj_t *mobj; // for easy access

	INT32 x1, x2;

	fixed_t gx, gy; // for line side calculation
	fixed_t gz, gzt; // global bottom/top for silhouette clipping
	fixed_t pz, pzt; // physical bottom/top for sorting with 3D floors

	fixed_t startfrac; // horizontal position of x1
	fixed_t xscale, scale; // projected horizontal and vertical scales
	fixed_t thingscale; // the object's scale
	fixed_t sortscale; // sortscale only differs from scale for paper sprites and floor sprites
	fixed_t linkscale; // the sortscale for MF2_LINKDRAW sprites
	fixed_t xiscale; // negative if flipped

	union {
		// for paper sprites
		struct {
			angle_t centerangle;
			fixed_t scalestep; // only for paper sprites, 0 otherwise
			fixed_t paperoffset, paperdistance; // for paper sprites, offset/dist relative to the angle
		};

		// for floor sprites
		struct {
			floorsplat_t splat; // floorsprite data
			fixed_t sortsplat; // the sortscale from in front of the floor sprite
			struct {
				fixed_t x, y, z; // the viewpoint's current position
				angle_t angle; // the viewpoint's current angle
			} viewpoint;
		};
	};

	struct {
		fixed_t tan; // The amount to shear the sprite vertically per row
		INT32 offset; // The center of the shearing location offset from x1
	} shear;

	fixed_t texturemid;
	patch_t *patch;

	lighttable_t *colormap; // for color translation and shadow draw
	                        // maxbright frames as well

	UINT8 *transmap; // for MF2_SHADOW sprites, which translucency table to use

	INT32 mobjflags;

	INT32 heightsec; // height sector for underwater/fake ceiling support

	extracolormap_t *extra_colormap; // global colormaps

	fixed_t thingheight; // The actual height of the thing (for 3D floors)
	sector_t *sector; // The sector containing the thing.

	// Precalculated top and bottom screen coords for the sprite.
	INT16 sz, szt;

	spritecut_e cut;
	UINT32 renderflags;
	UINT8 rotateflags;

	fixed_t spritexscale, spriteyscale;
	fixed_t spritexoffset, spriteyoffset;

	fixed_t shadowscale;

	skincolornum_t color;
	UINT16 translation;

	INT16 clipbot[MAXVIDWIDTH], cliptop[MAXVIDWIDTH];

	INT32 dispoffset; // copy of mobj->dispoffset, affects ordering but not drawing
} vissprite_t;

extern UINT32 visspritecount, numvisiblesprites;

void R_ClipSprites(drawseg_t* dsstart, portal_t* portal);

void R_DrawThingBoundingBox(vissprite_t *spr);

// ----------
// DRAW NODES
// ----------

// A drawnode is something that points to a 3D floor, 3D side, or masked
// middle texture. This is used for sorting with sprites.
typedef struct drawnode_s
{
	visplane_t *plane;
	drawseg_t *seg;
	drawseg_t *thickseg;
	ffloor_t *ffloor;
	vissprite_t *sprite;

	struct drawnode_s *next;
	struct drawnode_s *prev;
} drawnode_t;

void R_InitDrawNodes(void);

// -----------------------
// SPRITE FRAME CHARACTERS
// -----------------------

// Functions to go from sprite character ID to frame number
// for 2.1 compatibility this still uses the old 'A' + frame code
// The use of symbols tends to be painful for wad editors though
// So the future version of this tries to avoid using symbols
// as much as possible while also defining all 64 slots in a sane manner
// 2.1:    [[ ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~   ]]
// Future: [[ ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz!@ ]]
FUNCMATH FUNCINLINE static ATTRINLINE char R_Frame2Char(UINT8 frame)
{
#if 0 // 2.1 compat
	return 'A' + frame;
#else
	if (frame < 26) return 'A' + frame;
	if (frame < 36) return '0' + (frame - 26);
	if (frame < 62) return 'a' + (frame - 36);
	if (frame == 62) return '!';
	if (frame == 63) return '@';
	return '\xFF';
#endif
}

FUNCMATH FUNCINLINE static ATTRINLINE UINT8 R_Char2Frame(char cn)
{
#if 0 // 2.1 compat
	if (cn == '+') return '\\' - 'A'; // PK3 can't use backslash, so use + instead
	return cn - 'A';
#else
	if (cn >= 'A' && cn <= 'Z') return (cn - 'A');
	if (cn >= '0' && cn <= '9') return (cn - '0') + 26;
	if (cn >= 'a' && cn <= 'z') return (cn - 'a') + 36;
	if (cn == '!') return 62;
	if (cn == '@') return 63;
	return 255;
#endif
}

// "Left" and "Right" character symbols for additional rotation functionality
#define ROT_L 17
#define ROT_R 18

FUNCMATH FUNCINLINE static ATTRINLINE char R_Rotation2Char(UINT8 rot)
{
	if (rot <=     9) return '0' + rot;
	if (rot <=    16) return 'A' + (rot - 10);
	if (rot == ROT_L) return 'L';
	if (rot == ROT_R) return 'R';
	return '\xFF';
}

FUNCMATH FUNCINLINE static ATTRINLINE UINT8 R_Char2Rotation(char cn)
{
	if (cn >= '0' && cn <= '9') return (cn - '0');
	if (cn >= 'A' && cn <= 'G') return (cn - 'A') + 10;
	if (cn == 'L') return ROT_L;
	if (cn == 'R') return ROT_R;
	return 255;
}

#endif //__R_THINGS__
