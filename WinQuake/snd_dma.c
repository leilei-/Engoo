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
// snd_dma.c -- main control for any streaming sound output device

#include "quakedef.h"

#ifdef _WIN32
#include "winquake.h"
#endif

#ifdef	DUMB
#include "..\dumb\dumb.h"
#endif

#ifdef	ASS_MIDI
#ifdef _WIN32
#include "..\midilib\music.h"
#else
#include "..\mididos\music.h"
#endif				// TODO: Care for Linux
#endif

void S_Play(void);
void S_PlayVol(void);
void S_SoundList(void);
void S_Update_();
void S_StopAllSounds(qboolean clear);
void S_StopAllSoundsC(void);

// =======================================================================
// Internal sound data & structures
// =======================================================================

channel_t   channels[MAX_CHANNELS];
int			total_channels;

int				snd_blocked = 0;
static qboolean	snd_ambient = 1;
qboolean		snd_initialized = false;

// pointer should go away
volatile dma_t  *shm = 0;
volatile dma_t sn;

vec3_t		listener_origin;
vec3_t		listener_forward;
vec3_t		listener_right;
vec3_t		listener_up;
vec_t		sound_nominal_clip_dist=1000.0;

int			soundtime;		// sample PAIRS
int   		paintedtime; 	// sample PAIRS


#define	MAX_SFX		512
sfx_t		*known_sfx;		// hunk allocated [MAX_SFX]
int			num_sfx;

sfx_t		*ambient_sfx[NUM_AMBIENTS];

int 		desired_speed = 11025;
int 		desired_bits = 16;

int sound_started=0;

cvar_t	*bgmvolume;
cvar_t	*midivolume;
cvar_t	*volume;

cvar_t	*nosound;
cvar_t	*precache;
cvar_t	*loadas8bit;
cvar_t	*bgmbuffer;
cvar_t	*ambient_level;
cvar_t	*ambient_fade;
cvar_t	*snd_noextraupdate;
cvar_t	*snd_show;
cvar_t	*s_pitchin;
cvar_t	*s_oldspatial;
cvar_t	*_snd_mixahead;
cvar_t	*cl_gameplayfix_soundsmovewithentities;
cvar_t	*s_underwater;
cvar_t	*s_playerdeath;
cvar_t	*s_gibs;
cvar_t	*s_blood;
cvar_t	*r_ludicrous;
cvar_t	*r_ekg;

// ====================================================================
// User-setable variables
// ====================================================================


//
// Fake dma is a synchronous faking of the DMA progress used for
// isolating performance in the renderer.  The fakedma_updates is
// number of times S_Update() is called per second.
//

qboolean fakedma = false;
int fakedma_updates = 15;


void S_AmbientOff (void)
{
	snd_ambient = false;
}


void S_AmbientOn (void)
{
	snd_ambient = true;
}


void S_SoundInfo_f(void)
{
	if (!sound_started || !shm)
	{
		Con_Printf ("sound system not started\n");
		return;
	}

	Con_Printf("%5d stereo\n", shm->channels - 1);
	Con_Printf("%5d samples\n", shm->samples);
	Con_Printf("%5d samplepos\n", shm->samplepos);
	Con_Printf("%5d samplebits\n", shm->samplebits);
	Con_Printf("%5d submission_chunk\n", shm->submission_chunk);
	Con_Printf("%5d speed\n", shm->speed);
	Con_Printf("0x%x dma buffer\n", shm->buffer);
	Con_Printf("%5d total_channels\n", total_channels);
}


/*
================
S_Startup
================
*/

