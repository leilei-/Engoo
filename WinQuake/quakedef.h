/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// quakedef.h -- primary header for client
	
//#define	GLTEST			// experimental stuff
#define	QUAKE_GAME			// as opposed to utilities
//#define BENCH				// Standalone benchmark only (no sound or menu)
//#define PROTO				// Prototype additions
							// this game isn't anything anymore
#ifdef _WIN32
#include "mathlib.h"		// I make djgpp upset for some reason
#endif
#include "matrixlib.h"
#include "version.h"
//#define	EXPERIMENT			// This just enables the -fixmefixmefixme- menu
							// this is pretty much the line between release versions
							// and unreleased versions
							// for the sake of the end user really


							// WARNING: THESE CAN INCREASE MEMORY USAGE TO 200MB! CAUTION!
//#define	QSB					// QSB Limit increases to meet the standard of QSB
//#define	QSB_NET				// QSB's changes to network protocol that breaks things
							// leilei note: I raised the limit on unreliable messages
							// to a stupidly high amount for debugging. Please read below
							// for why.
// EXPERIMENTALY HACKS
#define	twentyfourbithack			// Converts textures to 24-bit for dithered rendering. Doesn't work
//#define	dithermodelhack		// Attempts dithered lighting on models
//#define	statictest		// uses Static for the tables used by rgb surfaceblocks
//#define	LOOKANGLE			// adds aimangle (crude pointing of the viewmodel to autoaim angle)
#define ITSFIX
//#define ANTIPACKETOVERFLOW // argh
//#define WINDOWS31			// to allow the engine to be run in windows 3 w/ win32s
							// it doesn't work right now (and latest win32s aborts it too)
							// but theoretically it is broken at the TIMER level, at least
							// in older mid-1995 win32s versions

//#define SPLIT				// Splitscreen hack (NOTE: Depends on GLOBOT for now, for client functionality)
#define REALLYCRASHESITHINK
// -----------------------------------------
// Video Features
// -----------------------------------------

//#define EGA				// EGA 4-bit colors experiment *WIP*
//#define EGAHACK			// EGA Palette Hackup  Test
//#define VGA				// VGA 4-bit colors experiment to allow game running in safe mode *WIP*
//#define INTERPOL			// buggy interpolation (ToChriS/makaqu port - this will crash.)
//#define MHINTERPOL			// other interpolation (still buggy)*WIP*
#define INTERPOL7			// other interpolation again (still buggy)*WIP*
//#define NOFULLSCREENEVER	// absolutely force windowed mode 
//#define INTERPOL2			// Tochris interpolation
//#define ALPHASCALE			// enable .alpha and .scale*WIP* (may break protocol?)
//#define SCALEE				// enable just the scale ocde, no protocols
#define	SCALED2D			// enables scaled hud/menu etc. adapted from siggi's uhexen2 patch
//#define	THIRTYTWOBITHACK
//#define	MMXHACK				// try to shove in MMX intrinsics to some functions

//#define		DECALS				// Port of FTEQW's disabled SW DECALS feature. Probably doesn't work.
//#define COMBINED
//#define			STAINMAPS			// I don't like this common feature much, but we'll try it anyway.
//#define		WATERLOW			// Use a warpbuffer clone for storing the reflection in, rendering it stretched to the buffer (why)
//#define		EXPREND			// Experimental shadowmap rendering mode
#define	WATERREFLECTIONS
//#define	INTERPOLENTITIES

//#define	VOODOO				// vid_win.c only - try to mock 3dfx Voodoo 4x1 filter

// -----------------------------------------
// Audio Features
// -----------------------------------------

//#define DUMB				// Use DUMB Module playback library *WIP*
#define ASS_MIDI			// Use Apogee Sound System for MIDI playback only*WIP*

int inthedos;


#define EFFINGMOUSE			// disables "Enhance Pointer Precision" 
							// a definite MUST for debugging

// -----------------------------------------
// Gameplay Features
// -----------------------------------------

