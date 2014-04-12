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

#include "quakedef.h"

#define	RETURN_EDICT(e) (((int *)pr_globals)[OFS_RETURN] = EDICT_TO_PROG(e))

#define PR_MAX_TEMPSTRING 2048	// 2001-10-25 Enhanced temp string handling by Maddes

memzone_t	*zone_progstrings;	// 2001-09-20 QuakeC string zone by Maddes

// 2001-10-20 Extension System by LordHavoc/Maddes  start
char *pr_extensions[] =
{
// add the extension names here, syntax: "extensionname",
	"TIMESCALE",	// 2001-10-20 TIMESCALE extension by Tomaz/Maddes
// 2001-09-16 Quake 2 builtin functions by id/Maddes  start
	"DP_QC_ETOS",
	"DP_QC_CHANGEPITCH",
	"DP_QC_TRACETOSS",
// 2001-09-16 Quake 2 builtin functions by id/Maddes  end
// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  start
	"DP_REGISTERCVAR",	// 2001-09-18 New BuiltIn Function: cvar_create() by Maddes
	"DP_QC_SINCOSSQRTPOW",
	"DP_QC_TRACEBOX",
	"DP_QC_RANDOMVEC",
	"DP_QC_MINMAXBOUND",
	"DP_QC_FINDFLOAT",
	"DP_QC_COPYENTITY",
	"DP_SV_SETCOLOR",
	"DP_QC_FINDCHAIN",
	"DP_QC_FINDCHAINFLOAT",
// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  end
// 2010-11-02 More Darkplaces extensions added by leilei start
	"DP_EF_ADDITIVE",	
	"DP_EF_BLUE",		
	"DP_EF_RED",		
	"DP_EF_NODRAW",
	"DP_EF_FULLBRIGHT",
//	"DP_ENT_ALPHA",
	"DP_MOVETYPEBOUNCEMISSILE",	
	"DP_LITSUPPORT",	// finally!
	"FRIK_FILE",	
	"TQ_RAILTRAIL",	
	"TQ_RAIN",	
	"TQ_SNOW",	
	// To be added! QSB Proposed extensions laundry list:
	/*


	DP_GFX_EXTERNALTEXTURES	// may be unneccessary, but needed for md2/md3
	DP_GFX_SKYBOX
	DP_CL_LOADSKY
	DP_BUTTONUSE	// there is +use, but nothing to the actual +buttonuse
	DP_CON_SET
	DP_CON_SETA		// i think this is added by maddes, but unextensioned
	DP_ENT_SCALE	// some skeleton scaling stuff in already
	DP_ENT_EXTERIORMODELTOCLIENT
	DP_ENT_VIEWMODEL	// this one is tough as it involves matrix math
	DP_INPUTBUTTONS
	DP_MONSTERWALK
	DP_MOVETYPEFOLLOW	// movetype_follow in currently is 'quake2' stuff
	DP_QC_CVAR_STRING
	DP_QC_FINDFLAGS
	DP_QC_FS_SEARCH
	DP_QC_UNLIMITEDTEMPSTRINGS
	DP_QC_TRACE_MOVETYPE_WORLDONLY
	DP_QC_VECTOANGLES_WITH_ROLL
	DP_QUAKE2_MODEL	// do we really need crappy md2? GUYS...
	DP_SND_DIRECTIONLESSATTNNONE	// and cvared, too. quakeguy death uses attnnone
	DP_SND_FAKETRACKS	// sort of in for midi, but not ogg or mod at the moment
	DP_SOLIDCORPSE
	DP_SV_DRAWONLYTOCLIENT
	DP_SV_ENTITYCONTENTSTRANSITION
	DP_SV_MOVETYPESTEP_LANDEVENT	// oof!
	DP_SV_POINTSOUND
	DP_SV_NODRAWTOCLIENT		// good for 'parental lock', and germany
	DP_SV_PRECACHEANYTIME
	DP_SV_ROTATINGBMODEL	// this in sort of
	DP_TE_STANDARDEFFECTBUILTINS
	FTE_TE_STANDARDEFFECTBUILTINS	// both i think may be implemented. untested
	DP_VIEWZOOM				// simple in concept but this one is very hard to do in wq
	KRIMZON_SV_PARSECLIENTCOMMAND	
	FTE_QC_CHECKPVS
	FTE_STRINGS
	NEH_RESTOREGAME
	QSB_FOG
	QSB_GAMEPLAYFIXES
	QSB_MOVINGSOUNDS	// lol i can do this








  */
};

int pr_numextensions = sizeof(pr_extensions)/sizeof(pr_extensions[0]);

qboolean extension_find(char *name)
{
	int	i;

	for (i=0; i < pr_numextensions; i++)
	{
		if (!Q_strcasecmp(pr_extensions[i], name))
			return true;
	}
	return false;
}

/*
=================
PF_extension_find

returns true if the extension is supported by the server

float extension_find(string name)
=================
*/
void PF_extension_find (void)
{
	G_FLOAT(OFS_RETURN) = extension_find(G_STRING(OFS_PARM0));
}
// 2001-10-20 Extension System by LordHavoc/Maddes  end

/*
===============================================================================

						BUILT-IN FUNCTIONS

===============================================================================
*/

char	pr_varstring_temp[PR_MAX_TEMPSTRING];	// 2001-10-25 Enhanced temp string handling by Maddes

char *PF_VarString (int	first)
{
	int		i;
// 2001-10-25 Enhanced temp string handling by Maddes  start
/*
	static char out[256];

	out[0] = 0;
	for (i=first ; i<pr_argc ; i++)
	{
		strcat (out, G_STRING((OFS_PARM0+i*3)));
	}
	return out;
*/
	int		maxlen;
	char	*add;

	pr_varstring_temp[0] = 0;
	for (i=first ; i<pr_argc ; i++)
	{
		maxlen = PR_MAX_TEMPSTRING - strlen(pr_varstring_temp) - 1;	// -1 is EndOfString
		add = G_STRING((OFS_PARM0+i*3));
		if (maxlen > strlen(add))
		{
			strcat (pr_varstring_temp, add);
		}
		else
		{
			strncat (pr_varstring_temp, add, maxlen);
			pr_varstring_temp[PR_MAX_TEMPSTRING-1] = 0;
			break;	// can stop here
		}
	}
	return pr_varstring_temp;
// 2001-10-25 Enhanced temp string handling by Maddes  end
}


/*
=================
PF_error

This is a TERMINAL error, which will kill off the entire server.
Dumps self.

error(value)
=================
*/
void PF_error (void)
{
	char	*s;
	edict_t	*ed;

	s = PF_VarString(0);
	Con_Printf ("======SERVER ERROR in %s:\n%s\n"
	,pr_strings + pr_xfunction->s_name,s);
	ed = PROG_TO_EDICT(pr_global_struct->self);
	ED_Print (ed);

	Host_Error ("Program error");
}

/*
=================
PF_objerror

Dumps out self, then an error message.  The program is aborted and self is
removed, but the level can continue.

objerror(value)
=================
*/
void PF_objerror (void)
{
	char	*s;
	edict_t	*ed;

	s = PF_VarString(0);
	Con_Printf ("======OBJECT ERROR in %s:\n%s\n", pr_strings + pr_xfunction->s_name, s);
	ed = PROG_TO_EDICT(pr_global_struct->self);
	ED_Print (ed);
	ED_Free (ed);

//	Host_Error ("Program error");	// 2001-12-16 Do not stop server on objerror
}



/*
==============
PF_makevectors

Writes new values for v_forward, v_up, and v_right based on angles
makevectors(vector)
==============
*/
void PF_makevectors (void)
{
	AngleVectors (G_VECTOR(OFS_PARM0), pr_global_struct->v_forward, pr_global_struct->v_right, pr_global_struct->v_up);
}

/*
=================
PF_setorigin

This is the only valid way to move an object without using the physics of the world (setting velocity and waiting).  Directly changing origin will not set internal links correctly, so clipping would be messed up.  This should be called when an object is spawned, and then only if it is teleported.

setorigin (entity, origin)
=================
*/
void PF_setorigin (void)
{
	edict_t	*e;
	float	*org;

	e = G_EDICT(OFS_PARM0);
	org = G_VECTOR(OFS_PARM1);
	VectorCopy (org, e->v.origin);
	SV_LinkEdict (e, false);
}


void SetMinMaxSize (edict_t *e, float *min, float *max, qboolean rotate)
{
	float	*angles;
	vec3_t	rmin, rmax;
	float	bounds[2][3];
	float	xvector[2], yvector[2];
	float	a;
	vec3_t	base, transformed;
	int		i, j, k, l;

	for (i=0 ; i<3 ; i++)
		if (min[i] > max[i])
			PR_RunError ("backwards mins/maxs");

	rotate = false;		// FIXME: implement rotation properly again

	if (!rotate)
	{
		VectorCopy (min, rmin);
		VectorCopy (max, rmax);
	}
	else
	{
	// find min / max for rotations
		angles = e->v.angles;

		a = angles[1]/180 * M_PI;

		xvector[0] = cos(a);
		xvector[1] = sin(a);
		yvector[0] = -sin(a);
		yvector[1] = cos(a);

		VectorCopy (min, bounds[0]);
		VectorCopy (max, bounds[1]);

		rmin[0] = rmin[1] = rmin[2] = 9999;
		rmax[0] = rmax[1] = rmax[2] = -9999;

		for (i=0 ; i<= 1 ; i++)
		{
			base[0] = bounds[i][0];
			for (j=0 ; j<= 1 ; j++)
			{
				base[1] = bounds[j][1];
				for (k=0 ; k<= 1 ; k++)
				{
					base[2] = bounds[k][2];

				// transform the point
					transformed[0] = xvector[0]*base[0] + yvector[0]*base[1];
					transformed[1] = xvector[1]*base[0] + yvector[1]*base[1];
					transformed[2] = base[2];

					for (l=0 ; l<3 ; l++)
					{
						if (transformed[l] < rmin[l])
							rmin[l] = transformed[l];
						if (transformed[l] > rmax[l])
							rmax[l] = transformed[l];
					}
				}
			}
		}
	}

// set derived values
	VectorCopy (rmin, e->v.mins);
	VectorCopy (rmax, e->v.maxs);
	VectorSubtract (max, min, e->v.size);

	SV_LinkEdict (e, false);
}

/*
=================
PF_setsize

the size box is rotated by the current angle

setsize (entity, minvector, maxvector)
=================
*/
void PF_setsize (void)
{
	edict_t	*e;
	float	*min, *max;

	e = G_EDICT(OFS_PARM0);
	min = G_VECTOR(OFS_PARM1);
	max = G_VECTOR(OFS_PARM2);
	SetMinMaxSize (e, min, max, false);
}

void THE_precache_model (char *barf);
/*
=================
PF_setmodel

setmodel(entity, model)
=================
*/
void PF_setmodel (void)
{
	edict_t	*e;
	char	*m, **check;
	model_t	*mod;
	int		i;

	e = G_EDICT(OFS_PARM0);
	m = G_STRING(OFS_PARM1);

// check to see if model was properly precached
	for (i=0, check = sv.model_precache ; *check ; i++, check++)
		if (!strcmp(*check, m))
			break;

		if (!*check){

		//PR_RunError ("no precache: %s\n", m);
			THE_precache_model(m);
			//e->v.model = NULL;
			return;
			

		}


	e->v.model = m - pr_strings;
	e->v.modelindex = i;	//SV_ModelIndex (m);

	mod = sv.models[(int)e->v.modelindex];	// Mod_ForName (m, true);

	if (mod){
		SetMinMaxSize (e, mod->mins, mod->maxs, true);

	}
	else
	{
		SetMinMaxSize (e, vec3_origin, vec3_origin, true);
		
	}
}

