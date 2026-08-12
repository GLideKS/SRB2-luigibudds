// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2011-2016 by Matthew "Kaito Sinclaire" Walsh.
// Copyright (C) 1999-2025 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  m_menu.c
/// \brief XMOD's extremely revamped menu system.

#ifdef __GNUC__
#include <unistd.h>
#endif

#include "m_menu.h"

#include "r_local.h"
#include "g_game.h"
#include "g_input.h"

#include "v_video.h"
#include "i_video.h"
#include "z_zone.h"
#include "lua_hook.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#include "i_joy.h" // for joystick menu controls

#if defined (__GNUC__) && (__GNUC__ >= 4)
#define FIXUPO0
#endif

I_mutex m_menu_mutex;

M_waiting_mode_t m_waiting_mode = M_NOT_WAITING;

// Stuff for customizing the player select screen Tails 09-22-2003
description_t *description = NULL;
INT32 numdescriptions = 0;

INT16 char_on = -1, startchar = 0;

boolean menuactive = false;
boolean fromlevelselect = false;

static saveinfo_t* savegameinfo = NULL; // Extra info about the save games.

// Prototyping is fun, innit?
// romoney5: NO IT IS NOT!!

// ==========================================================================
// CONSOLE VARIABLES AND THEIR POSSIBLE VALUES GO HERE.
// ==========================================================================

consvar_t cv_showfocuslost = CVAR_INIT ("showfocuslost", "Yes", CV_SAVE, CV_YesNo, NULL);

static CV_PossibleValue_t map_cons_t[] = {
	{1,"MIN"},
	{NUMMAPS, "MAX"},
	{0,NULL}
};
consvar_t cv_nextmap = CVAR_INIT ("nextmap", "1", CV_HIDEN|CV_CALL, map_cons_t, NULL);

static CV_PossibleValue_t skins_cons_t[MAXSKINS+1] = {{1, DEFAULTSKIN}};
consvar_t cv_chooseskin = CVAR_INIT ("chooseskin", DEFAULTSKIN, CV_HIDEN|CV_CALL, skins_cons_t, NULL);

// This gametype list is integral for many different reasons.
// When you add gametypes here, don't forget to update them in deh_tables.c and doomstat.h!
CV_PossibleValue_t gametype_cons_t[NUMGAMETYPES+1];

consvar_t cv_newgametype = CVAR_INIT ("newgametype", "Co-op", CV_HIDEN|CV_CALL, gametype_cons_t, NULL);

static CV_PossibleValue_t serversort_cons_t[] = {
	{0,"Ping"},
	{1,"Modified State"},
	{2,"Most Players"},
	{3,"Least Players"},
	{4,"Max Player Slots"},
	{5,"Gametype"},
	{0,NULL}
};
consvar_t cv_serversort = CVAR_INIT ("serversort", "Ping", CV_HIDEN | CV_CALL, serversort_cons_t, M_SortServerList);

// first time memory
consvar_t cv_tutorialprompt = CVAR_INIT ("tutorialprompt", "On", CV_SAVE, CV_OnOff, NULL);

// autorecord demos for time attack
static consvar_t cv_autorecord = CVAR_INIT ("autorecord", "Yes", 0, CV_YesNo, NULL);

CV_PossibleValue_t ghost_cons_t[] = {{0, "Hide"}, {1, "Show"}, {2, "Show All"}, {0, NULL}};
CV_PossibleValue_t ghost2_cons_t[] = {{0, "Hide"}, {1, "Show"}, {0, NULL}};

consvar_t cv_ghost_bestscore = CVAR_INIT ("ghost_bestscore", "Show", CV_SAVE, ghost_cons_t, NULL);
consvar_t cv_ghost_besttime  = CVAR_INIT ("ghost_besttime",  "Show", CV_SAVE, ghost_cons_t, NULL);
consvar_t cv_ghost_bestrings = CVAR_INIT ("ghost_bestrings", "Show", CV_SAVE, ghost_cons_t, NULL);
consvar_t cv_ghost_last      = CVAR_INIT ("ghost_last",      "Show", CV_SAVE, ghost_cons_t, NULL);
consvar_t cv_ghost_guest     = CVAR_INIT ("ghost_guest",     "Show", CV_SAVE, ghost2_cons_t, NULL);

