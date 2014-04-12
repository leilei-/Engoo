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
// cl_tent.c -- client side temporary entities

#include "quakedef.h"

int			num_temp_entities;
// 2001-09-20 Configurable entity limits by Maddes  start
//entity_t	cl_temp_entities[MAX_TEMP_ENTITIES];
entity_t	*cl_temp_entities;
// 2001-09-20 Configurable entity limits by Maddes  end
beam_t		cl_beams[MAX_BEAMS];


sfx_t			*cl_sfx_wizhit;
sfx_t			*cl_sfx_knighthit;
sfx_t			*cl_sfx_tink1;
sfx_t			*cl_sfx_ric1;
sfx_t			*cl_sfx_ric2;
sfx_t			*cl_sfx_ric3;
sfx_t			*cl_sfx_r_exp3;
sfx_t			*cl_sfx_imp;
sfx_t			*cl_sfx_rail;
sfx_t			*cl_sfx_bloodhit1;
sfx_t			*cl_sfx_bloodhit2;
sfx_t			*cl_sfx_bloodhit3;

/*
=================
CL_ParseTEnt
=================
*/
void CL_InitTEnts (void)
{
	cl_sfx_wizhit = S_PrecacheSound ("wizard/hit.wav");
	cl_sfx_knighthit = S_PrecacheSound ("hknight/hit.wav");
	cl_sfx_tink1 = S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1 = S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2 = S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3 = S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3 = S_PrecacheSound ("weapons/r_exp3.wav");

	cl_sfx_imp = S_PrecacheSound ("shambler/sattck1.wav");
	cl_sfx_rail = S_PrecacheSound ("weapons/lstart.wav");

	cl_sfx_bloodhit1 = S_PrecacheSound ("player/tornoff2.wav");
	cl_sfx_bloodhit2 = S_PrecacheSound ("demon/dhit2.wav");
	cl_sfx_bloodhit3 = S_PrecacheSound ("zombie/z_miss.wav");

}
extern cvar_t *r_flares;
/*
=================
CL_ParseBeam
=================
*/
void CL_ParseBeam (model_t *m)
{
	int		ent;
	vec3_t	start, end;
	beam_t	*b;
	int		i;

	ent = MSG_ReadShort ();

	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();

	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

// override any beam with the same entity
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;

			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);

			
			return;
		}
	}
	Con_Printf ("beam list overflow!\n");
}



extern	cvar_t	*r_coloreddyns;
/*
=================
CL_ParseTEnt
=================
*/
extern int particleset;
extern void R_Decal (vec3_t org, int decframe, int blend, float sceel, int splat);
void CL_ParseTEnt (void)
{
	int		type;
	vec3_t	pos;
	vec3_t	pos2;
	vec3_t	vel;
	vec3_t	dir;
	vec3_t	mins;
	vec3_t	maxs;
	vec3_t	vel1;
	vec3_t	vel2;
	int		count;
	int		thecol;
	int		speed;
	vec3_t	endpos;
	int		amount;
	dlight_t	*dl;
	int		rnd;
	int		colorStart, colorLength;

	type = MSG_ReadByte ();
	switch (type)
	{
	case TE_WIZSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 20, 30);
		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;

	case TE_KNIGHTSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 226, 20);
		S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;

	case TE_SPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (particleset == 7){		
			//R_FlareTest (pos, 1, 255, 0, 0);
			R_FlareTest (pos, 1, rand()&255, rand()&255, rand()&255, 0, NULL);	// tseting random colored flares with nails
		}
			else
		R_RunParticleEffect (pos, vec3_origin, 0, 10);
		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;
	case TE_SUPERSPIKE:			// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20);

		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

// TomazQuake code
	case TE_SNOW:
		pos[0] = MSG_ReadCoord (); // mins
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		pos2[0] = MSG_ReadCoord (); // maxs
		pos2[1] = MSG_ReadCoord ();
		pos2[2] = MSG_ReadCoord ();
		amount	= MSG_ReadShort ();
		R_Snow(pos, pos2, amount);
		break;
		
	case TE_RAIN:
		pos[0]	= MSG_ReadCoord (); // mins
		pos[1]	= MSG_ReadCoord ();
		pos[2]	= MSG_ReadCoord ();
		pos2[0] = MSG_ReadCoord (); // maxs
		pos2[1] = MSG_ReadCoord ();
		pos2[2] = MSG_ReadCoord ();
		amount	= MSG_ReadShort ();
		R_Rain(pos, pos2, amount);
		break;			
