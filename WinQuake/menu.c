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

#ifdef _WIN32
#include "winquake.h"
#endif

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

enum {m_none, m_main, m_singleplayer, m_load, m_save, m_multiplayer, m_setup, m_net, m_options, m_video, m_keys, m_help, m_quit, m_serialconfig, m_modemconfig, m_lanconfig, m_gameoptions, m_search, m_slist} m_state;

void M_Menu_Main_f (void);
	void M_Menu_SinglePlayer_f (void);
		void M_Menu_Load_f (void);
		void M_Menu_Save_f (void);
	void M_Menu_MultiPlayer_f (void);
		void M_Menu_Setup_f (void);
		void M_Menu_Net_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Keys_f (void);
		void M_Menu_Video_f (void);
	void M_Menu_Help_f (void);
	void M_Menu_Quit_f (void);
void M_Menu_SerialConfig_f (void);
	void M_Menu_ModemConfig_f (void);
void M_Menu_LanConfig_f (void);
void M_Menu_GameOptions_f (void);
void M_Menu_Search_f (void);
void M_Menu_ServerList_f (void);

void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Load_Draw (void);
		void M_Save_Draw (void);
	void M_MultiPlayer_Draw (void);
		void M_Setup_Draw (void);
		void M_Net_Draw (void);
//	void M_Options_Draw (void);	// 2002-01-31 New menu system by Maddes
		void M_Keys_Draw (void);
		void M_Video_Draw (void);
	void M_Help_Draw (void);
	void M_Quit_Draw (void);
void M_SerialConfig_Draw (void);
	void M_ModemConfig_Draw (void);
void M_LanConfig_Draw (void);
void M_GameOptions_Draw (void);
void M_Search_Draw (void);
void M_ServerList_Draw (void);

void M_Main_Key (int key);
	void M_SinglePlayer_Key (int key);
		void M_Load_Key (int key);
		void M_Save_Key (int key);
	void M_MultiPlayer_Key (int key);
		void M_Setup_Key (int key);
		void M_Net_Key (int key);
//	void M_Options_Key (int key);	// 2002-01-31 New menu system by Maddes
		void M_Keys_Key (int key);
		void M_Video_Key (int key);
	void M_Help_Key (int key);
	void M_Quit_Key (int key);
void M_SerialConfig_Key (int key);
	void M_ModemConfig_Key (int key);
void M_LanConfig_Key (int key);
void M_GameOptions_Key (int key);
void M_Search_Key (int key);
void M_ServerList_Key (int key);

qboolean	m_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
qboolean	m_recursiveDraw;

int			m_return_state;
qboolean	m_return_onerror;
char		m_return_reason [32];

#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame		(m_multiplayer_cursor == 0)
#define SerialConfig	(m_net_cursor == 0)
#define DirectConfig	(m_net_cursor == 1)
#define	IPXConfig		(m_net_cursor == 2)
#define	TCPIPConfig		(m_net_cursor == 3)

void M_ConfigureNetSubsystem(void);

// 2002-01-31 New menu system by Maddes  start
// menu entry types
#define MENU_SELECTABLE	0
#define MENU_DRAW_ONLY	1
#define MENU_INVISIBLE	2

typedef struct menu_definition_s
{
	int	funcno;	// Unique number used for displaying and executing a menu entry. Zero = end of menu definition
				// First entry in menu definition defines the ESC function
	int	type;	// Entry type flag, use above DEFINEs and M_Menu_DrawCheck()
} menu_definition_t;

// data of current displayed menu
int		*current_cursor;
menu_definition_t	*current_menu;
int 	menu_last_index, menu_first_index;

// function number definitions
// Menus
#define MENU_OFF				1
#define MENU_MAIN				2
#define MENU_SINGLEPLAYER		3
#define MENU_MULTIPLAYER		4
#define MENU_OPTIONS			5
#define MENU_HELP				6
#define MENU_QUIT				7

// Options
#define MENU_CUSTOMIZE_CONTROLS			101
#define MENU_GO_TO_CONSOLE				102
#define MENU_LOAD_DEFAULT_CFG			103
#define MENU_SCREENSIZE					104
#define MENU_BRIGHTNESS					105
#define MENU_MOUSESPEED					106
#define MENU_CD_VOLUME					107
#define MENU_SOUND_VOLUME				108
#define MENU_ALWAYS_RUN					109
#define MENU_INVERT_MOUSE				110
#define MENU_LOOKSPRING					111
#define MENU_LOOKSTRAFE					112
#define MENU_VIDEO_RESOLUTION			113
#define MENU_USE_MOUSE					114

#define MENU_CONTROL_OPTIONS			120
#define MENU_MOUSELOOK					121

#define MENU_SOUND_OPTIONS				130

#define MENU_EXTERNAL_DATA				140
#define MENU_EXTERNAL_ENT				141
#define MENU_EXTERNAL_VIS				142
#define MENU_EXTERNAL_LIT				143

#define MENU_CLIENT_OPTIONS				150
#define MENU_CL_ENTITIES_MIN			151
#define MENU_CL_ENTITIES_TEMP_MIN		152
#define MENU_CL_ENTITIES_STATIC_MIN		153
#define MENU_CL_COMPATIBILITY			154

#define MENU_SERVER_OPTIONS				160
#define MENU_SV_ENTITIES				161
#define MENU_SV_ENTITIES_TEMP			162
#define MENU_SV_ENTITIES_STATIC			163
#define MENU_SV_ENTITIES_COPY_TO_CL		164
#define MENU_PR_ZONE_MIN_STRINGS		165
#define MENU_BUILTIN_REMAP				166
#define MENU_SV_COMPATIBILITY			167
#define MENU_NVS_ENABLE					168

#define MENU_VIDEO_OPTIONS				170
#define MENU_CON_ALPHA					171
#define MENU_CON_HEIGHT					172
#define MENU_SHOW_FPS					173
#define MENU_GL_MAXDEPTH				174

// menu definitions
int		options_cursor;
menu_definition_t	m_menu_options[] =
{	// Options Menu
	{MENU_MAIN, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_CUSTOMIZE_CONTROLS, MENU_SELECTABLE},
	{MENU_GO_TO_CONSOLE, MENU_SELECTABLE},
	{MENU_LOAD_DEFAULT_CFG, MENU_SELECTABLE},
	{MENU_CONTROL_OPTIONS, MENU_SELECTABLE},
	{MENU_SOUND_OPTIONS, MENU_SELECTABLE},
	{MENU_EXTERNAL_DATA, MENU_SELECTABLE},
	{MENU_CLIENT_OPTIONS, MENU_SELECTABLE},
	{MENU_SERVER_OPTIONS, MENU_SELECTABLE},
	{MENU_VIDEO_OPTIONS, MENU_SELECTABLE},
	{MENU_VIDEO_RESOLUTION, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

int		control_options_cursor;
menu_definition_t	m_menu_control_options[] =
{	// Control Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_ALWAYS_RUN, MENU_SELECTABLE},
	{MENU_MOUSELOOK, MENU_SELECTABLE},
	{MENU_LOOKSPRING, MENU_SELECTABLE},
	{MENU_LOOKSTRAFE, MENU_SELECTABLE},
	{MENU_MOUSESPEED, MENU_SELECTABLE},
	{MENU_INVERT_MOUSE, MENU_SELECTABLE},
#ifdef _WIN32
	{MENU_USE_MOUSE, MENU_INVISIBLE},	// only present in windowed mode on Win32
#endif
	{0, 0},	// end of submenu
};

int		sound_options_cursor;
menu_definition_t	m_menu_sound_options[] =
{	// Sound Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_CD_VOLUME, MENU_SELECTABLE},
	{MENU_SOUND_VOLUME, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

int		external_data_cursor;
menu_definition_t	m_menu_external_data[] =
{	// External Data
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_EXTERNAL_ENT, MENU_SELECTABLE},
	{MENU_EXTERNAL_VIS, MENU_SELECTABLE},
#ifdef GLQUAKE
	{MENU_EXTERNAL_LIT, MENU_SELECTABLE},	// only useful in GLQuake
#endif
	{0, 0},	// end of submenu
};

int		client_options_cursor;
menu_definition_t	m_menu_client_options[] =
{	// Client Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_CL_ENTITIES_MIN, MENU_SELECTABLE},
	{MENU_CL_ENTITIES_TEMP_MIN, MENU_SELECTABLE},
	{MENU_CL_ENTITIES_STATIC_MIN, MENU_SELECTABLE},
	{MENU_CL_COMPATIBILITY, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

int		server_options_cursor;
menu_definition_t	m_menu_server_options[] =
{	// Server Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_SV_ENTITIES, MENU_SELECTABLE},
	{MENU_SV_ENTITIES_TEMP, MENU_SELECTABLE},
	{MENU_SV_ENTITIES_STATIC, MENU_SELECTABLE},
	{MENU_SV_ENTITIES_COPY_TO_CL, MENU_SELECTABLE},
	{MENU_PR_ZONE_MIN_STRINGS, MENU_SELECTABLE},
	{MENU_BUILTIN_REMAP, MENU_SELECTABLE},
	{MENU_SV_COMPATIBILITY, MENU_SELECTABLE},
	{MENU_NVS_ENABLE, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

int		video_options_cursor;
menu_definition_t	m_menu_video_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_SCREENSIZE, MENU_SELECTABLE},
	{MENU_BRIGHTNESS, MENU_SELECTABLE},
	{MENU_CON_ALPHA, MENU_SELECTABLE},
	{MENU_CON_HEIGHT, MENU_SELECTABLE},
	{MENU_SHOW_FPS, MENU_SELECTABLE},
#ifdef GLQUAKE
	{MENU_GL_MAXDEPTH, MENU_SELECTABLE},
#endif
#ifdef _WIN32
	{MENU_USE_MOUSE, MENU_INVISIBLE},	// only present in windowed mode on Win32
#endif
	{0, 0},	// end of submenu
};
// 2002-01-31 New menu system by Maddes  end

/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character ( cx + ((vid.width - 320)>>1), line, num);
}

void M_Print (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void M_PrintWhite (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic (int x, int y, qpic_t *pic)
{
	Draw_TransPic (x + ((vid.width - 320)>>1), y, pic);
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x + ((vid.width - 320)>>1), y, pic);
}

byte identityTable[256];
byte translationTable[256];

void M_BuildTranslationTable(int top, int bottom)
{
	int		j;
	byte	*dest, *source;

	for (j = 0; j < 256; j++)
		identityTable[j] = j;
	dest = translationTable;
	source = identityTable;
	memcpy (dest, source, 256);

	if (top < 128)	// the artists made some backwards ranges.  sigh.
		memcpy (dest + TOP_RANGE, source + top, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[TOP_RANGE+j] = source[top+15-j];

	if (bottom < 128)
		memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[BOTTOM_RANGE+j] = source[bottom+15-j];
}


void M_DrawTransPicTranslate (int x, int y, qpic_t *pic)
{
	Draw_TransPicTranslate (x + ((vid.width - 320)>>1), y, pic, translationTable);
}


void M_DrawTextBox (int x, int y, int width, int lines)
{
	qpic_t	*p;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawTransPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawTransPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawTransPic (cx, cy+8, p);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawTransPic (cx, cy+8, p);
}

//=============================================================================

int m_save_demonum;

/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu_f (void)
{
	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (m_state != m_main)
		{
			M_Menu_Main_f ();
			return;
		}
		key_dest = key_game;
		m_state = m_none;
		return;
	}
	if (key_dest == key_console)
	{
		Con_ToggleConsole_f ();
	}
	else
	{
		M_Menu_Main_f ();
	}
}


//=============================================================================
/* MAIN MENU */

int	m_main_cursor;
#define	MAIN_ITEMS	5


void M_Menu_Main_f (void)
{
	if (key_dest != key_menu)
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;
}


void M_Main_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_main.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mainmenu.lmp") );

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end

	M_DrawTransPic (54, 32 + m_main_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_Main_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		key_dest = key_game;
		m_state = m_none;
		cls.demonum = m_save_demonum;
		if (cls.demonum != -1 && !cls.demoplayback && cls.state != ca_connected)
			CL_NextDemo ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_main_cursor >= MAIN_ITEMS)
			m_main_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_main_cursor < 0)
			m_main_cursor = MAIN_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;

		switch (m_main_cursor)
		{
		case 0:
			M_Menu_SinglePlayer_f ();
			break;

		case 1:
			M_Menu_MultiPlayer_f ();
			break;

		case 2:
			M_Menu_Options_f ();
			break;

		case 3:
			M_Menu_Help_f ();
			break;

		case 4:
			M_Menu_Quit_f ();
			break;
		}
	}
}

//=============================================================================
/* SINGLE PLAYER MENU */

int	m_singleplayer_cursor;
#define	SINGLEPLAYER_ITEMS	3


void M_Menu_SinglePlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_singleplayer;
	m_entersound = true;
}


void M_SinglePlayer_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_sgl.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/sp_menu.lmp") );

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end

	M_DrawTransPic (54, 32 + m_singleplayer_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_SinglePlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
			m_singleplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_singleplayer_cursor < 0)
			m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;

		switch (m_singleplayer_cursor)
		{
		case 0:
			if (sv.active)
				if (!SCR_ModalMessage("Are you sure you want to\nstart a new game?\n"))
					break;
			key_dest = key_game;
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("map start\n");
			break;

		case 1:
			M_Menu_Load_f ();
			break;

		case 2:
			M_Menu_Save_f ();
			break;
		}
	}
}