void Addons_option_Onchange(void) {}
void Nextmap_OnChange(void) {}
void Moviemode_mode_Onchange(void) {}
void Screenshot_option_Onchange(void) {}
void Moviemode_option_Onchange(void) {}

// ==========================================================================
// ORGANIZATION START.
// ==========================================================================

UINT32 roomIds[NUM_LIST_ROOMS];

menuitem_t MP_RoomMenu[] =
{
	{IT_STRING | IT_CALL, NULL, "<Unlisted Mode>", NULL,   9},
	{IT_DISABLED,         NULL, "",               NULL,  18},
	{IT_DISABLED,         NULL, "",               NULL,  27},
	{IT_DISABLED,         NULL, "",               NULL,  36},
	{IT_DISABLED,         NULL, "",               NULL,  45},
	{IT_DISABLED,         NULL, "",               NULL,  54},
	{IT_DISABLED,         NULL, "",               NULL,  63},
	{IT_DISABLED,         NULL, "",               NULL,  72},
	{IT_DISABLED,         NULL, "",               NULL,  81},
	{IT_DISABLED,         NULL, "",               NULL,  90},
	{IT_DISABLED,         NULL, "",               NULL,  99},
	{IT_DISABLED,         NULL, "",               NULL, 108},
	{IT_DISABLED,         NULL, "",               NULL, 117},
	{IT_DISABLED,         NULL, "",               NULL, 126},
	{IT_DISABLED,         NULL, "",               NULL, 135},
	{IT_DISABLED,         NULL, "",               NULL, 144},
	{IT_DISABLED,         NULL, "",               NULL, 153},
	{IT_DISABLED,         NULL, "",               NULL, 162},
};

// ==========================================================================
// ALL MENU DEFINITIONS GO HERE
// ==========================================================================

// Main Menu and related
// romoney5: I love terrible hacks spread all across the codebase!
menu_t MainDef = { MN_MAIN, NULL, sizeof(NULL) / sizeof(menuitem_t), NULL, NULL, NULL, BASEVIDWIDTH / 2, 72, 0, NULL };
menu_t MessageDef = &MainDef;
menu_t OP_JoystickSetDef = &MainDef;
menu_t SP_LoadDef = &MainDef;

gtdesc_t gametypedesc[NUMGAMETYPES] =
{
	{{ 54,  54}, "Play through the single-player campaign with your friends, teaming up to beat Dr Eggman's nefarious challenges!"},
	{{103, 103}, "Speed your way through the main acts, competing in several different categories to see who's the best."},
	{{190, 190}, "There's not much to it - zoom through the level faster than everyone else."},
	{{ 66,  66}, "Sling rings at your foes in a free-for-all battle. Use the special weapon rings to your advantage!"},
	{{153,  37}, "Sling rings at your foes in a color-coded battle. Use the special weapon rings to your advantage!"},
	{{123, 123}, "Whoever's IT has to hunt down everyone else. If you get caught, you have to turn on your former friends!"},
	{{150, 150}, "Try and find a good hiding place in these maps - we dare you."},
	{{ 37, 153}, "Steal the flag from the enemy's base and bring it back to your own, but watch out - they could just as easily steal yours!"},
};

// ==========================================================================
// END ORGANIZATION STUFF.
// ==========================================================================

// current menudef
menu_t *currentMenu = &MainDef;

// =========================================================================
// MENU PRESENTATION PARAMETER HANDLING (BACKGROUNDS)
// =========================================================================

// menu IDs are equal to current/prevMenu in most cases, except MN_SPECIAL when we don't want to operate on Message, Pause, etc.
UINT32 prevMenuId = 0;
UINT32 activeMenuId = 0;

menupres_t menupres[NUMMENUTYPES];

void M_InitMenuPresTables(void) { }

// ====================================
// EFFECTS
// ====================================

void M_SetMenuCurBackground(const char *defaultname) { }

void M_SetMenuCurFadeValue(UINT8 defaultvalue) { }

void M_SetMenuCurTitlePics(void) { }

// =========================================================================
// BASIC MENU HANDLING
// =========================================================================

