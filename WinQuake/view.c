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
// view.c -- player eye positioning

#include "quakedef.h"
#include "r_local.h"

/*

The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.

*/

cvar_t	*lcd_x;
cvar_t	*cl_splitscreen;

cvar_t	*v_detail; // LEI
cvar_t	*lcd_yaw;

cvar_t	*scr_ofsx;
cvar_t	*scr_ofsy;
cvar_t	*scr_ofsz;

cvar_t	*cl_rollspeed;
cvar_t	*cl_rollangle;

cvar_t	*cl_bob;
cvar_t	*cl_bobcycle;
cvar_t	*cl_bobup;
// DARKPLACES!
// DARKPLACES!
cvar_t	*cl_bobmodel;
cvar_t	*cl_bobmodel_up;
cvar_t	*cl_bobmodel_side;
cvar_t	*cl_bobmodel_speed;
cvar_t	*cl_bobmodel_ex;

cvar_t	*cl_bobfall;
cvar_t	*cl_bobfallcycle;
cvar_t	*cl_bobfallminspeed;
cvar_t	*cl_bob2;
cvar_t	*cl_bob2cycle;
cvar_t	*cl_bob2smooth;

cvar_t	*cl_leanmodel;
cvar_t	*cl_leanmodel_side_speed;
cvar_t	*cl_leanmodel_side_limit;
cvar_t	*cl_leanmodel_side_highpass1;
cvar_t	*cl_leanmodel_side_highpass;
cvar_t	*cl_leanmodel_side_lowpass;
cvar_t	*cl_leanmodel_up_speed;
cvar_t	*cl_leanmodel_up_limit;
cvar_t	*cl_leanmodel_up_highpass1;
cvar_t	*cl_leanmodel_up_highpass;
cvar_t	*cl_leanmodel_up_lowpass;

cvar_t	*cl_followmodel;
cvar_t	*cl_followmodel_side_speed;
cvar_t	*cl_followmodel_side_limit;
cvar_t	*cl_followmodel_side_highpass1;
cvar_t	*cl_followmodel_side_highpass;
cvar_t	*cl_followmodel_side_lowpass;
cvar_t	*cl_followmodel_up_speed;
cvar_t	*cl_followmodel_up_limit;
cvar_t	*cl_followmodel_up_highpass1;
cvar_t	*cl_followmodel_up_highpass;
cvar_t	*cl_followmodel_up_lowpass;



// NEW

cvar_t  *cl_oldview;
cvar_t	*cl_gundraw;		// leilei - draw the gun when selected
cvar_t	*cl_gunhold;		// leilei - hold the gun motion when firing (ala d**m)
cvar_t	*cl_gunsniff;		// leilei - something experimental...

cvar_t	*cl_bobslam;
cvar_t	*cl_bobslamcycle;
cvar_t	*cl_bobslamminspeed;

cvar_t	*v_kicktime;
cvar_t	*v_kickroll;
cvar_t	*v_kickpitch;

cvar_t	*v_iyaw_cycle;
cvar_t	*v_iroll_cycle;
cvar_t	*v_ipitch_cycle;
cvar_t	*v_iyaw_level;
cvar_t	*v_iroll_level;
cvar_t	*v_ipitch_level;

cvar_t	*v_idlescale;

cvar_t	*crosshair;
cvar_t	*cl_crossx;
cvar_t	*cl_crossy;

cvar_t	*gl_cshiftpercent;

float	v_dmg_time, v_dmg_roll, v_dmg_pitch;

extern	int			in_forward, in_forward2, in_back;
int	shiftshif;
int shiftalpha;
extern int skipbob;
// BOBEN
// 16 pixels of bob

#define MAXBOB  0x100000

int ongrounid; // whether player is on ground or in air
int	aimlock;	// leilei - aim locking
float	aimlockangle;	// maximum extremities of aiming in the aimlock
vec3_t		lockedangle;


// leilei death cam

extern cvar_t	*cl_diecam;
extern float	deathcamtime;	
extern vec3_t	deathcam_angles;	
extern int		deathcam_yesiamdead;	


/*
===============
V_CalcRoll

Used by view and sv_user
===============
*/
vec3_t	forward, right, up;

float V_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;

	AngleVectors (angles, forward, right, up);
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	value = cl_rollangle->value;
//	if (cl.inwater)
//		value *= 6;

	if (side < cl_rollspeed->value)
		side = side * value / cl_rollspeed->value;
	else
		side = value;

	return side*sign;

}



/*
===============
V_CalcBob

===============
*/
float V_CalcBob (void)
{
	float	bob;
	float	cycle;
	int caycle;

//		float xyspeed;
//	float bspeed;
	cycle = cl.time - (int)(cl.time/cl_bobcycle->value)*cl_bobcycle->value;
	cycle /= cl_bobcycle->value;
	if (cycle < cl_bobup->value)
		cycle = M_PI * cycle / cl_bobup->value;
	else
		cycle = M_PI + M_PI*(cycle-cl_bobup->value)/(1.0 - cl_bobup->value);
	caycle =1;
// bob is proportional to velocity in the xy plane
// (don't count Z, or jumping messes it up)

	bob = sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]) * cl_bob->value;
	//bob = sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]);

	

	if (!cl_bobmodel->value){
	
	bob = bob*0.3 + bob*0.7*sin(cycle);
	if (bob > 4)
		bob = 4;
	else if (bob < -7)
		bob = -7;
	}
	else
		bob = 0;

	return bob;

}

/*
===============
V_CalcNon
For calculating doom-style horizontal bob
===============
*/
float V_CalcNon (void)
{
	float	non;
	float	cycle;

	cycle = cl.time - (int)(cl.time/cl_bobcycle->value)*cl_bobcycle->value;
	cycle /= cl_bobcycle->value;
	if (cycle < cl_bobup->value)
		cycle = M_PI * cycle / cl_bobup->value;
	else
		cycle = M_PI + M_PI*(cycle-cl_bobup->value)/(1.0 - cl_bobup->value);

// bob is proportional to velocity in the xy plane
// (don't count Z, or jumping messes it up)

	non = sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]) * cl_bob->value;
//Con_Printf ("speed: %5.1f\n", Length(cl.velocity));
	non = non*0.3 + non*0.7*sin(cycle);
//	if (non > 4)
//		non = 4;
//	else if (non < -7)
//		non = -7;
	return non;

}

//=============================================================================


cvar_t	*v_centermove;
cvar_t	*v_centerspeed;


void V_StartPitchDrift (void)
{
#if 1
	if (cl.laststop == cl.time)
	{
		return;		// something else is keeping it from drifting
	}
#endif
	if (cl.nodrift || !cl.pitchvel)
	{
		cl.pitchvel = v_centerspeed->value;
		cl.nodrift = false;
		cl.driftmove = 0;
	}
}

void V_StopPitchDrift (void)
{
	cl.laststop = cl.time;
	cl.nodrift = true;
	cl.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards cl.idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.

Drifting is enabled when the center view key is hit, mlook is released and
lookspring is non 0, or when
===============
*/
void V_DriftPitch (void)
{
	float		delta, move;

	if (noclip_anglehack || !cl.onground || cls.demoplayback )
	{
		cl.driftmove = 0;
		cl.pitchvel = 0;
		return;
	}

// don't count small mouse motion
	if (cl.nodrift)
	{
		if ( fabs(cl.cmd.forwardmove) < cl_forwardspeed->value)
			cl.driftmove = 0;
		else
			cl.driftmove += host_frametime;

		if ( cl.driftmove > v_centermove->value)
		{
			V_StartPitchDrift ();
		}
		return;
	}

	delta = cl.idealpitch - cl.viewangles[PITCH];

	if (!delta)
	{
		cl.pitchvel = 0;
		return;
	}

	move = host_frametime * cl.pitchvel;
	cl.pitchvel += host_frametime * v_centerspeed->value;

//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			cl.pitchvel = 0;
			move = delta;
		}
		cl.viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			cl.pitchvel = 0;
			move = -delta;
		}
		cl.viewangles[PITCH] -= move;
	}
}





/*
==============================================================================

						PALETTE FLASHES

==============================================================================
*/


cshift_t	cshift_empty = { {130,80,50}, 0 };
cshift_t	cshift_water = { {130,80,50}, 128 };
cshift_t	cshift_slime = { {0,25,5}, 150 };
cshift_t	cshift_lava = { {255,80,0}, 150 };

cvar_t	*v_gamma;
cvar_t	*v_saturation;			// leilei
cvar_t	*v_contrast;			// leilei

byte		gammatable[256];	// palette is sent through this

#ifdef	GLQUAKE
byte		ramps[3][256];

#endif	// GLQUAKE
float		v_blend[4];		// rgba 0.0 - 1.0

void BuildGammaTable (float g)
{
	int		i, inf;

	if (g == 1.0)
	{
		for (i=0 ; i<256 ; i++)
			gammatable[i] = i;
		return;
	}

	for (i=0 ; i<256 ; i++)
	{
		inf = 255 * pow ( (i+0.5)/255.5 , g ) + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		gammatable[i] = inf;
	}
}

/*
=================
V_CheckGamma
=================
*/
float oldgammavalue;
qboolean V_CheckGamma (void)
{
//	static float oldgammavalue;

	if (v_gamma->value == oldgammavalue)
		return false;
	oldgammavalue = v_gamma->value;

	BuildGammaTable (v_gamma->value);
	vid.recalc_refdef = 1;				// force a surface cache flush

	return true;
}




