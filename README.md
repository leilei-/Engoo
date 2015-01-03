This is the vanilla Quake source experiment that aims only to be faster with the 80486 CPU in mind.  

It is not Engoo.  It's not going to have featurecreep.  

If it makes Quake slower and look better, it's not going here.  

If it makes Quake in 1080p for Windows 7, it's not going here.  

If it removes ASM code for C unroll hell and crossing fingers with great optimism that GCC is the king of optimization, it's not going here.

If it's SDL, it's not going here.

If it makes Quake not choke so much on very old PCs, it's going here.


TODO:

* Get the DOS version working because DOS is fast
* Port some optimizations from libretro/TyrQuake
* Port some optimizations from PocketQuake
* get the refdef actually take the whole screen instead of leaving some sort of 2/1 pixel border, mistake made in the source release
* Port some of the 'fast' things from Engoo like screen detail, r_dynamic, and fast lightmap mode
* Don't break anything