void S_Startup (void)
{
	int		rc;

	if (!snd_initialized)
		return;

	if (!fakedma)
	{
		rc = SNDDMA_Init();

		if (!rc)
		{
#ifndef	_WIN32
			Con_Printf("S_Startup: SNDDMA_Init failed.\n");
#endif
			sound_started = 0;
			return;
		}
	}

	sound_started = 1;
}

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
================
S_Init_Cvars
================
*/
void S_Init_Cvars (void)
{
	nosound = Cvar_Get ("nosound", "0", CVAR_ORIGINAL);
	volume = Cvar_Get ("volume", "0.7", CVAR_ARCHIVE|CVAR_ORIGINAL);
	precache = Cvar_Get ("precache", "1", CVAR_ORIGINAL);
	loadas8bit = Cvar_Get ("loadas8bit", "0", CVAR_ORIGINAL);
	bgmvolume = Cvar_Get ("bgmvolume", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	midivolume = Cvar_Get ("midivolume", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	bgmbuffer = Cvar_Get ("bgmbuffer", "4096", CVAR_ORIGINAL);
	ambient_level = Cvar_Get ("ambient_level", "0.3", CVAR_ORIGINAL);
	ambient_fade = Cvar_Get ("ambient_fade", "100", CVAR_ORIGINAL);
	snd_noextraupdate = Cvar_Get ("snd_noextraupdate", "0", CVAR_ORIGINAL);
	snd_show = Cvar_Get ("snd_show", "0", CVAR_ORIGINAL);
	_snd_mixahead = Cvar_Get ("_snd_mixahead", "0.1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	s_pitchin = Cvar_Get ("s_pitchin", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	s_oldspatial = Cvar_Get ("s_oldspatial", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	s_underwater = Cvar_Get ("s_underwater", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	s_gibs = Cvar_Get ("s_gibs", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	s_blood = Cvar_Get ("s_blood", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	s_playerdeath = Cvar_Get ("s_playerdeath", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	r_ludicrous = Cvar_Get ("r_ludicrous", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	r_ekg = Cvar_Get ("r_ekg", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	
	cl_gameplayfix_soundsmovewithentities = Cvar_Get ("cl_gameplayfix_soundsmovewithentities", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);

	if (host_parms.memsize < 0x800000)
	{
		Cvar_Set (loadas8bit, "1");
		Con_Printf ("loading all sounds as 8bit\n");
	}
}
// 2001-09-18 New cvar system by Maddes (Init)  end

/*
================
S_Init
================
*/
#ifdef DUMB
void register_dumbfile_system(DUMBFILE_SYSTEM *dfs);
#endif
#ifdef DUMB
	DUH *duh;
	DUH_SIGRENDERER *sr;	
	extern loadedfile_t	*thesong; 
#endif
	
void S_Init (void)
{
	int madi;
Con_Printf("\nSound Initialization\n");

	if (COM_CheckParm("-nosound"))
		return;

	if (COM_CheckParm("-simsound"))
		fakedma = true;

	Cmd_AddCommand("play", S_Play);
	Cmd_AddCommand("playvol", S_PlayVol);
	Cmd_AddCommand("stopsound", S_StopAllSoundsC);
	Cmd_AddCommand("soundlist", S_SoundList);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);


// 2001-09-18 New cvar system by Maddes (Init)  end

	snd_initialized = true;

	S_Startup ();

	SND_InitScaletable ();

	known_sfx = Hunk_AllocName (MAX_SFX*sizeof(sfx_t), "sfx_t");
	num_sfx = 0;

// create a piece of DMA memory

	if (fakedma)
	{
		shm = (void *) Hunk_AllocName(sizeof(*shm), "shm");
		shm->splitbuffer = 0;
		shm->samplebits = 16;
		shm->speed = 22050;
		shm->channels = 2;
		shm->samples = 32768;
		shm->samplepos = 0;
		shm->soundalive = true;
		shm->gamealive = true;
		shm->submission_chunk = 1;
		shm->buffer = Hunk_AllocName(1<<16, "shmbuf");
	}

	if (shm)	// 2000-07-09 GPF when GUS or BLASTER variable not set fix by Maddes/Raymond Martineau
		Con_Printf ("Sound sampling rate: %i\n", shm->speed);

	// provides a tick sound until washed clean

//	if (shm->buffer)
//		shm->buffer[4] = shm->buffer[5] = 0x7f;	// force a pop for debugging

	ambient_sfx[AMBIENT_WATER] = S_PrecacheSound ("ambience/water1.wav");
	ambient_sfx[AMBIENT_SKY] = S_PrecacheSound ("ambience/wind2.wav");

#ifdef DUMB


	// FIXME: DUMB ISNT EVEN IMPLEMENTED PROPERLY!
	dumb_register_stdfiles();	// eww! stds!
	dumb_it_max_to_mix = 64;
	/*
	// you don't have testeng so this will crash. get your own damn mod files
	//duh = dumb_load_mod("testeng/sound/cdtracks/track004.mod");
	if(thesong)
	duh = dumb_load_mod("testeng/sound/cdtracks/track004.mod");
	if (!duh) {
					Con_Printf("DUH OUCH! CAN NOT LOAD THE DUMB MOD!\n");
					}
	else
					Con_Printf("YES..... I CAN\n");
	sr = duh_start_sigrenderer(duh, 0, 2, 0);

		if (!sr) {
		unload_duh(duh);
		
		Con_Printf("DUH OUCH! can't play into the dumb buffer! goh dumb it!\n");
	}
		else
			Con_Printf("THE SIGRENDERER'S STARTED.\n");

 //register_dumbfile_system(&duhfile);
 */
#endif
#ifdef ASS_MIDI
	// It's 2010 and we're still using MIDI!?

	madi = MUSIC_Init(0, 0);// we only want the default device on non-dos platforms
      	Con_Printf ("MIDI system - %s\n", MUSIC_ErrorString( MUSIC_Error ) );

	// TODO: do optional opl3 emulator for this driver

#endif
	S_StopAllSounds (true);
}


// =======================================================================
// Shutdown sound engine
// =======================================================================

void S_Shutdown(void)
{

	if (!sound_started)
		return;

	if (shm)
		shm->gamealive = 0;
#ifdef ASS_MIDI
	MUSIC_Shutdown();
#endif
	shm = 0;
	sound_started = 0;

	if (!fakedma)
	{
		SNDDMA_Shutdown();
	}
}


// =======================================================================
// Load a sound
// =======================================================================

/*
==================
S_FindName

==================
*/
sfx_t *S_FindName (char *name)
{
	int		i;
	sfx_t	*sfx;

	if (!name)
		Sys_Error ("S_FindName: NULL\n");

	if (strlen(name) >= MAX_QPATH)
		Sys_Error ("Sound name too long: %s", name);

// see if already loaded
	for (i=0 ; i < num_sfx ; i++)
		if (!strcmp(known_sfx[i].name, name))
		{
			return &known_sfx[i];
		}

	if (num_sfx == MAX_SFX)
		Sys_Error ("S_FindName: out of sfx_t");

	sfx = &known_sfx[i];
	strcpy (sfx->name, name);

	num_sfx++;

	return sfx;
}


/*
==================
S_TouchSound

==================
*/
void S_TouchSound (char *name)
{
	sfx_t	*sfx;

	if (!sound_started)
		return;

	sfx = S_FindName (name);
	Cache_Check (&sfx->cache);
}

/*
==================
S_PrecacheSound

==================
*/
sfx_t *S_PrecacheSound (char *name)
{
	sfx_t	*sfx;

	if (!sound_started || nosound->value)
		return NULL;

	sfx = S_FindName (name);

// cache it in
	if (precache->value)
		S_LoadSound (sfx);

	return sfx;
}


//=============================================================================

/*
=================
SND_PickChannel
=================
*/
channel_t *SND_PickChannel(int entnum, int entchannel)
{
    int ch_idx;
    int first_to_die;
    int life_left;

// Check for replacement sound, or find the best one to replace
    first_to_die = -1;
    life_left = 0x7fffffff;
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++)
    {
		if (entchannel != 0		// channel 0 never overrides
		&& channels[ch_idx].entnum == entnum
		&& (channels[ch_idx].entchannel == entchannel || entchannel == -1) )
		{	// always override sound from same entity
			first_to_die = ch_idx;
			break;
		}

		// don't let monster sounds override player sounds
		if (channels[ch_idx].entnum == cl.viewentity && entnum != cl.viewentity && channels[ch_idx].sfx)
			continue;

		if (channels[ch_idx].end - paintedtime < life_left)
		{
			life_left = channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
   }

	if (first_to_die == -1)
		return NULL;

	if (channels[first_to_die].sfx)
		channels[first_to_die].sfx = NULL;

    return &channels[first_to_die];
}

extern int oldspatval;

void SND_Spatialize(channel_t *ch)
{
	vec_t dot;
	vec_t dist;
	vec_t lscale, rscale, scale;
	vec3_t source_vec;

// anything coming from the view entity will always be full volume
	if (ch->entnum == cl.viewentity)
	{
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}



// calculate stereo seperation and distance attenuation

	VectorSubtract(ch->origin, listener_origin, source_vec);

	dist = VectorNormalize(source_vec) * ch->dist_mult;

	dot = DotProduct(listener_right, source_vec);

	if (shm->channels == 1)
	{
		rscale = 1.0;
		lscale = 1.0;
	}
	else
	{
		rscale = 1.0 + dot;
		lscale = 1.0 - dot;
	}
	

// add in distance effect
	scale = (1.0 - dist) * rscale;

	ch->rightvol = (int) (ch->master_vol * scale);
	if (ch->rightvol < 0)
		ch->rightvol = 0;

     scale = (1.0 - dist) * lscale;

	ch->leftvol = (int) (ch->master_vol * scale);
	if (ch->leftvol < 0)
		ch->leftvol = 0;

	
}

extern qboolean	inwat;	// leilei - for underwater sounds
// =======================================================================
// Start a sound effect
// =======================================================================

extern cvar_t	*temp2;

float		leicho;
float		leivol;
int			leitap;
int			leiverb;

void S_StartSoundForReal(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation)
{
	channel_t *target_chan, *check;
	sfxcache_t	*sc;
	int		vol;
	int		ch_idx;
	int		skip;
	int		pitch;
	float	itch;
	float	itchr;

	vec3_t what;


	// leilei - CONTENT HACK - really stupid hack to add special effects to sound effects.   TOTALLY OPTIONAL and completely stupid
	// and is the WRONG way to go about doing this.
	if (s_gibs->value){
		int gibbb;
		gibbb = rand() % 100;
	if (!strncmp(sfx->name,"player/gib",10) || !strncmp(sfx->name,"zombie/z_gib",12) || !strncmp(sfx->name,"player/udeath",12))
			
	//	R_ParticleBloodSpewism (origin, 18);
		if (gibbb < 4 && r_ludicrous->value || r_ekg->value){
			R_ParticleBloodSpewism (origin, 38);
			R_ParticleBloodSpewism (origin, 18);
			if (!r_ekg->value )
				if (r_ludicrous->value > 2)
			Con_Printf("Ludicrous Gibs!\n");
		}
	else
	{
			R_ParticleBloodSpewism (origin, 11);
			R_ParticleBloodSpewism (origin, 7);
	}
		
	}
		
	

	if (!sound_started)
		return;

	if (!sfx)
		return;

	if (nosound->value)
		return;

// leilei - CONTENT HACK - anti-global scream option.  Slightly more reasonable than the crap seen above.
	if (s_playerdeath->value)
	if ((!strncmp(sfx->name,"player/",7)) && attenuation < 1)
			attenuation = 1;



	itch = (float)s_pitchin->value; // leilei  - get pitch for speed here
	vol = fvol*255 * leivol;

// leilei - CONTENT HACK - don't do random pitches on doors/platforms/ambients
	if (!strncmp(sfx->name,"ambien",6) || !strncmp(sfx->name,"door",4) || !strncmp(sfx->name,"plat",4))
			itch = 0;


// pick a channel to play on
	target_chan = SND_PickChannel(entnum, entchannel);
	if (!target_chan)
		return;

// spatialize
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = vol;
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
	SND_Spatialize(target_chan);


	if (!target_chan->leftvol && !target_chan->rightvol)
		return;		// not audible at all


// new channel
	sc = S_LoadSound (sfx);

	
	if (!sc)
	{
		target_chan->sfx = NULL;
		return;		// couldn't load the sound's data
	}
	
	{
		if (inwat && s_underwater->value){
					sc->flags = 2;			// Muffle the sound in the water
					itchr = 1;
		}
				else
				{
					sc->flags = 0;			// Clear sound
					itchr = 0;
				}
	}
	

	//	 AJA - Pitch Shift
	if (itch)
		target_chan->step = (int)(256 - (itch*2)) + (int)((rand()%64) * (itch/2));
		else
	 target_chan->step = 256;


	//arget_chan->step += sc->ratio;
	//		Con_Printf("%f", sc->ratio);
		if (itchr)
			target_chan->step *= 0.8;		// slow down sounds in underwater slightly
	if (sc->loopstart > 1) 
		target_chan->step = 256; // don't shift looped sounds at all

	//	target_chan->step *= host_timescale->value; // slowmo sounds hack
	target_chan->sfx = sfx;
	target_chan->pos = 0.0;
	target_chan->end = paintedtime + ((sc->length<<8)/target_chan->step);

// if an identical sound has also been started this frame, offset the pos
// a bit to keep it from just making the first one louder
	check = &channels[NUM_AMBIENTS];
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++, check++)
    {
		if (check == target_chan)
			continue;
		if (check->sfx == sfx && !check->pos)
		{
			skip = rand () % (int)(0.1*shm->speed);
			if (skip >= target_chan->end)
				skip = target_chan->end - 1;
		    target_chan->pos += skip << 8;  // AJA - Pitch Shift
			target_chan->end -= skip;
			break;
		}

	}


	// leilei's crappy echo hack
	{

			leiverb = (int)leicho % (int)(0.1*shm->speed);
			if (leiverb >= target_chan->end)
				leiverb = target_chan->end - 1;
		    target_chan->pos -= leiverb << 8;  // AJA - Pitch Shift
			target_chan->end += leiverb;
	}
//	if (sc->loopstart > 1)
//	Con_Printf("loopstart %i\n", sc->loopstart);
}

void S_StartSound(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation)
{
	int ah, sh;
/*
	for (ah=1; ah<4; ah++){
			leivol = fvol / (ah * 0.5);
			leiverb = ah * temp2->value;
			S_StartSoundForReal(entnum, entchannel, sfx, origin, fvol, attenuation);
	}*/
	leivol = 1;
	S_StartSoundForReal(entnum, entchannel, sfx, origin, fvol, attenuation);

}


// =======================================================================
// Start a custom sound effect
// =======================================================================

void S_StartSound2(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation, int pitch)
{
	channel_t *target_chan, *check;
	sfxcache_t	*sc;
	int		vol;
	int		ch_idx;
	int		skip;
//	int		pitch;
	float	itch;

	if (!sound_started)
		return;

	if (!sfx)
		return;

	if (nosound->value)
		return;

	//itch = (float)s_pitchin->value; // leilei  - get pitch for speed here
	itch = (float)pitch; // leilei  - get pitch for speed here
	vol = fvol*255;

// pick a channel to play on
	target_chan = SND_PickChannel(entnum, entchannel);
	if (!target_chan)
		return;

// spatialize
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = vol;
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
	SND_Spatialize(target_chan);


	if (!target_chan->leftvol && !target_chan->rightvol)
		return;		// not audible at all


// new channel
	sc = S_LoadSound (sfx);
	if (!sc)
	{
		target_chan->sfx = NULL;
		return;		// couldn't load the sound's data
	}
	
	
	//	 AJA - Pitch Shift
	if (itch)
		target_chan->step = (int)(256 - (itch*2)) + (int)(64 * (itch/2));
		else
	 target_chan->step = 256;
	if (sc->loopstart > 1) 
		target_chan->step = 256; // don't shift looped sounds at all

	//	target_chan->step *= host_timescale->value; // slowmo sounds hack
	target_chan->sfx = sfx;
	target_chan->pos = 0.0;
	target_chan->end = paintedtime + ((sc->length<<8)/target_chan->step);

// if an identical sound has also been started this frame, offset the pos
// a bit to keep it from just making the first one louder
	check = &channels[NUM_AMBIENTS];
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++, check++)
    {
		if (check == target_chan)
			continue;
		if (check->sfx == sfx && !check->pos)
		{
			skip = rand () % (int)(0.1*shm->speed);
			if (skip >= target_chan->end)
				skip = target_chan->end - 1;
		    target_chan->pos += skip << 8;  // AJA - Pitch Shift
			target_chan->end -= skip;
			break;
		}

	}
}

