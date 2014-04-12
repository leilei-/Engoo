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
// sv_main.c -- server main program

#include "quakedef.h"

server_t		sv;
server_static_t	svs;

char	localmodels[MAX_MODELS][5];			// inline model names for precache

// 2001-09-20 Configurable entity limits by Maddes  start
cvar_t	*sv_entities;
cvar_t	*sv_entities_static;
cvar_t	*sv_entities_temp;
// 2001-09-20 Configurable entity limits by Maddes  end

cvar_t	*sv_standstill;
cvar_t	*sv_compatibility;	// 2001-12-24 Keeping full backwards compatibility by Maddes

//============================================================================

/*
===============
SV_Init
===============
*/
void SV_Init (void)
{
	int		i;
	extern cvar_t	*sv_maxvelocity;
	extern cvar_t	*sv_gravity;
	extern cvar_t	*sv_nostep;
	extern cvar_t	*sv_friction;
	extern cvar_t	*sv_edgefriction;
	extern cvar_t	*sv_stopspeed;
	extern cvar_t	*sv_maxspeed;
	extern cvar_t	*sv_accelerate;
	extern cvar_t	*sv_idealpitchscale;
	extern cvar_t	*sv_aim;

	sv_maxvelocity = Cvar_Get ("sv_maxvelocity", "2000", CVAR_ORIGINAL);
	sv_gravity = Cvar_Get ("sv_gravity", "800", CVAR_NOTIFY|CVAR_SERVERINFO|CVAR_ORIGINAL);
	sv_friction = Cvar_Get ("sv_friction", "4", CVAR_NOTIFY|CVAR_SERVERINFO|CVAR_ORIGINAL);
	sv_edgefriction = Cvar_Get ("edgefriction", "2", CVAR_ORIGINAL);
	sv_stopspeed = Cvar_Get ("sv_stopspeed", "100", CVAR_ORIGINAL);
	sv_maxspeed = Cvar_Get ("sv_maxspeed", "320", CVAR_NOTIFY|CVAR_SERVERINFO|CVAR_ORIGINAL);
	sv_accelerate = Cvar_Get ("sv_accelerate", "10", CVAR_ORIGINAL);
	sv_idealpitchscale = Cvar_Get ("sv_idealpitchscale", "0.8", CVAR_ORIGINAL);
	sv_aim = Cvar_Get ("sv_aim", "0.93", CVAR_ARCHIVE | CVAR_ORIGINAL);	// leilei - archiving now since it is annoying to set manually
	sv_nostep = Cvar_Get ("sv_nostep", "0", CVAR_ORIGINAL);
// 2001-09-20 Configurable entity limits by Maddes  start
#ifdef QSB
	sv_entities = Cvar_Get ("sv_entities", "8192", CVAR_NONE);
	Cvar_SetRangecheck (sv_entities, Cvar_RangecheckInt, MIN_EDICTS, MAX_EDICTS);
	Cvar_Set(sv_entities, sv_entities->string);	// do rangecheck

	sv_entities_static = Cvar_Get ("sv_entities_static", "1024", CVAR_NONE);
	Cvar_SetRangecheck (sv_entities_static, Cvar_RangecheckInt, MIN_STATIC_ENTITIES, MAX_EDICTS);
	Cvar_Set(sv_entities_static, sv_entities_static->string);	// do rangecheck

	sv_entities_temp = Cvar_Get ("sv_entities_temp", "1024", CVAR_NONE);
	Cvar_SetRangecheck (sv_entities_temp, Cvar_RangecheckInt, MIN_TEMP_ENTITIES, MAX_EDICTS);
	Cvar_Set(sv_entities_temp, sv_entities_temp->string);	// do rangecheck
#else
	sv_entities = Cvar_Get ("sv_entities", "0", CVAR_NONE);
	Cvar_SetRangecheck (sv_entities, Cvar_RangecheckInt, MIN_EDICTS, MAX_EDICTS);
	Cvar_Set(sv_entities, sv_entities->string);	// do rangecheck

	sv_entities_static = Cvar_Get ("sv_entities_static", "0", CVAR_NONE);
	Cvar_SetRangecheck (sv_entities_static, Cvar_RangecheckInt, MIN_STATIC_ENTITIES, MAX_EDICTS);
	Cvar_Set(sv_entities_static, sv_entities_static->string);	// do rangecheck

	sv_entities_temp = Cvar_Get ("sv_entities_temp", "0", CVAR_NONE);
	Cvar_SetRangecheck (sv_entities_temp, Cvar_RangecheckInt, MIN_TEMP_ENTITIES, MAX_EDICTS);
	Cvar_Set(sv_entities_temp, sv_entities_temp->string);	// do rangecheck
#endif
// 2001-09-20 Configurable entity limits by Maddes  end

// 2001-12-24 Keeping full backwards compatibility by Maddes  start
	sv_compatibility = Cvar_Get ("sv_compatibility", "0", CVAR_NONE);
	Cvar_SetRangecheck (sv_compatibility, Cvar_RangecheckBool, 0, 1);
	Cvar_SetDescription (sv_compatibility, "When set to 1, this server will not reply on enhanced client requests and will not allow any NVS enhanced messages (precise client aiming, etc.). If you just want to record demos that shall run on all Quake executables, then use CL_COMPATIBILITY instead of SV_COMPATIBILITY.");
	Cvar_Set(sv_compatibility, sv_compatibility->string);	// do rangecheck
// 2001-12-24 Keeping full backwards compatibility by Maddes  end

	
	sv_standstill = Cvar_Get ("sv_standstill", "0",CVAR_ORIGINAL);
	for (i=0 ; i<MAX_MODELS ; i++)
		sprintf (localmodels[i], "*%i", i);
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
==================
SV_StartParticle

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count)
{
	int	i, v;

// 2000-05-02 NVS SVC_particle by Maddes  start
/*
	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;
	MSG_WriteByte (&sv.datagram, svc_particle);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
*/
	NVS_InitSVCMsg(MSG_BROADCAST, svc_particle, 0, NULL);
	NVS_WriteByte (MSG_BROADCAST, svc_particle, NULL);
	NVS_WriteCoord (MSG_BROADCAST, org[0], NULL);
	NVS_WriteCoord (MSG_BROADCAST, org[1], NULL);
	NVS_WriteCoord (MSG_BROADCAST, org[2], NULL);
// 2000-05-02 NVS SVC_particle by Maddes  end
	for (i=0 ; i<3 ; i++)
	{
		v = dir[i]*16;
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
// 2000-05-02 NVS SVC_particle by Maddes  start
//		MSG_WriteChar (&sv.datagram, v);
		NVS_WriteChar (MSG_BROADCAST, v, NULL);
// 2000-05-02 NVS SVC_particle by Maddes  end
	}
// 2000-05-02 NVS SVC_particle by Maddes  start
/*
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, color);
*/
	NVS_WriteByte (MSG_BROADCAST, count, NULL);
	NVS_WriteByte (MSG_BROADCAST, color, NULL);
// 2000-05-02 NVS SVC_particle by Maddes  end
}

