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
// screen.c -- master for refresh, status bar, console, chat, notify, etc

#include "quakedef.h"
#include "r_local.h"

// only the refresh window will be updated unless these variables are flagged
int			scr_copytop;
int			scr_copyeverything;
int			reflectavailable;
float		scr_con_current;
float		scr_conlines;		// lines of console to display
float		oldsbar;
float		oldscreensize, oldfov;
cvar_t	*scr_viewsize;
cvar_t	*scr_fov;	// 10 - 170
cvar_t	*scr_fovadapt;// "Hor+" scaling
cvar_t	*scr_conspeed;
cvar_t	*scr_centertime;
cvar_t	*scr_showram;
cvar_t	*scr_showturtle;
cvar_t	*scr_showpause;
cvar_t	*scr_printspeed;
cvar_t	*scr_conheight;	// 2000-01-12 Variable console height by Fett
cvar_t  *scr_scale;		// leilei
cvar_t  *scr_retroscale;		// leilei
cvar_t  *scr_aspectmode;		// leilei
qboolean	scr_initialized;		// ready to draw

qpic_t		*scr_ram;
qpic_t		*scr_net;
qpic_t		*scr_turtle;
int			sb_what_lines;	// leilei - refdef sbar scale hack
int			scr_fullupdate;

extern int	vidmodetweak;	// leilei - 0: normal resolution
							//          1: 16:10= 4:3	(320x200, 640x400)
							//			2: 2:3	= 4:3	(320x480 Tweaked)
							//			3: 8:10 = 4:3	(320x400 Tweaked)
int			clearconsole;
int			clearnotify;
int			retroscalefactor;	// leilei
viddef_t	vid;				// global video state

vrect_t		*pconupdate;
vrect_t		scr_vrect;

qboolean	scr_disabled_for_loading;
qboolean	scr_drawloading;
float		scr_disabled_time;
qboolean	scr_skipupdate;

qboolean	block_drawing;

void SCR_ScreenShot_f (void);

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int			scr_center_lines;
int			scr_erase_lines;
int			scr_erase_center;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (char *str)
{
	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime->value;
	scr_centertime_start = cl.time;

// count the number of lines for centering
	scr_center_lines = 1;
	while (*str)
	{
		if (*str == '\n')
			scr_center_lines++;
		str++;
	}
}

void SCR_EraseCenterString (void)
{
	int		y;

	if (scr_erase_center++ > vid.numpages)
	{
		scr_erase_lines = 0;
		return;
	}

	if (scr_center_lines <= 4)
		y = vid.height*0.35;
	else
		y = 48;

	scr_copytop = 1;
	Draw_TileClear (0, y,vid.width, 8*scr_erase_lines);
}

void SCR_DrawCenterString (void)
{
	char	*start;
	int		l;
	int		j;
	int		x, y;
	int		remaining;

// the finale prints the characters one at a time
	if (cl.intermission)
		remaining = scr_printspeed->value * (cl.time - scr_centertime_start);
	else
		remaining = 9999;
#ifdef SCALED2D
		scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = vid.vconheight*0.35;
	else
		y = 48;

	do
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (start[l] == '\n' || !start[l])
				break;
		x = (vid.vconwidth - l*8)/2;
		for (j=0 ; j<l ; j++, x+=8)
		{
			Draw_Character_Scaled (x, y, start[j]);
			if (!remaining--)
				return;
		}

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
#else
	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = vid.height*0.35;
	else
		y = 48;

	do
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (start[l] == '\n' || !start[l])
				break;
		x = (vid.width - l*8)/2;
		for (j=0 ; j<l ; j++, x+=8)
		{
			Draw_Character (x, y, start[j]);
			if (!remaining--)
				return;
		}

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
#endif
}

void SCR_CheckDrawCenterString (void)
{
	scr_copytop = 1;
	if (scr_center_lines > scr_erase_lines)
		scr_erase_lines = scr_center_lines;

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	scr_centertime_off -= host_frametime;
	scr_centertime_off -= host_cpu_frametime;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end

	if (scr_centertime_off <= 0 && !cl.intermission)
		return;
	if (key_dest != key_game)
		return;

	SCR_DrawCenterString ();
}

//=============================================================================

float scalefactor;
float scalefactorv;


// QUAKESPASM CODE BY SEZERO START

/*
====================
AdaptFovx
Adapt a 4:3 horizontal FOV to the current screen size using the "Hor+" scaling:
2.0 * atan(width / height * 3.0 / 4.0 * tan(fov_x / 2.0))
====================
*/
extern cvar_t *temp2;
float AdaptFovx (float fov_x, float width, float height)
{
	float	a, x;
	float ferv;

	if (cl_sbar->value)
	ferv = 0.15 * ((float)sb_lines * 0.02083333); // leilei - fudge around the size of fov
	else
	ferv = 0;

	if (fov_x < 1 || fov_x > 179)
		fov_x = 90; // fallback to 90 if we fail...
	//	Sys_Error ("Bad fov: %f", fov_x);
#ifndef _WIN32
if (vid.aspect > 1.10f)
		return fov_x;			// there's no such thing as a widescreen dos machine	
#endif
//	if (!scr_fov_adapt.value)
//		return fov_x;
	if ((x = height / width) == 0.75)
		return fov_x;
	a = atan((0.75 - ferv)/ x * tan(fov_x / 360 * M_PI));
	a = a * 360 / M_PI;
	return a;
}