/*
=================
V_CheckSaturation
=================
*/
qboolean V_CheckSaturation (void)
{
	static float oldsatvalue;

	if (v_saturation->value == oldsatvalue)
		return false;
	oldsatvalue = v_saturation->value;

	
	vid.recalc_refdef = 1;				// force a surface cache flush

	return true;
}

/*
=================
V_CheckContrast
=================
*/
qboolean V_CheckContrast (void)
{
	static float oldcontvalue;

	if (v_contrast->value == oldcontvalue)
		return false;
	oldcontvalue = v_contrast->value;

	
	vid.recalc_refdef = 1;				// force a surface cache flush

	return true;
}



/*
===============
V_ParseDamage
===============
*/
void V_ParseDamage (void)
{
	int		armor, blood;
	vec3_t	from;
	int		i;
	vec3_t	forward, right, up;
	entity_t	*ent;
	float	side;
	float	count;

	armor = MSG_ReadByte ();
	blood = MSG_ReadByte ();
	for (i=0 ; i<3 ; i++)
		from[i] = MSG_ReadCoord ();

	count = blood*0.5 + armor*0.5;
	if (count < 10)
		count = 10;

	cl.faceanimtime = cl.time + 0.2;		// but sbar face into pain frame

	cl.cshifts[CSHIFT_DAMAGE].percent += 3*count;
	if (cl.cshifts[CSHIFT_DAMAGE].percent < 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;
	if (cl.cshifts[CSHIFT_DAMAGE].percent > 150)
		cl.cshifts[CSHIFT_DAMAGE].percent = 150;

	if (armor > blood)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}

//
// calculate view angle kicks
//
	ent = &cl_entities[cl.viewentity];

	VectorSubtract (from, ent->origin, from);
	VectorNormalize (from);

	AngleVectors (ent->angles, forward, right, up);

	side = DotProduct (from, right);
	v_dmg_roll = count*side*v_kickroll->value;

	side = DotProduct (from, forward);
	v_dmg_pitch = count*side*v_kickpitch->value;

	v_dmg_time = v_kicktime->value;
}


/*
==================
V_cshift_f
==================
*/
void V_cshift_f (void)
{
	cshift_empty.destcolor[0] = atoi(Cmd_Argv(1));
	cshift_empty.destcolor[1] = atoi(Cmd_Argv(2));
	cshift_empty.destcolor[2] = atoi(Cmd_Argv(3));
	cshift_empty.percent = atoi(Cmd_Argv(4));
}


/*
==================
V_BonusFlash_f

When you run over an item, the server sends this command
==================
*/
void V_BonusFlash_f (void)
{
	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	cl.cshifts[CSHIFT_BONUS].percent = 50;
}

/*
=============
V_SetContentsColor

Underwater, lava, etc each has a color shift
=============
*/
void V_SetContentsColor (int contents)
{
	switch (contents)
	{
	case CONTENTS_EMPTY:
	case CONTENTS_SOLID:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		break;
	case CONTENTS_LAVA:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case CONTENTS_SLIME:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	default:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}
}

/*
=============
V_CalcPowerupCshift
=============
*/
void V_CalcPowerupCshift (void)
{
	if (cl.items & IT_QUAD)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		cl.cshifts[CSHIFT_POWERUP].percent = 30;

	}
	else if (cl.items & IT_SUIT)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cl.cshifts[CSHIFT_POWERUP].percent = 20;
	}
	else if (cl.items & IT_INVISIBILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		cl.cshifts[CSHIFT_POWERUP].percent = 100;
	}
	else if (cl.items & IT_INVULNERABILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cl.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
}

/*
=============
V_CalcBlend
=============
*/
#ifdef	GLQUAKE
void V_CalcBlend (void)
{
	float	r, g, b, a, a2;
	int		j;

	r = 0;
	g = 0;
	b = 0;
	a = 0;

	for (j=0 ; j<NUM_CSHIFTS ; j++)
	{
		if (!gl_cshiftpercent->value)
			continue;

		a2 = ((cl.cshifts[j].percent * gl_cshiftpercent->value) / 100.0) / 255.0;

//		a2 = cl.cshifts[j].percent/255.0;
		if (!a2)
			continue;
		a = a + a2*(1-a);
//Con_Printf ("j:%i a:%f\n", j, a);
		a2 = a2/a;
		r = r*(1-a2) + cl.cshifts[j].destcolor[0]*a2;
		g = g*(1-a2) + cl.cshifts[j].destcolor[1]*a2;
		b = b*(1-a2) + cl.cshifts[j].destcolor[2]*a2;
	}

	v_blend[0] = r/255.0;
	v_blend[1] = g/255.0;
	v_blend[2] = b/255.0;
	v_blend[3] = a;
	if (v_blend[3] > 1)
		v_blend[3] = 1;
	if (v_blend[3] < 0)
		v_blend[3] = 0;
}
#endif


void V_CalcAlphaShift (void)
{
	float	r, g, b, a, a2;
	int		j;

	r = 0;
	g = 0;
	b = 0;
	a = 0;

	for (j=0 ; j<NUM_CSHIFTS ; j++)
	{
//		if (!gl_cshiftpercent->value)
//			continue;

		a2 = ((cl.cshifts[j].percent * 1) / 100.0) / 255.0;

//		a2 = cl.cshifts[j].percent/255.0;
		if (!a2)
			continue;
		a = a + a2*(1-a);
//Con_Printf ("j:%i a:%f\n", j, a);
		a2 = a2/a;
		r = r*(1-a2) + cl.cshifts[j].destcolor[0]*a2;
		g = g*(1-a2) + cl.cshifts[j].destcolor[1]*a2;
		b = b*(1-a2) + cl.cshifts[j].destcolor[2]*a2;
	}

	v_blend[0] = r;
	v_blend[1] = g;
	v_blend[2] = b;
	v_blend[3] = a * 255;
	if (v_blend[3] > 1)
		v_blend[3] = 1;
	if (v_blend[3] < 0)
		v_blend[3] = 0;
	
	shiftshif = FindColor18(v_blend[0],v_blend[1],v_blend[2]);
		shiftalpha = ((int)v_blend[3]);
	


};


/*
=============
V_UpdatePalette
=============
*/
#ifdef	GLQUAKE
void V_UpdatePalette (void)
{
	int		i, j;
	qboolean	new;
	byte	*basepal, *newpal;
	byte	pal[768];
	float	r,g,b,a;
	int		ir, ig, ib;
	qboolean force;

	V_CalcPowerupCshift ();

	new = false;

	for (i=0 ; i<NUM_CSHIFTS ; i++)
	{
		if (cl.cshifts[i].percent != cl.prev_cshifts[i].percent)
		{
			new = true;
			cl.prev_cshifts[i].percent = cl.cshifts[i].percent;
		}
		for (j=0 ; j<3 ; j++)
			if (cl.cshifts[i].destcolor[j] != cl.prev_cshifts[i].destcolor[j])
			{
				new = true;
				cl.prev_cshifts[i].destcolor[j] = cl.cshifts[i].destcolor[j];
			}
	}

// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime*150;
	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime*100;
	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	force = V_CheckGamma ();
	if (!new && !force)
		return;

	V_CalcBlend ();

	a = v_blend[3];
	r = 255*v_blend[0]*a;
	g = 255*v_blend[1]*a;
	b = 255*v_blend[2]*a;

	a = 1-a;
	for (i=0 ; i<256 ; i++)
	{
		ir = i*a + r;
		ig = i*a + g;
		ib = i*a + b;
		if (ir > 255)
			ir = 255;
		if (ig > 255)
			ig = 255;
		if (ib > 255)
			ib = 255;

		ramps[0][i] = gammatable[ir];
		ramps[1][i] = gammatable[ig];
		ramps[2][i] = gammatable[ib];
	}

	basepal = host_basepal;
	newpal = pal;

	for (i=0 ; i<256 ; i++)
	{
		ir = basepal[0];
		ig = basepal[1];
		ib = basepal[2];
		basepal += 3;

		newpal[0] = ramps[0][ir];
		newpal[1] = ramps[1][ig];
		newpal[2] = ramps[2][ib];
		newpal += 3;
	}

	VID_ShiftPalette (pal);
}
#else	// !GLQUAKE

void D_AlphaShift (void);

int	foralphashift;

