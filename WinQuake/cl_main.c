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
// cl_main.c  -- client main loop

#include "quakedef.h"

// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// these two are not intended to be set directly
cvar_t	*cl_name;
cvar_t	*cl_color;

cvar_t	*cl_shownet;	// can be 0, 1, or 2
cvar_t	*cl_nolerp;

cvar_t	*cl_sbar;
cvar_t	*cl_hudswap;

cvar_t	*lookspring;
cvar_t	*lookstrafe;
cvar_t	*sensitivity;

cvar_t	*m_pitch;
cvar_t	*m_yaw;
cvar_t	*m_forward;
cvar_t	*m_side;
cvar_t	*m_look;	// 2001-12-16 M_LOOK cvar by Heffo/Maddes

cvar_t	*cl_showfps;	// 2001-11-31 FPS display by QuakeForge/Muff

cvar_t	*cl_compatibility;	// 2001-12-24 Keeping full backwards compatibility by Maddes

// 2001-09-20 Configurable entity limits by Maddes  start
cvar_t	*cl_entities_min;
cvar_t	*cl_entities_min_static;
cvar_t	*cl_entities_min_temp;
// 2001-09-20 Configurable entity limits by Maddes  end

client_static_t	cls;
client_state_t	cl;

// split....

client_static_t	clssplit;
client_state_t	clsplit;
// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
// 2001-09-20 Configurable entity limits by Maddes  start
/*
entity_t		cl_entities[MAX_EDICTS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
*/
entity_t		*cl_entities;
entity_t		*cl_static_entities;
// 2001-09-20 Configurable entity limits by Maddes  end
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];
shadow_t		cl_shadows[MAX_SHADOWS];
wlight_t		cl_wlights[MAX_WLIGHTS];
flare_t			cl_flares[MAX_FLARES];
int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];

/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	int			i;

	if (!sv.active)
		Host_ClearMemory ();

// wipe the entire cl structure
	memset (&cl, 0, sizeof(cl));

	SZ_Clear (&cls.message);

// clear other arrays
	memset (cl_efrags, 0, sizeof(cl_efrags));
//	memset (cl_entities, 0, sizeof(cl_entities));	// 2001-09-20 Configurable entity limits by Maddes
	memset (cl_dlights, 0, sizeof(cl_dlights));
	memset (cl_shadows, 0, sizeof(cl_shadows));
	memset (cl_wlights, 0, sizeof(cl_wlights));
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
//	memset (cl_temp_entities, 0, sizeof(cl_temp_entities));	// 2001-09-20 Configurable entity limits by Maddes
	memset (cl_beams, 0, sizeof(cl_beams));

//
// allocate the efrags and chain together into a free list
//
	cl.free_efrags = cl_efrags;
	for (i=0 ; i<MAX_EFRAGS-1 ; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;
}

/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
// stop sounds (especially looping!)
	S_StopAllSounds (true);


// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

// if running a local server, shut it down
	if (cls.demoplayback)
		CL_StopPlayback ();
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer(false);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
}

void CL_Disconnect_f (void)
{
	CL_Disconnect ();
	if (sv.active)
		Host_ShutdownServer (false);
}




/*
=====================
CL_EstablishConnection

Host should be either "local" or a net address to be passed on
=====================
*/
void CL_EstablishConnection (char *host)
{
	if (cls.state == ca_dedicated)
		return;

	if (cls.demoplayback)
		return;

	CL_Disconnect ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon)
		Host_Error ("CL_Connect: connect failed");
	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host);

	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing
	VID_HandlePause (false);	// leilei - lock the mouse back in
}

/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
	char 	str[8192];

	Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
// 2000-04-30 NVS HANDSHAKE SRV<->CL by Maddes  start
		if (!sv.active)
		{
			Cvar_Set(nvs_current_ssvc, "0");
		}
		Cvar_Set(nvs_current_csvc, "0");
		Cvar_Set(nvs_current_cclc, "0");