/*
====================
CalcFovy
====================
*/
float CalcFovy (float fov_x, float width, float height)
{
	float	a, x;

	if (fov_x < 1 || fov_x > 179)
		fov_x = 90; // fallback to 90 if we fail...
	//	Sys_Error ("Bad fov: %f", fov_x);

	x = width / tan(fov_x / 360 * M_PI);
	a = atan(height / x);
	a = a * 360 / M_PI;
	return a;
}

// QUAKESPASM CODE BY SEZERO END

// leilei - i am a dirty bitch for not figuring this out on my own .-_-.


/*
====================
CalcFov
====================
*/
float CalcFov (float fov_x, float width, float height)
{
	/*
	float	a;
	float	x;

	if (fov_x < 1 || fov_x > 179){
		
		fov_x = 179;		// leilei - don't crash.
	}
	//	Sys_Error ("Bad fov: %f", fov_x);

	x = width/tan(fov_x/360*M_PI);

	a = atan (height/x);

	a = a*360/M_PI;

	return a;
	*/
		float x;
	x = width / tan(fov_x / 360 * M_PI);
	return atan (height / x) * 360 / M_PI;
}


// MH's correct thing

float SCR_CalcFovX (float fov_y, float width, float height)
{
   // bound, don't crash
   if (fov_y < 1) fov_y = 1;
   if (fov_y > 179) fov_y = 179;

   return (atan (width / (height / tan ((fov_y * M_PI) / 360.0f))) * 360.0f) / M_PI;
}


float SCR_CalcFovY (float fov_x, float width, float height)
{
   // bound, don't crash
   if (fov_x < 1) fov_x = 1;
   if (fov_x > 179) fov_x = 179;

   return (atan (height / (width / tan ((fov_x * M_PI) / 360.0f))) * 360.0f) / M_PI;
}


void SCR_SetFOV (float fovvar, int width, int height)
{
   float aspect = (float) height / (float) width;
   float BASELINE_W, BASELINE_H;



   BASELINE_W = 640.0f;
   BASELINE_H = 480.0f - (24 * scalefactor);	// leilei - new baseline



   // horizontalFov = atan (tan (verticalFov) * aspectratio)
   // verticalFov = atan (tan (horizontalFov) / aspectratio)
   if (aspect > (BASELINE_H / BASELINE_W))
   {
      // use the same calculation as GLQuake did (horizontal is constant, vertical varies)
      r_refdef.fov_x = fovvar;
      r_refdef.fov_y = SCR_CalcFovY (r_refdef.fov_x, width, height);
   }
   else
   {
      // alternate calculation (vertical is constant, horizontal varies)
      r_refdef.fov_y = SCR_CalcFovY (fovvar, BASELINE_W, BASELINE_H);
      r_refdef.fov_x = SCR_CalcFovX (r_refdef.fov_y, width, height);
   }
}

float CalcFovOld (float fov_x, float width, float height)
{
        float   a;
        float   x;

        if (fov_x < 1 || fov_x > 179)
                Sys_Error ("Bad fov: %f", fov_x);

        x = width/tan(fov_x/360*M_PI);

        a = atan (height/x);

        a = a*360/M_PI;

        return a;
}
int yeahimconsoled;
int screenfake;	// leilei - forcing 320x200 metrics in any resolution no matter what (HACK HACK HACK!)

void SCR_StretchInit (void)
{

	int		i;
	float eh;



	if ((i = COM_CheckParm("-conwidth")) != 0){
		vid.vconwidth = Q_atoi(com_argv[i+1]);
		scalefactor = vid.width / vid.vconwidth;
		scalefactorv = vid.height / vid.vconheight;
	//	vid.vconwidth = vid.width / scalefactor;
	//	vid.vconheight = vid.height / scalefactor; // leilei - variable height doesn't work right now.
		yeahimconsoled = 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 1;	// currently bugged
	}
	
	else if ((i = COM_CheckParm("-youpickedupaclip")) != 0){
		vid.vconwidth = 32;
		vid.vconheight = 32;
		scalefactor = vid.width / vid.vconwidth;
		scalefactorv = vid.height / 32;
	//	vid.vconwidth = vid.width / scalefactor;
	//	vid.vconheight = vid.height / scalefactor; // leilei - variable height doesn't work right now.
		yeahimconsoled	= 0;
		sb_scaled		= 1;
		menu_scaled		= 0;
		console_scaled	= 0;	// currently bugged
	}
	else if ((i = COM_CheckParm("-conheight")) != 0){
		vid.vconheight = Q_atoi(com_argv[i+1]);
			scalefactor = vid.width / vid.vconwidth;
		scalefactorv = vid.height / vid.vconheight;
	//	vid.vconwidth = vid.width / scalefactor;
	//	vid.vconheight = vid.height / scalefactor; // leilei - variable height doesn't work right now.
		yeahimconsoled  = 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 1;	// currently bugged
	}
	else{
		scalefactor = 0;	// we don't stretch.
	}

		if (scalefactor > (vid.height / 200))
				scalefactor = vid.height / 200;
	
//	scr_scale->value = scalefactor + scalefactorv / 2; // transfer are cvar's ovar.
	

	/*
		int		i;
	float eh;
	if ((i = COM_CheckParm("-conwidth")) != 0){
		vid.vconwidth = Q_atoi(com_argv[i+1]);
		eh = vid.width / vid.vconwidth;
		vid.vconwidth = vid.width / eh;
		vid.vconheight = vid.height / eh; // leilei - variable height doesn't work right now.
		yeahimconsoled = 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 0;	// currently bugged
	}

	else
		vid.vconwidth = vid.width;
		*/
/*
	if (yeahimconsoled){
	vid.vconwidth &= 0xfff8; // make it a multiple of eight

	// pick a conheight that matches with correct aspect
//	vid.vconheight = vid.vconwidth*3 / 4;

	if ((i = COM_CheckParm("-conheight")) != 0)
	//	vid.vconheight = Q_atoi(com_argv[i+1]);

		vid.vconheight = vid.height / (vid.width / vid.vconwidth); // leilei - variable height doesn't work right now.
	if (vid.vconheight < 200)
		vid.vconheight = 200;
	}
	else
	{
		vid.vconwidth = vid.width;
		vid.vconheight = vid.height;
	}
	*/
	
	
//	if (vid.vconheight > vid.height)
//		vid.vconheight = 200; // clamping it crashes, so fall back to 320x200 anyway

//	if (yeahimconsoled)
//			vid.vconheight = vid.height / (vid.width / vid.vconwidth); // leilei - variable height doesn't work right now.


};