//
// M_Responder
//
boolean M_Responder(event_t *ev)
{
	INT32 ch = -1;
	static tic_t joywait = 0, mousewait = 0;
	static INT32 pjoyx = 0, pjoyy = 0;
	static INT32 pmousex = 0, pmousey = 0;
	static INT32 lastx = 0, lasty = 0;

	if (dedicated || (demoplayback && titledemo)
	|| gamestate == GS_INTRO || gamestate == GS_ENDING || gamestate == GS_CUTSCENE
	|| gamestate == GS_CREDITS || gamestate == GS_EVALUATION || gamestate == GS_GAMEEND)
		return false;

	if (gamestate == GS_TITLESCREEN && finalecount < (cv_tutorialprompt.value ? TICRATE : 0))
		return false;

	if (CON_Ready() && gamestate != GS_WAITINGPLAYERS)
		return false;

	if (menuactive)
	{
		if (ev->type == ev_keydown || ev->type == ev_text)
		{
			ch = ev->key;
			if (ev->type == ev_keydown)
			{
				// added 5-2-98 remap virtual keys (mouse & joystick buttons)
				switch (ch)
				{
					case KEY_MOUSE1:
					case KEY_JOY1:
						ch = KEY_ENTER;
						break;
					case KEY_JOY1 + 3:
						ch = 'n';
						break;
					case KEY_MOUSE1 + 1:
					case KEY_JOY1 + 1:
						ch = KEY_ESCAPE;
						break;
					case KEY_JOY1 + 2:
						ch = KEY_BACKSPACE;
						break;
					case KEY_HAT1:
						ch = KEY_UPARROW;
						break;
					case KEY_HAT1 + 1:
						ch = KEY_DOWNARROW;
						break;
					case KEY_HAT1 + 2:
						ch = KEY_LEFTARROW;
						break;
					case KEY_HAT1 + 3:
						ch = KEY_RIGHTARROW;
						break;
				}
			}
		}
		else if (ev->type == ev_joystick  && ev->key == 0 && joywait < I_GetTime())
		{
			const INT32 jdeadzone = (JOYAXISRANGE * cv_digitaldeadzone.value) / FRACUNIT;
			if (ev->y != INT32_MAX)
			{
				if (Joystick.bGamepadStyle || abs(ev->y) > jdeadzone)
				{
					if (ev->y < 0 && pjoyy >= 0)
					{
						ch = KEY_UPARROW;
						joywait = I_GetTime() + NEWTICRATE/7;
					}
					else if (ev->y > 0 && pjoyy <= 0)
					{
						ch = KEY_DOWNARROW;
						joywait = I_GetTime() + NEWTICRATE/7;
					}
					pjoyy = ev->y;
				}
				else
					pjoyy = 0;
			}

			if (ev->x != INT32_MAX)
			{
				if (Joystick.bGamepadStyle || abs(ev->x) > jdeadzone)
				{
					if (ev->x < 0 && pjoyx >= 0)
					{
						ch = KEY_LEFTARROW;
						joywait = I_GetTime() + NEWTICRATE/17;
					}
					else if (ev->x > 0 && pjoyx <= 0)
					{
						ch = KEY_RIGHTARROW;
						joywait = I_GetTime() + NEWTICRATE/17;
					}
					pjoyx = ev->x;
				}
				else
					pjoyx = 0;
			}
		}
		else if (ev->type == ev_mouse && mousewait < I_GetTime())
		{
			pmousey -= ev->y;
			if (pmousey < lasty-30)
			{
				ch = KEY_DOWNARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousey = lasty -= 30;
			}
			else if (pmousey > lasty + 30)
			{
				ch = KEY_UPARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousey = lasty += 30;
			}

			pmousex += ev->x;
			if (pmousex < lastx - 30)
			{
				ch = KEY_LEFTARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousex = lastx -= 30;
			}
			else if (pmousex > lastx+30)
			{
				ch = KEY_RIGHTARROW;
				mousewait = I_GetTime() + NEWTICRATE/7;
				pmousex = lastx += 30;
			}
		}
	}
	else if (ev->type == ev_keydown) // Preserve event for other responders
		ch = ev->key;

	if (ch == -1)
		return false;
	else if (ev->type != ev_text && (ch == gamecontrol[GC_SYSTEMMENU][0] || ch == gamecontrol[GC_SYSTEMMENU][1])) // allow remappable ESC key
		ch = KEY_ESCAPE;

	// F-Keys
	if (!menuactive)
	{
		switch (ch)
		{
			case KEY_F1: // Help key
				return true;

			case KEY_F2: // Empty
				return true;

			case KEY_F3: // Toggle HUD
				CV_SetValue(&cv_showhud, !cv_showhud.value);
				return true;

			case KEY_F5: // Video Mode
				if (modeattacking)
					return true;
				return true;

			case KEY_F6: // Empty
				return true;

			case KEY_F7: // Options
				if (modeattacking)
					return true;
				return true;

			// Screenshots on F8 now handled elsewhere
			// Same with Moviemode on F9

			case KEY_F10: // Renderer toggle, also processed inside menus
				CV_AddValue(&cv_renderer, 1);
				return true;

			case KEY_F11: // Fullscreen toggle, also processed inside menus
				CV_SetValue(&cv_fullscreen, !cv_fullscreen.value);
				return true;

			// Spymode on F12 handled in game logic

			case KEY_ESCAPE: // Pop up menu
				if (chat_on)
					HU_clearChatChars();
				else
					M_StartControlPanel();
				return true;
		}
		return false;
	}

	if (gks_luamenu)
		return false;

	return true;
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
	LUA_HUDHOOK(menu, NULL);

	// focus lost notification goes on top of everything, even the former everything
	if (window_notinfocus && cv_showfocuslost.value)
	{
		M_DrawTextBox((BASEVIDWIDTH/2) - (60), (BASEVIDHEIGHT/2) - (16), 13, 2);
		if (gamestate == GS_LEVEL && (P_AutoPause() || paused))
			V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2) - (4), MENUCOLOR, "Game Paused");
		else
			V_DrawCenteredString(BASEVIDWIDTH/2, (BASEVIDHEIGHT/2) - (4), MENUCOLOR, "Focus Lost");
	}
}

