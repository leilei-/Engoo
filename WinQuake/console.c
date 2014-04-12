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
// console.c

#ifdef NeXT
#include <libc.h>
#endif
#ifndef _MSC_VER
// 2001-12-10 Compilable with LCC-Win32 by Jeff Ford  start
#ifdef __LCC__
#include <io.h>
#else
// 2001-12-10 Compilable with LCC-Win32 by Jeff Ford  end
#include <unistd.h>
#endif	// 2001-12-10 Compilable with LCC-Win32 by Jeff Ford
#endif
#include <fcntl.h>
#include "quakedef.h"
extern int		console_scaled;	
extern int		sb_scaled;	
int 		con_linewidth;
extern int lilchar;
float		con_cursorspeed = 4;

// 2000-01-05 Console scrolling fix by Maddes  start
//#define		CON_TEXTSIZE	16384
#define		CON_TEXTSIZE	64*1024
// 2000-01-05 Console scrolling fix by Maddes  end

qboolean 	con_forcedup;		// because no entities to refresh

int			con_totallines;		// total lines in console scrollback
int			con_backscroll;		// lines up from bottom to display
int			con_current;		// where next message will be printed
int			con_x;				// offset in current line for next print
char		*con_text=0;

cvar_t	*con_notifytime;		//seconds
cvar_t	*con_alpha;				// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes
								// 2000-01-11 Transparent console by Radix

#define	NUM_CON_TIMES 4
float		con_times[NUM_CON_TIMES];	// realtime time the line was generated
								// for transparent notify lines

int			con_vislines;

qboolean	con_debuglog;

#define		MAXCMDLINE	256
extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;
extern	int		key_insert;	// 2000-01-05 Console typing enhancement by Radix
					// insert key toggle


qboolean	con_initialized;

int			con_notifylines;		// scan lines to clear for notify lines

extern void M_Menu_Main_f (void);

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	if (key_dest == key_console)
	{
		if (cls.state == ca_connected)
		{
			key_dest = key_game;
			key_lines[edit_line][1] = 0;	// clear any typing
			key_linepos = 1;
		}
		else
		{
			M_Menu_Main_f ();
		}
			VID_HandlePause (false);
	}
	else{
		key_dest = key_console;
			VID_HandlePause (true);
	}

	SCR_EndLoadingPlaque ();
	memset (con_times, 0, sizeof(con_times));
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	if (con_text)
		Q_memset (con_text, ' ', CON_TEXTSIZE);
	con_current = 0;	// 2000-01-05 Console scrolling fix by Maddes
}


/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;

	for (i=0 ; i<NUM_CON_TIMES ; i++)
		con_times[i] = 0;
}


/*
================
Con_MessageMode_f
================
*/
extern qboolean team_message;

void Con_MessageMode_f (void)
{
	key_dest = key_message;
	team_message = false;
}


/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	key_dest = key_message;
	team_message = true;
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
extern int menu_scaled;
extern int console_scaled;
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE];

if(console_scaled)
	width = (vid.vconwidth >> 3) - 2;
else
	width = (vid.width >> 3) - 2;

	if (width == con_linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 38;
		con_linewidth = width;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		Q_memset (con_text, ' ', CON_TEXTSIZE);
		con_current = 0;	// 2000-01-05 Console scrolling fix by Maddes
	}
	else
	{
		oldwidth = con_linewidth;
		oldtotallines = con_totallines;

		con_linewidth = width;
		con_totallines = CON_TEXTSIZE / con_linewidth;

// 2000-01-05 Console scrolling fix by Maddes  start
//		numlines = oldtotallines;
		numlines = (oldtotallines < con_current) ? oldtotallines : con_current;	// available lines in old buffer
// 2000-01-05 Console scrolling fix by Maddes  end
		if (numlines > con_totallines)	// 2000-01-05 Console scrolling fix by Maddes
			numlines = con_totallines;

		numchars = oldwidth;
		if (numchars > con_linewidth)	// 2000-01-05 Console scrolling fix by Maddes
			numchars = con_linewidth;

		Q_memcpy (tbuf, con_text, CON_TEXTSIZE);
		Q_memset (con_text, ' ', CON_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
// 2000-01-05 Console scrolling fix by Maddes  start
/*
				con_text[(con_totallines - 1 - i) * con_linewidth + j] =
						tbuf[((con_current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
*/
				con_text[i*con_linewidth + j] =
					tbuf[((con_current-numlines+i) % oldtotallines) * oldwidth + j];
// 2000-01-05 Console scrolling fix by Maddes  end
			}
		}
		con_current = numlines;	// 2000-01-05 Console scrolling fix by Maddes

		Con_ClearNotify ();
	}

	con_backscroll = 0;
