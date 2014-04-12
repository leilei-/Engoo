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
#include "r_local.h"

//#define MAX_PARTICLES			8192	// default max # of particles at one
										//  time
#define MAX_PARTICLES			16384	// default max # of particles at one



#define ABSOLUTE_MIN_PARTICLES	512		// no fewer than this no matter what's
										//  on the command line

#define	ZEROPROPERTIES (p->scale = 1; p->scaley = p->scale; p->blend = 0; p->alpha = 1; p->frame = 0; p->alphavel = 0;p->scalexvel = 0;p->scaleyvel = 0;p->anglevel[0] = 0;p->anglevel[1] = 0;p->anglevel[2] = 0;);


int		ramp1[8] = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
int		ramp2[8] = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
int		ramp3[8] = {0x6d, 0x6b, 6, 5, 4, 3};
int		ramp4[8] = {239, 236, 231, 227, 224, 0, 0, 0};
float	defaultscale = 0.4f;
particle_t	*active_particles, *free_particles;

particle_t	*particles;
extern flare_t		*currentflare;
flare_t	*active_flares, *free_flares;
flare_t	*flares;
int			r_numparticles;
int			r_numflares;

vec3_t			r_pright, r_pup, r_ppn;
void R_NormalFromHerePlease (vec3_t org, vec3_t steer);
extern	int	particlespray;
extern	int	particleblood;
extern	int	particleset;
extern	int	particlelit;

int		sprity = 1;		// enabled if particle.spr exists and if r_particlesprite is enabled



extern qboolean SV_RecursiveHullCheck(hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace);

static trace_t Particle_TraceLine(vec3_t start, vec3_t end)
{
	trace_t	trace;

	memset(&trace, 0, sizeof(trace));
	trace.fraction = 1.0f;
	trace.allsolid = true;
	VectorCopy(end, trace.endpos);
	trace.color = 4;
	SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	return trace;
}

/*
===============
R_InitParticles
===============
*/
void R_InitParticles (void)
{
	int		i;

	i = COM_CheckParm ("-particles");

	if (i)
	{
		r_numparticles = (int)(Q_atoi(com_argv[i+1]));
		if (r_numparticles < ABSOLUTE_MIN_PARTICLES)
			r_numparticles = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		r_numparticles = MAX_PARTICLES;
	}

	particles = (particle_t *)
			Hunk_AllocName (r_numparticles * sizeof(particle_t), "particles");

}


void R_InitFlares (void)
{
	int		i;

	i = COM_CheckParm ("-flares");

	if (i)
	{
		r_numflares = (int)(Q_atoi(com_argv[i+1]));
		if (r_numflares < ABSOLUTE_MIN_PARTICLES)
			r_numflares = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		r_numflares = 256;
	}

	flares = (flare_t *)
			Hunk_AllocName (r_numparticles * sizeof(flare_t), "flares");

}


/*
===============
R_EntityParticles
===============
*/

#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];
vec3_t	avelocities[NUMVERTEXNORMALS];
float	beamlength = 16;
vec3_t	avelocity = {23, 7, 3};
float	partstep = 0.01;
float	timescale = 0.01;

void R_EntityParticles (entity_t *ent)
{
//	int			count;	// 2001-12-10 Reduced compiler warnings by Jeff Ford
	int			i;
	particle_t	*p;
	float		angle;
	float		sp, sy, cp, cy;	//sr, cr, 	// 2001-12-10 Reduced compiler warnings by Jeff Ford
	vec3_t		forward;
	float		dist;

	dist = 64;
//	count = 50;	// 2001-12-10 Reduced compiler warnings by Jeff Ford

	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}

	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		angle = cl.time * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = cl.time * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = cl.time * avelocities[i][2];
//		sr = sin(angle);	// 2001-12-10 Reduced compiler warnings by Jeff Ford
//		cr = cos(angle);	// 2001-12-10 Reduced compiler warnings by Jeff Ford

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		if (!free_particles)
			return;
		p = free_particles;
		
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->frame = 0;
//		p->owner = NULL;
		p->die = cl.time + 0.01;
		p->polor = 0x6f;
		p->type = pt_explode;

		p->org[0] = ent->origin[0] + r_avertexnormals[i][0]*dist + forward[0]*beamlength;
		p->org[1] = ent->origin[1] + r_avertexnormals[i][1]*dist + forward[1]*beamlength;
		p->org[2] = ent->origin[2] + r_avertexnormals[i][2]*dist + forward[2]*beamlength;
	}
}


// an entity on fire!
void R_EntityFireParticles (entity_t *ent, vec3_t org, float sizefix)
{
//	int			count;	// 2001-12-10 Reduced compiler warnings by Jeff Ford
	int			i, g;
	particle_t	*p;
	float		angle;
	float		sp, sy, cp, cy,	sr, cr; 	// 2001-12-10 Reduced compiler warnings by Jeff Ford
	vec3_t		forward, right;
	float		dist;
	float		langth;
	int			moed;
	langth = 5;
	dist = 1;
//	count = 50;	// 2001-12-10 Reduced compiler warnings by Jeff Ford
	

	moed = 1;
	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}

		
	if (cl.time < ent->partfinished)
			return;

	if (!sizefix)
		sizefix = 1.0f;
	ent->partfinished = cl.time + 0.1;

	for (g=0 ; g<3 ; g++)
	{
		angle = cl.time * avelocities[g][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = cl.time * avelocities[g][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = cl.time * avelocities[g][2];
		sr = sin(angle);	// 2001-12-10 Reduced compiler warnings by Jeff Ford
		cr = cos(angle);	// 2001-12-10 Reduced compiler warnings by Jeff Ford

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
		right[0] = cp*cr;
		right[1] = cp*sr;
		right[2] = -sr;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;

		p->die = cl.time + 0.8;
		p->type = pt_smoke;
		p->blend = 8;
		p->frame = 1;
		p->alpha = 1.4f;
		p->scale = 0.2f;
		p->scaley = 0.2f;
		p->alphavel = -1.4;
		p->sprtype = SPR_VP_PARALLEL_ORIENTED;
		p->scalexvel = 1.7;
		p->scaleyvel = 1.2;

		p->angles[PITCH] = 0;
		p->angles[YAW]	 = 0;
		p->angles[ROLL]  = rand()&360;

		p->ramp = 0;
		p->org[0] = org[0];
		p->org[1] = org[1];
		p->org[2] = org[2];
		p->vel[2] = 35 + rand()%66;

		p->angles[0] += ent->angles[0];
		p->angles[1] += ent->angles[1];
		p->angles[2] += ent->angles[2];

		p->polor = 237;
//		p->polor = ramp4[(int)p->ramp];
	//	p->type = pt_fire6;
				
					
	}
	

	// flutter streamer
	for (i=0 ; i<11 ; i++)
	{
		angle = cl.time * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = cl.time * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = cl.time * avelocities[i][2];
		sr = sin(angle);	// 2001-12-10 Reduced compiler warnings by Jeff Ford
		cr = cos(angle);	// 2001-12-10 Reduced compiler warnings by Jeff Ford

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
		right[0] = cp*cr;
		right[1] = cp*sr;
		right[2] = -sr;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;

		p->die = cl.time + 0.8;
		//p->polor = 0x6f;
		p->polor = 44;
		p->type = pt_smoke;
		p->blend = 8;
		p->frame = 1;
		p->alpha = 1.0f;
		p->scale = 0.5f;
		p->scaley = 0.5f;
		p->alphavel = -0.8;
		p->sprtype = SPR_ORIENTED;
		p->scalexvel = 0.4;
		p->scaleyvel = -1.7;

		p->angles[PITCH] = 90;
		p->angles[YAW]	 = rand()&720 - 360;
		p->angles[ROLL]  = rand()&360;

		p->anglevel[PITCH] = -180;
		//p->anglevel[YAW] = -180;
		p->anglevel[ROLL] = rand()&720 - 360;
		//p->vel[2] = 655;
		p->ramp = 0;
//		p->org[0] = ent->origin[0] + r_avertexnormals[i][0]*dist + forward[0]*langth;
//		p->org[1] = ent->origin[1] + r_avertexnormals[i][1]*dist + forward[1]*langth;
//		p->org[2] = ent->origin[2] + r_avertexnormals[i][2]*dist + forward[2]*langth;
		p->org[0] = org[0] + r_avertexnormals[i][0]*dist + forward[0]*langth;
		p->org[1] = org[1] + r_avertexnormals[i][1]*dist + forward[1]*langth;
		p->org[2] = org[2] + r_avertexnormals[i][2]*dist + forward[2]*langth;
		p->vel[2] = 77;

		p->angles[0] += ent->angles[0];
		p->angles[1] += ent->angles[1];
		p->angles[2] += ent->angles[2];

		p->polor = 232;
//		p->polor = ramp4[(int)p->ramp];
	//	p->type = pt_fire6;
				
					
	}
	


}



// leilei - sprite versions!
void R_FireTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	
		dec = 1;
		
//	if (len < 0)
//	{
//		R_EntityFireParticles (ent, ent->origin);		// if not moving, go to the still fire effect
//		return; 
//	}

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time + 2;
		p->sprtype = 4;
		p->blend = 0;
		p->scalexvel = 0;
		p->trail = 0; // damnit
		p->scaleyvel = 0;
		p->alphavel = 0;
		p->scale = 0.3f;
		p->scaley = p->scale;
		p->anglevel[0] = 0;
		p->anglevel[1] = 0;
		p->anglevel[2] = 0;
		p->alpha = 1.7f;
		
		p->type = pt_smoke;
		p->frame = 1;
		p->angles[2] = rand()&450;
		p->alphavel = -1.3f;
		p->blend = 8;
		p->die = cl.time + 0.1f;
		p->scalexvel = -2.3;
		p->polor = 235;
		p->scaleyvel = -2.3;
		p->anglevel[2] = rand()&666 - 333;
		for (j=0 ; j<3 ; j++)
		p->org[j] = start[j] + ((rand()%2)-1);


		if (len > 3){	// embers
			p->scale = 0.1f;
			p->scaley = 0.1f;
			p->alphavel = -0.6f - (rand()%7 * 0.1);
			p->scalexvel = -0.1f;
			p->scaleyvel = -0.1f;
			p->polor = 234;
			p->die = cl.time + 0.8;
			p->sprtype = SPR_ORIENTED;
			
			for (j=0 ; j<3 ; j++){
			p->org[j] = start[j] + ((rand()%5)-2);
			p->vel[j] = rand()%12 - 6;

			p->angles[j] = rand()*1024;
			}
			p->vel[2] += 17;
		}

		VectorAdd (start, vec, start);
	}
}


/*
===============
R_ClearParticles
===============
*/
void R_ClearParticles (void)
{
	int		i;

	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i<r_numparticles ; i++)
		particles[i].next = &particles[i+1];
	particles[r_numparticles-1].next = NULL;
}


void R_ClearFlares (void)
{
	int		i;

	free_flares = &flares[0];
	active_flares = NULL;

	for (i=0 ;i<r_numflares ; i++)
		flares[i].next = &flares[i+1];
	flares[r_numflares-1].next = NULL;
}

void R_ReadPointFile_f (void)
{
	FILE	*f;
	vec3_t	org;
	int		r;
	int		c;
	particle_t	*p;
	char	name[MAX_OSPATH];

	sprintf (name,"maps/%s.pts", sv.name);

	COM_FOpenFile (name, &f, NULL);	// 2001-09-12 Returning from which searchpath a file was loaded by Maddes
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}

	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for ( ;; )
	{
		r = fscanf (f,"%f %f %f\n", &org[0], &org[1], &org[2]);
		if (r != 3)
			break;
		c++;

		if (!free_particles)
		{
			Con_Printf ("Not enough free particles\n");
			break;
		}
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->frame = 0;
//		p->owner = NULL;
		p->die = 99999;
		p->polor = (-c)&15;
		p->type = pt_static;
		VectorCopy (vec3_origin, p->vel);
		VectorCopy (org, p->org);
	}

	fclose (f);
	Con_Printf ("%i points read\n", c);
}

/*
===============
R_ParseParticleEffect

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect (void)
{
	vec3_t		org, dir;
	int			i, count, msgcount, color;

	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	for (i=0 ; i<3 ; i++)
		dir[i] = MSG_ReadChar () * (1.0/16);
	msgcount = MSG_ReadByte ();
	color = MSG_ReadByte ();

if (msgcount == 255)
	count = 1024;
else
	count = msgcount;

if (particlespray)
	R_RunWarticleEffect (org, dir, color, count);
else
	R_RunParticleEffect (org, dir, color, count);
}

/*
===============
R_ParticleExplosion

===============
*/
void R_ParticleExplosion (vec3_t org)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
		p->frame = 0;


		p->die = cl.time + 5;
		p->polor = ramp1[0];
		p->ramp = rand()&3;
		if (i & 1)
		{
			p->type = pt_explode;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
		else
		{
			p->type = pt_explode2;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
	}
}