// 2001-12-24 Keeping full backwards compatibility by Maddes  start
		if (!(cl_compatibility->value))	// request, unlike the original Quake executable
		{
// 2001-12-24 Keeping full backwards compatibility by Maddes  end
			MSG_WriteByte (&cls.message, clc_stringcmd);
			MSG_WriteString (&cls.message, va("nvs_request %1.2f\n", nvs_current_cclc->maxvalue));
		}	// 2001-12-24 Keeping full backwards compatibility by Maddes
// 2000-04-30 NVS HANDSHAKE SRV<->CL by Maddes  end

// 2001-09-20 Configurable limits by Maddes  start
// 2001-09-20 Configurable entity limits by Maddes  start
		cl.max_edicts = 0;
		cl.max_static_edicts = 0;
		cl.max_temp_edicts = 0;
		cl_entities = NULL;
		cl_static_entities = NULL;
		cl_temp_entities = NULL;
// 2001-09-20 Configurable entity limits by Maddes  end
// 2001-12-24 Keeping full backwards compatibility by Maddes  start
		if (!(cl_compatibility->value))	// request, unlike the original Quake executable
		{
// 2001-12-24 Keeping full backwards compatibility by Maddes  end
			MSG_WriteByte (&cls.message, clc_stringcmd);
			MSG_WriteString (&cls.message, "limit_request\n");
		}	// 2001-12-24 Keeping full backwards compatibility by Maddes