/*
=================
PF_bprint

broadcast print to everyone on server

bprint(value)
=================
*/
void PF_bprint (void)
{
	char		*s;

	s = PF_VarString(0);
	SV_BroadcastPrintf ("%s", s);
}

/*
=================
PF_sprint

single print to a specific client

sprint(clientent, value)
=================
*/
void PF_sprint (void)
{
	char		*s;
	client_t	*client;
	int			entnum;
#ifdef GLOBOT
edict_t		*ent;
	ent = G_EDICT(OFS_PARM0);	

	if (!ent->bot.isbot)
	{

#endif

	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteChar (&client->message,svc_print);
	MSG_WriteString (&client->message, s );
#ifdef GLOBOT
		}
#endif
}


/*
=================
PF_centerprint

single print to a specific client

centerprint(clientent, value)
=================
*/
void PF_centerprint (void)
{
	char		*s;
	client_t	*client;
	int			entnum;
#ifdef GLOBOT
edict_t		*ent;
	ent = G_EDICT(OFS_PARM0);	

	if (!ent->bot.isbot)
	{

#endif
	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteChar (&client->message,svc_centerprint);
	MSG_WriteString (&client->message, s );
#ifdef GLOBOT
		}
#endif
}


/*
=================
PF_normalize

vector normalize(vector)
=================
*/
void PF_normalize (void)
{
	float	*value1;
	vec3_t	newvalue;
	float	new;

	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);

	if (new == 0)
		newvalue[0] = newvalue[1] = newvalue[2] = 0;
	else
	{
		new = 1/new;
		newvalue[0] = value1[0] * new;
		newvalue[1] = value1[1] * new;
		newvalue[2] = value1[2] * new;
	}

	VectorCopy (newvalue, G_VECTOR(OFS_RETURN));
}

/*
=================
PF_vlen

scalar vlen(vector)
=================
*/
void PF_vlen (void)
{
	float	*value1;
	float	new;

	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);

	G_FLOAT(OFS_RETURN) = new;
}

/*
=================
PF_vectoyaw

float vectoyaw(vector)
=================
*/
void PF_vectoyaw (void)
{
	float	*value1;
	float	yaw;

	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	G_FLOAT(OFS_RETURN) = yaw;
}


/*
=================
PF_vectoangles

vector vectoangles(vector)
=================
*/
void PF_vectoangles (void)
{
	float	*value1;
	float	forward;
	float	yaw, pitch;

	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	G_FLOAT(OFS_RETURN+0) = pitch;
	G_FLOAT(OFS_RETURN+1) = yaw;
	G_FLOAT(OFS_RETURN+2) = 0;
}

/*
=================
PF_Random

Returns a number from 0>= num <= 1

random()
=================
*/
void PF_random (void)
{
	float		num;

	num = (rand ()&0x7fff) / ((float)0x7fff);

	G_FLOAT(OFS_RETURN) = num;
}

/*
=================
PF_particle

particle(origin, color, count)
=================
*/
void PF_particle (void)
{
	float		*org, *dir;
	float		color;
	float		count;

	org = G_VECTOR(OFS_PARM0);
	dir = G_VECTOR(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	count = G_FLOAT(OFS_PARM3);
	SV_StartParticle (org, dir, color, count);
}


/*
=================
PF_ambientsound

=================
*/
void PF_ambientsound (void)
{
	char		**check;
	char		*samp;
	float		*pos;
	float 		vol, attenuation;
	int			i, soundnum;

	pos = G_VECTOR (OFS_PARM0);
	samp = G_STRING(OFS_PARM1);
	vol = G_FLOAT(OFS_PARM2);
	attenuation = G_FLOAT(OFS_PARM3);

// check to see if samp was properly precached
	for (soundnum=0, check = sv.sound_precache ; *check ; check++, soundnum++)
		if (!strcmp(*check,samp))
			break;

	if (!*check)
	{
		Con_Printf ("no precache: %s\n", samp);
		return;
	}

// add an svc_spawnambient command to the level signon packet

	MSG_WriteByte (&sv.signon,svc_spawnstaticsound);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord(&sv.signon, pos[i]);

	MSG_WriteByte (&sv.signon, soundnum);

	MSG_WriteByte (&sv.signon, vol*255);
	MSG_WriteByte (&sv.signon, attenuation*64);

}

/*
=================
PF_sound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.

=================
*/
void PF_sound (void)
{
	char		*sample;
	int			channel;
	edict_t		*entity;
	int 		volume;
	float attenuation;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);
	sample = G_STRING(OFS_PARM2);
	volume = G_FLOAT(OFS_PARM3) * 255;
	attenuation = G_FLOAT(OFS_PARM4);

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	SV_StartSound (entity, channel, sample, volume, attenuation);
}


/*
=================
PF_sound3

With a pitch

=================
*/

void PF_sound3 (void)
{
	char		*sample;
	int			channel;
	edict_t		*entity;
	int 		volume;
	float		pitch;
//	float flags;
	float attenuation;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);
	sample = G_STRING(OFS_PARM2);
	volume = G_FLOAT(OFS_PARM3) * 255;
	attenuation = G_FLOAT(OFS_PARM4);
	pitch = G_FLOAT(OFS_PARM5);
	Con_Printf ("pitch got %i\n", pitch);
//	flags = G_FLOAT(OFS_PARM6);

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound2: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound2: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound2: channel = %i", channel);

	SV_StartSound2 (entity, channel, sample, volume, attenuation, pitch); //, flags);

}
/*
=================
PF_break

break()
=================
*/
void PF_break (void)
{
Con_Printf ("break statement\n");
*(int *)-4 = 0;	// dump to debugger
//	PR_RunError ("break statement");
}

/*
=================
PF_traceline

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entities, and also slide box entities
if the tryents flag is set(?).

void(vector v1, vector v2, float nomonsters, entity forent) traceline
=================
*/
void PF_traceline (void)
{
	float	*v1, *v2;
	trace_t	trace;
	int		nomonsters;
	edict_t	*ent;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	nomonsters = G_FLOAT(OFS_PARM2);
	ent = G_EDICT(OFS_PARM3);

	trace = SV_Move (v1, vec3_origin, vec3_origin, v2, nomonsters, ent);

	pr_global_struct->trace_allsolid = trace.allsolid;
	pr_global_struct->trace_startsolid = trace.startsolid;
	pr_global_struct->trace_fraction = trace.fraction;
	pr_global_struct->trace_inwater = trace.inwater;
	pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, pr_global_struct->trace_plane_normal);
	pr_global_struct->trace_plane_dist = trace.plane.dist;
	if (trace.ent)
		pr_global_struct->trace_ent = EDICT_TO_PROG(trace.ent);
	else
		pr_global_struct->trace_ent = EDICT_TO_PROG(sv.edicts);
}


//#ifdef QUAKE2	// 2001-09-16 Quake 2 builtin functions by id/Maddes
extern trace_t SV_Trace_Toss (edict_t *ent, edict_t *ignore);

void PF_TraceToss (void)
{
	trace_t	trace;
	edict_t	*ent;
	edict_t	*ignore;

	ent = G_EDICT(OFS_PARM0);
	ignore = G_EDICT(OFS_PARM1);

	trace = SV_Trace_Toss (ent, ignore);

	pr_global_struct->trace_allsolid = trace.allsolid;
	pr_global_struct->trace_startsolid = trace.startsolid;
	pr_global_struct->trace_fraction = trace.fraction;
	pr_global_struct->trace_inwater = trace.inwater;
	pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, pr_global_struct->trace_plane_normal);
	pr_global_struct->trace_plane_dist = trace.plane.dist;
	if (trace.ent)
		pr_global_struct->trace_ent = EDICT_TO_PROG(trace.ent);
	else
		pr_global_struct->trace_ent = EDICT_TO_PROG(sv.edicts);
}
//#endif	// 2001-09-16 Quake 2 builtin functions by id/Maddes


/*
=================
PF_checkpos

Returns true if the given entity can move to the given position from it's
current position by walking or rolling.
FIXME: make work...
scalar checkpos (entity, vector)
=================
*/
void PF_checkpos (void)
{
}

//============================================================================

byte	checkpvs[MAX_MAP_LEAFS/8];

int PF_newcheckclient (int check)
{
	int		i;
	byte	*pvs;
	edict_t	*ent;
	mleaf_t	*leaf;
	vec3_t	org;

// cycle to the next one

	if (check < 1)
		check = 1;
	if (check > svs.maxclients)
		check = svs.maxclients;

	if (check == svs.maxclients)
		i = 1;
	else
		i = check + 1;

	for ( ; ; i++)
	{
		if (i == svs.maxclients+1)
			i = 1;

		ent = EDICT_NUM(i);

		if (i == check)
			break;	// didn't find anything else

		if (ent->free)
			continue;
		if (ent->v.health <= 0)
			continue;
		if ((int)ent->v.flags & FL_NOTARGET)
			continue;

	// anything that is a client, or has a client as an enemy
		break;
	}

// get the PVS for the entity
	VectorAdd (ent->v.origin, ent->v.view_ofs, org);
	leaf = Mod_PointInLeaf (org, sv.worldmodel);
	pvs = Mod_LeafPVS (leaf, sv.worldmodel);
	memcpy (checkpvs, pvs, (sv.worldmodel->numleafs+7)>>3 );

	return i;
}

/*
=================
PF_checkclient

Returns a client (or object that has a client enemy) that would be a
valid target.

If there are more than one valid options, they are cycled each frame

If (self.origin + self.viewofs) is not in the PVS of the current target,
it is not returned at all.

name checkclient ()
=================
*/
#define	MAX_CHECK	16
int c_invis, c_notvis;
void PF_checkclient (void)
{
	edict_t	*ent, *self;
	mleaf_t	*leaf;
	int		l;
	vec3_t	view;

// find a new check if on a new frame
	if (sv.time - sv.lastchecktime >= 0.1)
	{
		sv.lastcheck = PF_newcheckclient (sv.lastcheck);
		sv.lastchecktime = sv.time;
	}

// return check if it might be visible
	ent = EDICT_NUM(sv.lastcheck);
	if (ent->free || ent->v.health <= 0)
	{
		RETURN_EDICT(sv.edicts);
		return;
	}

// if current entity can't possibly see the check entity, return 0
	self = PROG_TO_EDICT(pr_global_struct->self);
	VectorAdd (self->v.origin, self->v.view_ofs, view);
	leaf = Mod_PointInLeaf (view, sv.worldmodel);
	l = (leaf - sv.worldmodel->leafs) - 1;
	if ( (l<0) || !(checkpvs[l>>3] & (1<<(l&7)) ) )
	{
c_notvis++;
		RETURN_EDICT(sv.edicts);
		return;
	}

// might be able to see it
c_invis++;
	RETURN_EDICT(ent);
}

//============================================================================