/*
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

==================
*/
void SV_StartSound (edict_t *entity, int channel, char *sample, int volume,
    float attenuation)
{
	int	sound_num;
	int	field_mask;
	int	i;
	int	ent;
	sfx_t	*sfx;
	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

// find precache number for sound
	for (sound_num=1 ; sound_num<MAX_SOUNDS
	&& sv.sound_precache[sound_num] ; sound_num++)
		if (!strcmp(sample, sv.sound_precache[sound_num]))
			break;

	if ( sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num] )
	{

	//	Con_Printf ("SV_StartSound: %s not precacheed\n", sample);
	
		
		// leilei - attempt to cache it

		{
					static int hash=345;
					int 	i;
					char name[256];

					sfx = S_PrecacheSound(sample);


		}
	
	}

	ent = NUM_FOR_EDICT(entity);

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;

// directed messages go only to the entity the are targeted on
	MSG_WriteByte (&sv.datagram, svc_sound);
	MSG_WriteByte (&sv.datagram, field_mask);
	if (field_mask & SND_VOLUME)
		MSG_WriteByte (&sv.datagram, volume);
	if (field_mask & SND_ATTENUATION)
		MSG_WriteByte (&sv.datagram, attenuation*64);
	MSG_WriteShort (&sv.datagram, channel);
	MSG_WriteByte (&sv.datagram, sound_num);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.datagram, entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i]));
}








/*
==================
SV_StartSound2

The sequel, now with pitch alteration, and sound flags reserved for THE FUTURE!
(to be implemented come 2012)

==================
*/
void SV_StartSound2 (edict_t *entity, int channel, char *sample, int volume, float attenuation, float pitch)
{
	int	sound_num;
	int	field_mask;
	int	i;
	int	ent;

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound2: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound2: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound2: channel = %i", channel);
	
	if (pitch < 3 || pitch > 255){
	Con_Printf ("SV_StartSound2: pitch = %f\n", pitch);
	pitch = 100;		// something messed up here?!
	}
	
	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

// find precache number for sound
	for (sound_num=1 ; sound_num<MAX_SOUNDS
	&& sv.sound_precache[sound_num] ; sound_num++)
		if (!strcmp(sample, sv.sound_precache[sound_num]))
			break;

	if ( sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num] )
	{
		Con_Printf ("SV_StartSound2: %s not precacheed\n", sample);
		return;
	}

	ent = NUM_FOR_EDICT(entity);

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;
	if (pitch != 100)
		field_mask |= SND_PITCH;
// directed messages go only to the entity the are targeted on
	MSG_WriteByte (&sv.datagram, svc_sound3);
	MSG_WriteByte (&sv.datagram, field_mask);
	if (field_mask & SND_VOLUME)
		MSG_WriteByte (&sv.datagram, volume);
	if (field_mask & SND_ATTENUATION)
		MSG_WriteByte (&sv.datagram, attenuation*64);
	MSG_WriteShort (&sv.datagram, channel);
	MSG_WriteByte (&sv.datagram, sound_num);
	if (field_mask & SND_PITCH)
	MSG_WriteByte (&sv.datagram, (int)pitch);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.datagram, entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i]));
		
}


/*
==============================================================================

CLIENT SPAWNING

==============================================================================
*/