//=============================================================================
/* LOAD/SAVE MENU */

int		load_cursor;		// 0 < load_cursor < MAX_SAVEGAMES

#define	MAX_SAVEGAMES		12
char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
int		loadable[MAX_SAVEGAMES];

void M_ScanSaves (void)
{
	int		i, j;
	char	name[MAX_OSPATH];
	FILE	*f;
	int		version;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		sprintf (name, "%s/s%i.sav", com_gamedir, i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		strncpy (m_filenames[i], name, sizeof(m_filenames[i])-1);

	// change _ back to space
		for (j=0 ; j<SAVEGAME_COMMENT_LENGTH ; j++)
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		loadable[i] = true;
		fclose (f);
	}
}

void M_Menu_Load_f (void)
{
	m_entersound = true;
	m_state = m_load;
	key_dest = key_menu;
	M_ScanSaves ();
}


void M_Menu_Save_f (void)
{
	if (!sv.active)
		return;
	if (cl.intermission)
		return;
	if (svs.maxclients != 1)
		return;
	m_entersound = true;
	m_state = m_save;
	key_dest = key_menu;
	M_ScanSaves ();
}


void M_Load_Draw (void)
{
	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_load.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	for (i=0 ; i< MAX_SAVEGAMES; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Save_Draw (void)
{
	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_save.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Load_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_SinglePlayer_f ();
		break;

	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		key_dest = key_game;

	// Host_Loadgame_f can't bring up the loading plaque because too much
	// stack space has been used, so do it now
		SCR_BeginLoadingPlaque ();

	// issue the load command
		Cbuf_AddText (va ("load s%i\n", load_cursor) );
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}


void M_Save_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_SinglePlayer_f ();
		break;

	case K_ENTER:
		m_state = m_none;
		key_dest = key_game;
		Cbuf_AddText (va("save s%i\n", load_cursor));
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}

//=============================================================================
/* MULTIPLAYER MENU */

int	m_multiplayer_cursor;
#define	MULTIPLAYER_ITEMS	3


void M_Menu_MultiPlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_multiplayer;
	m_entersound = true;
}


void M_MultiPlayer_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mp_menu.lmp") );

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end

	M_DrawTransPic (54, 32 + m_multiplayer_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );

	if (serialAvailable || ipxAvailable || tcpipAvailable)
		return;
	M_PrintWhite ((320/2) - ((27*8)/2), 148, "No Communications Available");
}


void M_MultiPlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_multiplayer_cursor >= MULTIPLAYER_ITEMS)
			m_multiplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_multiplayer_cursor < 0)
			m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;
		switch (m_multiplayer_cursor)
		{
		case 0:
			if (serialAvailable || ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 1:
			if (serialAvailable || ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 2:
			M_Menu_Setup_f ();
			break;
		}
	}
}

//=============================================================================
/* SETUP MENU */

int		setup_cursor = 4;
int		setup_cursor_table[] = {40, 56, 80, 104, 140};

char	setup_hostname[16];
char	setup_myname[16];
int		setup_oldtop;
int		setup_oldbottom;
int		setup_top;
int		setup_bottom;

#define	NUM_SETUP_CMDS	5

void M_Menu_Setup_f (void)
{
	key_dest = key_menu;
	m_state = m_setup;
	m_entersound = true;
	strcpy(setup_myname, cl_name->string);
	strcpy(setup_hostname, hostname->string);
	setup_top = setup_oldtop = ((int)cl_color->value) >> 4;
	setup_bottom = setup_oldbottom = ((int)cl_color->value) & 15;
}


void M_Setup_Draw (void)
{
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	M_Print (64, 40, "Hostname");
	M_DrawTextBox (160, 32, 16, 1);
	M_Print (168, 40, setup_hostname);

	M_Print (64, 56, "Your name");
	M_DrawTextBox (160, 48, 16, 1);
	M_Print (168, 56, setup_myname);

	M_Print (64, 80, "Shirt color");
	M_Print (64, 104, "Pants color");

	M_DrawTextBox (64, 140-8, 14, 1);
	M_Print (72, 140, "Accept Changes");

	p = Draw_CachePic ("gfx/bigbox.lmp");
	M_DrawTransPic (160, 64, p);
	p = Draw_CachePic ("gfx/menuplyr.lmp");
	M_BuildTranslationTable(setup_top*16, setup_bottom*16);
	M_DrawTransPicTranslate (172, 72, p);

	M_DrawCharacter (56, setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));

	if (setup_cursor == 0)
		M_DrawCharacter (168 + 8*strlen(setup_hostname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));

	if (setup_cursor == 1)
		M_DrawCharacter (168 + 8*strlen(setup_myname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
}


void M_Setup_Key (int k)
{
	int			l;

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor--;
		if (setup_cursor < 0)
			setup_cursor = NUM_SETUP_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 0;
		break;

	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 2)
			setup_top = setup_top - 1;
		if (setup_cursor == 3)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 2)
			setup_top = setup_top + 1;
		if (setup_cursor == 3)
			setup_bottom = setup_bottom + 1;
		break;

	case K_ENTER:
		if (setup_cursor == 0 || setup_cursor == 1)
			return;

		if (setup_cursor == 2 || setup_cursor == 3)
			goto forward;

		// setup_cursor == 4 (OK)
		if (strcmp(cl_name->string, setup_myname) != 0)
			Cbuf_AddText ( va ("name \"%s\"\n", setup_myname) );
		if (strcmp(hostname->string, setup_hostname) != 0)
			Cvar_Set(hostname, setup_hostname);
		if (setup_top != setup_oldtop || setup_bottom != setup_oldbottom)
			Cbuf_AddText( va ("color %i %i\n", setup_top, setup_bottom) );
		m_entersound = true;
		M_Menu_MultiPlayer_f ();
		break;

	case K_BACKSPACE:
		if (setup_cursor == 0)
		{
			if (strlen(setup_hostname))
				setup_hostname[strlen(setup_hostname)-1] = 0;
		}

		if (setup_cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
		}
		break;

	default:
		if (k < 32 || k > 127)
			break;
		if (setup_cursor == 0)
		{
			l = strlen(setup_hostname);
			if (l < 15)
			{
				setup_hostname[l+1] = 0;
				setup_hostname[l] = k;
			}
		}
		if (setup_cursor == 1)
		{
			l = strlen(setup_myname);
			if (l < 15)
			{
				setup_myname[l+1] = 0;
				setup_myname[l] = k;
			}
		}
	}

	if (setup_top > 13)
		setup_top = 0;
	if (setup_top < 0)
		setup_top = 13;
	if (setup_bottom > 13)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 13;
}

//=============================================================================
/* NET MENU */

int	m_net_cursor;
int m_net_items;
int m_net_saveHeight;

char *net_helpMessage [] =
{
/* .........1.........2.... */
  "                        ",
  " Two computers connected",
  "   through two modems.  ",
  "                        ",

  "                        ",
  " Two computers connected",
  " by a null-modem cable. ",
  "                        ",

  " Novell network LANs    ",
  " or Windows 95 DOS-box. ",
  "                        ",
  "(LAN=Local Area Network)",

  " Commonly used to play  ",
  " over the Internet, but ",
  " also used on a Local   ",
  " Area Network.          "
};

void M_Menu_Net_f (void)
{
	key_dest = key_menu;
	m_state = m_net;
	m_entersound = true;
	m_net_items = 4;

	if (m_net_cursor >= m_net_items)
		m_net_cursor = 0;
	m_net_cursor--;
	M_Net_Key (K_DOWNARROW);
}


void M_Net_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	f = 32;

	if (serialAvailable)
	{
		p = Draw_CachePic ("gfx/netmen1.lmp");
	}
	else
	{
#ifdef _WIN32
		p = NULL;
#else
		p = Draw_CachePic ("gfx/dim_modm.lmp");
#endif
	}

	if (p)
		M_DrawTransPic (72, f, p);

	f += 19;

	if (serialAvailable)
	{
		p = Draw_CachePic ("gfx/netmen2.lmp");
	}
	else
	{
#ifdef _WIN32
		p = NULL;
#else
		p = Draw_CachePic ("gfx/dim_drct.lmp");
#endif
	}

	if (p)
		M_DrawTransPic (72, f, p);

	f += 19;
	if (ipxAvailable)
		p = Draw_CachePic ("gfx/netmen3.lmp");
	else
		p = Draw_CachePic ("gfx/dim_ipx.lmp");
	M_DrawTransPic (72, f, p);

	f += 19;
	if (tcpipAvailable)
		p = Draw_CachePic ("gfx/netmen4.lmp");
	else
		p = Draw_CachePic ("gfx/dim_tcp.lmp");
	M_DrawTransPic (72, f, p);

	if (m_net_items == 5)	// JDC, could just be removed
	{
		f += 19;
		p = Draw_CachePic ("gfx/netmen5.lmp");
		M_DrawTransPic (72, f, p);
	}

	f = (320-26*8)/2;
	M_DrawTextBox (f, 134, 24, 4);
	f += 8;
	M_Print (f, 142, net_helpMessage[m_net_cursor*4+0]);
	M_Print (f, 150, net_helpMessage[m_net_cursor*4+1]);
	M_Print (f, 158, net_helpMessage[m_net_cursor*4+2]);
	M_Print (f, 166, net_helpMessage[m_net_cursor*4+3]);

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
	M_DrawTransPic (54, 32 + m_net_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_Net_Key (int k)
{
again:
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_net_cursor >= m_net_items)
			m_net_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_net_cursor < 0)
			m_net_cursor = m_net_items - 1;
		break;

	case K_ENTER:
		m_entersound = true;

		switch (m_net_cursor)
		{
		case 0:
			M_Menu_SerialConfig_f ();
			break;

		case 1:
			M_Menu_SerialConfig_f ();
			break;

		case 2:
			M_Menu_LanConfig_f ();
			break;

		case 3:
			M_Menu_LanConfig_f ();
			break;

		case 4:
// multiprotocol
			break;
		}
	}

	if (m_net_cursor == 0 && !serialAvailable)
		goto again;
	if (m_net_cursor == 1 && !serialAvailable)
		goto again;
	if (m_net_cursor == 2 && !ipxAvailable)
		goto again;
	if (m_net_cursor == 3 && !tcpipAvailable)
		goto again;
}

//=============================================================================
/* OPTIONS MENU */

// 2002-01-31 New menu system by Maddes  start
// no more messing around with OPTIONS_ITEMS :)
/*
#ifdef _WIN32
#define	OPTIONS_ITEMS	15	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
#else
#define	OPTIONS_ITEMS	14	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
#endif
*/
// 2002-01-31 New menu system by Maddes  end

#define	SLIDER_RANGE	10

//int		options_cursor;	// 2002-01-31 New menu system by Maddes

void M_Menu_Options_f (void)
{
	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;

// 2002-01-31 New menu system by Maddes  start
	current_menu = m_menu_options;
	current_cursor = &options_cursor;

/*
#ifdef _WIN32
	if ((options_cursor == 14) && (modestate != MS_WINDOWED))	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
	{
		options_cursor = 0;
	}
#endif
*/
// 2002-01-31 New menu system by Maddes  end
}