//	con_current = con_totallines - 1;	// 2000-01-05 Console scrolling fix by Maddes
}

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
================
Con_Init_Cvars
================
*/
void Con_Init_Cvars (void)
{
	con_notifytime = Cvar_Get ("con_notifytime", "3", CVAR_ORIGINAL);

// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  start
// 2000-01-11 Transparent console by Radix  start
	con_alpha = Cvar_Get ("con_alpha", "1", CVAR_ARCHIVE);
	Cvar_SetRangecheck (con_alpha, Cvar_RangecheckFloat, 0, 1);
	Cvar_Set(con_alpha, con_alpha->string);	// do rangecheck
// 2000-01-11 Transparent console by Radix  end
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  end
}
// 2001-09-18 New cvar system by Maddes (Init)  end

/*
================
Con_Init
================
*/
void Con_Init (void)
{
#define MAXGAMEDIRLEN	1000
	char	temp[MAXGAMEDIRLEN+1];
	char	*t2 = "/qconsole.log";

	con_debuglog = COM_CheckParm("-condebug");

	if (con_debuglog)
	{
		if (strlen (com_gamedir) < (MAXGAMEDIRLEN - strlen (t2)))
		{
			sprintf (temp, "%s%s", com_gamedir, t2);
			unlink (temp);
		}
	}

	con_text = Hunk_AllocName (CON_TEXTSIZE, "context");
// 2000-01-05 Console scrolling fix by Maddes  start
//	Q_memset (con_text, ' ', CON_TEXTSIZE);
	Con_Clear_f();
// 2000-01-05 Console scrolling fix by Maddes  end
	con_linewidth = -1;
	Con_CheckResize ();

//	Con_Printf ("Console initialized.\n");	// 2000-01-05 Console scrolling fix by Maddes

//
// register our commands
//
// 2001-09-18 New cvar system by Maddes (Init)  start
/*
	con_notifytime = Cvar_Get ("con_notifytime", "3", CVAR_ORIGINAL);

// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  start
// 2000-01-11 Transparent console by Radix  start
	con_alpha = Cvar_Get ("con_alpha", "1", CVAR_ARCHIVE);
	Cvar_SetRangecheck (con_alpha, Cvar_RangecheckFloat, 0, 1);
	Cvar_Set(con_alpha, con_alpha->string);	// do rangecheck
// 2000-01-11 Transparent console by Radix  end
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  end
*/
// 2001-09-18 New cvar system by Maddes (Init)  end

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("clear", Con_Clear_f);

	con_initialized = true;
	Con_Printf ("Console initialized.\n");	// 2000-01-05 Console scrolling fix by Maddes
}