// TomazQuake code

	case TE_GUNSHOT:			// bullet hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (particleset == 2){
			R_Smoke (pos, vec3_origin, 0, 20);
			R_Decal (pos, 4, 6, 0, 0);
		}
			else
		R_RunParticleEffect (pos, vec3_origin, 0, 20);
		break;

	case TE_EXPLOSION:			// rocket explosion
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();

		if (particleset == 2){
	//		R_Smoke (pos, vec3_origin, 0, 20);
			R_ParticleExplosionSpritey (pos, 13, 6, 2);
			R_Decal (pos, 13, 6, 2, 0);
		}
		else
		R_ParticleExplosion (pos);
		dl = CL_AllocDlight (0);
			if (r_coloreddyns->value){
				dl->color[0] = 4.0f;
				dl->color[1] = 2.0f;
				dl->color[2] = 0.5f;	// TODO: get avg color from model
		
					}
		VectorCopy (pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;
// 2000-05-02 NVS SVC_TE te_explosion by Maddes  start
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
		if ((nvs_current_csvc->value >= 0.50) && (cls.signon > 1))
		{
			dl->flashcolor[0] = dl->color[0] = MSG_ReadCoord();
			dl->flashcolor[1] = dl->color[1] = MSG_ReadCoord();
			dl->flashcolor[2] = dl->color[2] = MSG_ReadCoord();
			dl->flashcolor[3] = MSG_ReadCoord();
		}
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
// 2000-05-02 NVS SVC_TE te_explosion by Maddes  end
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);


		break;

	case TE_TAREXPLOSION:			// tarbaby explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_BlobExplosion (pos);

		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

	case TE_LIGHTNING1:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt.mdl", true));
		break;

	case TE_LIGHTNING2:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt2.mdl", true));
		break;

	case TE_LIGHTNING3:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt3.mdl", true));
		break;

// PGM 01/21/97
	case TE_BEAM:				// grappling hook beam
		CL_ParseBeam (Mod_ForName("progs/beam.mdl", true));
		break;
// PGM 01/21/97

	case TE_LAVASPLASH:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_LavaSplash (pos);
		break;

	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_TeleportSplash (pos);
		break;

	case TE_EXPLOSION2:				// color mapped explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		colorStart = MSG_ReadByte ();
		colorLength = MSG_ReadByte ();
		R_ParticleExplosion2 (pos, colorStart, colorLength);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

//	case TE_IMPLOSION:
//		pos[0] = MSG_ReadCoord ();
//		pos[1] = MSG_ReadCoord ();
//		pos[2] = MSG_ReadCoord ();
//		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
//		break;

	case TE_RAILTRAIL:				// Straight out of Quake2!
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		endpos[0] = MSG_ReadCoord ();
		endpos[1] = MSG_ReadCoord ();
		endpos[2] = MSG_ReadCoord ();
		vel[0] = MSG_ReadCoord ();
		vel[1] = MSG_ReadCoord ();
		vel[2] = MSG_ReadCoord ();
		R_RailTrail (pos, endpos);
//		R_RocketTrail (pos, endpos, 7);
		break;

	case TE_BLOOD:
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		vel[0] = (float)MSG_ReadChar();
		vel[1] = (float)MSG_ReadChar();
		vel[2] = (float)MSG_ReadChar();
		count = MSG_ReadByte();
		R_Blood(count, pos, pos, vel, vel);
		break;

	case TE_BLOODSHOWER:
		mins[0] = MSG_ReadCoord();
		mins[1] = MSG_ReadCoord();
		mins[2] = MSG_ReadCoord();
		maxs[0] = MSG_ReadCoord();
		maxs[1] = MSG_ReadCoord();
		maxs[2] = MSG_ReadCoord();
		speed = MSG_ReadCoord();
		count = MSG_ReadShort();
		vel1[0] = -speed;
		vel1[1] = -speed;
		vel1[2] = -speed;
		vel2[0] = speed;
		vel2[1] = speed;
		vel2[2] = speed;
		R_Blood(count, mins, maxs, vel1, vel2);
		break;

	case TE_PLASMABURN:			// plasma burn
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord(); // all thius is, is just a quick white flash
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 250;
		dl->die = cl.time + 0.2;
		dl->decay = 1500;
		break;

	case TE_TEI_SMOKE:			// makes some puff of smoke
							// the direction doesn't do much in DP
							// but we do it anyway
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		vel[0] = MSG_ReadCoord ();
		vel[1] = MSG_ReadCoord ();
		vel[2] = MSG_ReadCoord ();
		count = MSG_ReadByte();
		R_Smoke (pos, vel, 0, count * 4);
		break;

	case TE_TEI_BIGEXPLOSION:			// rocket explosion
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_ParticleExplosion (pos); // in dp, this is just a explosion with a 
			//much much much much bigger dynamic light. Why? don't ask.
		dl = CL_AllocDlight (0);
			if (r_coloreddyns->value){
				dl->color[0] = 0.6f;
				dl->color[1] = 0.5f;
				dl->color[2] = 0.1f;	// TODO: get avg color from model
		
					}
		VectorCopy (pos, dl->origin);
		dl->radius = 850;
		dl->die = cl.time + 1;
		dl->decay = 800;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;