// 2002-01-31 New menu system by Maddes  start
/*
void M_AdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (options_cursor)
	{
	case 3:	// screen size
		scr_viewsize->value += dir * 10;
		if (scr_viewsize->value < 30)
			scr_viewsize->value = 30;
		if (scr_viewsize->value > 120)
			scr_viewsize->value = 120;
		Cvar_SetValue (scr_viewsize, scr_viewsize->value);
		break;
	case 4:	// gamma
		v_gamma->value -= dir * 0.05;
		if (v_gamma->value < 0.5)
			v_gamma->value = 0.5;
		if (v_gamma->value > 1)
			v_gamma->value = 1;
		Cvar_SetValue (v_gamma, v_gamma->value);
		break;
	case 5:	// mouse speed
		sensitivity->value += dir * 0.5;
		if (sensitivity->value < 1)
			sensitivity->value = 1;
		if (sensitivity->value > 11)
			sensitivity->value = 11;
		Cvar_SetValue (sensitivity, sensitivity->value);
		break;
	case 6:	// music volume
#ifdef _WIN32
		bgmvolume->value += dir * 1.0;
#else
		bgmvolume->value += dir * 0.1;
#endif
		if (bgmvolume->value < 0)
			bgmvolume->value = 0;
		if (bgmvolume->value > 1)
			bgmvolume->value = 1;
		Cvar_SetValue (bgmvolume, bgmvolume->value);
		break;
	case 7:	// sfx volume
		volume->value += dir * 0.1;
		if (volume->value < 0)
			volume->value = 0;
		if (volume->value > 1)
			volume->value = 1;
		Cvar_SetValue (volume, volume->value);
		break;

	case 8:	// always run
		if (cl_forwardspeed->value > 200)
		{
			Cvar_Set (cl_forwardspeed, "200");
			Cvar_Set (cl_backspeed, "200");
		}
		else
		{
			Cvar_Set (cl_forwardspeed, "400");
			Cvar_Set (cl_backspeed, "400");
		}
		break;

// 2001-12-16 M_LOOK cvar by Heffo/Maddes  start
/ *
	case 9:	// invert mouse
		Cvar_SetValue (m_pitch, -m_pitch->value);
		break;
* /
// 2001-12-16 M_LOOK cvar by Heffo/Maddes  end

	case 9:	// lookspring	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
		Cvar_SetValue (lookspring, !lookspring->value);
		break;

	case 10:	// lookstrafe	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
		Cvar_SetValue (lookstrafe, !lookstrafe->value);
		break;

// 2001-12-16 M_LOOK cvar by Heffo/Maddes  start
	case 11:	// mouselook
		Cvar_SetValue (m_look, !m_look->value);
		break;

	case 12:	// invert mouse
		Cvar_SetValue (m_pitch, -m_pitch->value);
		break;
// 2001-12-16 M_LOOK cvar by Heffo/Maddes  end

#ifdef _WIN32
	case 14:	// _windowed_mouse	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
		Cvar_SetValue (_windowed_mouse, !_windowed_mouse->value);
		break;
#endif
	}
}
*/
// 2002-01-31 New menu system by Maddes  end

void M_DrawSlider (int x, int y, float range)
{
	int	i;

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;
	M_DrawCharacter (x-8, y, 128);
	for (i=0 ; i<SLIDER_RANGE ; i++)
		M_DrawCharacter (x + i*8, y, 129);
	M_DrawCharacter (x+i*8, y, 130);
	M_DrawCharacter (x + (SLIDER_RANGE-1)*8 * range, y, 131);
}

void M_DrawCheckbox (int x, int y, int on)
{
#if 0
	if (on)
		M_DrawCharacter (x, y, 131);
	else
		M_DrawCharacter (x, y, 129);
#endif
	if (on)
		M_Print (x, y, "on");
	else
		M_Print (x, y, "off");
}

// 2002-01-31 New menu system by Maddes  end
/*
void M_Options_Draw (void)
{
	float		r;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	M_Print (16, 32, "    Customize controls");
	M_Print (16, 40, "         Go to console");
	M_Print (16, 48, "     Reset to defaults");

	M_Print (16, 56, "           Screen size");
	r = (scr_viewsize->value - 30) / (120 - 30);
	M_DrawSlider (220, 56, r);

	M_Print (16, 64, "            Brightness");
	r = (1.0 - v_gamma->value) / 0.5;
	M_DrawSlider (220, 64, r);

	M_Print (16, 72, "           Mouse Speed");
	r = (sensitivity->value - 1)/10;
	M_DrawSlider (220, 72, r);

	M_Print (16, 80, "       CD Music Volume");
	r = bgmvolume->value;
	M_DrawSlider (220, 80, r);

	M_Print (16, 88, "          Sound Volume");
	r = volume->value;
	M_DrawSlider (220, 88, r);

	M_Print (16, 96,  "            Always Run");
	M_DrawCheckbox (220, 96, cl_forwardspeed->value > 200);

// 2001-12-16 M_LOOK cvar by Heffo/Maddes  start
/ *
	M_Print (16, 104, "          Invert Mouse");
	M_DrawCheckbox (220, 104, m_pitch->value < 0);
* /

	M_Print (16, 104, "            Lookspring");
	M_DrawCheckbox (220, 104, lookspring->value);

	M_Print (16, 112, "            Lookstrafe");
	M_DrawCheckbox (220, 112, lookstrafe->value);

	M_Print (16, 120, "             Mouselook");
	M_DrawCheckbox (220, 120, m_look->value);

	M_Print (16, 128, "          Invert Mouse");
	M_DrawCheckbox (220, 128, m_pitch->value < 0);
// 2001-12-16 M_LOOK cvar by Heffo/Maddes  end

	if (vid_menudrawfn)
		M_Print (16, 136, "         Video Options");	// 2001-12-16 M_LOOK cvar by Heffo/Maddes

#ifdef _WIN32
	if (modestate == MS_WINDOWED)
	{
		M_Print (16, 144, "             Use Mouse");	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
		M_DrawCheckbox (220, 144, _windowed_mouse->value);	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
	}
#endif

// cursor
	M_DrawCharacter (200, 32 + options_cursor*8, 12+((int)(realtime*4)&1));
}
*/

int M_DrawFunction (menu_definition_t *menu_definition, int y)
{
	float	r;

	switch (menu_definition->funcno)
	{
		// Options menu
		case MENU_CUSTOMIZE_CONTROLS:
			M_Print (16, y, "    Customize controls");
			y += 8;
			break;

		case MENU_GO_TO_CONSOLE:
			M_Print (16, y, "         Go to console");
			y += 8;
			break;

		case MENU_LOAD_DEFAULT_CFG:
			M_Print (16, y, "     Reset to defaults");
			y += 8;
			break;

		case MENU_VIDEO_RESOLUTION:
			M_Print (16, y, "      Video Resolution");
			y += 8;
			break;

		// Control options menu
		case MENU_CONTROL_OPTIONS:
			M_Print (16, y, "       Control options");
			y += 8;
			break;

		case MENU_MOUSESPEED:
			M_Print (16, y, "           Mouse Speed");
			r = (sensitivity->value - 1)/10;
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_ALWAYS_RUN:
			M_Print (16, y,  "            Always Run");
			M_DrawCheckbox (220, y, cl_forwardspeed->value > 200);
			y += 8;
			break;

		case MENU_LOOKSPRING:
			M_Print (16, y, "            Lookspring");
			M_DrawCheckbox (220, y, lookspring->value);
			y += 8;
			break;

		case MENU_LOOKSTRAFE:
			M_Print (16, y, "            Lookstrafe");
			M_DrawCheckbox (220, y, lookstrafe->value);
			y += 8;
			break;

		case MENU_MOUSELOOK:
			M_Print (16, y, "             Mouselook");
			M_DrawCheckbox (220, y, m_look->value);
			y += 8;
			break;

		case MENU_INVERT_MOUSE:
			M_Print (16, y, "          Invert Mouse");
			M_DrawCheckbox (220, y, m_pitch->value < 0);
			y += 8;
			break;

#ifdef _WIN32
		case MENU_USE_MOUSE:
			M_Print (16, y, "             Use Mouse");
			M_DrawCheckbox (220, y, _windowed_mouse->value);
			y += 8;
			break;
#endif

		// Sound options menu
		case MENU_SOUND_OPTIONS:
			M_Print (16, y, "         Sound options");
			y += 8;
			break;

		case MENU_CD_VOLUME:
			M_Print (16, y, "       CD Music Volume");
			r = bgmvolume->value;
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_SOUND_VOLUME:
			M_Print (16, y, "          Sound Volume");
			r = volume->value;
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		// External data menu
		case MENU_EXTERNAL_DATA:
			M_Print (16, y, "         External data");
			y += 8;
			break;

		case MENU_EXTERNAL_ENT:
			M_Print (16, y, "    Entity data (.ENT)");
			M_DrawCheckbox (220, y, external_ent->value);
			y += 8;
			break;

		case MENU_EXTERNAL_VIS:
			M_Print (16, y, "Visibility data (.VIS)");
			M_DrawCheckbox (220, y, external_vis->value);
			y += 8;
			break;

#ifdef GLQUAKE
		case MENU_EXTERNAL_LIT:
			M_Print (16, y, "  Colored Light (.LIT)");
			M_DrawCheckbox (220, y, external_lit->value);
			y += 8;
			break;
#endif

		// Client options menu
		case MENU_CLIENT_OPTIONS:
			M_Print (16, y, "        Client options");
			y += 8;
			break;

		case MENU_CL_ENTITIES_MIN:
			M_Print (16, y, "      Min. normal ents");
			M_Print (220, y, va("%i", (int)cl_entities_min->value));
			y += 8;
			break;

		case MENU_CL_ENTITIES_TEMP_MIN:
			M_Print (16, y, "        Min. temp ents");
			M_Print (220, y, va("%i", (int)cl_entities_min_temp->value));
			y += 8;
			break;

		case MENU_CL_ENTITIES_STATIC_MIN:
			M_Print (16, y, "      Min. static ents");
			M_Print (220, y, va("%i", (int)cl_entities_min_static->value));
			y += 8;
			break;

		case MENU_CL_COMPATIBILITY:
			M_Print (16, y, "  Client Compatibility");
			M_DrawCheckbox (220, y, cl_compatibility->value);
			y += 8;
			M_Print (16, y, "  (for demo recording)");
			y += 8;
			break;

		// Server options menu
		case MENU_SERVER_OPTIONS:
			M_Print (16, y, "        Server options");
			y += 8;
			break;

		case MENU_SV_ENTITIES:
			M_Print (16, y, "           Normal ents");
			M_Print (220, y, va("%i", (int)sv_entities->value));
			y += 8;
			break;

		case MENU_SV_ENTITIES_TEMP:
			M_Print (16, y, "             Temp ents");
			M_Print (220, y, va("%i", (int)sv_entities_temp->value));
			y += 8;
			break;

		case MENU_SV_ENTITIES_STATIC:
			M_Print (16, y, "           Static ents");
			M_Print (220, y, va("%i", (int)sv_entities_static->value));
			y += 8;
			break;

		case MENU_SV_ENTITIES_COPY_TO_CL:
			M_Print (16, y, "   ent setup to client");
			y += 8;
			break;

		case MENU_PR_ZONE_MIN_STRINGS:
			M_Print (16, y, "     String zone in KB");
			M_Print (220, y, va("%i", (int)pr_zone_min_strings->value));
			y += 8;
			break;

		case MENU_BUILTIN_REMAP:
			M_Print (16, y, "     Builtin Remapping");
			M_DrawCheckbox (220, y, pr_builtin_remap->value);
			y += 8;
			break;

		case MENU_SV_COMPATIBILITY:
			M_Print (16, y, "  Server Compatibility");
			M_DrawCheckbox (220, y, sv_compatibility->value);
			y += 8;
			break;

		case MENU_NVS_ENABLE:
			M_Print (16, y, "   Enhanced SVC (BETA)");
			M_DrawCheckbox (220, y, nvs_svc_enable->value);
			y += 8;
			break;

		// Video options menu
		case MENU_VIDEO_OPTIONS:
			M_Print (16, y, "         Video options");
			y += 8;
			break;

		case MENU_SCREENSIZE:
			M_Print (16, y, "           Screen size");
			r = (scr_viewsize->value - 30) / (120 - 30);
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_BRIGHTNESS:
			M_Print (16, y, "            Brightness");
			r = (1.0 - v_gamma->value) / 0.5;
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_CON_ALPHA:
			M_Print (16, y, "  Console transparency");
			r = (1.0 - con_alpha->value);
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_CON_HEIGHT:
			M_Print (16, y, "        Console height");
			r = scr_conheight->value;
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_SHOW_FPS:
			M_Print (16, y, "              Show FPS");
			M_DrawCheckbox (220, y, cl_showfps->value);
			y += 8;
			break;

#ifdef GLQUAKE
		case MENU_GL_MAXDEPTH:
			M_Print (16, y, "       Max. draw depth");
			M_Print (220, y, va("%i", (int)gl_maxdepth->value));
			y += 8;
			break;
#endif

		default:
			M_Print (16, y, "      Unknown function");
			y += 8;
			break;
	}

	return y;
}