/*
=================
PF_stuffcmd

Sends text over to the client's execution buffer

stuffcmd (clientent, value)
=================
*/
void PF_stuffcmd (void)
{
	int		entnum;
	char	*str;
	client_t	*old;
#ifdef GLOBOT
	edict_t		*ent;
	static qboolean	next_is_value;
	ent = G_EDICT(OFS_PARM0);	
	 next_is_value = false;
	if (!ent->bot.isbot)
	{

#endif
	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("Parm 0 not a client");
	str = G_STRING(OFS_PARM1);

	old = host_client;
	host_client = &svs.clients[entnum-1];
	Host_ClientCommands ("%s", str);
	host_client = old;
#ifdef GLOBOT
		}
		// MAD UGLY HACK TO GET TEAM FORTRESS TO WORK WITH GLOBOT
	else
	{
		str = G_STRING (OFS_PARM1);

		if (str[0] == 'c' &&
			str[1] == 'o' &&
			str[2] == 'l' &&
			str[3] == 'o' &&
			str[4] == 'r')
		{
			next_is_value = true;
			return;
		}
		else if (next_is_value)
		{
			int		value;

			Con_Printf (str);

			next_is_value = false;

			entnum = G_EDICTNUM(OFS_PARM0);
			old = &svs.clients[entnum-1];

			if (str[0] == '4')
				value = 4;
			else if (str[0] == '1' && str[1] == '1')
				value = 11;
			else if (str[0] == '1' && str[1] == '2')
				value = 12;
			else if (str[0] == '1' && str[1] == '3')
				value = 13;
			else
				return;

			old->colors = value * 16 + value;
			old->edict->v.team = value + 1;

		// send notification to all clients
			MSG_WriteByte (&sv.reliable_datagram, svc_updatecolors);
			MSG_WriteByte (&sv.reliable_datagram, old - svs.clients);
			MSG_WriteByte (&sv.reliable_datagram, old->colors);
		}
	}
	// MAD UGLY HACK TO GET TEAM FORTRESS TO WORK WITH GLOBOT

#endif
}

/*
=================
PF_localcmd

Adds text to the server's execution buffer

localcmd (string)
=================
*/
void PF_localcmd (void)
{
	char	*str;

	str = G_STRING(OFS_PARM0);
	Cbuf_AddText (str);
}

// 2001-09-18 New cvar system by Maddes  start
//            completly new functions
/*
=================
PF_cvar

float cvar (string)
=================
*/
void PF_cvar (void)
{
	char	*varname;
	cvar_t	*var;
	float	value;

	value = 0;

	varname = G_STRING(OFS_PARM0);
	var = Cvar_FindVar (varname);
	if (var)
	{
		value = var->value;
	}
	G_FLOAT(OFS_RETURN) = value;
}

/*
=================
PF_cvar_set

float cvar_set (string, string)
=================
*/
void PF_cvar_set (void)
{
	char	*varname;
	cvar_t	*var;

	varname = G_STRING(OFS_PARM0);
	var = Cvar_FindVar (varname);
	if (!var)
	{
		Con_DPrintf ("Cvar_Set: variable \"%s\" not found\n", varname);	// 2001-09-09 Made 'Cvar not found' a developer message by Maddes
		return;
	}

	if ( (var->flags & CVAR_ROM)		// check for progs-protected cvar (=not a progs or user created variable)
	     && (!(var->flags & (CVAR_USER_CREATED|CVAR_PROGS_CREATED))) )
	{
		Con_DPrintf ("Cvar_Set: variable \"%s\" is read-only\n", var->name);
		return;
	}

	Cvar_Set (var, G_STRING(OFS_PARM1));
}
// 2001-09-18 New cvar system by Maddes  end

/*
=================
PF_findradius

Returns a chain of entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
void PF_findradius (void)
{
	edict_t	*ent, *chain;
	float	rad;
	float	*org;
	vec3_t	eorg;
	int		i;//, j;

	chain = (edict_t *)sv.edicts;

	org = G_VECTOR(OFS_PARM0);
	rad = G_FLOAT(OFS_PARM1);

	ent = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		if (ent->v.solid == SOLID_NOT)
			continue;
//		leilei - unrolled
			eorg[0] = org[0] - (ent->v.origin[0] + (ent->v.mins[0] + ent->v.maxs[0])*0.5);
			eorg[1] = org[1] - (ent->v.origin[1] + (ent->v.mins[1] + ent->v.maxs[1])*0.5);
			eorg[2] = org[2] - (ent->v.origin[2] + (ent->v.mins[2] + ent->v.maxs[2])*0.5);
		if (Length(eorg) > rad)
			continue;

		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}

	RETURN_EDICT(chain);
}


/*
=========
PF_dprint
=========
*/
void PF_dprint (void)
{
	Con_DPrintf ("%s",PF_VarString(0));
}

// 2001-10-25 Enhanced temp string handling by Maddes  start
//char	pr_string_temp[128];
char	pr_string_temp[PR_MAX_TEMPSTRING];
// 2001-10-25 Enhanced temp string handling by Maddes  end

void PF_ftos (void)
{
	float	v;
	int	i;	// 2000-01-14 Maximum precision for FTOS by Maddes

	v = G_FLOAT(OFS_PARM0);

	if (v == (int)v)
		sprintf (pr_string_temp, "%d",(int)v);
	else
// 1999-07-25 FTOS fix by Maddes  start
// 2000-01-14 Maximum precision for FTOS by Maddes  start
	{
//		sprintf (pr_string_temp, "%5.1f",v);
		sprintf (pr_string_temp, "%1f", v);
		for (i=strlen(pr_string_temp)-1 ; i>0 && pr_string_temp[i]=='0' && pr_string_temp[i-1]!='.' ; i--)
		{
			pr_string_temp[i] = 0;
		}
	}
// 2000-01-14 Maximum precision for FTOS by Maddes  end
// 1999-07-25 FTOS fix by Maddes  end
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

void PF_fabs (void)
{
	float	v;
	v = G_FLOAT(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = fabs(v);
}

void PF_vtos (void)
{
	sprintf (pr_string_temp, "'%5.1f %5.1f %5.1f'", G_VECTOR(OFS_PARM0)[0], G_VECTOR(OFS_PARM0)[1], G_VECTOR(OFS_PARM0)[2]);
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

//#ifdef QUAKE2	// 2001-09-16 Quake 2 builtin functions by id/Maddes
void PF_etos (void)
{
	sprintf (pr_string_temp, "entity %i", G_EDICTNUM(OFS_PARM0));
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}
//#endif		// 2001-09-16 Quake 2 builtin functions by id/Maddes

void PF_Spawn (void)
{
	edict_t	*ed;
	ed = ED_Alloc();
	RETURN_EDICT(ed);
}

void PF_Remove (void)
{
	edict_t	*ed;

	ed = G_EDICT(OFS_PARM0);

	ED_Free (ed);
}


// entity (entity start, .string field, string match) find = #5;
void PF_Find (void)
#ifdef QUAKE2
{
	int		e;
	int		f;
	char	*s, *t;
	edict_t	*ed;
	edict_t	*first;
	edict_t	*second;
	edict_t	*last;

	first = second = last = (edict_t *)sv.edicts;
	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("PF_Find: bad search string");

	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_STRING(ed,f);
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			if (first == (edict_t *)sv.edicts)
				first = ed;
			else if (second == (edict_t *)sv.edicts)
				second = ed;
			ed->v.chain = EDICT_TO_PROG(last);
			last = ed;
		}
	}

	if (first != last)
	{
		if (last != second)
			first->v.chain = last->v.chain;
		else
			first->v.chain = EDICT_TO_PROG(last);
		last->v.chain = EDICT_TO_PROG((edict_t *)sv.edicts);
		if (second && second != last)
			second->v.chain = EDICT_TO_PROG(last);
	}
	RETURN_EDICT(first);
}
#else
{
	int		e;
	int		f;
	char	*s, *t;
	edict_t	*ed;

	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("PF_Find: bad search string");

	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_STRING(ed,f);
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			RETURN_EDICT(ed);
			return;
		}
	}

	RETURN_EDICT(sv.edicts);
}
#endif

void PR_CheckEmptyString (char *s)
{
	if (s[0] <= ' ')
		PR_RunError ("Bad string");
}

void PF_precache_file (void)
{	// precache_file is only used to copy files with qcc, it does nothing
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
}

void PF_precache_sound (void)
{
	char	*s;
	int		i;

	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

	for (i=0 ; i<MAX_SOUNDS ; i++)
	{
		if (!sv.sound_precache[i])
		{
			sv.sound_precache[i] = s;
			return;
		}
		if (!strcmp(sv.sound_precache[i], s))
			return;
	}
	PR_RunError ("PF_precache_sound: overflow");
}

void PF_precache_model (void)
{
	char	*s;
	int		i;

	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
		{
			sv.model_precache[i] = s;
			sv.models[i] = Mod_ForName (s, true);
			return;
		}
		if (!strcmp(sv.model_precache[i], s))
			return;
	}
	PR_RunError ("PF_precache_model: overflow");
}

// leilei - for debuggereeeng, plz don't use!
void THE_precache_model (char *barf)
{
	char	*s;
	int		i;

//	if (sv.state != ss_loading)
//		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");

	s = barf;
	
	PR_CheckEmptyString (s);

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
		{
			sv.model_precache[i] = s;
			sv.models[i] = Mod_ForName (s, true);
			//Sys_Error("it  load. %s. It is model %i. How bitchin is that!", s, i);
			return;
		}
		if (!strcmp(sv.model_precache[i], s)){
			//Sys_Error("it didn't load. %s", s);
			return;
		}
	}
	PR_RunError ("THE_precache_model: overflow");
}


void PF_coredump (void)
{
	ED_PrintEdicts ();
}

void PF_traceon (void)
{
	pr_trace = true;
}

void PF_traceoff (void)
{
	pr_trace = false;
}

void PF_eprint (void)
{
	ED_PrintNum (G_EDICTNUM(OFS_PARM0));
}

/*
===============
PF_walkmove

float(float yaw, float dist) walkmove
===============
*/
void PF_walkmove (void)
{
	edict_t	*ent;
	float	yaw, dist;
	vec3_t	move;
	dfunction_t	*oldf;
	int 	oldself;

	ent = PROG_TO_EDICT(pr_global_struct->self);
	yaw = G_FLOAT(OFS_PARM0);
	dist = G_FLOAT(OFS_PARM1);

	if ( !( (int)ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM) ) )
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	yaw = yaw*M_PI*2 / 360;

	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

// save program state, because SV_movestep may call other progs
	oldf = pr_xfunction;
	oldself = pr_global_struct->self;

	G_FLOAT(OFS_RETURN) = SV_movestep(ent, move, true);


// restore program state
	pr_xfunction = oldf;
	pr_global_struct->self = oldself;
}

/*
===============
PF_droptofloor

void() droptofloor
===============
*/
void PF_droptofloor (void)
{
	edict_t		*ent;
	vec3_t		end;
	trace_t		trace;

	ent = PROG_TO_EDICT(pr_global_struct->self);

	VectorCopy (ent->v.origin, end);
	end[2] -= 256;

	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

	if (trace.fraction == 1 || trace.allsolid)
		G_FLOAT(OFS_RETURN) = 0;
	else
	{
		VectorCopy (trace.endpos, ent->v.origin);
		SV_LinkEdict (ent, false);
		ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
		ent->v.groundentity = EDICT_TO_PROG(trace.ent);
		G_FLOAT(OFS_RETURN) = 1;
	}
}