/*
================
SV_SendServerinfo

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void SV_SendServerinfo (client_t *client)
{
	char			**s;
	char			message[2048];
#ifdef DPPROTOCOLS
	MSG_WriteByte (&client->message, svc_print);
	sprintf (message, "%c\nENGOO VERSION %4.2f SERVER (%i CRC)", 2, VERSION, pr_crc);
	MSG_WriteString (&client->message,message);

	MSG_WriteByte (&client->message, svc_serverinfo);
	MSG_WriteLong (&client->message, DPPROTOCOL_VERSION);
	MSG_WriteByte (&client->message, svs.maxclients);

#else
	MSG_WriteByte (&client->message, svc_print);
	sprintf (message, "%c\nVERSION %4.2f SERVER (%i CRC)\n", 2, VERSION, pr_crc);	// 2000-01-08 Missing linefeeds fix by Maddes
	MSG_WriteString (&client->message,message);

	MSG_WriteByte (&client->message, svc_serverinfo);
	MSG_WriteLong (&client->message, PROTOCOL_VERSION);
	MSG_WriteByte (&client->message, svs.maxclients);

#endif
	if (!coop->value && deathmatch->value)
		MSG_WriteByte (&client->message, GAME_DEATHMATCH);
	else
		MSG_WriteByte (&client->message, GAME_COOP);

	sprintf (message, pr_strings+sv.edicts->v.message);

	MSG_WriteString (&client->message,message);

	for (s = sv.model_precache+1 ; *s ; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

	for (s = sv.sound_precache+1 ; *s ; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

// send music
	MSG_WriteByte (&client->message, svc_cdtrack);
	MSG_WriteByte (&client->message, sv.edicts->v.sounds);
	MSG_WriteByte (&client->message, sv.edicts->v.sounds);

// set view
	MSG_WriteByte (&client->message, svc_setview);
	MSG_WriteShort (&client->message, NUM_FOR_EDICT(client->edict));

	MSG_WriteByte (&client->message, svc_signonnum);
	MSG_WriteByte (&client->message, 1);

	client->sendsignon = true;
	client->spawned = false;		// need prespawn, spawn, etc
}

/*
================
SV_ConnectClient

Initializes a client_t for a new net connection.  This will only be called
once for a player each game, not once for each level change.
================
*/
void SV_ConnectClient (int clientnum)
{
	edict_t			*ent;
	client_t		*client;
	int				edictnum;
	struct qsocket_s *netconnection;
	int				i;
	float			spawn_parms[NUM_SPAWN_PARMS];

	client = svs.clients + clientnum;

	Con_DPrintf ("Client %s connected\n", client->netconnection->address);

	edictnum = clientnum+1;

	ent = EDICT_NUM(edictnum);

// set up the client_t
	netconnection = client->netconnection;

	if (sv.loadgame)
		memcpy (spawn_parms, client->spawn_parms, sizeof(spawn_parms));
	memset (client, 0, sizeof(*client));
	client->netconnection = netconnection;

	strcpy (client->name, "unconnected");
	client->active = true;
	client->spawned = false;
	client->edict = ent;
	client->message.data = client->msgbuf;
	client->message.maxsize = sizeof(client->msgbuf);
	client->message.allowoverflow = true;		// we can catch it

// 2000-05-02 NVS SVC by Maddes  start
	SZ_Clear(&client->message);

	client->datagram.data = client->datagram_buf;
	client->datagram.maxsize = sizeof(client->datagram_buf);
	SZ_Clear(&client->datagram);

	client->nvs_msgconversion = NULL;
	client->nvs_msgignore = true;		// safety first
// 2000-05-02 NVS SVC by Maddes  end

// 2000-04-30 NVS HANDSHAKE SRV<->CL by Maddes  start
	client->nvs_cmax = 0;
	client->nvs_cclc = 0;
	client->nvs_csvc = 0;
// 2000-04-30 NVS HANDSHAKE SRV<->CL by Maddes  end

#ifdef IDGODS
	client->privileged = IsID(&client->netconnection->addr);
#else
	client->privileged = false;
#endif

	if (sv.loadgame)
		memcpy (client->spawn_parms, spawn_parms, sizeof(spawn_parms));
	else
	{
	// call the progs to get default spawn parms for the new client
		PR_ExecuteProgram (pr_global_struct->SetNewParms);
		for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
			client->spawn_parms[i] = (&pr_global_struct->parm1)[i];
	}

	SV_SendServerinfo (client);
}


/*
===================
SV_CheckForNewClients

===================
*/
void SV_CheckForNewClients (void)
{
	struct qsocket_s	*ret;
	int				i;

//
// check for new connections
//
	while (1)
	{
		ret = NET_CheckNewConnections ();
		if (!ret)
			break;

	//
	// init a new client structure
	//
		for (i=0 ; i<svs.maxclients ; i++)
			if (!svs.clients[i].active)
				break;
		if (i == svs.maxclients)
			Sys_Error ("Host_CheckForNewClients: no free clients");

		svs.clients[i].netconnection = ret;
		SV_ConnectClient (i);

		net_activeconnections++;
	}
}



/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

/*
==================
SV_ClearDatagram

==================
*/
void SV_ClearDatagram (void)
{
	SZ_Clear (&sv.datagram);
}

/*
=============================================================================

The PVS must include a small area around the client to allow head bobbing
or other small motion on the client side.  Otherwise, a bob might cause an
entity that should be visible to not show up, especially when the bob
crosses a waterline.

=============================================================================
*/

int		fatbytes;
byte	fatpvs[MAX_MAP_LEAFS/8];