#define	GLOBOT				// tomaz globots
//#define VMTOC				// viewmodelforclient changes
#define SONOFABITS			// Bits workaround :(
//#define	BITSAGAIN			// qip way

// -----------------------------------------
// Versioning
// -----------------------------------------

int	qbeta;							// when 1, it tries to force guestimated pre-1.00 behaviors
int	protocol;						// OK.
int Nehahrademcompatibility; // LordHavoc: to allow playback of the early Nehahra movie segments
int dpprotocol;			
#define	DPPROTOCOLS	
#define	QIP_VERSION			"Build 277"
#define	QIP_URL				"nowhere"
#define	VERSION				1.08
#define	TNQ_VERSION			240	// todo: increment build numbers
#define	GLQUAKE_VERSION		1.00
#define	D3DQUAKE_VERSION	0.01
#define	WINQUAKE_VERSION	2.77	// was 0.996, but WinQuake was already final
#define	LINUX_VERSION		1.30
#define	X11_VERSION			1.10


#define	PROTOCOL_STOCK		0		// old 1.09
#define	PROTOCOL_QUAKEDP	1		// dp105
#define	PROTOCOL_TQ			2		// Tomazquake



//define	PARANOID			// speed sapping error checking

// -----------------------------------------
// Game Data
// -----------------------------------------

#ifdef PROTO
#define	GAMENAME	"data"
#else
#define	GAMENAME	"id1"
#endif

#define FIGHT				1 // fight experiment

// -----------------------------------------
// Game Modes
//
// Support for other Q1-derivative engine games
//
// This is used to keep built-ins used by these 
// games from conflicting with others, as well as
// spiffying them up in general (i.e. hud in 
// Transfusion)
//
// THIS IS NOT FINAL AND IS SUBJECT TO CHANGE
// AND PROMISE
// -----------------------------------------

int		gamemode;	

#define GAME_QUAKE			0	// 1997 - 1.07-1.09
#define GAME_QUAKE_OLD		1	// 1996 - 0.8-0.92
#define GAME_QUAKE_ARCADE	2	// 1998 - mame cabinet, has avi playback (TODO: MCI this) and a new menu system
#define GAME_QUAKE_106		3	// 1996 - 1.00-1.06 (lacks all mp changes)
#define GAME_LASER_ARENA	4	// 2000 - trainwreck
#define GAME_TRANSFUSION	5	// 2002 
#define GAME_CIA_OPERATIVE	6	// 2001 - trainwreck
#define GAME_JESUSTHEFPS	7	// 2003 - very jesusy
#define GAME_GRASS			8	// 2002
#define	GAME_MINIRACER		9	// 2002
//#define GAME_HEXEN_II		10	// 1997 - too many system vars changes atm
#define GAME_FIEND_HUNTER	11  // 2001 v0.64 
#define GAME_NEHAHRA		12	// 2000 
#define GAME_URBANMERC		13	// 2000 - profits are "emulated"
#define GAME_KUROK			14	// 2008
#define GAME_FIGHT			15	// 2015?

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

//
// The precompiler definition "__i386__" comes from the DJGPP package and
// states that the code is compiled for an Intel x386 compatible machine.
// Windows compilers (definition "_WIN32") do not provide this DJGPP definition,
// but provide something similar called "_M_IX86". So when using a Windows
// compiler "__i386__" can be created when "_WIN32" and "_M_IX86" are defined.
//
// If you do not want to use assembler code, then set the definition "id386" to
// zero (0) in QUAKEASM.H for asm sources and in QUAKEDEF.H for C sources
//

#if defined(_WIN32) && !defined(WINDED)

#if defined(_M_IX86)
#define __i386__	1
#endif

void	VID_LockBuffer (void);
void	VID_UnlockBuffer (void);

#else

#define	VID_LockBuffer()
#define	VID_UnlockBuffer()

#endif