//
// M_StartControlPanel
//
void M_StartControlPanel(void) // romoney5 TODO: manual
{
	if (gks_luamenu)
		return;
}

void M_EndModeAttackRun(void)
{
	G_ClearModeAttackRetryFlag();
}

//
// M_ClearMenus
//
void M_ClearMenus(boolean callexitmenufunc)
{
	if (!menuactive)
		return;
}

// Guess I'll put this here, idk
boolean M_MouseNeeded(void)
{
	return false;
}

//
// M_Ticker
//
void M_Ticker(void) { }

//
// M_Init
//
void M_Init(void)
{
	//COM_AddCommand("manual", Command_Manual_f, COM_LUA);

	CV_RegisterVar(&cv_nextmap);
	CV_RegisterVar(&cv_newgametype);
	CV_RegisterVar(&cv_chooseskin);
	CV_RegisterVar(&cv_autorecord);

	if (dedicated)
		return;

	// Menu hacks  are GONE!!

	CV_RegisterVar(&cv_serversort);
}

static void M_InitCharacterDescription(INT32 i)
{
	// Setup description table
	description_t *desc = &description[i];
	desc->picname[0] = '\0';
	desc->nametag[0] = '\0';
	desc->skinname[0] = '\0';
	desc->displayname[0] = '\0';
	desc->prev = desc->next = 0;
	desc->charpic = NULL;
	desc->namepic = NULL;
	desc->oppositecolor = SKINCOLOR_NONE;
	desc->tagtextcolor = SKINCOLOR_NONE;
	desc->tagoutlinecolor = SKINCOLOR_NONE;
	strcpy(desc->notes, "???");
}

void M_InitCharacterTables(INT32 num)
{
	INT32 i = numdescriptions;

	description = Z_Realloc(description, sizeof(description_t) * num, PU_STATIC, NULL);
	numdescriptions = num;

	for (; i < numdescriptions; i++)
		M_InitCharacterDescription(i);
}

// ==========================================================================
// SPECIAL MENU OPTION DRAW ROUTINES GO HERE
// ==========================================================================