/*
===============
PF_lightstyle

void(float style, string value) lightstyle
===============
*/
void PF_lightstyle (void)
{
	int		style;
	char	*val;
	client_t	*client;
	int			j;

	style = G_FLOAT(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

// change the string in sv
	sv.lightstyles[style] = val;

// send message to all clients on this server
	if (sv.state != ss_active)
		return;

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
		if (client->active || client->spawned)
		{
			MSG_WriteChar (&client->message, svc_lightstyle);
			MSG_WriteChar (&client->message,style);
			MSG_WriteString (&client->message, val);
		}
}

void PF_rint (void)
{
	float	f;
	f = G_FLOAT(OFS_PARM0);
	if (f > 0)
		G_FLOAT(OFS_RETURN) = (int)(f + 0.5);
	else
		G_FLOAT(OFS_RETURN) = (int)(f - 0.5);
}
void PF_floor (void)
{
	G_FLOAT(OFS_RETURN) = floor(G_FLOAT(OFS_PARM0));
}
void PF_ceil (void)
{
	G_FLOAT(OFS_RETURN) = ceil(G_FLOAT(OFS_PARM0));
}


/*
=============
PF_checkbottom
=============
*/
void PF_checkbottom (void)
{
	edict_t	*ent;

	ent = G_EDICT(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_CheckBottom (ent);
}

/*
=============
PF_pointcontents
=============
*/
void PF_pointcontents (void)
{
	float	*v;

	v = G_VECTOR(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_PointContents (v);
}

/*
=============
PF_nextent

entity nextent(entity)
=============
*/
void PF_nextent (void)
{
	int		i;
	edict_t	*ent;

	i = G_EDICTNUM(OFS_PARM0);
	while (1)
	{
		i++;
		if (i == sv.num_edicts)
		{
			RETURN_EDICT(sv.edicts);
			return;
		}
		ent = EDICT_NUM(i);
		if (!ent->free)
		{
			RETURN_EDICT(ent);
			return;
		}
	}
}

/*
=============
PF_aim

Pick a vector for the player to shoot along
vector aim(entity, missilespeed)
=============
*/
cvar_t	*sv_aim;
extern float gunaimtime;
void PF_aim (void)
{
	edict_t	*ent, *check, *bestent;
	vec3_t	start, dir, end, bestdir;
	vec3_t	aimang;
	int		i;
	trace_t	tr;
	float	dist, bestdist;
//	float	speed;	// 2001-12-10 Reduced compiler warnings by Jeff Ford
// debug
	extern particle_t	*active_particles, *free_particles;
	particle_t	*p;

	ent = G_EDICT(OFS_PARM0);
//	speed = G_FLOAT(OFS_PARM1);	// 2001-12-10 Reduced compiler warnings by Jeff Ford

	VectorCopy (ent->v.origin, start);
	start[2] += 20;

// try sending a trace straight
	VectorCopy (pr_global_struct->v_forward, dir);
	VectorMA (start, 2048, dir, end);
	tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
	if (tr.ent && tr.ent->v.takedamage == DAMAGE_AIM
	&& (!teamplay->value || ent->v.team <=0 || ent->v.team != tr.ent->v.team) )
	{
		VectorCopy (pr_global_struct->v_forward, G_VECTOR(OFS_RETURN));
		return;
	}


// try all possible entities
	VectorCopy (dir, bestdir);
	bestdist = sv_aim->value;
	bestent = NULL;

	check = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, check = NEXT_EDICT(check) )
	{
		if (check->v.takedamage != DAMAGE_AIM)
			continue;
		if (check == ent)
			continue;
		if (teamplay->value && ent->v.team > 0 && ent->v.team == check->v.team)
			continue;	// don't aim at teammate
		// leilei - unrolled
			end[0] = check->v.origin[0]+ 0.5*(check->v.mins[0] + check->v.maxs[0]);
			end[1] = check->v.origin[1]+ 0.5*(check->v.mins[1] + check->v.maxs[1]);
			end[2] = check->v.origin[2]+ 0.5*(check->v.mins[2] + check->v.maxs[2]);
		VectorSubtract (end, start, dir);
		VectorNormalize (dir);
		dist = DotProduct (dir, pr_global_struct->v_forward);
		if (dist < bestdist)
			continue;	// to far to turn
		tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
		if (tr.ent == check)
		{	// can shoot at this one
			bestdist = dist;
			bestent = check;
		}
	}

	if (bestent)
	{
		VectorSubtract (bestent->v.origin, ent->v.origin, dir);
		dist = DotProduct (dir, pr_global_struct->v_forward);
		VectorScale (pr_global_struct->v_forward, dist, end);
		end[2] = dir[2];

		
		VectorNormalize (end);
		VectorCopy (end, G_VECTOR(OFS_RETURN));

		VectorCopy (end, aimang);
	//	VectorSubtract (end, dir, aimang);
		VectorNormalize(aimang);
	}
	else
	{
		VectorCopy (bestdir, G_VECTOR(OFS_RETURN));
		VectorCopy (bestdir, aimang);
	//	VectorSubtract (bestdir, dir, aimang);
	}

#ifdef LOOKANGLE
	// leilei - try to hack it in here...
	{
		vec3_t	sem;
		int eh;
		VectorNormalize(dir);
		VectorNormalize(bestdir);
		VectorSubtract(dir, bestdir, sem);
		//VectorNormalize(sem);
		for (eh=0; eh<3; eh++){
	//		if (sem[eh] < 2 && sem[eh] > -2)
		//		sem[eh] = 0;
				
		}
		
	gunaimtime = host_frametime; // set the last time we aimed the gun (for interpolation)
	Con_DPrintf ("%f %f %f AIM\n", aimang[0], aimang[1], aimang[2]);
	Con_DPrintf ("%f %f %f DIR\n", dir[0], dir[1], dir[2]);
	Con_DPrintf ("%f %f %f BESTDIR\n", bestdir[0], bestdir[1], bestdir[2]);
	Con_DPrintf ("%f %f %f START\n", start[0], start[1], start[2]);
	cl.aimangle[0] = sem[0];
	cl.aimangle[1] = sem[1];
	cl.aimangle[2] = sem[2];
	}
#endif
}

/*
==============
PF_changeyaw

This was a major timewaster in progs, so it was converted to C
==============
*/
void PF_changeyaw (void)
{
	edict_t		*ent;
	float		ideal, current, move, speed;

	ent = PROG_TO_EDICT(pr_global_struct->self);
	current = anglemod( ent->v.angles[1] );
	ideal = ent->v.ideal_yaw;
	speed = ent->v.yaw_speed;

	if (current == ideal)
		return;
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->v.angles[1] = anglemod (current + move);
}

//#ifdef QUAKE2	// 2001-09-16 Quake 2 builtin functions by id/Maddes
/*
==============
PF_changepitch
==============
*/
void PF_changepitch (void)
{
	edict_t		*ent;
	float		ideal, current, move, speed;
	eval_t		*val;	// 2001-09-16 PF_changepitch entity check by LordHavoc

	ent = G_EDICT(OFS_PARM0);
	current = anglemod( ent->v.angles[0] );
#ifdef QUAKE2	// 2001-09-16 PF_changepitch entity check by LordHavoc
	ideal = ent->v.idealpitch;
	speed = ent->v.pitch_speed;
// 2001-09-16 PF_changepitch entity check by LordHavoc  start
#else
// 2001-11-15 Better GetEdictFieldValue performance by LordHavoc/Maddes  start
//	val = GetEdictFieldValue(ent, "idealpitch");
	val = GETEDICTFIELDVALUE(ent, pr_field_idealpitch);
// 2001-11-15 Better GetEdictFieldValue performance by LordHavoc/Maddes  end
	if (val)
		ideal = val->_float;
	else
	{
		PR_RunError ("PF_changepitch: .float idealpitch and .float pitch_speed must be defined to use changepitch");
		return;
	}
// 2001-11-15 Better GetEdictFieldValue performance by LordHavoc/Maddes  start
//	val = GetEdictFieldValue(ent, "pitch_speed");
	val = GETEDICTFIELDVALUE(ent, pr_field_pitch_speed);
// 2001-11-15 Better GetEdictFieldValue performance by LordHavoc/Maddes  end
	if (val)
		speed = val->_float;
	else
	{
		PR_RunError ("PF_changepitch: .float idealpitch and .float pitch_speed must be defined to use changepitch");
		return;
	}
#endif
// 2001-09-16 PF_changepitch entity check by LordHavoc  end

	if (current == ideal)
		return;
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->v.angles[0] = anglemod (current + move);
}
//#endif	// 2001-09-16 Quake 2 builtin functions by id/Maddes

/*
===============================================================================

MESSAGE WRITING

===============================================================================
*/

// 2000-05-02 NVS SVC by Maddes  start
/*
#define	MSG_BROADCAST	0		// unreliable to all
#define	MSG_ONE			1		// reliable to one (msg_entity)
#define	MSG_ALL			2		// reliable to all
#define	MSG_INIT		3		// write to the init string
*/
// 2000-05-02 NVS SVC by Maddes  end

sizebuf_t *WriteDest (void)
{
	int		entnum;
	int		dest;
	edict_t	*ent;

	dest = G_FLOAT(OFS_PARM0);
	switch (dest)
	{
	case MSG_BROADCAST:
		return &sv.datagram;

	case MSG_ONE:
		ent = PROG_TO_EDICT(pr_global_struct->msg_entity);
		entnum = NUM_FOR_EDICT(ent);
		if (entnum < 1 || entnum > svs.maxclients)
			PR_RunError ("WriteDest: not a client");
		return &svs.clients[entnum-1].message;

	case MSG_ALL:
		return &sv.reliable_datagram;

	case MSG_INIT:
		return &sv.signon;

	default:
		PR_RunError ("WriteDest: bad destination");
		break;
	}

	return NULL;
}

void PF_WriteByte (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteByte (G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteByte (WriteDest(), G_FLOAT(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}

void PF_WriteChar (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteChar (G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteChar (WriteDest(), G_FLOAT(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}

void PF_WriteShort (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
	// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteShort (G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteShort (WriteDest(), G_FLOAT(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}

void PF_WriteLong (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
	// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteLong (G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteLong (WriteDest(), G_FLOAT(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}

void PF_WriteAngle (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
	// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteAngle (G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteAngle (WriteDest(), G_FLOAT(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}

void PF_WriteCoord (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
	// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteCoord (G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteCoord (WriteDest(), G_FLOAT(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}

void PF_WriteString (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
	// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteString (G_FLOAT(OFS_PARM0), G_STRING(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteString (WriteDest(), G_STRING(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}


void PF_WriteEntity (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
	// 2000-05-02 NVS SVC by Maddes  start
	if (sv.nvs_msgwrites)
	{
		NVS_WriteShort (G_FLOAT(OFS_PARM0), G_EDICTNUM(OFS_PARM1), NULL);
	}
	else
	{
// 2000-05-02 NVS SVC by Maddes  end
		MSG_WriteShort (WriteDest(), G_EDICTNUM(OFS_PARM1));
	}			// 2000-05-02 NVS SVC by Maddes
}

// 2001-09-16 New BuiltIn Function: WriteFloat() by Maddes  start
/*
PF_WriteFloat

void (float to, float f) WriteFloat
*/
void PF_WriteFloat (void)
{
#ifdef GLOBOT
		edict_t	*ent = PROG_TO_EDICT(pr_global_struct->msg_entity);

	if (G_FLOAT(OFS_PARM0) == MSG_ONE && ent->bot.isbot)
		return;
#endif
	if (sv.nvs_msgwrites)
	{
		NVS_WriteFloat (G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), NULL);
	}
	else
	{
		MSG_WriteFloat (WriteDest(), G_FLOAT(OFS_PARM1));
	}
}
// 2001-09-16 New BuiltIn Function: WriteFloat() by Maddes  end

//=============================================================================

int SV_ModelIndex (char *name);

void PF_makestatic (void)
{
	edict_t	*ent;
	int		i;

	ent = G_EDICT(OFS_PARM0);

	MSG_WriteByte (&sv.signon,svc_spawnstatic);

	MSG_WriteByte (&sv.signon, SV_ModelIndex(pr_strings + ent->v.model));

	MSG_WriteByte (&sv.signon, ent->v.frame);
	MSG_WriteByte (&sv.signon, ent->v.colormap);
	MSG_WriteByte (&sv.signon, ent->v.skin);
	// leilei - unrolled

		MSG_WriteCoord(&sv.signon, ent->v.origin[0]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[0]);
		MSG_WriteCoord(&sv.signon, ent->v.origin[1]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[1]);
		MSG_WriteCoord(&sv.signon, ent->v.origin[2]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[2]);

#ifdef ALPHASCALE

if(dpprotocol)
		{
		float	alpha=1;
		float	glowcolor = 0;
		float	glowsize = 0;

		float	scale = 1;
		int		bits=0;
		eval_t  *val;

		
		if (val = GETEDICTFIELDVALUE(ent, pr_field_alpha)  )
				alpha = val->_float;

		if (alpha < 1)
				bits |= U_ALPHA;

		MSG_WriteLong(&sv.signon, bits);
		
		if (bits & U_ALPHA)
			MSG_WriteFloat (&sv.signon, alpha);
		if (bits & U_GLOWSIZE)
			MSG_WriteFloat (&sv.signon, glowsize);
		if (bits & U_GLOWCOLOR)
			MSG_WriteFloat (&sv.signon, glowcolor);
		MSG_WriteShort(&sv.signon, ent->v.effects);

		}

#endif


// throw the entity away now
	ED_Free (ent);
}

//=============================================================================

/*
==============
PF_setspawnparms
==============
*/
void PF_setspawnparms (void)
{
	edict_t	*ent;
	int		i;
	client_t	*client;
#ifdef GLOBOT
	ent = G_EDICT(OFS_PARM0);	

	if (!ent->bot.isbot)
	{

#endif
	ent = G_EDICT(OFS_PARM0);
	i = NUM_FOR_EDICT(ent);
	if (i < 1 || i > svs.maxclients)
		PR_RunError ("Entity is not a client");

	// copy spawn parms out of the client_t
	client = svs.clients + (i-1);

	for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
		(&pr_global_struct->parm1)[i] = client->spawn_parms[i];
#ifdef GLOBOT
		}
#endif
}

/*
==============
PF_changelevel
==============
*/
void PF_changelevel (void)
{
#ifdef QUAKE2
	char	*s1, *s2;

	if (svs.changelevel_issued)
		return;
	svs.changelevel_issued = true;

	s1 = G_STRING(OFS_PARM0);
	s2 = G_STRING(OFS_PARM1);

	if ((int)pr_global_struct->serverflags & (SFL_NEW_UNIT | SFL_NEW_EPISODE))
		Cbuf_AddText (va("changelevel %s %s\n",s1, s2));
	else
		Cbuf_AddText (va("changelevel2 %s %s\n",s1, s2));
#else
	char	*s;

// make sure we don't issue two changelevels
	if (svs.changelevel_issued)
		return;
	svs.changelevel_issued = true;

	s = G_STRING(OFS_PARM0);
	Cbuf_AddText (va("changelevel %s\n",s));
#endif
}


#ifdef QUAKE2

#define	CONTENT_WATER	-3
#define CONTENT_SLIME	-4
#define CONTENT_LAVA	-5

#define FL_IMMUNE_WATER	131072
#define	FL_IMMUNE_SLIME	262144
#define FL_IMMUNE_LAVA	524288

#define	CHAN_VOICE	2
#define	CHAN_BODY	4

#define	ATTN_NORM	1

void PF_WaterMove (void)
{
	edict_t		*self;
	int			flags;
	int			waterlevel;
	int			watertype;
	float		drownlevel;
	float		damage = 0.0;

	self = PROG_TO_EDICT(pr_global_struct->self);

	if (self->v.movetype == MOVETYPE_NOCLIP)
	{
		self->v.air_finished = sv.time + 12;
		G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (self->v.health < 0)
	{
		G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (self->v.deadflag == DEAD_NO)
		drownlevel = 3;
	else
		drownlevel = 1;

	flags = (int)self->v.flags;
	waterlevel = (int)self->v.waterlevel;
	watertype = (int)self->v.watertype;

	if (!(flags & (FL_IMMUNE_WATER + FL_GODMODE)))
		if (((flags & FL_SWIM) && (waterlevel < drownlevel)) || (waterlevel >= drownlevel))
		{
			if (self->v.air_finished < sv.time)
				if (self->v.pain_finished < sv.time)
				{
					self->v.dmg = self->v.dmg + 2;
					if (self->v.dmg > 15)
						self->v.dmg = 10;
//					T_Damage (self, world, world, self.dmg, 0, FALSE);
					damage = self->v.dmg;
					self->v.pain_finished = sv.time + 1.0;
				}
		}
		else
		{
			if (self->v.air_finished < sv.time)
//				sound (self, CHAN_VOICE, "player/gasp2.wav", 1, ATTN_NORM);
				SV_StartSound (self, CHAN_VOICE, "player/gasp2.wav", 255, ATTN_NORM);
			else if (self->v.air_finished < sv.time + 9)
//				sound (self, CHAN_VOICE, "player/gasp1.wav", 1, ATTN_NORM);
				SV_StartSound (self, CHAN_VOICE, "player/gasp1.wav", 255, ATTN_NORM);
			self->v.air_finished = sv.time + 12.0;
			self->v.dmg = 2;
		}

	if (!waterlevel)
	{
		if (flags & FL_INWATER)
		{
			// play leave water sound
//			sound (self, CHAN_BODY, "misc/outwater.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "misc/outwater.wav", 255, ATTN_NORM);
			self->v.flags = (float)(flags &~FL_INWATER);
		}
		self->v.air_finished = sv.time + 12.0;
		G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (watertype == CONTENT_LAVA)
	{	// do damage
		if (!(flags & (FL_IMMUNE_LAVA + FL_GODMODE)))
			if (self->v.dmgtime < sv.time)
			{
				if (self->v.radsuit_finished < sv.time)
					self->v.dmgtime = sv.time + 0.2;
				else
					self->v.dmgtime = sv.time + 1.0;
//				T_Damage (self, world, world, 10*self.waterlevel, 0, TRUE);
				damage = (float)(10*waterlevel);
			}
	}
	else if (watertype == CONTENT_SLIME)
	{	// do damage
		if (!(flags & (FL_IMMUNE_SLIME + FL_GODMODE)))
			if (self->v.dmgtime < sv.time && self->v.radsuit_finished < sv.time)
			{
				self->v.dmgtime = sv.time + 1.0;
//				T_Damage (self, world, world, 4*self.waterlevel, 0, TRUE);
				damage = (float)(4*waterlevel);
			}
	}

	if ( !(flags & FL_INWATER) )
	{

// player enter water sound
		if (watertype == CONTENT_LAVA)
//			sound (self, CHAN_BODY, "player/inlava.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "player/inlava.wav", 255, ATTN_NORM);
		if (watertype == CONTENT_WATER)
//			sound (self, CHAN_BODY, "player/inh2o.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "player/inh2o.wav", 255, ATTN_NORM);
		if (watertype == CONTENT_SLIME)
//			sound (self, CHAN_BODY, "player/slimbrn2.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "player/slimbrn2.wav", 255, ATTN_NORM);

		self->v.flags = (float)(flags | FL_INWATER);
		self->v.dmgtime = 0;
	}

	if (! (flags & FL_WATERJUMP) )
	{
//		self.velocity = self.velocity - 0.8*self.waterlevel*frametime*self.velocity;
		VectorMA (self->v.velocity, -0.8 * self->v.waterlevel * host_frametime, self->v.velocity, self->v.velocity);
	}

	G_FLOAT(OFS_RETURN) = damage;
}
#endif	// 2001-09-16 Quake 2 builtin functions by id/Maddes

void PF_sin (void)
{
	G_FLOAT(OFS_RETURN) = sin(G_FLOAT(OFS_PARM0));
}

void PF_cos (void)
{
	G_FLOAT(OFS_RETURN) = cos(G_FLOAT(OFS_PARM0));
}

void PF_sqrt (void)
{
	G_FLOAT(OFS_RETURN) = sqrt(G_FLOAT(OFS_PARM0));
}
//#endif	// 2001-09-16 Quake 2 builtin functions by id/Maddes

void PF_Fixme (void)
{
	PR_RunError ("unimplemented builtin");	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes
}

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start
/*
=================
PF_builtin_find

float builtin_find (string)
=================
*/
void PF_builtin_find (void)
{
	int		j;
	float	funcno;
	char	*funcname;

	funcno = 0;
	funcname = G_STRING(OFS_PARM0);

	// search function name
	for ( j=1 ; j < pr_ebfs_numbuiltins ; j++)
	{
		if ((pr_ebfs_builtins[j].funcname) && (!(Q_strcasecmp(funcname,pr_ebfs_builtins[j].funcname))))
		{
			break;	// found
		}
	}

	if (j < pr_ebfs_numbuiltins)
	{
		funcno = pr_ebfs_builtins[j].funcno;
	}

	G_FLOAT(OFS_RETURN) = funcno;
}
// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end

// 2001-09-16 New BuiltIn Function: cmd_find() by Maddes  start
/*
=================
PF_cmd_find

float cmd_find (string)
=================
*/
void PF_cmd_find (void)
{
	char	*cmdname;
	float	result;

	cmdname = G_STRING(OFS_PARM0);

	result = Cmd_Exists (cmdname);

	G_FLOAT(OFS_RETURN) = result;
}
// 2001-09-16 New BuiltIn Function: cmd_find() by Maddes  end

// 2001-09-16 New BuiltIn Function: cvar_find() by Maddes  start
/*
=================
PF_cvar_find

float cvar_find (string)
=================
*/
void PF_cvar_find (void)
{
	char	*varname;
	float	result;

	varname = G_STRING(OFS_PARM0);

	result = 0;
	if (Cvar_FindVar (varname))
	{
		result = 1;
	}

	G_FLOAT(OFS_RETURN) = result;
}
// 2001-09-16 New BuiltIn Function: cvar_find() by Maddes  end

// 2001-09-16 New BuiltIn Function: cvar_string() by Maddes  start
/*
=================
PF_cvar_string

string cvar_string (string)
=================
*/
void PF_cvar_string (void)
{
	char	*varname;
	cvar_t	*var;

	varname = G_STRING(OFS_PARM0);
	var = Cvar_FindVar (varname);
	if (!var)
	{
		Con_DPrintf ("Cvar_String: variable \"%s\" not found\n", varname);	// 2001-09-09 Made 'Cvar not found' a developer message by Maddes
		G_INT(OFS_RETURN) = OFS_NULL;
	}
	else
	{
		G_INT(OFS_RETURN) = var->string - pr_strings;
	}
}
// 2001-09-16 New BuiltIn Function: cvar_string() by Maddes  end

// 2001-09-18 New BuiltIn Function: cvar_create() by Maddes  start
#define CVAR_QC_NONE		0		//cvar has no flags
#define CVAR_QC_ARCHIVE		1		//cvar will be stored in config.cfg
#define CVAR_QC_ROM			2		//cvar is readonly
#define CVAR_QC_NOTIFY		4		//cvar changes will be broadcasted to all players
#define CVAR_QC_SERVERINFO	8		//cvar will be send to clients (net_dgrm.c)
#define CVAR_QC_USERINFO	16		//cvar will be send to server (QW-like)

/*
=================
PF_cvar_create

void cvar_create (string, string, float)
=================
*/
void PF_cvar_create (void)
{
	char	*varname;
	int		qc_flags;
	int		flags;
	cvar_t	*var;

	varname = G_STRING(OFS_PARM0);
	qc_flags = G_FLOAT(OFS_PARM2);

	// convert QC flags to engine flags
	flags = CVAR_NONE;
	if (qc_flags & CVAR_QC_ARCHIVE)
	{
		flags |= CVAR_ARCHIVE;
	}
	if (qc_flags & CVAR_QC_ROM)
	{
		flags |= CVAR_ROM;
	}
	if (qc_flags & CVAR_QC_NOTIFY)
	{
		flags |= CVAR_NOTIFY;
	}
	if (qc_flags & CVAR_QC_SERVERINFO)
	{
		flags |= CVAR_SERVERINFO;
	}
	if (qc_flags & CVAR_QC_USERINFO)
	{
		flags |= CVAR_QC_USERINFO;
	}
	flags |= CVAR_PROGS_CREATED;

	var = Cvar_FindVar (varname);
	if (!var)
	{
		var = Cvar_Get (varname, G_STRING(OFS_PARM1), flags);
		return;
	}

	if (!(var->flags & (CVAR_USER_CREATED|CVAR_PROGS_CREATED)))
	{
		Con_DPrintf ("Cvar_Create: variable \"%s\" is not progs or user created\n", var->name);
		return;
	}

	// always throw out flags
	if (!(flags & CVAR_ROM))	// keep ARCHIVE flag if not ROM
	{
		flags |= var->flags & CVAR_ARCHIVE;
	}
	var->flags = flags;
}
// 2001-09-18 New BuiltIn Function: cvar_create() by Maddes  end

// 2001-09-18 New BuiltIn Function: cvar_free() by Maddes  start
/*
=================
PF_cvar_free

void cvar_free (string)
=================
*/
void PF_cvar_free (void)
{
	char	*varname;
	cvar_t	*var;

	varname = G_STRING(OFS_PARM0);
	var = Cvar_FindVar (varname);
	if (!var)
	{
		Con_DPrintf ("Cvar_Free: variable \"%s\" not found\n", varname);	// 2001-09-09 Made 'Cvar not found' a developer message by Maddes
		return;
	}

	if (!(var->flags & (CVAR_USER_CREATED|CVAR_PROGS_CREATED)))
	{
		Con_DPrintf ("Cvar_Free: variable \"%s\" is not progs or user created\n", var->name);
		return;
	}

	var = Cvar_Free (var);
}
// 2001-09-18 New BuiltIn Function: cvar_free() by Maddes  end

// 2001-09-25 New BuiltIn Function: etof() by Maddes  start
/*
=================
PF_etof

float etof (entity)
=================
*/
void PF_etof (void)
{
	G_FLOAT(OFS_RETURN) = G_EDICTNUM(OFS_PARM0);
}
// 2001-09-25 New BuiltIn Function: etof() by Maddes  end

// 2001-09-25 New BuiltIn Function: ftoe() by Maddes  start
/*
=================
PF_ftoe

entity ftoe (float)
=================
*/
void PF_ftoe (void)
{
	edict_t		*e;

	e = EDICT_NUM(G_FLOAT(OFS_PARM0));
	RETURN_EDICT(e);
}
// 2001-09-25 New BuiltIn Function: ftoe() by Maddes  end

// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes  start
// 2001-09-20 QuakeC string zone by Maddes  start
/*
=================
PF_allocate_zone_progstrings
=================
*/
void PF_allocate_zone_progstrings (void)
{
	int	zonesize_progstrings;

	Cvar_Set(pr_zone_min_strings, pr_zone_min_strings->string);	// do rangecheck
	zonesize_progstrings = pr_zone_min_strings->value * 1024;
	zone_progstrings = Hunk_AllocName (zonesize_progstrings, "qcstrings");	// note only 8 chars copied
	Z_ClearZone (zone_progstrings, zonesize_progstrings);
}
// 2001-09-20 QuakeC string zone by Maddes  end

/*
=================
PF_strzone

string strzone (string)
=================
*/
void PF_strzone (void)
{
	char *m, *p;

// 2001-09-20 QuakeC string zone by Maddes  start
	if (!zone_progstrings)
	{
		PF_allocate_zone_progstrings();
	}
// 2001-09-20 QuakeC string zone by Maddes  end

	m = G_STRING(OFS_PARM0);
	p = Z_Malloc(zone_progstrings, strlen(m) + 1);	// 2001-09-20 QuakeC string zone by Maddes
	strcpy(p, m);

	G_INT(OFS_RETURN) = p - pr_strings;
}

/*
=================
PF_strunzone

string strunzone (string)
=================
*/
void PF_strunzone (void)
{
// 2001-09-20 QuakeC string zone by Maddes  start
	if (!zone_progstrings)
	{
		PF_allocate_zone_progstrings();
	}
// 2001-09-20 QuakeC string zone by Maddes  end

	Z_Free(zone_progstrings, G_STRING(OFS_PARM0));	// 2001-09-20 QuakeC string zone by Maddes
	G_INT(OFS_PARM0) = OFS_NULL; // empty the def
};

/*
=================
PF_strlen

float strlen (string)
=================
*/
void PF_strlen (void)
{
	char *p = G_STRING(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = strlen(p);
}

/*
=================
PF_strcat

string strcat (string, string)
=================
*/

//char pr_strcat_buf [128]; // 2001-10-25 Enhanced temp string handling by Maddes
							// need this because pr_string_temp sucks

void PF_strcat (void)
{
	char *s1, *s2;
	int		maxlen;	// 2001-10-25 Enhanced temp string handling by Maddes

//	memset(pr_strcat_buf, 0, 127);	// 2001-10-25 Enhanced temp string handling by Maddes
	s1 = G_STRING(OFS_PARM0);
	s2 = PF_VarString(1);

// 2001-10-25 Enhanced temp string handling by Maddes  start
//	strcpy(pr_strcat_buf, s1);
	pr_string_temp[0] = 0;
	if (strlen(s1) < PR_MAX_TEMPSTRING)
	{
		strcpy(pr_string_temp, s1);
	}
	else
	{
		strncpy(pr_string_temp, s1, PR_MAX_TEMPSTRING);
		pr_string_temp[PR_MAX_TEMPSTRING-1] = 0;
	}

//	strcat(pr_strcat_buf, s2);
	maxlen = PR_MAX_TEMPSTRING - strlen(pr_string_temp) - 1;	// -1 is EndOfString
	if (maxlen > 0)
	{
		if (maxlen > strlen(s2))
		{
			strcat (pr_string_temp, s2);
		}
		else
		{
			strncat (pr_string_temp, s2, maxlen);
			pr_string_temp[PR_MAX_TEMPSTRING-1] = 0;
		}
	}

//	G_INT(OFS_RETURN) = pr_strcat_buf - pr_strings;
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
// 2001-10-25 Enhanced temp string handling by Maddes  end
}

/*
=================
PF_substring

string substring (string, float, float)
=================
*/
void PF_substring (void)
{
	int		offset, length;
	int		maxoffset;		// 2001-10-25 Enhanced temp string handling by Maddes
	char	*p;

	p = G_STRING(OFS_PARM0);
	offset = (int)G_FLOAT(OFS_PARM1); // for some reason, Quake doesn't like G_INT
	length = (int)G_FLOAT(OFS_PARM2);

	// cap values
	maxoffset = strlen(p);
	if (offset > maxoffset)
	{
		offset = maxoffset;
	}
	if (offset < 0)
		offset = 0;
// 2001-10-25 Enhanced temp string handling by Maddes  start
	if (length >= PR_MAX_TEMPSTRING)
		length = PR_MAX_TEMPSTRING-1;
// 2001-10-25 Enhanced temp string handling by Maddes  end
	if (length < 0)
		length = 0;

	p += offset;
	strncpy(pr_string_temp, p, length);
	pr_string_temp[length]=0;

	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

/*
=================
PF_stof

float stof (string)
=================
*/
// thanks Zoid, taken from QuakeWorld
void PF_stof (void)
{
	char	*s;

	s = G_STRING(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = atof(s);
}

/*
=================
PF_stov

vector stov (string)
=================
*/
void PF_stov (void)
{
	char *v;
	int i;
	vec3_t d;

	v = G_STRING(OFS_PARM0);

	for (i=0; i<3; i++)
	{
		while(v && (v[0] == ' ' || v[0] == '\'')) //skip unneeded data
			v++;
		d[i] = atof(v);
		while (v && v[0] != ' ') // skip to next space
			v++;
	}
	VectorCopy (d, G_VECTOR(OFS_RETURN));
}
// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes  end

// 2001-09-20 QuakeC file access by FrikaC/Maddes  start
/*
=================
PF_fopen

float fopen (string,float)
=================
*/
void PF_fopen (void)
{
	char *p = G_STRING(OFS_PARM0);
	char *ftemp;
	int fmode = G_FLOAT(OFS_PARM1);
	int h = 0, fsize = 0;

	switch (fmode)
	{
		case 0: // read
			Sys_FileOpenRead (va("%s/%s",com_gamedir, p), &h);
			G_FLOAT(OFS_RETURN) = (float) h;
			return;
		case 1: // append -- this is nasty
			// copy whole file into the zone
			fsize = Sys_FileOpenRead(va("%s/%s",com_gamedir, p), &h);
			if (h == -1)
			{
				h = Sys_FileOpenWrite(va("%s/%s",com_gamedir, p));
				G_FLOAT(OFS_RETURN) = (float) h;
				return;
			}

// 2001-09-20 QuakeC string zone by Maddes  start
			if (!zone_progstrings)
			{
				PF_allocate_zone_progstrings();
			}
// 2001-09-20 QuakeC string zone by Maddes  end

			ftemp = Z_Malloc(zone_progstrings, fsize + 1);	// 2001-09-20 QuakeC string zone by Maddes
			Sys_FileRead(h, ftemp, fsize);
			Sys_FileClose(h);
			// spit it back out
			h = Sys_FileOpenWrite(va("%s/%s",com_gamedir, p));
			Sys_FileWrite(h, ftemp, fsize);
			Z_Free(zone_progstrings, ftemp); // free it from memory	// 2001-09-20 QuakeC string zone by Maddes
			G_FLOAT(OFS_RETURN) = (float) h;  // return still open handle
			return;
		default: // write
			h = Sys_FileOpenWrite (va("%s/%s", com_gamedir, p));
			G_FLOAT(OFS_RETURN) = (float) h;
			return;
	}
}

/*
=================
PF_fclose

void fclose (float)
=================
*/
void PF_fclose (void)
{
	int h = (int)G_FLOAT(OFS_PARM0);
	Sys_FileClose(h);
}

/*
=================
PF_fgets

string fgets (float)
=================
*/
void PF_fgets (void)
{
	// reads one line (up to a \n) into a string
	int		h;
	int		i;
	int		count;
	char	buffer;

	h = (int)G_FLOAT(OFS_PARM0);

	count = Sys_FileRead(h, &buffer, 1);
	if (count && buffer == '\r')	// carriage return
	{
		count = Sys_FileRead(h, &buffer, 1);	// skip
	}
	if (!count)	// EndOfFile
	{
		G_INT(OFS_RETURN) = OFS_NULL;	// void string
		return;
	}

	i = 0;
	while (count && buffer != '\n')
	{
		if (i < PR_MAX_TEMPSTRING-1)	// no place for character in temp string
		{
			pr_string_temp[i++] = buffer;
		}

		// read next character
		count = Sys_FileRead(h, &buffer, 1);
		if (count && buffer == '\r')	// carriage return
		{
			count = Sys_FileRead(h, &buffer, 1);	// skip
		}
	};
	pr_string_temp[i] = 0;

	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

/*
=================
PF_fputs

void fputs (float,string)
=================
*/
void PF_fputs (void)
{
	// writes to file, like bprint
	float handle = G_FLOAT(OFS_PARM0);
	char *str = PF_VarString(1);
	Sys_FileWrite (handle, str, strlen(str));
}
// 2001-09-20 QuakeC file access by FrikaC/Maddes  end

// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  start
/*
=================
PF_fmin

Returns the minimum of two or more supplied floats

float fmin(float f1, float f2, ...)
=================
*/
void PF_fmin (void)
{
	// LordHavoc: 3+ argument enhancement suggested by FrikaC
	if (pr_argc == 2)
		G_FLOAT(OFS_RETURN) = min(G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1));
	else if (pr_argc >= 3)
	{
		int i;
		float f = G_FLOAT(OFS_PARM0);
		for (i = 1;i < pr_argc;i++)
			if (G_FLOAT((OFS_PARM0+i*3)) < f)
				f = G_FLOAT((OFS_PARM0+i*3));
		G_FLOAT(OFS_RETURN) = f;
	}
	else
		PR_RunError("fmin: must supply at least 2 floats\n");
}

/*
=================
PF_fmax

Returns the maximum of two or more supplied floats

float fmax(float f1, float f2, ...)
=================
*/
void PF_fmax (void)
{
	// LordHavoc: 3+ argument enhancement suggested by FrikaC
	if (pr_argc == 2)
		G_FLOAT(OFS_RETURN) = max(G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1));
	else if (pr_argc >= 3)
	{
		int i;
		float f = G_FLOAT(OFS_PARM0);
		for (i = 1;i < pr_argc;i++)
			if (G_FLOAT((OFS_PARM0+i*3)) > f)
				f = G_FLOAT((OFS_PARM0+i*3));
		G_FLOAT(OFS_RETURN) = f;
	}
	else
		PR_RunError("fmax: must supply at least 2 floats\n");
}

/*
=================
PF_fbound

Returns number bounded by supplied range

float fbound(float min, float f, float max)
=================
*/
void PF_fbound (void)
{
	G_FLOAT(OFS_RETURN) = bound(G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), G_FLOAT(OFS_PARM2));
}

/*
=================
PF_fpow

Returns base raised to power exp (base^exp)

float fpow(float base, float exp)
=================
*/
void PF_fpow (void)
{
	G_FLOAT(OFS_RETURN) = pow(G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1));
}

/*
=================
PF_findfloat

Loops through all entities beginning with the "start" entity, and checks the named entity field for a match.

entity findfloat(entity start, .string fld, float match)
=================
*/
// LordHavoc: added this for searching float, int, and entity reference fields
void PF_FindFloat (void)
{
	int		e;
	int		f;
	float	s;
	edict_t	*ed;

	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_FLOAT(OFS_PARM2);

	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		if (E_FLOAT(ed,f) == s)
		{
			RETURN_EDICT(ed);
			return;
		}
	}

	RETURN_EDICT(sv.edicts);
}

/*
=================
PF_tracebox

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entities, and also slide box entities
if the tryents flag is set(?).

void(vector v1, vector mins, vector maxs, vector v2, float nomonsters, entity forent) tracebox
=================
*/
// LordHavoc: added this for my own use, VERY useful, similar to traceline
void PF_tracebox (void)
{
	float	*v1, *v2, *m1, *m2;
	trace_t	trace;
	int		nomonsters;
	edict_t	*ent;

	v1 = G_VECTOR(OFS_PARM0);
	m1 = G_VECTOR(OFS_PARM1);
	m2 = G_VECTOR(OFS_PARM2);
	v2 = G_VECTOR(OFS_PARM3);
	nomonsters = G_FLOAT(OFS_PARM4);
	ent = G_EDICT(OFS_PARM5);

	trace = SV_Move (v1, m1, m2, v2, nomonsters, ent);

	pr_global_struct->trace_allsolid = trace.allsolid;
	pr_global_struct->trace_startsolid = trace.startsolid;
	pr_global_struct->trace_fraction = trace.fraction;
	pr_global_struct->trace_inwater = trace.inwater;
	pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, pr_global_struct->trace_plane_normal);
	pr_global_struct->trace_plane_dist =  trace.plane.dist;
	if (trace.ent)
		pr_global_struct->trace_ent = EDICT_TO_PROG(trace.ent);
	else
		pr_global_struct->trace_ent = EDICT_TO_PROG(sv.edicts);
}

/*
=================
PF_randomvec

Returns a vector of length < 1

vector randomvec()
=================
*/
void PF_randomvec (void)
{
	vec3_t		temp;
	do
	{
		temp[0] = (rand()&32767) * (2.0 / 32767.0) - 1.0;
		temp[1] = (rand()&32767) * (2.0 / 32767.0) - 1.0;
		temp[2] = (rand()&32767) * (2.0 / 32767.0) - 1.0;
	}
	while (DotProduct(temp, temp) >= 1);
	VectorCopy (temp, G_VECTOR(OFS_RETURN));
}

// goldquake inheritance


void PF_min(void)
{
	if (pr_argc >= 3)
	{
		int i;
		float f = G_FLOAT(OFS_PARM0);
		for (i = 1; i < pr_argc; i++)
			if (f > G_FLOAT(OFS_PARM0+i*3))
				f = G_FLOAT(OFS_PARM0+i*3);
		G_FLOAT(OFS_RETURN) = f;
	}
	else
	{
		float a = G_FLOAT(OFS_PARM0);
		float b = G_FLOAT(OFS_PARM1);

		G_FLOAT(OFS_RETURN) = min(a, b);
	}
}

void PF_max(void)
{
	if (pr_argc >= 3)
	{
		int i;
		float f = G_FLOAT(OFS_PARM0);
		for (i = 1; i < pr_argc; i++)
			if (f < G_FLOAT(OFS_PARM0+i*3))
				f = G_FLOAT(OFS_PARM0+i*3);
		G_FLOAT(OFS_RETURN) = f;
	}
	else
	{
		float a = G_FLOAT(OFS_PARM0);
		float b = G_FLOAT(OFS_PARM1);

		G_FLOAT(OFS_RETURN) = max(a, b);
	}
}

void PF_bound(void)
{
	float xmin = G_FLOAT(OFS_PARM0);
	float x = G_FLOAT(OFS_PARM1);
	float xmax = G_FLOAT(OFS_PARM2);

	if (x < xmin)
		x = xmin;
	if (x > xmax)
		x = xmax;

	G_FLOAT(OFS_RETURN) = x;
}

void PF_te_blood(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_BLOOD);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
	MSG_WriteChar(&sv.datagram, bound(-128, (int)G_VECTOR(OFS_PARM1)[0], 127));
	MSG_WriteChar(&sv.datagram, bound(-128, (int)G_VECTOR(OFS_PARM1)[1], 127));
	MSG_WriteChar(&sv.datagram, bound(-128, (int)G_VECTOR(OFS_PARM1)[2], 127));
	MSG_WriteByte(&sv.datagram, bound(0, (int)G_FLOAT(OFS_PARM2), 255));
}

void PF_te_bloodshower(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_BLOODSHOWER);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[2]);
	MSG_WriteCoord(&sv.datagram, G_FLOAT(OFS_PARM2));
	MSG_WriteShort(&sv.datagram, (int)bound(0, G_FLOAT(OFS_PARM3), 65535));
}

void PF_te_gunshot(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_GUNSHOT);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_spike(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_SPIKE);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_superspike(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_SUPERSPIKE);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_explosion(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_EXPLOSION);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_tarexplosion(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_TAREXPLOSION);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_wizspike(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_WIZSPIKE);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_knightspike(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_KNIGHTSPIKE);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_lavasplash(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_LAVASPLASH);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_teleport(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_TELEPORT);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}

void PF_te_explosion2(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_EXPLOSION2);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
	MSG_WriteByte(&sv.datagram, (int)G_FLOAT(OFS_PARM1));
	MSG_WriteByte(&sv.datagram, (int)G_FLOAT(OFS_PARM2));
}

void PF_te_lightning1(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_LIGHTNING1);
	MSG_WriteShort(&sv.datagram, G_EDICTNUM(OFS_PARM0));
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[2]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[2]);
}

void PF_te_lightning2(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_LIGHTNING2);
	MSG_WriteShort(&sv.datagram, G_EDICTNUM(OFS_PARM0));
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[2]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[2]);
}

void PF_te_lightning3(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_LIGHTNING3);
	MSG_WriteShort(&sv.datagram, G_EDICTNUM(OFS_PARM0));
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[2]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[2]);
}

void PF_te_beam(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_BEAM);
	MSG_WriteShort(&sv.datagram, G_EDICTNUM(OFS_PARM0));
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM1)[2]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM2)[2]);
}