void M_Menu_DrawCheck(menu_definition_t *menu_definition)
{
	int	i;

	menu_first_index = 0;
	menu_last_index = 0;

	i = 1;
	while (menu_definition[i].funcno != 0)
	{
		// adjust ignore flag to current situation
		switch (menu_definition[i].funcno)
		{
#ifdef _WIN32
			case MENU_USE_MOUSE:		// only present in windowed mode on Win32
				if (modestate == MS_WINDOWED)
				{
					menu_definition[i].type = MENU_SELECTABLE;	// full usage
				}
				else
				{
					menu_definition[i].type = MENU_INVISIBLE;	// ignore completely
				}
				break;
#endif

			case MENU_VIDEO_RESOLUTION:
				if (vid_menudrawfn)
				{
					menu_definition[i].type = MENU_SELECTABLE;	// full usage
				}
				else
				{
					menu_definition[i].type = MENU_INVISIBLE;	// ignore completely
				}
				break;
		}

		// find first and last valid index
		if (menu_definition[i].type == MENU_SELECTABLE)	// selectable
		{
			if (menu_first_index == 0)
			{
				menu_first_index = i;
			}
			menu_last_index = i;
		}

		i++;
	}
}

void M_Menu_Draw (menu_definition_t *menu_definition, int *current_index)
{
	qpic_t	*p;
	int i, last_valid_index;
	int	y;

	// Adjust ignore flags to current situation
	M_Menu_DrawCheck(menu_definition);

	// check selection for cursor drawing
	if (*current_index < menu_first_index)
	{
		*current_index = menu_first_index;
	}
	else if (*current_index > menu_last_index)
	{
		*current_index = menu_last_index;
	}
	else
	{
		last_valid_index = 0;
		i = 1;
		while (menu_definition[i].funcno != 0)
		{
			// check cursor
			if (menu_definition[i].type == MENU_SELECTABLE)	// selectable
			{
				last_valid_index = i;
				if (*current_index == 0)	// nothing choosen, then use first valid selection
				{
					*current_index = i;
				}
			}
			else if (i == *current_index)	// incorrect selection, jump back to last valid selection
	 		{
				*current_index = last_valid_index;
			}

			i++;
		}
	}

	// Left side plaque and title
	if (menu_definition[0].type)
	{
		switch (menu_definition[0].type)
		{
			case MENU_OPTIONS:
				M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
				p = Draw_CachePic ("gfx/p_option.lmp");
				M_DrawPic ( (320-p->width)/2, 4, p);
				break;
		}
	}

	// display menu with cursor
	y = 32;
	i = 1;
	while (menu_definition[i].funcno != 0)
	{
		if (menu_definition[i].type != MENU_INVISIBLE)	// drawable menu point
		{
			if (i == *current_index)	// draw cursor
	 		{
				M_DrawCharacter (200, y, 12+((int)(realtime*4)&1));
			}

			y = M_DrawFunction(&menu_definition[i], y);
		}

		i++;
	}
}
// 2002-01-31 New menu system by Maddes  end

// 2002-01-31 New menu system by Maddes  start
/*
void M_Options_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		switch (options_cursor)
		{
		case 0:
			M_Menu_Keys_f ();
			break;
		case 1:
			m_state = m_none;
			Con_ToggleConsole_f ();
			break;
		case 2:
			Cbuf_AddText ("exec default.cfg\n");
			break;
		case 13:	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
			M_Menu_Video_f ();
			break;
		default:
			M_AdjustSliders (1);
			break;
		}
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor++;
		if (options_cursor >= OPTIONS_ITEMS)
			options_cursor = 0;
		break;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_AdjustSliders (1);
		break;
	}

	if (options_cursor == 13 && vid_menudrawfn == NULL)	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
	{
		if (k == K_UPARROW)
			options_cursor = 12;	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
		else
			options_cursor = 0;
	}

#ifdef _WIN32
	if ((options_cursor == 14) && (modestate != MS_WINDOWED))	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
	{
		if (k == K_UPARROW)
			options_cursor = 13;	// 2001-12-16 M_LOOK cvar by Heffo/Maddes
		else
			options_cursor = 0;
	}
#endif
}
*/

void M_ExecFunction (menu_definition_t *menu_definition, int key)
{
	qboolean	m_changesound;
	int dir;

	// determine direction for sliders
	dir = 0;
	switch (key)
	{
		case K_RIGHTARROW:
		case K_ENTER:
			dir = 1;
			break;

		case K_LEFTARROW:
			dir = -1;
			break;
	}

	// execute function
	m_changesound = false;
	switch (menu_definition->funcno)
	{
		case MENU_MAIN:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = NULL;
				M_Menu_Main_f ();
			}
			break;

		// Options menu
		case MENU_OPTIONS:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = m_menu_options;
				current_cursor = &options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_CUSTOMIZE_CONTROLS:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = NULL;
				M_Menu_Keys_f ();
			}
			break;

		case MENU_GO_TO_CONSOLE:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = NULL;
				m_state = m_none;
				Con_ToggleConsole_f ();
			}
			break;

		case MENU_LOAD_DEFAULT_CFG:
			if (key == K_ENTER)
			{
				Cbuf_AddText ("exec default.cfg\n");
				m_changesound = true;
			}
			break;

		case MENU_VIDEO_RESOLUTION:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = NULL;
				M_Menu_Video_f ();
			}
			break;

		// Control options menu
		case MENU_CONTROL_OPTIONS:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = m_menu_control_options;
				current_cursor = &control_options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_MOUSESPEED:
			if (dir != 0)
			{
				sensitivity->value += dir * 0.5;
				if (sensitivity->value < 1)
					sensitivity->value = 1;
				if (sensitivity->value > 11)
					sensitivity->value = 11;
				Cvar_SetValue (sensitivity, sensitivity->value);
				m_changesound = true;
			}
			break;

		case MENU_ALWAYS_RUN:
			if (dir != 0)
			{
				if (cl_forwardspeed->value > 200)
				{
					Cvar_Set (cl_forwardspeed, "200");
					Cvar_Set (cl_backspeed, "200");
				}
				else
				{
					Cvar_Set (cl_forwardspeed, "400");
					Cvar_Set (cl_backspeed, "400");
				}
				m_changesound = true;
			}
			break;

		case MENU_LOOKSPRING:
			if (dir != 0)
			{
				Cvar_SetValue (lookspring, !lookspring->value);
				m_changesound = true;
			}
			break;

		case MENU_LOOKSTRAFE:
			if (dir != 0)
			{
				Cvar_SetValue (lookstrafe, !lookstrafe->value);
				m_changesound = true;
			}
			break;

		case MENU_MOUSELOOK:
			if (dir != 0)
			{
				Cvar_SetValue (m_look, !m_look->value);
				m_changesound = true;
			}
			break;

		case MENU_INVERT_MOUSE:
			if (dir != 0)
			{
				Cvar_SetValue (m_pitch, -m_pitch->value);
				m_changesound = true;
			}
			break;

#ifdef _WIN32
		case MENU_USE_MOUSE:
			if (dir != 0)
			{
				Cvar_SetValue (_windowed_mouse, !_windowed_mouse->value);
				m_changesound = true;
			}
			break;
#endif

		// Sound options menu
		case MENU_SOUND_OPTIONS:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = m_menu_sound_options;
				current_cursor = &sound_options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_CD_VOLUME:
			if (dir != 0)
			{
#ifdef _WIN32
				bgmvolume->value += dir * 1.0;
#else
				bgmvolume->value += dir * 0.1;
#endif
				if (bgmvolume->value < 0)
					bgmvolume->value = 0;
				if (bgmvolume->value > 1)
					bgmvolume->value = 1;
				Cvar_SetValue (bgmvolume, bgmvolume->value);
				m_changesound = true;
			}
			break;

		case MENU_SOUND_VOLUME:
			if (dir != 0)
			{
				volume->value += dir * 0.1;
				if (volume->value < 0)
					volume->value = 0;
				if (volume->value > 1)
					volume->value = 1;
				Cvar_SetValue (volume, volume->value);
				m_changesound = true;
			}
			break;

		// External data menu
		case MENU_EXTERNAL_DATA:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = m_menu_external_data;
				current_cursor = &external_data_cursor;
				m_entersound = true;
			}
			break;

		case MENU_EXTERNAL_ENT:
			if (dir != 0)
			{
				Cvar_SetValue (external_ent, !external_ent->value);
				m_changesound = true;
			}
			break;

		case MENU_EXTERNAL_VIS:
			if (dir != 0)
			{
				Cvar_SetValue (external_vis, !external_vis->value);
				m_changesound = true;
			}
			break;

#ifdef GLQUAKE
		case MENU_EXTERNAL_LIT:
			if (dir != 0)
			{
				Cvar_SetValue (external_lit, !external_lit->value);
				m_changesound = true;
			}
			break;
#endif

		// Client options menu
		case MENU_CLIENT_OPTIONS:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = m_menu_client_options;
				current_cursor = &client_options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_CL_ENTITIES_MIN:
			if (dir != 0)
			{
				Cvar_SetValue (cl_entities_min, cl_entities_min->value + dir * 50);
				m_changesound = true;
			}
			break;

		case MENU_CL_ENTITIES_TEMP_MIN:
			if (dir != 0)
			{
				Cvar_SetValue (cl_entities_min_temp, cl_entities_min_temp->value + dir * 32);
				m_changesound = true;
			}
			break;

		case MENU_CL_ENTITIES_STATIC_MIN:
			if (dir != 0)
			{
				Cvar_SetValue (cl_entities_min_static, cl_entities_min_static->value + dir * 32);
				m_changesound = true;
			}
			break;

		case MENU_CL_COMPATIBILITY:
			if (dir != 0)
			{
				Cvar_SetValue (cl_compatibility, !cl_compatibility->value);
				m_changesound = true;
			}
			break;

		// Server options menu
		case MENU_SERVER_OPTIONS:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = m_menu_server_options;
				current_cursor = &server_options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_SV_ENTITIES:
			if (dir != 0)
			{
				Cvar_SetValue (sv_entities, sv_entities->value + dir * 50);
				m_changesound = true;
			}
			break;

		case MENU_SV_ENTITIES_TEMP:
			if (dir != 0)
			{
				Cvar_SetValue (sv_entities_temp, sv_entities_temp->value + dir * 32);
				m_changesound = true;
			}
			break;

		case MENU_SV_ENTITIES_STATIC:
			if (dir != 0)
			{
				Cvar_SetValue (sv_entities_static, sv_entities_static->value + dir * 32);
				m_changesound = true;
			}
			break;

		case MENU_SV_ENTITIES_COPY_TO_CL:
			if (key == K_ENTER)
			{
				Cvar_SetValue (cl_entities_min, sv_entities->value);
				Cvar_SetValue (cl_entities_min_temp, sv_entities_temp->value);
				Cvar_SetValue (cl_entities_min_static, sv_entities_static->value);
				m_changesound = true;
			}
			break;

		case MENU_PR_ZONE_MIN_STRINGS:
			if (dir != 0)
			{
				Cvar_SetValue (pr_zone_min_strings, pr_zone_min_strings->value + dir * 32);
				m_changesound = true;
			}
			break;

		case MENU_BUILTIN_REMAP:
			if (dir != 0)
			{
				Cvar_SetValue (pr_builtin_remap, !pr_builtin_remap->value);
				m_changesound = true;
			}
			break;

		case MENU_SV_COMPATIBILITY:
			if (dir != 0)
			{
				Cvar_SetValue (sv_compatibility, !sv_compatibility->value);
				m_changesound = true;
			}
			break;

		case MENU_NVS_ENABLE:
			if (dir != 0)
			{
				Cvar_SetValue (nvs_svc_enable, !nvs_svc_enable->value);
				m_changesound = true;
			}
			break;

		// Video options menu
		case MENU_VIDEO_OPTIONS:
			if (key == K_ENTER || key == K_ESCAPE)
			{
				current_menu = m_menu_video_options;
				current_cursor = &video_options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_SCREENSIZE:
			if (dir != 0)
			{
				scr_viewsize->value += dir * 10;
				if (scr_viewsize->value < 30)
					scr_viewsize->value = 30;
				if (scr_viewsize->value > 120)
					scr_viewsize->value = 120;
				Cvar_SetValue (scr_viewsize, scr_viewsize->value);
				m_changesound = true;
			}
			break;

		case MENU_BRIGHTNESS:
			if (dir != 0)
			{
				v_gamma->value -= dir * 0.05;
				if (v_gamma->value < 0.5)
					v_gamma->value = 0.5;
				if (v_gamma->value > 1)
					v_gamma->value = 1;
				Cvar_SetValue (v_gamma, v_gamma->value);
				m_changesound = true;
			}
			break;

		case MENU_CON_ALPHA:
			if (dir != 0)
			{
				Cvar_SetValue (con_alpha, con_alpha->value - dir * 0.1);
				m_changesound = true;
			}
			break;

		case MENU_CON_HEIGHT:
			if (dir != 0)
			{
				Cvar_SetValue (scr_conheight, scr_conheight->value + dir * 0.1);
				m_changesound = true;
			}
			break;

		case MENU_SHOW_FPS:
			if (dir != 0)
			{
				Cvar_SetValue (cl_showfps, !cl_showfps->value);
				m_changesound = true;
			}
			break;

#ifdef GLQUAKE
		case MENU_GL_MAXDEPTH:
			if (dir != 0)
			{
				Cvar_SetValue (gl_maxdepth, gl_maxdepth->value + dir * 256);
				m_changesound = true;
			}
			break;
#endif
	}

	if (m_changesound)
	{
		S_LocalSound ("misc/menu3.wav");
	}
}