#if defined __i386__ && !defined NOASM // && !defined __sun__
#define id386	1
#define id386poly 0
#define id386rgb	0
#else
#define id386	0
#define id386poly 0
#define id386rgb	0
#endif

#if id386
#define UNALIGNED_OK	1	// set to 0 if unaligned accesses are not supported
#else
#define UNALIGNED_OK	0
#endif

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define CACHE_SIZE	32		// used to align key data structures

#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings
#ifdef BENCH
#define	MINIMUM_MEMORY			0x0186A0 // 100kb hack test
#else
#define	MINIMUM_MEMORY			0x550000
#endif
#define	MINIMUM_MEMORY_LEVELPAK	(MINIMUM_MEMORY + 0x100000)

#define MAX_NUM_ARGVS	50

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


#define	MAX_QPATH		64			// max length of a quake game pathname
#define	MAX_OSPATH		128			// max length of a filesystem pathname

#define	ON_EPSILON		0.1			// point on plane side epsilon
#ifdef QSB_NET
#define	MAX_MSGLEN		65536		// max length of a reliable message
//#define	MAX_DATAGRAM	1400		// max length of unreliable message
#define	MAX_DATAGRAM	16000		// max length of unreliable message
									// leilei - I only raised this just to 
									// have fun with excessive entity stress.
									// (craploads of monsters) because it's nice
									// to not see "PACKET OVERFLOW" spammed.
									// please lower this to 1400 for serious
									// QSB standard use.
#else


#define	MAX_MSGLEN		8000		// max length of a reliable message
#define	MAX_DATAGRAM	1024		// max length of unreliable message

#endif




//
// per-level limits
//
// 2001-09-20 Configurable entity limits by Maddes  start
//#define	MAX_EDICTS		600			// FIXME: ouch! ouch! ouch!


#ifdef QSB_NET
#define	MAX_MODELS		4096		// these are sent over the net as bytes
#define	MAX_SOUNDS		4096			// so they cannot be blindly increased

#define	MIN_EDICTS		600				// must be the original default value: 600
#define	MAX_EDICTS		8192		// Maximum without network changes, as entity number is send as signed short (2 bytes)

#define	MIN_TEMP_ENTITIES	1024			// lightning bolts, etc
#define	MIN_STATIC_ENTITIES	1024			// torches, etc
// 2001-09-20 Configurable entity limits by Maddes  end
#define	MAX_LIGHTSTYLES	64


#else
#define	MAX_MODELS		256			// these are sent over the net as bytes
#define	MAX_SOUNDS		256			// so they cannot be blindly increased

#define	MIN_EDICTS		600				// must be the original default value: 600
#define	MAX_EDICTS		0x7FFF			// Maximum without network changes, as entity number is send as signed short (2 bytes)

#define	MIN_TEMP_ENTITIES	64			// lightning bolts, etc
#define	MIN_STATIC_ENTITIES	128			// torches, etc
// 2001-09-20 Configurable entity limits by Maddes  end
#define	MAX_LIGHTSTYLES	64

#endif

#define	SAVEGAME_COMMENT_LENGTH	39

#define	MAX_STYLESTRING	64

//
// stats are integers communicated to the client by the server
//
#define	MAX_CL_STATS		32
#define	STAT_HEALTH			0
#define	STAT_FRAGS			1
#define	STAT_WEAPON			2
#define	STAT_AMMO			3
#define	STAT_ARMOR			4
#define	STAT_WEAPONFRAME	5
#define	STAT_SHELLS			6
#define	STAT_NAILS			7
#define	STAT_ROCKETS		8
#define	STAT_CELLS			9
#define	STAT_ACTIVEWEAPON	10
#define	STAT_TOTALSECRETS	11
#define	STAT_TOTALMONSTERS	12
#define	STAT_SECRETS		13		// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14		// bumped by svc_killedmonster

// stock defines

