# Major edits (Luigi Budd)
- Toggable mobj pitch/roll functional (3d rotation for models on slopes, like DRRR). Option located in Video Settings -> Lebel
- CEchos/CSays print message into console
- Skin change at any time
- renderhitbox in multiplayer
- Hud interpolation from SRB2K Saturn (v.interpolate(bool/int) in lua)
- Joining netgame shows progress bar on "checking files" (TODO: 2.2.14 broke it)
- Adjustable gif size cap (toggable too!)
- Draw gif information to hud
- Crosshairs can invert pixels behind, to improve their visibility (must add tinyfontfix locally to work)
- Addfilelocal!
- Thin captions and thin FPS! (tinyfontfix.pk3 needed for captions)
- Ring-Racers-styled screen quakes! ("rr_quakes" in console)

"This is a fork from Luigi Budd's build, mostly the same features as the original. The difference is that I'm merging open merge requests stuff from srb2's official repo lol. Maybe I'll add something original from me idk. I do this for git learning" -GLide KS 
  
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