// 2001-09-20 Configurable limits by Maddes  end

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "prespawn\n");
		break;

	case 2:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("name \"%s\"\n", cl_name->string));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", ((int)cl_color->value)>>4, ((int)cl_color->value)&15));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		sprintf (str, "spawn %s\n", cls.spawnparms);
		MSG_WriteString (&cls.message, str);
		break;

	case 3:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin\n");
		Cache_Report ();		// print remaining memory
		break;

	case 4:
		SCR_EndLoadingPlaque ();		// allow normal screen updates
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/
void CL_NextDemo (void)
{
	char	str[1024];

	if (cls.demonum == -1)
		return;		// don't play demos

	SCR_BeginLoadingPlaque ();

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
			Con_Printf ("No demos listed with startdemos\n");
			cls.demonum = -1;
			return;
		}
	}

	sprintf (str,"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}

/*
==============
CL_PrintEntities_f
==============
*/
void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int			i;

	for (i=0,ent=cl_entities ; i<cl.num_entities ; i++,ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n"
		,ent->model->name,ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}


/*
===============
SetPal

Debugging tool, just flashes the screen
===============
*/
void SetPal (int i)
{
#if 0
	static int old;
	byte	pal[768];
	int		c;

	if (i == old)
		return;
	old = i;

	if (i==0)
		VID_SetPalette (host_basepal);
	else if (i==1)
	{
		for (c=0 ; c<768 ; c+=3)
		{
			pal[c] = 0;
			pal[c+1] = 255;
			pal[c+2] = 0;
		}
		VID_SetPalette (pal);
	}
	else
	{
		for (c=0 ; c<768 ; c+=3)
		{
			pal[c] = 0;
			pal[c+1] = 0;
			pal[c+2] = 255;
		}
		VID_SetPalette (pal);
	}
#endif
}

/*
===============
CL_AllocDlight

===============
*/
dlight_t *CL_AllocDlight (int key)
{
	int		i;
	dlight_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
			//	dl->color[0] = dl->color[1] = dl->color[2] = 1.0f;
				dl->color[0] = dl->color[1] = dl->color[2] = 1; // LordHavoc: .lit support
				dl->flashcolor[0] = 1.0f;
				dl->flashcolor[1] = 0.5f;
				dl->flashcolor[2] = 0.0f;
				dl->flashcolor[3] = 0.2f;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
			dl->color[0] = dl->color[1] = dl->color[2] = 1; // LordHavoc: .lit support
			dl->flashcolor[0] = 1.0f;
			dl->flashcolor[1] = 0.5f;
			dl->flashcolor[2] = 0.0f;
			dl->flashcolor[3] = 0.2f;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
	dl->color[0] = dl->color[1] = dl->color[2] = 1; // LordHavoc: .lit support
	dl->flashcolor[0] = 1.0f;
	dl->flashcolor[1] = 0.5f;
	dl->flashcolor[2] = 0.0f;
	dl->flashcolor[3] = 0.2f;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
	return dl;
}







/*
===============
CL_AllocShadow

  leilei's shadow hack

===============
*/
shadow_t *CL_AllocShadow (int key)
{
	int		i;
	shadow_t	*dl;	//haha still called 'dl'

// first look for an exact key match
	if (key)
	{
		dl = cl_shadows;
		for (i=0 ; i<MAX_SHADOWS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_shadows;
	for (i=0 ; i<MAX_SHADOWS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_shadows[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}



/*
===============
CL_AllocWlight

	Wlights are only used for calculating 
	the light normals on models
	they are not added to the lightmap

===============
*/
wlight_t *CL_AllocWlight (int key)
{
	int		i;
	wlight_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_wlights;
		for (i=0 ; i<MAX_WLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
			//	dl->color[0] = dl->color[1] = dl->color[2] = 1.0f;
				dl->color[0] = dl->color[1] = dl->color[2] = 1; // LordHavoc: .lit support

// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
				Con_Printf("got one\n");
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_wlights;
	for (i=0 ; i<MAX_WLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
			dl->color[0] = dl->color[1] = dl->color[2] = 1; // LordHavoc: .lit support
			
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
			Con_Printf("got two\n");
			return dl;
		}
	}

	dl = &cl_wlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
	dl->color[0] = dl->color[1] = dl->color[2] = 1; // LordHavoc: .lit support
Con_Printf("got three\n");
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
	return dl;
}


/*
===============
CL_DecayLights

===============
*/
void CL_DecayLights (void)
{
	int			i;
	dlight_t	*dl;
	shadow_t	*shd;
	
	float		time;

	time = cl.time - cl.oldtime;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time || !dl->radius)
			continue;

		dl->radius -= time*dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}

	shd = cl_shadows;
	for (i=0 ; i<MAX_SHADOWS ; i++, dl++)
	{
		if (shd->die < cl.time || !shd->radius)
			continue;

		shd->radius -= time*1;
		if (shd->radius < 0)
			shd->radius = 0;
	}



}


/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
float	CL_LerpPoint (void)
{
	float	f, frac;

	f = cl.mtime[0] - cl.mtime[1];

	if (!f || cl_nolerp->value || cls.timedemo || sv.active)
	{
		cl.time = cl.mtime[0];
		return 1;
	}

	if (f > 0.1)
	{	// dropped packet, or start of demo
		cl.mtime[1] = cl.mtime[0] - 0.1;
		f = 0.1;
	}
	frac = (cl.time - cl.mtime[1]) / f;
//Con_Printf ("frac: %f\n",frac);
	if (frac < 0)
	{
		if (frac < -0.01)
		{
SetPal(1);
			cl.time = cl.mtime[1];
//				Con_Printf ("low frac\n");
		}
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
		{
SetPal(2);
			cl.time = cl.mtime[0];
//				Con_Printf ("high frac\n");
		}
		frac = 1;
	}
	else
		SetPal(0);

	return frac;
}
extern int shadowhack;
extern int dyncolor;
extern int particleset;
extern int	deathcam_yesiamdead;	// leilei - deathcam
extern cvar_t *r_coloredlights;
extern cvar_t *r_coloreddyns;
/*
===============
CL_RelinkEntities
===============
*/
extern	int	particleblood;
extern  cvar_t *r_flares;
extern  cvar_t *r_flamehack;
extern cvar_t *temp1;
extern cvar_t *temp2;
void CL_RelinkEntities (void)
{
	entity_t	*ent;
#ifdef TO_INTERP
	centity_t	*cent;
#endif
	int			i, j;
	float		frac, f, d;
	vec3_t		delta;
	float		bobjrotate;
	vec3_t		oldorg;
	
	dlight_t	*dl;
	shadow_t	*shd;		// leilei - shadows
	
	unsigned int			shadpacity;
	float			shadorigin;

    float blend;
    vec3_t dee;
    int iee;

// determine partial update time
	frac = CL_LerpPoint ();

	cl_numvisedicts = 0;



//
// interpolate player info
//
	for (i=0 ; i<3 ; i++)
		cl.velocity[i] = cl.mvelocity[1][i] +
			frac * (cl.mvelocity[0][i] - cl.mvelocity[1][i]);

	if (cls.demoplayback)
	{
	// interpolate the angles
		// leilei - unrolled these
		{	
			d = cl.mviewangles[0][0] - cl.mviewangles[1][0];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[0] = cl.mviewangles[1][0] + frac*d;

				d = cl.mviewangles[0][1] - cl.mviewangles[1][1];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[1] = cl.mviewangles[1][1] + frac*d;

			d = cl.mviewangles[0][2] - cl.mviewangles[1][2];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[2] = cl.mviewangles[1][2] + frac*d;
	}
	}

	bobjrotate = anglemod(100*cl.time);

// start on the entity after the world
	for (i=1,ent=cl_entities+1 ; i<cl.num_entities ; i++,ent++)
	{
			ent->leifect = 0;
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				R_RemoveEfrags (ent);	// just became empty
			continue;
		}

// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime[0])
		{
			ent->model = NULL;

			continue;
		}

		VectorCopy (ent->origin, oldorg);


		if (ent->forcelink)
		{	// the entity was not updated in the last message
			// so move to the final spot
			VectorCopy (ent->msg_origins[0], ent->origin);
			VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			  for (j=0 ; j<3 ; j++)
			  {
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];
				if (delta[j] > 100 || delta[j] < -100)
				  f = 1;    // assume a teleportation, not a motion
			  }



		ent->origin[0] = ent->msg_origins[1][0] + f*delta[0];

				d = ent->msg_angles[0][0] - ent->msg_angles[1][0];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[0] = ent->msg_angles[1][0] + f*d;

				ent->origin[1] = ent->msg_origins[1][1] + f*delta[1];

				d = ent->msg_angles[0][1] - ent->msg_angles[1][1];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[1] = ent->msg_angles[1][1] + f*d;

				ent->origin[2] = ent->msg_origins[1][2] + f*delta[2];

				d = ent->msg_angles[0][2] - ent->msg_angles[1][2];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[2] = ent->msg_angles[1][2] + f*d;


		}


		if (ent->effects & EF_MUZZLEFLASH)
		{
			vec3_t		fv, rv, uv;

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			AngleVectors (ent->angles, fv, rv, uv);
				if (dyncolor > 2){
				dl->color[0] = 1.5f;
				dl->color[1] = 0.75f;
				dl->color[2] = 0.25f;	// darkplaces values /3
		
					}
			VectorMA (dl->origin, 18, fv, dl->origin);
			dl->radius = 200 + (rand()&31);
			dl->minlight = 32;
			dl->die = cl.time + 0.1;
		}

// rotate binary objects locally
		if (ent->model->flags & EF_ROTATE){
			
			ent->angles[1] = bobjrotate;
		}

		if (ent->effects & EF_BRIGHTFIELD)
			R_EntityParticles (ent);
#ifdef QUAKE2
		if (ent->effects & EF_DARKFIELD)
			R_DarkFieldParticles (ent);
#endif
		
		if (ent->effects & EF_BRIGHTLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			if (dyncolor > 2){
					dl->color[0] = 1.5f;
				dl->color[1] = 0.75f;
				dl->color[2] = 0.25f;	// darkplaces values /3
		
					}
			dl->origin[2] += 16;
			dl->radius = 400 + (rand()&31);
			dl->die = cl.time + 0.001;
	//		if (r_flares->value > 1)
	//			R_FlareInstant2(ent->origin, 4, 135, 135, 65);	// we moved it to dynamic lights now
		}

		if (ent->effects  & EF_FLAME)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			if (dyncolor){
				dl->color[0] = 0.1;
				dl->color[1] = 0.1;
				dl->color[2] = 1;	
		
					}
				R_RocketTrail (oldorg, ent->origin, 1);
			dl->radius = 200;
			dl->die = cl.time + 0.01;
	//		if (r_flares->value > 1)
	//			R_FlareInstant2(ent->origin, 4, 145, 65, 22);	// we moved it to dynamic lights now
		}
		if (ent->effects & EF_DIMLIGHT)
		{
			dl = CL_AllocDlight (i);
				if (dyncolor > 2){
				dl->color[0] = 1.5f;
				dl->color[1] = 0.75f;
				dl->color[2] = 0.25f;	// darkplaces values /3
		
					}
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200 + (rand()&31);
			dl->die = cl.time + 0.001;
	//		if (r_flares->value > 1)
	//			R_FlareInstant(ent->origin, 4, 55, 35, 12);	// we moved it to dynamic lights now
		}
		if (ent->effects  & EF_BLUE)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			if (dyncolor){
				dl->color[0] = 0.1;
				dl->color[1] = 0.1;
				dl->color[2] = 1;	
		
					}
				
			dl->radius = 200;
			dl->die = cl.time + 0.01;
//			if (r_flares->value > 1)
//				R_FlareTest(ent->origin, 4, 65, 65, 255, 0, ent);	// we moved it to dynamic lights now
		}

		if (ent->effects & EF_RED)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			if (dyncolor){
				dl->color[0] = 1;
				dl->color[1] = 0.1;
				dl->color[2] = 0.1;	
		
					}
				
			dl->radius = 200;
			dl->die = cl.time + 0.01;
	//		if (r_flares->value > 1)
	//			R_FlareTest(ent->origin, 4, 255, 65, 65, 0, ent);	// we moved it to dynamic lights now			
		}
		if (shadowhack && !(ent->effects & EF_ADDITIVE) && !(ent->effects & EF_NODRAW)){
		{


			// We find our shadow size here

			if (ent->model){
			ent->shadowsize = (ent->model->maxs[0]);
			ent->shadowsize *= 3.9;
			shadorigin = (ent->model->maxs[2] - ent->model->mins[2]);

			if ((ent->shadowsize > 777) && (ent->shadowsize < 20))
					ent->shadowsize = 0; // hack to not make the level itself shadow

			if (ent->model->dontshadow)
				ent->shadowsize = 0; // don't try to shadow what we don't want to shadow
			if (ent->alpha > 0)
				shadpacity = 32 * ent->alpha; // make shadow less hard on alphaed stuff
				else
				shadpacity = 32;
			}

			// Ok we have a shadow ready. allocate it and do it!!!!!
			if (ent->shadowsize){
			shd = CL_AllocShadow (i);

			VectorCopy (ent->origin,  shd->origin);
			if (shadowhack == 2)	// leilei - extra directional shadow
			{
				int e;
				
				float	shasize = 1;
								shadorigin = 0;
						
				for(e=0;e<3;e++){
						shd->origin[e] += (ent->shadowvec[e] * 18);
						shasize += ent->shadowvec[e] * 0.333;
						shasize -= ent->model->mins[e] * 0.333;
						shasize += ent->model->maxs[e] * 0.333;
						
						if (shasize < 0)
						shasize *= -1;
					}
				shadpacity = 5 - (shasize * -0.1);
				
				shasize *= 0.05;

				//shadpacity *= ent->shadowopacity;

				
				//shadpacity = shasize * 0.2;
				ent->shadowsize = shasize * 36;
					
			}
			shd->minlight = shadpacity;
			shd->origin[2] -= shadorigin;
			shd->radius = ent->shadowsize;
			shd->die = cl.time + 0.001;
			}
		}

		}