void S_StopSound(int entnum, int entchannel)
{
	int i;

	for (i=0 ; i<MAX_DYNAMIC_CHANNELS ; i++)
	{
		if (channels[i].entnum == entnum
			&& channels[i].entchannel == entchannel)
		{
			channels[i].end = 0;
			channels[i].sfx = NULL;
			return;
		}
	}
}

void S_StopAllSounds(qboolean clear)
{
	int		i;

	if (!sound_started)
		return;

#ifdef ASS_MIDI
	MUSIC_StopSong();
#endif

	total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;	// no statics

	for (i=0 ; i<MAX_CHANNELS ; i++)
		if (channels[i].sfx)
			channels[i].sfx = NULL;

	Q_memset(channels, 0, MAX_CHANNELS * sizeof(channel_t));

	if (clear)
		S_ClearBuffer ();
}

void S_StopAllSoundsC (void)
{
	S_StopAllSounds (true);
}

void S_ClearBuffer (void)
{
	int		clear;
#ifdef ASS_MIDI
	//	MUSIC_StopSong();
#endif
#ifdef _WIN32
	if (!sound_started || !shm || (!shm->buffer && !pDSBuf))
#else
	if (!sound_started || !shm || !shm->buffer)
#endif
		return;

	if (shm->samplebits == 8)
		clear = 0x80;
	else
		clear = 0;

#ifdef _WIN32
	if (pDSBuf)
	{
		DWORD	dwSize;
		DWORD	*pData;
		int		reps;
		HRESULT	hresult;

		reps = 0;

		while ((hresult = pDSBuf->lpVtbl->Lock(pDSBuf, 0, gSndBufSize, &pData, &dwSize, NULL, NULL, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("S_ClearBuffer: DS::Lock Sound Buffer Failed\n");
				S_Shutdown ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("S_ClearBuffer: DS: couldn't restore buffer\n");
				S_Shutdown ();
				return;
			}
		}

		Q_memset(pData, clear, shm->samples * shm->samplebits/8);

		pDSBuf->lpVtbl->Unlock(pDSBuf, pData, dwSize, NULL, 0);

	}
	else
#endif
	{
		Q_memset(shm->buffer, clear, shm->samples * shm->samplebits/8);
	}
}


/*
=================
S_StaticSound
=================
*/
void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
	channel_t	*ss;
	sfxcache_t		*sc;

	if (!sfx)
		return;

	if (total_channels == MAX_CHANNELS)
	{
		Con_Printf ("total_channels == MAX_CHANNELS\n");
		return;
	}

	ss = &channels[total_channels];
	total_channels++;

	sc = S_LoadSound (sfx);
	if (!sc)
		return;

	if (sc->loopstart == -1)
	{
		Con_Printf ("Sound %s not looped\n", sfx->name);
		return;
	}
	ss->step = 256; 
	ss->sfx = sfx;
	VectorCopy (origin, ss->origin);
	ss->master_vol = vol;
	ss->dist_mult = (attenuation/64) / sound_nominal_clip_dist;
    ss->end = paintedtime + sc->length;

	SND_Spatialize (ss);

}


