# Sonic Robo Blast 2 Banpyura

A custom net-compatible build of SRB2 that includes a plethora of fixes, improvements, and new features.

# Installation:

- You can either use the Github Actions build (go to the Actions tab on the top, then select the one that corresponds to your platform; x64 Windows builds require - different DLLs)
- Compile from the source code.
- [Download the build from nightly.link (x86)](https://nightly.link/GLideKS/SRB2-Banpyura/workflows/windows-x86-makefile/master?preview)

It works without any additional assets, just copy the build to your existing SRB2 folder and it will work fine.

> [!IMPORTANT]  
> This fork uses a different config file: `econfig.cfg`
> 
> If needed, copy/paste your `config.cfg` and rename it to `econfig.cfg`.

# Changes

## HUD/Menus
- Change menu's background color! ("`menubgcolor`", Video Options -> Heads-Up Display, "Menu Background Color")
- Lowercase menus inspired by SRB2Kart Saturn! ("`menucaps`", Video Options -> Heads-Up Display, "Uppercase Menus")
- The background of the Addons menu is now translucent.
- Addons menu uses thin font.
- Player names on the rankings uses a thin font.
- Toggle screen wipes to speed up gameplay! ("`wipes`", not effective in Marathon Run)
- "Quit Game" and "Abort" options are red colored.
- Perfstats blue text uses sky color instead.
- Tics per second counter! ("`showtps`") (Code from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic) and TSoURDt3rd)
- New camera options!
  - "`cam_clipping`": off = noclip, on = vanilla, exact = 3D platformer-style clip
  - "`cam_exact`": off = vanilla, on = precise camera movement
  - Combine `cam_clipping 2`, `cam_exact on`, and `cam_speed 1` for a Roblox-like camera!