void SV_AddToFatPVS (vec3_t org, mnode_t *node)
{
	int		i;
	byte	*pvs;
	mplane_t	*plane;
	float	d;

	while (1)
	{
	// if this is a leaf, accumulate the pvs bits
		if (node->contents < 0)
		{
			if (node->contents != CONTENTS_SOLID)
			{
				pvs = Mod_LeafPVS ( (mleaf_t *)node, sv.worldmodel);
				for (i=0 ; i<fatbytes ; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}

		plane = node->plane;
		d = DotProduct (org, plane->normal) - plane->dist;
		if (d > 8)
			node = node->children[0];
		else if (d < -8)
			node = node->children[1];
		else
		{	// go down both
			SV_AddToFatPVS (org, node->children[0]);
			node = node->children[1];
		}
	}
}

/*
=============
SV_FatPVS

Calculates a PVS that is the inclusive or of all leafs within 8 pixels of the
given point.
=============
*/
byte *SV_FatPVS (vec3_t org)
{
	fatbytes = (sv.worldmodel->numleafs+31)>>3;
	Q_memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVS (org, sv.worldmodel->nodes);
	return fatpvs;
}

//=============================================================================


/*
=============
SV_WriteEntitiesToClient

=============
*/
void SV_WriteEntitiesToClient (edict_t	*clent, sizebuf_t *msg)
{
	int		e, i;
	int		bits;
	byte	*pvs;
	vec3_t	org;
	float	miss;
	edict_t	*ent;
	// from tomazquake
	int		clentnum;
	float	alpha;
	float	glowcolor;
	float	glowsize;
	float	scale;
	eval_t  *val;
	// from tomazquake

	// from makaqu
#ifdef VMTOC
		
	qboolean viewmodel = false;
#endif
// find the client's PVS
	VectorAdd (clent->v.origin, clent->v.view_ofs, org);
	pvs = SV_FatPVS (org);

// send over all entities (excpet the client) that touch the pvs
	ent = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
	{
				// don't send if flagged for NODRAW and there are no lighting effects
		if (ent->v.effects == EF_NODRAW)
			continue;

#ifdef VMTOC
if ((val = GetEdictFieldValue(ent, "viewmodelforclient")) && val->_int)
		{
			if (val->_int == EDICT_TO_PROG(clent))
				viewmodel = true;
			else
				continue;
		}
		else
			viewmodel = false;
#endif
// ignore if not touching a PV leaf
		if (ent != clent)	// clent is always sent
		{
// ignore ents without visible models
			if (!ent->v.modelindex || !pr_strings[ent->v.model])
				continue;
#ifdef VMTOC
			
			if (!viewmodel)
			{
				for (i=0 ; i < ent->num_leafs ; i++)
					if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i]&7) ))
						break;
				
				if (i == ent->num_leafs)
					continue;		// not visible
			}
#else
			for (i=0 ; i < ent->num_leafs ; i++)
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i]&7) ))
					break;

			if (i == ent->num_leafs)
				continue;		// not visible
#endif
		}

		if (msg->maxsize - msg->cursize < 16)
		{
			Con_Printf ("packet overflow\n");
			return;
		}

// send an update
		bits = 0;

		for (i=0 ; i<3 ; i++)
		{
			miss = ent->v.origin[i] - ent->baseline.origin[i];
			if ( miss < -0.1 || miss > 0.1 )
				bits |= U_ORIGIN1<<i;
		}

		if ( ent->v.angles[0] != ent->baseline.angles[0] )
			bits |= U_ANGLE1;

		if ( ent->v.angles[1] != ent->baseline.angles[1] )
			bits |= U_ANGLE2;

		if ( ent->v.angles[2] != ent->baseline.angles[2] )
			bits |= U_ANGLE3;

		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_NOLERP;	// don't mess up the step animation

		if (ent->baseline.colormap != ent->v.colormap)
			bits |= U_COLORMAP;

		if (ent->baseline.skin != ent->v.skin)
			bits |= U_SKIN;

		if (ent->baseline.frame != ent->v.frame)
			bits |= U_FRAME;

		if (ent->baseline.effects != ent->v.effects)
			bits |= U_EFFECTS;

		if (ent->baseline.modelindex != ent->v.modelindex)
			bits |= U_MODEL;
#ifdef ALPHASCALE
		// from tomazquake
if(dpprotocol)
		{
			alpha		= 1;
			glowcolor	= 0;
			glowsize	= 0;
			scale		= 1;
		if (val = GETEDICTFIELDVALUE(ent, pr_field_alpha)  )
				alpha = val->_float;

		if (val = GETEDICTFIELDVALUE(ent, pr_field_glowcolor)  )
				glowcolor = val->_float;

		if (val = GETEDICTFIELDVALUE(ent, pr_field_glowsize)  )
				glowsize = val->_float;
					if ((val = GetEdictFieldValue(ent, "scale")))
			{
				scale = val->_float;
				if (scale > 4)
					scale = 4;
				if (scale > 0)
					bits |= U_SCALE;
			}

//			if ((val = GETEDICTFIELDVALUE(ent, eval_renderamt)) && val->_float != 0) // HalfLife support
//				alpha = val->_float / 255;

//			if (val = GETEDICTFIELDVALUE(ent, eval_scale))
//				scale = val->_float;

	//		if (scale > 4)
	//			scale = 4;
			
		if (alpha < 1)
				bits |= U_ALPHA;

		if (glowsize)
				bits |= U_GLOWSIZE;

		if (glowcolor)
				bits |= U_GLOWCOLOR;
		
//		if ((scale <= 4) && (scale > 0))
//				bits |= U_SCALE;

		}
#endif
		if (e >= 256)
			bits |= U_LONGENTITY;

		
#ifdef VMTOC
		if (viewmodel)
			bits |= U_VIEWMODEL;

		if (bits >= 256)
			bits |= U_MOREBITS;
		if (bits >= 65536)
			bits |= U_EXTEND1;
		if (bits >= 16777216)
			bits |= U_EXTEND2;
#else
			if (bits >= 256)
			bits |= U_MOREBITS;
#ifdef SONOFABITS
			// Tomaz - QC Control Begin
		if (bits >= 65536)
			bits |= U_EXTEND1;

		if (bits >= 16777216)
			bits |= U_EXTEND2;
		// Tomaz - QC Control End