void M_Menu_Key (menu_definition_t *menu_definition, int *current_index, int key)
{
	qboolean	m_movesound;

	m_movesound = false;
	switch (key)
	{
		case K_UPARROW:
		case K_DOWNARROW:
			if (menu_first_index != 0
			&&  menu_first_index != menu_last_index)
			{
				int dir;
				int old_index;

				if (key == K_UPARROW)
				{
					dir = -1;
				}
				else
				{
					dir = 1;
				}

				old_index = *current_index;

				do
				{
					*current_index += dir;
					if (*current_index < menu_first_index)
					{
						*current_index = menu_last_index;
					}
					else if (*current_index > menu_last_index)
					{
						*current_index = menu_first_index;
					}
				}
				while (menu_definition[*current_index].type != MENU_SELECTABLE);

				if (*current_index != old_index)
				{
					m_movesound = true;
				}
			}
			break;

		case K_ESCAPE:
			M_ExecFunction(&menu_definition[0], key);
			break;

		default:
			if (*current_index > 0)
			{
				M_ExecFunction(&menu_definition[*current_index], key);
			}
	}

	if (m_movesound)
	{
		S_LocalSound ("misc/menu1.wav");
	}
}
// 2002-01-31 New menu system by Maddes  start

//=============================================================================
/* KEYS MENU */

char *bindnames[][2] =
{
{"+attack", 		"attack"},
{"impulse 10", 		"change weapon"},
{"+jump", 			"jump / swim up"},
{"+forward", 		"walk forward"},
{"+back", 			"backpedal"},
{"+left", 			"turn left"},
{"+right", 			"turn right"},
{"+speed", 			"run"},
{"+moveleft", 		"step left"},
{"+moveright", 		"step right"},
{"+strafe", 		"sidestep"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"centerview", 		"center view"},
{"+mlook", 			"mouse look"},
{"+klook", 			"keyboard look"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"}
};

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))

int		keys_cursor;
int		bind_grab;

void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}


void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}


void M_Keys_Draw (void)
{
	int		i;	//, l	// 2001-12-10 Reduced compiler warnings by Jeff Ford
	int		keys[2];
	char	*name;
	int		x, y;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	if (bind_grab)
		M_Print (12, 32, "Press a key or button for this action");
	else
		M_Print (18, 32, "Enter to change, backspace to clear");

// search for known bindings
	for (i=0 ; i<NUMCOMMANDS ; i++)
	{
		y = 48 + 8*i;

		M_Print (16, y, bindnames[i][1]);

//		l = strlen (bindnames[i][0]);	// 2001-12-10 Reduced compiler warnings by Jeff Ford

		M_FindKeysForCommand (bindnames[i][0], keys);

		if (keys[0] == -1)
		{
			M_Print (140, y, "???");
		}
		else
		{
			name = Key_KeynumToString (keys[0]);
			M_Print (140, y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				M_Print (140 + x + 8, y, "or");
				M_Print (140 + x + 32, y, Key_KeynumToString (keys[1]));
			}
		}
	}

	if (bind_grab)
		M_DrawCharacter (130, 48 + keys_cursor*8, '=');
	else
		M_DrawCharacter (130, 48 + keys_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[2];

	if (bind_grab)
	{	// defining a key
		S_LocalSound ("misc/menu1.wav");
		if (k == K_ESCAPE)
		{
			bind_grab = false;
		}
		else if (k != '`')
		{
			sprintf (cmd, "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor][0]);
			Cbuf_InsertText (cmd);
		}

		bind_grab = false;
		return;
	}

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor++;
		if (keys_cursor >= NUMCOMMANDS)
			keys_cursor = 0;
		break;

	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("misc/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		bind_grab = true;
		break;

	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
		S_LocalSound ("misc/menu2.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}
}

//=============================================================================
/* VIDEO MENU */

void M_Menu_Video_f (void)
{
	key_dest = key_menu;
	m_state = m_video;
	m_entersound = true;
}


void M_Video_Draw (void)
{
	(*vid_menudrawfn) ();
}


void M_Video_Key (int key)
{
	(*vid_menukeyfn) (key);
}

//=============================================================================
/* HELP MENU */

int		help_page;
#define	NUM_HELP_PAGES	6


void M_Menu_Help_f (void)
{
	key_dest = key_menu;
	m_state = m_help;
	m_entersound = true;
	help_page = 0;
}



void M_Help_Draw (void)
{
	M_DrawPic (0, 0, Draw_CachePic ( va("gfx/help%i.lmp", help_page)) );
}


void M_Help_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (++help_page >= NUM_HELP_PAGES)
			help_page = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
			help_page = NUM_HELP_PAGES-1;
		break;
	}

}

//=============================================================================
/* QUIT MENU */

int		msgNumber;
int		m_quit_prevstate;
qboolean	wasInMenus;

#ifndef	_WIN32
char *quitMessage [] =
{
/* .........1.........2.... */
  "  Are you gonna quit    ",
  "  this game just like   ",
  "   everything else?     ",
  "                        ",

  " Milord, methinks that  ",
  "   thou art a lowly     ",
  " quitter. Is this true? ",
  "                        ",

  " Do I need to bust your ",
  "  face open for trying  ",
  "        to quit?        ",
  "                        ",

  " Man, I oughta smack you",
  "   for trying to quit!  ",
  "     Press Y to get     ",
  "      smacked out.      ",

  " Press Y to quit like a ",
  "   big loser in life.   ",
  "  Press N to stay proud ",
  "    and successful!     ",

  "   If you press Y to    ",
  "  quit, I will summon   ",
  "  Satan all over your   ",
  "      hard drive!       ",

  "  Um, Asmodeus dislikes ",
  " his children trying to ",
  " quit. Press Y to return",
  "   to your Tinkertoys.  ",

  "  If you quit now, I'll ",
  "  throw a blanket-party ",
  "   for you next time!   ",
  "                        "
};
#endif

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = rand()&7;
}


void M_Quit_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case 'Y':
	case 'y':
		key_dest = key_console;
		Host_Quit_f ();
		break;

	default:
		break;
	}

}


void M_Quit_Draw (void)
{
	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

#ifdef _WIN32
	M_DrawTextBox (0, 0, 38, 23);
	M_PrintWhite (16, 12,  "  Quake version 1.09 by id Software\n\n");
	M_PrintWhite (16, 28,  "Programming        Art \n");
	M_Print (16, 36,  " John Carmack       Adrian Carmack\n");
	M_Print (16, 44,  " Michael Abrash     Kevin Cloud\n");
	M_Print (16, 52,  " John Cash          Paul Steed\n");
	M_Print (16, 60,  " Dave 'Zoid' Kirsch\n");
	M_PrintWhite (16, 68,  "Design             Biz\n");
	M_Print (16, 76,  " John Romero        Jay Wilbur\n");
	M_Print (16, 84,  " Sandy Petersen     Mike Wilson\n");
	M_Print (16, 92,  " American McGee     Donna Jackson\n");
	M_Print (16, 100,  " Tim Willits        Todd Hollenshead\n");
	M_PrintWhite (16, 108, "Support            Projects\n");
	M_Print (16, 116, " Barrett Alexander  Shawn Green\n");
	M_PrintWhite (16, 124, "Sound Effects\n");
	M_Print (16, 132, " Trent Reznor and Nine Inch Nails\n\n");
	M_PrintWhite (16, 140, "Quake is a trademark of Id Software,\n");
	M_PrintWhite (16, 148, "inc., (c)1996 Id Software, inc. All\n");
	M_PrintWhite (16, 156, "rights reserved. NIN logo is a\n");
	M_PrintWhite (16, 164, "registered trademark licensed to\n");
	M_PrintWhite (16, 172, "Nothing Interactive, Inc. All rights\n");
	M_PrintWhite (16, 180, "reserved. Press y to exit\n");
#else
	M_DrawTextBox (56, 76, 24, 4);
	M_Print (64, 84,  quitMessage[msgNumber*4+0]);
	M_Print (64, 92,  quitMessage[msgNumber*4+1]);
	M_Print (64, 100, quitMessage[msgNumber*4+2]);
	M_Print (64, 108, quitMessage[msgNumber*4+3]);
#endif
}

//=============================================================================

/* SERIAL CONFIG MENU */

int		serialConfig_cursor;
int		serialConfig_cursor_table[] = {48, 64, 80, 96, 112, 132};
#define	NUM_SERIALCONFIG_CMDS	6

static int ISA_uarts[]	= {0x3f8,0x2f8,0x3e8,0x2e8};
static int ISA_IRQs[]	= {4,3,4,3};
int serialConfig_baudrate[] = {9600,14400,19200,28800,38400,57600};

int		serialConfig_comport;
int		serialConfig_irq ;
int		serialConfig_baud;
char	serialConfig_phone[16];

void M_Menu_SerialConfig_f (void)
{
	int		n;
	int		port;
	int		baudrate;
	qboolean	useModem;

	key_dest = key_menu;
	m_state = m_serialconfig;
	m_entersound = true;
	if (JoiningGame && SerialConfig)
		serialConfig_cursor = 4;
	else
		serialConfig_cursor = 5;

	(*GetComPortConfig) (0, &port, &serialConfig_irq, &baudrate, &useModem);

	// map uart's port to COMx
	for (n = 0; n < 4; n++)
		if (ISA_uarts[n] == port)
			break;
	if (n == 4)
	{
		n = 0;
		serialConfig_irq = 4;
	}
	serialConfig_comport = n + 1;

	// map baudrate to index
	for (n = 0; n < 6; n++)
		if (serialConfig_baudrate[n] == baudrate)
			break;
	if (n == 6)
		n = 5;
	serialConfig_baud = n;

	m_return_onerror = false;
	m_return_reason[0] = 0;
}