// Darkplaces QUAD DAMAGE TE's

	case TE_GUNSHOTQUAD:			// bullet hitting wall... one that does 4x damage
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20);
				dl = CL_AllocDlight (0);
		if (r_coloreddyns->value){
				dl->color[0] = 0.1f;dl->color[1] = 0.1f;dl->color[2] = 1.0f;}
		VectorCopy (pos, dl->origin);
		dl->radius = 200;
		dl->die = cl.time + 0.2;
		dl->decay = 1500;
		break;


	case TE_SPIKEQUAD:			// spike hitting wall WITH A BLUE FLASH of QUAD DAMAGE POWER!
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 10);
		dl = CL_AllocDlight (0);
		if (r_coloreddyns->value){
				dl->color[0] = 0.1f;dl->color[1] = 0.1f;dl->color[2] = 1.0f;}
		VectorCopy (pos, dl->origin);
		dl->radius = 200;
		dl->die = cl.time + 0.2;
		dl->decay = 1500;
		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;
	case TE_SUPERSPIKEQUAD:			// THE INFAMOUSLY REDUNDANT SPIKE FUNCTION GETS FLASH TOO
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20);
		dl = CL_AllocDlight (0);
		if (r_coloreddyns->value){
				dl->color[0] = 0.1f;dl->color[1] = 0.1f;dl->color[2] = 1.0f;}
		VectorCopy (pos, dl->origin);
		dl->radius = 200;
		dl->die = cl.time + 0.2;
		dl->decay = 1500;
		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_EXPLOSIONQUAD:			// one explosion no one wants to be part of
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_ParticleExplosion (pos);
		dl = CL_AllocDlight (0);
			if (r_coloreddyns->value){
				dl->color[0] = 0.1f;
				dl->color[1] = 0.1f;
				dl->color[2] = 1.0f;
		
					}
		VectorCopy (pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

// ---------------------------------------------------------
// Weird Teitei effects implemented by Leilei
// ---------------------------------------------------------
	case TE_TEI_G3:		// This was used for the Nex weapon in 
						// super old (2002) Nexuiz alphas
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		endpos[0] = MSG_ReadCoord ();
		endpos[1] = MSG_ReadCoord ();
		endpos[2] = MSG_ReadCoord ();
		vel[0] = MSG_ReadCoord () * 0.25;	// NONSTANDARD NOTE - this is "Angles" in the spec
		vel[1] = MSG_ReadCoord () * 0.25;;	// but in DP the angles aren't actually used
		vel[2] = MSG_ReadCoord () * 0.25;;	// for *ANYTHING* so we'll use them for color instead!
									// LH, can you 'color' the angles too?
									// vel[0] is inner colorindex
									// vel[1] is outer colorindex
									// vel[2] might tell it how to blend
	//	thecol = FindColorNoFB((int)vel[0],(int)vel[1],(int)vel[2]);
		
		if (vel[0] > 63) vel[0] = 63;
		if (vel[1] > 63) vel[1] = 63;
		if (vel[2] > 63) vel[2] = 63;
		if (vel[0] < 0) vel[0] = 0;
		if (vel[1] < 0) vel[1] = 0;
		if (vel[2] < 0) vel[2] = 0;
		//thecol = BestColor((int)vel[0],(int)vel[1],(int)vel[2]);
		thecol = palmap2[(int)vel[0]][(int)vel[1]][(int)vel[2]];
		R_BeamBeam (pos, endpos, thecol, 1);
		break;

	case TE_TEI_PLASMAHIT:				// Nexuiz electro explosion!
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		vel[0] = MSG_ReadCoord ();	// in dp, angle/dir isn't really used
		vel[1] = MSG_ReadCoord ();
		vel[2] = MSG_ReadCoord ();	// tei is messy :(
		count = MSG_ReadByte();
		R_PlasmaExplosion (pos, 40, 7, count);
		dl = CL_AllocDlight (0);
					if (r_coloreddyns->value){
				dl->color[0] = 0.2f;
				dl->color[1] = 0.2f;
				dl->color[2] = 1.0f;
		
					}
		VectorCopy (pos, dl->origin);
		dl->radius = 250;
		dl->die = cl.time + 0.3;
		dl->decay = 800;
		break;




	default:
// 2001-12-16 Various crashes changed to host errors by Maddes  start
//		Sys_Error ("CL_ParseTEnt: bad type");
		Host_Error ("CL_ParseTEnt: bad type %i", type);
		break;
// 2001-12-16 Various crashes changed to host errors by Maddes  end
	}
}