//=============================================================================

/*
===================
S_UpdateAmbientSounds
===================
*/
void S_UpdateAmbientSounds (void)
{
	mleaf_t		*l;
	float		vol;
	int			ambient_channel;
	channel_t	*chan;

	if (!snd_ambient)
		return;

// calc ambient sound levels
	if (!cl.worldmodel)
		return;

	l = Mod_PointInLeaf (listener_origin, cl.worldmodel);
	if (!l || !ambient_level->value)
	{
		for (ambient_channel = 0 ; ambient_channel< NUM_AMBIENTS ; ambient_channel++)
			channels[ambient_channel].sfx = NULL;
		return;
	}

	for (ambient_channel = 0 ; ambient_channel< NUM_AMBIENTS ; ambient_channel++)
	{
		chan = &channels[ambient_channel];
		chan->sfx = ambient_sfx[ambient_channel];
		chan->step = 256; // ambient suck hack
		vol = ambient_level->value * l->ambient_sound_level[ambient_channel];
		if (vol < 8)
			vol = 0;

	// don't adjust volume too fast
		if (chan->master_vol < vol)
		{
			chan->master_vol += host_org_frametime * ambient_fade->value;
			if (chan->master_vol > vol)
				chan->master_vol = vol;
		}
		else if (chan->master_vol > vol)
		{
			chan->master_vol -= host_org_frametime * ambient_fade->value;
			if (chan->master_vol < vol)
				chan->master_vol = vol;
		}
		
		chan->leftvol = chan->rightvol = chan->master_vol;
	
	}
}