extern cvar_t *temp2;


// leilei's redo
void R_NormalFromHerePlease (vec3_t org, vec3_t steer);
void R_ParticleExplosionSpritey (vec3_t org)
{
	int			i, j;
	particle_t	*p;
	flare_t		*f;



	for (i=0 ; i<2 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
		p->frame = 0;


		p->die = cl.time + 4.7;
		
		// Fireball
		if (i < 2){
		p->frame = 11;
		p->scale = 0.1;
		p->scalexvel = 4;
		p->scaleyvel = 0;
		p->blend = 1;
		p->alpha = 1.0f;
		p->alphavel = -0.9f;
		p->angles[2] = rand()&360;
		p->anglevel[2] = rand()&360 - 180;
		p->sprtype = 4;
		p->scaley = p->scale;
		p->type = pt_smoke;

			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%64)-32;
			}
		}
		else
		{
		p->frame = 12;
		p->scale = 0.1;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		p->polor = 15;
		p->die = cl.time + 2.7;
		p->blend = 8;
		p->polor = 56 + (rand()%15);
		VectorCopy(org, p->org);
//		R_NormalFromHerePlease(p->org,p->angles);
		p->scaley = p->scale;
		p->alpha = 2.0f;
		p->alphavel = -1.2f - rand()%2;
		p->angles[2] = rand()&360;
		p->type = pt_sparkvel;
		//p->sprtype = 1;	// for directional sprites!
		p->sprtype = 1;	// for directional sprites!
		//p->scaley = 4;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = rand()% 1800 - 900;
			}
		//	p->vel[2] = (rand()%356)-128;
		}
	}

	// then the flare.
	


	
//	else
	
	/*
	if (!free_flares)
			return;
	
	f = free_flares;
	free_flares = f->next;
	f->next = active_flares;
	active_flares = f;
	f->frame = 12;
	f->scale = 0.2;
	f->scalexvel = 6;
	p->scaleyvel = 0;
	f->blend = 3;
	f->polor = 79;
//	f->alpha = 1.2f;
	f->die = cl.time + 0.3;
	f->alphavel = -1.9f;
	f->angles[2] = 0;
	f->anglevel[2] = 0;
	f->sprtype = 5;
	for (j=0 ; j<3 ; j++)
	{
		f->org[j] = org[j];
		f->vel[j] = 0;
	}
	
	
	{
		trace_t tarce;
		tarce = Particle_TraceLine(org, r_refdef.vieworg);
		if(tarce.fraction < 1)
			f->alpha = 0;
		else
			f->alpha = 1;

	}
	*/
}

extern cvar_t *r_particletrails;

/*
===============
R_ParticleTrail

===============
*/
void R_ParticleTrail (vec3_t org, vec3_t veloc, int trl, float tim, int coll, float alf, int blend)
{
	int			i, j;
	particle_t	*p;


		if (!free_particles)
			return;
		if (!r_particletrails->value)
			return;
		if ((cl.time - cl.oldtime) < 0.01)	// you're too fast
			return;
		if ((cl.time - cl.oldtime) > 0.3)	// you're slow
			return;
		return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
		p->die = cl.time + tim;
		p->frame = 0;
		//p->die = cl.time + ( cl.time - cl.oldtime);
		p->polor = coll;
		p->type = pt_trail;
		p->trail = trl - 1;
		p->alpha = alf;
		p->blend = blend;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j];
			p->vel[j] = veloc[j] * -0.5;
		}
}


/*
===============
R_ParticleBloodSplash


  for blood drip maybe
===============
*/
extern sfx_t			*cl_sfx_wizhit;

extern sfx_t			*cl_sfx_bloodhit1;
extern sfx_t			*cl_sfx_bloodhit2;
extern sfx_t			*cl_sfx_bloodhit3;
void R_ParticleBloodSplash (vec3_t org, int veel, int col)
{
	int			i, j;
	float		wa;
	particle_t	*p;

	wa = veel;
	if (wa > 0.6)
		wa = 0.6;
if (wa < 0)
		wa = 0.1;

	veel = veel * -0.2;
	S_StartSound2 (-1, 0, cl_sfx_wizhit, org, 0.2, 3.7, (rand()%12) + 4);
	for (i=0 ; i<3 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
		p->die = cl.time + 0.6;
		p->polor = col;
		p->ramp = rand()&3;
		p->trail = 3;
		if (particleset == 2){
		p->frame = 7;
		p->scale = 0.02f;
		p->scaley = 0.02f;
		p->scalexvel = 1;
		p->scaleyvel = 0;
		p->alphavel = -0.3;
		p->blend = 0;
		p->alpha = 0.5f;
		p->trail = 0;
		}
		else
		{
		p->alpha = 1;
		p->alphavel = 0;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		p->frame = 0;
		p->scale = 1;
		p->scaley = 1;
		}
		
		{
			if (p->polor > 63 && p->polor < 80)
			p->type = pt_bloodsplat;
			else
				p->type = pt_fastgrav;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%2)-1);
				p->vel[j] = (rand()%32)-16;
				
			}
			p->vel[2] += (rand()%veel);
		}
	}
}


void R_ParticleWat (vec3_t org, int colr)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<2 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
		p->die = cl.time + 0.02;
		p->polor = colr;
		
		{
			p->type = pt_static;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j];
				p->vel[j] = (rand()%64)-32;
			}
		}
	}
}
/*
===============
R_ParticleExplosion2

===============
*/
void R_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength)
{
	int			i, j;
	particle_t	*p;
	int			colorMod = 0;

	for (i=0; i<512; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
		p->frame = 0;
		p->die = cl.time + 0.3;
		p->polor = colorStart + (colorMod % colorLength);
		colorMod++;

		p->type = pt_blob;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()%32)-16);
			p->vel[j] = (rand()%512)-256;
		}
	}
}


void R_ParticleBloodSpewism (vec3_t org, int much)
{
	int			i, j;
	vec3_t		fe;
	int			colorMod = 0;

	for (i=1; i<much; i+=2)
	{
		for (j=0 ; j<3 ; j++)
		{
			fe[j] = ((rand()%32)-16);
		}
		fe[2] += 4+i;

		R_RunParticleEffect(org, fe, 79, 3 * i);
	}
}

/*
===============
R_BlobExplosion

===============
*/
void R_BlobExplosion (vec3_t org)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;

		p->die = cl.time + 1 + (rand()&8)*0.05;

		if (i & 1)
		{
			p->type = pt_blob;
			p->polor = 66 + rand()%6;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
		else
		{
			p->type = pt_blob2;
			p->polor = 150 + rand()%6;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
	}
}



/*
===============
R_PlasmaExplosion
used by TE_TEI_PLASMAHIT
===============
*/
void R_PlasmaExplosion (vec3_t org, int colorStart, int colorLength, int count)
{
	int			i, j;
	particle_t	*p;
	int			colorMod = 0;

	for (i=0; i<count; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		// TODO: Implement Spark Trail type
		p->die = cl.time + 0.4;
		p->frame = 0;
		p->polor = colorStart + (colorMod % colorLength);
		colorMod++;

		p->type = pt_decel;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()%4)-2);
			p->vel[j] = (rand()%1500)-750;
		}
	}
}
/*
===============
R_RunParticleEffect



===============
*/

//
// The original Quack particles
//
extern cvar_t *r_particlesparks;
extern cvar_t *r_particletrails;
extern cvar_t *r_particlebloodfade;
extern cvar_t *temp2;
extern cvar_t *s_blood;

void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	int			i, j;
	particle_t	*p;
	int	density;
	if (particleset == 2)
		//density = (1 * (count / 5));	// reduce the spraying for sprites
		density = 8;	// reduce the spraying for sprites
	else
		density = 1;

	if (particleblood < 0  && (color > 63) && (color < 80) && count != 1024)
		return;	 //we disable the blood!

	if (particleblood > 1  && (color > 63) && (color < 80) && count != 1024){
		int	o, m, f, g;
		vec3_t  shoo, ting, tha, tway;
		// Blood Spurt of some sort.

// sound hack
		if (s_blood->value){
		if (count < 15)
			S_StartSound2 (-1, 0, cl_sfx_bloodhit3, org, 0.2, 3.1, (12  / count ));
		else if (count > 44)
			S_StartSound2 (-1, 0, cl_sfx_bloodhit2, org, 0.6, 3.1, (20  / count ));
		else
			S_StartSound2 (-1, 0, cl_sfx_bloodhit3, org, 0.4, 3.1, (44  / count ));
		}
		
		o = (particleblood * 0.3)* count;
		m = o;
	
			for (j=0 ; j<3 ; j++)
			{
				shoo[j] = dir[j] + (rand()%5)-2.5 * -1;
				ting[j] = dir[j] + (rand()%5)-2.5;
				tha[j]	= dir[j] + (rand()%5)-2.5;
				tway[j] = dir[j] + (rand()%5)-2.5;
			}


		for (i=-3 ; i<o ; i+=density)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		//p->flat = 1; // testing
		p->frame = 0;
		p->scale = 0.4f;
		p->scaley = 0.4f;
		p->alpha = 1.0f;
		p->alphavel = -0.5f;
		p->scalexvel = 0.15f * (count / 2.4); // scale upper
		p->scaleyvel = 0.0f;	// don't do sticky drop...
		
		

		p->next = active_particles;
		if (particleset == 2){ p->frame = 8 - rand()%4; p->blend = 10; p->scale = 0.23f; p->scaley = p->scale; p->angles[2] = rand()&360; p->anglevel[2] = rand()&500-250; p->sprtype = 4; }
//		p->owner = NULL;
		active_particles = p;
		{
			p->die = cl.time + 0.6*(rand()%32);
			p->polor =  79 - (rand()&6);
			if (particleblood > 2){
				p->die = cl.time + 0.6*(rand()%32);
			p->type = pt_bloodsplat;
			}
			else
			{
				p->type = pt_slowgrav;
			p->die = cl.time + 0.6*(rand()%2);
			}
	

			if (i<12){
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + (rand()%4)-2;
				p->vel[j] =  dir[j]*(rand()*(count*0.5))-(count); 
				//p->polor = (color&~7) + (rand()&7);
			//		p->polor = 66 + (rand()&12);
			}
			}
			else if (i<28){
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + (rand()&((count/8))-(count/16));
				p->vel[j] = dir[j]*(i * 0.8);
				p->vel[2] += (i / 5);
			}
			}
			else if (i<96){
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + (rand()&((count/8))-(count/16)); // Prydon had a problem with this (fixed now though)
				p->vel[j] = shoo[j]*(i * 0.2); 
				p->vel[2] += (i / 5);
			}
			}
			else if (i<112){
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + (rand()&((count/8))-(count/16));
				p->vel[j] = ting[j]*(i * 0.2);
				p->vel[2] += (i / 5);
			}
			}
			else if (i<128){
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + (rand()&((count/8))-(count/16));
				p->vel[j] = tha[j]*(i * 0.2); 
				p->vel[2] += (i / 5);
			}
			}
			else if (i<132){
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + (rand()&((count/8))-(count/16));
				p->vel[j] = tway[j]*(i * 0.2); 
				p->vel[2] += (i / 5);
			}
			}
			else if (i>200){
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + (rand()&((count/8))-(count/16));
				p->vel[j] = dir[j]* (rand()%i)-(i*0.5);
			}
			}
			else
			{
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j]; 
				p->vel[j] = dir[j]*(i * 0.3);
				p->vel[2] *= 2;
			
			}
			
			}
		
			}
		}
		if (particleset == 2){
			p->scale = count / 48;
			p->scalexvel = p->scale * 0.4;
			p->scaleyvel = 0;
		}
		return;
	}

	if (r_particlesparks->value && (color > 224)){
	for (i=0 ; i<count ; i+=density)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->trail = 14;	// testing
		p->blend = 1;
		p->frame = 0;
		p->die = cl.time + 0.8*(rand()%5);
		p->alphavel = -1;
		p->polor = (color&~7) + (rand()&7);
		//p->polor = color;
		p->type = pt_spark;
			for (j=0 ; j<3 ; j++)
			{
			//	p->org[j] = org[j] + ((rand()&15)-8);
				p->org[j] = org[j];//+ ((rand()&15)-8);
				p->vel[j] = dir[j]*16656 + (rand()%300)-150 ;
			
			}
		}
	return;
	}
	

	for (i=0 ; i<count ; i+=density)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
	//	p->trail = 6;	// testing
		