// QC Glows!

		if (ent->glowsize)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			if (dyncolor){
				byte  *tempcolor;
			 tempcolor = (byte *)&d_8to24table[ent->glowcolor];
			dl->color[0] = (float)tempcolor[0] * (1.0f / 299.0f);
			dl->color[1] = (float)tempcolor[1] * (1.0f / 299.0f);
			dl->color[2] = (float)tempcolor[2] * (1.0f / 299.0f);
	//			if (r_flares->value > 1)
	//			R_FlareTest(ent->origin, 4, (int)tempcolor[0], (int)tempcolor[1], (int)tempcolor[2], 0, ent);	// we moved it to dynamic lights now
					}
				
			dl->radius = ent->glowsize;
			dl->die = cl.time + 0.01;
			
		}

		if (ent->model->flags & EF_GIB)
			if (particleblood > 2){
				R_GusherizeCurlyBloodTrail (oldorg, ent->origin, 5);
//				if (!ent->gushed){
//					vec3_t	eheheh;

			//		eheheh[0] = rand() * 1000 - 500; eheheh[1] = rand() * 1000 - 500; eheheh[2] = rand() * 1000 - 500;
//				ent->gushed = 1;
				
				//	R_RunParticleEffect(ent->origin, eheheh, 79, 60);
					
				//	R_RunParticleEffect(oldorg, eheheh, 79, 60);
//
	//				Con_Printf("GUSH!!!!!!!! %s\n", sfx->name);
		//		}

			}
			else if (particleblood < 0)
				ent->effects |= EF_NODRAW; // hide gibs on blood removal mode
			else
			R_RocketTrail (oldorg, ent->origin, 2);
		else if (ent->model->flags & EF_ZOMGIB)
			if (particleblood > 2)
			R_SpiralBloodTrail (oldorg, ent->origin);	// by the way - blood removal code 
			else										// should NEVER REMOVE EF_ZOMGIB ENTITIES
			R_RocketTrail (oldorg, ent->origin, 4);		// because it is a gameplay element (zombie gib throws)
														// instead we can make it appear less graphic by blue gelmap
		else if (ent->model->flags & EF_TRACER)
			R_RocketTrail (oldorg, ent->origin, 3);
		else if (ent->model->flags & EF_TRACER2)
			R_RocketTrail (oldorg, ent->origin, 5);
		else if (ent->model->flags & EF_ROCKET)
		{
			if (particleset == 2){
			R_RocketTrailSprites (oldorg, ent->origin, 0);

			}
			else
			{
			R_RocketTrail (oldorg, ent->origin, 0);
			}
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			if (dyncolor){
				dl->color[0] = 1.5f;
				dl->color[1] = 0.75f;
				dl->color[2] = 0.25f;	// darkplaces values /3
		
					}
				
			dl->radius = 200;
			dl->die = cl.time + 0.01;
		//	if (r_flares->value > 1)
		//		R_FlareTest(ent->origin, 4, 95, 65, 22, 0, ent);	// we moved it to dynamic lights now
			
		}
		else if (ent->model->flags & EF_GRENADE){
			if (particleset == 2)
			R_RocketTrailSprites (oldorg, ent->origin, 1);
			else
			R_RocketTrail (oldorg, ent->origin, 1);

		}
		else if (ent->model->flags & EF_TRACER3)
						if (particleset == 2)
			R_RocketTrailSprites (oldorg, ent->origin, 6);
						else
			R_RocketTrail (oldorg, ent->origin, 6);
		
		ent->forcelink = false;
		if (!ent->model->flags && EF_GIB)