/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
	int			i, j;
	int			total;
	channel_t	*ch;
	channel_t	*combine;

	if (!sound_started || (snd_blocked > 0))
		return;

	VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);

// update general area ambient sound sources
	S_UpdateAmbientSounds ();
	
	combine = NULL;

// update spatialization for static and dynamic sounds
	ch = channels+NUM_AMBIENTS;
	for (i=NUM_AMBIENTS ; i<total_channels; i++, ch++)
	{
		if (!ch->sfx)
			continue;
		
		SND_Spatialize(ch);	// respatialize channel
		if (!ch->leftvol && !ch->rightvol)
			continue;

	// try to combine static sounds with a previous channel of the same
	// sound effect so we don't mix five torches every frame

		if (i >= MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS)
		{
		// see if it can just use the last one
			if (combine && combine->sfx == ch->sfx)
			{
				combine->leftvol += ch->leftvol;
				combine->rightvol += ch->rightvol;
				ch->leftvol = ch->rightvol = 0;
				continue;
			}
		// search for one
			combine = channels+MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
			for (j=MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS ; j<i; j++, combine++)
				if (combine->sfx == ch->sfx)
					break;

			if (j == total_channels)
			{
				combine = NULL;
			}
			else
			{
				if (combine != ch)
				{
					combine->leftvol += ch->leftvol;
					combine->rightvol += ch->rightvol;
					ch->leftvol = ch->rightvol = 0;
				}
				continue;
			}
		}


	}