// !
/*
void PF_te_tei_bigexplosion(void)
{
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_TEI_BIG); // whoops, i stopped typing here
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
}
*/

/*
=================
PF_copyentity

copies data from one entity to another

copyentity(src, dst)
=================
*/
void PF_copyentity (void)
{
	edict_t *in, *out;
	in = G_EDICT(OFS_PARM0);
	out = G_EDICT(OFS_PARM1);
	memcpy(out, in, pr_edict_size);
}

/*
=================
PF_setcolor

sets the color of a client and broadcasts the update to all connected clients

setcolor(clientent, value)
=================
*/
void PF_setcolor (void)
{
	client_t	*client;
	int			entnum, i;

	entnum = G_EDICTNUM(OFS_PARM0);
	i = G_FLOAT(OFS_PARM1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_DPrintf ("PROGS.DAT tried to setcolor a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];
	client->colors = i;
	client->edict->v.team = (i & 15) + 1;

	MSG_WriteByte (&sv.reliable_datagram, svc_updatecolors);
	MSG_WriteByte (&sv.reliable_datagram, entnum - 1);
	MSG_WriteByte (&sv.reliable_datagram, i);
}

// chained search for strings in entity fields
// entity(.string field, string match) findchain = #402;
void PF_findchain (void)
{
	int		i;
	int		f;
	char	*s, *t;
	edict_t	*ent, *chain;

	chain = (edict_t *)sv.edicts;

	f = G_INT(OFS_PARM0);
	s = G_STRING(OFS_PARM1);
	if (!s || !s[0])
	{
		RETURN_EDICT(sv.edicts);
		return;
	}

	ent = NEXT_EDICT(sv.edicts);
	for (i = 1;i < sv.num_edicts;i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		t = E_STRING(ent,f);
		if (!t)
			continue;
		if (strcmp(t,s))
			continue;

		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}

	RETURN_EDICT(chain);
}

// LordHavoc: chained search for float, int, and entity reference fields
// entity(.string field, float match) findchainfloat = #403;
void PF_findchainfloat (void)
{
	int		i;
	int		f;
	float	s;
	edict_t	*ent, *chain;

	chain = (edict_t *)sv.edicts;

	f = G_INT(OFS_PARM0);
	s = G_FLOAT(OFS_PARM1);

	ent = NEXT_EDICT(sv.edicts);
	for (i = 1;i < sv.num_edicts;i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		if (E_FLOAT(ent,f) != s)
			continue;

		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}

	RETURN_EDICT(chain);
}
// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  end

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start
/*
builtin_t pr_builtin[] =
{
...
};
*/
builtin_t *pr_builtins;	// = pr_builtin;
int pr_numbuiltins;	// = sizeof(pr_builtin)/sizeof(pr_builtin[0]);

// for builtin function definitions see Quake Standards Group at http://www.quakesrc.org/
ebfs_builtin_t pr_ebfs_builtins[] =
{
	{   0, NULL, PF_Fixme },				// has to be first entry as it is needed for initialization in PR_LoadProgs()
	{   1, "makevectors", PF_makevectors },	// void(entity e)	makevectors 		= #1;
	{   2, "setorigin", PF_setorigin },		// void(entity e, vector o) setorigin	= #2;
	{   3, "setmodel", PF_setmodel },		// void(entity e, string m) setmodel	= #3;
	{   4, "setsize", PF_setsize },			// void(entity e, vector min, vector max) setsize = #4;
//	{   5, "fixme", PF_Fixme },				// void(entity e, vector min, vector max) setabssize = #5;
	{   6, "break", PF_break },				// void() break						= #6;
	{   7, "random", PF_random },			// float() random						= #7;
	{   8, "sound", PF_sound },				// void(entity e, float chan, string samp) sound = #8;
	{   9, "normalize", PF_normalize },		// vector(vector v) normalize			= #9;
	{  10, "error", PF_error },				// void(string e) error				= #10;
	{  11, "objerror", PF_objerror },		// void(string e) objerror				= #11;
	{  12, "vlen", PF_vlen },				// float(vector v) vlen				= #12;
	{  13, "vectoyaw", PF_vectoyaw },		// float(vector v) vectoyaw		= #13;
	{  14, "spawn", PF_Spawn },				// entity() spawn						= #14;
	{  15, "remove", PF_Remove },			// void(entity e) remove				= #15;
	{  16, "traceline", PF_traceline },		// float(vector v1, vector v2, float tryents) traceline = #16;
	{  17, "checkclient", PF_checkclient },	// entity() clientlist					= #17;
	{  18, "find", PF_Find },				// entity(entity start, .string fld, string match) find = #18;
	{  19, "precache_sound", PF_precache_sound },	// void(string s) precache_sound		= #19;
	{  20, "precache_model", PF_precache_model },	// void(string s) precache_model		= #20;
	{  21, "stuffcmd", PF_stuffcmd },		// void(entity client, string s)stuffcmd = #21;
	{  22, "findradius", PF_findradius },	// entity(vector org, float rad) findradius = #22;
	{  23, "bprint", PF_bprint },			// void(string s) bprint				= #23;
	{  24, "sprint", PF_sprint },			// void(entity client, string s) sprint = #24;
	{  25, "dprint", PF_dprint },			// void(string s) dprint				= #25;
	{  26, "ftos", PF_ftos },				// void(string s) ftos				= #26;
	{  27, "vtos", PF_vtos },				// void(string s) vtos				= #27;
	{  28, "coredump", PF_coredump },
	{  29, "traceon", PF_traceon },
	{  30, "traceoff", PF_traceoff },
	{  31, "eprint", PF_eprint },			// void(entity e) debug print an entire entity
	{  32, "walkmove", PF_walkmove },		// float(float yaw, float dist) walkmove
//	{  33, "fixme", PF_Fixme },				// float(float yaw, float dist) walkmove
	{  34, "droptofloor", PF_droptofloor },
	{  35, "lightstyle", PF_lightstyle },
	{  36, "rint", PF_rint },
	{  37, "floor", PF_floor },
	{  38, "ceil", PF_ceil },
//	{  39, "fixme", PF_Fixme },
	{  40, "checkbottom", PF_checkbottom },
	{  41, "pointcontents", PF_pointcontents },
//	{  42, "fixme", PF_Fixme },
	{  43, "fabs", PF_fabs },
	{  44, "aim", PF_aim },
	{  45, "cvar", PF_cvar },
	{  46, "localcmd", PF_localcmd },
	{  47, "nextent", PF_nextent },
	{  48, "particle", PF_particle },
	{  49, "ChangeYaw", PF_changeyaw },
//	{  50, "fixme", PF_Fixme },
	{  51, "vectoangles", PF_vectoangles },

	{  52, "WriteByte", PF_WriteByte },
	{  53, "WriteChar", PF_WriteChar },
	{  54, "WriteShort", PF_WriteShort },
	{  55, "WriteLong", PF_WriteLong },
	{  56, "WriteCoord", PF_WriteCoord },
	{  57, "WriteAngle", PF_WriteAngle },
	{  58, "WriteString", PF_WriteString },
	{  59, "WriteEntity", PF_WriteEntity },

// 2001-09-16 Quake 2 builtin functions by id/Maddes  start
//#ifdef QUAKE2
	{  60, "sin", PF_sin },
	{  61, "cos", PF_cos },
	{  62, "sqrt", PF_sqrt },
	{  63, "changepitch", PF_changepitch },
	{  64, "TraceToss", PF_TraceToss },
	{  65, "etos", PF_etos },
#ifdef QUAKE2
	{  66, "WaterMove", PF_WaterMove },
// 2001-09-16 Quake 2 builtin functions by id/Maddes  end
#endif

	{  67, "movetogoal", SV_MoveToGoal },
	{  68, "precache_file", PF_precache_file },
	{  69, "makestatic", PF_makestatic },

	{  70, "changelevel", PF_changelevel },
//	{  71, "fixme", PF_Fixme },

	{  72, "cvar_set", PF_cvar_set },
	{  73, "centerprint", PF_centerprint },

	{  74, "ambientsound", PF_ambientsound },

	{  75, "precache_model2", PF_precache_model },
	{  76, "precache_sound2", PF_precache_sound },	// precache_sound2 is different only for qcc
	{  77, "precache_file2", PF_precache_file },

	{  78, "setspawnparms", PF_setspawnparms },

	{  81, "stof", PF_stof },	// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes

// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  start
	{  90, "tracebox", PF_tracebox },
	{  91, "randomvec", PF_randomvec },
//	{  92, "getlight", PF_GetLight },	// not implemented yet
	{  93, "cvar_create", PF_cvar_create },		// 2001-09-18 New BuiltIn Function: cvar_create() by Maddes
	{  94, "fmin", PF_fmin },
	{  95, "fmax", PF_fmax },
	{  96, "fbound", PF_fbound },
	{  97, "fpow", PF_fpow },
	{  98, "findfloat", PF_FindFloat },
	{ PR_DEFAULT_FUNCNO_EXTENSION_FIND, "extension_find", PF_extension_find },	// 2001-10-20 Extension System by LordHavoc/Maddes
	{   0, "registercvar", PF_cvar_create },		// 0 indicates that this entry is just for remapping (because of name change)
	{   0, "checkextension", PF_extension_find },
// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  end

	{ PR_DEFAULT_FUNCNO_BUILTIN_FIND, "builtin_find", PF_builtin_find },		// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes

	{ 101, "cmd_find", PF_cmd_find },		// 2001-09-16 New BuiltIn Function: cmd_find() by Maddes

	{ 102, "cvar_find", PF_cvar_find },		// 2001-09-16 New BuiltIn Function: cvar_find() by Maddes

	{ 103, "cvar_string", PF_cvar_string },	// 2001-09-16 New BuiltIn Function: cvar_string() by Maddes

	{ 105, "cvar_free", PF_cvar_free },		// 2001-09-18 New BuiltIn Function: cvar_free() by Maddes

	{ 106, "NVS_InitSVCMsg", PF_NVS_InitSVCMsg },	// 2000-05-02 NVS SVC by Maddes

	{ 107, "WriteFloat", PF_WriteFloat },	// 2001-09-16 New BuiltIn Function: WriteFloat() by Maddes

	{ 108, "etof", PF_etof },	// 2001-09-25 New BuiltIn Function: etof() by Maddes

	{ 109, "ftoe", PF_ftoe },	// 2001-09-25 New BuiltIn Function: ftoe() by Maddes

// 2001-09-20 QuakeC file access by FrikaC/Maddes  start
	{ 110, "fopen", PF_fopen },
	{ 111, "fclose", PF_fclose },
	{ 112, "fgets", PF_fgets },
	{ 113, "fputs", PF_fputs },
	{   0, "open", PF_fopen },		// 0 indicates that this entry is just for remapping (because of name and number change)
	{   0, "close", PF_fclose },
	{   0, "read", PF_fgets },
	{   0, "write", PF_fputs },
// 2001-09-20 QuakeC file access by FrikaC/Maddes  end

// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes  start
	{ 114, "strlen", PF_strlen },
	{ 115, "strcat", PF_strcat },
	{ 116, "substring", PF_substring },
	{ 117, "stov", PF_stov },
	{ 118, "strzone", PF_strzone },
	{ 119, "strunzone", PF_strunzone },
	{   0, "zone", PF_strzone },		// 0 indicates that this entry is just for remapping (because of name and number change)
	{   0, "unzone", PF_strunzone },
// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes  end
	{ 322, "sound3", PF_sound3 },				// leilei - Sound Pitch Builtin

// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  start
	{ 400, "copyentity", PF_copyentity },
	{ 401, "setcolor", PF_setcolor },
	{ 402, "findchain", PF_findchain },
	{ 403, "findchainfloat", PF_findchainfloat },
	{ 404, "te_gunshot", PF_te_gunshot },	
	{ 405, "te_blood", PF_te_blood },				
	{ 406, "te_bloodshower", PF_te_bloodshower },				
	{ 411, "te_gunshot", PF_te_gunshot },	// te_spark
	{ 418, "te_gunshot", PF_te_gunshot },	
	{ 419, "te_spike", PF_te_spike },	
	{ 420, "te_superspike", PF_te_superspike },	
	{ 421, "te_explosion", PF_te_explosion },	
	{ 422, "te_tarexplosion", PF_te_tarexplosion },	
	{ 423, "te_wizspike", PF_te_wizspike },	
	{ 424, "te_knightspike", PF_te_knightspike },	
	{ 425, "te_lavasplash", PF_te_lavasplash },	
	{ 426, "te_teleport", PF_te_teleport },	
	{ 427, "te_explosion2", PF_te_explosion2 },	
	{ 428, "te_lightning1", PF_te_lightning1 },	
	{ 429, "te_lightning2", PF_te_lightning2 },	
	{ 430, "te_lightning3", PF_te_lightning3 },	
	{ 431, "te_beam", PF_te_beam },	
//	{ 432, "te_rain", PF_te_rain },	
//	{ 433, "te_snow", PF_te_snow },	
//	{ 433, "te_gunshot", PF_te_gunshot },	// te_plasmaburn
//	{ 433, "te_gunshot", PF_te_gunshot },	// te_plasmaburn
//	{ 433, "te_gunshot", PF_te_gunshot },	// te_plasmaburn
//	{ 432, "vectorvectors", PF_te_vectorvectors }







};

int pr_ebfs_numbuiltins = sizeof(pr_ebfs_builtins)/sizeof(pr_ebfs_builtins[0]);
// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end