//
//  Draw a textbox, like Quake does, because sometimes it's difficult
//  to read the text with all the stuff in the background...
//
void M_DrawTextBox(INT32 x, INT32 y, INT32 width, INT32 boxlines)
{
	// Solid color textbox.
	V_DrawFill(x+5, y+5, width*8+6, boxlines*8+6, cv_menubgcolor.value);
}

//
// M_CanShowLevelInList
//
// Determines whether to show a given map in level-select lists where you don't want to see locked levels.
// Set gt = -1 to ignore gametype.
//
boolean M_CanShowLevelInList(INT32 mapnum, INT32 gt)
{
	return true;//(M_CanShowLevelOnPlatter(mapnum, gt) && M_LevelAvailableOnPlatter(mapnum));
}

// ==================================================
// MESSAGE BOX (aka: a hacked, cobbled together menu)
// ==================================================

void M_StartMessage(const char *string, void *routine, menumessagetype_t itemtype)
{
	(void)string;
	(void)routine;
}

// ==================
// NEW GAME FUNCTIONS
// ==================

INT32 ultimate_selectable = false;

void M_TutorialSaveControlResponse(INT32 ch)
{
	if (ch == 'y' || ch == KEY_ENTER)
	{
		G_CopyControls(gamecontrol, gamecontroldefault[tutorialgcs], gcl_tutorial_full, num_gcl_tutorial_full);
		CV_Set(&cv_usemouse, cv_usemouse.defaultvalue);
		CV_Set(&cv_alwaysfreelook, cv_alwaysfreelook.defaultvalue);
		CV_Set(&cv_mousemove, cv_mousemove.defaultvalue);
		CV_Set(&cv_analog[0], cv_analog[0].defaultvalue);
		S_StartSound(NULL, sfx_itemup);
	}
	else
		S_StartSound(NULL, sfx_menu1);
}