extern cvar_t *v_saturation;
void V_UpdatePalette (void)
{
	unsigned int		i, j;
	qboolean	new;
	byte	*basepal, *newpal;
	byte	pal[768];
	int		r,g,b,s;
	float	sat, cont, gamma;
	
	
	qboolean force, forcetwo, forcethree;

	gamma = v_gamma->value;
	sat = v_saturation->value;
	cont = v_contrast->value;
	foralphashift = r_alphashift->value;
	V_CalcPowerupCshift ();

	new = false;
	;
	for (i=0 ; i<NUM_CSHIFTS ; i++)
	{
		if (cl.cshifts[i].percent != cl.prev_cshifts[i].percent)
		{
			new = true;
			cl.prev_cshifts[i].percent = cl.cshifts[i].percent;
		}
		
			if (cl.cshifts[i].destcolor[0] != cl.prev_cshifts[i].destcolor[0])
			{
				new = true;
				cl.prev_cshifts[i].destcolor[0] = cl.cshifts[i].destcolor[0];
			}
			if (cl.cshifts[i].destcolor[1] != cl.prev_cshifts[i].destcolor[1])
			{
				new = true;
				cl.prev_cshifts[i].destcolor[1] = cl.cshifts[i].destcolor[1];
			}
			if (cl.cshifts[i].destcolor[2] != cl.prev_cshifts[i].destcolor[2])
			{
				new = true;
				cl.prev_cshifts[i].destcolor[2] = cl.cshifts[i].destcolor[2];
			}
	}

// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime*150;
	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime*100;
	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	force = V_CheckGamma ();
	forcetwo = V_CheckSaturation ();
	forcethree = V_CheckContrast ();
	if (!new && !force && !forcetwo && !forcethree)
		return;
	

	basepal = host_basepal;
	newpal = pal;


	if (foralphashift)
	{
	for (i=0 ; i<256 ; i++)
	{
		r = basepal[0];
		g = basepal[1];
		b = basepal[2];
		for (j=0 ; j<NUM_CSHIFTS ; j++)
		{
			r += (cl.cshifts[j].percent*0)>>8;
			g += (cl.cshifts[j].percent*0)>>8;
			b += (cl.cshifts[j].percent*0)>>8;

		}

		// jitspoian saturation stuff hacked in through leilei luck work
		r = r * cont;
		g = g * cont;
		b = b * cont;
		
		s = (r * 0.33333) + (g * 0.33333) + (b * 0.33333);
		r = s + (r - s) * sat;
		g = s + (g - s) * sat;
		b = s + (b - s) * sat;


		if (r > 255) r = 255; if (r < 0) r = 0;
		if (g > 255) g = 255; if (g < 0) g = 0;
		if (b > 255) b = 255; if (b < 0) b = 0;


		basepal += 3;

		newpal[0] = gammatable[r];
		newpal[1] = gammatable[g];
		newpal[2] = gammatable[b];
		newpal += 3;
	}
	}
		else
		{
	for (i=0 ; i<256 ; i++)
	{
		r = basepal[0];
		g = basepal[1];
		b = basepal[2];

		for (j=0 ; j<NUM_CSHIFTS ; j++)
		{
			r += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[0]-r))>>8;
			g += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[1]-g))>>8;
			b += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[2]-b))>>8;

		}


		// jitspoian saturation stuff hacked in through leilei luck work
		r = r * 0.5 * (cont * 2);
		g = g * 0.5 * (cont * 2);
		b = b * 0.5 * (cont * 2);
		
		s = (r * 0.333) + (g * 0.333) + (b * 0.333);
		r = s + (r - s) * sat;
		g = s + (g - s) * sat;
		b = s + (b - s) * sat;


		if (r > 255) r = 255; if (r < 0) r = 0;
		if (g > 255) g = 255; if (g < 0) g = 0;
		if (b > 255) b = 255; if (b < 0) b = 0;


		basepal += 3;




		newpal[0] = gammatable[r];
		newpal[1] = gammatable[g];
		newpal[2] = gammatable[b];
		newpal += 3;
	}
		}
		
	// leilei - alpha shifting
	if (r_alphashift->value){
		shiftshif = FindColor(cl.cshifts[j].destcolor[0],cl.cshifts[j].destcolor[1],cl.cshifts[j].destcolor[2]);
		
		V_CalcAlphaShift();
	}

	shiftalpha = ((int)cl.cshifts[j].percent);
//	if (!foralphashift)
	VID_ShiftPalette (pal);
//	else
//	V_CalcAlphaShift();
}
#endif	// !GLQUAKE


/*
==============================================================================

						VIEW RENDERING

==============================================================================
*/

float angledelta (float a)
{
	a = anglemod(a);
	if (a > 180)
		a -= 360;
	return a;
}
#define IR_YAWRANGE	100.0f
#define IR_PITCHRANGE	60.0f
/*
==================
CalcGunAngle
==================
*/
void CalcGunAngle (void)
{
	float	yaw, pitch, move;
	static float oldyaw = 0;
	static float oldpitch = 0;

	yaw = r_refdef.viewangles[YAW];
	pitch = -r_refdef.viewangles[PITCH];

	yaw = angledelta(yaw - r_refdef.viewangles[YAW]) * 0.4;
	if (yaw > 10)
		yaw = 10;
	if (yaw < -10)
		yaw = -10;
	pitch = angledelta(-pitch - r_refdef.viewangles[PITCH]) * 0.4;
	if (pitch > 10)
		pitch = 10;
	if (pitch < -10)
		pitch = -10;
	move = host_frametime*20; 
	if (yaw > oldyaw)
	{
		if (oldyaw + move < yaw)
			yaw = oldyaw + move;
	}
	else
	{
		if (oldyaw - move > yaw)
			yaw = oldyaw - move;
	}

	if (pitch > oldpitch)
	{
		if (oldpitch + move < pitch)
			pitch = oldpitch + move;
	}
	else
	{
		if (oldpitch - move > pitch)
			pitch = oldpitch - move;
	}

	oldyaw = yaw;
	oldpitch = pitch;



	cl.viewent.angles[YAW] = r_refdef.viewangles[YAW] + yaw;
	cl.viewent.angles[PITCH] = - (r_refdef.viewangles[PITCH] + pitch);

	cl.viewent.angles[ROLL] -= v_idlescale->value * sin(cl.time*v_iroll_cycle->value) * v_iroll_level->value;
	cl.viewent.angles[PITCH] -= v_idlescale->value * sin(cl.time*v_ipitch_cycle->value) * v_ipitch_level->value;
	cl.viewent.angles[YAW] -= v_idlescale->value * sin(cl.time*v_iyaw_cycle->value) * v_iyaw_level->value;

//	cl.viewent.angles[PITCH] -= bob * sin(cl.time*v_ipitch_cycle->value) * v_ipitch_level->value;
//	cl.viewent.angles[YAW] -= bob * sin(cl.time*v_iyaw_cycle->value) * v_iyaw_level->value;

	
}