//			ent->gushed = 0;	// leilei - reset gushing...
#ifndef GLQUAKE
		if (i == cl.viewentity && !chase_active->value && !reflectpass && !deathcam_yesiamdead)
			continue;
#else
		if (i == cl.viewentity && !chase_active->value && !deathcam_yesiamdead)
			continue;
#endif


		if ( ent->effects & EF_NODRAW )
			continue;
	
		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}

		if (r_flamehack->value == 3){
		if (!strcmp (ent->model->name, "progs/flame.mdl")) ent->leifect = 0;
		if (!strcmp (ent->model->name, "progs/flame2.mdl")) ent->leifect = 12;
		if (!strcmp (ent->model->name, "progs/flame3.mdl")) ent->leifect = 12;
		if ( ent->leifect == 12)
				R_FireTrail (oldorg, ent->origin, ent);
		}
		
	}

}


/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int		ret;

	cl.oldtime = cl.time;
	cl.time += host_frametime;

	do
	{
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;

		cl.last_received_message = realtime;
		CL_ParseServerMessage ();
	} while (ret && cls.state == ca_connected);

	if (cl_shownet->value)
		Con_Printf ("\n");

	CL_RelinkEntities ();
	CL_UpdateTEnts ();

//
// bring the links up to date
//
	return 0;
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t		cmd;

	if (cls.state != ca_connected)
		return;

	if (cls.signon == SIGNONS)
	{
	// get basic movement from keyboard
		CL_BaseMove (&cmd);

	// allow mice or other external controllers to add to the move
		IN_Move (&cmd);

	// send the unreliable message
		CL_SendMove (&cmd);

	}

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

