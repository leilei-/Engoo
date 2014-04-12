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
// chase.c -- chase camera code

#include "quakedef.h"

cvar_t	*chase_back;
cvar_t	*chase_up;
cvar_t	*chase_right;
cvar_t	*chase_active;

vec3_t	chase_pos;
vec3_t	chase_angles;

vec3_t	chase_dest;
vec3_t	chase_dest_angles;


//	leilei's stupid deathcam experiment
cvar_t	*cl_diecam;	// leilei
		// 0 - regular quake tilt-the-view-suddenly death.
		// 1 - more doom-like, no tilt - camera sinks to ground and weapon is dropped. probably impossible to turn toward killer
		// 2 - quake3-like - fixed chase camera, can't move it at all. no tilt either. 
		// 3 - unrealike -  rotatable chase camera - scores are automatically displayed a few seconds later.
float		deathcamtime;	// resets to 0 if we're alive. 

float	deathcam_whenidied;		// sets to time if died, used for timing deathcam effects...
float	deathcam_whenidiedafter; 
	// used to time the sinks into the floor or the rankings appearing.....
vec3_t	deathcam_angles;	// this'll update chase angles when we're dead so we don't move the
							// player - so we can rotate the death cam in cl_diecam 3.
int		deathcam_yesiamdead;	// pretty stupid indicator of dead.

// 2001-09-18 New cvar system by Maddes (Init)  start
void Chase_Init_Cvars (void)
{
	chase_back = Cvar_Get ("chase_back", "100", CVAR_ORIGINAL);
	chase_up = Cvar_Get ("chase_up", "16", CVAR_ORIGINAL);
	chase_right = Cvar_Get ("chase_right", "0", CVAR_ORIGINAL);
	chase_active = Cvar_Get ("chase_active", "0", CVAR_ORIGINAL);

	cl_diecam = Cvar_Get ("cl_diecam", "0", CVAR_ORIGINAL | CVAR_ARCHIVE);	// leilei
}
// 2001-09-18 New cvar system by Maddes (Init)  end

void Chase_Init (void)
{
// 2001-09-18 New cvar system by Maddes (Init)  start
/*
	chase_back = Cvar_Get ("chase_back", "100", CVAR_ORIGINAL);
	chase_up = Cvar_Get ("chase_up", "16", CVAR_ORIGINAL);
	chase_right = Cvar_Get ("chase_right", "0", CVAR_ORIGINAL);
	chase_active = Cvar_Get ("chase_active", "0", CVAR_ORIGINAL);
*/
// 2001-09-18 New cvar system by Maddes (Init)  end
}

#ifdef VMTOC

extern qboolean SV_RecursiveHullCheck(hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace);

static trace_t CL_TraceLine(vec3_t start, vec3_t end)
{
	trace_t	trace;

	memset(&trace, 0, sizeof(trace));
	trace.fraction = 1.0f;
	trace.allsolid = true;
	VectorCopy(end, trace.endpos);

	SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	return trace;
}

void Chase_Update(void)
{
	float dist;
	vec3_t forward, right;
	vec3_t chase_dest;
	trace_t trace;

	vec_t camback = chase_back->value;
	vec_t camup = chase_up->value;


	if (deathcam_yesimdead)
		AngleVectors(deathcam_angles, forward, right, NULL);
		else
		AngleVectors(cl.viewangles, forward, right, NULL);

	dist = -camback - 8;
	chase_dest[0] = r_refdef.vieworg[0] + forward[0] * dist;
	chase_dest[1] = r_refdef.vieworg[1] + forward[1] * dist;
	chase_dest[2] = r_refdef.vieworg[2] + forward[2] * dist + camup;

	trace = CL_TraceLine(r_refdef.vieworg, chase_dest);

	r_refdef.vieworg[0] = trace.endpos[0] + 8 * forward[0] + 4 * trace.plane.normal[0];
	r_refdef.vieworg[1] = trace.endpos[1] + 8 * forward[1] + 4 * trace.plane.normal[1];
	r_refdef.vieworg[2] = trace.endpos[2] + 8 * forward[2] + 4 * trace.plane.normal[2];
}

#else
void Chase_Reset (void)
{
	// for respawning and teleporting
//	start position 12 units behind head
}

void TraceLine (vec3_t start, vec3_t end, vec3_t impact)
{
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorCopy (trace.endpos, impact);
}

void TraceLineWithAVengeance (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal)
{
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorCopy (trace.endpos, impact);
	VectorCopy (trace.plane.normal, normal);
	//	Con_Printf("%f %f %f norm\n",normal[0],normal[1],normal[2]);
}

void TraceLineOrDie (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal, vec3_t fracin)
{
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorCopy (trace.endpos, impact);
	VectorCopy (trace.plane.normal, normal);
	
	fracin[0] = trace.fraction;

		Con_Printf("%f %f %f norm\n",normal[0],normal[1],normal[2]);
		Con_Printf("%f fraction\n",fracin[0]);
}