/*
==============
V_BoundOffsets
==============
*/
void V_BoundOffsets (void)
{
	entity_t	*ent;

	ent = &cl_entities[cl.viewentity];

// absolutely bound refresh reletive to entity clipping hull
// so the view can never be inside a solid wall

	if (r_refdef.vieworg[0] < ent->origin[0] - 14)
		r_refdef.vieworg[0] = ent->origin[0] - 14;
	else if (r_refdef.vieworg[0] > ent->origin[0] + 14)
		r_refdef.vieworg[0] = ent->origin[0] + 14;
	if (r_refdef.vieworg[1] < ent->origin[1] - 14)
		r_refdef.vieworg[1] = ent->origin[1] - 14;
	else if (r_refdef.vieworg[1] > ent->origin[1] + 14)
		r_refdef.vieworg[1] = ent->origin[1] + 14;
	if (r_refdef.vieworg[2] < ent->origin[2] - 22)
		r_refdef.vieworg[2] = ent->origin[2] - 22;
	else if (r_refdef.vieworg[2] > ent->origin[2] + 30)
		r_refdef.vieworg[2] = ent->origin[2] + 30;
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
#ifdef VMTOC
void V_AddIdle(float idlescale)
{
	r_refdef.viewangles[ROLL]  += idlescale * sin(cl.time*v_iroll_cycle->value)  * v_iroll_level->value;
	r_refdef.viewangles[PITCH] += idlescale * sin(cl.time*v_ipitch_cycle->value) * v_ipitch_level->value;
	r_refdef.viewangles[YAW]   += idlescale * sin(cl.time*v_iyaw_cycle->value)   * v_iyaw_level->value;
}
#else
void V_AddIdle (void)
{
	
//	non = V_CalcNon ();
	r_refdef.viewangles[ROLL] += v_idlescale->value * sin(cl.time*v_iroll_cycle->value) * v_iroll_level->value;
	r_refdef.viewangles[PITCH] += v_idlescale->value * sin(cl.time*v_ipitch_cycle->value) * v_ipitch_level->value;
	r_refdef.viewangles[YAW] += v_idlescale->value * sin(cl.time*v_iyaw_cycle->value) * v_iyaw_level->value;
}
#endif

/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll (void)
{
	float	side;

	side = V_CalcRoll (cl_entities[cl.viewentity].angles, cl.velocity);
	r_refdef.viewangles[ROLL] += side;

	if (v_dmg_time > 0)
	{
		r_refdef.viewangles[ROLL] += v_dmg_time/v_kicktime->value*v_dmg_roll;
		r_refdef.viewangles[PITCH] += v_dmg_time/v_kicktime->value*v_dmg_pitch;
		v_dmg_time -= host_frametime;
	}

	if (cl.stats[STAT_HEALTH] <= 0)
	{
		if (cl_diecam->value == 2) {	// q3ish

		}
		else if (cl_diecam->value == 3) {	// uish

		}

		else{
		r_refdef.viewangles[ROLL] = 80;	// dead view angle
		}
		return;
	}

}


/*
==================
V_CalcIntermissionRefdef

==================
*/
#ifndef VMTOC
void V_CalcIntermissionRefdef (void)
{
	entity_t	*ent, *view;
	float		old;

// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	VectorCopy (ent->origin, r_refdef.vieworg);
	VectorCopy (ent->angles, r_refdef.viewangles);
	view->model = NULL;

// always idle in intermission
	old = v_idlescale->value;
	v_idlescale->value = 1;
	V_AddIdle ();
	v_idlescale->value = old;
}
#endif
#define crappy = 1; // enable broken thing.
static vec_t lowpass(vec_t value, vec_t frac, vec_t *store)
{
	frac = bound(0, frac, 1);
	return (*store = *store * (1 - frac) + value * frac);
}

static vec_t lowpass_limited(vec_t value, vec_t frac, vec_t limit, vec_t *store)
{
	lowpass(value, frac, store);
	return (*store = bound(value - limit, *store, value + limit));
}

static vec_t highpass(vec_t value, vec_t frac, vec_t *store)
{
	return value - lowpass(value, frac, store);
}

static vec_t highpass_limited(vec_t value, vec_t frac, vec_t limit, vec_t *store)
{
	return value - lowpass_limited(value, frac, limit, store);
}

static void lowpass3(vec3_t value, vec_t fracx, vec_t fracy, vec_t fracz, vec3_t store, vec3_t out)
{
	out[0] = lowpass(value[0], fracx, &store[0]);
	out[1] = lowpass(value[1], fracy, &store[1]);
	out[2] = lowpass(value[2], fracz, &store[2]);
}

static void highpass3(vec3_t value, vec_t fracx, vec_t fracy, vec_t fracz, vec3_t store, vec3_t out)
{
	out[0] = highpass(value[0], fracx, &store[0]);
	out[1] = highpass(value[1], fracy, &store[1]);
	out[2] = highpass(value[2], fracz, &store[2]);
}

static void highpass3_limited(vec3_t value, vec_t fracx, vec_t limitx, vec_t fracy, vec_t limity, vec_t fracz, vec_t limitz, vec3_t store, vec3_t out)
{
	out[0] = highpass_limited(value[0], fracx, limitx, &store[0]);
	out[1] = highpass_limited(value[1], fracy, limity, &store[1]);
	out[2] = highpass_limited(value[2], fracz, limitz, &store[2]);
}

/*
==================
V_CalcRefdef

==================
*/
extern cvar_t *temp2;
#ifdef VMTOC
static void V_CalcRefdef(void)
{
	entity_t	*ent;
	int			i;
	vec3_t		forward, right, up;
	vec3_t		angles;
	float		bob;
	static float oldz = 0;

// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
	if (splitpass)
		ent = &cl_entities[clsplit.viewentity];

	if (cl.intermission)
	{
	// intermission / finale rendering
		ent = &cl_entities[cl.viewentity];

		VectorCopy(ent->origin, r_refdef.vieworg);
		VectorCopy(ent->angles, r_refdef.viewangles);

		V_AddIdle(1.0f);
		return;
	}

	if (cl.paused)
		return;


	V_DriftPitch ();


// transform the view offset by the model's matrix to get the offset from
// model origin for the view
	ent->angles[YAW] = cl.viewangles[YAW];	// the model should face the view dir
	ent->angles[PITCH] = -cl.viewangles[PITCH];	// the model should face the view dir

	

	bob = V_CalcBob ();
	
// refresh position
	VectorCopy (ent->origin, r_refdef.vieworg);
	r_refdef.vieworg[2] += cl.viewheight + bob;

// never let it sit exactly on a node line, because a water plane can
// dissapear when viewed with the eye exactly on it.
// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdef.vieworg[0] += 1.0/32;
	r_refdef.vieworg[1] += 1.0/32;
	r_refdef.vieworg[2] += 1.0/32;

	VectorCopy (cl.viewangles, r_refdef.viewangles);
	V_CalcViewRoll ();
	V_AddIdle (v_idlescale->value);

// offsets
	angles[PITCH] = -ent->angles[PITCH]; // because entity pitches are actually backward
	angles[YAW]   = ent->angles[YAW];
	angles[ROLL]  = ent->angles[ROLL];

	AngleVectors (angles, forward, right, up);

	for (i=0 ; i<3 ; i++)
		r_refdef.vieworg[i] += scr_ofsx->value*forward[i] + scr_ofsy->value*right[i] + scr_ofsz->value*up[i];


	V_BoundOffsets ();
	
// set up gun position
	VectorCopy (cl.viewangles, cl.weapon_angles);

	cl.weapon_angles[YAW]   = r_refdef.viewangles[YAW];
	cl.weapon_angles[PITCH] = -r_refdef.viewangles[PITCH];

	cl.weapon_angles[ROLL]  -= v_idlescale->value * sin(cl.time*v_iroll_cycle->value)  * v_iroll_level->value;
	cl.weapon_angles[PITCH] -= v_idlescale->value * sin(cl.time*v_ipitch_cycle->value) * v_ipitch_level->value;
	cl.weapon_angles[YAW]   -= v_idlescale->value * sin(cl.time*v_iyaw_cycle->value)   * v_iyaw_level->value;

	VectorCopy (ent->origin, cl.weapon_origin);
	cl.weapon_origin[2] += cl.viewheight;

	for (i=0 ; i<3 ; i++)
	{
		cl.weapon_origin[i] += forward[i]*bob*0.4;
	}
	cl.weapon_origin[2] += bob;

//	if (v_fudgeweapon->value)
	{
	// fudge position around to keep amount of weapon visible
	// roughly equal with different FOV

		if (scr_viewsize->value == 110)
			cl.weapon_origin[2] += 1;
		else if (scr_viewsize->value == 100)
			cl.weapon_origin[2] += 2;
		else if (scr_viewsize->value == 90)
			cl.weapon_origin[2] += 1;
		else if (scr_viewsize->value == 80)
			cl.weapon_origin[2] += 0.5;
	}

// set up the refresh position
	VectorAdd (r_refdef.viewangles, cl.punchangle, r_refdef.viewangles);

// smooth out stair step ups
	if (cl.onground && ent->origin[2] - oldz > 0)
	{
		float steptime;
		
		steptime = cl.time - cl.oldtime;
		if (steptime < 0)
	//FIXME		I_Error ("steptime < 0");
			steptime = 0;

		oldz += steptime * 80;
		if (oldz > ent->origin[2])
			oldz = ent->origin[2];
		if (ent->origin[2] - oldz > 12)
			oldz = ent->origin[2] - 12;
		r_refdef.vieworg[2] += oldz - ent->origin[2];
		cl.weapon_origin[2] += oldz - ent->origin[2];
	}
	else
		oldz = ent->origin[2];

	if (chase_active->value || deathcam_yesiamdead)
		Chase_Update ();

	
	
}



void V_CalcRefdefOld (void)
{
	V_CalcRefdef();
}
#else
vec3_t gunholdorg;
vec3_t gunholdang;
float  gunholdtime;
float  gunbobtime;
float  lastgunbob;
int		skipbob;		// skip the update of the weapon bob (hack)
float  gunbobthen = 0;
float  gunaimtime;
extern	vec3_t	reflectorg;
void V_CalcRefdef (void)
{
	entity_t	*ent, *view;
	int			i;
	vec3_t		desiredaim;
							float s;
//	struct model_s		oldmodel;	// Was to be used for cl_gundraw 4 (show old model holstering first)
	float t;
	vec3_t		forward, right, up;
	vec3_t		angles;
	float		bob, draw;//, undraw;
	
	int		holdgun;

	static float oldz = 0;
	float bspeed;
		double xyspeed, bobfall;
					float cycle;
					vec_t frametime;
						float vieworg[3], viewangles[3];//, smoothtime;
	float gunorg[3], gunangles[3];
//	float snifforg[3], snifforg2[3]; // leilei's leixperiment.
//	vec3_t sniffme;
	V_DriftPitch ();
	frametime = cl.time;
// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

// transform the view offset by the model's matrix to get the offset from
// model origin for the view
	ent->angles[YAW] = cl.viewangles[YAW];	// the model should face
										// the view dir
	ent->angles[PITCH] = -cl.viewangles[PITCH];	// the model should face
										// the view dir

	

	bob = V_CalcBob ();


//	bob = 0;





	if (chase_active->value || deathcam_yesiamdead)
	skipbob = 1; // don't bob in the chase cam...
	
// refresh position
	VectorCopy (ent->origin, r_refdef.vieworg);
	
	r_refdef.vieworg[2] += cl.viewheight; // bob was here
	//r_refdef.vieworg[1] += non;
// never let it sit exactly on a node line, because a water plane can
// dissapear when viewed with the eye exactly on it.
// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdef.vieworg[0] += 1.0/32;
	r_refdef.vieworg[1] += 1.0/32;
	r_refdef.vieworg[2] += 1.0/32;

//	VectorCopy (r_refdef.vieworg, snifforg);	

	VectorCopy (cl.viewangles, r_refdef.viewangles);
	V_CalcViewRoll ();
//	V_AddIdle ();// leilei - done elsewhere

// offsets
	angles[PITCH] = -ent->angles[PITCH];	// because entity pitches are
											//  actually backward
	angles[YAW] = ent->angles[YAW];
	angles[ROLL] = ent->angles[ROLL];

	AngleVectors (angles, forward, right, up);

	for (i=0 ; i<3 ; i++)
		r_refdef.vieworg[i] += scr_ofsx->value*forward[i]
			+ scr_ofsy->value*right[i]
			+ scr_ofsz->value*up[i];


	V_BoundOffsets ();

// set up gun position
	VectorCopy (cl.viewangles, view->angles);
	

	CalcGunAngle ();
		
// k it's  ended

	VectorCopy (ent->origin, view->origin);
//	if (reflectpass)
//		VectorCopy (ent->origin, view->origin2);
	view->origin[2] += cl.viewheight;
	for (i=0 ; i<3 ; i++)
	{
		view->origin[i] += forward[i]*bob*0.4;
	}

// fudge position around to keep amount of weapon visible
// roughly equal with different FOV


	if (scr_viewsize->value == 110)
		view->origin[2] += 1;
	else if (scr_viewsize->value == 100)
		view->origin[2] += 2;
	else if (scr_viewsize->value == 90)
		view->origin[2] += 1;
	else if (scr_viewsize->value == 80)
		view->origin[2] += 0.5;



	view->model = cl.model_precache[cl.stats[STAT_WEAPON]];

	// leilei - gundraw
	// FIXME: Skip draw animation on models with matching number of verts
	// (i.e. spy knife and medic axe in tf)

	view->frame = cl.stats[STAT_WEAPONFRAME];
	view->colormap = vid.colormap;

// set up the refresh position
	VectorAdd (r_refdef.viewangles, cl.punchangle, r_refdef.viewangles);
	
// smooth stair stepping, but only if onground and enabled
         
// smooth out stair step ups
if (cl.onground && ent->origin[2] - oldz > 0)
{
	float steptime;
	
	steptime = cl.time - cl.oldtime;
	if (steptime < 0)
//FIXME		I_Error ("steptime < 0");
		steptime = 0;

	oldz += steptime * 80;
	if (oldz > ent->origin[2])
		oldz = ent->origin[2];
	if (ent->origin[2] - oldz > 12)
		oldz = ent->origin[2] - 12;
	r_refdef.vieworg[2] += oldz - ent->origin[2];
	view->origin[2] += oldz - ent->origin[2];
}
else
	oldz = ent->origin[2];



	if (cl_gunhold->value){

		if (cl.viewent.frame > 0)
		holdgun = 1;	// leilei - hold gun 
		else
		holdgun = 0;
	}
	else
		holdgun = 0;
	if(!skipbob)
if (cl.stats[STAT_HEALTH] >= 1){

			
	// leilei - gundraw
	if(cl_gundraw->value){
			if (view->lastmodel != view->model){
		// initiate gun drawing
			cl.gundrawtime = cl.time;
				view->lastmodel = view->model;
				draw = -4;
			
			}
			if (cl.time - cl.gundrawtime < 0.2){
					draw = cl.time - cl.gundrawtime;
					draw = bound(-6, draw, 2);
					draw *= 24;
					draw -= 4.92;
					//draw = draw * 0.9;
				VectorMA (view->origin, draw, up, view->origin);
				if(cl_gundraw->value == 3)
					VectorMA (view->origin, draw * 4, right, view->origin);
				if(cl_gundraw->value == 4)
					VectorMA (view->origin, draw * -4, right, view->origin);
				if(cl_gundraw->value == 2)
					VectorMA (viewangles, draw * 7, up, cl.viewent.angles);
				
					}
	}

	
					frametime = host_frametime;
			
//
			//			VectorCopy(view->angles, cl.gunangles_prev);
					VectorCopy(view->origin, cl.gunorg_prev);
				
//
						VectorAdd(cl.gunorg_highpass, cl.gunorg_prev, cl.gunorg_highpass);
	//						highpass3_limited(view->origin, frametime*(int)cl_followmodel_side_highpass1->value, (int)cl_followmodel_side_limit->value, frametime*(int)cl_followmodel_side_highpass1->value, (int)cl_followmodel_side_limit->value, frametime*(int)cl_followmodel_up_highpass1->value, (int)cl_followmodel_up_limit->value, cl.gunorg_highpass, view->origin);
							highpass3_limited(view->origin, frametime*(int)cl_followmodel_side_highpass1->value, (int)cl_followmodel_side_limit->value, frametime*(int)cl_followmodel_side_highpass1->value, (int)cl_followmodel_side_limit->value, frametime*(int)cl_followmodel_up_highpass1->value, (int)cl_followmodel_up_limit->value, cl.gunorg_highpass, gunorg);
							//highpass3_limited(r_refdef.vieworg, frametime*cl_followmodel_side_highpass1->value, cl_followmodel_side_limit->value, frametime*cl_followmodel_side_highpass1->value, cl_followmodel_side_limit->value, frametime*cl_followmodel_up_highpass1->value, cl_followmodel_up_limit->value, cl.gunorg_highpass, gunorg);
						VectorCopy(view->origin, cl.gunorg_prev);
						VectorSubtract(cl.gunorg_highpass, cl.gunorg_prev, cl.gunorg_highpass);
					VectorCopy(r_refdef.vieworg, vieworg);
					VectorCopy(r_refdef.viewangles, viewangles);
					VectorCopy(view->origin, gunorg);
					VectorCopy(view->angles, gunangles);

					

					// 1. if we teleported, clear the frametime... the lowpass will recover the previous value then
//					if(!ent->persistent.trail_allowed) // FIXME improve this check
			//		{
						// try to fix the first highpass; result is NOT
						// perfect! TODO find a better fix
			//			VectorCopy(viewangles, cl.gunangles_prev);
			//			VectorCopy(vieworg, cl.gunorg_prev);
			//		}

					// 2. for the gun origin, only keep the high frequency (non-DC) parts, which is "somewhat like velocity"
					VectorAdd(cl.gunorg_highpass, cl.gunorg_prev, cl.gunorg_highpass);
					highpass3_limited(vieworg, frametime*cl_followmodel_side_highpass1->value, cl_followmodel_side_limit->value, frametime*cl_followmodel_side_highpass1->value, cl_followmodel_side_limit->value, frametime*cl_followmodel_up_highpass1->value, cl_followmodel_up_limit->value, cl.gunorg_highpass, gunorg);
					VectorCopy(vieworg, cl.gunorg_prev);
					VectorSubtract(cl.gunorg_highpass, cl.gunorg_prev, cl.gunorg_highpass);

					// in the highpass, we _store_ the DIFFERENCE to the actual view angles...
					VectorAdd(cl.gunangles_highpass, cl.gunangles_prev, cl.gunangles_highpass);
					cl.gunangles_highpass[PITCH] += 360 * floor((viewangles[PITCH] - cl.gunangles_highpass[PITCH]) / 360 + 0.5);
					cl.gunangles_highpass[YAW] += 360 * floor((viewangles[YAW] - cl.gunangles_highpass[YAW]) / 360 + 0.5);
					cl.gunangles_highpass[ROLL] += 360 * floor((viewangles[ROLL] - cl.gunangles_highpass[ROLL]) / 360 + 0.5);
					highpass3_limited(viewangles, frametime*cl_leanmodel_up_highpass1->value, cl_leanmodel_up_limit->value, frametime*cl_leanmodel_side_highpass1->value, cl_leanmodel_side_limit->value, 0, 0, cl.gunangles_highpass, gunangles);
					VectorCopy(viewangles, cl.gunangles_prev);
					VectorSubtract(cl.gunangles_highpass, cl.gunangles_prev, cl.gunangles_highpass);

					// 3. calculate the RAW adjustment vectors
					gunorg[0] *= (cl_followmodel->value ? -cl_followmodel_side_speed->value : 0);
					gunorg[1] *= (cl_followmodel->value ? -cl_followmodel_side_speed->value : 0);
					gunorg[2] *= (cl_followmodel->value ? -cl_followmodel_up_speed->value : 0);

					gunangles[PITCH] *= (cl_leanmodel->value ? -cl_leanmodel_up_speed->value : 0);
					gunangles[YAW] *= (cl_leanmodel->value ? -cl_leanmodel_side_speed->value : 0);
					gunangles[ROLL] = 0;
	
					// 4. perform highpass/lowpass on the adjustment vectors (turning velocity into acceleration!)
					//    trick: we must do the lowpass LAST, so the lowpass vector IS the final vector!
					highpass3(cl.viewent.origin, frametime*cl_followmodel_side_highpass->value, frametime*cl_followmodel_side_highpass->value, frametime*cl_followmodel_up_highpass->value, cl.gunorg_adjustment_highpass, gunorg);
					lowpass3(cl.viewent.origin, frametime*cl_followmodel_side_lowpass->value, frametime*cl_followmodel_side_lowpass->value, frametime*cl_followmodel_up_lowpass->value, cl.gunorg_adjustment_lowpass, gunorg);
					// we assume here: PITCH = 0, YAW = 1, ROLL = 2
					highpass3(gunangles, frametime*cl_leanmodel_up_highpass->value, frametime*cl_leanmodel_side_highpass->value, 0, cl.gunangles_adjustment_highpass, gunangles);
					lowpass3(gunangles, frametime*cl_leanmodel_up_lowpass->value, frametime*cl_leanmodel_side_lowpass->value, 0, cl.gunangles_adjustment_lowpass, gunangles);
		
					// 5. use the adjusted vectors

					if (cl_followmodel->value){
						if (cl_gunhold->value){
								if (!holdgun){
										VectorSubtract(gunorg, view->origin, gunholdorg); // leilei - back up the origin
									}
									else
									{
								VectorSubtract(gunorg, gunholdorg, gunorg); 
									}	
						}

						VectorAdd(view->origin, gunorg, cl.viewent.origin);
			
						cl.viewent.origin[0] *= 0.5;
						cl.viewent.origin[1] *= 0.5;
						cl.viewent.origin[2] = cl.viewent.origin[2] * 0.5;


					
					}
				VectorAdd(viewangles, gunangles, cl.viewent.angles);
				//
				
				cl.viewent.angles[PITCH] = - (r_refdef.viewangles[PITCH]); // correct the pitch

// leilei's aim gunning mode


#ifdef LOOKANGLE
		cl.viewent.angles[PITCH] = - (r_refdef.viewangles[PITCH] + cl.aimangle[0] * temp2->value);


	//	if (cl.aimangle[0])
	//			Sys_Error("WHAT");
		//	VectorAdd (cl.viewent.angles, cl.aimangle, cl.viewent.angles);
//				VectorCopy(cl.viewent.angles, desiredaim);
//				if (cl.aimangle[2])	// we got something!
//					desiredaim[1] = cl.aimangle[2] * 90;
//				else
//					desiredaim[1] = cl.viewent.angles[PITCH]; // we don't got something
				//cl.viewent.angles[PITCH] = cl.aimangle[2] * 90;
				//highpass3(cl.viewent.angles, gunaimtime*40, gunaimtime*40, gunaimtime*40, cl.gunorg_adjustment_highpass, desiredaim);
				//lowpass3(cl.viewent.angles, frametime*cl_followmodel_side_highpass->value, frametime*cl_followmodel_side_highpass->value, frametime*cl_followmodel_up_highpass->value, cl.gunorg_adjustment_highpass, desiredaim);
//				highpass(cl.viewent.angles[PITCH], 5, desiredaim);
//				cl.viewent.angles[PITCH] = desiredaim[1]; 
				//Con_DPrintf ("%f VIEWPITCH\n", cl.viewent.angles[PITCH]);
			//	VectorCopy (cl.aimangle, cl.viewent.angles);

			//	Con_Printf("%f %f\n", cl.aimangle[0], cl.aimangle[1]);
	//	cl.viewent.angles[YAW] = r_refdef.viewangles[YAW] - cl.aimangle[1] * IR_YAWRANGE;
        

#endif
//	  -----------------------------------------
// ** SAJT'S KEGENDARY BOBMODEL CODE STARTS HERE! **
//    -----------------------------------------
	xyspeed = bound (0, sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]), 400);

			if(cl_bobmodel->value)
					{
							// calculate for swinging gun model
							// the gun bobs when running on the ground, but doesn't bob when you're in the air.
							// Sajt: I tried to smooth out the transitions between bob and no bob, which works
							// for the most part, but for some reason when you go through a message trigger or
							// pick up an item or anything like that it will momentarily jolt the gun.
							if (!holdgun)
									gunbobtime = cl.time;
							if (holdgun){ s = gunbobtime; t = gunbobtime; 
										gunbobthen = cl.time - gunbobtime;
								}
								else s = gunbobtime * cl_bobmodel_speed->value;
						
							if (cl.onground)
							{
								if (gunbobtime - cl.hitgroundtime < 0.2)
							{
									// just hit the ground, speed the bob back up over the next 0.2 seconds
									t = gunbobtime - cl.hitgroundtime;
								t = bound(0, t, 0.2);
									t *= 5;
								}
							else
									t = 1;
							}
							else
							{
								// recently left the ground, slow the bob down over the next 0.2 seconds
								t = gunbobtime - cl.lastongroundtime;
								t = 0.2 - bound(0, t, 0.2);
								t *= 5;
							}
							bspeed = xyspeed * 0.01f;
							xyspeed = sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]);
							bspeed = bound (0, xyspeed, 400) * 0.01f;
							
							AngleVectors (view->angles, forward, right, up);

					
							// leilei - The engine now has other hacked types of DP bobbing
							// to capture other 'feels'.
							if (cl_bobmodel->value == 2){
							// Arc2  bobbing
							bob = bspeed * cl_bobmodel_side->value * (cos (s)) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * cos (s * 2) * t;
							VectorMA (view->origin, bob, up, view->origin);
							}
							else if (cl_bobmodel->value == 3){
							// Figure 8  bobbing
							bob = bspeed * cl_bobmodel_side->value * sin (s) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * sin (s * 2) * t;
							VectorMA (view->origin, bob, up, view->origin);
							}
							else if (cl_bobmodel->value == 4){
							// -ish Bobbing
							bob = bspeed * cl_bobmodel_side->value * sin (s) * (t * 0.3);
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * sin (s * 2) * t;

							VectorMA (view->origin, bob, up, view->origin);
						
							}
							else if (cl_bobmodel->value == 5){
							// A unique bob.
							// This one pushes the weapon toward the right
							// and down so it appears 'aiming' when no longer
							// moving. This will tick players off for those who
							// use the gun's center as an aiming reticle.
						//		t = t * 0.01; // soften the time!
							bob = bspeed * cl_bobmodel_side->value * (sin (s) + 6) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * (cos (s * 2) - 3) * t;
							VectorMA (view->origin, bob, up, view->origin);
						
							}
							else if (cl_bobmodel->value == 6){
							// A variation, but in figure 8
						
							bob = bspeed * cl_bobmodel_side->value * (sin (s) + 6) * t;
							VectorMA (view->origin, bob, right, view->origin);
							
							bob = bspeed * cl_bobmodel_up->value * (sin (s * 2) - 4) * t;
							VectorMA (view->origin, bob, up, view->origin);
							
							}
							else if (cl_bobmodel->value == 7){

							bob = bspeed * cl_bobmodel_side->value * sinh (s) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * sinh (s * 2) * t;
							VectorMA (view->origin, bob, up, view->origin);
							}
							else if (cl_bobmodel->value == 8){

							bob = bspeed * cl_bobmodel_side->value * cos (s) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * cos (s * 2) * t;
							VectorMA (view->origin, bob, up, view->origin);
							}
							else if (cl_bobmodel->value == 9){

							bob = bspeed * cl_bobmodel_side->value * cosh (s) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * cosh (s * 2) * t;
							VectorMA (view->origin, bob, up, view->origin);
							}
							else if (cl_bobmodel->value == 10){

							bob = bspeed * cl_bobmodel_side->value * asin (s) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * asin (s * 2) * t;
							VectorMA (view->origin, bob, up, view->origin);
							}
							else
							{
							// Default Darkplaces bobbing
							bob = bspeed * cl_bobmodel_side->value * sin (s) * t;
							VectorMA (view->origin, bob, right, view->origin);
							bob = bspeed * cl_bobmodel_up->value * cos (s * 2) * t;
							VectorMA (view->origin, bob, up, view->origin);
							}
							lastgunbob = bob;				
							//view->angles[1] = view->angles[1] +  (bob * 2); // leilei - why did i put this here?
							//view->angles[2] = view->angles[2] +  (bob * -6);
							
					}
					else
					{
							if (!holdgun)
									gunbobtime = cl.time;
							if (holdgun){ s = gunbobtime; t = gunbobtime; 
										gunbobthen = cl.time - gunbobtime;
								}
								else s = gunbobtime * cl_bobmodel_speed->value;
						
							if (cl.onground)
							{
								if (gunbobtime - cl.hitgroundtime < 0.2)
							{
									// just hit the ground, speed the bob back up over the next 0.2 seconds
									t = gunbobtime - cl.hitgroundtime;
								t = bound(0, t, 0.2);
									t *= 5;
								}
							else
									t = 1;
							}
							else
							{
								// recently left the ground, slow the bob down over the next 0.2 seconds
								t = gunbobtime - cl.lastongroundtime;
								t = 0.2 - bound(0, t, 0.2);
								t *= 5;
							}
							bspeed = xyspeed * 0.01f;
							xyspeed = sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]);
							bspeed = bound (0, xyspeed, 400) * 0.01f;
							AngleVectors (view->angles, forward, right, up);
					}

			
			




		// ok!




					// vertical view bobbing code
					if (cl_bob->value && cl_bobcycle->value)
					{
						// LordHavoc: this code is *weird*, but not replacable (I think it
						// should be done in QC on the server, but oh well, quake is quake)
						// LordHavoc: figured out bobup: the time at which the sin is at 180
						// degrees (which allows lengthening or squishing the peak or valley)
						cycle = gunbobtime / cl_bobcycle->value;
						cycle -= (int) cycle;
						if (cycle < cl_bobup->value)
							cycle = sin(M_PI * cycle / cl_bobup->value);
						else
							cycle = sin(M_PI + M_PI * (cycle-cl_bobup->value)/(1.0 - cl_bobup->value));
						// bob is proportional to velocity in the xy plane
						// (don't count Z, or jumping messes it up)
						bob = xyspeed * bound(0, cl_bob->value, 0.05);
						bob = bob*0.3 + bob*0.7*cycle;
						r_refdef.vieworg[2] += bob;
						// we also need to adjust gunorg, or this appears like pushing the gun!
						// In the old code, this was applied to vieworg BEFORE copying to gunorg,
						// but this is not viable with the new followmodel code as that would mean
						// that followmodel would work on the munged-by-bob vieworg and do feedback
						view->origin[2] += bob;
					}