/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (void)
{
	con_x = 0;
// 2001-12-15 Avoid automatic console scrolling by Fett  start
	if (con_backscroll)
		con_backscroll++;
// 2001-12-15 Avoid automatic console scrolling by Fett  end
	con_current++;
	Q_memset (&con_text[((con_current-1)%con_totallines)*con_linewidth]
	, ' ', con_linewidth);	// 2000-01-05 Console scrolling fix by Maddes
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the notify window will pop up.
================
*/
void Con_Print (char *txt)
{
	int		y;
	int		c, l;
	static int	cr;
	int		mask;

//	con_backscroll = 0;	// 2001-12-15 Avoid automatic console scrolling by Fett

	if (txt[0] == 1)
	{
		mask = 128;		// go to colored text
		S_LocalSound ("misc/talk.wav");
	// play talk wav
		txt++;
	}
	else if (txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
		mask = 0;


	while ( (c = *txt) )
	{
	// count word length
		for (l=0 ; l< con_linewidth ; l++)
			if ( txt[l] <= ' ')
				break;

	// word wrap
		if (l != con_linewidth && (con_x + l > con_linewidth) )
			con_x = 0;

		txt++;

		if (cr)
		{
			con_current--;
			cr = false;
		}


		if (!con_x)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
// 2000-01-05 Console scrolling fix by Maddes  start
/*
			if (con_current >= 0)
				con_times[con_current % NUM_CON_TIMES] = realtime;
*/
			if (con_current > 0)
				con_times[(con_current-1) % NUM_CON_TIMES] = realtime;
// 2000-01-05 Console scrolling fix by Maddes  end
		}

		switch (c)
		{
		case '\n':
			con_x = 0;
			break;

		case '\r':
			con_x = 0;
			cr = 1;
			break;

		default:	// display character and advance
// 2000-01-05 Console scrolling fix by Maddes  start
//			y = con_current % con_totallines;
			y = (con_current-1) % con_totallines;
// 2000-01-05 Console scrolling fix by Maddes  end
			con_text[y*con_linewidth+con_x] = c | mask;
			con_x++;
			if (con_x >= con_linewidth)
				con_x = 0;
			break;
		}

	}
}


/*
================
Con_DebugLog
================
*/
void Con_DebugLog(char *file, char *fmt, ...)
{
	va_list	argptr;
	static char	data[1024];
	int	fd;

	va_start(argptr, fmt);
	vsprintf(data, fmt, argptr);
	va_end(argptr);
	fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
	write(fd, data, strlen(data));
	close(fd);
}


/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
================
*/
#define	MAXPRINTMSG	4096
// FIXME: make a buffer size safe vsprintf?
void Con_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

// also echo to debugging console
	Sys_Printf ("%s", msg);	// also echo to debugging console

// log all messages to file
	if (con_debuglog)
		Con_DebugLog(va("%s/qconsole.log",com_gamedir), "%s", msg);

	if (!con_initialized)
		return;

	if (cls.state == ca_dedicated)
		return;		// no graphics mode

// write it to the scrollable buffer
	Con_Print (msg);

// update the screen if the console is displayed
	if (cls.signon != SIGNONS && !scr_disabled_for_loading )
	{
	// protect against infinite loop if something in SCR_UpdateScreen calls
	// Con_Printd
		if (!inupdate)
		{
			inupdate = true;
			SCR_UpdateScreen ();
			inupdate = false;
		}
	}
}

/*
================
Con_DPrintf

A Con_Printf that only shows up if the "developer" cvar is set
================
*/
void Con_DPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!developer->value)
		return;			// don't confuse non-developers with techie stuff...

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	Con_Printf ("%s", msg);
}


/*
==================
Con_SafePrintf

Okay to call even when the screen can't be updated
==================
*/
void Con_SafePrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[1024];
	int			temp;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	Con_Printf ("%s", msg);
	scr_disabled_for_loading = temp;
}