// send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all

	if (!NET_CanSendMessage (cls.netcon))
	{
		Con_DPrintf ("CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1)
		Host_Error ("CL_WriteToServer: lost server connection");

	SZ_Clear (&cls.message);
}

// 2001-12-16 M_LOOK cvar by Heffo/Maddes  start
void Callback_M_Look (cvar_t *var)
{
	if ( !((in_mlook.state & 1) ^ ((int)m_look->value & 1)) && lookspring->value)
		V_StartPitchDrift();
}
// 2001-12-16 M_LOOK cvar by Heffo/Maddes  end

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
=================
CL_Init_Cvars
=================
*/
void CL_Init_Cvars (void)
{
	cl_name = Cvar_Get ("_cl_name", "player", CVAR_ARCHIVE|CVAR_ORIGINAL);
	cl_color = Cvar_Get ("_cl_color", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);

	cl_upspeed = Cvar_Get ("cl_upspeed", "200", CVAR_ORIGINAL);
	cl_forwardspeed = Cvar_Get ("cl_forwardspeed", "200", CVAR_ARCHIVE|CVAR_ORIGINAL);
	cl_backspeed = Cvar_Get ("cl_backspeed", "200", CVAR_ARCHIVE|CVAR_ORIGINAL);
 	cl_sidespeed = Cvar_Get ("cl_sidespeed", "350", CVAR_ORIGINAL);
 	cl_movespeedkey = Cvar_Get ("cl_movespeedkey", "2.0", CVAR_ORIGINAL);
 	cl_yawspeed = Cvar_Get ("cl_yawspeed", "140", CVAR_ORIGINAL);
 	cl_pitchspeed = Cvar_Get ("cl_pitchspeed", "150", CVAR_ORIGINAL);
 	cl_anglespeedkey = Cvar_Get ("cl_anglespeedkey", "1.5", CVAR_ORIGINAL);
	cl_shownet = Cvar_Get ("cl_shownet", "0", CVAR_ORIGINAL);
// 2001-09-18 New cvar system by Maddes  start
	Cvar_SetRangecheck (cl_shownet, Cvar_RangecheckInt, 0, 2);
	Cvar_Set(cl_shownet, cl_shownet->string);	// do rangecheck
// 2001-09-18 New cvar system by Maddes  end
	cl_nolerp = Cvar_Get ("cl_nolerp", "0", CVAR_ORIGINAL);
	lookspring = Cvar_Get ("lookspring", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	lookstrafe = Cvar_Get ("lookstrafe", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	sensitivity = Cvar_Get ("sensitivity", "3", CVAR_ARCHIVE|CVAR_ORIGINAL);

	m_pitch = Cvar_Get ("m_pitch", "0.022", CVAR_ARCHIVE|CVAR_ORIGINAL);
	m_yaw = Cvar_Get ("m_yaw", "0.022", CVAR_ARCHIVE|CVAR_ORIGINAL);
	m_forward = Cvar_Get ("m_forward", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	m_side = Cvar_Get ("m_side", "0.8", CVAR_ARCHIVE|CVAR_ORIGINAL);
// 2001-12-16 M_LOOK cvar by Heffo/Maddes  start
	m_look = Cvar_Get ("m_look", "0", CVAR_ARCHIVE);
	Cvar_SetRangecheck (m_look, Cvar_RangecheckBool, 0, 1);
	Cvar_SetCallback (m_look, Callback_M_Look);
	Cvar_Set(m_look, m_look->string);	// do rangecheck
// 2001-12-16 M_LOOK cvar by Heffo/Maddes  end

// 2001-11-31 FPS display by QuakeForge/Muff  start
	cl_showfps = Cvar_Get ("cl_showfps", "0", CVAR_NONE);
	Cvar_SetRangecheck (cl_showfps, Cvar_RangecheckBool, 0, 1);
	Cvar_Set(cl_showfps, cl_showfps->string);	// do rangecheck
// 2001-11-31 FPS display by QuakeForge/Muff  end

// 2001-12-24 Keeping full backwards compatibility by Maddes  start
	cl_compatibility = Cvar_Get ("cl_compatibility", "0", CVAR_ARCHIVE);
	Cvar_SetRangecheck (cl_compatibility, Cvar_RangecheckBool, 0, 1);
	Cvar_SetDescription (cl_compatibility, "When set to 1, this client will not request enhanced information from the server (server's entity limit, NVS handshake, etc.) and disables enhanced client messages (precise client aiming, etc.). This is necessary for recording demos that shall run on all Quake executables. Also see SV_COMPATIBILITY.");
	Cvar_Set(cl_compatibility, cl_compatibility->string);	// do rangecheck
// 2001-12-24 Keeping full backwards compatibility by Maddes  end

// 2001-09-20 Configurable entity limits by Maddes  start
	cl_entities_min = Cvar_Get ("cl_entities_min", "0", CVAR_NONE);
	Cvar_SetRangecheck (cl_entities_min, Cvar_RangecheckInt, MIN_EDICTS, MAX_EDICTS);
	Cvar_Set(cl_entities_min, cl_entities_min->string);	// do rangecheck

	cl_entities_min_static = Cvar_Get ("cl_entities_min_static", "0", CVAR_NONE);
	Cvar_SetRangecheck (cl_entities_min_static, Cvar_RangecheckInt, MIN_STATIC_ENTITIES, MAX_EDICTS);
	Cvar_Set(cl_entities_min_static, cl_entities_min_static->string);	// do rangecheck

	cl_entities_min_temp = Cvar_Get ("cl_entities_min_temp", "0", CVAR_NONE);
	Cvar_SetRangecheck (cl_entities_min_temp, Cvar_RangecheckInt, MIN_TEMP_ENTITIES, MAX_EDICTS);
	Cvar_Set(cl_entities_min_temp, cl_entities_min_temp->string);	// do rangecheck
// 2001-09-20 Configurable entity limits by Maddes  end


	cl_hudswap = Cvar_Get ("cl_hudswap", "0", CVAR_ARCHIVE | CVAR_ORIGINAL);
	cl_sbar = Cvar_Get ("cl_sbar", "1", CVAR_ARCHIVE | CVAR_ORIGINAL);
}
// 2001-09-18 New cvar system by Maddes (Init)  end
void TheForceLoadLighting (void)
{
	LoadPointLighting(sv.worldmodel->entities);
}

/*
=================
CL_Init
=================
*/
void CL_Init (void)
{
	SZ_Alloc (&cls.message, 1024);

	CL_InitInput ();
	CL_InitTEnts ();

//
// register our commands
//
// 2001-09-18 New cvar system by Maddes (Init)  start
// 2001-09-18 New cvar system by Maddes (Init)  end

//	Cvar_RegisterVariable (&cl_autofire);

	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
	Cmd_AddCommand ("engoo_forceloadlighting", TheForceLoadLighting);
}