void M_SerialConfig_Draw (void)
{
	qpic_t	*p;
	int		basex;
	char	*startJoin;
	char	*directModem;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320-p->width)/2;
	M_DrawPic (basex, 4, p);

	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (SerialConfig)
		directModem = "Modem";
	else
		directModem = "Direct Connect";
	M_Print (basex, 32, va ("%s - %s", startJoin, directModem));
	basex += 8;

	M_Print (basex, serialConfig_cursor_table[0], "Port");
	M_DrawTextBox (160, 40, 4, 1);
	M_Print (168, serialConfig_cursor_table[0], va("COM%u", serialConfig_comport));

	M_Print (basex, serialConfig_cursor_table[1], "IRQ");
	M_DrawTextBox (160, serialConfig_cursor_table[1]-8, 1, 1);
	M_Print (168, serialConfig_cursor_table[1], va("%u", serialConfig_irq));

	M_Print (basex, serialConfig_cursor_table[2], "Baud");
	M_DrawTextBox (160, serialConfig_cursor_table[2]-8, 5, 1);
	M_Print (168, serialConfig_cursor_table[2], va("%u", serialConfig_baudrate[serialConfig_baud]));

	if (SerialConfig)
	{
		M_Print (basex, serialConfig_cursor_table[3], "Modem Setup...");
		if (JoiningGame)
		{
			M_Print (basex, serialConfig_cursor_table[4], "Phone number");
			M_DrawTextBox (160, serialConfig_cursor_table[4]-8, 16, 1);
			M_Print (168, serialConfig_cursor_table[4], serialConfig_phone);
		}
	}

	if (JoiningGame)
	{
		M_DrawTextBox (basex, serialConfig_cursor_table[5]-8, 7, 1);
		M_Print (basex+8, serialConfig_cursor_table[5], "Connect");
	}
	else
	{
		M_DrawTextBox (basex, serialConfig_cursor_table[5]-8, 2, 1);
		M_Print (basex+8, serialConfig_cursor_table[5], "OK");
	}

	M_DrawCharacter (basex-8, serialConfig_cursor_table [serialConfig_cursor], 12+((int)(realtime*4)&1));

	if (serialConfig_cursor == 4)
		M_DrawCharacter (168 + 8*strlen(serialConfig_phone), serialConfig_cursor_table [serialConfig_cursor], 10+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (basex, 148, m_return_reason);
}


void M_SerialConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		serialConfig_cursor--;
		if (serialConfig_cursor < 0)
			serialConfig_cursor = NUM_SERIALCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		serialConfig_cursor++;
		if (serialConfig_cursor >= NUM_SERIALCONFIG_CMDS)
			serialConfig_cursor = 0;
		break;

	case K_LEFTARROW:
		if (serialConfig_cursor > 2)
			break;
		S_LocalSound ("misc/menu3.wav");

		if (serialConfig_cursor == 0)
		{
			serialConfig_comport--;
			if (serialConfig_comport == 0)
				serialConfig_comport = 4;
			serialConfig_irq = ISA_IRQs[serialConfig_comport-1];
		}

		if (serialConfig_cursor == 1)
		{
			serialConfig_irq--;
			if (serialConfig_irq == 6)
				serialConfig_irq = 5;
			if (serialConfig_irq == 1)
				serialConfig_irq = 7;
		}

		if (serialConfig_cursor == 2)
		{
			serialConfig_baud--;
			if (serialConfig_baud < 0)
				serialConfig_baud = 5;
		}

		break;

	case K_RIGHTARROW:
		if (serialConfig_cursor > 2)
			break;
forward:
		S_LocalSound ("misc/menu3.wav");

		if (serialConfig_cursor == 0)
		{
			serialConfig_comport++;
			if (serialConfig_comport > 4)
				serialConfig_comport = 1;
			serialConfig_irq = ISA_IRQs[serialConfig_comport-1];
		}

		if (serialConfig_cursor == 1)
		{
			serialConfig_irq++;
			if (serialConfig_irq == 6)
				serialConfig_irq = 7;
			if (serialConfig_irq == 8)
				serialConfig_irq = 2;
		}

		if (serialConfig_cursor == 2)
		{
			serialConfig_baud++;
			if (serialConfig_baud > 5)
				serialConfig_baud = 0;
		}

		break;

	case K_ENTER:
		if (serialConfig_cursor < 3)
			goto forward;

		m_entersound = true;

		if (serialConfig_cursor == 3)
		{
			(*SetComPortConfig) (0, ISA_uarts[serialConfig_comport-1], serialConfig_irq, serialConfig_baudrate[serialConfig_baud], SerialConfig);

			M_Menu_ModemConfig_f ();
			break;
		}

		if (serialConfig_cursor == 4)
		{
			serialConfig_cursor = 5;
			break;
		}

		// serialConfig_cursor == 5 (OK/CONNECT)
		(*SetComPortConfig) (0, ISA_uarts[serialConfig_comport-1], serialConfig_irq, serialConfig_baudrate[serialConfig_baud], SerialConfig);

		M_ConfigureNetSubsystem ();

		if (StartingGame)
		{
			M_Menu_GameOptions_f ();
			break;
		}

		m_return_state = m_state;
		m_return_onerror = true;
		key_dest = key_game;
		m_state = m_none;

		if (SerialConfig)
			Cbuf_AddText (va ("connect \"%s\"\n", serialConfig_phone));
		else
			Cbuf_AddText ("connect\n");
		break;

	case K_BACKSPACE:
		if (serialConfig_cursor == 4)
		{
			if (strlen(serialConfig_phone))
				serialConfig_phone[strlen(serialConfig_phone)-1] = 0;
		}
		break;

	default:
		if (key < 32 || key > 127)
			break;
		if (serialConfig_cursor == 4)
		{
			l = strlen(serialConfig_phone);
			if (l < 15)
			{
				serialConfig_phone[l+1] = 0;
				serialConfig_phone[l] = key;
			}
		}
	}

	if (DirectConfig && (serialConfig_cursor == 3 || serialConfig_cursor == 4))
	{	// 1999-12-24 explicit brackets by Maddes
		if (key == K_UPARROW)
			serialConfig_cursor = 2;
		else
			serialConfig_cursor = 5;
	}	// 1999-12-24 explicit brackets by Maddes

	if (SerialConfig && StartingGame && serialConfig_cursor == 4)
	{	// 1999-12-24 explicit brackets by Maddes
		if (key == K_UPARROW)
			serialConfig_cursor = 3;
		else
			serialConfig_cursor = 5;
	}	// 1999-12-24 explicit brackets by Maddes
}

//=============================================================================
/* MODEM CONFIG MENU */

int		modemConfig_cursor;
int		modemConfig_cursor_table [] = {40, 56, 88, 120, 156};
#define NUM_MODEMCONFIG_CMDS	5

char	modemConfig_dialing;
char	modemConfig_clear [16];
char	modemConfig_init [32];
char	modemConfig_hangup [16];

void M_Menu_ModemConfig_f (void)
{
	key_dest = key_menu;
	m_state = m_modemconfig;
	m_entersound = true;
	(*GetModemConfig) (0, &modemConfig_dialing, modemConfig_clear, modemConfig_init, modemConfig_hangup);
}


void M_ModemConfig_Draw (void)
{
	qpic_t	*p;
	int		basex;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320-p->width)/2;
	M_DrawPic (basex, 4, p);
	basex += 8;

	if (modemConfig_dialing == 'P')
		M_Print (basex, modemConfig_cursor_table[0], "Pulse Dialing");
	else
		M_Print (basex, modemConfig_cursor_table[0], "Touch Tone Dialing");

	M_Print (basex, modemConfig_cursor_table[1], "Clear");
	M_DrawTextBox (basex, modemConfig_cursor_table[1]+4, 16, 1);
	M_Print (basex+8, modemConfig_cursor_table[1]+12, modemConfig_clear);
	if (modemConfig_cursor == 1)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_clear), modemConfig_cursor_table[1]+12, 10+((int)(realtime*4)&1));

	M_Print (basex, modemConfig_cursor_table[2], "Init");
	M_DrawTextBox (basex, modemConfig_cursor_table[2]+4, 30, 1);
	M_Print (basex+8, modemConfig_cursor_table[2]+12, modemConfig_init);
	if (modemConfig_cursor == 2)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_init), modemConfig_cursor_table[2]+12, 10+((int)(realtime*4)&1));

	M_Print (basex, modemConfig_cursor_table[3], "Hangup");
	M_DrawTextBox (basex, modemConfig_cursor_table[3]+4, 16, 1);
	M_Print (basex+8, modemConfig_cursor_table[3]+12, modemConfig_hangup);
	if (modemConfig_cursor == 3)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_hangup), modemConfig_cursor_table[3]+12, 10+((int)(realtime*4)&1));

	M_DrawTextBox (basex, modemConfig_cursor_table[4]-8, 2, 1);
	M_Print (basex+8, modemConfig_cursor_table[4], "OK");

	M_DrawCharacter (basex-8, modemConfig_cursor_table [modemConfig_cursor], 12+((int)(realtime*4)&1));
}


void M_ModemConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
		M_Menu_SerialConfig_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		modemConfig_cursor--;
		if (modemConfig_cursor < 0)
			modemConfig_cursor = NUM_MODEMCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		modemConfig_cursor++;
		if (modemConfig_cursor >= NUM_MODEMCONFIG_CMDS)
			modemConfig_cursor = 0;
		break;

	case K_LEFTARROW:
	case K_RIGHTARROW:
		if (modemConfig_cursor == 0)
		{
			if (modemConfig_dialing == 'P')
				modemConfig_dialing = 'T';
			else
				modemConfig_dialing = 'P';
			S_LocalSound ("misc/menu1.wav");
		}
		break;

	case K_ENTER:
		if (modemConfig_cursor == 0)
		{
			if (modemConfig_dialing == 'P')
				modemConfig_dialing = 'T';
			else
				modemConfig_dialing = 'P';
			m_entersound = true;
		}

		if (modemConfig_cursor == 4)
		{
			(*SetModemConfig) (0, va ("%c", modemConfig_dialing), modemConfig_clear, modemConfig_init, modemConfig_hangup);
			m_entersound = true;
			M_Menu_SerialConfig_f ();
		}
		break;

	case K_BACKSPACE:
		if (modemConfig_cursor == 1)
		{
			if (strlen(modemConfig_clear))
				modemConfig_clear[strlen(modemConfig_clear)-1] = 0;
		}

		if (modemConfig_cursor == 2)
		{
			if (strlen(modemConfig_init))
				modemConfig_init[strlen(modemConfig_init)-1] = 0;
		}

		if (modemConfig_cursor == 3)
		{
			if (strlen(modemConfig_hangup))
				modemConfig_hangup[strlen(modemConfig_hangup)-1] = 0;
		}
		break;

	default:
		if (key < 32 || key > 127)
			break;

		if (modemConfig_cursor == 1)
		{
			l = strlen(modemConfig_clear);
			if (l < 15)
			{
				modemConfig_clear[l+1] = 0;
				modemConfig_clear[l] = key;
			}
		}

		if (modemConfig_cursor == 2)
		{
			l = strlen(modemConfig_init);
			if (l < 29)
			{
				modemConfig_init[l+1] = 0;
				modemConfig_init[l] = key;
			}
		}

		if (modemConfig_cursor == 3)
		{
			l = strlen(modemConfig_hangup);
			if (l < 15)
			{
				modemConfig_hangup[l+1] = 0;
				modemConfig_hangup[l] = key;
			}
		}
	}
}

//=============================================================================
/* LAN CONFIG MENU */

int		lanConfig_cursor = -1;
int		lanConfig_cursor_table [] = {72, 92, 124};
#define NUM_LANCONFIG_CMDS	3

int 	lanConfig_port;
char	lanConfig_portname[6];
char	lanConfig_joinname[22];

void M_Menu_LanConfig_f (void)
{
	key_dest = key_menu;
	m_state = m_lanconfig;
	m_entersound = true;
	if (lanConfig_cursor == -1)
	{
		if (JoiningGame && TCPIPConfig)
			lanConfig_cursor = 2;
		else
			lanConfig_cursor = 1;
	}
	if (StartingGame && lanConfig_cursor == 2)
		lanConfig_cursor = 1;
	lanConfig_port = DEFAULTnet_hostport;
	sprintf(lanConfig_portname, "%u", lanConfig_port);

	m_return_onerror = false;
	m_return_reason[0] = 0;
}