void Chase_Update (void)
{
	int		i;
	float	dist;
	vec3_t	forward, up, right;
	vec3_t	dest, stop;
	float	deest;
	int dontspinme;


		deest = sv.time - deathcam_whenidied;

	// 
	if(deathcam_yesiamdead == 4)
	{
		if (deest < 1){
		deathcam_angles[PITCH] = 0;
		deathcam_angles[YAW] = cl.viewangles[YAW];
		deathcam_angles[ROLL] = 0;
		}
		else
		{
		deathcam_angles[PITCH] = 90;
		deathcam_angles[YAW] = 0;
		

		}
	}

	// if can't see player, reset
	if(deathcam_yesiamdead)
	AngleVectors (deathcam_angles, forward, right, up);
		else
	AngleVectors (cl.viewangles, forward, right, up);

	// calc exact destination
	if (deathcam_yesiamdead == 2){			// close, nonspinnable
			AngleVectors (cl.viewangles, forward, right, up);
	for (i=0 ; i<3 ; i++)
		chase_dest[i] = r_refdef.vieworg[i]
		- forward[i]*70
		- right[i]*0;
	chase_dest[2] = r_refdef.vieworg[2];
		dontspinme = 1;
	}
	else
		if (deathcam_yesiamdead == 3){			// spinabble...
	for (i=0 ; i<3 ; i++)
		chase_dest[i] = r_refdef.vieworg[i]
		- forward[i]*150
		- right[i]*0;
//	chase_dest[2] = r_refdef.vieworg[2];
		dontspinme = 0;
	}
	else
		if (deathcam_yesiamdead == 4){			// non spinabble... and after a duration it kinda increases....

		if (deest < 1)
				deest = 0;
	dontspinme = 1;
	for (i=0 ; i<3 ; i++)
		chase_dest[i] = r_refdef.vieworg[i]
		- forward[i]*(55 + deest * 22)
		- right[i]*0;
//	chase_dest[2] = r_refdef.vieworg[2];

	}
		else
		{
	for (i=0 ; i<3 ; i++)
		chase_dest[i] = r_refdef.vieworg[i]
		- forward[i]*chase_back->value
		- right[i]*chase_right->value;
	chase_dest[2] = r_refdef.vieworg[2] + chase_up->value;
	dontspinme = 1;
		}
	// find the spot the player is looking at
	VectorMA (r_refdef.vieworg, 4096, forward, dest);
	TraceLine (r_refdef.vieworg, dest, stop);

	// calculate pitch to look at the same spot from camera
	VectorSubtract (stop, r_refdef.vieworg, stop);
	dist = DotProduct (stop, forward);
	if (dist < 1)
		dist = 1;

	if (dontspinme)	// leilei - allow spin....
	r_refdef.viewangles[PITCH] = -atan(stop[2] / dist) / M_PI * 180;

	if (deathcam_yesiamdead){

		// The..... style?
		if (deathcam_yesiamdead == 1){
			r_refdef.viewangles[YAW] = deathcam_angles[YAW];
			r_refdef.viewangles[PITCH] = deathcam_angles[PITCH];
			r_refdef.viewangles[ROLL] = deathcam_angles[ROLL];
		}


		// The Q3 style - fixed pitch and yaw
		else if (deathcam_yesiamdead == 2){
			r_refdef.viewangles[YAW] = cl.viewangles[YAW];
			r_refdef.viewangles[PITCH] =	0;
			r_refdef.viewangles[ROLL] =		0;
		}


		// The U style
		else if (deathcam_yesiamdead == 3){
			r_refdef.viewangles[YAW] = deathcam_angles[YAW];
			r_refdef.viewangles[PITCH] = deathcam_angles[PITCH];
			r_refdef.viewangles[ROLL] = deathcam_angles[ROLL];
		}

		// The DX style
		else if (deathcam_yesiamdead == 4){
			if (deest < 1){
					r_refdef.viewangles[YAW] = cl.viewangles[YAW];
					r_refdef.viewangles[PITCH] =	0;
					r_refdef.viewangles[ROLL] =		0;

			}
			else
			{
			r_refdef.viewangles[YAW] = 0;
			r_refdef.viewangles[PITCH] = 90;
			r_refdef.viewangles[ROLL] = deest * 18;
			}
		}
	}

// 2000-01-09 ChaseCam fix by FrikaC  start
	TraceLine(r_refdef.vieworg, chase_dest, stop);
	if (Length(stop) != 0)
	{
		VectorCopy(stop, chase_dest);
	}
// 2000-01-09 ChaseCam fix by FrikaC  end

	// move towards destination
	VectorCopy (chase_dest, r_refdef.vieworg);
}

#endif