//		p->splatter = 0;
		p->frame = 0;
		
		if (count == 1024)
		{	// rocket explosion
			p->die = cl.time + 5;
			p->polor = ramp1[0];
			;
			p->ramp = rand()&3;
			if (particleset == 2){ R_ParticleExplosionSpritey(org); return; }			
			if (i & 1)
			{
				p->type = pt_explode;
				for (j=0 ; j<3 ; j++)
				{
					p->org[j] = org[j] + ((rand()%32)-16);
					p->vel[j] = (rand()%512)-256;
				}
			}
			else
			{
				p->type = pt_explode2;
				for (j=0 ; j<3 ; j++)
				{
					p->org[j] = org[j] + ((rand()%32)-16);
					p->vel[j] = (rand()%512)-256;
				}
			}
			

		}
		else
		{
			p->die = cl.time + 0.1*(rand()%5);
			p->polor = (color&~7) + (rand()&7);
			p->type = pt_slowgrav;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()&15)-8);
				p->vel[j] = dir[j]*15;// + (rand()%300)-150;
			
			}
			if (particleblood){
			if ((color > 63) && (color < 80)){
				//p->lit = 1;
				p->type = pt_blood;
				}
			}
			if (particleset == 2){ p->alpha = 1.0f; p->frame = 1; p->blend = 8; p->scalexvel = 3; p->scaleyvel = 0; p->alphavel = -2.9f; p->scale = 0.01f;p->scaley = p->scale; p->angles[2] = rand()&360; p->anglevel[2] = rand()&800-400; p->sprtype = 4; }
		}
		
	}

}



// different alternate 'additive' method
// origin is smaller and velocity is more random
// it also fades and prefers scale over quantity (to be implemented)
// using it depends on your taste.

void R_RunWarticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
		p->frame = 0;
		{
			p->die = cl.time + 0.2*(rand()%5);
			p->polor = color;
			p->type = pt_bloodsplat;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j];
				p->vel[j] = dir[j]*15 + (rand()%50)-25;
			}
					
			}
		}
	
}








// --------------------
// Used by TE_TEI_SMOKE
// --------------------

void R_Smoke (vec3_t org, vec3_t dir, int color, int count)
{
	int			i, j;
	particle_t	*p;
	int		nexte = 1;
	if (sprity) nexte = 8;
	for (i=0 ; i<count ; i+=nexte)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
//		p->owner = NULL;
					p->type = pt_smoke;
		p->frame = 1;
		p->blend = 1;
		R_NormalFromHerePlease(p->org, p->angles);
//		p->vel[0] = p->angles[0];
	//	p->vel[1] = p->angles[1];
//		p->vel[2] = p->angles[2];
//		p->splatter = 0;
		p->sprtype = 4;
		p->angles[2] = rand()&555;
		p->alpha = 1.0f;
		p->scale = 0.02f;
		p->scaley = p->scale;
		p->alphavel = -0.5;
		p->trail = 0;
		p->scalexvel = 0.7;
		p->scaleyvel = 0.5;
		p->anglevel[2] = rand()&1000 - 500;
		{
			if (sprity)
			p->die = cl.time + 2.6*(rand()%5);
			else
				p->die = cl.time + 0.1*(rand()%5);
			p->polor = (color&~7) + (rand()&7);

			for (j=0 ; j<3 ; j++)
			{	
				if (sprity)
					p->org[j] = org[j] + ((rand()%2)-1);
			else
				p->org[j] = org[j] + ((rand()%6)-3);
				p->vel[j] = (rand()%50)-25;
			}
		}
	}
}




void vectoangles2 (vec3_t value1, vec3_t angles);

void vectoangles(vec3_t vec, vec3_t ang)
{
	float	forward;
	float	yaw, pitch;
	
	if (vec[1] == 0 && vec[0] == 0)
	{
		yaw = 0;
		if (vec[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec[1], vec[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec[0]*vec[0] + vec[1]*vec[1]);
		pitch = (int) (atan2(vec[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	ang[0] = pitch;
	ang[1] = yaw;
	ang[2] = 0;
}

// partially butchered from Darkplaces'  CL_SpawnDecalParticleForPoint
void R_Decal (vec3_t org, int decframe, int blend, float sceel, int splat)
{
	int			i, j;
	particle_t	*p;
	int		nexte = 1;
	int	count = 1;
	
	trace_t trace;

	float bestfrac, bestorg[3], bestnormal[3];
	float org2[3];
	float	fraccy = 0;
	vec3_t	heer;
	vec3_t	nermal;
	vec3_t	teer;
	int besthitent = 0, hitent;
	int	maxdist = 9;
	bestfrac = 10;
	//decframe = 4;
	
		for (i = 0;i < 32;i++)
	{
		org2[0] = rand()%4 - rand()%2;
		org2[1] = rand()%4 - rand()%2;
		org2[2] = rand()%4 - rand()%2;
		VectorMA(org, maxdist, org2, org2);
		trace = Particle_TraceLine(org, org2);
		// take the closest trace result that doesn't end up hitting a NOMARKS
		// surface (sky for example)
		if (bestfrac > trace.fraction)
		{
			bestfrac = trace.fraction;
		//	besthitent = hitent;
			VectorCopy(trace.endpos, bestorg);
			VectorCopy(trace.plane.normal, bestnormal);
			
		}
	}

			
	if (bestfrac < 1)		// it's time to create.
	{
		
	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->type = pt_decal;
		p->frame = decframe;
		p->blend = blend;
//		p->splatter = 0;
		p->sprtype = 3;
		
		p->alpha = 8.0f;
		
			if (!sceel)
				p->scale = 0.4f;
				else
		p->scale = sceel;
		p->scaley = p->scale;
		p->alphavel = 0;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		p->anglevel[0] = 0;
		p->trail = 0;
		p->anglevel[1] = 0;
		p->anglevel[2] = 0;
		p->die = cl.time + 76;
		
		p->angles[0] = -bestnormal[0];
		p->angles[1] = -bestnormal[1];
		p->angles[2] = -bestnormal[2];
		p->angles[2] *= -1;
		if (splat){
		p->type = pt_bloodrun;
		p->blend = blend;
		}

	//	Con_Printf("%f yes\n",p->angles[2]);
		vectoangles(p->angles, p->angles);
		
			for (j=0 ; j<3 ; j++)
			{	
				p->org[j] = bestorg[j];
			
				p->vel[j] = 0;
			}


	
	}
	}
}



void R_NormalFromHerePlease (vec3_t org, vec3_t steer)
{
	int			i, j;
	particle_t	*p;
	int		nexte = 1;
	int	count = 1;
	
	trace_t trace;

	float bestfrac, bestorg[3], bestnormal[3];
	float org2[3];
	float org4[3];
	float	fraccy = 0;
	vec3_t	heer;
	vec3_t	nermal;
	vec3_t	teer;
	
	int besthitent = 0, hitent;
	int	maxdist = 9;
	bestfrac = 10;
	//decframe = 4;
	org4[0] = org[0];
	org4[1] = org[1];
	org4[2] = org[2];
		for (i = 0;i < 32;i++)
	{
		org2[0] = rand()%4 - rand()%2;
		org2[1] = rand()%4 - rand()%2;
		org2[2] = rand()%4 - rand()%2;

		VectorMA(org4, maxdist, org2, org2);
		trace = Particle_TraceLine(org4, org2);
		// take the closest trace result that doesn't end up hitting a NOMARKS
		// surface (sky for example)
		if (bestfrac > trace.fraction)
		{
			bestfrac = trace.fraction;
		//	besthitent = hitent;
			VectorCopy(trace.endpos, bestorg);
			VectorCopy(trace.plane.normal, bestnormal);
			
		}
	}

			
	if (bestfrac < 1)		// it's time to create.
	{
		
		steer[0] = bestnormal[0];
		steer[1] = bestnormal[1];
		steer[2] = bestnormal[2];
		//steer[2] *= -1;
		//org[0] = bestorg[0];	// don't return orgies if we don't want them
		//org[1] = bestorg[1];
		//org[2] = bestorg[2];
		vectoangles(steer, steer);

	}
}



int R_GimmeTheColorOfThisWall (vec3_t org)
{
	int			i, j;
	particle_t	*p;
	int		nexte = 1;
	int	count = 1;
	int color;
	trace_t trace;

	float bestfrac, bestorg[3], bestnormal[3];
	float org2[3];
	float	fraccy = 0;
	vec3_t	heer;
	vec3_t	nermal;
	vec3_t	teer;
	
	int besthitent = 0, hitent;
	int	maxdist = 9;
	bestfrac = 10;
	//decframe = 4;
	
		for (i = 0;i < 32;i++)
	{
		org2[0] = rand()%4 - rand()%2;
		org2[1] = rand()%4 - rand()%2;
		org2[2] = rand()%4 - rand()%2;
		VectorMA(org, maxdist, org2, org2);
		trace = Particle_TraceLine(org, org2);
		if (bestfrac > trace.fraction)
		{
			bestfrac = trace.fraction;
		}
	}

			
	if (bestfrac < 1)		// it's time to create.
	{
		color = trace.color;
		return color;
	}
}


void R_FlareInstant (vec3_t org, int decframe, int colr, int colg, int colb, int tayp)
{
	int			i, j;
	particle_t	*p;
	int		nexte = 1;
	int	count = 1;
	int	gmcol;


	for (i=0 ; i<1 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;

		p->frame = decframe;
		p->blend = 5;
		gmcol = BestColor(colr, colg, colb, 12, 222);	// determine color from bestcoloring
		p->polor = gmcol;
		p->color = gmcol;


		// determine initial visibility first so we don't see exploded flares on spawn
		p->sprtype = tayp;
		p->angles[2] = 0; // will random these.
		//p->alpha = 1.0f;
		p->alpha = 1.0f; // we are drawing in front of everything anyway.		
		p->alphavel = 0;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		p->anglevel[0] = 0;
		p->anglevel[1] = 0;
		p->anglevel[2] = 0;
					
		p->type = pt_flare_will_die;
			p->scale = (p->org[0] - r_origin[0])*vpn[0] + (p->org[1] - r_origin[1])*vpn[1]
			+ (p->org[2] - r_origin[2])*vpn[2];
		if (p->scale < 40)
			p->scale = 0.3;
		else
			p->scale = 0.3 + p->scale * 0.004;
		}
		p->scaley = p->scale;
		{
			p->die = cl.time + 0.08; // for now
			for (j=0 ; j<3 ; j++)
			{	
				p->org[j] = org[j];
				p->vel[j] = 0;
			}
		}




	
}

extern cvar_t *r_flares;

//	leilei - experimental unr**l-style flares
void R_FlareTest (vec3_t org, int decframe, int colr, int colg, int colb, float gonnjardie, entity_t *ownme)
{
	int			i, j;
	flare_t	*p;
	int		nexte = 1;
	int	count = 1;
	int	gmcol;
	int	ownin;
	int	gonnadie = 0;
	float	initialalpha;
	trace_t	tarce;
	if (!r_flares->value)
		return;	// don't draw flares if we don't want flares.
	decframe = 12;
	if (cls.demoplayback)
		return;	// because my checks for visibility cause a crash in demo playback. :(
	if (ownme == NULL)
		ownin = 0;
	else
		ownin = 1;

	if (ownin)
	if (ownme->ourparticle)
		return;	//we got one.
		

	// determine if we've spawned in a wall like an idiot would do.
	
	{
		mleaf_t		*o;
		o = Mod_PointInLeaf (org, cl.worldmodel);
		if (o->contents != CONTENTS_EMPTY )
			return;
		}


	
//	else
	
	
	{
//		tarce = Particle_TraceLine(org, r_refdef.vieworg);
//		if(tarce.fraction < 1)
/*		if(p->amiseen)
			initialalpha = 0;
		else{
			if (gonnjardie)
			initialalpha = gonnjardie;
				else
			initialalpha = 1;
		}
*/
	}



		//if (){


	for (i=0 ; i<count ; i++)
	{
		if (!free_flares)
			return;
		p = free_flares;
		free_flares = p->next;
		p->next = active_flares;
		p->owner = NULL;
		active_flares = p;

		p->frame = decframe;
		p->blend = 5;
	//	if (gonnjardie)
		D_TestOurFlare(p);
		if(!p->amiseen)
			initialalpha = 0;
		else{
			if (gonnjardie)
			initialalpha = gonnjardie;
				else
			initialalpha = 1;
		}

		// determine color. we are going to use gelmaps for flares.
		gmcol = BestColor(colr, colg, colb, 12, 222);	// determine color from bestcoloring
	//	gmcol = 27;

		p->polor = gmcol;
		p->color = gmcol;

//		if (ownme != NULL)
		if (ownin){
		p->owner = ownme;
//		p->oldmodel = ownme->model;
		}
		// determine initial visibility first so we don't see exploded flares on spawn
		{
		if((Traceline(p->org, r_refdef.vieworg, NULL, NULL)))
		p->alpha = 1.0f;
		else
		p->alpha = 0.02f;


		}
//		p->splatter = 0;
	//	
		p->blend = 5;
		p->sprtype = 5;
		p->angles[2] = 0; // will random these.
		p->alpha = initialalpha;
		if (ownin){
		p->owner = ownme;
		ownme->ourparticle = p;
		p->owned = 1;	// OWNED.
		}
		p->alphavel = 0;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		p->anglevel[0] = 0;
		p->anglevel[1] = 0;
		p->anglevel[2] = 0;
					
		p->type = pt_flare;
		p->scale = 0;
		p->scaley = p->scale;
	
		{
			p->die = cl.time + 7500; // for EVER!

			for (j=0 ; j<3 ; j++)
			{	
				p->org[j] = org[j];
				p->vel[j] = 0;
			}
		}



	//Con_Printf("FAT!!! %f %f %f\n", p->org[0], p->org[1], p->org[2]	);

	}
}




void R_FlareInstant2 (vec3_t org, int decframe, int colr, int colg, int colb, float alpfer)
{
	int			i, j;
	flare_t	*p;
	int		nexte = 1;
	int	count = 1;
	int	gmcol;
	int	ownin;
	int	gonnadie = 0;
	float	initialalpha;
	trace_t	tarce;
#ifndef GLQUAKE
	if (!r_flares->value && reflectpass)
		return;	// don't draw flares if we don't want flares.
#endif
	decframe = 12;
	{
		trace_t tarce;
		tarce = Particle_TraceLine(org, r_refdef.vieworg);
		if(tarce.fraction < 1)
			return;

	}	
//	return;
/*
	// determine if we've spawned in a wall like an idiot would do.
	{
		mleaf_t		*o;
		o = Mod_PointInLeaf (org, cl.worldmodel);
		if (o->contents != CONTENTS_EMPTY )
			return;
		}


*/	
		if (!free_flares)
			return;
		p = free_flares;
		free_flares = p->next;
		p->next = active_flares;
		p->owner = NULL;
		active_flares = p;

		p->frame = 12;
		p->blend = 5;

		gmcol = BestColor(colr, colg, colb, 12, 222);	// determine color from bestcoloring

		p->polor = gmcol;
		p->color = gmcol;

		p->sprtype = 5;
		p->angles[2] = 0; // will random these.
		if (alpfer)
		p->alpha = alpfer;
			else
		p->alpha = 1.0f;
		p->alphavel = 0;
		p->owned = 0;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		p->anglevel[0] = 0;
		p->anglevel[1] = 0;
		p->anglevel[2] = 0;
		p->scale = 0.1;			
		p->type = pt_flare_will_die;
		p->scaley = p->scale;

	
		{
			p->die = cl.time + 0.001; // for now
			for (j=0 ; j<3 ; j++)
			{	
				p->org[j] = org[j];
				p->vel[j] = 0;
			}
		}


	//Con_Printf("FAT!!! %f %f %f\n", p->org[0], p->org[1], p->org[2]	);


}





// --------------------
// Used for the engine splash calls (by r_particle_splash)
// --------------------

void R_Splash (vec3_t org, vec3_t dir, int color, int count, int power)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
//		p->splatter = 0;
		p->frame = 0;
		p->trail = 0;
		
		if (count == 1024)
		{	// rocket explosion
			p->die = cl.time + 5;
			p->polor = ramp1[0];
			p->ramp = rand()&3;
			if (i & 1)
			{
				p->type = pt_smoke;
				for (j=0 ; j<3 ; j++)
				{
					p->org[j] = org[j] + ((rand()%32)-16);
					p->vel[j] = (rand()%512)-256;
				}
			}
			else
			{
				p->type = pt_smoke;
				for (j=0 ; j<3 ; j++)
				{
					p->org[j] = org[j] + ((rand()%32)-16);
					p->vel[j] = (rand()%512)-256;
				}
			}
		}
		else
		{
			p->die = cl.time + 0.4*(rand()%5);
			p->polor = color;
			p->type = pt_blob;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j];
				p->vel[j] = (rand()%350)-(125);
			}
		}
	}
}

/*
===============
R_LavaSplash

===============
*/

void R_LavaSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i++)
		for (j=-16 ; j<16 ; j++)
			for (k=0 ; k<1 ; k++)
			{
				if (!free_particles)
					return;
				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
//				p->owner = NULL;
				active_particles = p;

				p->die = cl.time + 2 + (rand()&31) * 0.02;
				p->polor = 224 + (rand()&7);
				p->type = pt_slowgrav;
				p->frame = 0;
				dir[0] = j*8 + (rand()&7);
				dir[1] = i*8 + (rand()&7);
				dir[2] = 256;

				p->org[0] = org[0] + dir[0];
				p->org[1] = org[1] + dir[1];
				p->org[2] = org[2] + (rand()&63);

				VectorNormalize (dir);
				vel = 50 + (rand()&63);
				VectorScale (dir, vel, p->vel);
			}
}