#define VERSIONSIZE 16
#define MISSING { savegameinfo[slot].continuescore = -62; savegameinfo[slot].lives = -666; Z_Free(savebuffer); return; }
#define BADSAVE { savegameinfo[slot].lives = -666; Z_Free(savebuffer); return; }
#define CHECKPOS if (sav_p >= end_p) BADSAVE
// Reads the save file to list lives, level, player, etc.
// Tails 05-29-2003
static void M_ReadSavegameInfo(UINT32 slot)
{
	size_t length;
	char savename[255];
	UINT8 *savebuffer;
	UINT8 *end_p; // buffer end point, don't read past here
	UINT8 *sav_p;
	INT32 fake; // Dummy variable
	char temp[sizeof(timeattackfolder)];
	char vcheck[VERSIONSIZE];
#ifdef NEWSKINSAVES
	INT16 backwardsCompat = 0;
#endif

	sprintf(savename, savegamename, slot);

	slot--;

	length = FIL_ReadFile(savename, &savebuffer);
	if (length == 0)
	{
		savegameinfo[slot].lives = -42;
		return;
	}

	end_p = savebuffer + length;

	// skip the description field
	sav_p = savebuffer;

	// Version check
	memset(vcheck, 0, sizeof (vcheck));
	sprintf(vcheck, "version %d", VERSION);
	if (strcmp((const char *)sav_p, (const char *)vcheck)) BADSAVE
	sav_p += VERSIONSIZE;

	// dearchive all the modifications
	// P_UnArchiveMisc()

	CHECKPOS
	fake = READINT16(sav_p);

	if (((fake-1) & 8191) >= NUMMAPS) BADSAVE

	if(!mapheaderinfo[(fake-1) & 8191])
		savegameinfo[slot].levelname[0] = '\0';
	else if (V_ThinStringWidth(mapheaderinfo[(fake-1) & 8191]->lvlttl, 0) <= 78)
		strlcpy(savegameinfo[slot].levelname, mapheaderinfo[(fake-1) & 8191]->lvlttl, 22);
	else
	{
		strlcpy(savegameinfo[slot].levelname, mapheaderinfo[(fake-1) & 8191]->lvlttl, 15);
		strcat(savegameinfo[slot].levelname, "...");
	}

	savegameinfo[slot].gamemap = fake;

	CHECKPOS
	savegameinfo[slot].numemeralds = READUINT16(sav_p)-357; // emeralds

	CHECKPOS
	READSTRINGN(sav_p, temp, sizeof(temp)); // mod it belongs to

	if (strcmp(temp, timeattackfolder)) BADSAVE

	// P_UnArchivePlayer()
#ifdef NEWSKINSAVES
	CHECKPOS
	backwardsCompat = READUINT16(sav_p);

	if (backwardsCompat != NEWSKINSAVES)
	{
		// Backwards compat
		savegameinfo[slot].skinnum = backwardsCompat & ((1<<5) - 1);

		if (savegameinfo[slot].skinnum >= numskins
		|| !R_SkinUsable(-1, savegameinfo[slot].skinnum))
			BADSAVE

		savegameinfo[slot].botskin = backwardsCompat >> 5;
		if (savegameinfo[slot].botskin-1 >= numskins
		|| !R_SkinUsable(-1, savegameinfo[slot].botskin-1))
			BADSAVE
	}
	else
#endif
	{
		char ourSkinName[SKINNAMESIZE+1];
		char botSkinName[SKINNAMESIZE+1];

		CHECKPOS
		READSTRINGN(sav_p, ourSkinName, SKINNAMESIZE);
		savegameinfo[slot].skinnum = R_SkinAvailable(ourSkinName);
		STRBUFCPY(savegameinfo[slot].skinname, ourSkinName);

		if (savegameinfo[slot].skinnum >= numskins
		|| !R_SkinUsable(-1, savegameinfo[slot].skinnum))
			MISSING

		CHECKPOS
		READSTRINGN(sav_p, botSkinName, SKINNAMESIZE);
		savegameinfo[slot].botskin = (R_SkinAvailable(botSkinName) + 1);

		if (savegameinfo[slot].botskin-1 >= numskins
		|| !R_SkinUsable(-1, savegameinfo[slot].botskin-1))
			MISSING
	}

	CHECKPOS
	savegameinfo[slot].numgameovers = READUINT8(sav_p); // numgameovers
	CHECKPOS
	savegameinfo[slot].lives = READSINT8(sav_p); // lives
	CHECKPOS
	savegameinfo[slot].continuescore = READINT32(sav_p); // score
	CHECKPOS
	fake = READINT32(sav_p); // continues
	if (useContinues)
		savegameinfo[slot].continuescore = fake;

	// File end marker check
	CHECKPOS
	switch (READUINT8(sav_p))
	{
		case 0xb7:
			{
				UINT8 i, banksinuse;
				CHECKPOS
				banksinuse = READUINT8(sav_p);
				CHECKPOS
				if (banksinuse > NUM_LUABANKS)
					BADSAVE
				for (i = 0; i < banksinuse; i++)
				{
					(void)READINT32(sav_p);
					CHECKPOS
				}
				if (READUINT8(sav_p) != 0x1d)
					BADSAVE
			}
		case 0x1d:
			break;
		default:
			BADSAVE
	}

	// done
	Z_Free(savebuffer);
}
#undef CHECKPOS
#undef BADSAVE
#undef MISSING

//
// Used by cheats to force the save menu to a specific spot.
//
void M_ForceSaveSlotSelected(INT32 sslot) { }

void M_ModeAttackRetry(INT32 choice)
{
	(void)choice;
	// todo -- maybe seperate this out and G_SetRetryFlag() here instead? is just calling this from the menu 100% safe?
	G_CheckDemoStatus(); // Cancel recording
}

void M_SortServerList(void) { }

void M_AddMenuColor(UINT16 color) {
	menucolor_t *c;

	if (color >= numskincolors) {
		CONS_Printf("M_AddMenuColor: color %d does not exist.",color);
		return;
	}

	c = (menucolor_t *)malloc(sizeof(menucolor_t));
	c->color = color;
	if (menucolorhead == NULL) {
		c->next = c;
		c->prev = c;
		menucolorhead = c;
		menucolortail = c;
	} else {
		c->next = menucolorhead;
		c->prev = menucolortail;
		menucolortail->next = c;
		menucolorhead->prev = c;
		menucolortail = c;
	}
}