// horizontal view bobbing code i couldn't get to work.
					if (cl_bob2->value && cl_bob2cycle->value)
					{
						vec3_t bob2vel;
						vec3_t forward, right, up;
						float side, front;

						cycle = gunbobtime / cl_bob2cycle->value;
						cycle -= (int) cycle;
						if (cycle < 0.5)
							cycle = cos(M_PI * cycle / 0.5); // cos looks better here with the other view bobbing using sin
						else
							cycle = cos(M_PI + M_PI * (cycle-0.5)/0.5);
						bob = bound(0, cl_bob2->value, 0.05) * cycle;

						// this value slowly decreases from 1 to 0 when we stop touching the ground.
						// The cycle is later multiplied with it so the view smooths back to normal
					//	if (cl.onground && !cl.cmd.jump) // also block the effect while the jump button is pressed, to avoid twitches when bunny-hopping
					//		cl.bob2_smooth = 1;
					//	else
						{
							if(cl.bob2_smooth > 0)
								cl.bob2_smooth -= bound(0, cl_bob2smooth->value, 1);
							else
								cl.bob2_smooth = 0;
						}

						// calculate the front and side of the player between the X and Y axes
						AngleVectors(ent->angles, forward, right, up);
						// now get the speed based on those angles. The bounds should match the same value as xyspeed's
						side = bound(-400, DotProduct (cl.velocity, right) * cl.bob2_smooth, 400);
						front = bound(-400, DotProduct (cl.velocity, forward) * cl.bob2_smooth, 400);
						VectorScale(forward, bob, forward);
						VectorScale(right, bob, right);
						// we use side with forward and front with right, so the bobbing goes
						// to the side when we walk forward and to the front when we strafe
						VectorMAMAM(side, forward, front, right, 0, up, bob2vel);
						r_refdef.vieworg[0] += bob2vel[0];
						r_refdef.vieworg[1] += bob2vel[1];
						// we also need to adjust gunorg, or this appears like pushing the gun!
						// In the old code, this was applied to vieworg BEFORE copying to gunorg,
						// but this is not viable with the new followmodel code as that would mean
						// that followmodel would work on the munged-by-bob vieworg and do feedback
						view->origin[0] += bob2vel[0];
						view->origin[1] += bob2vel[1];

					}