/*
===============
R_Blood
===============
*/
void R_Blood(int count, vec3_t mins, vec3_t maxs, vec3_t vel_mins, vec3_t vel_maxs)
{
	const int color = 73;
	int idist[3], vdist[3];
	int i, j;
	particle_t *p;

	for (i = 0; i < 3; i++)
	{
		idist[i] = (int)(maxs[i] - mins[i] + 0.1f);
		vdist[i] = (int)(vel_maxs[i] - vel_mins[i] + 0.1f);

		if (idist[i] < 1)
			idist[i] = 1;
		if (vdist[i] < 1)
			vdist[i] = 1;
	}

	for (i = 0; i < count; i++)
	{
		if (!free_particles)
			break;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		//p->die = cl.time + 0.1*(rand()%5);
		p->die = cl.time + 0.1*(rand()%65);
		p->polor = (float)((color&~7) + (rand()&7));
		//p->splatter = 3;
		//p->lit = 1;
		p->type = pt_bloodsplat;
		for (j = 0; j < 3; j++)
		{
			p->org[j] = mins[j] + rand() % idist[j] + ((rand()&15)-8);
			p->vel[j] = vel_mins[j] + rand() % vdist[j];
		}
	}
}



/*
===============
R_Rain

  Tomazquake code adopted
===============
*/
void R_Rain (vec3_t min, vec3_t max, int drops)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	
	for (i=0 ; i<drops ; i++)
	{
		if (!free_particles)
			break;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		VectorSubtract(max, min, difference);
		p->die				= cl.time + 10;
		
		p->alpha			= 0.4f;

		p->polor			= 8;
		if (particleset == 2)
		p->trail			= 0;
			else
		p->trail			= 6;
		p->type				= pt_drip;
		p->scale			= 0.01f;
		p->scaley			= 2;
		p->scalexvel		= 0;
		p->scaleyvel		= 3;
		p->frame			= 3;
		p->blend			= 1;
		p->sprtype			= 3;
		
		p->org[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->org[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->org[2]		= max[2] - 10;

		p->vel[0]		= 0;
		p->vel[1]		= 0;
		p->vel[2]		= -400 - (rand() & 600);
	}
	
}


void R_Snow (vec3_t min, vec3_t max, int drops)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	for (i=0 ; i<drops ; i++)
	{
		if (!free_particles)
			break;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		VectorSubtract(max, min, difference);
		p->die				= cl.time + 10;
		
		p->alpha			= 0.4f;

		p->polor			= 14;
		p->trail			= 0;
		p->alpha			= 0.27f;
		p->blend			= 1;
		p->type				= pt_snow;

		if (particleset == 2){
		p->scale			= 0.05f;
		p->scaley			= 0.05f;
		p->scalexvel		= 0;
		p->scaleyvel		= 0;
		p->frame			= 3;
		p->blend			= 1;
		
	}	

		p->org[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->org[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->org[2]		= max[2] - 10;

		p->vel[0]		= 0;
		p->vel[1]		= 0;
		p->vel[2]		= -40;
	}
	
}

/*
===============
R_TeleportSplash

===============
*/
void R_TeleportSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i+=4)
		for (j=-16 ; j<16 ; j+=4)
			for (k=-24 ; k<32 ; k+=4)
			{
				if (!free_particles)
					return;
				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
//				p->owner = NULL;
				p->alpha = 1.0f; // leilei - don't alpha!
				active_particles = p;
				p->frame = 0;
				p->die = cl.time + 0.2 + (rand()&7) * 0.02;
				p->polor = 7 + (rand()&7);
				p->type = pt_slowgrav;

				dir[0] = j*8;
				dir[1] = i*8;
				dir[2] = k*8;

				p->org[0] = org[0] + i + (rand()&3);
				p->org[1] = org[1] + j + (rand()&3);
				p->org[2] = org[2] + k + (rand()&3);

				VectorNormalize (dir);
				vel = 50 + (rand()&63);
				VectorScale (dir, vel, p->vel);
			}
}


/*
======
QUAKE2 SOURCE
vectoangles2 - this is duplicated in the game DLL, but I need it here.
======
*/
void vectoangles2 (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
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
	// PMM - fixed to correct for pitch of 0
		if (value1[0])
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = 270;

		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

/*
===============
QUAKE2 SOURCE
R_CurlyBloodTrail

	Adapted from Quake2's CL_TrackerTrail for the unused Disintegrator/Disruptor
  weapon for q2 rogue
===============
*/
void R_CurlyBloodTrail (vec3_t start, vec3_t end, int particleColor)
{
	vec3_t		move;
	vec3_t		vec;
	vec3_t		forward,right,up,angle_dir;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	float		dist;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorCopy(vec, forward);
	vectoangles2 (forward, angle_dir);
	AngleVectors (angle_dir, forward, right, up);

	dec = 3;
	VectorScale (vec, 3, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		VectorClear (p->vel);
		p->frame = 0;
		p->die = cl.time + 7.5;
		p->type = pt_bloodsplat;
		p->alpha = 1.0;
		p->blend = 0;
		if (particleset == 2)
			p->frame = 7;
		//p->alphavel = -2.0;
		p->alphavel = 0;p->scalexvel = 0;p->scaleyvel = 0;p->anglevel[0] = 0;p->anglevel[1] = 0;p->anglevel[2] = 0;
		p->polor = 75;
		p->scaley = p->scale;
		dist = DotProduct(move, forward);
		VectorMA(move, 7 * cos(dist), up,	  p->org);
		VectorMA(move, 7 * cos(dist), right, p->org);
		p->scalexvel = 0.3f;
		
		for (j=0 ; j<3 ; j++)
		{
	//		p->org[j] = start[j] + ((rand()%6)-3);
			//p->vel[j] = 100 * cos(dist) + (100 * sin(dist));
			
//			p->vel[j] = 0;
			//p->accel[j] = 0;

		}
		p->vel[0] += 22 * sin(dist);
		p->vel[1] += 22 * sin(dist);
		p->vel[2] += 22 * cos(dist);
		//p->vel[2] = 5;

		VectorAdd (move, vec, move);
	}
}

// f;or the plarticle blood vlariable
void R_GusherizeCurlyBloodTrail (vec3_t start, vec3_t end, int particleColor)
{
	vec3_t		move;
	vec3_t		vec;
	vec3_t		forward,right,up,angle_dir;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	float		dist;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorCopy(vec, forward);
	vectoangles2 (forward, angle_dir);
	AngleVectors (angle_dir, forward, right, up);

	dec = (56 / particleblood);
	VectorScale (vec, 3, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		VectorClear (p->vel);
		p->alpha = 1; p->scale = defaultscale; p->alphavel = 0;p->scalexvel = 0;p->scaleyvel = 0;p->anglevel[0] = 0;p->anglevel[1] = 0;p->anglevel[2] = 0;
		
		p->scaley = p->scale;
		p->die = cl.time + 7.5;
		p->type = pt_bloodsplat;
		p->alpha = 1.0;
		
		p->frame = 0;
		if (particleset == 2)
			p->frame = 7;
		//p->alphavel = -2.0;
		p->polor = 75;
		
		dist = DotProduct(move, forward);
		VectorMA(move, len * 5 * cos(dist), up,	  p->org);
		VectorMA(move, len * 5 * cos(dist), right, p->org);
		p->blend = 10;
		p->alphavel = -0.5f;
		for (j=0 ; j<3 ; j++)
		{
		//	p->org[j] = start[j] + ((rand()len)-(len * 0.5));
			//p->vel[j] = 100 * cos(dist) + (100 * sin(dist));
			
//			p->vel[j] = 0;
			//p->accel[j] = 0;

		}
		p->vel[0] += 12 * sin(dist);
		p->vel[1] += 12 * sin(dist);
		p->vel[2] += 12 * cos(dist);
		p->vel[0] *= vec[0];
		p->vel[1] *= vec[1];
		p->vel[2] *= vec[2];

		p->scalexvel = p->vel[2] * 0.08;
		p->scaleyvel = 0;
		//p->vel[2] = 5;

		VectorAdd (move, vec, move);
	}
}

void R_RocketTrail (vec3_t start, vec3_t end, int type)
{
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	if (type < 128)
		dec = 3;
	else 	if (type == 7)
		dec = 1;
	else if (type == 2 && particleblood)
		dec = 1;
	else
	{
		dec = 1;
		type -= 128;
	}

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		p->trail = 0;
		p->scale = 1;
		p->scaley = p->scale;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time + 2;

		switch (type)
		{
			case 0:	// rocket trail
				p->ramp = (rand()&3);
				p->polor = ramp3[(int)p->ramp];
				p->type = pt_fire;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;

			case 1:	// smoke smoke
				p->ramp = (rand()&3) + 2;
				p->polor = ramp3[(int)p->ramp];
				p->type = pt_fire;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;

			case 2:	// blood
				if (particleblood < 0)
					break;
				if (particleset == 2)
					p->frame = 7;
				if (particleblood > 2){
				p->type = pt_bloodsplat;
				p->polor = 77 + (rand()&3);
				
				p->die = cl.time + (1 *(rand()%6));
				if (len < 1){
					for (j=0 ; j<3 ; j++){
					p->org[j] = start[j] + ((rand()&4)-2);
					p->vel[j] =  ((rand()%100)-50) * (len * 2);
					}
					
					p->vel[2] *= len;
					
					
				}
					else
					{
					for (j=0 ; j<3 ; j++){
					p->org[j] = start[j] + ((rand()*(len))-(len*0.5));
					//p->vel[j] += len;
					p->die = cl.time + 0.6*(rand()%5);
					}
					
				//	p->vel[2] -= len * 2;
			
					
					}
				

					
				}
				else
				{
				p->type = pt_grav;
				p->polor = 67 + (rand()&3);
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				}
				break;

			case 3:
			case 5:	// tracer
				p->die = cl.time + 0.5;
				p->type = pt_static;
				if (type == 3)
					p->polor = 52 + ((tracercount&4)<<1);
				else
					p->polor = 230 + ((tracercount&4)<<1);

				tracercount++;

				VectorCopy (start, p->org);
				if (tracercount & 1)
				{
					p->vel[0] = 30*vec[1];
					p->vel[1] = 30*-vec[0];
				}
				else
				{
					p->vel[0] = 30*-vec[1];
					p->vel[1] = 30*vec[0];
				}
				break;

			case 4:	// slight blood
				if (particleblood < 0)
					break;
				else
				{
					if (particleset == 2)
					p->frame = 7;
				p->type = pt_grav;
				p->polor = 67 + (rand()&3);
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				len -= 3;
				}
				break;

			case 6:	// voor trail
				p->polor = 9*16 + 8 + (rand()&3);
				p->type = pt_static;
				p->die = cl.time + 0.3;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()&15)-8);
				break;
			case 7:	// rail trail smoke
				p->ramp = (rand()&3) + 2;
				p->polor = ramp3[(int)p->ramp];
				p->alpha = 1;
				p->alphavel = -3.7 - rand()%1;
				p->type = pt_staticfade;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;
		}
	

		VectorAdd (start, vec, start);
	}
}