#define	IT_SHOTGUN				1
#define	IT_SUPER_SHOTGUN		2
#define	IT_NAILGUN				4
#define	IT_SUPER_NAILGUN		8
#define	IT_GRENADE_LAUNCHER		16
#define	IT_ROCKET_LAUNCHER		32
#define	IT_LIGHTNING			64
#define IT_SUPER_LIGHTNING		128
#define IT_SHELLS				256
#define IT_NAILS				512
#define IT_ROCKETS				1024
#define IT_CELLS				2048
#define IT_AXE					4096
#define IT_ARMOR1				8192
#define IT_ARMOR2				16384
#define IT_ARMOR3				32768
#define IT_SUPERHEALTH			65536
#define IT_KEY1					131072
#define IT_KEY2					262144
#define	IT_INVISIBILITY			524288
#define	IT_INVULNERABILITY		1048576
#define	IT_SUIT					2097152
#define	IT_QUAD					4194304
#define IT_SIGIL1				(1<<28)
#define IT_SIGIL2				(1<<29)
#define IT_SIGIL3				(1<<30)
#define IT_SIGIL4				(1<<31)

//===========================================
//rogue changed and added defines

#define RIT_SHELLS				128
#define RIT_NAILS				256
#define RIT_ROCKETS				512
#define RIT_CELLS				1024
#define RIT_AXE					2048
#define RIT_LAVA_NAILGUN		4096
#define RIT_LAVA_SUPER_NAILGUN	8192
#define RIT_MULTI_GRENADE		16384
#define RIT_MULTI_ROCKET		32768
#define RIT_PLASMA_GUN			65536
#define RIT_ARMOR1				8388608
#define RIT_ARMOR2				16777216
#define RIT_ARMOR3				33554432
#define RIT_LAVA_NAILS			67108864
#define RIT_PLASMA_AMMO			134217728
#define RIT_MULTI_ROCKETS		268435456
#define RIT_SHIELD				536870912
#define RIT_ANTIGRAV			1073741824
#define RIT_SUPERHEALTH			2147483648

//MED 01/04/97 added hipnotic defines
//===========================================
//hipnotic added defines
#define HIT_PROXIMITY_GUN_BIT	16
#define HIT_MJOLNIR_BIT			7
#define HIT_LASER_CANNON_BIT	23
#define HIT_PROXIMITY_GUN		(1<<HIT_PROXIMITY_GUN_BIT)
#define HIT_MJOLNIR				(1<<HIT_MJOLNIR_BIT)
#define HIT_LASER_CANNON		(1<<HIT_LASER_CANNON_BIT)
#define HIT_WETSUIT				(1<<(23+2))
#define HIT_EMPATHY_SHIELDS		(1<<(23+3))

//===========================================

#define	MAX_SCOREBOARD		16
#define	MAX_SCOREBOARDNAME	32

#define	SOUND_CHANNELS		8

// This makes anyone on id's net privileged
// Use for multiplayer testing only - VERY dangerous!!!
// #define IDGODS

#include "common.h"
#include "bspfile.h"
#include "vid.h"
#include "sys.h"
#include "zone.h"
#include "mathlib.h"
#include "matrixlib.h"

typedef struct
{
	vec3_t	origin;
	vec3_t	angles;
	int		modelindex;
	int		frame;
	int		colormap;
	int		skin;
	int		effects;
} entity_state_t;


#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "screen.h"
#include "net.h"
#include "protocol.h"
#include "cmd.h"
#include "sbar.h"
#include "sound.h"
#include "render.h"
#include "client.h"
#include "progs.h"
#include "server.h"

#include "nvs_common.h"		// 2000-04-30 NVS COMMON by Maddes

#ifdef GLQUAKE
#include "gl_model.h"
#else
#include "model.h"
#include "d_iface.h"
#endif
#include "model_common.h"	// 2001-12-28 Merged model functions by Maddes

#include "input.h"
#include "world.h"
#include "keys.h"
#include "console.h"
#include "view.h"
#include "menu.h"
#include "crc.h"
#include "cdaudio.h"

