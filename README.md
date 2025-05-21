# Changes
- Toggable mobj pitch/roll functional (3d rotation for models on slopes, like DRRR). Option located in Video Settings -> Level -> "Pitch/Roll Rotation"
- CEchos/CSays print message into console
- Skin change at any time
- renderhitbox in multiplayer
- Lua HUD interpolation from SRB2K Saturn
- Joining netgame shows progress bar on "checking files" ([Lugent's PR](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2446) [Lugent's PR](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2556))
- Adjustable gif size cap, toggable too! (gif_maxsize, "Max GIF Size (MB)")
    - ^ When gif is capped, gif_rolling allows for another gif to immediately start! (gif_rolling, "Keep recording when capped")
- Draw gif information to hud
- Crosshairs can invert pixels behind, to improve their visibility
- Addfilelocal from SRB2K Saturn! (use "addfilelocal" command or press SHIFT in the addons menu)
- Thin captions and thin FPS! (tinyfontfix.pk3 needed for captions)
- Ring-Racers-styled screen quakes! ("rr_quakes" in console)
- Countdown beep isn't ear piercingly loud!
- "freezelevel" debug command (Do not use when clients are connected)
- Snake download game background fixed!
- HUD camera struct updates position in first person! (credits [Jiskster](https://git.do.srb2.org/STJr/SRB2/-/merge_requests/2629) & [Hanicef](https://git.do.srb2.org/Hanicef/SRB2Classic/-/commit/681bd160f5be3925a97d798d00e67b32a8c1df71))
- Built in sprite clipping fix (gr_clipfix)
- Master Server list background for better readability!
- Server Menu before joining! (Code from [SRB2Classic](https://git.do.srb2.org/Hanicef/SRB2Classic))

# Lua Additions
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
- P_GetLocalAiming(player_t player) : Returns the angle_t `aiming` of `player` if they are a local player. Returns 0 otherwise.
- P_GetLocalAngle(player_t player) : Returns the angle_t `angle` of `player` if they are a local player. Returns 0 otherwise.
- v.interpolate/v.interpLatch(boolean/int) : See [SRB2K Saturn's documentation](https://github.com/Indev450/SRB2Kart-Saturn/blob/Saturn/LUASTUFF.md)
- v.drawFixedFill : Same as v.drawFill, but x, y, width, and height arguments are all in fixed point scale.
- M_Random* : Same as v.Random* functions, except also client-sided and not limited to HUD hooks.
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

## Compiling

See [SRB2 Wiki/Source code compiling](http://wiki.srb2.org/wiki/Source_code_compiling)

## Disclaimer
Sonic Team Junior is in no way affiliated with SEGA or Sonic Team. We do not claim ownership of any of SEGA's intellectual property used in SRB2.