// leilei - sprite versions!
void R_RocketTrailSprites (vec3_t start, vec3_t end, int type)
{
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	if (type < 128)
		dec = 32;
	else 	if (type == 7)
		dec = 6;
	else if (type == 2 && particleblood)
		dec = 7;
	else
	{
		dec = 1;
		type -= 128;
	}

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time + 2;
		p->sprtype = 4;
		p->blend = 0;
		p->scalexvel = 0;
		p->trail = 0; // damnit
		p->scaleyvel = 0;
		p->alphavel = 0;
		p->scale = 0.2f;
		p->scaley = p->scale;
		p->anglevel[0] = 0;
		p->anglevel[1] = 0;
		p->anglevel[2] = 0;
		p->alpha = 1.0f;
		

		switch (type)
		{
			case 0:	// rocket trail
			//	p->ramp = (rand()&3);
			//	p->polor = ramp3[(int)p->ramp];
				p->type = pt_smoke;
				p->frame = 1;
				p->angles[2] = rand()&450;
				p->scale = 0.03f;
				p->blend = 1;
				p->alphavel = -0.37f;
				p->scalexvel = 0.7;
				p->anglevel[2] = rand()&666 - 333;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;

			case 1:	// smoke smoke
			//	p->ramp = (rand()&3);
			//	p->polor = ramp3[(int)p->ramp];
				p->type = pt_smoke;
				p->frame = 1;
				p->angles[2] = rand()&450;
				p->alphavel = -0.37f;
				p->blend = 1;
				p->scale = 0.03f;
				p->scalexvel = 0.5;
				p->anglevel[2] = rand()&666 - 333;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;

			case 2:	// blood
				if (particleblood < 0)
					break;
				if (particleblood > 2){
				p->type = pt_bloodsplat;
				if (particleset == 2)
					p->frame = 7;
				p->polor = 77 + (rand()&3);
				p->blend = 0;
				p->die = cl.time + (1 *(rand()%6));
				if (len < 1){
					for (j=0 ; j<3 ; j++){
					p->org[j] = start[j] + ((rand()&4)-2);
					p->vel[j] =  ((rand()%100)-50) * (len * 2);
					}
					
					p->vel[2] *= len;
					
					
				}
					else
					{
					for (j=0 ; j<3 ; j++){
					p->org[j] = start[j] + ((rand()*(len))-(len*0.5));
					//p->vel[j] += len;
					p->die = cl.time + 0.6*(rand()%5);
					}
					
				//	p->vel[2] -= len * 2;
			
					
					}
				

					
				}
				else
				{
				p->type = pt_grav;
				p->polor = 67 + (rand()&3);
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				}
				break;

			case 3:
			case 5:	// tracer
				p->die = cl.time + 0.5;
				p->type = pt_static;
				if (type == 3)
					p->polor = 52 + ((tracercount&4)<<1);
				else
					p->polor = 230 + ((tracercount&4)<<1);

				tracercount++;

				VectorCopy (start, p->org);
				if (tracercount & 1)
				{
					p->vel[0] = 30*vec[1];
					p->vel[1] = 30*-vec[0];
				}
				else
				{
					p->vel[0] = 30*-vec[1];
					p->vel[1] = 30*vec[0];
				}
				break;

			case 4:	// slight blood
				if (particleblood < 0)
					break;
				else
				{
					if (particleset == 2)
					p->frame = 7;
					p->blend = 0;
				p->type = pt_grav;
				p->polor = 67 + (rand()&3);
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				len -= 3;
				}
				break;

			case 6:	// voor trail
				p->polor = 9*16 + 8 + (rand()&3);
				p->type = pt_static;
				p->die = cl.time + 0.3;
				if (particleset == 2){
					p->frame = 1;
					p->scalexvel = 2;
					p->alphavel = -0.4;
					p->anglevel[2] = rand()&600-300;
					p->angles[2] = rand()&360;
					p->blend = 8;
				}
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()&15)-8);
				break;
			case 7:	// rail trail smoke
				p->ramp = (rand()&3) + 2;
				p->polor = ramp3[(int)p->ramp];
				p->alpha = 1;
				p->alphavel = -3.7 - rand()%1;
				p->type = pt_staticfade;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;
		}
	

		VectorAdd (start, vec, start);
	}
}

// LEI - particle trails 2

void R_ParticleTrail2 (vec3_t start, vec3_t end, int type, int blend, int color, float alpha)
{
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;
	int       thetrail;

	thetrail = r_particletrails->value;
	if (!thetrail)
		return;
	if (thetrail)
	{
		D_DrawSparkTrans(start,end,alpha,color, blend);
			return;
	}
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	{
		dec = 8;
		type -= 128;
	}

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;

		active_particles = p;

		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time + 0.02;
		p->frame = 0;
		p->type = pt_static;
		p->polor = color;
		p->blend = blend;
		p->trail = 0;	// do not ecurse
		p->alpha = len * 0.3;
			for (j=0 ; j<3 ; j++)
					p->org[j] = start[j];
		len -= 6;
		
		
		
		
	

		VectorAdd (start, vec, start);
	}
}

// LEI - lame particle beam
// (TODO: Quake2 polygon beam)
void R_ParticleBeam (vec3_t start, vec3_t end, int thecol)
{
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;
	int	wat;
	//int	thecol;

	

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	dec = 1;

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time;
	
	//	wat = rand()&2;
		{
		/*	if (wat == 2){	// Outer Glow
				p->polor = thecol;
				p->type = pt_staticfadeadd;
				p->die = cl.time + 2;
				p->alpha = 0.6;
				p->alphavel = -0.7;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%4)-2);
			}
			else */
			{	// Inner Line
				p->polor = thecol;
				p->type = pt_staticfadeadd;
				p->die = cl.time + 0.01;
				p->alpha = 1;

				p->alphavel = -0.9;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j];
			}
		}


		VectorAdd (start, vec, start);
	}
}


// uses the beam texture
void R_BeamBeam (vec3_t start, vec3_t end, int thecol, float thesize)
{
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;
	//int	thecol;
	vec3_t	angel;
	

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	dec = 1;

	vectoangles2 (vec, angel);


	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;

		active_particles = p;
		p->frame = 0;
		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time;
		p->frame = 18;
		p->blend = 8;
		p->sprtype = 98;
		p->angles[0] = angel[0] + 90;
		p->angles[1] = angel[1];
		p->angles[2] = angel[2];
		//p->angles[0] = 0;
		//p->angles[1] = 0;
		//p->angles[2] = 0;
		p->scale = 0.03f;

		{
				p->polor = thecol;
				p->type = pt_staticfadeadd;
				p->die = cl.time + 0.01;
				p->alpha = 1;

				p->alphavel = -0.9;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j];
		}


		VectorAdd (start, vec, start);
	}
}


// uses the beam texture
void R_LightningBeam (vec3_t start, vec3_t end, float sized)
{
	vec3_t		vec;
	vec3_t		vec2;
	float		len;
	float		len2;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;
	int	thecol = 214;
	float	dert;
	float	dert2;
	vec3_t	angel;
	

	vec3_t	divert;

	VectorSubtract (end, start, vec);
	VectorSubtract (start, end, vec2);
	len = VectorNormalize (vec);
	len2 = VectorNormalize (vec2);
	if (!sized)
		sized = 1;
	dec = 1;

	vectoangles2 (vec, angel);


	while (len > 0)
	{
		len -= dec;
		//dert = sin(len2);

		//dert2 = 1 - (len / len2);
		dert = (len / len2);
		dert2 = 1 - dert;

		if (dert > 1)dert = 1;
		if (dert2 > 1)dert2 = 1;
		if (dert2 < 0)dert2 = 0;
		if (dert < 0)dert = 0;

		divert[0] += rand()%160 - 80;
		divert[1] += rand()%160 - 80;
		divert[2] += rand()%160 - 80;



		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;

		active_particles = p;
		p->frame = 0;
		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time;
		p->frame = 12;
		p->blend = 8;
		p->sprtype = 1;
		
		p->angles[0] = angel[0] + 90;
		p->angles[1] = angel[1];
		
		p->vel[0] = 0;
		p->vel[1] = 0;
		p->vel[2] = 0;
		p->angles[2] = angel[2];
		//p->angles[0] = 0;
		//p->angles[1] = 0;
		//p->angles[2] = 0;
		p->scale = 0.3f * sized * dert * dert2;
		p->scalexvel = -0.7f;
		p->scaleyvel = -0.7f;
		p->scaley = p->scale;
		
		{
				p->polor = thecol;
				p->type = pt_staticfadeadd;
				p->die = cl.time + 0.03;
				p->alpha = 1.0f;

				//p->alphavel = -0.9;
				for (j=0 ; j<3 ; j++){
					p->org[j] = start[j];
					p->vel[j] += (divert[j] * dert2 * dert);
					p->org[j] += (divert[j] * (dert2 * dert * 0.03f));
				}
		}


		VectorAdd (start, vec, start);
	}
}
/*
===============
R_RailTrail
Used by TE_RAILTRAIL
i'm a quake2 function short and stout
===============
*/


void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up)
{
	float		d;

	// this rotate and negat guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct (right, forward);
	VectorMA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}



void R_RailTrail (vec3_t start, vec3_t end)
{
vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	float		dec;
	vec3_t		right, up;
	int			i;
	float		d, c, s;
	vec3_t		dir;
	byte		clr = 40;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	MakeNormalVectors (vec, right, up);

	for (i=0 ; i<len ; i++)
	{
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;
		p->die = cl.time + 2.0;
		VectorClear (p->vel);

		d = i * 0.1;
		c = cos(d);
		s = sin(d);

		VectorScale (right, c, dir);
		VectorMA (dir, s, up, dir);
		p->type = pt_staticfadeadd;
		p->alpha = 1.0;
	//	p->alphavel = -1.0 / (1+frand()*0.2);
		p->alphavel = -1.0;

		p->polor = clr + (rand()&7);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + dir[j]*3;
			p->vel[j] = dir[j]*6;
		}

		VectorAdd (move, vec, move);
	}

	dec = 0.75;
	VectorScale (vec, dec, vec);
	VectorCopy (start, move);

}