- Server connection shows the types of addons loaded by the server! ("`showaddoninfo`", Code from [SRB2Kart Saturn](https://github.com/Indev450/SRB2Kart-Saturn/pull/224))

- Thin captions and thin FPS! (Load "tinyfontfix.pk3" for music note on thin captions)
- See your ping in frame delay instead of milliseconds! ("`pingmeasurement`")
- Compact FPS/TPS info! ("`compactinfo`")
- Master Server list background for better readability!
- Server Menu before joining! ("`showserverinfo`") (Code from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic))
- Joining netgame shows progress bar on "checking files" ([Lugent's PR](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2446) [Lugent's PR](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2556))
- Snake download game background fixed!
- Countdown beep isn't ear piercingly loud!
- Easily rejoin servers you've played before! ("`connect last`", Multiplayer -> Rejoin Previous Servers...)
- CEchos/CSays print message into console ("`showcsays`")
- Crosshairs can invert pixels behind, to improve their visibility ("`crosshair_invert`" and "`crosshair2_invert`")

## Visual
- Toggleable Screenshake effects! ("`earthquake`", ported from SRB2Classic by @archiNiko)
- Better "Fake Contrast"! (https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2680, @GLideKS)
- Render distance for OpenGL! ("`gr_renderdistance`", https://git.srb2.org/Hanicef/SRB2Classic/-/merge_requests/4, @GLideKS)
- Configurable minimum sector brightness! ("`r_secbright`", ported from [SRB2 Legacy](https://github.com/srb2-preservation/srb2-legacy))
- Added 1360x768 resolution (personal use)
- Light Dithering from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic)
- Removed RR's fake contrast since it makes brightness off on walls.

- Toggleable mobj pitch/roll functional (3D rotation for models on slopes, like DRRR). Option located in Video Options -> Level, "Pitch/Roll Rotation"
- Ring-Racers-styled screen quakes! ("`rr_quakes`" in console)
- Experimental translation support for models! ("`gl_modeltranslations`")
- View rollangle is interpolated!

## Gameplay / Netplay
- Skin change at any time
- Savegame errors return to the title screen
- Addfilelocal from SRB2K Saturn! (use "`addfilelocal`" command or press R-ALT in the addons menu)
- Minimum input delay from SRB2Kart Saturn/Ring Racers! ("`mindelay`")
- Improved startup times! (Code from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic))
- "`cam_centertoggle`" and "`cam2_centertoggle`" are no longer exclusive to Automatic!
- See private messages as host! (Code from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic))
- Lifted maxsend limits
- Allow gamepad controls on unfocused window ("`cv_gamepadifunfocused`")

## Modding and Debugging
- "`renderhitbox`" in multiplayer
- Lua HUD interpolation from SRB2K Saturn
- "`freezelevel`" debug command (Be careful using when clients are connected!)
- HUD camera struct updates position in first person! (credits [Jiskster](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2629) & [Hanicef](https://git.do.srb2.org/Hanicef/SRB2Classic/-/commit/681bd160f5be3925a97d798d00e67b32a8c1df71))
- `v.cachePatch` accepts a second parameter for rotation! (https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2662)
- "`getlogfile`" command (Prints the absolute path of the current log, useful when latest-log.txt is sym-linked to a different log)

## GIFs
- Draw gif information to hud ("`moviemodeinfo`", Video Options -> Diagnostic, "Show Movie Info")
- Adjustable gif size cap, toggable too! (`gif_maxsize`, "Max GIF Size (MB)")
    - ^ When gif is capped, gif_rolling allows for another gif to immediately start! (`gif_rolling`, "Keep recording when capped")
- Pause GIFs *WHILE* Recording! (Bound to F2 by default)

## Console & Misc. Commands
- `help` now lists commands and variables by origin. Parameters are as follows:
  | Param      | Desc      |
  | ------------- | ------------- |
  | `-v` | Only show variables and/or commands from vanilla SRB2 only.  |
  | `-c` | Only show variables and/or commands that are in SRB2 Banpyura, and not vanilla. |
  | `-a` | Only show variables and/or commands created by addons |

- Console variables can no longer be used as an argument for `help`, they now print their info instead of just their current and default value. "`cvarinfo`" lets you hide the flags and origin sections ("Show All" by default).
- "`cycle`" command (`cycle <cvar> [values]`): Cycles given values on the cvar if the current value is found in the list (also loops around). Fails if the current value is not found, unless `-b` is specified (starts at the first arg if so). Inaccessible by Lua.

# Lua Additions

## Global variables
- "`gks_custombuild`" (Read only) (boolean):  Global to detect if the client is using this build
- "`gks_complexlocaladdons`" (Read only) (boolean): Global to detect if the client has loaded local addons with lua in them
- "`gks_locallyloading`" (Read only) (boolean): Only set during script loading, detects whether the script is being loaded locally
- "`gks_lumpname`" (Read only) (string): Only set during script loading, returns the name of the Lua script being loaded

  Example:
  ```lua
  if (gks_locallyloading) then
    -- do client-side logic here
    return
  end
  -- do synched logic here
  ```
- "`demoplayback`" (Read only) (boolean) : True if viewing a demo.

## Functions
- `P_GetLocalAiming(player_t player)` : Returns the angle_t `aiming` of `player` if they are a local player. Returns 0 otherwise.
- `P_GetLocalAngle(player_t player)` : Returns the angle_t `angle` of `player` if they are a local player. Returns 0 otherwise.

- `R_CreateTranslation(string name, string translation, ...)` : Adds a custom translation, using the same parser as TRNSLATE.
- `R_RemoveTranslation(string name)` : Removes a custom translation. Can only remove translations made by Lua.
- `R_TranslationExists(string name)` : Returns true if a custom translation with a given name exists, false if not.

- `io.openlump(string filename, [string mode])` : Similar to `io.openlocal`, but reads a lump inside any addon loaded. Two new options are supported: `f` to scan addons forward from start to end, and `m` to only search in game-modifying addons (exclude local or non-important addons).

  Example:
  ```lua
  local file = io.openlump("lua/main.lua", "r")
  
  if file then
  	local dat = file:read("*a")
  	print("Length: "..dat:len())
  	file:close()
  else
  	print("Could not read lump")
  end
  ```

- `v.interpolate/v.interpLatch(boolean/int)` : See [SRB2K Saturn's documentation](https://github.com/Indev450/SRB2Kart-Saturn/blob/Saturn/LUASTUFF.md)
- `v.drawFixedFill` : Same as `v.drawFill`, but x, y, width, and height arguments are all in fixed point scale.
### For "uncappedgame" HUD hooks:
- `v.timeFraction()` : Returns the value of `rendertimefrac` [0, FRACUNIT]. "The fraction of a tic being drawn (for interpolation between two tics)"
- `v.deltaTics()` : Returns the value of `renderdeltatics`. "Evaluated delta tics for this frame (how many tics since the last frame)"
- `v.isNewTic()` : Returns whether the current frame is the start of a new gamelogic tic or not. (boolean)
- `v.usingInterpolation()` : Returns whether or not frame interpolation is enabled. (boolean)

Example script that uses manual interpolation:
```lua
local scroll = 0
addHook("HUD", function(v)
	local width = (v.width() / v.dupx()) * FU

	if v.isNewTic() then
		scroll = ($ + FU * 4) % width
	end

	local x = (scroll + v.timeFraction() * 4) % width
	v.drawScaled(x, FU * 100, FU, v.cachePatch("MISSING"), V_SNAPTOLEFT)

	v.drawScaled(scroll, FU * 140, FU, v.cachePatch("MISSING"), V_SNAPTOLEFT)
end, "uncappedgame")
```

- `M_Random*` : Same as `v.Random*` functions, except also client-sided and not limited to HUD hooks.

## Hooks
- `"uncappedgame"` : A new HUD hook that runs during gameplay, except not bound to the 35 FPS limit. Introduces new functions into the draw for manual interpolation.

## mobj_t
- `mobj.pitch/roll` : Rotates mobjs and models in 3D space.
- `mobj.resetinterp` : Resets ALL interpolation values. (`P_SetOrigin` only resets positional interpolation values)


Example that tilts your character in their 3D direction:
```lua
addHook("PlayerThink", function(player)
    local mo = player.mo
    if not (mo and mo.valid) then return end

    local angle = R_PointToAngle2(0, 0, mo.momx, mo.momy)
    local mang = R_PointToAngle2(0, 0, FixedHypot(mo.momx, mo.momy), mo.momz)
    mang = InvAngle($)

    mo.roll = FixedMul(mang, sin(angle))
    mo.pitch = FixedMul(mang, cos(angle))
end)
```

## player_t
- `player.ipaddress` (string) (read only): For use in moderation addons, this only returns a string for the server of the players IP address. Clients _cannot_ see other clients' IP addresses. The only way for other clients to know is if the server sends a command with them or something :p
- `player.muted` (boolean) (read + write): Returns whether or not the player is muted. (though changes may not be reflected in servers not running edit/Banpyura)


## renderflags_t
- `RF_ALWAYSONTOP` : The sprite always draws on top of level geometry and other sprites. Not supported for models, and culled sprites wont be rendered. Note that in OpenGL, anything transparent will render on any `RF_ALWAYSONTOP` sprites
- `RF_HIDEINSKYBOX` : The sprite will be hidden in the skybox.
- `RF_NOMODEL` : The sprite will be forced to not use a model in OpenGL.


## eflags_t
- `MFE_NOPITCHROLLEASING` : When `pitchroll-easing` is toggled, adding this eflag will not ease the pitch/roll axis this tic. Removed at the end of MobjThinker.

# Compiling

See [SRB2 Wiki/Source code compiling](http://wiki.srb2.org/wiki/Source_code_compiling)


# Sonic Robo Blast 2
[![latest release](https://badgen.net/github/release/STJr/SRB2/stable)](https://github.com/STJr/SRB2/releases/latest)

[![Build status](https://ci.appveyor.com/api/projects/status/399d4hcw9yy7hg2y?svg=true)](https://ci.appveyor.com/project/STJr/srb2)
[![Build status](https://travis-ci.org/STJr/SRB2.svg?branch=master)](https://travis-ci.org/STJr/SRB2)
[![CircleCI](https://circleci.com/gh/STJr/SRB2/tree/master.svg?style=svg)](https://circleci.com/gh/STJr/SRB2/tree/master)

[Sonic Robo Blast 2](https://srb2.org/) is a 3D Sonic the Hedgehog fangame based on a modified version of [Doom Legacy](http://doomlegacy.sourceforge.net/).

## Dependencies (Linux/macOS only)
- SDL2
- SDL2-Mixer
- libupnp
- libgme
- libopenmpt

## Disclaimer
Sonic Team Junior is in no way affiliated with SEGA or Sonic Team. We do not claim ownership of any of SEGA's intellectual property used in SRB2.