void SCR_CvarCheck (void)
{
		scalefactor = scr_scale->value;
		scalefactorv = scr_scale->value;

		if (scr_retroscale->value){
			
			scalefactorv = vid.height / 200;	// force old metrics etc!!

			scalefactor = scalefactorv;
			retroscalefactor = 1;
		}
		else
			retroscalefactor = 0;
		if (scalefactorv > (vid.height / 200))
				scalefactorv = vid.height / 200;  // if we go beyond 320x200, then clamp!
//		if (scalefactor > (vid.width / 320))
//				scalefactor = vid.width / 320;  // if we go beyond 320x200, then clamp!
		if (scalefactor < 1)
				scalefactor = 1;

		if (scalefactorv < 1)
			scalefactorv = 1;



		SCR_StretchRefresh();

};

void SCR_StretchRefresh (void)
{

	int		i;
	float eh;
	if (!scalefactor)
			return; // don't even try
	if (!scalefactorv)
			return; // don't even try
	screenfake = 0;
	if (screenfake == 1){
		
		vid.vconwidth = 320;
		vid.vconheight = 200;
		scalefactor = vid.width / vid.vconwidth;
		scalefactorv = vid.height / vid.vconheight;
		yeahimconsoled = 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 1;	// currently bugged
			vid.recalc_refdef = 1;		// yeah
		return;

	}

	if (screenfake == 2){
		
		vid.vconwidth = 320;
		vid.vconheight = 400;
		scalefactor = vid.width / vid.vconwidth;
		scalefactorv = vid.height / vid.vconheight;
		yeahimconsoled = 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 1;	// currently bugged
			vid.recalc_refdef = 1;		// yeah
		return;

	}

	if (screenfake == 3){
		
		vid.vconwidth = 360;
		vid.vconheight = 480;
		scalefactor = vid.width / vid.vconwidth;
		scalefactorv = vid.height / vid.vconheight;
		yeahimconsoled = 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 1;	// currently bugged
			vid.recalc_refdef = 1;		// yeah
		return;

	}

	if (screenfake == 4){
		
		vid.vconwidth = 640;
		vid.vconheight = 400;
		scalefactor = vid.width / vid.vconwidth;
		scalefactorv = vid.height / vid.vconheight;
		yeahimconsoled = 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 1;	// currently bugged
			vid.recalc_refdef = 1;		// yeah
		return;

	}
	if (scalefactorv < 1){
		vid.vconheight = vid.height;
		yeahimconsoled	= 0;
		sb_scaled		= 0;
		menu_scaled		= 0;
		console_scaled	= 0;	// currently bugged
	}
	if (scalefactor < 1){
		vid.vconwidth = vid.width;
		yeahimconsoled	= 0;
		sb_scaled		= 0;
		menu_scaled		= 0;
		console_scaled	= 0;	// currently bugged
			return;	// avoid div0 at all costs

	}
//		if (scalefactor > (vid.height / 200))
//				scalefactor = vid.height / 200;
//	if (scalefactor < (vid.vconheight / vid.height))
//		scalefactor = vid.vconheight / vid.height;

	vid.vconwidth = vid.width / scalefactor;
	vid.vconheight = vid.height / scalefactorv; // leilei - variable height doesn't work right now.
		yeahimconsoled	= 1;
		sb_scaled		= 1;
		menu_scaled		= 1;
		console_scaled	= 0;	// currently bugged

	// minimum safeguards so we don't make a size too small to enter
	// the evil land of crash city
	if (vid.vconwidth	< 320)	vid.vconwidth	= 320;
	if (vid.vconheight	< 200)	vid.vconheight	= 200;


	if (vid.vconwidth > vid.width && vid.vconheight > vid.height){
		vid.vconwidth = vid.width;
		vid.vconheight = vid.height;
		yeahimconsoled = 0;
		sb_scaled		= 0;
		menu_scaled		= 0;
		console_scaled	= 0;	
		scalefactor		= 1;
		scalefactorv	= 1;
	}

	vid.recalc_refdef = 1;		// yeah
};


/*
=================
SCR_CalcRefdef

Must be called whenever vid changes
Internal use only
=================
*/