// fall bobbing code
					// causes the view to swing down and back up when touching the ground
					if (cl_bobfall->value && cl_bobfallcycle->value)
					{
						if (!cl.onground)
						{
							cl.bobfall_speed = bound(-400, cl.velocity[2], 0) * bound(0, cl_bobfall->value, 0.1);
							if (cl.velocity[2] < -cl_bobfallminspeed->value)
								cl.bobfall_swing = 1;
							else
								cl.bobfall_swing = 0;
						}
						else
						{
							if(cl.bobfall_swing > 0)
								cl.bobfall_swing -= bound(0, cl_bobfallcycle->value * frametime, 1);
							else
								cl.bobfall_swing = 0;

							bobfall = sin(M_PI * cl.bobfall_swing) * cl.bobfall_speed;
							r_refdef.vieworg[2] += bobfall;
							view->origin[2] += bobfall;
						}
					}
			
		}			

	V_AddIdle (); // leilei - moved it way down here.
	
	
	if (chase_active->value || deathcam_yesiamdead)
		Chase_Update ();
	if (skipbob)
			skipbob = 0;
	// leilei - water reflections
	{
	//	if (reflectpass)
	//	{
	//		vec3_t	orgin;

	//		VectorSubtract(view->origin2, view->origin, orgin);
			
			//VectorSubtract(view->origin2, orgin, reflectorg);

	//	}
	}

		

}