//
// debugging output
//
	if (snd_show->value)
	{
		total = 0;
		ch = channels;
		for (i=0 ; i<total_channels; i++, ch++)
			if (ch->sfx && (ch->leftvol || ch->rightvol) )
			{
				//Con_Printf ("%3i %3i %s\n", ch->leftvol, ch->rightvol, ch->sfx->name);
				total++;
			}

		Con_Printf ("----(%i)----\n", total);
	}

// mix some sound
	S_Update_();
}

void GetSoundtime(void)
{
	int		samplepos;
	static	int		buffers;
	static	int		oldsamplepos;
	int		fullsamples;

	fullsamples = shm->samples / shm->channels;

// it is possible to miscount buffers if it has wrapped twice between
// calls to S_Update.  Oh well.
#ifdef __sun__
	soundtime = SNDDMA_GetSamples();
#else
	samplepos = SNDDMA_GetDMAPos();


	if (samplepos < oldsamplepos)
	{
		buffers++;					// buffer wrapped

		if (paintedtime > 0x40000000)
		{	// time to chop things off to avoid 32 bit limits
			buffers = 0;
			paintedtime = fullsamples;
			S_StopAllSounds (true);
		}
	}
	oldsamplepos = samplepos;

	soundtime = buffers*fullsamples + samplepos/shm->channels;
#endif
}