void M_MoveColorBefore(UINT16 color, UINT16 targ) {
	menucolor_t *look, *c = NULL, *t = NULL;

	if (color == targ)
		return;
	if (color >= numskincolors) {
		CONS_Printf("M_MoveColorBefore: color %d does not exist.",color);
		return;
	}
	if (targ >= numskincolors) {
		CONS_Printf("M_MoveColorBefore: target color %d does not exist.",targ);
		return;
	}

	for (look=menucolorhead;;look=look->next) {
		if (look->color == color)
			c = look;
		else if (look->color == targ)
			t = look;
		if (c != NULL && t != NULL)
			break;
		if (look==menucolortail)
			return;
	}

	if (c == t->prev)
		return;

	if (t==menucolorhead)
		menucolorhead = c;
	if (c==menucolortail)
		menucolortail = c->prev;

	c->prev->next = c->next;
	c->next->prev = c->prev;

	c->prev = t->prev;
	c->next = t;
	t->prev->next = c;
	t->prev = c;
}

void M_MoveColorAfter(UINT16 color, UINT16 targ) {
	menucolor_t *look, *c = NULL, *t = NULL;

	if (color == targ)
		return;
	if (color >= numskincolors) {
		CONS_Printf("M_MoveColorAfter: color %d does not exist.\n",color);
		return;
	}
	if (targ >= numskincolors) {
		CONS_Printf("M_MoveColorAfter: target color %d does not exist.\n",targ);
		return;
	}

	for (look=menucolorhead;;look=look->next) {
		if (look->color == color)
			c = look;
		else if (look->color == targ)
			t = look;
		if (c != NULL && t != NULL)
			break;
		if (look==menucolortail)
			return;
	}

	if (t == c->prev)
		return;

	if (t==menucolortail)
		menucolortail = c;
	else if (c==menucolortail)
		menucolortail = c->prev;

	c->prev->next = c->next;
	c->next->prev = c->prev;

	c->next = t->next;
	c->prev = t;
	t->next->prev = c;
	t->next = c;
}

UINT16 M_GetColorBefore(UINT16 color) {
	menucolor_t *look;

	if (color >= numskincolors) {
		CONS_Printf("M_GetColorBefore: color %d does not exist.\n",color);
		return 0;
	}

	for (look=menucolorhead;;look=look->next) {
		if (look->color == color)
			return look->prev->color;
		if (look==menucolortail)
			return 0;
	}
}

UINT16 M_GetColorAfter(UINT16 color) {
	menucolor_t *look;

	if (color >= numskincolors) {
		CONS_Printf("M_GetColorAfter: color %d does not exist.\n",color);
		return 0;
	}

	for (look=menucolorhead;;look=look->next) {
		if (look->color == color)
			return look->next->color;
		if (look==menucolortail)
			return 0;
	}
}

UINT16 M_GetColorIndex(UINT16 color) {
	menucolor_t *look;
	UINT16 i = 0;

	if (color >= numskincolors) {
		CONS_Printf("M_GetColorIndex: color %d does not exist.\n",color);
		return 0;
	}

	for (look=menucolorhead;;look=look->next) {
		if (look->color == color)
			return i;
		if (look==menucolortail)
			return 0;
		i++;
	}
}

menucolor_t* M_GetColorFromIndex(UINT16 index) {
	menucolor_t *look = menucolorhead;
	UINT16 i = 0;

	if (index >= numskincolors) {
		CONS_Printf("M_GetColorIndex: index %d does not exist.\n",index);
		return 0;
	}

	for (i = 0; i <= index; i++) {
		if (look==menucolortail)
			return menucolorhead;
		look=look->next;
	}

	return look;
}

void M_InitPlayerSetupColors(void) {
	UINT8 i;
	numskincolors = SKINCOLOR_FIRSTFREESLOT;
	menucolorhead = menucolortail = NULL;
	for (i=0; i<numskincolors; i++)
		M_AddMenuColor(i);
}

void M_FreePlayerSetupColors(void) {
	menucolor_t *look = menucolorhead, *tmp;

	if (menucolorhead==NULL)
		return;

	while (true) {
		if (look != menucolortail) {
			tmp = look;
			look = look->next;
			free(tmp);
		} else {
			free(look);
			return;
		}
	}

	menucolorhead = menucolortail = NULL;
}

void M_SetupJoystickMenu(INT32 choice)
{
	(void)choice;
}