/*
==================
V_CalcRefdefOld


  the old one for speed
==================
*/
void V_CalcRefdefOld (void)
{
	entity_t	*ent, *view;
	int			i;
	vec3_t		forward, right, up;
	vec3_t		angles;
	float		bob;
	static float oldz = 0;

	V_DriftPitch ();

// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
// view is the weapon model (only visible from inside body)
	view = &cl.viewent;
	

// transform the view offset by the model's matrix to get the offset from
// model origin for the view
	ent->angles[YAW] = cl.viewangles[YAW];	// the model should face
										// the view dir
	ent->angles[PITCH] = -cl.viewangles[PITCH];	// the model should face
										// the view dir
										
	
	bob = V_CalcBob ();
	
// refresh position
	VectorCopy (ent->origin, r_refdef.vieworg);
	r_refdef.vieworg[2] += cl.viewheight + bob;

// never let it sit exactly on a node line, because a water plane can
// dissapear when viewed with the eye exactly on it.
// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdef.vieworg[0] += 1.0/32;
	r_refdef.vieworg[1] += 1.0/32;
	r_refdef.vieworg[2] += 1.0/32;

	VectorCopy (cl.viewangles, r_refdef.viewangles);
	V_CalcViewRoll ();
	V_AddIdle ();

// offsets
	angles[PITCH] = -ent->angles[PITCH];	// because entity pitches are
											//  actually backward
	angles[YAW] = ent->angles[YAW];
	angles[ROLL] = ent->angles[ROLL];

	AngleVectors (angles, forward, right, up);

	for (i=0 ; i<3 ; i++)
		r_refdef.vieworg[i] += scr_ofsx->value*forward[i]
			+ scr_ofsy->value*right[i]
			+ scr_ofsz->value*up[i];
	
	
	V_BoundOffsets ();
		
// set up gun position
	VectorCopy (cl.viewangles, view->angles);
	
	CalcGunAngle ();

	VectorCopy (ent->origin, view->origin);
	view->origin[2] += cl.viewheight;

	for (i=0 ; i<3 ; i++)
	{
		view->origin[i] += forward[i]*bob*0.4;
//		view->origin[i] += right[i]*bob*0.4;
//		view->origin[i] += up[i]*bob*0.8;
	}
	view->origin[2] += bob;

// fudge position around to keep amount of weapon visible
// roughly equal with different FOV

	if (scr_viewsize->value == 110)
		view->origin[2] += 1;
	else if (scr_viewsize->value == 100)
		view->origin[2] += 2;
	else if (scr_viewsize->value == 90)
		view->origin[2] += 1;
	else if (scr_viewsize->value == 80)
		view->origin[2] += 0.5;

	view->model = cl.model_precache[cl.stats[STAT_WEAPON]];
	view->frame = cl.stats[STAT_WEAPONFRAME];
	view->colormap = vid.colormap;

// set up the refresh position
	VectorAdd (r_refdef.viewangles, cl.punchangle, r_refdef.viewangles);

// smooth out stair step ups
if (cl.onground && ent->origin[2] - oldz > 0)
{
	float steptime;
	
	steptime = cl.time - cl.oldtime;
	if (steptime < 0)
//FIXME		I_Error ("steptime < 0");
		steptime = 0;

	oldz += steptime * 80;
	if (oldz > ent->origin[2])
		oldz = ent->origin[2];
	if (ent->origin[2] - oldz > 12)
		oldz = ent->origin[2] - 12;
	r_refdef.vieworg[2] += oldz - ent->origin[2];
	view->origin[2] += oldz - ent->origin[2];
}
else
	oldz = ent->origin[2];

	if (chase_active->value || deathcam_yesiamdead)
		Chase_Update ();

	
}


#endif




/*
==================
V_RenderView

The player's clipping box goes from (-16 -16 -24) to (16 16 32) from
the entity origin, so any view position inside that will be valid
==================
*/
extern vrect_t	scr_vrect;
void PushShadows (void);	// shut up msvc.
extern int	reflectpass;
void V_RenderView (void)
{
	if (con_forcedup)
		return;

// don't allow cheats in multiplayer
	if (cl.maxclients > 1)
	{
		Cvar_Set (scr_ofsx, "0");
		Cvar_Set (scr_ofsy, "0");
		Cvar_Set (scr_ofsz, "0");
	}

#ifdef VMTOC
		V_CalcRefdef ();
#else

	// wat...
		
	// leilei - some crosshair.

	if (aimlock){
			AngleVectors (cl.viewangles, forward, right, up);
			forward[0] *= 566;
			forward[1] *= 566;
			forward[2] *= 566;
	//		D_DrawSparkTrans(r_refdef.vieworg,r_refdef.vieworg,15, 0);
		}

		if(!reflectpass)
	if (cl.intermission)
	{	// intermission / finale rendering
		V_CalcIntermissionRefdef ();
	}
	else
	{
		if (!cl.paused /* && (sv.maxclients > 1 || key_dest == key_game) */ )
			if (cl_oldview->value)
			V_CalcRefdefOld ();
				else
			V_CalcRefdef ();


	
	}
#endif
	

#ifndef GLQUAKE
	R_PushShadows ();	// this is used and msvc is a paranoid liar for warning about it being unused
	
#endif


	R_PushDlights ();
//	R_PushWlights ();	
	// leilei - aim lock
if (aimlock){
	r_refdef.viewangles[0] = lockedangle[0];
	r_refdef.viewangles[1] = lockedangle[1];
//	r_refdef.viewangles[2] = lockedangle[2];
	}

		R_RenderView ();
#ifndef GLQUAKE
		if (crosshair->value && !deathcam_yesiamdead){
			vec3_t	crossthere;
			if (aimlock){
			crossthere[0] = r_refdef.viewangles[0] - lockedangle[0];
			crossthere[1] = r_refdef.viewangles[1] - lockedangle[1];
//			crossthere[2] = r_refdef.viewangles[2] - lockedangle[2];
			}
			else
			{
				crossthere[0] = 0;
				crossthere[1] = 0;
				crossthere[2] = 0;
			}
			
#ifdef SCALED2D
		//Draw_Character (scr_vrect.x + scr_vrect.width/2 + cl_crossx->value,scr_vrect.y + scr_vrect.height/2 + cl_crossy->value, '+');
			Draw_Character (scr_vrect.x + scr_vrect.width/2 + cl_crossx->value + crossthere[0],scr_vrect.y + scr_vrect.height/2 + cl_crossy->value+ crossthere[1], '+');
#else
		Draw_Character (scr_vrect.x + scr_vrect.width/2 + cl_crossx->value,scr_vrect.y + scr_vrect.height/2 + cl_crossy->value, '+');
#endif
		}
#endif

}