void S_ExtraUpdate (void)
{

#ifdef _WIN32
	IN_Accumulate ();
#endif

	if (snd_noextraupdate->value)
		return;		// don't pollute timings
	S_Update_();
}

void MIDI_Update(void)
{


#ifdef ASS_MIDI
		MUSIC_SetVolume( midivolume->value * 256);
		MUSIC_Update(); // leilei - IT WORKS! IT WOOOOOOOOORKS!!!!!!!!!
		
#endif


}

int oldspat;
int oldspatval;

void S_Update_(void)
{
	unsigned	endtime;
	int			samps;

	if (!sound_started || (snd_blocked > 0))
		return;

// Updates DMA time
	GetSoundtime();

// check to make sure that we haven't overshot
	if (paintedtime < soundtime)
	{
		//Con_Printf ("S_Update_ : overflow\n");
		paintedtime = soundtime;
	}

// mix ahead of current position
	endtime = soundtime + _snd_mixahead->value * shm->speed;
	samps = shm->samples >> (shm->channels-1);
	if (endtime - soundtime > samps)
		endtime = soundtime + samps;

	if (oldspatval != s_oldspatial->value)
	{
		oldspatval = s_oldspatial->value;


	}
	// lordhavoc / leilei - classic attenuation
	if (oldspatval){
		sound_nominal_clip_dist = 1200;
	}
	else
	{
		sound_nominal_clip_dist = 1000;
	}

#ifdef _WIN32
// if the buffer was lost or stopped, restore it and/or restart it
	{
		DWORD	dwStatus;

		if (pDSBuf)
		{
			if (pDSBuf->lpVtbl->GetStatus (pDSBuf, &dwStatus) != DD_OK)
				Con_Printf ("Couldn't get sound buffer status\n");

			if (dwStatus & DSBSTATUS_BUFFERLOST)
				pDSBuf->lpVtbl->Restore (pDSBuf);

			if (!(dwStatus & DSBSTATUS_PLAYING))
				pDSBuf->lpVtbl->Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);
		}
	}