void R_SpiralBloodTrail (vec3_t start, vec3_t end)
{
vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	float		dec;
	vec3_t		right, up;
	int			i;
	float		d, c, s;
	vec3_t		dir;
	byte		clr = 40;
	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	
	len = VectorNormalize (vec);
	len *= 4;
	MakeNormalVectors (vec, right, up);
	dec = 24;
//	for (i=0 ; i<len ; i++)
//	{
	while (len > 0)
	{
		len -= dec;
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
//		p->owner = NULL;
		active_particles = p;
		p->frame = 0;		
		p->die = cl.time + 2.0;
		VectorClear (p->vel);
		i = len * 6;
		d = i * 0.1;
		c = cos(d);
		s = sin(d);
		
		VectorScale (right, c, dir);
		VectorMA (dir, s, up, dir);
		p->type = pt_bloodsplat;
		p->alpha = 1.0;
		p->scale = 0.1;
		p->blend = 0;

		if (particleset == 2){

	//	p->alphavel = -1.0 / (1+frand()*0.2);
			p->scalexvel = 0.8;
			p->alphavel = -3.0;
			p->frame = 7;
			
		}

		p->polor = 71;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + dir[j]*1;
			p->vel[j] = dir[j]* (len * 5);
		}
		
		VectorAdd (move, vec, move);
	}

//	dec = 8;
	VectorScale (vec, dec, vec);
	VectorCopy (start, move);

}


/*
===============
R_DrawParticles
===============
*/
extern cvar_t	*sv_gravity;
extern int currentparticlepass;
extern cvar_t *r_particletrans;
extern cvar_t *r_particlesprite;
extern byte remapmap[256];				// For translating an old palette to new on load
extern	int particletypeonly;
extern particle_t		*currentparticle;
void R_DrawParticles (void)
{
	particle_t		*p, *kill;
	entity_t	*ent;
	float			grav;
	int				i;
	int				forg;
	float			time2, time3;
	float			time1;
	int				polor;
	float			dvel;
	float			frametime;
	int				where;
	vec3_t			whare;
	int			liftoff;
// hexen 2
		mleaf_t		*l;
		mleaf_t		*o;
		vec3_t		diff;
		vec3_t		below;
		qboolean	in_solid;

#ifdef GLQUAKE
	vec3_t			up, right;
	float			scale;
	GL_Bind(particletexture);
	glEnable (GL_BLEND);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBegin (GL_TRIANGLES);

	VectorScale (vup, 8.5, up);
	VectorScale (vright, 8.5, right);

#else
	D_StartParticles ();

	VectorScale (vright, xscaleshrink, r_pright);
	VectorScale (vup, yscaleshrink, r_pup);
	VectorCopy (vpn, r_ppn);
#endif
	frametime = cl.time - cl.oldtime;
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * sv_gravity->value * 0.05;
	dvel = 4*frametime;
	
	for ( ;; )
	{
		kill = active_particles;
		if (kill && kill->die < cl.time)
		{
						
			active_particles = kill->next;

			kill->next = free_particles;
			
			free_particles = kill;
			continue;
		}
		break;
	}

	for (p=active_particles ; p ; p=p->next)
	{
		for ( ;; )
		{
			kill = p->next;
			if (kill && kill->die < cl.time)
			{
				p->next = kill->next;
				kill->next = free_particles;
				free_particles = kill;
				continue;
			}
			break;
		}
	/*	if (particletypeonly == 1 && p->sprtype == 5)
		{
			model_t		*hello;
			if ((p->model = Mod_ForName("particle.spr", false)))
			{
			currentparticle = p;
			VectorCopy(p->org, r_entorigin);
			VectorSubtract(r_origin, r_entorigin, modelorg);
			R_DrawSprite2();
			return;
			}
			
		}*/
#ifdef GLQUAKE
		// hack a scale up to keep particles from disapearing
		scale = (p->org[0] - r_origin[0])*vpn[0] + (p->org[1] - r_origin[1])*vpn[1]
			+ (p->org[2] - r_origin[2])*vpn[2];
		if (scale < 20)
			scale = 1;
		else
			scale = 1 + scale * 0.004;
		glColor3ubv ((byte *)&d_8to24table[(int)p->polor]);
		glTexCoord2f (0,0);
		glVertex3fv (p->org);
		glTexCoord2f (1,0);
		glVertex3f (p->org[0] + up[0]*scale, p->org[1] + up[1]*scale, p->org[2] + up[2]*scale);
		glTexCoord2f (0,1);
		glVertex3f (p->org[0] + right[0]*scale, p->org[1] + right[1]*scale, p->org[2] + right[2]*scale);
#else

		if (p->blend == 5)
			p->blend = 1;
		if (p->sprtype == 5)
			p->sprtype = 4;

	//	if (p->stickable == 2){
	//		VectorCopy(p->vstuck1, p->org);
//
	//	}

	if (particleset == 1){
		p->frame = 3;	// glquake style particle hack
		p->anglevel[2] = 0;
		p->angles[2] = 0;
		p->scale = 0.3f;
		p->scaley = p->scale;
		p->blend = 3;
		p->alphavel = 0;
		p->scalexvel = 0;
		p->scaleyvel = 0;
		p->sprtype = 2;
	}

		// COLOR PRECALCULATE


/*
	// LIT PARTICLES
if (p->lit){
	int	la, ra, ga, ba;
	float		lightvec[3];
	float		rlightvec[3];
	float		glightvec[3];
	float		blightvec[3];
	float		rlightvec2[3];
	float		glightvec2[3];
	float		blightvec2[3];
	vec3_t		dist;
	float		add;
	float		radd;
	float		gadd;
	float		badd;
	int			lnum;
	dlight_t	*dl;
	vec3_t		t;
	vec3_t		ar;
	vec3_t		ag;
	vec3_t		ab;
		la = R_LightPoint(p->org);
	


	// add dynamic lights
	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		dl = &cl_dlights[lnum];
		if (!dl->radius)
			continue;
		if (dl->die < cl.time)
			continue;
		if (dl->unmark) // leilei - skip world light
			continue;

		VectorSubtract (p->org, dl->origin, dist);
		add = dl->radius - Length(dist);
		radd = dl->radius * dl->color[0] - Length(dist) ;
		gadd = dl->radius * dl->color[1] - Length(dist) ;
		badd = dl->radius * dl->color[2] - Length(dist) ;
		
		if (add > 0)
		{
			float scale = 1.0-(Length(dist)/dl->radius);
			scale *= scale; // scale ^ 2
			la += add*(scale);
			ra += radd*(scale);
			ga += gadd*(scale);
			ba += badd*(scale);

			
		}

		if (radd > 0)
		{
			float scale = 1.0-(Length(dist)/dl->radius);
			scale *= scale; // scale ^ 2
			ra += radd*(scale);
			VectorScale (dist, scale, dist);

		
		}

		if (gadd > 0)
		{
			float scale = 1.0-(Length(dist)/dl->radius);
			scale *= scale; // scale ^ 2
			ga += gadd*(scale);
			VectorScale (dist, scale, dist);
				
		}

		if (badd > 0)
		{
			float scale = 1.0-(Length(dist)/dl->radius);
			scale *= scale; // scale ^ 2
			ba += badd*(scale);
			VectorScale (dist, scale, dist);
			
		}
	}
	la *= -64;	la += 16300;
	if (la < 0) la = 0;
	polor = ((byte *)host_colormap)[p->polor + (la & 0xFF00)];







		}
else	
*/
		polor = p->polor;


	// FOGGED PARTICLES
	
	if (foguse){ 
		
		vec3_t	local, transformed;
		float	zi;
		int	izi;
			// transform point
		VectorSubtract (p->org, r_origin, local);

		transformed[0] = DotProduct(local, r_pright);
		transformed[1] = DotProduct(local, r_pup);
		transformed[2] = DotProduct(local, r_ppn);

		zi = 1.0 / transformed[2];
		izi = (int)(zi * 0x8000);
		forg = (float)izi * -128 + 32768;
		if (forg > 32764)	forg = 32764; if (forg < 0)	forg = 0;
		p->color  = (int)host_fogmap [(int)polor + (forg >> 2 & 0xFF00)];		
		
		p->trail = 0;	// don't do trails if we are fogged (covering up a visual bug that needs to be fixed)
	} 
	else{
		// PALETTE REMAPPED PARTICLES
	if (rmap_ready)
		polor= coltranslate[polor];
	p->color = polor; //we got it.
	}



	if (!p->trail && !p->frame && !particletypeonly){
	if (r_particletrans->value){
	if (p->blend == 1)			// Additive map case
		D_DrawParticle_Add (p);	
//	else if (p->blend == 2)		// Gelmap case
//		D_DrawParticle_Gel (p);		
	else 
		if (p->alpha < 0.33f)
		D_DrawParticle_A33 (p);
		else if (p->alpha < 0.66f)
		D_DrawParticle_A66 (p);
		else
		D_DrawParticle (p);	
	}
	else
		D_DrawParticle (p);	


	}
#endif// PARTICLE COLLISION CODE FROM HEXEN II! (It used to be for snow now it's for everything!)
// BEIGN


			// leilei - particle sprites
		if (p->frame && r_particlesprite->value && !particletypeonly)
		{
			model_t		*hello;
			if ((p->model = Mod_ForName("particle.spr", false)))
			{
			currentparticle = p;
			VectorCopy(p->org, r_entorigin);
			VectorSubtract(r_origin, r_entorigin, modelorg);
			
			currentparticlepass = 3;
			R_DrawSprite2();	// Subtractives first (i.e. decals!)
			currentparticlepass = 1;
			R_DrawSprite2();	// Alphas second....
			currentparticlepass = 2;
			R_DrawSprite2();	// Additives last

			}
		}	

		
		
	if(!r_particletrails->value)
		p->trail = 0; // terminate trails
		//else

			if (p->die < cl.time - 0.1)

			p->trail = 0; // don't trail...
		//	if (p->die > cl.time - 0.03)
		

				// leilei - hook up a trail
		if (p->trail && particleset != 2)
		{
			

			if (!p->org[0])
				return;	// apparently there isnt 
				VectorMA(p->org, -p->trail/(Length(p->vel)), p->vel, whare);
				// Since Lines are drawn after particles, we also have to calculate fog. AGAIN!
							if (foguse){ 
						
						vec3_t	local, transformed;
						float	zi;
						int	izi;
							// transform point
						VectorSubtract (whare, r_origin, local);

						transformed[0] = DotProduct(local, r_pright);
						transformed[1] = DotProduct(local, r_pup);
						transformed[2] = DotProduct(local, r_ppn);

						zi = 1.0 / transformed[2];
						izi = (int)(zi * 0x8000);
						forg = (float)izi * -128 + 32768;
						if (forg > 32764)	forg = 32764; if (forg < 0)	forg = 0;
						polor  = (int)host_fogmap [(int)p->polor + (forg >> 2 & 0xFF00)];		
						

					} 
					else
					polor = p->polor; //we got it.
					
//	VectorMA(p->org, -p->trail/(Length(p->vel)), p->vel, whare);
				D_DrawSparkTrans(p->org,whare,polor, p->blend);

		}

	// make 'em move
#ifndef GLQUAKE
		if (reflectpass || particletypeonly)
			break;
#else
			if (particletypeonly)
			break;
#endif
		if(particletypeonly == 0){
			p->org[0] += p->vel[0] * frametime;
			p->org[1] += p->vel[1] * frametime;
			p->org[2] += p->vel[2] * frametime;

			p->alpha += frametime*p->alphavel;
			p->scale += frametime*p->scalexvel;
			p->scaley += frametime*p->scaleyvel;
			p->angles[0] += frametime*p->anglevel[0];
			p->angles[1] += frametime*p->anglevel[1];
			p->angles[2] += frametime*p->anglevel[2];
			
			if (p->alpha < 0.06 && p->alphavel && !(p->type==pt_flare))
					p->die = -1;
			else if (p->alpha < 0.06 && p->alphavel && (p->type==pt_flare))
					p->alpha = 0.06;
			if (p->alpha > 1.1)
				p->alpha = 1.1;

		}
		if (p->stickable != 2)
		p->stickable = 0; // don't stick........
		switch (p->type)
		{
		case pt_static:
		
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_bloodsplatten:						// leilei - blood hitting the world. We check for where in the world and how to treat it right.
			p->alpha = 1;p->blend = 0;p->lit = 1; 
				if (!p->stickable)
				p->stickable = 1;
			//p->trail = 0;
				// check floor
							p->scalexvel = 0.0f; 
							p->scaleyvel = 0.0f;
				if (p->scale > 3)
					p->die = -1; // leilei - too much blood...
			// does this actually work???
			{
				trace_t	trace;
				vec3_t nah;
				vec3_t there;
				VectorAdd(p->org, p->vel, there);
				VectorAdd(p->org, p->vel, there);
				VectorAdd(p->org, p->vel, there);
				VectorAdd(p->org, p->vel, there);
				VectorAdd(p->org, p->vel, there);
				

			}

				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] - 6;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 5;
				else	
						where = 6;

				// check sky (relaly quick)
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] + 12;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents == CONTENTS_SKY){
						where = 54;
						p->die = -1;	// die particle! hahaha
				}

				// check ceil (relaly quick)
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] + 2;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if ((where == 6) && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 1;
				


				// check walls...
				whare[0] = p->org[0] + 4; whare[1] = p->org[1];
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if ((where == 6) && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;

				whare[0] = p->org[0] - 4; whare[1] = p->org[1];
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if ((where == 6) && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;

				whare[0] = p->org[0]; whare[1] = p->org[1] + 4;
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if ((where == 6) && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;

				whare[0] = p->org[0]; whare[1] = p->org[1] - 4;
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if ((where == 6) && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;

					whare[0] = p->org[0]; whare[1] = p->org[1];
						whare[2] = p->org[2] + 32;
						o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (where == 7 && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME)) {
						where = 2; // nothing over us (assumed on a wall now)
				}

			// Check the floor again
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] - 6;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if ((where == 7) && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 5;






				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] + 24;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if ((where == 6) && o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME)) {
						where = 1; // nothing under us...
				//		p->polor = 15;
				
				}

				// Case processing

				if (where == 7)	{				// A Wall	(Run down the wall)
							p->type = pt_bloodrun;
							p->scalexvel = -0.3f; 
							p->scaleyvel = 0.5f;
							
							if (particleset == 2){
							R_Decal (p->org, 6+rand()%3, 6, 0.8f, 1);	// splatify.
							p->die = -1;
							}
							else
							{
							p->trail = 3; // the linesy line method i teased before.
							}
							p->die += 5;
							p->alphavel = -r_particlebloodfade->value;
							
							// decal instead?
							if (particleset == 2){
					//		if (rand()%8 > 7)
								if (p->vel[2] > 0)	// if it's going up, stick up
							R_Decal (p->org, 6+rand()%3, 6, 0.8f, 0);
							}
				}

				else if (where == 1){			// A Ceiling
						//	p->polor = 15;
							//p->type = pt_blooddrip;
					p->sprtype = 1;
							p->angles[0] = 0;
							p->angles[1] = 0;
							p->angles[2] = 0;
							p->scalexvel = -0.2f; 
							p->scaleyvel = 0.6f;	// gravity pulls it downer...
							p->sprtype = 3;
							p->frame = 5;
						if(p->die < cl.time + 1 || p->scale < 0.4){
							p->type = pt_blooddrip;
							p->trail = 10;
							p->die += 5;	// it's time to drip!
							p->scaley = 5;
							p->sprtype = 3;
							// decal instead?
							if (particleset == 2){
							R_Decal (p->org, 6+rand()%3, 6, 1.2f, 0);
							p->frame = 5;
							p->trail = 0;
							p->alpha = 0;

							}
									}
				}
				else if (where == 5){			// Floor
							p->type = pt_bloodfloor;
							p->trail = 0;
							p->alphavel = -r_particlebloodfade->value;
							p->scalexvel = 0.0f; 
							p->scaleyvel = 0.0f;
							// decal instead?
							if (particleset == 2){
									p->die = -1;
								if (rand()%8 > 4)
							R_Decal (p->org, 6+rand()%3, 6, 0.7f, 0);//
						
							}
				}
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_blooddrip:						// leilei - Blood dripping from the ceiling.
			p->alpha = 1;p->blend = 0;p->lit = 1;p->trail = 5; //p->flat = 1;
			if (!p->stickable)
				p->stickable = 1;
				p->sprtype = 1;
				p->angles[0] = 0;
				p->angles[1] = 0;
				p->angles[2] = 0;
			if (p->scale > 1){
					p->scaley = p->scale; // leilei - too much blood.
					p->scale = 1;
			}
		//	p->scalexvel = -0.1f;
				if (p->scale < 0.2)
					p->scale = 0.2;
					p->scaleyvel = 1; //.. leilei - drip vertically fast
					if (p->scaley > 4)
						p->scaley = 4;
					if (!p->vel[2])
							p->vel[2] = -70;	// give an initial gravity pull as the drop gets off the ceiling.
					
							p->vel[2] -= grav * 17;

	

				// check floor
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] - 6;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 5;
				else	
						where = 6;

					// check sky (relaly quick)
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] + 12;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents == CONTENTS_SKY){
						where = 54;
						p->die = -1;	// die particle! hahaha
				}


				// Case processing

				if (where == 5){			// Floor
					R_ParticleBloodSplash(p->org, p->vel[2], p->polor);	// splat!
					p->type = pt_bloodfloor;
					if (particleset == 2){
							R_Decal (p->org, 6+rand()%3, 6, 0.8f, 0);	// splatify.
							p->die = -1;
							}
							//p->die = -1;		// terminate the drop
				}
				
	
			break;
		case pt_drip:						// leilei - generic dripping
			p->lit = 0;
				p->sprtype = 1;
				p->angles[0] = 0;
				p->angles[1] = 0;
				p->angles[2] = 0;
			if (p->scale > 1){
					p->scaley = p->scale; 
					p->scale = 1;
			}
		//	p->scalexvel = -0.1f;
				if (p->scale < 0.2)
					p->scale = 0.2;
					p->scaleyvel = 5; //.. leilei - drip vertically fast
					if (p->scaley > 4)
						p->scaley = 4;
					if (!p->vel[2])
							p->vel[2] = -70;	// give an initial gravity pull as the drop gets off the ceiling.
					
							p->vel[2] -= grav * 17;

	

				// check floor
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] - 6;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 5;
				else	
						where = 6;

					// check sky (relaly quick)
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] + 12;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents == CONTENTS_SKY){
						where = 54;
						p->die = -1;	// die particle! hahaha
				}


				// Case processing

				if (where == 5){			// Floor
					
					
					if (particleset == 2){
							p->die = -1;
							}
							//p->die = -1;		// terminate the drop
				}
				
	
			break;
		case pt_bloodrun:						// leilei - Blood running down the wall.
				//p->flat = 1;
				//p->alpha += frametime*p->alphavel;
							//	p->scalexvel = -0.03f; 
							//p->scaleyvel = 0.3f;

			p->scalexvel = p->vel[2] * 0.02;
			p->scaleyvel = p->vel[2] * -0.2;
					
							if (p->scaley > 1.7){
								p->scaley = 1.7;
								p->vel[0] = 0;
								p->vel[1] = 0;
								p->vel[2] = 0;
								p->scalexvel = 0; 
								p->scaleyvel = 0;
									break;
							}

							
			if (p->scale > 1)
					p->die = -1; // leilei - too much blood...
				if (p->alpha <= 0)
				p->die = -1;
				// TODO: Add a trail
					if (!p->vel[2])
							p->vel[2] = grav * -7; //- (rand()*55);

					

				
					// check floor
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] - 1;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 5;
				else	
						where = 6;
			// check walls...
				whare[0] = p->org[0] + 4; whare[1] = p->org[1];
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;

				whare[0] = p->org[0] - 4; whare[1] = p->org[1];
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;

				whare[0] = p->org[0]; whare[1] = p->org[1] + 4;
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;

				whare[0] = p->org[0]; whare[1] = p->org[1] - 4;
				whare[2] = p->org[2];
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 7;
	

				// Case processing

				if (where == 5){			// Floor
				 p->trail = 0;
							p->type = pt_bloodfloor;		// Become a drip
							}

							else if (where != 7){			// Not on a wall
								p->trail = 6;
							p->type = pt_blooddrip;		// Become a drip
							p->sprtype = 3;
							}
				
			//p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_bloodfloor:						// leilei - Blood is drying up on that floor.
				p->blend = 0;p->lit = 0;p->trail = 0;
				p->alpha += frametime*p->alphavel;
				if (p->scale > 4)
					p->die = -1; // leilei - too much blood...
				if (p->alpha <= 0)
				p->die = -1;
				// check floor
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] - 6;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents != CONTENTS_EMPTY && ( o->contents != CONTENTS_WATER || o->contents != CONTENTS_LAVA || o->contents != CONTENTS_SLIME))
						where = 5;
				else	
						where = 6;

					// check sky (relaly quick)
				whare[0] = p->org[0]; whare[1] = p->org[1];
				whare[2] = p->org[2] + 12;
				o = Mod_PointInLeaf (whare, cl.worldmodel);
				if (o->contents == CONTENTS_SKY){
						where = 54;
						p->die = -1;	// die particle! hahaha
				}

				// Case processing

				if (where == 6){			// Not the Floor
							p->type = pt_blooddrip; // move off incase it's a platform maybe
							p->die = cl.time + 2;
				}
				
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_fire:
			p->alpha = 1;p->blend = 0;p->lit = 0;p->trail = 0;
			p->ramp += time1;
			if (p->ramp >= 6)
				p->die = -1;
			else
				p->polor = ramp3[(int)p->ramp];
			p->vel[2] += grav;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;

		case pt_explode:
			
			p->ramp += time2;