/*
=================
CL_NewTempEntity
=================
*/
entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

// 2001-09-20 Configurable entity limits by Maddes  start
	if (!cl_temp_entities)
	{
		Cvar_Set(cl_entities_min_temp, cl_entities_min_temp->string);	// do rangecheck
		if (cl.max_temp_edicts < cl_entities_min_temp->value)
		{
			cl.max_temp_edicts = cl_entities_min_temp->value;
		}
		Con_DPrintf("Allocating memory for %i temp entities.\n", cl.max_temp_edicts);

		cl_temp_entities = Hunk_AllocName (cl.max_temp_edicts*sizeof(entity_t), "cl_ed_temp");
		memset (cl_temp_entities, 0, cl.max_temp_edicts*sizeof(entity_t));
	}
// 2001-09-20 Configurable entity limits by Maddes  end

	if (cl_numvisedicts == MAX_VISEDICTS)
		return NULL;
// 2001-09-20 Configurable entity limits by Maddes  start
//	if (num_temp_entities == MAX_TEMP_ENTITIES)
	if (num_temp_entities >= cl.max_temp_edicts)
// 2001-09-20 Configurable entity limits by Maddes  end
		return NULL;
	ent = &cl_temp_entities[num_temp_entities];
	memset (ent, 0, sizeof(*ent));
	num_temp_entities++;
	cl_visedicts[cl_numvisedicts] = ent;
	cl_numvisedicts++;

	ent->colormap = vid.colormap;
	return ent;
}


/*
=================
CL_UpdateTEnts
=================
*/

void CL_UpdateTEnts (void)
{
	int			i;
	beam_t		*b;

	vec3_t		dist, org;
	float		d;
	entity_t	*ent;
	float		yaw, pitch;
	float		forward;

	num_temp_entities = 0;

// update lightning
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
			continue;

	// if coming from the player, update the start position
		if (b->entity == cl.viewentity)
		{
			VectorCopy (cl_entities[cl.viewentity].origin, b->start);
		}

	// calculate pitch and yaw
		VectorSubtract (b->end, b->start, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(dist[1], dist[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int) (atan2(dist[2], forward) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		if (particleset == 7){
			VectorCopy (b->start, org);
			d = VectorNormalize(dist);

			// leilei - an electric beam...
			//R_BeamBeam (org, b->end, 211, 3);
			
			R_LightningBeam (b->start, b->end, 6);
		}
		else
		{
	// add new entities for the lightning
		VectorCopy (b->start, org);
		d = VectorNormalize(dist);
		while (d > 0)
		{
			ent = CL_NewTempEntity ();
			if (!ent)
				return;
			VectorCopy (org, ent->origin);
			ent->model = b->model;
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;
			ent->angles[2] = rand()%360;
#ifdef SCALEE
			ent->scale2 = 1.0f; // makaqu
#endif

		// leilei - unrolled
				org[0] += dist[0]*30;
				org[1] += dist[1]*30;
				org[2] += dist[2]*30;



			d -= 30;
			//if (r_flares->value > 2)
			//	R_FlareTest(ent->origin,12,7,15,36,0,ent);
		}
		}
	}



}