#endif

	S_PaintChannels (endtime);

	SNDDMA_Submit ();
}

/*
===============================================================================

console functions

===============================================================================
*/

void S_Play(void)
{
	static int hash=345;
	int 	i;
	char name[256];
	sfx_t	*sfx;
	int	poop;
	i = 1;
	while (i<Cmd_Argc())
	{
		if (!strrchr(Cmd_Argv(i), '.'))
		{
			strcpy(name, Cmd_Argv(i));
			strcat(name, ".wav");
		}
		else
			strcpy(name, Cmd_Argv(i));
		sfx = S_PrecacheSound(name);
		
		S_StartSound(hash++, 0, sfx, listener_origin, 1.0, 1.0);

		//					for (poop = 2; poop<8; poop++){
		//					float aw = 1.6 / poop;
		//					S_StartSoundDelayed(hash++, 0, sfx, listener_origin, (1 * aw), 1.0, (1 + aw));
		//					
		//					}
		i++;
	}
}

void S_PlayVol(void)
{
	static int hash=543;
	int i;
	float vol;
	char name[256];
	sfx_t	*sfx;

	i = 1;
	while (i<Cmd_Argc())
	{
		if (!strrchr(Cmd_Argv(i), '.'))
		{
			strcpy(name, Cmd_Argv(i));
			strcat(name, ".wav");
		}
		else
			strcpy(name, Cmd_Argv(i));
		sfx = S_PrecacheSound(name);
		vol = Q_atof(Cmd_Argv(i+1));
		S_StartSound(hash++, 0, sfx, listener_origin, vol, 1.0);
		i+=2;
	}
}

void S_SoundList(void)
{
	int		i;
	sfx_t	*sfx;
	sfxcache_t	*sc;
	int		size, total;

	total = 0;
	for (sfx=known_sfx, i=0 ; i<num_sfx ; i++, sfx++)
	{
		sc = Cache_Check (&sfx->cache);
		if (!sc)
			continue;
		size = sc->length*sc->width*(sc->stereo+1);
		total += size;
		if (sc->loopstart >= 0)
			Con_Printf ("L");
		else
			Con_Printf (" ");
		Con_Printf("(%2db) %6i : %s\n",sc->width*8,  size, sfx->name);
	}
	Con_Printf ("Total resident: %i\n", total);
}


void S_LocalSound (char *sound)
{
	sfx_t	*sfx;

	if (nosound->value)
		return;
	if (!sound_started)
		return;

	sfx = S_PrecacheSound (sound);
	if (!sfx)
	{
		Con_Printf ("S_LocalSound: can't cache %s\n", sound);
		return;
	}
	S_StartSound (cl.viewentity, -1, sfx, vec3_origin, 1, 1);
}


void S_ClearPrecache (void)
{
}


void S_BeginPrecaching (void)
{
}


void S_EndPrecaching (void)
{
}