void M_LanConfig_Draw (void)
{
	qpic_t	*p;
	int		basex;
	char	*startJoin;
	char	*protocol;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320-p->width)/2;
	M_DrawPic (basex, 4, p);

	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (IPXConfig)
		protocol = "IPX";
	else
		protocol = "TCP/IP";
	M_Print (basex, 32, va ("%s - %s", startJoin, protocol));
	basex += 8;

	M_Print (basex, 52, "Address:");
	if (IPXConfig)
		M_Print (basex+9*8, 52, my_ipx_address);
	else
		M_Print (basex+9*8, 52, my_tcpip_address);

	M_Print (basex, lanConfig_cursor_table[0], "Port");
	M_DrawTextBox (basex+8*8, lanConfig_cursor_table[0]-8, 6, 1);
	M_Print (basex+9*8, lanConfig_cursor_table[0], lanConfig_portname);

	if (JoiningGame)
	{
		M_Print (basex, lanConfig_cursor_table[1], "Search for local games...");
		M_Print (basex, 108, "Join game at:");
		M_DrawTextBox (basex+8, lanConfig_cursor_table[2]-8, 22, 1);
		M_Print (basex+16, lanConfig_cursor_table[2], lanConfig_joinname);
	}
	else
	{
		M_DrawTextBox (basex, lanConfig_cursor_table[1]-8, 2, 1);
		M_Print (basex+8, lanConfig_cursor_table[1], "OK");
	}

	M_DrawCharacter (basex-8, lanConfig_cursor_table [lanConfig_cursor], 12+((int)(realtime*4)&1));

	if (lanConfig_cursor == 0)
		M_DrawCharacter (basex+9*8 + 8*strlen(lanConfig_portname), lanConfig_cursor_table [0], 10+((int)(realtime*4)&1));

	if (lanConfig_cursor == 2)
		M_DrawCharacter (basex+16 + 8*strlen(lanConfig_joinname), lanConfig_cursor_table [2], 10+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (basex, 148, m_return_reason);
}


void M_LanConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		lanConfig_cursor--;
		if (lanConfig_cursor < 0)
			lanConfig_cursor = NUM_LANCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		lanConfig_cursor++;
		if (lanConfig_cursor >= NUM_LANCONFIG_CMDS)
			lanConfig_cursor = 0;
		break;

	case K_ENTER:
		if (lanConfig_cursor == 0)
			break;

		m_entersound = true;

		M_ConfigureNetSubsystem ();

		if (lanConfig_cursor == 1)
		{
			if (StartingGame)
			{
				M_Menu_GameOptions_f ();
				break;
			}
			M_Menu_Search_f();
			break;
		}

		if (lanConfig_cursor == 2)
		{
			m_return_state = m_state;
			m_return_onerror = true;
			key_dest = key_game;
			m_state = m_none;
			Cbuf_AddText ( va ("connect \"%s\"\n", lanConfig_joinname) );
			break;
		}

		break;

	case K_BACKSPACE:
		if (lanConfig_cursor == 0)
		{
			if (strlen(lanConfig_portname))
				lanConfig_portname[strlen(lanConfig_portname)-1] = 0;
		}

		if (lanConfig_cursor == 2)
		{
			if (strlen(lanConfig_joinname))
				lanConfig_joinname[strlen(lanConfig_joinname)-1] = 0;
		}
		break;

	default:
		if (key < 32 || key > 127)
			break;

		if (lanConfig_cursor == 2)
		{
			l = strlen(lanConfig_joinname);
			if (l < 21)
			{
				lanConfig_joinname[l+1] = 0;
				lanConfig_joinname[l] = key;
			}
		}

		if (key < '0' || key > '9')
			break;
		if (lanConfig_cursor == 0)
		{
			l = strlen(lanConfig_portname);
			if (l < 5)
			{
				lanConfig_portname[l+1] = 0;
				lanConfig_portname[l] = key;
			}
		}
	}

	if (StartingGame && lanConfig_cursor == 2)
	{	// 1999-12-24 explicit brackets by Maddes
		if (key == K_UPARROW)
			lanConfig_cursor = 1;
		else
			lanConfig_cursor = 0;
	}	// 1999-12-24 explicit brackets by Maddes

	l =  Q_atoi(lanConfig_portname);
	if (l > 65535)
		l = lanConfig_port;
	else
		lanConfig_port = l;
	sprintf(lanConfig_portname, "%u", lanConfig_port);
}

//=============================================================================
/* GAME OPTIONS MENU */

typedef struct
{
	char	*name;
	char	*description;
} level_t;

level_t	levels[] =
{
	{"start", "Entrance"},				// 0

	{"e1m1", "Slipgate Complex"},		// 1
	{"e1m2", "Castle of the Damned"},
	{"e1m3", "The Necropolis"},
	{"e1m4", "The Grisly Grotto"},
	{"e1m5", "Gloom Keep"},
	{"e1m6", "The Door To Chthon"},
	{"e1m7", "The House of Chthon"},
	{"e1m8", "Ziggurat Vertigo"},

	{"e2m1", "The Installation"},		// 9
	{"e2m2", "Ogre Citadel"},
	{"e2m3", "Crypt of Decay"},
	{"e2m4", "The Ebon Fortress"},
	{"e2m5", "The Wizard's Manse"},
	{"e2m6", "The Dismal Oubliette"},
	{"e2m7", "Underearth"},

	{"e3m1", "Termination Central"},	// 16
	{"e3m2", "The Vaults of Zin"},
	{"e3m3", "The Tomb of Terror"},
	{"e3m4", "Satan's Dark Delight"},
	{"e3m5", "Wind Tunnels"},
	{"e3m6", "Chambers of Torment"},
	{"e3m7", "The Haunted Halls"},

	{"e4m1", "The Sewage System"},		// 23
	{"e4m2", "The Tower of Despair"},
	{"e4m3", "The Elder God Shrine"},
	{"e4m4", "The Palace of Hate"},
	{"e4m5", "Hell's Atrium"},
	{"e4m6", "The Pain Maze"},
	{"e4m7", "Azure Agony"},
	{"e4m8", "The Nameless City"},

	{"end", "Shub-Niggurath's Pit"},	// 31

	{"dm1", "Place of Two Deaths"},		// 32
	{"dm2", "Claustrophobopolis"},
	{"dm3", "The Abandoned Base"},
	{"dm4", "The Bad Place"},
	{"dm5", "The Cistern"},
	{"dm6", "The Dark Zone"}
};

//MED 01/06/97 added hipnotic levels
level_t	hipnoticlevels[] =
{
	{"start", "Command HQ"},			// 0

	{"hip1m1", "The Pumping Station"},	// 1
	{"hip1m2", "Storage Facility"},
	{"hip1m3", "The Lost Mine"},
	{"hip1m4", "Research Facility"},
	{"hip1m5", "Military Complex"},

	{"hip2m1", "Ancient Realms"},		// 6
	{"hip2m2", "The Black Cathedral"},
	{"hip2m3", "The Catacombs"},
	{"hip2m4", "The Crypt"},
	{"hip2m5", "Mortum's Keep"},
	{"hip2m6", "The Gremlin's Domain"},

	{"hip3m1", "Tur Torment"},			// 12
	{"hip3m2", "Pandemonium"},
	{"hip3m3", "Limbo"},
	{"hip3m4", "The Gauntlet"},

	{"hipend", "Armagon's Lair"},		// 16

	{"hipdm1", "The Edge of Oblivion"}	// 17
};

//PGM 01/07/97 added rogue levels
//PGM 03/02/97 added dmatch level
level_t		roguelevels[] =
{
	{"start",	"Split Decision"},
	{"r1m1",	"Deviant's Domain"},
	{"r1m2",	"Dread Portal"},
	{"r1m3",	"Judgement Call"},
	{"r1m4",	"Cave of Death"},
	{"r1m5",	"Towers of Wrath"},
	{"r1m6",	"Temple of Pain"},
	{"r1m7",	"Tomb of the Overlord"},
	{"r2m1",	"Tempus Fugit"},
	{"r2m2",	"Elemental Fury I"},
	{"r2m3",	"Elemental Fury II"},
	{"r2m4",	"Curse of Osiris"},
	{"r2m5",	"Wizard's Keep"},
	{"r2m6",	"Blood Sacrifice"},
	{"r2m7",	"Last Bastion"},
	{"r2m8",	"Source of Evil"},
	{"ctf1",	"Division of Change"}
};

typedef struct
{
	char	*description;
	int		firstLevel;
	int		levels;
} episode_t;

episode_t	episodes[] =
{
	{"Welcome to Quake", 0, 1},
	{"Doomed Dimension", 1, 8},
	{"Realm of Black Magic", 9, 7},
	{"Netherworld", 16, 7},
	{"The Elder World", 23, 8},
	{"Final Level", 31, 1},
	{"Deathmatch Arena", 32, 6}
};

//MED 01/06/97  added hipnotic episodes
episode_t   hipnoticepisodes[] =
{
	{"Scourge of Armagon", 0, 1},
	{"Fortress of the Dead", 1, 5},
	{"Dominion of Darkness", 6, 6},
	{"The Rift", 12, 4},
	{"Final Level", 16, 1},
	{"Deathmatch Arena", 17, 1}
};

//PGM 01/07/97 added rogue episodes
//PGM 03/02/97 added dmatch episode
episode_t	rogueepisodes[] =
{
	{"Introduction", 0, 1},
	{"Hell's Fortress", 1, 7},
	{"Corridors of Time", 8, 8},
	{"Deathmatch Arena", 16, 1}
};

int	startepisode;
int	startlevel;
int maxplayers;
qboolean m_serverInfoMessage = false;
double m_serverInfoMessageTime;

void M_Menu_GameOptions_f (void)
{
	key_dest = key_menu;
	m_state = m_gameoptions;
	m_entersound = true;
// 2000-01-11 Set default maximum clients to 16 instead of 4 by Maddes  start
//	if (maxplayers == 0)
	if (maxplayers < 2)
// 2000-01-11 Set default maximum clients to 16 instead of 4 by Maddes  end
		maxplayers = svs.maxclients;
	if (maxplayers < 2)
		maxplayers = svs.maxclientslimit;
}


int gameoptions_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 112, 120};
#define	NUM_GAMEOPTIONS	9
int		gameoptions_cursor;

void M_GameOptions_Draw (void)
{
	qpic_t	*p;
	int		x;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	M_DrawTextBox (152, 32, 10, 1);
	M_Print (160, 40, "begin game");

	M_Print (0, 56, "      Max players");
	M_Print (160, 56, va("%i", maxplayers) );

	M_Print (0, 64, "        Game Type");
	if (coop->value)
		M_Print (160, 64, "Cooperative");
	else
		M_Print (160, 64, "Deathmatch");

	M_Print (0, 72, "        Teamplay");
	if (rogue)
	{
		char *msg;

		switch((int)teamplay->value)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			case 3: msg = "Tag"; break;
			case 4: msg = "Capture the Flag"; break;
			case 5: msg = "One Flag CTF"; break;
			case 6: msg = "Three Team CTF"; break;
			default: msg = "Off"; break;
		}
		M_Print (160, 72, msg);
	}
	else
	{
		char *msg;

		switch((int)teamplay->value)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			default: msg = "Off"; break;
		}
		M_Print (160, 72, msg);
	}

	M_Print (0, 80, "            Skill");
	if (skill->value == 0)
		M_Print (160, 80, "Easy difficulty");
	else if (skill->value == 1)
		M_Print (160, 80, "Normal difficulty");
	else if (skill->value == 2)
		M_Print (160, 80, "Hard difficulty");
	else
		M_Print (160, 80, "Nightmare difficulty");

	M_Print (0, 88, "       Frag Limit");
	if (fraglimit->value == 0)
		M_Print (160, 88, "none");
	else
		M_Print (160, 88, va("%i frags", (int)fraglimit->value));

	M_Print (0, 96, "       Time Limit");
	if (timelimit->value == 0)
		M_Print (160, 96, "none");
	else
		M_Print (160, 96, va("%i minutes", (int)timelimit->value));

	M_Print (0, 112, "         Episode");
	//MED 01/06/97 added hipnotic episodes
	if (hipnotic)
		M_Print (160, 112, hipnoticepisodes[startepisode].description);
	//PGM 01/07/97 added rogue episodes
	else if (rogue)
		M_Print (160, 112, rogueepisodes[startepisode].description);
	else
		M_Print (160, 112, episodes[startepisode].description);

	M_Print (0, 120, "           Level");
	//MED 01/06/97 added hipnotic episodes
	if (hipnotic)
	{
		M_Print (160, 120, hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].name);
	}
	//PGM 01/07/97 added rogue episodes
	else if (rogue)
	{
		M_Print (160, 120, roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].name);
	}
	else
	{
		M_Print (160, 120, levels[episodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, levels[episodes[startepisode].firstLevel + startlevel].name);
	}

// line cursor
	M_DrawCharacter (144, gameoptions_cursor_table[gameoptions_cursor], 12+((int)(realtime*4)&1));

	if (m_serverInfoMessage)
	{
		if ((realtime - m_serverInfoMessageTime) < 5.0)
		{
			x = (320-26*8)/2;
			M_DrawTextBox (x, 138, 24, 4);
			x += 8;
// 2000-01-11 Set default maximum clients to 16 instead of 4 by Maddes  start
//			M_Print (x, 146, "  More than 4 players   ");
//			M_Print (x, 154, " requires using command ");
//			M_Print (x, 162, "line parameters; please ");
			M_Print (x, 146, " More players requires  ");
			M_Print (x, 154, "   using command line   ");
			M_Print (x, 162, "   parameters; please   ");
// 2000-01-11 Set default maximum clients to 16 instead of 4 by Maddes  end
			M_Print (x, 170, "   see techinfo.txt.    ");
		}
		else
		{
			m_serverInfoMessage = false;
		}
	}
}


