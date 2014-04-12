Engoo
=====

  This is a heavily modified Quake engine that adds multiple standard (and not so standard) features to the software renderer.

* Colored lighting in 8-bit color by 15bpp or 18bpp lookup tables, supports lits
* Translucent liquids with variable-adjusted alphas, supports vis
* Fog; support for the standard FitzQuake fog command syntax and values
* Widescreen display support, with horizontal area expansion; support for 16:9, 16:10, and many more aspects 
* Enhanced particle system including textured additive, modulated and alpha blended particles
* Enhanced blood effects, as well as an option to disable blood and gibs
* Coronas that occlude behind walls, sprites and models smoothly
* Sound pitches, including dynamic pitching of sound in time scaling modes
* Water refraction and reflection (DirectDraw only)
* Auto-saving
* Several weapon bobbing effects to help unstiff the player's sense of movement
* Bots
* MIDI playback
* Can load BSP28 maps (Quake 0.9 shareware versions)
* Can partially load BSP30 maps (Half-Life)
* It doesn't load any 'high definition' packs (and never will, they tarnish the game's image)


Currently it has been tested on:

* Windows 95 (eng95)
* Windows XP
* Windows 7
    
What I'd like to get it to working on

* DOS (regressed, will need to fix)
* Linux
* MacOS 
    
What it currently compiles with:

* Microsoft Visual C++ 6.0
* DJGPP


Some of the terrible bugs you should be aware of:

* CD audio is regressed.  It kept crashing under Windows 7 so I had to gut it for now :(
* MIDI doesn't work in DOS.
* Multiplayer is completely untested.  It may not work.
* Sometimes, resizing the window will cause it to become 1x1 instantly.
* It's a bit sluggish because the new colored lighting code is not written in assembly.


Some of the things that I want:

* Skyboxes
* MD2s
* MD3s including multiple surfaces and tag extension support
* IQMs
* External TGA loading including 24-bit support
* SPR32 including alpha channel support
* MDO playback via DUMB or some other Free module player that doesn't suck (no Mikmod)
* OGG playback also via DUMB via dumb_ogg
* Model texture filtering
* The DOS version working again
* More versatile DirectDraw support, including support for 4bpp, 8bpp, and 16bpp surfaces in Windows 95
* CPU speed detection so the extensive lookup table generation is disabled by default on anything less than 500mhz