//			p->splatter = 0;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->polor = ramp1[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;

		case pt_explode2:
			p->alpha = 1;p->blend = 0;p->lit = 0;p->trail = 0;
			p->ramp += time3;
//			p->splatter = 0;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->polor = ramp2[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] -= p->vel[i]*frametime;
			p->vel[2] -= grav;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;

		case pt_blob:
			p->alpha = 1;p->blend = 0;p->lit = 0;p->trail = 0;
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;

		case pt_blob2:
			p->alpha = 1;p->blend = 0;p->lit = 0;p->trail = 0;
			for (i=0 ; i<2 ; i++)
				p->vel[i] -= p->vel[i]*dvel;
			p->vel[2] -= grav;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;

		case pt_grav:
			
			p->vel[2] -= grav * 12;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
// huge blood crap
		case pt_bloodsplat:
			//p->blend = 0;	p->lit = 0;
			//p->alpha += frametime*p->alphavel;
			//p->scalexvel = 0;
			p->stickable = 1;
			//p->anglevel[2] = 0;
				if (p->scale > 2)
					p->scale = 2;; // leilei - too much blood.
				l = Mod_PointInLeaf (p->org, cl.worldmodel);

					if (l->contents == CONTENTS_WATER || l->contents == CONTENTS_LAVA || l->contents == CONTENTS_SLIME) // in a liquid
					{
							p->type = pt_bloodsplat;
							p->alphavel = -0.4 * (rand()&8);
							p->vel[0] *= 0.96;
							p->vel[1] *= 0.96;
							p->vel[2] *= 0.96;
							p->vel[0] += (rand()%4-2);
							p->vel[1] += (rand()%4-2);	// disperse, dissolve, dissipate
							p->vel[2] += (rand()%4-2);
							p->vel[0] *= p->alpha;
							p->vel[1] *= p->alpha;
							p->vel[2] *= p->alpha;
							p->scalexvel = 3.1;
							p->scaleyvel = 3.1;
						//	p->alphavel = -0.2;
							p->anglevel[2] *= 0.93; // reduce angle velocity as well, stop blood from spining
							p->blend = 1;	// additive because we get alpha in that 
					}
					else	if (l->contents != CONTENTS_EMPTY && ( l->contents != CONTENTS_WATER || l->contents != CONTENTS_LAVA || l->contents != CONTENTS_SLIME)) // || in_solid == true
					{
								p->type = pt_bloodsplatten;
							p->alphavel = -0.1 * (rand()&8);
							p->vel[0] = 0;
							p->vel[1] = 0;
							p->vel[2] = 0;
							p->anglevel[2] = 0;
				}
						
								else
								{

		//	p->vel[2] -= grav * 6;

			 VectorScale(p->vel, frametime, diff);
            VectorAdd(p->org, diff, p->org);

            // WHERE THE HITTING HAPPENS
            // if hit solid, go to last position,
            // no velocity, fade out.
            l = Mod_PointInLeaf (p->org, cl.worldmodel);
            if (l->contents != CONTENTS_EMPTY && ( l->contents != CONTENTS_WATER || l->contents != CONTENTS_LAVA || l->contents != CONTENTS_SLIME)) // || in_solid == true
            {
                VectorScale(diff, 0.2, p->vel);
                i = 6;
                while (l->contents != CONTENTS_EMPTY && ( l->contents != CONTENTS_WATER || l->contents != CONTENTS_LAVA || l->contents != CONTENTS_SLIME))
                {
                    VectorNormalize(p->vel);
                    p->org[0] -= p->vel[0]*3;
                    p->org[1] -= p->vel[1]*3;

                    p->org[2] -= p->vel[2]*3;


					p->scalexvel = 0;
					p->scaleyvel = 0;
					p->anglevel[2] = 0;
					//p->die += 1;
                    i--; //no infinite loops
                    if (!i)
                    {
                        p->die = -1;	//should never happen now!
                        break;
                    }
                    l = Mod_PointInLeaf (p->org, cl.worldmodel);
                }
					p->vel[0] = p->vel[1] = p->vel[2] = 0;
					p->anglevel[2] = 0;
           					if (particleset == 2){
						p->die = -1;
							R_Decal (p->org, p->frame, 6, p->scale, rand()%2);
							
					}
                p->ramp = 0;
				p->trail = 0;
					p->scalexvel = 0;
					p->scaleyvel = 0;
					p->anglevel[2] = 0;
                p->type = pt_bloodsplatten;
            }
            else
            {
              //p->alpha = 1; // leileileilei
				p->trail = 0;

				//           p->vel[2] -= grav * 4;
           p->vel[2] -= grav * 10;
            }
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			
			// END CODE of stupid hexen 2 particle collision
			break;

		case pt_slowgrav:
			
			p->vel[2] -= grav;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_fastgrav:
			
			p->vel[2] -= grav * 15;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_spark:
				
			p->blend = 1;p->lit = 0;
			
			p->vel[2] -= grav * 12;
		//	VectorMA(p->org, -8/(Length(p->vel)), p->vel, whare);
		//	R_ParticleTrail2(p->org,whare,0,p->blend,p->polor);
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_sparkvel:
					
			p->vel[2] -= grav * 12;
			p->vel[0] *= 0.9;
			p->vel[1] *= 0.9;
			p->vel[2] *= 0.9;
			{
				vec3_t vec, forward, angle_dir;
				float	lengthe;
				
			//	lengthe = Length(p->vel) * 0.0006;
			//VectorCopy(vec, forward);
			vectoangles2 (p->vel, angle_dir);
			p->angles[0] = angle_dir[0];
			p->angles[1] = angle_dir[2];
			p->angles[2] = angle_dir[1] - 90;
		//	p->angles[2] *= 3;
			p->sprtype = 44;
			p->scaley = lengthe; // there are some types you don't want a vertical scale for
			//p->scaley = 3; // there are some types you do want a vertical scale for
			}
			break;
		case pt_blood:
			p->lit = 1;p->trail = 0;
			p->stickable = 1;
			p->vel[2] -= grav * 12;
			//p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
			// leilei - flares
		case pt_flare:
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_flare_will_die:	// for entities.....

			
			p->scale = (p->org[0] - r_origin[0])*vpn[0] + (p->org[1] - r_origin[1])*vpn[1]
			+ (p->org[2] - r_origin[2])*vpn[2];
		if (p->scale < 40)
			p->scale = 0.1;
		else
			p->scale = 0.1 + p->scale * 0.004;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_smoke:
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			p->alpha += frametime*p->alphavel;
			if (sprity){
			p->vel[0] *= 0.96;
			p->vel[1] *= 0.96;
			p->vel[2] += (grav * 0.7);
			}
				else
			p->vel[2] += grav;
			p->anglevel[2] *= 0.96;
		
			break;
			// TomazQuake
		case pt_snow:
			if (cl.time > p->die)
				{
					p->die = cl.time + (rand() & 3) * 0.1;
					p->vel[0] = (rand() & 64) - 32;
					p->vel[1] = (rand() & 64) - 32;
				}
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
			// TomazQuake
		case pt_trail:
			p->trail = 0;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_add:
			p->trail = 0;
			if(p->alpha < 0.02)
					p->alpha = 1;
			else
			p->alpha -= 0.5;
			p->alpha = 0.5;
			p->blend = 2;p->lit = 0;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			
		break;
		case pt_decel:
			p->alpha = 1;p->trail = 0;
			p->blend = 0;p->lit = 0;
			
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]* -dvel;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_staticfade:
			p->alpha += frametime*p->alphavel;p->trail = 0;
				p->alpha -= cl.time*(0.001 +(rand() % 6 * 0.006));
			p->blend = 0;p->lit = 0;
			
			if (p->alpha <= 0)
				p->die = -1;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_staticfadeadd:
			p->alpha += frametime*p->alphavel;p->trail = 0;
			//p->blend = 2;p->lit = 0;
			p->vel[0] *= 0.8f;
			p->vel[1] *= 0.8f;
			p->vel[2] *= 0.8f;
			if (p->alpha <= 0)
				p->die = -1;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;
		case pt_slowgravaddfade:
			p->alpha += frametime*p->alphavel;p->trail = 0;
			p->blend = 2;p->lit = 0;
			p->vel[2] -= grav;
			
			if (p->alpha <= 0)
				p->die = -1;
			p->scaley = p->scale; // there are some types you don't want a vertical scale for
			break;

			
		}


	}