void M_NetStart_Change (int dir)
{
	int count;

	switch (gameoptions_cursor)
	{
	case 1:
		maxplayers += dir;
		if (maxplayers > svs.maxclientslimit)
		{
			maxplayers = svs.maxclientslimit;
// 2000-01-11 Set default maximum clients to 16 instead of 4 by Maddes  start
			if (svs.maxclientslimit < MAX_SCOREBOARD)
			{
// 2000-01-11 Set default maximum clients to 16 instead of 4 by Maddes  end
				m_serverInfoMessage = true;
				m_serverInfoMessageTime = realtime;
			}	// 2000-01-11 Set default maximum clients to 16 instead of 4 by Maddes
		}
		if (maxplayers < 2)
			maxplayers = 2;
		break;

	case 2:
		Cvar_Set (coop, coop->value ? "0" : "1");
		break;

	case 3:
		if (rogue)
			count = 6;
		else
			count = 2;

		Cvar_SetValue (teamplay, teamplay->value + dir);
		if (teamplay->value > count)
			Cvar_Set (teamplay, "0");
		else if (teamplay->value < 0)
			Cvar_SetValue (teamplay, count);
		break;

	case 4:
		Cvar_SetValue (skill, skill->value + dir);
		if (skill->value > 3)
			Cvar_Set (skill, "0");
		if (skill->value < 0)
			Cvar_Set (skill, "3");
		break;

	case 5:
		Cvar_SetValue (fraglimit, fraglimit->value + dir*10);
		if (fraglimit->value > 100)
			Cvar_Set (fraglimit, "0");
		if (fraglimit->value < 0)
			Cvar_Set (fraglimit, "100");
		break;

	case 6:
		Cvar_SetValue (timelimit, timelimit->value + dir*5);
		if (timelimit->value > 60)
			Cvar_Set (timelimit, "0");
		if (timelimit->value < 0)
			Cvar_Set (timelimit, "60");
		break;

	case 7:
		startepisode += dir;
	//MED 01/06/97 added hipnotic count
		if (hipnotic)
			count = 6;
	//PGM 01/07/97 added rogue count
	//PGM 03/02/97 added 1 for dmatch episode
		else if (rogue)
			count = 4;
		else if (registered->value)
			count = 7;
		else
			count = 2;

		if (startepisode < 0)
			startepisode = count - 1;

		if (startepisode >= count)
			startepisode = 0;

		startlevel = 0;
		break;

	case 8:
		startlevel += dir;
	//MED 01/06/97 added hipnotic episodes
		if (hipnotic)
			count = hipnoticepisodes[startepisode].levels;
	//PGM 01/06/97 added hipnotic episodes
		else if (rogue)
			count = rogueepisodes[startepisode].levels;
		else
			count = episodes[startepisode].levels;

		if (startlevel < 0)
			startlevel = count - 1;

		if (startlevel >= count)
			startlevel = 0;
		break;
	}
}

void M_GameOptions_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor--;
		if (gameoptions_cursor < 0)
			gameoptions_cursor = NUM_GAMEOPTIONS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor++;
		if (gameoptions_cursor >= NUM_GAMEOPTIONS)
			gameoptions_cursor = 0;
		break;

	case K_LEFTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menu3.wav");
		M_NetStart_Change (-1);
		break;

	case K_RIGHTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menu3.wav");
		M_NetStart_Change (1);
		break;

	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		if (gameoptions_cursor == 0)
		{
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
			Cbuf_AddText ( va ("maxplayers %u\n", maxplayers) );
			SCR_BeginLoadingPlaque ();

			if (hipnotic)
				Cbuf_AddText ( va ("map %s\n", hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].name) );
			else if (rogue)
				Cbuf_AddText ( va ("map %s\n", roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].name) );
			else
				Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + startlevel].name) );

			return;
		}

		M_NetStart_Change (1);
		break;
	}
}

//=============================================================================
/* SEARCH MENU */

qboolean	searchComplete = false;
double		searchCompleteTime;

void M_Menu_Search_f (void)
{
	key_dest = key_menu;
	m_state = m_search;
	m_entersound = false;
	slistSilent = true;
	slistLocal = false;
	searchComplete = false;
	NET_Slist_f();

}


void M_Search_Draw (void)
{
	qpic_t	*p;
	int x;

	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	x = (320/2) - ((12*8)/2) + 4;
	M_DrawTextBox (x-8, 32, 12, 1);
	M_Print (x, 40, "Searching...");

	if(slistInProgress)
	{
		NET_Poll();
		return;
	}

	if (! searchComplete)
	{
		searchComplete = true;
		searchCompleteTime = realtime;
	}

	if (hostCacheCount)
	{
		M_Menu_ServerList_f ();
		return;
	}

	M_PrintWhite ((320/2) - ((22*8)/2), 64, "No Quake servers found");
	if ((realtime - searchCompleteTime) < 3.0)
		return;

	M_Menu_LanConfig_f ();
}


void M_Search_Key (int key)
{
}

//=============================================================================
/* SLIST MENU */

int		slist_cursor;
qboolean slist_sorted;

void M_Menu_ServerList_f (void)
{
	key_dest = key_menu;
	m_state = m_slist;
	m_entersound = true;
	slist_cursor = 0;
	m_return_onerror = false;
	m_return_reason[0] = 0;
	slist_sorted = false;
}


void M_ServerList_Draw (void)
{
	int		n;
	char	string [64];
	qpic_t	*p;

	if (!slist_sorted)
	{
		if (hostCacheCount > 1)
		{
			int	i,j;
			hostcache_t temp;
			for (i = 0; i < hostCacheCount; i++)
				for (j = i+1; j < hostCacheCount; j++)
					if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
					{
						Q_memcpy(&temp, &hostcache[j], sizeof(hostcache_t));
						Q_memcpy(&hostcache[j], &hostcache[i], sizeof(hostcache_t));
						Q_memcpy(&hostcache[i], &temp, sizeof(hostcache_t));
					}
		}
		slist_sorted = true;
	}

	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	for (n = 0; n < hostCacheCount; n++)
	{
		if (hostcache[n].maxusers)
			sprintf(string, "%-15.15s %-15.15s %2u/%2u\n", hostcache[n].name, hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			sprintf(string, "%-15.15s %-15.15s\n", hostcache[n].name, hostcache[n].map);
		M_Print (16, 32 + 8*n, string);
	}
	M_DrawCharacter (0, 32 + slist_cursor*8, 12+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (16, 148, m_return_reason);
}


void M_ServerList_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_LanConfig_f ();
		break;

	case K_SPACE:
		M_Menu_Search_f ();
		break;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		slist_cursor--;
		if (slist_cursor < 0)
			slist_cursor = hostCacheCount - 1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		slist_cursor++;
		if (slist_cursor >= hostCacheCount)
			slist_cursor = 0;
		break;

	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		m_return_state = m_state;
		m_return_onerror = true;
		slist_sorted = false;
		key_dest = key_game;
		m_state = m_none;
		Cbuf_AddText ( va ("connect \"%s\"\n", hostcache[slist_cursor].cname) );
		break;

	default:
		break;
	}

}

//=============================================================================
/* Menu Subsystem */


void M_Init (void)
{
	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f);

	Cmd_AddCommand ("menu_main", M_Menu_Main_f);
	Cmd_AddCommand ("menu_singleplayer", M_Menu_SinglePlayer_f);
	Cmd_AddCommand ("menu_load", M_Menu_Load_f);
	Cmd_AddCommand ("menu_save", M_Menu_Save_f);
	Cmd_AddCommand ("menu_multiplayer", M_Menu_MultiPlayer_f);
	Cmd_AddCommand ("menu_setup", M_Menu_Setup_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
	Cmd_AddCommand ("help", M_Menu_Help_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);

// 2002-01-31 New menu system by Maddes
	current_cursor = NULL;
	current_menu = NULL;
// 2002-01-31 New menu system by Maddes
}


void M_Draw (void)
{
	if (m_state == m_none || key_dest != key_menu)
		return;

	if (!m_recursiveDraw)
	{
		scr_copyeverything = 1;

// 2000-01-12 Variable console height by Fett/Maddes  start
//		if (scr_con_current)
		if (con_forcedup)
// 2000-01-12 Variable console height by Fett/Maddes  end
		{
			Draw_ConsoleBackground (vid.height);
			VID_UnlockBuffer ();
			S_ExtraUpdate ();
			VID_LockBuffer ();
		}
		else
			Draw_FadeScreen ();

		scr_fullupdate = 0;
	}
	else
	{
		m_recursiveDraw = false;
	}

	switch (m_state)
	{
	case m_none:
		break;

	case m_main:
		M_Main_Draw ();
		break;

	case m_singleplayer:
		M_SinglePlayer_Draw ();
		break;

	case m_load:
		M_Load_Draw ();
		break;

	case m_save:
		M_Save_Draw ();
		break;

	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;

	case m_setup:
		M_Setup_Draw ();
		break;

	case m_net:
		M_Net_Draw ();
		break;

// 2002-01-31 New menu system by Maddes  start
/*
	case m_options:
		M_Options_Draw ();
		break;
*/
// 2002-01-31 New menu system by Maddes  end

	case m_keys:
		M_Keys_Draw ();
		break;

	case m_video:
		M_Video_Draw ();
		break;

	case m_help:
		M_Help_Draw ();
		break;

	case m_quit:
		M_Quit_Draw ();
		break;

	case m_serialconfig:
		M_SerialConfig_Draw ();
		break;

	case m_modemconfig:
		M_ModemConfig_Draw ();
		break;

	case m_lanconfig:
		M_LanConfig_Draw ();
		break;

	case m_gameoptions:
		M_GameOptions_Draw ();
		break;

	case m_search:
		M_Search_Draw ();
		break;

	case m_slist:
		M_ServerList_Draw ();
		break;

// 2002-01-31 New menu system by Maddes  start
	default:
		M_Menu_Draw(current_menu, current_cursor);
		break;
// 2002-01-31 New menu system by Maddes  end
	}

	if (m_entersound)
	{
		S_LocalSound ("misc/menu2.wav");
		m_entersound = false;
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}


void M_Keydown (int key)
{
	switch (m_state)
	{
	case m_none:
		return;

	case m_main:
		M_Main_Key (key);
		return;

	case m_singleplayer:
		M_SinglePlayer_Key (key);
		return;

	case m_load:
		M_Load_Key (key);
		return;

	case m_save:
		M_Save_Key (key);
		return;

	case m_multiplayer:
		M_MultiPlayer_Key (key);
		return;

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_net:
		M_Net_Key (key);
		return;

// 2002-01-31 New menu system by Maddes  start
/*
	case m_options:
		M_Options_Key (key);
		return;
*/
// 2002-01-31 New menu system by Maddes  end

	case m_keys:
		M_Keys_Key (key);
		return;

	case m_video:
		M_Video_Key (key);
		return;

	case m_help:
		M_Help_Key (key);
		return;

	case m_quit:
		M_Quit_Key (key);
		return;

	case m_serialconfig:
		M_SerialConfig_Key (key);
		return;

	case m_modemconfig:
		M_ModemConfig_Key (key);
		return;

	case m_lanconfig:
		M_LanConfig_Key (key);
		return;

	case m_gameoptions:
		M_GameOptions_Key (key);
		return;

	case m_search:
		M_Search_Key (key);
		break;

	case m_slist:
		M_ServerList_Key (key);
		return;

// 2002-01-31 New menu system by Maddes  start
	default:
		M_Menu_Key(current_menu, current_cursor, key);
		break;
// 2002-01-31 New menu system by Maddes  end
	}
}


void M_ConfigureNetSubsystem(void)
{
// enable/disable net systems to match desired config

	Cbuf_AddText ("stopdemo\n");
	if (SerialConfig || DirectConfig)
	{
		Cbuf_AddText ("com1 enable\n");
	}

	if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
}
