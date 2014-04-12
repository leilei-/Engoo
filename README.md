Engoo
=====

  This is a heavily modified Quake engine that adds multiple standard (and not so standard) features to the software renderer.

    - Colored lighting in 8-bit color by 15bpp or 18bpp lookup tables, supports lits
    - Translucent liquids with variable-adjusted alphas, supports vis
    - Fog, support for the standard FitzQuake fog command syntax and values
    - Widescreen display support, with hotizontal area expansion; support for 16:9, 16:10, and many more aspects 
    - Enhanced particle system including textured additive, modulated and alpha blended particles
    - Enhanced blood effects, as well as an option to disable blood and gibs
    - Coronas that occlude behind walls, sprites and models smoothly
    - Sound pitches, including dynamic pitching of sound in time scaling modes
    - Water refraction and reflection (DirectDraw only)
    - Auto-saving
    - Several weapon bobbing effects to help unstiff the player's sense of movement
    - Bots
    - MIDI playback
    - Can load BSP28 maps (Quake 0.9 shareware versions)
    - Can partially load BSP30 maps (Half-Life)
    - It doesn't load any 'high definition' packs (and never will, they tarnish the game's image)


Currently it has been tested on:

    - Windows 95 (eng95)
    - Windows XP
    - Windows 7
    
What I'd like to get it to working on

    - DOS (regressed, will need to fix)
    - Linux
    - MacOS 
    
What it currently compiles with:

    - Microsoft Visual C++ 6.0
    - DJGPP