/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Con_DrawInput (void)
{
// 2000-01-05 Console typing enhancement by Radix  start
// use strlen of edit_line instead of key_linepos to allow editing
// of early characters w/o erasing
// 2000-01-05 Console typing enhancement by Radix  end

	int		y;
	int		i;
	char	*text;
	char	editlinecopy[256];	// 2000-01-05 Console typing enhancement by Radix

	if (key_dest != key_console && !con_forcedup)
		return;		// don't draw anything

// 2000-01-05 Console typing enhancement by Radix  start
/*
	text = key_lines[edit_line];

// add the cursor frame
	text[key_linepos] = 10+((int)(realtime*con_cursorspeed)&1);

// fill out remainder with spaces
	for (i=key_linepos+1 ; i< con_linewidth ; i++)
		text[i] = ' ';
*/
	text = strcpy(editlinecopy, key_lines[edit_line]);

// fill out remainder with spaces
	y = strlen(text);
	for (i = y; i < 256; i++)
	{
		text[i] = ' ';
	}

// add the cursor frame
	if ((int)(realtime * con_cursorspeed) & 1)	// cursor is visible
	{
		text[key_linepos] = 11 + 130 * key_insert;	// either solid block or triagle facing right
	}
// 2000-01-05 Console typing enhancement by Radix  end

//	prestep if horizontally scrolling
	if (key_linepos >= con_linewidth)
		text += 1 + key_linepos - con_linewidth;

// draw it
	y = con_vislines-16;

	for (i=0 ; i<con_linewidth ; i++)
// 2001-12-10 Reduced compiler warnings by Jeff Ford  start
	{
#ifdef SCALED2D
		if (console_scaled)
		Draw_Character_Scaled ( (i+1)<<3, y, text[i]);
		else
		Draw_Character ( (i+1)<<3, y, text[i]);
#else
		Draw_Character ( (i+1)<<3, y, text[i]);
#endif
	}
// 2001-12-10 Reduced compiler warnings by Jeff Ford  end

// remove cursor
//	key_lines[edit_line][key_linepos] = 0;	// 2000-01-05 Console typing enhancement by Radix
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/

void Con_DrawNotify (void)
{
	int		x, v;
	char	*text;
	int		i;
	float	time;
	extern char chat_buffer[];

	v = 0;
// 2000-01-05 Console scrolling fix by Maddes  start
//	for (i= con_current-NUM_CON_TIMES+1 ; i<=con_current ; i++)
	for (i= con_current-NUM_CON_TIMES ; i<con_current ; i++)
// 2000-01-05 Console scrolling fix by Maddes  end
	{
		if (i < 0)
			continue;
		time = con_times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = realtime - time;
		if (time > con_notifytime->value)
			continue;
		text = con_text + (i % con_totallines)*con_linewidth;

		clearnotify = 0;
		scr_copytop = 1;
#ifdef SCALED2D
		if (sb_scaled){	// often used as a hud
		for (x = 0 ; x < con_linewidth ; x++)
			Draw_Character_Scaled ( (x+1)<<3, v, text[x]);
		}
		else
		{
			for (x = 0 ; x < con_linewidth ; x++)
			Draw_Character ( (x+1)<<3, v, text[x]);
		}

		v +=8;
	}


	if (key_dest == key_message)
	{
		clearnotify = 0;
		scr_copytop = 1;

		x = 0;
		if(sb_scaled){
		Draw_String_Scaled (8, v, "say:");
		while(chat_buffer[x])
		{
			Draw_Character_Scaled ( (x+5)<<3, v, chat_buffer[x]);
			x++;
		}
		Draw_Character_Scaled ( (x+5)<<3, v, 10+((int)(realtime*con_cursorspeed)&1));
		v += 8;
		}
		else
		{
		Draw_String (8, v, "say:");
		while(chat_buffer[x])
		{
			Draw_Character( (x+5)<<3, v, chat_buffer[x]);
			x++;
		}
		Draw_Character ( (x+5)<<3, v, 10+((int)(realtime*con_cursorspeed)&1));
		v += 8;
		}
	}
#else

			for (x = 0 ; x < con_linewidth ; x++)
			Draw_Character ( (x+1)<<3, v, text[x]);

		v +=8;
	}


	if (key_dest == key_message)
	{
		clearnotify = 0;
		scr_copytop = 1;

		x = 0;

		Draw_String (8, v, "say:");
		while(chat_buffer[x])
		{
			Draw_Character( (x+5)<<3, v, chat_buffer[x]);
			x++;
		}
		Draw_Character ( (x+5)<<3, v, 10+((int)(realtime*con_cursorspeed)&1));
		v += 8;
	}
#endif
	if (v > con_notifylines)
		con_notifylines = v;
}

