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
// snd_null.c -- include this instead of all the other snd_* files to have
// no sound code whatsoever

#include "quakedef.h"

cvar_t	*bgmvolume;
cvar_t	*midivolume;
cvar_t	*volume;


// 2001-09-18 New cvar system by Maddes (Init)  start
/*
================
S_Init_Cvars
================
*/
void
S_Init_Cvars (void)
{
	volume = Cvar_Get ("volume", "0.7", CVAR_ARCHIVE|CVAR_ORIGINAL);
	bgmvolume = Cvar_Get ("bgmvolume", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	midivolume = Cvar_Get ("midivolume", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
}
// 2001-09-18 New cvar system by Maddes (Init)  end

void S_Init (void)
{
// 2001-09-18 New cvar system by Maddes (Init)  start
/*
	volume = Cvar_Get ("volume", "0.7", CVAR_ARCHIVE|CVAR_ORIGINAL);
	bgmvolume = Cvar_Get ("bgmvolume", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
*/
// 2001-09-18 New cvar system by Maddes (Init)  end
}

void S_AmbientOff (void)
{
}

void S_AmbientOn (void)
{
}

void S_Shutdown (void)
{
}

void S_TouchSound (char *sample)
{
}

void S_ClearBuffer (void)
{
}

void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
}

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol,  float attenuation)
{
}

void S_StartSound2 (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol,  float attenuation, int pitch)
{
}

void S_StopSound (int entnum, int entchannel)
{
}

sfx_t *S_PrecacheSound (char *sample)
{
	return NULL;
}

void S_ClearPrecache (void)
{
}

void S_Update (vec3_t origin, vec3_t v_forward, vec3_t v_right, vec3_t v_up)
{
}

void S_StopAllSounds (qboolean clear)
{
}

void S_BeginPrecaching (void)
{
}

void S_EndPrecaching (void)
{
}

void S_ExtraUpdate (void)
{
}

void S_LocalSound (char *s)
{
}