#ifdef GLQUAKE
	glEnd ();
	glDisable (GL_BLEND);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#else
	D_EndParticles ();
#endif

}

}

// test visibility of flares first....
void R_TestFlares (void)
{
	flare_t		*p, *kill;
	entity_t	*ent;
	float			grav;
	int				i;
	float			time2, time3;
	float			time1;
	float			frametime;
	int			liftoff;
	int			amiseen;

	
#ifndef GLQUAKE	
	VectorScale (vright, xscaleshrink, r_pright);
	VectorScale (vup, yscaleshrink, r_pup);
	VectorCopy (vpn, r_ppn);
#else
//	VectorScale (vright, xscale, r_pright);
//	VectorScale (vup, yscale, r_pup);
//	VectorCopy (vpn, r_ppn);
#endif
	
	for ( ;; )
	{
		kill = active_flares;
		if (kill && kill->die < cl.time)
		{
						
			active_flares = kill->next;

			kill->next = free_flares;
			
			free_flares = kill;
			continue;
		}
		break;
	}

	for (p=active_flares ; p ; p=p->next)
	{
		for ( ;; )
		{
			kill = p->next;
			if (kill && kill->die < cl.time)
			{
				p->next = kill->next;
				kill->next = free_flares;
				free_flares = kill;
				continue;
			}
			break;
		}


	{	
				
			int there;
			int u1, v1, z1;
			int eh;
			int u2, v2, z2 = 88;
			vec3_t	hi;
			trace_t	yourface;
			vec3_t forward, left, up;
			AngleVectors(r_refdef.viewangles, forward, left, up);
			VectorCompare(yourface.endpos,r_refdef.vieworg);

			p->amiseen = 0; // reset to 0. when not seen. always. Stops a PVS bug where you saw a flare
							// turn your back towards it, move to next room, then turn back to see it
							// fade away late
			D_TestOurFlare(p);  // lets do it

		}	

	
	}
	

}




void R_DrawFlares (void)
{
	flare_t		*p, *kill;
	entity_t	*ent;
	float			grav;
	int				i;
	float			time2, time3;
	float			time1;
	float			frametime;
	int			liftoff;
	int			amiseen;

	
#ifndef GLQUAKE	
	VectorScale (vright, xscaleshrink, r_pright);
	VectorScale (vup, yscaleshrink, r_pup);
	VectorCopy (vpn, r_ppn);
#endif
	frametime = cl.time - cl.oldtime;
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * sv_gravity->value * 0.05;
//	dvel = 4*frametime;
	
	for ( ;; )
	{
		kill = active_flares;
		if (kill && kill->die < cl.time)
		{
						
			active_flares = kill->next;

			kill->next = free_flares;
			
			free_flares = kill;
			continue;
		}
		break;
	}

	for (p=active_flares ; p ; p=p->next)
	{
		for ( ;; )
		{
			kill = p->next;
			if (kill && kill->die < cl.time)
			{
				p->next = kill->next;
				kill->next = free_flares;
				free_flares = kill;
				continue;
			}
			break;
		}
#ifndef GLQUAKE
		if (!reflectpass)
#endif
		{
			p->org[0] += p->vel[0] * frametime;
			p->org[1] += p->vel[1] * frametime;
			p->org[2] += p->vel[2] * frametime;

			p->alpha += frametime*p->alphavel;
			p->scale += frametime*p->scalexvel;
			p->scaley += frametime*p->scaleyvel;
			p->angles[2] += frametime*p->anglevel[2];
			
			if (p->alpha < 0.06 && p->alphavel && !(p->type==pt_flare))
					p->die = -1;
			else if (p->alpha < 0.06 && p->alphavel && (p->type==pt_flare))
					p->alpha = 0.06;
			if (p->alpha > 1.1)
				p->alpha = 1.1;
			if (p->scale < 0)
				p->die = -1;
			if (p->scaley < 0)
				p->die = -1;

		}
	{	
				
			int there;
			int u1, v1, z1;
			int eh;
			int u2, v2, z2 = 88;
			vec3_t	hi;
			trace_t	yourface;
			vec3_t forward, left, up;
			AngleVectors(r_refdef.viewangles, forward, left, up);
			VectorCompare(yourface.endpos,r_refdef.vieworg);


			// Removing an entity's flare 
			if (p->owned){
				if (p->owner)	// stops a crash
			if (p->oldmodel != p->owner->model){
				p->owner->gotaflare = 0;
				p->owner->ourparticle = NULL;
				p->owner = NULL;
				
				
//				Con_Printf("old model isnt the same.\n");
				p->die = -1;	// oh.....noooooooooo
			}
			if (p->owner){

				p->org[0] = p->owner->origin[0];
				p->org[1] = p->owner->origin[1];
				p->org[2] = p->owner->origin[2];
				
			}

			}
			for(eh=0; eh<3; eh++){
				forward[eh] *= 350;
				left[eh] *= -380;	// 380
				up[eh] *= 135;

			}
			
		}

		// scale the (glquake particle code repurposed :) )
#ifndef GLQUAKE
	if (reflectpass)

	{
		p->scale = (p->org[0] - r_refdef.vieworg[0])*vpn[0] + (p->org[1] - r_refdef.vieworg[1])*vpn[1]
			+ (p->org[2] - r_refdef.vieworg[2])*vpn[2];
		if (p->scale < 50)
			p->scale = 0.06f;
		else
			p->scale = 0.02f + p->scale * 0.001;
		p->scaley = p->scale * 5;
		p->sprtype = SPR_VP_PARALLEL_UPRIGHT;

	}

		else
#endif
	{
		p->scale = (p->org[0] - r_refdef.vieworg[0])*vpn[0] + (p->org[1] - r_refdef.vieworg[1])*vpn[1]
			+ (p->org[2] - r_refdef.vieworg[2])*vpn[2];
		if (p->scale < 50)
			p->scale = 0.12f;
		else
			p->scale = 0.02f + p->scale * 0.002;
		p->scaley = p->scale;
		p->sprtype = 5;

	}
		// traceline to player to check visibility of flare!
#ifndef GLQUAKE
		if (!reflectpass)
#endif
		{

			// old traceline method:
		/*	{
			trace_t tarce;
				tarce = Particle_TraceLine(p->org, r_refdef.vieworg);
			if(tarce.fraction < 1)
				amiseen = 0;
			else
				amiseen = 1;
			}
			
		*/
			// new z check
			//amiseen = 0;
			//D_TestOurFlare(p);
			if (p->amiseen)
			{
				int sweetch;
							//p->alphavel = 1.8;
						// lightstyles hack - to check if our light is 'off' we'll use
								// lightpoint.
							sweetch = R_LightPoint (p->org);
							if (sweetch < 16)
								p->alphavel = -1.74;
							else
								p->alphavel = 1.8;

			}
				else
				{
				p->alphavel = -1.74;
				}
		}

					// leilei - particle sprites
//		if (p->frame && r_particlesprite->value && !particletypeonly)
		{
			model_t		*hello;
			if ((p->model = Mod_ForName("particle.spr", false)))
			{
			currentflare = p;
			VectorCopy(p->org, r_entorigin);
			VectorSubtract(r_origin, r_entorigin, modelorg);
			
			R_DrawSprite3();
			}
		}	

	
	}
	

}

// Stupid function for entities with instant flares to try and fade them out like normal flares!!
void	R_FlareEntityAlphaMakeCheckDoStuff (vec3_t checkhere, float fe, float fi) 
{
	trace_t tarce;
	
	float			time1;
	float			frametime;

	frametime = cl.time - cl.oldtime;
	
	time1 = frametime * 5;



	if (!fe)
		fe = 1.0f;


	//if (currententity->flalpha < 0.06 && currententity->flalphavel)
	//		currententity->flalpha = 0.01;
	if (fe < 0.06)
			fe  = 0.06;
	if (fe  > 1.1)
			fe  = 1.1;


	tarce = Particle_TraceLine(checkhere, r_refdef.vieworg);
		if(tarce.fraction < 1)
			fi = -1.74;
		else{
			fi = 1.8;
		}

			fe += frametime*fi;


			
};