#endif
#endif

	//
	// write the message
	//
		MSG_WriteByte (msg,bits | U_SIGNAL);
#ifdef SONOFABITS
/*	// ??
		if (bits & U_MOREBITS)
			MSG_WriteByte(msg, (bits & 0x0000FF00) >> 8);
		if (bits & U_EXTEND1)
			MSG_WriteByte(msg, (bits & 0x00FF0000) >> 16);
		if (bits & U_EXTEND2)
			MSG_WriteByte(msg, (bits & 0xFF000000) >> 24);
*/
			if (bits & U_MOREBITS)
			MSG_WriteByte (msg, bits>>8);
		if (bits & U_EXTEND1)
			MSG_WriteByte (msg, bits>>16);
		if (bits & U_EXTEND2)
			MSG_WriteByte (msg, bits>>24);

#else
		if (bits & U_MOREBITS)
			MSG_WriteByte (msg, bits>>8);
		if (bits & U_EXTEND1)
			MSG_WriteByte (msg, bits>>16);
		if (bits & U_EXTEND2)
			MSG_WriteByte (msg, bits>>24);
#endif
		if (bits & U_LONGENTITY)
			MSG_WriteShort (msg,e);
		else
			MSG_WriteByte (msg,e);

		if (bits & U_MODEL)
			MSG_WriteByte (msg,	ent->v.modelindex);
		if (bits & U_FRAME)
			MSG_WriteByte (msg, ent->v.frame);
		if (bits & U_COLORMAP)
			MSG_WriteByte (msg, ent->v.colormap);
		if (bits & U_SKIN)
			MSG_WriteByte (msg, ent->v.skin);
		if (bits & U_EFFECTS)
			MSG_WriteByte (msg, ent->v.effects);
		if (bits & U_ORIGIN1)
			MSG_WriteCoord (msg, ent->v.origin[0]);
		if (bits & U_ANGLE1)
			MSG_WriteAngle(msg, ent->v.angles[0]);
		if (bits & U_ORIGIN2)
			MSG_WriteCoord (msg, ent->v.origin[1]);
		if (bits & U_ANGLE2)
			MSG_WriteAngle(msg, ent->v.angles[1]);
		if (bits & U_ORIGIN3)
			MSG_WriteCoord (msg, ent->v.origin[2]);
		if (bits & U_ANGLE3)
			MSG_WriteAngle(msg, ent->v.angles[2]);
#ifdef ALPHASCALE
		if(dpprotocol){
		if (bits & U_ALPHA)
			MSG_WriteFloat(msg, alpha);
		if (bits & U_GLOWCOLOR)
			MSG_WriteFloat(msg, glowcolor);
		if (bits & U_GLOWSIZE)
			MSG_WriteFloat(msg, glowsize);

//		if (bits & U_SCALE)
	//		MSG_WriteFloat(msg, scale);
		}
#endif
	}
}

/*
=============
SV_CleanupEnts

=============
*/
void SV_CleanupEnts (void)
{
	int		e;
	edict_t	*ent;

	ent = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
	{
		ent->v.effects = (int)ent->v.effects & ~EF_MUZZLEFLASH;
	}

}

/*
==================
SV_WriteClientdataToMessage

==================
*/
void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg)
{
	int		bits;
	int		i;
	edict_t	*other;
	int		items;
#ifndef QUAKE2
	eval_t	*val;
#endif

//
// send a damage message
//
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = PROG_TO_EDICT(ent->v.dmg_inflictor);
		MSG_WriteByte (msg, svc_damage);
		MSG_WriteByte (msg, ent->v.dmg_save);
		MSG_WriteByte (msg, ent->v.dmg_take);
		for (i=0 ; i<3 ; i++)
			MSG_WriteCoord (msg, other->v.origin[i] + 0.5*(other->v.mins[i] + other->v.maxs[i]));

		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

//
// send the current viewpos offset from the view entity
//
	SV_SetIdealPitch ();		// how much to look up / down ideally

// a fixangle might get lost in a dropped packet.  Oh well.
	if ( ent->v.fixangle )
	{
// 2000-05-02 NVS SVC_setangle by Maddes  start
//		MSG_WriteByte (msg, svc_setangle);
		NVS_InitSVCMsg(MSG_ONE, svc_setangle, 0, host_client);
		NVS_WriteByte (MSG_ONE, svc_setangle, msg);
// 2000-05-02 NVS SVC_setangle by Maddes  end
		for (i=0 ; i < 3 ; i++)
// 2000-05-02 NVS SVC_setangle by Maddes  start
//			MSG_WriteAngle (msg, ent->v.angles[i] );
		{
			NVS_WriteAngle (MSG_ONE, ent->v.angles[i], msg);
		}
		if (nvs_current_ssvc->value >= 0.50)
		{
			for (i=0 ; i < 3 ; i++)
			{
				NVS_WriteFloat (MSG_ONE, ent->v.angles[i], msg);
			}
		}
// 2000-05-02 NVS SVC_setangle by Maddes  end
		ent->v.fixangle = 0;
	}

	bits = 0;

	if (ent->v.view_ofs[2] != DEFAULT_VIEWHEIGHT)
		bits |= SU_VIEWHEIGHT;

	if (ent->v.idealpitch)
		bits |= SU_IDEALPITCH;

// stuff the sigil bits into the high bits of items for sbar, or else
// mix in items2
#ifdef QUAKE2
	items = (int)ent->v.items | ((int)ent->v.items2 << 23);
#else
// 2001-11-15 Better GetEdictFieldValue performance by LordHavoc/Maddes  start
//	val = GetEdictFieldValue(ent, "items2");
	val = GETEDICTFIELDVALUE(ent, pr_field_items2);
// 2001-11-15 Better GetEdictFieldValue performance by LordHavoc/Maddes  end

	if (val)
		items = (int)ent->v.items | ((int)val->_float << 23);
	else
		items = (int)ent->v.items | ((int)pr_global_struct->serverflags << 28);
#endif

	bits |= SU_ITEMS;

	if ( (int)ent->v.flags & FL_ONGROUND)
		bits |= SU_ONGROUND;

	if ( ent->v.waterlevel >= 2)
		bits |= SU_INWATER;

	for (i=0 ; i<3 ; i++)
	{
		if (ent->v.punchangle[i])
			bits |= (SU_PUNCH1<<i);
		if (ent->v.velocity[i])
			bits |= (SU_VELOCITY1<<i);
	}

	if (ent->v.weaponframe)
		bits |= SU_WEAPONFRAME;

	if (ent->v.armorvalue)
		bits |= SU_ARMOR;

//	if (ent->v.weapon)
		bits |= SU_WEAPON;

// send the data

	MSG_WriteByte (msg, svc_clientdata);
	MSG_WriteShort (msg, bits);

	if (bits & SU_VIEWHEIGHT)
		MSG_WriteChar (msg, ent->v.view_ofs[2]);

	if (bits & SU_IDEALPITCH)
		MSG_WriteChar (msg, ent->v.idealpitch);

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i))
			MSG_WriteChar (msg, ent->v.punchangle[i]);
		if (bits & (SU_VELOCITY1<<i))
			MSG_WriteChar (msg, ent->v.velocity[i]/16);
	}
