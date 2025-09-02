If you want to use code from my repo, please contact me beforehand. (because most of it isnt mine Lul!!!)

## Compiling

See [SRB2 Wiki/Source code compiling](http://wiki.srb2.org/wiki/Source_code_compiling)

# Installation:

You can compile the source code normally (see "Compiling") and put the binary in your SRB2 directory. No additional assets required, it works right out the box!

# Changes

- Toggable mobj pitch/roll functional (3d rotation for models on slopes, like DRRR). Option located in Video Settings -> Level -> "Pitch/Roll Rotation"
- CEchos/CSays print message into console ("showcsays")
- Skin change at any time
- renderhitbox in multiplayer
- Lua HUD interpolation from SRB2K Saturn
- Joining netgame shows progress bar on "checking files" ([Lugent's PR](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2446) [Lugent's PR](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2556))
- Easily rejoin servers you've played before! (Multiplayer -> Rejoin Previous Servers...)
- Adjustable gif size cap, toggable too! (gif_maxsize, "Max GIF Size (MB)")
    - ^ When gif is capped, gif_rolling allows for another gif to immediately start! (gif_rolling, "Keep recording when capped")
- Draw gif information to hud ("moviemodeinfo")
- Crosshairs can invert pixels behind, to improve their visibility ("crosshair_invert")
- Addfilelocal from SRB2K Saturn! (use "addfilelocal" command or press SHIFT in the addons menu)
- Thin captions and thin FPS! (Load "tinyfontfix.pk3" for music note on thin captions)
- Ring-Racers-styled screen quakes! ("rr_quakes" in console)
- Countdown beep isn't ear piercingly loud!
- "freezelevel" debug command (Cannot use when clients are connected)
- Snake download game background fixed!
- HUD camera struct updates position in first person! (credits [Jiskster](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2629) & [Hanicef](https://git.do.srb2.org/Hanicef/SRB2Classic/-/commit/681bd160f5be3925a97d798d00e67b32a8c1df71))
- Master Server list background for better readability!
- Server Menu before joining! (Code from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic))
- See your ping in frame delay instead of milliseconds! ("pingmeasurement")
- Tics per second counter! ("showtps") (Code from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic))
- Compact FPS/TPS info! ("compactinfo")
- Improved startup times! (Code from [SRB2Classic](https://codeberg.org/srb2classic/srb2classic))
- Minimum input delay & "Gentleman's Delay" option from SRB2Kart Saturn/Ring Racers! (mindelay and gentlemensdelay respectively)
- Toggleable Screenshake effects! ("earthquake", @archiNiko)
- Better "Fake Contrast"! (https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2680)
- Pause GIFs *WHILE* Recording! (Bound to F2 by default)
- v.cachePatch accepts a second parameter for rotation! (https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2662)
# Lua Additions

## Global variables
- "takis_custombuild" : (Read only) (boolean) Global to detect if the client is using this build
- "takis_complexlocaladdons" : (Read only) (boolean) Global to detect if the client has loaded local addons with lua in them
- "takis_locallyloading" : (Read only) (boolean) Only set during script loading, detects whether the script is being loaded locally
  Example:
  ```lua
  if (takis_locallyloading) then
    --do local stuff here
    return
  end
  --normal, gameplay editing code
  ```

## Functions
- P_GetLocalAiming(player_t player) : Returns the angle_t `aiming` of `player` if they are a local player. Returns 0 otherwise.
- P_GetLocalAngle(player_t player) : Returns the angle_t `angle` of `player` if they are a local player. Returns 0 otherwise.

- v.interpolate/v.interpLatch(boolean/int) : See [SRB2K Saturn's documentation](https://github.com/Indev450/SRB2Kart-Saturn/blob/Saturn/LUASTUFF.md)
- v.drawFixedFill : Same as v.drawFill, but x, y, width, and height arguments are all in fixed point scale.
# For "uncappedgame" HUD hooks:
- v.timeFraction() : Returns the value of `rendertimefrac` [0 - FRACUNIT]. "The fraction of a tic being drawn (for interpolation between two tics)"
- v.deltaTics() : Returns the value of `renderdeltatics`. "Evaluated delta tics for this frame (how many tics since the last frame)"
- v.isNewTic() : Returns whether the current render is the start of a new gamelogic tic or not. (boolean)
- v.usingInterpolation() : Returns whether or not frame interpolation is enabled. (boolean)

Example script that uses manual interpolation:
```lua
local scroll = 0
addHook("HUD",function(v,p,cam)
	local width = (v.width() / v.dupx())*FU
	if v.isNewTic()
		scroll = ($ + 4*FU) % width
	end
	local x = (scroll + (4*v.timeFraction())) % width
	v.drawScaled(x, 100*FU, FU, v.cachePatch("MISSING"),V_SNAPTOLEFT)

	v.drawScaled(scroll, 140*FU, FU, v.cachePatch("MISSING"),V_SNAPTOLEFT)
end,"uncappedgame")
```

- M_Random* : Same as v.Random* functions, except also client-sided and not limited to HUD hooks.

## Hooks
- "uncappedgame" : A new hud hook that runs during gameplay, except not bound to the 35 fps limit. Introduces new functions into the draw for manual interpolation. (WIP)

## mobj_t
- mobj.pitch/roll : Now rotates mobjs in 3D space, including models

Example that tilts your character in their 3D direction:
```lua
addHook("PlayerThink",function(p)
    local me = p.mo
    if not (me and me.valid) then return end

    local angle = R_PointToAngle2(0,0, me.momx,me.momy)
    local mang = R_PointToAngle2(0,0, FixedHypot(me.momx, me.momy), me.momz)
    mang = InvAngle($)

    me.roll = FixedMul(mang, sin(angle))
    me.pitch = FixedMul(mang, cos(angle))
end)
```

## player_t
- player.ipaddress (string) (read only): For use in moderation addons, this only returns a string for the server of the players IP address. Clients _cannot_ see other clients' IP addresses. The only way for other clients to know is if the server sends a command with them or something :p
- player.muted (boolean) (read + write): Returns whether or not the player is muted. 
# Sonic Robo Blast 2
[![latest release](https://badgen.net/github/release/STJr/SRB2/stable)](https://github.com/STJr/SRB2/releases/latest)

[![Build status](https://ci.appveyor.com/api/projects/status/399d4hcw9yy7hg2y?svg=true)](https://ci.appveyor.com/project/STJr/srb2)
[![Build status](https://travis-ci.org/STJr/SRB2.svg?branch=master)](https://travis-ci.org/STJr/SRB2)
[![CircleCI](https://circleci.com/gh/STJr/SRB2/tree/master.svg?style=svg)](https://circleci.com/gh/STJr/SRB2/tree/master)

[Sonic Robo Blast 2](https://srb2.org/) is a 3D Sonic the Hedgehog fangame based on a modified version of [Doom Legacy](http://doomlegacy.sourceforge.net/).

## Dependencies
- SDL2 (Linux/OS X only)
- SDL2-Mixer (Linux/OS X only)
- libupnp (Linux/OS X only)
- libgme (Linux/OS X only)
- libopenmpt (Linux/OS X only)

## Disclaimer
Sonic Team Junior is in no way affiliated with SEGA or Sonic Team. We do not claim ownership of any of SEGA's intellectual property used in SRB2.