/*
================
Con_DrawConsole

Draws the console with the solid background
The typing input line at the bottom should only be drawn if typing is allowed
================
*/
void Con_DrawConsole (int lines, qboolean drawinput)
{
	int		i, x, y;
	int		rows;
	char	*text;
	int		j;
	int		sb;	// 2001-12-15 Avoid automatic console scrolling by Fett

	if (lines <= 0)
		return;

// draw the background
	Draw_ConsoleBackground (lines);

// draw the text
	con_vislines = lines;

	rows = (lines-16)>>3;		// rows of text to draw
	y = lines - 16 - (rows<<3);	// may start slightly negative

// 2000-01-05 Console scrolling fix by Maddes  start
	if (con_backscroll >= con_current)
		con_backscroll = con_current - 1;
	if (con_backscroll >= con_totallines)
		con_backscroll = con_totallines - 1;
	if (con_backscroll < 0)
		con_backscroll = 0;
// 2000-01-05 Console scrolling fix by Maddes  end

// 2001-12-15 Avoid automatic console scrolling by Fett  start
	if (con_backscroll)
	{
		sb=1;	// reserve line for scrollback indicator
	}
	else
	{
		sb=0;
	}
// 2001-12-15 Avoid automatic console scrolling by Fett  end

// 2000-01-05 Console scrolling fix by Maddes  start
//	for (i= con_current - rows + 1 ; i<=con_current ; i++, y+=8 )
// 2001-12-15 Avoid automatic console scrolling by Fett  start
//	for (i= con_current - rows; i<con_current ; i++, y+=8 )
	for (i= con_current - rows + sb; i<con_current ; i++, y+=8)
// 2001-12-15 Avoid automatic console scrolling by Fett  end
// 2000-01-05 Console scrolling fix by Maddes  end
	{
		j = i - con_backscroll;
// 2000-01-05 Console scrolling fix by Maddes  start
/*
		if (j<0)
			j = 0;
*/
		if ((j<0) || (j < (con_current - con_totallines)))
		{
			continue;
		}
// 2000-01-05 Console scrolling fix by Maddes  end
		text = con_text + (j % con_totallines)*con_linewidth;
#ifdef SCALED2D
		if (console_scaled){
		for (x=0 ; x<con_linewidth ; x++)
			Draw_Character_Scaled ( (x+1)<<3, y, text[x]);
		}
		else
#endif
		{

		for (x=0 ; x<con_linewidth ; x++)
			Draw_Character ( (x+1)<<3, y, text[x]);

		}
	}
// 2001-12-15 Avoid automatic console scrolling by Fett  start
	if (sb)	// are we scrolled back?
	{
#ifdef SCALED2D
		if (console_scaled){
		// draw arrows to show the buffer is backscrolled
		for (x=0 ; x<con_linewidth ; x+=4)
			Draw_Character_Scaled ((x+1)<<3, y, '^');
		} else 
#endif
		{
		for (x=0 ; x<con_linewidth ; x+=4)
			Draw_Character ((x+1)<<3, y, '^');
		}
	}
// 2001-12-15 Avoid automatic console scrolling by Fett  end

// draw the input prompt, user text, and cursor if desired
	if (drawinput)
		Con_DrawInput ();
}


/*
==================
Con_NotifyBox
==================
*/
void Con_NotifyBox (char *text)
{
	double		t1, t2;

// during startup for sound / cd warnings
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	Con_Printf (text);

	Con_Printf ("Press a key.\n");
	Con_Printf("\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	key_count = -2;		// wait for a key down and up
	key_dest = key_console;

	do
	{
		t1 = Sys_FloatTime ();
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();
		t2 = Sys_FloatTime ();
		realtime += t2-t1;		// make the cursor blink
	} while (key_count < 0);

	Con_Printf ("\n");
	key_dest = key_game;
	realtime = 0;				// put the cursor back to invisible
}