// [always sent]	if (bits & SU_ITEMS)
	MSG_WriteLong (msg, items);

	if (bits & SU_WEAPONFRAME)
		MSG_WriteByte (msg, ent->v.weaponframe);
	if (bits & SU_ARMOR)
		MSG_WriteByte (msg, ent->v.armorvalue);
	if (bits & SU_WEAPON)
		MSG_WriteByte (msg, SV_ModelIndex(pr_strings+ent->v.weaponmodel));

	MSG_WriteShort (msg, ent->v.health);
	MSG_WriteByte (msg, ent->v.currentammo);
	MSG_WriteByte (msg, ent->v.ammo_shells);
	MSG_WriteByte (msg, ent->v.ammo_nails);
	MSG_WriteByte (msg, ent->v.ammo_rockets);
	MSG_WriteByte (msg, ent->v.ammo_cells);

	if (standard_quake)
	{
		MSG_WriteByte (msg, ent->v.weapon);
	}
	else
	{
		for(i=0;i<32;i++)
		{
			if ( ((int)ent->v.weapon) & (1<<i) )
			{
				MSG_WriteByte (msg, i);
				break;
			}
		}
	}
}

/*
=======================
SV_SendClientDatagram
=======================
*/
qboolean SV_SendClientDatagram (client_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	MSG_WriteByte (&msg, svc_time);
	MSG_WriteFloat (&msg, sv.time);

// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client->edict, &msg);

	SV_WriteEntitiesToClient (client->edict, &msg);

// copy the server datagram if there is space
// 2000-05-02 NVS SVC by Maddes  start
	if (msg.cursize + client->datagram.cursize < msg.maxsize)
		SZ_Write (&msg, client->datagram.data, client->datagram.cursize);
// 2000-05-02 NVS SVC by Maddes  end
	if (msg.cursize + sv.datagram.cursize < msg.maxsize)
		SZ_Write (&msg, sv.datagram.data, sv.datagram.cursize);

// send the datagram
	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
	{
		SV_DropClient (true);// if the message couldn't send, kick off
		return false;
	}

	return true;
}

/*
=======================
SV_UpdateToReliableMessages
=======================
*/
void SV_UpdateToReliableMessages (void)
{
	int			i, j;
	client_t *client;

// check for changes to be sent over the reliable streams
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (host_client->old_frags != host_client->edict->v.frags)
		{
			for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
			{
				if (!client->active)
					continue;
				MSG_WriteByte (&client->message, svc_updatefrags);
				MSG_WriteByte (&client->message, i);
				MSG_WriteShort (&client->message, host_client->edict->v.frags);
			}

			host_client->old_frags = host_client->edict->v.frags;
		}
	}

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		SZ_Write (&client->message, sv.reliable_datagram.data, sv.reliable_datagram.cursize);
	}

	SZ_Clear (&sv.reliable_datagram);
}


/*
=======================
SV_SendNop

Send a nop message without trashing or sending the accumulated client
message buffer
=======================
*/
void SV_SendNop (client_t *client)
{
	sizebuf_t	msg;
	byte		buf[4];

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	MSG_WriteChar (&msg, svc_nop);

	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
		SV_DropClient (true);	// if the message couldn't send, kick off
	client->last_message = realtime;
}

/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	int			i;

// update frags, names, etc
	SV_UpdateToReliableMessages ();