static void SCR_CalcRefdef (void)
{
	vrect_t		vrect;
	float		size;
	float		stretchf = 1;
	
	scr_fullupdate = 0;		// force a background redraw
	vid.recalc_refdef = 0;

	if (vid.stretched == 2)
			stretchf = 2;
	else if (vid.stretched == 3)
			stretchf = 0.5;

	if (vid.width < 800 && vid.aspect > 1.6)
	{
			stretchf *= 0.8;	// try to fix it for 320x200....
	}

// force the status bar to redraw
	Sbar_Changed ();

//========================================

// bound viewsize
	if (scr_viewsize->value < 30)
		Cvar_Set (scr_viewsize, "30");
	if (scr_viewsize->value > 120)
		Cvar_Set (scr_viewsize, "120");

// bound field of view
	if (scr_fov->value < 10)
		Cvar_Set (scr_fov,"10");
//	if (scr_fov->value > 90)
//		Cvar_Set (scr_fov,"90"); // cheaters can eat crap
		if (scr_fov->value > 199)
		Cvar_Set (scr_fov,"199"); // cheaters can eat crap
				// leilei - Removed this because it breaks the 'tear gars' in swatteam

// intermission is always full screen
	if (cl.intermission)
		size = 120;
	else
		size = scr_viewsize->value;
#ifdef SCALED2D
	if(sb_scaled){
	if (size >= 120)
		sb_lines = 0;		// no status bar at all
	else if (size >= 110)
		sb_lines = 24;		// no inventory
	else
		sb_lines = 24+16+8;
	
}	else
#endif
	{
	if (size >= 120)
		sb_lines = 0;		// no status bar at all
	else if (size >= 110)
		sb_lines = 24;		// no inventory
	else
		sb_lines = 24+16+8;
}


#ifdef SCALED2D
	if (sb_scaled)
//	sb_what_lines = sb_lines * (vid.vconheight / vid.height); // leilei - refdef hack fix
	sb_what_lines = sb_lines * (scalefactorv); // leilei - refdef hack fix
	else
#endif
	sb_what_lines = sb_lines;
	
// these calculations mirror those in R_Init() for r_refdef, but take no
// account of water warping
	vrect.x = 0;
	vrect.y = 0;
	vrect.width = vid.width;
	vrect.height = vid.height;

	R_SetVrect (&vrect, &scr_vrect, sb_what_lines);
	r_refdef.vrect = scr_vrect;
	//r_refdef.fov_x = AdaptFovx (scr_fov->value, r_refdef.vrect.width, r_refdef.vrect.height);
	//r_refdef.fov_y = CalcFovy (r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height);
	//r_refdef.fov_y = CalcFovy (r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height);

		// quakespasm
	if (scr_aspectmode->value == 1){
	r_refdef.fov_x = AdaptFovx (scr_fov->value, r_refdef.vrect.width * stretchf, r_refdef.vrect.height);
	r_refdef.fov_y = CalcFovy (r_refdef.fov_x, r_refdef.vrect.width	 * stretchf, r_refdef.vrect.height);
	}
		// mh's "Quakespasm is wrong" post, a bit re-adapted
	else if (scr_aspectmode->value == 2){
	SCR_SetFOV(scr_fov->value, r_refdef.vrect.width * stretchf, r_refdef.vrect.height);
	}
	else
		// crappy old fov where we lose vert. just like Q3A!
	{
	r_refdef.fov_x = scr_fov->value;
	r_refdef.fov_y = CalcFovOld (r_refdef.fov_x, r_refdef.vrect.width * stretchf, r_refdef.vrect.height);


	}
// guard against going from one mode to another that's less than half the
// vertical resolution
	if (scr_con_current > vid.height)
		scr_con_current = vid.height;

// notify the refresh of the change
	R_ViewChanged (&vrect, sb_what_lines, vid.aspect);
//	R_ViewChanged (vid.aspect);
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void)
{
	Cvar_SetValue (scr_viewsize, scr_viewsize->value+10);
	vid.recalc_refdef = 1;
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void)
{
	Cvar_SetValue (scr_viewsize, scr_viewsize->value-10);
	vid.recalc_refdef = 1;
}

//============================================================================

extern cvar_t *cl_bobmodel;
extern cvar_t *r_shading;
extern cvar_t *s_oldspatial;

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
==================
SCR_Init_Cvars
==================
*/
cvar_t *scr_fov_adapt;
void SCR_Init_Cvars (void)
{
 	scr_fov = Cvar_Get ("fov", "90", CVAR_ORIGINAL);
	scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE|CVAR_ORIGINAL);
	scr_conspeed = Cvar_Get ("scr_conspeed", "300", CVAR_ORIGINAL);
	scr_showram = Cvar_Get ("showram", "1", CVAR_ORIGINAL);
	scr_showturtle = Cvar_Get ("showturtle", "0", CVAR_ORIGINAL);
	scr_showpause = Cvar_Get ("showpause", "1", CVAR_ORIGINAL);
	scr_centertime = Cvar_Get ("scr_centertime", "2", CVAR_ORIGINAL);
	scr_printspeed = Cvar_Get ("scr_printspeed", "8", CVAR_ORIGINAL);
	scr_fov_adapt = Cvar_Get ("scr_fov_adapt", "1", CVAR_ARCHIVE | CVAR_ORIGINAL);
// 2000-01-12 Variable console height by Fett  start
	scr_conheight = Cvar_Get ("scr_conheight", "0.5", CVAR_ARCHIVE);
	Cvar_SetRangecheck (scr_conheight, Cvar_RangecheckFloat, 0, 1);
	Cvar_Set(scr_conheight, scr_conheight->string);	// do rangecheck
// 2000-01-12 Variable console height by Fett  end

	scr_scale = Cvar_Get ("scr_scale", "2", CVAR_ARCHIVE | CVAR_ORIGINAL);	// leilei - default to 2
	scr_retroscale = Cvar_Get ("scr_retroscale", "1", CVAR_ARCHIVE | CVAR_ORIGINAL);	// leilei - default to 1 since it's fast now.....
	scr_aspectmode = Cvar_Get ("scr_aspectmode", "1", CVAR_ARCHIVE | CVAR_ORIGINAL);
}
// 2001-09-18 New cvar system by Maddes (Init)  end