//============================================================================

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
=============
V_Init_Cvars
=============
*/
void V_Init_Cvars (void)
{
	lcd_x = Cvar_Get ("lcd_x", "0", CVAR_ORIGINAL);
	lcd_yaw = Cvar_Get ("lcd_yaw", "0", CVAR_ORIGINAL);

// LEI low detail
	v_detail = Cvar_Get ("v_detail", "0", CVAR_ORIGINAL);
	cl_splitscreen = Cvar_Get ("cl_splitscreen", "0", CVAR_ORIGINAL);
	v_centermove = Cvar_Get ("v_centermove", "0.15", CVAR_ORIGINAL);
	v_centerspeed = Cvar_Get ("v_centerspeed", "500", CVAR_ORIGINAL);
/*
	v_iyaw_cycle = Cvar_Get ("v_iyaw_cycle", "2", CVAR_ORIGINAL);
	v_iroll_cycle = Cvar_Get ("v_iroll_cycle", "0.5", CVAR_ORIGINAL);
	v_ipitch_cycle = Cvar_Get ("v_ipitch_cycle", "1", CVAR_ORIGINAL);
	v_iyaw_level = Cvar_Get ("v_iyaw_level", "0.3", CVAR_ORIGINAL);
	v_iroll_level = Cvar_Get ("v_iroll_level", "0.1", CVAR_ORIGINAL);
	v_ipitch_level = Cvar_Get ("v_ipitch_level", "0.3", CVAR_ORIGINAL);

	v_idlescale = Cvar_Get ("v_idlescale", "0", CVAR_ORIGINAL);
*/

	v_iyaw_cycle = Cvar_Get ("v_iyaw_cycle", "1", CVAR_ORIGINAL);
	v_iroll_cycle = Cvar_Get ("v_iroll_cycle", "0.5", CVAR_ORIGINAL);
	v_ipitch_cycle = Cvar_Get ("v_ipitch_cycle", "1", CVAR_ORIGINAL);
	v_iyaw_level = Cvar_Get ("v_iyaw_level", "0.6", CVAR_ORIGINAL);
	v_iroll_level = Cvar_Get ("v_iroll_level", "0.1", CVAR_ORIGINAL);
	v_ipitch_level = Cvar_Get ("v_ipitch_level", "0.5", CVAR_ORIGINAL);

	


	crosshair = Cvar_Get ("crosshair", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	cl_crossx = Cvar_Get ("cl_crossx", "0", CVAR_ORIGINAL);
	cl_crossy = Cvar_Get ("cl_crossy", "0", CVAR_ORIGINAL);
	gl_cshiftpercent = Cvar_Get ("gl_cshiftpercent", "100", CVAR_ORIGINAL);

	scr_ofsx = Cvar_Get ("scr_ofsx", "0", CVAR_ORIGINAL);
	scr_ofsy = Cvar_Get ("scr_ofsy", "0", CVAR_ORIGINAL);
	scr_ofsz = Cvar_Get ("scr_ofsz", "0", CVAR_ORIGINAL);

	//cl_bob = Cvar_Get ("cl_bob", "0.02", CVAR_ORIGINAL);

	// Quake defaults
	v_idlescale = Cvar_Get ("v_idlescale", "0", CVAR_ORIGINAL);

	cl_bob = Cvar_Get ("cl_bob", "0.02", CVAR_ORIGINAL);
	cl_bobcycle = Cvar_Get ("cl_bobcycle", "0.6", CVAR_ORIGINAL);
	cl_bobup = Cvar_Get ("cl_bobup", "0.5", CVAR_ORIGINAL);

	
	cl_bobmodel = Cvar_Get ("cl_bobmodel", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	cl_bobmodel_side = Cvar_Get ("cl_bobmodel_side", "0.3", CVAR_ORIGINAL);
	cl_bobmodel_up = Cvar_Get ("cl_bobmodel_up", "0.15", CVAR_ORIGINAL);
	cl_bobmodel_speed = Cvar_Get ("cl_bobmodel_speed", "7", CVAR_ORIGINAL);
	cl_bobmodel_ex = Cvar_Get ("cl_bobmodel_ex", "0", CVAR_ORIGINAL);
	
	cl_rollspeed = Cvar_Get ("cl_rollspeed", "200", CVAR_ORIGINAL);
	cl_rollangle = Cvar_Get ("cl_rollangle", "2.0", CVAR_ORIGINAL);
	r_lerpmodels = Cvar_Get ("r_lerpmodels", "0", CVAR_ARCHIVE | CVAR_ORIGINAL);	

	cl_oldview = Cvar_Get ("cl_oldview", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	cl_followmodel = Cvar_Get ("cl_followmodel", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	 cl_followmodel_up_lowpass = Cvar_Get ("cl_followmodel_up_lowpass", "40", CVAR_ORIGINAL);
	 cl_followmodel_up_highpass = Cvar_Get ("cl_followmodel_up_highpass", "1", CVAR_ORIGINAL);
	 cl_followmodel_up_highpass1 = Cvar_Get ("cl_followmodel_up_highpass1", "22", CVAR_ORIGINAL);
	 cl_followmodel_up_limit = Cvar_Get ("cl_followmodel_up_limit ", "1", CVAR_ORIGINAL);
	 cl_followmodel_up_speed = Cvar_Get ("cl_followmodel_up_speed", "0.5", CVAR_ORIGINAL);
	 cl_followmodel_side_lowpass = Cvar_Get ("cl_followmodel_side_lowpass", "39", CVAR_ORIGINAL);
	 cl_followmodel_side_highpass = Cvar_Get ("cl_followmodel_side_highpass", "5", CVAR_ORIGINAL);
	 cl_followmodel_side_highpass1 = Cvar_Get ("cl_followmodel_side_highpass1", "30", CVAR_ORIGINAL);
	 cl_followmodel_side_limit = Cvar_Get ("cl_followmodel_side_limit", "6", CVAR_ORIGINAL);
	 cl_followmodel_side_speed = Cvar_Get ("cl_followmodel_side_speed", "0.25", CVAR_ORIGINAL);
	 cl_leanmodel_up_lowpass = Cvar_Get ("cl_leanmodel_up_lowpass", "20", CVAR_ORIGINAL);
	 cl_leanmodel_up_highpass = Cvar_Get ("cl_leanmodel_up_highpass", "15", CVAR_ORIGINAL);
	 cl_leanmodel_up_highpass1 = Cvar_Get ("cl_leanmodel_up_highpass1", "5", CVAR_ORIGINAL);
	 cl_leanmodel_up_limit = Cvar_Get ("cl_leanmodel_up_limit", "50", CVAR_ORIGINAL);
	 cl_leanmodel_up_speed = Cvar_Get ("cl_leanmodel_up_speed", "0.65", CVAR_ORIGINAL);
	 cl_leanmodel_side_lowpass = Cvar_Get ("cl_leanmodel_side_lowpass", "20", CVAR_ORIGINAL);
	 cl_leanmodel_side_highpass = Cvar_Get ("cl_leanmodel_side_highpass", "3", CVAR_ORIGINAL);
	 cl_leanmodel_side_highpass1 = Cvar_Get ("cl_leanmodel_side_highpass1", "30", CVAR_ORIGINAL);
	 cl_leanmodel_side_limit = Cvar_Get ("cl_leanmodel_side_limit", "35", CVAR_ORIGINAL);
	 cl_leanmodel_side_speed = Cvar_Get ("cl_leanmodel_side_speed", "0.7", CVAR_ORIGINAL);
	 cl_leanmodel = Cvar_Get ("cl_leanmodel", "0", CVAR_ARCHIVE | CVAR_ORIGINAL);
	 cl_bob2 = Cvar_Get ("cl_bob2", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	 cl_bob2cycle = Cvar_Get ("cl_bob2cycle", "0.6", CVAR_ORIGINAL);	 
	 cl_bob2smooth = Cvar_Get ("cl_bob2smooth", "0.05", CVAR_ORIGINAL);
	 cl_bobfall = Cvar_Get ("cl_bobfall", "0", CVAR_ARCHIVE | CVAR_ORIGINAL);
	 cl_bobfallcycle = Cvar_Get ("cl_bobfallcycle", "3", CVAR_ORIGINAL);
	 cl_bobfallminspeed = Cvar_Get ("cl_bobfallminspeed", "200", CVAR_ORIGINAL);
	
	 // to be done (bobfall into walls)
	 cl_bobslam = Cvar_Get ("cl_bobslam", "0", CVAR_ORIGINAL);
	 cl_bobslamcycle = Cvar_Get ("cl_bobslamcycle", "3", CVAR_ORIGINAL);
	 cl_bobslamminspeed = Cvar_Get ("cl_bobslamminspeed", "200", CVAR_ORIGINAL);



	cl_gundraw = Cvar_Get ("cl_gundraw", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	cl_gunhold = Cvar_Get ("cl_gunhold", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	cl_gunsniff = Cvar_Get ("cl_gunsniff", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	v_kicktime = Cvar_Get ("v_kicktime", "0.5", CVAR_ORIGINAL);
	v_kickroll = Cvar_Get ("v_kickroll", "0.6", CVAR_ORIGINAL);
	v_kickpitch = Cvar_Get ("v_kickpitch", "0.6", CVAR_ORIGINAL);

	BuildGammaTable (1.0);	// no gamma yet
	v_gamma = Cvar_Get ("gamma", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	v_saturation =	Cvar_Get ("saturation", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);	// leilei - saturation
	v_contrast =	Cvar_Get ("contrast", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);		// leilei - contrast
	
}
// 2001-09-18 New cvar system by Maddes (Init)  end




// leilei - aim lock hack
void V_LockAim (void){
//	Con_Printf("SP..........");
	lockedangle[0] = r_refdef.viewangles[0];
	lockedangle[1] = r_refdef.viewangles[1];
//	lockedangle[2] = r_refdef.viewangles[2];
	aimlock = 1;

};

void V_UnlockAim (void){
	
//	Con_Printf("OON!\n");
	aimlock = 0;
	VectorCopy(lockedangle, cl.viewangles);

};
/*
=============
V_Init
=============
*/
void V_Init (void)
{
	Cmd_AddCommand ("v_cshift", V_cshift_f);
	Cmd_AddCommand ("bf", V_BonusFlash_f);
	Cmd_AddCommand ("centerview", V_StartPitchDrift);

	Cmd_AddCommand ("+lockaim", V_LockAim);
	Cmd_AddCommand ("-lockaim", V_UnlockAim);
}