#ifdef GLQUAKE
#include "glquake.h"
#endif

//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct
{
	char	*basedir;
	char	*cachedir;		// for development over ISDN lines
	int		argc;
	char	**argv;
	void	*membase;
	int		memsize;
} quakeparms_t;


//=============================================================================



extern qboolean noclip_anglehack;


//
// host
//
extern	quakeparms_t host_parms;

extern	cvar_t		*sys_ticrate;
extern	cvar_t		*sys_nostdout;
extern	cvar_t		*developer;

extern	qboolean	host_initialized;		// true if into command execution

extern	double		host_frametime;
extern	byte		*host_basepal;
extern	byte		*host_colormap;
extern	byte		*host_colormap_red;
extern	byte		*host_colormap_green;
extern	byte		*host_colormap_blue;
extern	byte		*host_colormap_buffer;
extern	byte		*host_fogmap;		// leilei - fog
extern	byte		*host_egamap;
extern	byte		*glcolormap;
extern	byte		*host_noopal;		// for translations
extern	int			host_fullbrights;   // for preserving fullbrights in color operations

extern	int			host_framecount;	// incremented every frame, never reset
extern	double		realtime;			// not bounded in any way, changed at
										// start of every frame, never reset

void Host_ClearMemory (void);
void Host_ServerFrame (void);
void Host_InitCommands (void);
void Host_Init (quakeparms_t *parms);
void Host_Shutdown(void);
void Host_Error (char *error, ...);
void Host_EndGame (char *message, ...);
void Host_Frame (float time);
void Host_Quit_f (void);
void Host_ClientCommands (char *fmt, ...);
void Host_ShutdownServer (qboolean crash);
void Host_Version_f (void);				// 2000-04-30 NVS HANDSHAKE SRV<->CL/QC<->CL by Maddes

extern qboolean		msg_suppress_1;		// suppresses resolution and cache size console output
										//  an fullscreen DIB focus gain/loss
extern int			current_skill;		// skill level for currently loaded level (in case
										//  the user changes the cvar while the level is
										//  running, this reflects the level actually in use)

extern qboolean		isDedicated;

extern int			minimum_memory;

//
// chase
//
extern	cvar_t	*chase_active;

void Chase_Init (void);
void Chase_Reset (void);
void Chase_Update (void);

extern qboolean		nouse;	// 1999-10-29 +USE fix by Maddes

#define PROGHEADER080_CRC	26940	// leilei - beta check
#define PROGHEADER090_CRC	26940
#define PROGHEADER091_CRC	26940
#define PROGHEADER092_CRC	26940	
#define PROGHEADERLA_CRC	27094	// laser arena

// 1999-10-28 Compatibilty check by Maddes  start
#define PROGHEADER101_CRC	5927
extern qboolean		keep_compatibility;
// 1999-10-28 Compatibilty check by Maddes  end

// 2000-07-30 DJGPP compiler warning fix by Norberto Alfredo Bensa  start
qboolean SV_RecursiveHullCheck (hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace);
dfunction_t *ED_FindFunction (char *name);	// 2000-01-09 QCExec by FrikaC/Maddes
// 2000-07-30 DJGPP compiler warning fix by Norberto Alfredo Bensa  end

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
extern	double	host_cpu_frametime;
extern	double	host_org_frametime;

extern	cvar_t	*host_timescale;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end


//
// MusicInfo struct.
//

typedef struct {
  // up to 6-character name
  char *name;

  // lump number of music
  int lumpnum;

  // music data
  void *data;

  // music handle once registered
  int handle;
} musicinfo_t;


// the complete set of music
extern musicinfo_t  S_music[];

// some names for integers of various sizes, all unsigned 

typedef unsigned char UBYTE;  // a one-byte int 
typedef unsigned short UWORD; // a two-byte int 
//typedef unsigned int ULONG;   // a four-byte int (assumes int 4 bytes) 
typedef unsigned int OOLONG;   // a four-byte int (assumes int 4 bytes) 