// build individual updates
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

		if (host_client->spawned)
		{
			if (!SV_SendClientDatagram (host_client))
				continue;
		}
		else
		{
		// the player isn't totally in the game yet
		// send small keepalive messages if too much time has passed
		// send a full message when the next signon stage has been requested
		// some other message data (name changes, etc) may accumulate
		// between signon stages
			if (!host_client->sendsignon)
			{
				if (realtime - host_client->last_message > 5)
					SV_SendNop (host_client);
				continue;	// don't send out non-signon messages
			}
		}

		// check for an overflowed message.  Should only happen
		// on a very fucked up connection that backs up a lot, then
		// changes level
		if (host_client->message.overflowed)
		{
			SV_DropClient (true);
			host_client->message.overflowed = false;
			continue;
		}

		if (host_client->message.cursize || host_client->dropasap)
		{
			if (!NET_CanSendMessage (host_client->netconnection))
			{
//				I_Printf ("can't write\n");
				continue;
			}

			if (host_client->dropasap)
				SV_DropClient (false);	// went to another level
			else
			{
				if (NET_SendMessage (host_client->netconnection
				, &host_client->message) == -1)
					SV_DropClient (true);	// if the message couldn't send, kick off
				SZ_Clear (&host_client->message);
				host_client->last_message = realtime;
				host_client->sendsignon = false;
			}
		}
	}


// clear muzzle flashes
	SV_CleanupEnts ();
}


/*
==============================================================================

SERVER SPAWNING

==============================================================================
*/

/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (char *name)
{
	int		i;

	if (!name || !name[0])
		return 0;

	for (i=0 ; i<MAX_MODELS && sv.model_precache[i] ; i++)
		if (!strcmp(sv.model_precache[i], name))
			return i;
	if (i==MAX_MODELS || !sv.model_precache[i])
		Sys_Error ("SV_ModelIndex: model %s not precached", name);
	return i;
}

/*
================
SV_CreateBaseline

================
*/
void SV_CreateBaseline (void)
{
	int			i;
	edict_t			*svent;
	int				entnum;

	for (entnum = 0; entnum < sv.num_edicts ; entnum++)
	{
	// get the current server version
		svent = EDICT_NUM(entnum);
		if (svent->free)
			continue;
		if (entnum > svs.maxclients && !svent->v.modelindex)
			continue;

	//
	// create entity baseline
	//
		VectorCopy (svent->v.origin, svent->baseline.origin);
		VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = svent->v.frame;
		svent->baseline.skin = svent->v.skin;
		if (entnum > 0 && entnum <= svs.maxclients)
		{
			svent->baseline.colormap = entnum;
			if(gamemode == GAME_LASER_ARENA)
			svent->baseline.modelindex = SV_ModelIndex("progs/player.mdl");
			else
			svent->baseline.modelindex = SV_ModelIndex("progs/player.mdl");
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex =
				SV_ModelIndex(pr_strings + svent->v.model);
		}

	//
	// add to the message
	//
		MSG_WriteByte (&sv.signon,svc_spawnbaseline);
		MSG_WriteShort (&sv.signon,entnum);

		MSG_WriteByte (&sv.signon, svent->baseline.modelindex);
		MSG_WriteByte (&sv.signon, svent->baseline.frame);
		MSG_WriteByte (&sv.signon, svent->baseline.colormap);
		MSG_WriteByte (&sv.signon, svent->baseline.skin);
		for (i=0 ; i<3 ; i++)
		{
			MSG_WriteCoord(&sv.signon, svent->baseline.origin[i]);
			MSG_WriteAngle(&sv.signon, svent->baseline.angles[i]);
		}
	}
}


/*
================
SV_SendReconnect

Tell all the clients that the server is changing levels
================
*/
void SV_SendReconnect (void)
{
	char	data[128];
	sizebuf_t	msg;

	msg.data = data;
	msg.cursize = 0;
	msg.maxsize = sizeof(data);

	MSG_WriteChar (&msg, svc_stufftext);
	MSG_WriteString (&msg, "reconnect\n");
	NET_SendToAll (&msg, 5);

	if (cls.state != ca_dedicated)
#ifdef QUAKE2
		Cbuf_InsertText ("reconnect\n");
#else
		Cmd_ExecuteString ("reconnect\n", src_command);
#endif
}


/*
================
SV_SaveSpawnparms

Grabs the current state of each client for saving across the
transition to another level
================
*/
void SV_SaveSpawnparms (void)
{
	int		i, j;

	svs.serverflags = pr_global_struct->serverflags;

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

	// call the progs to get default spawn parms for the new client
		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram (pr_global_struct->SetChangeParms);
		for (j=0 ; j<NUM_SPAWN_PARMS ; j++)
			host_client->spawn_parms[j] = (&pr_global_struct->parm1)[j];
	}
}


/*
================
SV_SpawnServer

This is called at the start of each level
================
*/
extern float		scr_centertime_off;