/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{

// 2001-09-18 New cvar system by Maddes (Init)  end

//
// register our commands
//
	Cmd_AddCommand ("screenshot",SCR_ScreenShot_f);
	Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",SCR_SizeDown_f);


	scr_ram = Draw_PicFromWad ("ram");
	scr_net = Draw_PicFromWad ("net");
	scr_turtle = Draw_PicFromWad ("turtle");

	scr_initialized = true;
}



/*
==============
SCR_DrawRam
==============
*/
void SCR_DrawRam (void)
{
	if (!scr_showram->value)
		return;

	if (!r_cache_thrash)
		return;
if (menu_scaled)
	Draw_Pic_Scaled (scr_vrect.x+32, scr_vrect.y, scr_ram);
else
	Draw_Pic (scr_vrect.x+32, scr_vrect.y, scr_ram);
}

/*
==============
SCR_DrawTurtle
==============
*/
void SCR_DrawTurtle (void)
{
	static int	count;

	if (!scr_showturtle->value)
		return;

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	if (host_frametime < 0.1)
	if (host_cpu_frametime < 0.1)
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
	{
		count = 0;
		return;
	}

	count++;
	if (count < 3)
		return;
if (menu_scaled)
	Draw_Pic_Scaled (scr_vrect.x, scr_vrect.y, scr_turtle);
else
	Draw_Pic (scr_vrect.x, scr_vrect.y, scr_turtle);
}

/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void)
{
	if (realtime - cl.last_received_message < 0.3)
		return;
	if (cls.demoplayback)
		return;
if (menu_scaled)
	Draw_Pic_Scaled (scr_vrect.x+64, scr_vrect.y, scr_net);
else
	Draw_Pic (scr_vrect.x+64, scr_vrect.y, scr_net);
}

// 2001-11-31 FPS display by QuakeForge/Muff  start
/*
==============
SCR_DrawFPS
==============
*/
//muff - hacked out of SourceForge implementation + modified
void SCR_DrawFPS (void)
{
	static double lastframetime;
	double t;
	static int lastfps;
	int x, y;
	char st[80];

	if (!cl_showfps->value)
		return;

	t = Sys_FloatTime ();
	if ((t - lastframetime) >= 1.0) {
		lastfps = fps_count;
		fps_count = 0;
		lastframetime = t;
	}

	sprintf(st, "%3d FPS", lastfps);
#ifdef SCALED2D
	if (sb_scaled)
	x = vid.vconwidth - strlen(st) * 16 - 16;
	else
#endif
	x = vid.width - strlen(st) * 16 - 16;
	y = 0 ; 
#ifdef SCALED2D
	if (sb_scaled)
	Draw_String_Scaled(x, y, st);
	else
#endif
	Draw_String(x, y, st);

}
// 2001-11-31 FPS display by QuakeForge/Muff  end

/*
==============
DrawPause
==============
*/
void SCR_DrawPause (void)
{
	qpic_t	*pic;

	if (!scr_showpause->value)		// turn off for screenshots
		return;

	if (!cl.paused)
		return;
	
	pic = Draw_CachePic ("gfx/pause.lmp");
if (menu_scaled)
	Draw_Pic_Scaled ( (vid.vconwidth - pic->width)/2,
		(vid.vconheight - 48 - pic->height)/2, pic);
else
	Draw_Pic ( (vid.width - pic->width)/2,
		(vid.height - 48 - pic->height)/2, pic);
}



/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
	qpic_t	*pic;

	if (!scr_drawloading)
		return;

	pic = Draw_CachePic ("gfx/loading.lmp");
if (menu_scaled)
	Draw_Pic_Scaled ( (vid.vconwidth - pic->width)/2,
		(vid.vconheight - 48 - pic->height)/2, pic);
else
	Draw_Pic ( (vid.width - pic->width)/2,
		(vid.height - 48 - pic->height)/2, pic);
}



//=============================================================================


/*
==================
SCR_SetUpToDrawConsole
==================
*/
extern cvar_t	*con_alpha;	// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes

void SCR_SetUpToDrawConsole (void)
{
	Con_CheckResize ();

	if (scr_drawloading)
		return;		// never a console with loading plaque

// decide on the height of the console
	con_forcedup = !cl.worldmodel || cls.signon != SIGNONS;

	if (con_forcedup)
	{
		scr_conlines = vid.height;		// full screen
		scr_con_current = scr_conlines;
	}
	else if (key_dest == key_console)
// 2000-01-12 Variable console height by Fett/Maddes  start
	{
		scr_conlines = vid.height/2;	// half screen
#ifdef SCALED2D
		if (console_scaled)
			//		scr_conlines = vid.vconheight/2;	// half screen
			scr_conlines = vid.vconheight*scr_conheight->value;	// in-game console
			else
#endif
			scr_conlines = vid.height*scr_conheight->value;	// in-game console

		if (scr_conlines < (3*8+8+8))		// always leave three lines visible (plus command line and border)
		{
			scr_conlines = (3*8+8+8);
		}
		if (scr_conlines >= vid.height)
		{
#ifdef SCALED2D
if (console_scaled)
			scr_conlines = vid.vconheight - 1;	// maximum is full screen
		else
#endif
			scr_conlines = vid.height - 1;	// maximum is full screen
		}
	}
// 2000-01-12 Variable console height by Fett/Maddes  end
	else
		scr_conlines = 0;				// none visible

	if (scr_conlines < scr_con_current)
	{
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//		scr_con_current -= scr_conspeed->value*host_frametime;
		if (console_scaled)
		scr_con_current -= scr_conspeed->value*host_cpu_frametime * vid.height / vid.vconheight;
		else
		scr_con_current -= scr_conspeed->value*host_cpu_frametime;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if (scr_conlines > scr_con_current)
	{
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//		scr_con_current += scr_conspeed->value*host_frametime;
		if (console_scaled)
		scr_con_current += scr_conspeed->value*host_cpu_frametime  * vid.height / vid.vconheight;
		else
		scr_con_current += scr_conspeed->value*host_cpu_frametime;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

	if (clearconsole++ < vid.numpages)
	{
		scr_copytop = 1;
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  start
		if (con_alpha->value == 1.0)
		{
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  end
		Draw_TileClear (0,(int)scr_con_current,vid.width, vid.height - (int)scr_con_current);
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  start
		}
		else
		{
			// Draw full valid screen
			Draw_TileClear (0, 0, vid.width, vid.height );
		}
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  end
		Sbar_Changed ();
	}
	else if (clearnotify++ < vid.numpages)
	{
		scr_copytop = 1;
		Draw_TileClear (0,0,vid.width, con_notifylines);
	}
	else
		con_notifylines = 0;
}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void)
{
	if (scr_con_current)
	{
		scr_copyeverything = 1;
		Con_DrawConsole (scr_con_current, true);
		clearconsole = 0;
	}
	else
	{
		if (key_dest == key_game || key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}


/*
==============================================================================

						SCREEN SHOTS

==============================================================================
*/


typedef struct
{
    char	manufacturer;
    char	version;
    char	encoding;
    char	bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hres,vres;
    unsigned char	palette[48];
    char	reserved;
    char	color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char	filler[58];
    unsigned char	data;			// unbounded
} pcx_t;

/*
==============
WritePCXfile
==============
*/
void WritePCXfile (char *filename, byte *data, int width, int height,
	int rowbytes, byte *palette)
{
	int		i, j, length;
	pcx_t	*pcx;
	byte		*pack;

	pcx = Hunk_TempAlloc (width*height*2+1000);
	if (pcx == NULL)
	{
		Con_Printf("SCR_ScreenShot_f: not enough memory\n");
		return;
	}

	pcx->manufacturer = 0x0a;	// PCX id
	pcx->version = 5;			// 256 color
 	pcx->encoding = 1;		// uncompressed
	pcx->bits_per_pixel = 8;		// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleShort((short)(width-1));
	pcx->ymax = LittleShort((short)(height-1));
	pcx->hres = LittleShort((short)width);
	pcx->vres = LittleShort((short)height);
	Q_memset (pcx->palette,0,sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleShort((short)width);
	pcx->palette_type = LittleShort(2);		// not a grey scale
	Q_memset (pcx->filler,0,sizeof(pcx->filler));

// pack the image
	pack = &pcx->data;

	for (i=0 ; i<height ; i++)
	{
		for (j=0 ; j<width ; j++)
		{
			if ( (*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}

		data += rowbytes - width;
	}

// write the palette
	*pack++ = 0x0c;	// palette ID byte
	for (i=0 ; i<768 ; i++)
		*pack++ = *palette++;

// write output file
	length = pack - (byte *)pcx;
	COM_WriteFile (filename, pcx, length);
}
#ifdef _WIN32
extern unsigned char	vid_curpal[256*3];
#endif
/*
==================
SCR_ScreenShot_f
==================
*/
void SCR_ScreenShot_f (void)
{
	int		i;
	char	pcxname[80];
	char	pcxnametwo[80];
	char	checkname[MAX_OSPATH];

//
// find a file name to save it to
//
	strcpy(pcxname,"quake00.pcx");

	for (i=0 ; i<=99 ; i++)
	{
		pcxname[5] = i/10 + '0';
		pcxname[6] = i%10 + '0';
		sprintf (checkname, "%s/%s", com_gamedir, pcxname);
		if (Sys_FileTime(checkname) == -1)
			break;	// file doesn't exist
	}
	if (i==100)
	{
		Con_Printf ("SCR_ScreenShot_f: Couldn't create a PCX file\n");
		return;
 	}

//
// save the pcx file
//
	D_EnableBackBufferAccess ();	// enable direct drawing of console to back
									//  buffer


#ifdef _WIN32
	WritePCXfile (pcxname, vid.buffer, vid.width, vid.height, vid.rowbytes,
				  vid_curpal);
	
#else
	WritePCXfile (pcxname, vid.buffer, vid.width, vid.height, vid.rowbytes,
				  host_basepal);
#endif

	D_DisableBackBufferAccess ();	// for adapters that can't stay mapped in
									//  for linear writes all the time

	Con_Printf ("Wrote %s\n", pcxname);
}


//=============================================================================


/*
===============
SCR_BeginLoadingPlaque

================
*/
void SCR_BeginLoadingPlaque (void)
{
	S_StopAllSounds (true);

	if (cls.state != ca_connected)
		return;
	if (cls.signon != SIGNONS)
		return;

// redraw with no console and the loading plaque
	Con_ClearNotify ();
	scr_centertime_off = 0;
	scr_con_current = 0;

	scr_drawloading = true;
	scr_fullupdate = 0;
	Sbar_Changed ();
	SCR_UpdateScreen ();
	scr_drawloading = false;

	scr_disabled_for_loading = true;
	scr_disabled_time = realtime;
	scr_fullupdate = 0;
}

/*
===============
SCR_EndLoadingPlaque

================
*/
void SCR_EndLoadingPlaque (void)
{
	scr_disabled_for_loading = false;
	scr_fullupdate = 0;
	Con_ClearNotify ();
}

//=============================================================================

char	*scr_notifystring;
qboolean	scr_drawdialog;

void SCR_DrawNotifyString (void)
{
	char	*start;
	int		l;
	int		j;
	int		x, y;

	start = scr_notifystring;
#ifdef SCALED2D
	if (menu_scaled)
	y = vid.vconheight*0.35;


	else
#endif
	y = vid.height*0.35;

	do
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (start[l] == '\n' || !start[l])
				break;
		
#ifdef SCALED2D
		if (menu_scaled){		// leilei - we consider this part of the menu isntead
			x = (vid.vconwidth - l*8)/2;
		for (j=0 ; j<l ; j++, x+=8)
			Draw_Character_Scaled (x, y, start[j]);
		}
		else
#endif
			
		{
			x = (vid.width - l*8)/2;
		for (j=0 ; j<l ; j++, x+=8)
			Draw_Character (x, y, start[j]);
		}

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

/*
==================
SCR_ModalMessage

Displays a text string in the center of the screen and waits for a Y or N
keypress.
==================
*/
int SCR_ModalMessage (char *text)
{
	if (cls.state == ca_dedicated)
		return true;

	scr_notifystring = text;

// draw a fresh screen
	scr_fullupdate = 0;
	scr_drawdialog = true;
	SCR_UpdateScreen ();
	scr_drawdialog = false;

	S_ClearBuffer ();		// so dma doesn't loop current sound

	do
	{
		key_count = -1;		// wait for a key down and up
		Sys_SendKeyEvents ();
	} while (key_lastpress != 'y' && key_lastpress != 'n' && key_lastpress != K_ESCAPE);

	scr_fullupdate = 0;
	SCR_UpdateScreen ();

	return key_lastpress == 'y';
}


//=============================================================================

/*
===============
SCR_BringDownConsole

Brings the console down and fades the palettes back to normal
================
*/
void SCR_BringDownConsole (void)
{
	int		i;

	scr_centertime_off = 0;

	for (i=0 ; i<20 && scr_conlines != scr_con_current ; i++)
		SCR_UpdateScreen ();

	cl.cshifts[0].percent = 0;		// no area contents palette on next frame
	VID_SetPalette (host_basepal);
}

extern int nolookups;
extern float oldwateralpha;
extern float newwateralpha;
extern cvar_t *r_wateralpha;
extern cvar_t *r_waterblend;
extern cvar_t *r_shadowhack;
extern cvar_t *r_dynamic;
extern cvar_t *r_tranquality;
int oldwaterblend;
int oldtranquality;
int dyncolor;
extern unsigned int shadowhack;
void R_ApplyFog (void);
void R_ApplyDof (void);


/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.

WARNING: be very careful calling this from elsewhere, because the refresh
needs almost the entire 256k of stack space!
==================
*/
extern cvar_t *r_fogquality;
extern cvar_t *r_waterquality;
int	waterinsight;
extern client_static_t	clssplit;
extern client_state_t	clsplit;


void SCR_ReallyRender (void)
{
#ifdef WATERREFLECTIONS
	if(waterinsight && reflectavailable && r_waterquality->value > 1 && !r_dowarp && !r_docrap){
	reflectpass = 1;



	V_RenderView ();
	reflectpass = 0;
	
	V_RenderView ();
	}
	else
	{
	
	reflectpass = 0;
	V_RenderView ();
	}
#else
		reflectpass = 0;
	V_RenderView ();
#endif
};
int		dontevendraw;
void SCR_UpdateScreen (void)
{
	static float	oldscr_viewsize;
	static float	oldlcd_x;
	static float	oldv_detail;
	vrect_t		vrect;
	
	if (scr_skipupdate || block_drawing)
		return;
	
	scr_copytop = 0;
	scr_copyeverything = 0;

	if (fogenabled && r_fogquality->value){
		foguse2 = 0;foguse = 2;}
	else if (fogenabled && !r_fogquality->value){
		foguse = 1;foguse2 = 0;}
	else if (!fogenabled){
		foguse = 0; foguse2 = 0;}

	if (scr_disabled_for_loading)
	{
		if (realtime - scr_disabled_time > 60)
		{
			scr_disabled_for_loading = false;
			Con_Printf ("load failed.\n");
		}
		else
			return;
	}

	if (cls.state == ca_dedicated)
		return;				// stdout only

	if (!scr_initialized || !con_initialized)
		return;				// not initialized yet

	if (scr_viewsize->value != oldscr_viewsize)
	{
		oldscr_viewsize = scr_viewsize->value;
		vid.recalc_refdef = 1;
	}

//
// check for vid changes
//
	if (oldfov != scr_fov->value)
	{
		oldfov = scr_fov->value;
		vid.recalc_refdef = true;
	}

	if (oldlcd_x != lcd_x->value)
	{
		oldlcd_x = lcd_x->value;
		vid.recalc_refdef = true;
	}

		if (oldv_detail != v_detail->value)
	{
		oldv_detail = v_detail->value;
		vid.recalc_refdef = true;
	}
	if (oldscreensize != scr_viewsize->value)
	{
		oldscreensize = scr_viewsize->value;
		vid.recalc_refdef = true;
	}

		if (oldsbar != cl_sbar->value)
	{
		oldsbar = cl_sbar->value;
		vid.recalc_refdef = true;
	}
	if (!reflectpass){
	if (vid.recalc_refdef)
	{
	// something changed, so reorder the screen
		SCR_CalcRefdef ();
	}

//
// do 3D refresh drawing, and then update the screen
//
	D_EnableBackBufferAccess ();	// of all overlay stuff if drawing directly
	
	if (scr_fullupdate++ < vid.numpages)
	{	// clear the entire screen
		scr_copyeverything = 1;
		Draw_TileClear (0,0,vid.width,vid.height);
		Sbar_Changed ();
	}

	pconupdate = NULL;
}
	
	SCR_SetUpToDrawConsole ();
	SCR_EraseCenterString ();

	D_DisableBackBufferAccess ();	// for adapters that can't stay mapped in
														//  for linear writes all the time

//	Draw_FadeScreen ();

	VID_LockBuffer ();
//	Draw_GetShroomedPT1();	

	
	// leilei - water reflections need two passes
	// we all start it here!
	// TODO- mirror chain	
	splitpass = 0;
	if (!dontevendraw)
	SCR_ReallyRender();
	if(splitmeup){
		client_t	oldcle;
		client_state_t oldcl;
		//*oldcl = cl;
		oldcl = cl;
		//cl = clsplit;
		splitpass = 1;
		if (!dontevendraw)
		SCR_ReallyRender();
	}
	VID_UnlockBuffer ();

	D_EnableBackBufferAccess ();	// of all overlay stuff if drawing directly


	// stupid place to put this
	//if(oldwateralpha != newwateralpha){
	//	oldwateralpha = newwateralpha = r_wateralpha->value;
	//	if(newwateralpha < 1)	WaterTableGet();
	//}

	
	// leilei - make a new alpha tables.
	if (!nolookups){
	if(oldwateralpha != r_wateralpha->value || (oldtranquality != (int)r_tranquality->value) || (oldwaterblend != (int)r_waterblend->value)){
		oldtranquality = r_tranquality->value;
		oldwateralpha = r_wateralpha->value;
		oldwaterblend = r_waterblend->value;
		WaterTableGet();
	}
	
	}

	if(retroscalefactor != scr_retroscale->value){
		
		SCR_CvarCheck();
		
	}
	

	else if(scalefactor != scr_scale->value && !scr_retroscale->value){
		
		SCR_CvarCheck();
		
	}
	


	if (dyncolor != r_coloreddyns->value){

		dyncolor = r_coloreddyns->value;
	}

	if (shadowhack != r_shadowhack->value){

		shadowhack = r_shadowhack->value;
	}
	if (r_dynamic->value)
		dynlightenabled = 1;
	else
		dynlightenabled = 0;
//	Draw_GetShroomedPT2();			
	if (r_tinge->value == 3)
	Draw_DudeScreen ();
	else if (r_tinge->value == 2)
		Draw_DudeScreen66 ();
	else if (r_tinge->value == 1)
		Draw_DudeScreen33 ();
	else if (r_tinge->value == -1)
		Draw_DudeScreen33A ();
	else if (r_tinge->value == -2)
		Draw_DudeScreen66A ();
	
		//Draw_Bloom ();
	/*if (cl.paused || scr_drawloading || scr_drawdialog){
			dontevendraw = 1;
			Draw_Something ();
			
				Sbar_Draw ();
				
	}
	else
		dontevendraw = 0;
		*/

	if (scr_drawdialog)
	{
		Sbar_Draw ();
		Draw_FadeScreen ();
		SCR_DrawNotifyString ();
		scr_copyeverything = true;
	}

	else if (scr_drawloading)
	{
		//Draw_FadeScreen ();	// this WAS cool, but... i'll have to make a whole transition thing first
		SCR_DrawLoading ();
		Sbar_Draw ();
	}

	else if (cl.intermission == 1 && key_dest == key_game)
	{
		Sbar_IntermissionOverlay ();
	}
	else if (cl.intermission == 2 && key_dest == key_game)
	{
		Sbar_FinaleOverlay ();
		SCR_CheckDrawCenterString ();
	}
	else if (cl.intermission == 3 && key_dest == key_game)
	{
		SCR_CheckDrawCenterString ();
	}
	else
	{
		SCR_DrawRam ();
		SCR_DrawNet ();
		SCR_DrawTurtle ();
		SCR_DrawFPS ();	// 2001-11-31 FPS display by QuakeForge/Muff
		SCR_DrawPause ();
		SCR_CheckDrawCenterString ();
		Sbar_Draw ();
		SCR_DrawConsole ();
    	
		M_Draw ();
	}

	D_DisableBackBufferAccess ();	// for adapters that can't stay mapped in
									//  for linear writes all the time
	if (pconupdate)
	{
		D_UpdateRects (pconupdate);
	}

	V_UpdatePalette ();

//
// update one of three areas
//
#ifdef SCALED2D
	if (sb_scaled)
	sb_what_lines = sb_lines * (vid.height / vid.vconheight); // leilei - refdef hack fix
	else
#endif
//		if (!reflectpass)
		sb_what_lines = sb_lines;
//		else
//		sb_what_lines = 0;
	if (scr_copyeverything)
	{
		vrect.x = 0;
		vrect.y = 0;
		vrect.width = vid.width;
		vrect.height = vid.height;
		vrect.pnext = 0;

		VID_Update (&vrect);
	}
	else if (scr_copytop)
	{
		vrect.x = 0;
		vrect.y = 0;
		vrect.width = vid.width;
		vrect.height = vid.height - sb_lines;
		vrect.pnext = 0;

		VID_Update (&vrect);
	}
	else
	{
		vrect.x = scr_vrect.x;
		vrect.y = scr_vrect.y;
		vrect.width = scr_vrect.width;
		vrect.height = scr_vrect.height;
		vrect.pnext = 0;

		VID_Update (&vrect);
	}
	
}


/*
==================
SCR_UpdateWholeScreen
==================
*/
void SCR_UpdateWholeScreen (void)
{
	scr_fullupdate = 0;
	SCR_UpdateScreen ();
}