#ifdef QUAKE2
void SV_SpawnServer (char *server, char *startspot)
#else
void SV_SpawnServer (char *server)
#endif
{
	edict_t		*ent;
	int			i;
	imsaving = 1;
	// let's not have any servers with no name
	if (hostname->string[0] == 0)
		Cvar_Set (hostname, "UNNAMED");
	scr_centertime_off = 0;

	Con_DPrintf ("SpawnServer: %s\n",server);
	svs.changelevel_issued = false;		// now safe to issue another

//
// tell all connected clients that we are going to a new level
//
	if (sv.active)
	{
		SV_SendReconnect ();
	}

//
// make cvars consistant
//
	if (coop->value)
		Cvar_Set (deathmatch, "0");
	current_skill = (int)(skill->value + 0.5);
	if (current_skill < 0)
		current_skill = 0;
	if (current_skill > 3)
		current_skill = 3;

	Cvar_SetValue (skill, (float)current_skill);

//
// set up the new server
//
	Host_ClearMemory ();

	memset (&sv, 0, sizeof(sv));

	strcpy (sv.name, server);
#ifdef QUAKE2
	if (startspot)
		strcpy(sv.startspot, startspot);
#endif

// load progs to get entity field count
	PR_LoadProgs ();

// allocate server memory
// 2001-09-20 Configurable entity limits by Maddes  start
	// do rangechecks
	Cvar_Set(sv_entities, sv_entities->string);
	Cvar_Set(sv_entities_static, sv_entities_static->string);
	Cvar_Set(sv_entities_temp, sv_entities_temp->string);

//	sv.max_edicts = MAX_EDICTS;
	sv.max_edicts = sv_entities->value;

	moved_edict = Hunk_AllocName (sv.max_edicts*sizeof(edict_t *), "sv_mv_ed");
	moved_from = Hunk_AllocName (sv.max_edicts*sizeof(vec3_t), "sv_mvf_ed");
// 2001-09-20 Configurable entity limits by Maddes  end

	sv.edicts = Hunk_AllocName (sv.max_edicts*pr_edict_size, "edicts");

	sv.datagram.maxsize = sizeof(sv.datagram_buf);
	sv.datagram.cursize = 0;
	sv.datagram.data = sv.datagram_buf;

	sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
	sv.reliable_datagram.cursize = 0;
	sv.reliable_datagram.data = sv.reliable_datagram_buf;

	sv.signon.maxsize = sizeof(sv.signon_buf);
	sv.signon.cursize = 0;
	sv.signon.data = sv.signon_buf;

// leave slots at start for clients only
	sv.num_edicts = svs.maxclients+1;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;

// 2000-05-02 NVS SVC by Maddes  start
		svs.clients[i].datagram.data = svs.clients[i].datagram_buf;
		svs.clients[i].datagram.maxsize = sizeof(svs.clients[i].datagram_buf);
		SZ_Clear(&svs.clients[i].datagram);
// 2000-05-02 NVS SVC by Maddes  end
	}

	sv.state = ss_loading;
	sv.paused = false;

	sv.time = 1.0;

	strcpy (sv.name, server);
	sprintf (sv.modelname,"maps/%s.bsp", server);
	sv.worldmodel = Mod_ForName (sv.modelname, false);
// 2001-12-16 No crash on wrong BSP version by MrG  start
//	if (!sv.worldmodel)
	if ((!sv.worldmodel) || (sv.worldmodel->numvertexes == -1))
// 2001-12-16 No crash on wrong BSP version by MrG  end
	{
		Con_Printf ("Couldn't spawn server %s\n", sv.modelname);
		sv.active = false;
		return;
	}
	sv.models[1] = sv.worldmodel;

//
// clear world interaction links
//
	SV_ClearWorld ();

	sv.sound_precache[0] = pr_strings;

	sv.model_precache[0] = pr_strings;
	sv.model_precache[1] = sv.modelname;
	for (i=1 ; i<sv.worldmodel->numsubmodels ; i++)
	{
		sv.model_precache[1+i] = localmodels[i];
		sv.models[i+1] = Mod_ForName (localmodels[i], false);
	}

//
// load the rest of the entities
//
	ent = EDICT_NUM(0);
	memset (&ent->v, 0, progs->entityfields * 4);
	ent->free = false;
	ent->v.model = sv.worldmodel->name - pr_strings;
	ent->v.modelindex = 1;		// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop->value)
		pr_global_struct->coop = coop->value;
	else
		pr_global_struct->deathmatch = deathmatch->value;

	pr_global_struct->mapname = sv.name - pr_strings;
#ifdef QUAKE2
	pr_global_struct->startspot = sv.startspot - pr_strings;
#endif

// serverflags are for cross level information (sigils)
	pr_global_struct->serverflags = svs.serverflags;

// 2000-04-30 NVS HANDSHAKE SRV<->QC by Maddes  start
	Cvar_Set(nvs_current_ssvc, "0");
// 2001-12-24 Keeping full backwards compatibility by Maddes  start
	if ( (!(sv_compatibility->value))	// allow setting value, unlike the original Quake executable
	&&   (nvs_svc_enable->value))
	{
// 2001-12-24 Keeping full backwards compatibility by Maddes  end
		nvs_current_ssvc->flags &= ~CVAR_ROM;	// cvar can now be changed through QuakeC in WorldSpawn() of PROGS.DAT
	}	// 2001-12-24 Keeping full backwards compatibility by Maddes
// 2000-04-30 NVS HANDSHAKE SRV<->QC by Maddes  end
	zone_progstrings = NULL;	// 2001-09-20 QuakeC string zone by Maddes
	ED_LoadFromFile (sv.worldmodel->entities);
// 2000-04-30 NVS HANDSHAKE SRV<->QC by Maddes  start
	nvs_current_ssvc->flags |= CVAR_ROM;

	// not necessary to set .nvs_svc if no version is set
	if (!(nvs_current_ssvc->value))
	{
			pr_field_nvs_svc = NULL;
	}
// 2000-04-30 NVS HANDSHAKE SRV<->QC by Maddes  end

	sv.active = true;

// all setup is completed, any further precache statements are errors
	sv.state = ss_active;

// run two frames to allow everything to settle
	host_org_frametime =	// 2001-10-20 TIMESCALE extension by Tomaz/Maddes
	host_frametime = 0.1;
	SV_Physics ();
	SV_Physics ();

// create a baseline for more efficient communications
	SV_CreateBaseline ();

// send serverinfo to all connected clients
	for (i=0,host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_SendServerinfo (host_client);

	Con_DPrintf ("Server spawned.\n");
	imsaving = 0;
}
