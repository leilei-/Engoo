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
extern cvar_t *loadscreen;
enum {m_none, m_main, m_singleplayer, m_load, m_save, m_multiplayer, m_setup, m_net, m_options, m_video, m_keys, m_help, m_quit, m_serialconfig, m_modemconfig, m_lanconfig, m_gameoptions, m_search, m_slist, m_keys2, m_broken} m_state;

void M_Menu_Main_f (void);
	void M_Menu_SinglePlayer_f (void);
		void M_Menu_Load_f (void);
		void M_Menu_Save_f (void);
	void M_Menu_MultiPlayer_f (void);
		void M_Menu_Setup_f (void);
		void M_Menu_Net_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Keys_f (void);
		void M_Menu_Keys_f2 (void); // leilei
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

void Preset_Q101(void);
void Preset_Q107(void);
void Preset_Q64(void);
void Preset_GLQ(void);
void Preset_D(void);
void Preset_U(void);
void Preset_Xtreem(void);
void Preset_Lei(void);
void Preset_Crap(void);

qboolean	m_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
qboolean	m_recursiveDraw;
extern int	menu_scaled;
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

extern cvar_t *v_contrast;
extern cvar_t *v_saturation;
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
extern cvar_t *r_menucolor;
#ifndef BENCH
extern cvar_t *s_pitchin;
extern cvar_t *s_oldspatial;
#endif
extern cvar_t *r_coloredlights;
extern cvar_t *r_shiftlighting;
extern cvar_t *r_truecolor;
extern cvar_t *r_waterquality;
extern cvar_t *r_fogquality;
extern cvar_t *r_fogdither;
extern cvar_t *r_overbrightBits;
extern cvar_t *r_fullbrights;
extern cvar_t *r_coloreddyns;
extern cvar_t *r_particlespray;
extern cvar_t *r_particleset;
extern cvar_t *r_particleblood;
extern cvar_t *r_flares;
extern cvar_t *cl_bobmodel;
extern cvar_t *cl_bobfall;
extern cvar_t *cl_bob2;
extern cvar_t *cl_leanmodel;
extern cvar_t *cl_followmodel;
extern cvar_t *cl_gundraw;
extern cvar_t *r_shading;
extern cvar_t *cl_gunhold;

extern cvar_t *sv_standstill;
extern cvar_t *scr_scale;
extern cvar_t *scr_retroscale;
extern cvar_t *r_shadowhack;
extern cvar_t *r_alphashift;
extern cvar_t *r_tranquality;
extern cvar_t *r_lightquality;
extern cvar_t *r_shadedither;
extern cvar_t *r_shinygrays;
extern cvar_t *r_lowmodels;
extern cvar_t *r_lowdetail;
extern cvar_t *r_virtualmode;
extern cvar_t *r_lowworld;
extern cvar_t *r_filter;
extern cvar_t *r_dither;
extern cvar_t *d_mipdetail;
extern cvar_t *r_overbrightmdl;

extern cvar_t *r_wateralpha;
extern cvar_t *r_waterblend;
extern cvar_t *r_flamehack;
extern cvar_t *r_particlesprite;
extern cvar_t *r_lerpmodels;
extern cvar_t *midivolume;
extern cvar_t *autosaver;
extern cvar_t *loadscreen;
extern cvar_t *r_dynamic;
extern cvar_t *r_muzzlehack;
extern cvar_t *cl_diecam;
extern cvar_t *scr_aspectmode;

extern cvar_t *s_underwater;
extern cvar_t *s_gibs;
extern cvar_t *s_blood;
extern cvar_t *s_playerdeath;
#ifdef _WIN32
extern cvar_t *vid_stretch_by_2;
#endif
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
#define MENU_COLOR						175
#define MENU_SOUND_PITCH				176
#define MENU_COLORED					177
#define MENU_LOWDETAIL					178
#define MENU_LOWWORLD					179
#define MENU_LOWMODELS					180
#define MENU_LERPMODELS					181
#define MENU_MIDI_VOLUME				182
#define MENU_COLOREDDYNS				183
#define MENU_TRANQUALITY				184
#define MENU_LIGHTQUALITY				185
#define MENU_ALPHASHIFT					186
#define MENU_WATERALPHA					187
#define MENU_WATERBLEND					188
#define MENU_CUSTOMIZE_CONTROLS2		189
#define MENU_CLASSICSPAT				190
#define MENU_SHADOWHACK					191
#define MENU_BROKEN_OPTIONS				192
#define MENU_OVERBRIGHT					193
#define MENU_VIEW_OPTIONS				194
#define MENU_QUALITY_OPTIONS			195
#define MENU_SPRAY						196
#define MENU_BLOODHACK					197
#define MENU_PARTICLESET				198
#define MENU_STRETCHMODE				199
#define MENU_OLDSTATBAR					200
#define MENU_HUDSWAP					201
#define MENU_PSET_VANILLA				202
#define MENU_PSET_DP					203
#define MENU_PSET_FAKE					204
#define MENU_PSET_PLASMA				205
#define MENU_PSET_WEEEEE				206
#define MENU_FILTERING					207
#define MENU_COLORMAP_OPTIONS			208
#define MENU_OVERBRIGHTS				209
#define MENU_FULLBRIGHTS				210
#define MENU_REVERT_COLORMAP			211
#define MENU_MIPDETAIL					212
#define MENU_GUNDRAW					213
#define MENU_GUNHOLD					214
#define MENU_BOBMODEL					215
#define MENU_LEANMODEL					216
#define MENU_FOLLOWMODEL				217
#define MENU_INTERNAL_OPTIONS			218
#define MENU_DITHERING					219
#define MENU_SCREENSCALE				220
#define MENU_AUTOSAVER					221
#define MENU_LOADSCREEN					222
#define MENU_CONTRAST					223
#define MENU_SATURATION					224
#define MENU_BLOODLEVEL					225
#define MENU_DYNAMIC					226
#define MENU_CONTENT_OPTIONS			227
#define MENU_MUZZLEBLEND				228
#define MENU_FOGQUALITY					229
#define MENU_WATERQUALITY				230
#define MENU_SHADEDITHER				231
#define MENU_PARTICLESPRITES			232
#define MENU_FLAMEHACK					233
#define MENU_FLARES						234
#define MENU_SHADING						235
#define MENU_PRESET_OPTIONS				236
#define MENU_HUD_OPTIONS				237
#define MENU_PRESET_Q101				238
#define MENU_PRESET_Q107				239
#define MENU_PRESET_GLQ					240
#define MENU_PRESET_Q64					241
#define MENU_PRESET_D					242
#define MENU_PRESET_U					243
#define MENU_PRESET_XTREEM				244
#define MENU_PRESET_LEI					245
#define MENU_PRESET_CRAP				246

#define MENU_DEATHCAM					247
#define MENU_ASPECT						248
#define MENU_RETROSCALE					249
#define MENU_GIBBURST					250
#define MENU_PLAYERDEATH				251
#define MENU_UNDERWATER					252
#define MENU_STANDSTILL					253
#define MENU_BLOODBURST					254
#define MENU_SHINYGRAYS					255

extern int nolookups;
// menu definitions
int		options_cursor;
menu_definition_t	m_menu_options[] =
{	// Options Menu
	{MENU_MAIN, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_CUSTOMIZE_CONTROLS, MENU_SELECTABLE},
//	{MENU_CUSTOMIZE_CONTROLS2, MENU_SELECTABLE},
	{MENU_GO_TO_CONSOLE, MENU_SELECTABLE},
	{MENU_LOAD_DEFAULT_CFG, MENU_SELECTABLE},
	{MENU_CONTROL_OPTIONS, MENU_SELECTABLE},
	{MENU_SOUND_OPTIONS, MENU_SELECTABLE},
	{MENU_INTERNAL_OPTIONS, MENU_SELECTABLE},
//	{MENU_CLIENT_OPTIONS, MENU_SELECTABLE},	// leilei - moved this to another submenu
//	{MENU_SERVER_OPTIONS, MENU_SELECTABLE},	// also moved to another menu
	{MENU_VIEW_OPTIONS, MENU_SELECTABLE},
	{MENU_VIDEO_OPTIONS, MENU_SELECTABLE},
	{MENU_QUALITY_OPTIONS, MENU_SELECTABLE},
	{MENU_CONTENT_OPTIONS, MENU_SELECTABLE},
//	{MENU_COLORMAP_OPTIONS, MENU_SELECTABLE}, // deprecated
#ifdef EXPERIMENT
	{MENU_BROKEN_OPTIONS, MENU_SELECTABLE},
#endif
	{MENU_VIDEO_RESOLUTION, MENU_SELECTABLE},
	{MENU_PRESET_OPTIONS, MENU_SELECTABLE},
	//{MENU_HUD_OPTIONS, MENU_SELECTABLE}, // completely unfinished
	{MENU_AUTOSAVER, MENU_SELECTABLE},
	
	{0, 0},	// end of submenu
};
menu_definition_t	m_menu_internal_options[] =
{	// Options Menu
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_EXTERNAL_DATA, MENU_SELECTABLE},
	{MENU_CLIENT_OPTIONS, MENU_SELECTABLE},
	{MENU_SERVER_OPTIONS, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};
int		shadowhack;
extern	cvar_t  *r_shadowhack;

cvar_t  *menu_quitscreen;
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
	{MENU_MIDI_VOLUME, MENU_SELECTABLE},
	{MENU_SOUND_VOLUME, MENU_SELECTABLE},
	{MENU_SOUND_PITCH, MENU_SELECTABLE},
	{MENU_CLASSICSPAT, MENU_SELECTABLE},
	{MENU_UNDERWATER, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

int		external_data_cursor;
menu_definition_t	m_menu_external_data[] =
{	// External Data
	{MENU_INTERNAL_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_EXTERNAL_ENT, MENU_SELECTABLE},
	{MENU_EXTERNAL_VIS, MENU_SELECTABLE},
	{MENU_EXTERNAL_LIT, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

int		client_options_cursor;
menu_definition_t	m_menu_client_options[] =
{	// Client Options
	{MENU_INTERNAL_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_CL_ENTITIES_MIN, MENU_SELECTABLE},
	{MENU_CL_ENTITIES_TEMP_MIN, MENU_SELECTABLE},
	{MENU_CL_ENTITIES_STATIC_MIN, MENU_SELECTABLE},
	{MENU_CL_COMPATIBILITY, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

int		server_options_cursor;
menu_definition_t	m_menu_server_options[] =
{	// Server Options
	{MENU_INTERNAL_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
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
int		view_options_cursor;
int		quality_options_cursor;
int		content_options_cursor;
int		internal_options_cursor;
int		preset_options_cursor;
int		hud_options_cursor;
menu_definition_t	m_menu_video_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
//	{MENU_SCREENSIZE, MENU_SELECTABLE},		// redundant
//	{MENU_BRIGHTNESS, MENU_SELECTABLE},		// redundant
	{MENU_CON_ALPHA, MENU_SELECTABLE},
	{MENU_CON_HEIGHT, MENU_SELECTABLE},
	{MENU_SHOW_FPS, MENU_SELECTABLE},
	{MENU_COLOR, MENU_SELECTABLE},

	{MENU_COLORED, MENU_SELECTABLE},
	
	{MENU_WATERALPHA, MENU_SELECTABLE},
	{MENU_WATERBLEND, MENU_SELECTABLE},

	
#ifdef GLQUAKE
	{MENU_GL_MAXDEPTH, MENU_SELECTABLE},
#endif
#ifdef _WIN32
	{MENU_USE_MOUSE, MENU_INVISIBLE},	// only present in windowed mode on Win32
#endif
	{0, 0},	// end of submenu
};
// 2002-01-31 New menu system by Maddes  end

menu_definition_t	m_menu_quality_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	//{MENU_LOWWORLD, MENU_SELECTABLE},		// no c implementation
	{MENU_LOWDETAIL, MENU_SELECTABLE},	// crashes with water
	//{MENU_LOWMODELS, MENU_SELECTABLE},	// unimplemented
	{MENU_TRANQUALITY, MENU_SELECTABLE},
	{MENU_WATERQUALITY, MENU_SELECTABLE},
	//{MENU_LIGHTQUALITY, MENU_SELECTABLE},	// deprecated 
	{MENU_MIPDETAIL, MENU_SELECTABLE},
	{MENU_FILTERING, MENU_SELECTABLE},
	{MENU_LERPMODELS, MENU_SELECTABLE},
	{MENU_SHADEDITHER, MENU_SELECTABLE},
	{MENU_SHADING, MENU_SELECTABLE},
	{MENU_DYNAMIC, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};

menu_definition_t	m_menu_preset_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_PRESET_Q101, MENU_SELECTABLE},
	{MENU_PRESET_Q107, MENU_SELECTABLE},
	{MENU_PRESET_GLQ, MENU_SELECTABLE},
	{MENU_PRESET_Q64, MENU_SELECTABLE},
	{MENU_PRESET_D, MENU_SELECTABLE},
	{MENU_PRESET_U, MENU_SELECTABLE},
	{MENU_PRESET_XTREEM, MENU_SELECTABLE},
	{MENU_PRESET_LEI, MENU_SELECTABLE},
	{MENU_PRESET_CRAP, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};


menu_definition_t	m_menu_hud_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_SHADEDITHER, MENU_SELECTABLE},
	{MENU_SHADING, MENU_SELECTABLE},
	{MENU_DYNAMIC, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};


menu_definition_t	m_menu_content_options[] =
{	// Content Hacks
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_COLOREDDYNS, MENU_SELECTABLE},
	{MENU_MUZZLEBLEND, MENU_SELECTABLE},
	{MENU_SHINYGRAYS, MENU_SELECTABLE},
	{MENU_SHADOWHACK, MENU_SELECTABLE},
	{MENU_BLOODLEVEL, MENU_SELECTABLE},
	{MENU_FLAMEHACK, MENU_SELECTABLE},
	{MENU_FLARES, MENU_SELECTABLE},
	{MENU_PARTICLESET, MENU_SELECTABLE},
	{MENU_GIBBURST, MENU_SELECTABLE},
	{MENU_BLOODBURST, MENU_SELECTABLE},
	{MENU_PLAYERDEATH, MENU_SELECTABLE},
	{MENU_STANDSTILL, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};


menu_definition_t	m_menu_view_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_SCREENSIZE, MENU_SELECTABLE},
	{MENU_ASPECT, MENU_SELECTABLE},
	{MENU_BRIGHTNESS, MENU_SELECTABLE},
	{MENU_SATURATION, MENU_SELECTABLE},
	{MENU_CONTRAST, MENU_SELECTABLE},
	{MENU_OLDSTATBAR, MENU_SELECTABLE},
	{MENU_HUDSWAP, MENU_SELECTABLE},
#ifdef _WIN32
//	{MENU_STRETCHMODE,MENU_SELECTABLE},	// only present in windowed mode on Win32
#endif
	{MENU_BOBMODEL, MENU_SELECTABLE},
	{MENU_FOLLOWMODEL, MENU_SELECTABLE},
	{MENU_LEANMODEL, MENU_SELECTABLE},
	{MENU_GUNDRAW, MENU_SELECTABLE},
	{MENU_GUNHOLD, MENU_SELECTABLE},
	{MENU_DEATHCAM, MENU_SELECTABLE},
	{MENU_SCREENSCALE, MENU_SELECTABLE},
	{MENU_RETROSCALE, MENU_SELECTABLE},
	{MENU_LOADSCREEN, MENU_SELECTABLE},
	
	{0, 0},	// end of submenu
};



int		broken_options_cursor;
menu_definition_t	m_menu_broken_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
//	{MENU_LOWWORLD, MENU_SELECTABLE},
//	{MENU_LOWMODELS, MENU_SELECTABLE},
	{MENU_ALPHASHIFT, MENU_SELECTABLE},
	{MENU_OVERBRIGHT, MENU_SELECTABLE},
	{MENU_SPRAY, MENU_SELECTABLE},
	{MENU_PARTICLESET, MENU_SELECTABLE},
	{MENU_BLOODHACK, MENU_SELECTABLE},
	{0, 0},	// end of submenu
};


int		colormap_options_cursor;
menu_definition_t	m_menu_colormap_options[] =
{	// Video Options
	{MENU_OPTIONS, MENU_OPTIONS},	// this is the ESC key function and title
	{MENU_OVERBRIGHTS, MENU_SELECTABLE},
	{MENU_FULLBRIGHTS, MENU_SELECTABLE},
//	{MENU_REVERT_COLORMAP, MENU_SELECTABLE}, // causes a crash.
	{0, 0},	// end of submenu
};

extern int lilchar;
/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
#ifdef SCALED2D
	if (menu_scaled)
	Draw_Character_Scaled ( cx + ((vid.vconwidth - 320)>>1), line, num);
	else
	Draw_Character ( cx + ((vid.width - 320)>>1), line, num);
	
#else
	Draw_Character ( cx + ((vid.width - 320)>>1), line, num);
#endif
}

void M_Print (int cx, int cy, char *str)
{	
	//int ye; if(lilchar)	ye = 4;	else ye = 8;
	if (!qbeta){
	while (*str)
	{
		
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
	}
	else
	{
		while (*str)
	{	// leilei - old versions had white menus
		
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
	}
}

void M_PrintWhite (int cx, int cy, char *str)
{
	//int ye; if(lilchar)	ye = 4;	else ye = 8;
	while (*str)
	{
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic (int x, int y, qpic_t *pic)
{
#ifdef SCALED2D
	if (menu_scaled)
	Draw_TransPic_Scaled (x + ((vid.vconwidth - 320)>>1), y, pic);
	else
	Draw_TransPic (x + ((vid.width - 320)>>1), y, pic);
#else
	Draw_TransPic (x + ((vid.width - 320)>>1), y, pic);
#endif
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
#ifdef SCALED2D
	if (menu_scaled)
	if (gamemode == GAME_TRANSFUSION) Draw_TransPic_Scaled (x + ((vid.vconwidth - 320)>>1), y, pic); // transfusion uses transparencies a lot for the main menu. Maybe this can be default?
	else Draw_Pic_Scaled (x + ((vid.vconwidth - 320)>>1), y, pic);
	else
	Draw_Pic (x + ((vid.width - 320)>>1), y, pic);
#else
	Draw_Pic (x + ((vid.width - 320)>>1), y, pic);
#endif
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
#ifdef SCALED2D
	if (menu_scaled)
	Draw_TransPicTranslate_Scaled (x + ((vid.vconwidth - 320)>>1), y, pic, translationTable);
	else
#endif
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

	if (qbeta){
	M_DrawTransPic (0, 0, Draw_CachePic ("gfx/mainmenu.lmp") );
	}
	else
	{
		if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_main.lmp");

	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mainmenu.lmp") );
		}
	}
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	if (qbeta) f = (int)(realtime * 10)%2;	else	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
if(gamemode != GAME_LASER_ARENA)
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

// leilei - xb controller standard

	case K_AUX29:
		S_LocalSound ("misc/menu1.wav");
		if (--m_main_cursor < 0)
			m_main_cursor = MAIN_ITEMS - 1;
		break;

	case K_AUX31:
		S_LocalSound ("misc/menu1.wav");
		if (++m_main_cursor >= MAIN_ITEMS)
			m_main_cursor = 0;
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_sgl.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/sp_menu.lmp") );
	}
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	if (qbeta) f = (int)(realtime * 10)%2;	else	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
if(gamemode != GAME_LASER_ARENA)
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
		if (loadscreen->value)
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mp_menu.lmp") );
	}
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	if (qbeta) f = (int)(realtime * 10)%2;	else	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
#ifdef QSB_NET 
	//	leilei - warn, because it most likely will not work.
	M_PrintWhite ((320/2) - ((27*8)/2), 148, "WARNING: This build is an\nexperi-");
	M_PrintWhite ((320/2) - ((27*8)/2), 156, "mental build with a highly raised ");
	M_PrintWhite ((320/2) - ((27*8)/2), 164, "max datagram value. Compatibility ");
	M_PrintWhite ((320/2) - ((27*8)/2), 172, "with other clients not guaranteed.");
	M_PrintWhite ((320/2) - ((27*8)/2), 180, " USE AT YOUR OWN RISK!!! PERIOD!!!");

#endif
if(gamemode != GAME_LASER_ARENA)
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
	case K_JOY2:
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
	case K_JOY1:
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");

	M_DrawPic ( (320-p->width)/2, 4, p);
	}
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

	case K_JOY2:
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

	case K_JOY1:
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	}
	M_DrawPic ( (320-p->width)/2, 4, p);

	f = 32;

	if (serialAvailable)
	{
		p = Draw_CachePic ("gfx/netmen1.lmp");
	}
	else
	{
		p = Draw_CachePic ("gfx/dim_modm.lmp"); // leilei - dim the modem thing.
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
		p = Draw_CachePic ("gfx/dim_drct.lmp");
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
	
	M_Print (f, 142, net_helpMessage[m_net_cursor*4+0]);
	M_Print (f, 150, net_helpMessage[m_net_cursor*4+1]);
	M_Print (f, 158, net_helpMessage[m_net_cursor*4+2]);
	M_Print (f, 166, net_helpMessage[m_net_cursor*4+3]);

// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  start
//	f = (int)(host_time * 10)%6;
	if (qbeta) f = (int)(realtime * 10)%2;	else	f = (int)(realtime * 10)%6;
// 2001-10-20 TIMESCALE extension by Tomaz/Maddes  end
	if(gamemode != GAME_LASER_ARENA)
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
	case K_JOY2:
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
case K_JOY1:
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

}
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

void M_DrawCheckboxMidi (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "midi");
	else if (on == 2)
		M_Print (x, y, "mod");
	else if (on == 3)
		M_Print (x, y, "ogg");  // someday...
	else
		M_Print (x, y, "cd");	
}


void M_DrawCheckboxPset (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "glquake");
	else if (on == 2)
		M_Print (x, y, "2001");
	else if (on == 3)
		M_Print (x, y, "transfusn");
	else
		M_Print (x, y, "classic");
}



void M_DrawCheckboxFlamehack (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "normal");
	else if (on == 2)
		M_Print (x, y, "additive");
	else if (on == 3)
		M_Print (x, y, "particles");
	else
		M_Print (x, y, "normal");
}



void M_DrawCheckboxShading (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "normal");
	else if (on == 2)
		M_Print (x, y, "enhanced");
	else if (on == 3)
		M_Print (x, y, "bold");
	else
		M_Print (x, y, "simple");
}

void M_DrawCheckboxFlares (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "normal");
	else if (on == 2)
		M_Print (x, y, "dyn. lights");
	else if (on == 3)
		M_Print (x, y, "stupid");
	else
		M_Print (x, y, "none");
}


void M_DrawCheckboxPsprites (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "normal");
	else if (on == 2)
		M_Print (x, y, "glquake");
	else if (on == 3)
		M_Print (x, y, "stupid");
	else
		M_Print (x, y, "disabled");
}


void M_DrawCheckboxBlood (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "increased");
	else if (on == 2)
		M_Print (x, y, "pressured");
	else if (on == 3)
		M_Print (x, y, "omfg!");
	else
		M_Print (x, y, "normal");
}

void M_DrawCheckboxWaterQuality (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "refractions");
	else if (on == 2)
		M_Print (x, y, "reflections");
	else
		M_Print (x, y, "none");
}



void M_DrawCheckboxBlood2 (int x, int y, int on)
{
	if (on == 4)
		M_Print (x, y, "gushy");
	else if (on == 8)
		M_Print (x, y, "spurty");
	else if (on == 12)
		M_Print (x, y, "ketchup");
	else if (on == 16)
		M_Print (x, y, "brutal");
	else if (on <0)
		M_Print (x, y, "removed");
	
	
	

	else if (on > 665)
		M_Print (x, y, "you're evil");	
	else if (on > 96)
		M_Print (x, y, "thats too much");
	else if (on > 80)
		M_Print (x, y, "you monster");
	else if (on > 64)
		M_Print (x, y, "get out");
	else if (on > 48)
		M_Print (x, y, "insane");
	else if (on > 24)
		M_Print (x, y, "you're ill");
	
	

	else
		M_Print (x, y, "original");
}
void M_DrawCheckboxLoad (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "some");
	else if (on == 2)
		M_Print (x, y, "always");
	else if (on == 3)
		M_Print (x, y, "too much");
	else
		M_Print (x, y, "none");
}


void M_DrawCheckboxDeathcam (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "yes");
	else if (on == 2)
		M_Print (x, y, "arena");
	else if (on == 3)
		M_Print (x, y, "tournament");
	else if (on == 4)
		M_Print (x, y, "a shame");
	else
		M_Print (x, y, "no");
}


void M_DrawCheckboxAspect (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "hor+ Qspasm");
	else if (on == 2)
		M_Print (x, y, "hor+ MH");
	else if (on == 3)
		M_Print (x, y, "idunnolol");
	else if (on == 4)
		M_Print (x, y, "what");
	else
		M_Print (x, y, "vert-");
}

void M_DrawCheckboxVirtual (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "Crap");
	else if (on == 2)
		M_Print (x, y, "Low");
	else if (on == 3)
		M_Print (x, y, "320x400");
	else if (on == 4)
		M_Print (x, y, "360x480");
	else if (on == 5)
		M_Print (x, y, "640x400");
	else
		M_Print (x, y, "Normal");
}


void M_DrawCheckboxCLight (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "low");
	else if (on == 2)
		M_Print (x, y, "high");
	else if (on == 3)
		M_Print (x, y, "ultra(dither)");
	else
		M_Print (x, y, "off");
}


void M_DrawCheckboxTexFilter (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "kernel");
	else if (on == 2)
		M_Print (x, y, "bilinear");
	else if (on == 3)
		M_Print (x, y, "error diffusion");
	else
		M_Print (x, y, "nearest");
}

void M_DrawCheckboxBobmodel (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "arc");
	else if (on == 3)
		M_Print (x, y, "fig. 8");
	else if (on == 4)
		M_Print (x, y, "subtl 8");
	else if (on == 5)
		M_Print (x, y, "ads");
	else if (on == 6)
		M_Print (x, y, "ads 2");
	else if (on == 7)
		M_Print (x, y, "6");
	else if (on == 8)
		M_Print (x, y, "7");
	else if (on == 9)
		M_Print (x, y, "8");
	else if (on == 10)
		M_Print (x, y, "9");
	else if (on == 2)
		M_Print (x, y, "arc 2");
	else
		M_Print (x, y, "thrust");
}


void M_DrawCheckboxDetail (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "medium");
	else if (on == 2)
		M_Print (x, y, "low");
	else if (on == 3)
		M_Print (x, y, "crap");
	else
		M_Print (x, y, "high");
}


void M_DrawCheckboxWblend (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "additive");
	else if (on == 2)
		M_Print (x, y, "multiply");
	else if (on == 3)
		M_Print (x, y, "tint");
	else
		M_Print (x, y, "alpha");
}



void M_DrawCheckboxSmode (int x, int y, int on)
{
	if (on == 1)
		M_Print (x, y, "2x2");
	else if (on == 2)
		M_Print (x, y, "2x1");
	else if (on == 3)
		M_Print (x, y, "1x2");
	else
		M_Print (x, y, "1x1");
}

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

		case MENU_CUSTOMIZE_CONTROLS2:
			M_Print (16, y, "        Extra Controls");
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

		case MENU_PRESET_Q101:
			M_Print (16, y, "      Feel: Quake 1.01");
			y += 8;
			break;
		case MENU_PRESET_Q107:
			M_Print (16, y, "      Feel: Quake 1.07");
			y += 8;
			break;
		case MENU_PRESET_GLQ:
			M_Print (16, y, "      Feel:    GLQuake");
			y += 8;
			break;
		case MENU_PRESET_Q64:
			M_Print (16, y, "      Feel:    Quake64");
			y += 8;
			break;
		case MENU_PRESET_D:
			M_Print (16, y, "      Feel:       damn");
			y += 8;
			break;
		case MENU_PRESET_U:
			M_Print (16, y, "      Feel:     fake  ");
			y += 8;
			break;
		case MENU_PRESET_XTREEM:
			M_Print (16, y, "      Feel: XTREEM!!!!");
			y += 8;
			break;
		case MENU_PRESET_LEI:
			M_Print (16, y, "      Feel:  My choice");
			y += 8;
			break;
		case MENU_PRESET_CRAP:
			M_Print (16, y, "      Feel:       Crap");
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

		case MENU_MIDI_VOLUME:
			M_Print (16, y, "     MIDI Music Volume");
			r = midivolume->value;
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_SOUND_VOLUME:
			M_Print (16, y, "          Sound Volume");
			r = volume->value;
			M_DrawSlider (220, y, r);
			y += 8;
			break;
#ifndef BENCH
		case MENU_SOUND_PITCH:
			M_Print (16, y, "        Pitch Variancy");
			r = (s_pitchin->value)/2;
			M_DrawSlider (220, y, r);
			y += 8;
			break;
#endif

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

		case MENU_EXTERNAL_LIT:
			M_Print (16, y, "  Colored Light (.LIT)");
			M_DrawCheckbox (220, y, external_lit->value);
			y += 8;
			break;

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
		case MENU_INTERNAL_OPTIONS:
			M_Print (16, y, "      Advanced options");
			y += 8;
			break;

		// Video options menu
		case MENU_VIDEO_OPTIONS:
			M_Print (16, y, "         Video options");
			y += 8;
			break;
		case MENU_QUALITY_OPTIONS:
			M_Print (16, y, "       Quality options");
			y += 8;
			break;
		case MENU_PRESET_OPTIONS:
			M_Print (16, y, "               Presets");
			y += 8;
			break;
		case MENU_HUD_OPTIONS:
			M_Print (16, y, "           HUD options");
			y += 8;
			break;
		case MENU_CONTENT_OPTIONS:
			M_Print (16, y, "         Content hacks");
			y += 8;
			break;
		case MENU_VIEW_OPTIONS:
			M_Print (16, y, "          View options");
			y += 8;
			break;


		case MENU_BROKEN_OPTIONS:
			M_Print (16, y, "       FIXME FIXME FIX");
			y += 8;
			break;

		case MENU_COLORMAP_OPTIONS:
			M_Print (16, y, "       Shading Options");
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

		case MENU_CONTRAST:
			M_Print (16, y, "             Intensity");
			r = (v_contrast->value - 1) / 2;
			M_DrawSlider (220, y, r);
			y += 8;
			break;


		case MENU_SATURATION:
			M_Print (16, y, "                 Color");
			r = (v_saturation->value) / 3;
			M_DrawSlider (220, y, r);
			y += 8;
			break;


		case MENU_BLOODLEVEL:
			M_Print (16, y, "        Violence Level");
			r = (r_particleblood->value) / 16;
			//M_DrawSlider (220, y, r);
			M_DrawCheckboxBlood2 (220, y, r_particleblood->value);
			y += 8;
			break;

		case MENU_WATERQUALITY:
			M_Print (16, y, "         Water Effects");
			r = (r_waterquality->value) / 2;
			
			M_DrawCheckboxWaterQuality (220, y, r_waterquality->value);
			y += 8;
			break;


		case MENU_CON_ALPHA:
			M_Print (16, y, "  Console transparency");
			r = (1.0 - con_alpha->value);
			M_DrawSlider (220, y, r);
			y += 8;
			break;
		case MENU_WATERALPHA:
			M_Print (16, y, "         Water Opacity");
			r = (r_wateralpha->value);
			M_DrawSlider (220, y, r);
			y += 8;
			break;

		case MENU_COLOR:
			M_Print (16, y, "       Menu Fade Color");
			r = (r_menucolor->value)/16;
			M_DrawSlider (220, y, r);
			y += 8;
			break;


		case MENU_COLORED:
			M_Print (16, y, "      Colored Lighting");
			M_DrawCheckboxCLight (220, y, r_coloredlights->value);
			//M_DrawCheckbox (220, y, r_coloredlights->value);
			y += 8;
			break;
			#ifdef _WIN32
		case MENU_STRETCHMODE:
			M_Print (16, y, "    Video Stretch Mode");
			M_DrawCheckboxSmode (220, y, vid_stretch_by_2->value);
			y += 8;
			break;
#endif
		case MENU_COLOREDDYNS:
			M_Print (16, y, "     Colored dynlights");
			M_DrawCheckbox (220, y, r_coloreddyns->value);
			y += 8;
			break;
		case MENU_OVERBRIGHT:
			M_Print (16, y, "     Overbright Models");
			M_DrawCheckbox (220, y, r_overbrightmdl->value);
			y += 8;
			break;
		case MENU_SCREENSCALE:
			M_Print (16, y, "        Graphics Scale");
			r = (scr_scale->value - 1)/ 3;
			M_DrawSlider (220, y, r);
			y += 8;
			break;
		case MENU_RETROSCALE:
			M_Print (16, y, "     Automatic Scaling");
			M_DrawCheckbox (220, y, scr_retroscale->value);
			y += 8;
		break;
			case MENU_HUDSWAP:
			M_Print (16, y, "      hud on left side");
			M_DrawCheckbox (220, y, cl_hudswap->value);
			y += 8;
			break;
			case MENU_OLDSTATBAR:
			M_Print (16, y, "    use old status bar");
			M_DrawCheckbox (220, y, cl_sbar->value);
			y += 8;
			break;
		case MENU_SPRAY:
			M_Print (16, y, "       spray particles");
			M_DrawCheckbox (220, y, r_particlespray->value);
			y += 8;
			break;
		case MENU_BLOODHACK:
			M_Print (16, y,	"			blood level");
			M_DrawCheckboxBlood (220, y, r_particleblood->value);
			y += 8;
			break;
		case MENU_PARTICLESET:
			M_Print (16, y, "          particle set");
			M_DrawCheckboxPset (220, y, r_particleset->value);
			y += 8;
			break;

		case MENU_LOWDETAIL:
			M_Print (16, y, "    virtual resolution");
			M_DrawCheckboxVirtual (220, y, r_virtualmode->value);
			y += 8;
			break;
		case MENU_LOWWORLD:
			M_Print (16, y, "         fast lighting");
			M_DrawCheckbox (220, y, r_lowworld->value);
			y += 8;
			break;

		case MENU_FILTERING:
			M_Print (16, y, "     texture filtering");
			M_DrawCheckbox (220, y, r_filter->value);
			y += 8;
			break;

		case MENU_DITHERING:
			M_Print (16, y, "             dithering");
			M_DrawCheckbox (220, y, r_dither->value);
			y += 8;
			break;

		case MENU_SHADOWHACK:
			M_Print (16, y, "         model shadows");
			M_DrawCheckbox (220, y, r_shadowhack->value);
			y += 8;
			break;

		case MENU_DYNAMIC:
			M_Print (16, y, "        dynamic lights");
			M_DrawCheckbox (220, y, r_dynamic->value);
			y += 8;
			break;


		case MENU_MUZZLEBLEND:
			M_Print (16, y, "     muzzleflash blend");
			M_DrawCheckbox (220, y, r_muzzlehack->value);
			y += 8;
			break;

		case MENU_SHINYGRAYS:
			M_Print (16, y, "          chrome metal");
			M_DrawCheckbox (220, y, r_shinygrays->value);
			y += 8;
			break;



		case MENU_LOWMODELS:
			M_Print (16, y, "    The Worst Models !");
			M_DrawCheckbox (220, y, r_lowmodels->value);
			y += 8;
			break;

		case MENU_LERPMODELS:
			M_Print (16, y, "   Model Interpolation");
			M_DrawCheckbox (220, y, r_lerpmodels->value);
			y += 8;
			break;
		
		case MENU_TRANQUALITY:
			M_Print (16, y, "       HQ Water Lookup");
			M_DrawCheckbox (220, y, r_tranquality->value);
			y += 8;
			break;
		case MENU_MIPDETAIL:
			M_Print (16, y, "        Texture Detail");
			M_DrawCheckboxDetail (220, y, d_mipdetail->value);
			y += 8;
			break;
		case MENU_ALPHASHIFT:
			M_Print (16, y, "      Alpha Colorshift");
			M_DrawCheckbox (220, y, r_alphashift->value);
			y += 8;
			break;

#ifndef BENCH
	case MENU_CLASSICSPAT:
			M_Print (16, y, "       Old Attenuation");
			M_DrawCheckbox (220, y, s_oldspatial->value);
			y += 8;
			break;
#endif

		case MENU_LIGHTQUALITY:
			M_Print (16, y, "   HQ Colored Lighting");
			M_DrawCheckbox (220, y, r_lightquality->value);
			y += 8;
			break;
		case MENU_SHADEDITHER:
			M_Print (16, y, "      Dithered Shading");
			M_DrawCheckbox (220, y, r_shadedither->value);
			y += 8;
			break;
		case MENU_SHADING:
			M_Print (16, y, "         Model Shading");
			M_DrawCheckboxShading (220, y, r_shading->value);
			y += 8;
			break;

		case MENU_CON_HEIGHT:
			M_Print (16, y, "        Console height");
			r = scr_conheight->value;
			M_DrawSlider (220, y, r);
			y += 8;
			break;
		case MENU_PARTICLESPRITES:
			M_Print (16, y, "      Particle Sprites");
			r = (r_particlesprite->value)/2;
			M_DrawCheckboxPsprites (220, y, r_particlesprite->value);
			y += 8;
			break;

		case MENU_FLAMEHACK:
			M_Print (16, y, "          Modify Fires");
			r = (r_flamehack->value)/2;
			M_DrawCheckboxFlamehack (220, y, r_flamehack->value);
			y += 8;
			break;
		case MENU_FLARES:
			M_Print (16, y, "   Light Corona Flares");
			r = (r_flares->value)/2;
			M_DrawCheckboxFlares (220, y, r_flares->value);
			y += 8;
			break;
		case MENU_WATERBLEND:
			M_Print (16, y, "      Water Blend Mode");
			r = (r_waterblend->value)/3;
			M_DrawCheckboxWblend (220, y, r_waterblend->value);
			y += 8;
			break;
		case MENU_SHOW_FPS:
			M_Print (16, y, "              Show FPS");
			M_DrawCheckbox (220, y, cl_showfps->value);
			y += 8;
			break;
		case MENU_OVERBRIGHTS:
			M_Print (16, y, "           Overbrights");
			M_DrawCheckbox (220, y, r_overbrightBits->value);
			y += 8;
			break;
		case MENU_FULLBRIGHTS:
			M_Print (16, y, "           Fullbrights");
			M_DrawCheckbox (220, y, r_fullbrights->value);
			y += 8;
			break;
		case MENU_REVERT_COLORMAP:
			M_Print (16, y, "	    Revert Colormap");
			y += 8;
			break;
		case MENU_BOBMODEL:
			M_Print (16, y, "     Weapon Bob Method");
			M_DrawCheckboxBobmodel (220, y, cl_bobmodel->value);
			y += 8;
			break;
		case MENU_LEANMODEL:
			M_Print (16, y, "        Weapon Leaning");
			M_DrawCheckbox (220, y, cl_leanmodel->value);
			y += 8;
			break;
		case MENU_FOLLOWMODEL:
			M_Print (16, y, "      Weapon Following");
			M_DrawCheckbox (220, y, cl_followmodel->value);
			y += 8;
			break;
		case MENU_GUNDRAW:
			M_Print (16, y, "        Weapon Drawing");
			M_DrawCheckbox (220, y, cl_gundraw->value);
			y += 8;
			break;
		case MENU_GUNHOLD:
			M_Print (16, y, "          Fall Bobbing");
			M_DrawCheckbox (220, y, cl_bobfall->value);
			y += 8;
			break;
		case MENU_UNDERWATER:
			M_Print (16, y, "      Underwater Pitch");
			M_DrawCheckbox (220, y, s_underwater->value);
			y += 8;
			break;
		case MENU_PLAYERDEATH:
			M_Print (16, y, "          Quiet Deaths");
			M_DrawCheckbox (220, y, s_playerdeath->value);
			y += 8;
			break;
		case MENU_GIBBURST:
			M_Print (16, y, "       Gib sounds Gush");
			M_DrawCheckbox (220, y, s_gibs->value);
			y += 8;
			break;
		case MENU_BLOODBURST:
			M_Print (16, y, "          Blood sounds");
			M_DrawCheckbox (220, y, s_blood->value);
			y += 8;
			break;
		case MENU_STANDSTILL:
			M_Print (16, y, "     'standstill' mode");
			M_DrawCheckbox (220, y, sv_standstill->value);
			y += 8;
			break;
#ifdef GLQUAKE
		case MENU_GL_MAXDEPTH:
			M_Print (16, y, "       Max. draw depth");
			M_Print (220, y, va("%i", (int)gl_maxdepth->value));
			y += 8;
			break;
#endif
		case MENU_AUTOSAVER:
			M_Print (16, y, "             Autosaver");
			M_DrawCheckbox (220, y, autosaver->value);
			y += 8;
			break;
		case MENU_LOADSCREEN:
			M_Print (16, y, "        Loading Screen");
			M_DrawCheckboxLoad (220, y, loadscreen->value);
			y += 8;
			break;
		case MENU_DEATHCAM:
			M_Print (16, y, "          Death Camera");
			M_DrawCheckboxDeathcam (220, y, cl_diecam->value);
			y += 8;
			break;
		case MENU_ASPECT:
			M_Print (16, y, "         Aspect Method");
			M_DrawCheckboxAspect (220, y, scr_aspectmode->value);
			y += 8;
			break;
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
				if(gamemode != GAME_LASER_ARENA){
				M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
				p = Draw_CachePic ("gfx/p_option.lmp");
		
				M_DrawPic ( (320-p->width)/2, 4, p);
		}
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

void M_ExecFunction (menu_definition_t *menu_definition, int key)
{
	qboolean	m_changesound;
	int dir;

	// determine direction for sliders
	dir = 0;
	switch (key)
	{
		case K_RIGHTARROW:
		case K_JOY1:
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
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = NULL;
				M_Menu_Main_f ();
			}
			break;

		// Options menu
		case MENU_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_options;
				current_cursor = &options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_CUSTOMIZE_CONTROLS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = NULL;
				M_Menu_Keys_f ();
			}
			break;

		case MENU_CUSTOMIZE_CONTROLS2:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = NULL;
				M_Menu_Keys_f2 ();
			}
			break;

		case MENU_GO_TO_CONSOLE:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = NULL;
				m_state = m_none;
				Con_ToggleConsole_f ();
			}
			break;

		case MENU_LOAD_DEFAULT_CFG:
			if (key == K_ENTER  || key == K_JOY1 )
			{
				Cbuf_AddText ("exec default.cfg\n");
				m_changesound = true;
			}
			break;

		case MENU_PRESET_Q101:
			if (key == K_ENTER){Preset_Q101();	m_changesound = true;}break;
		case MENU_PRESET_Q107:
			if (key == K_ENTER){Preset_Q107();	m_changesound = true;}break;
		case MENU_PRESET_GLQ:
			if (key == K_ENTER){Preset_GLQ();	m_changesound = true;}break;
		case MENU_PRESET_Q64:
			if (key == K_ENTER){Preset_Q64();	m_changesound = true;}break;
		case MENU_PRESET_D:
			if (key == K_ENTER){Preset_D();	m_changesound = true;}break;
		case MENU_PRESET_U:
			if (key == K_ENTER){Preset_U();	m_changesound = true;}break;
		case MENU_PRESET_XTREEM:
			if (key == K_ENTER){Preset_Xtreem();	m_changesound = true;}break;
		case MENU_PRESET_LEI:
			if (key == K_ENTER){Preset_Lei();	m_changesound = true;}break;
		case MENU_PRESET_CRAP:
			if (key == K_ENTER){Preset_Crap();	m_changesound = true;}break;

		case MENU_VIDEO_RESOLUTION:
			if (key == K_ENTER || key == K_JOY1|| key == K_ESCAPE)
			{
				current_menu = NULL;
				M_Menu_Video_f ();
			}
			break;

		// Control options menu
		case MENU_CONTROL_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
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
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_sound_options;
				current_cursor = &sound_options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_BROKEN_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_broken_options;
				current_cursor = &sound_options_cursor;
				m_entersound = true;
			}
			break;
		case MENU_COLORMAP_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_colormap_options;
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
		case MENU_MIDI_VOLUME:
			if (dir != 0)
			{
				midivolume->value += dir * 0.1;
				if (midivolume->value < 0)
					midivolume->value = 0;
				if (midivolume->value > 1)
					midivolume->value = 1;
				Cvar_SetValue (midivolume, midivolume->value);
#ifndef BENCH
				MIDI_Update();
#endif
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
#ifndef BENCH
		case MENU_SOUND_PITCH:
			if (dir != 0)
			{
				s_pitchin->value += dir * 0.1;
				if (s_pitchin->value < 0)
					s_pitchin->value = 0;
				if (s_pitchin->value > 2)
					s_pitchin->value = 2;
				Cvar_SetValue (s_pitchin, s_pitchin->value);
				m_changesound = true;
			}
			break;
#endif
		// External data menu
		case MENU_EXTERNAL_DATA:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
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


		case MENU_EXTERNAL_LIT:
			if (dir != 0)
			{
				Cvar_SetValue (external_lit, !external_lit->value);
				m_changesound = true;
			}
			break;


		// Client options menu
		case MENU_CLIENT_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
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
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
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
			if (key == K_ENTER || key == K_JOY1)
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

		case MENU_INTERNAL_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_internal_options;
				current_cursor = &internal_options_cursor;
				m_entersound = true;
			}
			break;



		// Video options menu
		case MENU_VIDEO_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_video_options;
				current_cursor = &video_options_cursor;
				m_entersound = true;
			}
			break;

			// Quality options menu
		case MENU_QUALITY_OPTIONS:
			if (key == K_ENTER || key == K_JOY1  || key == K_ESCAPE)
			{
				current_menu = m_menu_quality_options;
				current_cursor = &quality_options_cursor;
				m_entersound = true;
			}
			break;

		case MENU_PRESET_OPTIONS:
			if (key == K_ENTER || key == K_JOY1  || key == K_ESCAPE)
			{
				current_menu = m_menu_preset_options;
				current_cursor = &preset_options_cursor;
				m_entersound = true;
			}
			break;
		case MENU_HUD_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_hud_options;
				current_cursor = &hud_options_cursor;
				m_entersound = true;
			}
			break;


		case MENU_CONTENT_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_content_options;
				current_cursor = &content_options_cursor;
				m_entersound = true;
			}
			break;

			// View options menu
		case MENU_VIEW_OPTIONS:
			if (key == K_ENTER || key == K_JOY1 || key == K_ESCAPE)
			{
				current_menu = m_menu_view_options;
				current_cursor = &view_options_cursor;
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

		case MENU_CONTRAST:
			if (dir != 0)
			{
				v_contrast->value += dir * 0.2;
				if (v_contrast->value < 1)
					v_contrast->value = 1;
				if (v_contrast->value > 3)
					v_contrast->value = 3;
				Cvar_SetValue (v_contrast, v_contrast->value);
				m_changesound = true;
			}
			break;

		case MENU_SATURATION:
			if (dir != 0)
			{
				v_saturation->value += dir * 0.2;
				if (v_saturation->value < 0)
					v_saturation->value = 0;
				if (v_saturation->value > 3)
					v_saturation->value = 3;
				Cvar_SetValue (v_saturation, v_saturation->value);
				m_changesound = true;
			}
			break;

		case MENU_BLOODLEVEL:
			if (dir != 0)
			{
				r_particleblood->value += dir * 4;
				if (r_particleblood->value < -4)
					r_particleblood->value = -4;
				if (r_particleblood->value > 16)
					r_particleblood->value = 16;
				Cvar_SetValue (r_particleblood, r_particleblood->value);
				m_changesound = true;
			}
			break;

		case MENU_WATERQUALITY:
			if (dir != 0)
			{
				r_waterquality->value += dir * 1;
				if (r_waterquality->value < 0)
					r_waterquality->value = 0;
				if (r_waterquality->value > 2)
					r_waterquality->value = 2;
				Cvar_SetValue (r_waterquality, r_waterquality->value);
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

		case MENU_WATERALPHA:
			if (dir != 0)
			{
					r_wateralpha->value += dir * 0.05;
				if (r_wateralpha->value < 0)
					r_wateralpha->value = 0;
				if (r_wateralpha->value > 1)
					r_wateralpha->value = 1;
				Cvar_SetValue (r_wateralpha, r_wateralpha->value);
				m_changesound = true;
			}
			break;

		case MENU_REVERT_COLORMAP:
			if (key == K_ENTER || key == K_JOY1)
			{
				ColormapForceLoad();
				m_changesound = true;
			}
			break;
		case MENU_COLOR:
			if (dir != 0)
			{
					r_menucolor->value += dir * 1;
				if (r_menucolor->value < 0)
					r_menucolor->value = 0;
				if (r_menucolor->value > 15)
					r_menucolor->value = 15;
				Cvar_SetValue (r_menucolor, r_menucolor->value);
				m_changesound = true;
			}
			break;


		case MENU_WATERBLEND:
			if (dir != 0)
			{
					r_waterblend->value += dir * 1;
				if (r_waterblend->value < 0)
					r_waterblend->value = 3;
				if (r_waterblend->value > 3)
					r_waterblend->value = 0;
				Cvar_SetValue (r_waterblend, r_waterblend->value);
				m_changesound = true;
			}
			break;
#ifdef _WIN32
		case MENU_STRETCHMODE:
			if (dir != 0)
			{
					vid_stretch_by_2->value += dir * 1;
				if (vid_stretch_by_2->value < 0)
					vid_stretch_by_2->value = 3;
				if (vid_stretch_by_2->value > 3)
					vid_stretch_by_2->value = 0;
				Cvar_SetValue (vid_stretch_by_2, vid_stretch_by_2->value);
				m_changesound = true;
			}
			break;
#endif

		case MENU_COLORED:
			if (dir != 0)
			{
					r_coloredlights->value += dir * 1;
				if (r_coloredlights->value < 0)
					r_coloredlights->value = 3;
				if (r_coloredlights->value > 3)
					r_coloredlights->value = 0;
				Cvar_SetValue (r_coloredlights, r_coloredlights->value);
				m_changesound = true;
			}
			break;
#ifndef GLQUAKE
		case MENU_OVERBRIGHT:
			if (dir != 0)
			{
				Cvar_SetValue (r_overbrightmdl, !r_overbrightmdl->value);
				m_changesound = true;
			}
			break;
					case MENU_MIPDETAIL:
			if (dir != 0)
			{
					d_mipdetail->value += dir * 1;
				if (d_mipdetail->value < 0)
					d_mipdetail->value = 3;
				if (d_mipdetail->value > 3)
					d_mipdetail->value = 0;
				Cvar_SetValue (d_mipdetail, d_mipdetail->value);
				m_changesound = true;
			}
			break;
	case MENU_SCREENSCALE:
			if (dir != 0)
			{
					scr_scale->value += dir * 1;
				if (scr_scale->value < 1)
					scr_scale->value = 1;
				if (scr_scale->value > 4)
					scr_scale->value = 4;
				Cvar_SetValue (scr_scale, scr_scale->value);
				m_changesound = true;
			}
			break;
	case MENU_RETROSCALE:
			if (dir != 0)
			{
				Cvar_SetValue (scr_retroscale, !scr_retroscale->value);
				m_changesound = true;
			}
			break;
	
#endif
		case MENU_SPRAY:
			if (dir != 0)
			{
				Cvar_SetValue (r_particlespray, !r_particlespray->value);
				m_changesound = true;
			}
			break;
		case MENU_BLOODHACK:
			if (dir != 0)
			{
				Cvar_SetValue (r_particleblood, !r_particleblood->value);
				m_changesound = true;
			}
			break;
		case MENU_PARTICLESET:
			if (dir != 0)
			{
					r_particleset->value += dir * 1;
				if (r_particleset->value < 0)
					r_particleset->value = 3;
				if (r_particleset->value > 3)
					r_particleset->value = 0;
				Cvar_SetValue (r_particleset, r_particleset->value);
				m_changesound = true;
			}
			break;
		case MENU_FLAMEHACK:
			if (dir != 0)
			{
					r_flamehack->value += dir * 1;
				if (r_flamehack->value < 0)
					r_flamehack->value = 3;
				if (r_flamehack->value > 3)
					r_flamehack->value = 0;
				Cvar_SetValue (r_flamehack, r_flamehack->value);
				m_changesound = true;
			}
			break;
		case MENU_FLARES:
			if (dir != 0)
			{
					r_flares->value += dir * 1;
				if (r_flares->value < 0)
					r_flares->value = 2;
				if (r_flares->value > 2)
					r_flares->value = 0;
				Cvar_SetValue (r_flares, r_flares->value);
				m_changesound = true;
			}
			break;
		case MENU_PARTICLESPRITES:
			if (dir != 0)
			{
					r_particlesprite->value += dir * 1;
				if (r_particlesprite->value < 0)
					r_particlesprite->value = 2;
				if (r_particlesprite->value > 2)
					r_particlesprite->value = 0;
				Cvar_SetValue (r_particlesprite, r_particlesprite->value);
				m_changesound = true;
			}
			break;
		case MENU_LOADSCREEN:
			if (dir != 0)
			{
					loadscreen->value += dir * 1;
				if (loadscreen->value < 0)
					loadscreen->value = 2;
				if (loadscreen->value > 2)
					loadscreen->value = 0;
				Cvar_SetValue (loadscreen, loadscreen->value);
				m_changesound = true;
			}
			break;

	case MENU_DEATHCAM:
			if (dir != 0)
			{
					cl_diecam->value += dir * 1;
				if (cl_diecam->value < 0)
					cl_diecam->value = 4;
				if (cl_diecam->value > 4)
					cl_diecam->value = 0;
				Cvar_SetValue (cl_diecam, cl_diecam->value);
				m_changesound = true;
			}
			break;

	case MENU_ASPECT:
			if (dir != 0)
			{
					scr_aspectmode->value += dir * 1;
					vid.recalc_refdef = 1;
				if (scr_aspectmode->value < 0)
					scr_aspectmode->value = 2;
				if (scr_aspectmode->value > 2)
					scr_aspectmode->value = 0;
				Cvar_SetValue (scr_aspectmode, scr_aspectmode->value);
				m_changesound = true;
			}
			break;
		case MENU_BOBMODEL:
			if (dir != 0)
			{
					cl_bobmodel->value += dir * 1;
				if (cl_bobmodel->value < 0)
					cl_bobmodel->value = 4;
				if (cl_bobmodel->value > 4)
					cl_bobmodel->value = 0;
				Cvar_SetValue (cl_bobmodel, cl_bobmodel->value);
				m_changesound = true;
			}
			break;
		case MENU_LEANMODEL:
			if (dir != 0)
			{
				Cvar_SetValue (cl_leanmodel, !cl_leanmodel->value);
				m_changesound = true;
			}
			break;
		case MENU_FOLLOWMODEL:
			if (dir != 0)
			{
				Cvar_SetValue (cl_followmodel, !cl_followmodel->value);
				m_changesound = true;
			}
			break;
		case MENU_GUNDRAW:
			if (dir != 0)
			{
				Cvar_SetValue (cl_gundraw, !cl_gundraw->value);
				m_changesound = true;
			}
			break;
		case MENU_GUNHOLD:
			if (dir != 0)
			{
				Cvar_SetValue (cl_bobfall, !cl_bobfall->value);
				m_changesound = true;
			}
			break;
		case MENU_COLOREDDYNS:
			if (dir != 0)
			{
				Cvar_SetValue (r_coloreddyns, !r_coloreddyns->value);
				m_changesound = true;
			}
			break;
		case MENU_OVERBRIGHTS:
			if (dir != 0)
			{
				Cvar_SetValue (r_overbrightBits, !r_overbrightBits->value);
				m_changesound = true;
			}
			break;
		case MENU_FULLBRIGHTS:
			if (dir != 0)
			{
				Cvar_SetValue (r_fullbrights, !r_fullbrights->value);
				m_changesound = true;
			}
			break;
		case MENU_UNDERWATER:
			if (dir != 0)
			{
				Cvar_SetValue (s_underwater, !s_underwater->value);
				m_changesound = true;
			}
			break;
		case MENU_GIBBURST:
			if (dir != 0)
			{
				Cvar_SetValue (s_gibs, !s_gibs->value);
				m_changesound = true;
			}
			break;
		case MENU_BLOODBURST:
			if (dir != 0)
			{
				Cvar_SetValue (s_blood, !s_blood->value);
				m_changesound = true;
			}
			break;
		case MENU_PLAYERDEATH:
			if (dir != 0)
			{
				Cvar_SetValue (s_playerdeath, !s_playerdeath->value);
				m_changesound = true;
			}
			break;
		case MENU_STANDSTILL:
			if (dir != 0)
			{
				Cvar_SetValue (sv_standstill, !sv_standstill->value);
				m_changesound = true;
			}
			break;
#ifndef GLQUAKE
		case MENU_LOWDETAIL:
			if (dir != 0)
			{
					r_virtualmode->value += dir * 1;
				if (r_virtualmode->value < 0)
					r_virtualmode->value = 2;
				if (r_virtualmode->value > 2)
					r_virtualmode->value = 0;
				Cvar_SetValue (r_virtualmode, r_virtualmode->value);
				m_changesound = true;
			}
			break;
		case MENU_SHADING:
			if (dir != 0)
			{
					r_shading->value += dir * 1;
				if (r_shading->value < 0)
					r_shading->value = 2;
				if (r_shading->value > 2)
					r_shading->value = 0;
				Cvar_SetValue (r_shading, r_shading->value);
				m_changesound = true;
			}
			break;
			case MENU_LOWWORLD:
			if (dir != 0)
			{
				Cvar_SetValue (r_lowworld, !r_lowworld->value);
				m_changesound = true;
			}
			break;
			case MENU_FILTERING:
			if (dir != 0)
			{
				Cvar_SetValue (r_filter, !r_filter->value);
				m_changesound = true;
			}
			break;
			case MENU_DITHERING:		// obsolete
			if (dir != 0)
			{
				Cvar_SetValue (r_dither, !r_dither->value);
				m_changesound = true;
			}
			break;
			case MENU_SHADOWHACK:
			if (dir != 0)
			{
				Cvar_SetValue (r_shadowhack, !r_shadowhack->value);
				shadowhack = r_shadowhack->value;
				m_changesound = true;
			}
			break;
			case MENU_DYNAMIC:
			if (dir != 0)
			{
				Cvar_SetValue (r_dynamic, !r_dynamic->value);
				m_changesound = true;
			}
			break;
			case MENU_MUZZLEBLEND:
			if (dir != 0)
			{
				Cvar_SetValue (r_muzzlehack, !r_muzzlehack->value);
				m_changesound = true;
			}
			break;
			case MENU_SHINYGRAYS:
			if (dir != 0)
			{
				Cvar_SetValue (r_shinygrays, !r_shinygrays->value);
				m_changesound = true;
			}
			break;
			case MENU_LOWMODELS:
			if (dir != 0)
			{
				Cvar_SetValue (r_lowmodels, !r_lowmodels->value);
				m_changesound = true;
			}
			break;
			case MENU_LIGHTQUALITY:
			if (dir != 0)
			{
				Cvar_SetValue (r_lightquality, !r_lightquality->value);
				m_changesound = true;
			}
				break;
		case MENU_SHADEDITHER:
			if (dir != 0)
			{
				Cvar_SetValue (r_shadedither, !r_shadedither->value);
				m_changesound = true;
			}
				break;
		case MENU_TRANQUALITY:
			if (dir != 0)
			{
				Cvar_SetValue (r_tranquality, !r_tranquality->value);
				m_changesound = true;
			}
				break;
		case MENU_AUTOSAVER:
			if (dir != 0)
			{
				Cvar_SetValue (autosaver, !autosaver->value);
				m_changesound = true;
			}
				break;
		case MENU_ALPHASHIFT:
			if (dir != 0)
			{
				Cvar_SetValue (r_alphashift, !r_alphashift->value);
				m_changesound = true;
			}
				break;
#endif
		case MENU_HUDSWAP:
			if (dir != 0)
			{
				Cvar_SetValue (cl_hudswap, !cl_hudswap->value);
				m_changesound = true;
			}
			break;
		case MENU_OLDSTATBAR:
			if (dir != 0)
			{
				Cvar_SetValue (cl_sbar, !cl_sbar->value);
				m_changesound = true;
			}
			break;

		
		case MENU_LERPMODELS:
			if (dir != 0)
			{
				Cvar_SetValue (r_lerpmodels, !r_lerpmodels->value);
				m_changesound = true;
			}
			break;
#ifndef BENCH		
		case MENU_CLASSICSPAT:
			if (dir != 0)
			{
				Cvar_SetValue (s_oldspatial, !s_oldspatial->value);
				m_changesound = true;
			}
				break;
#endif		
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
{"impulse 12", 		"reverse cycle"},
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
{"+movedown",		"swim down"},
/*{"+moveup",			"swim up"},
{"+movedown",		"swim down"},		// testing lists that exceed screen size
{"+moveup",			"swim up"},
{"+movedown",		"swim down"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"},  */
};





char *extrabinds[][2] =
{
{"+use", 		"eat"},
{"impulse 9", 		"poop"},
{"disconnect",		"fart down"}
};



void InitCustomKeyList (void)
{
	loadedfile_t	*fileinfo;	// leilei - custom key bind thing lists

	fileinfo = COM_LoadHunkFile ("keys.lst");

	if (!fileinfo)
	{
		Con_Printf ("couldn't load keys.lst! Custom binds not available.");
		return;
	}
	
	//extrabinds = fileinfo->data;
	//	else
//	extrabinds = fileinfo->data;




}

/*
===============
Cmd_Exec_f
===============

void Cmd_Exec_f (void)
{
	char	*f;
	int		mark;
	loadedfile_t	*fileinfo;	// 2001-09-12 Returning information about loaded file by Maddes

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}

	mark = Hunk_LowMark ();
// 2001-09-12 Returning information about loaded file by Maddes  start

	f = (char *)COM_LoadHunkFile (Cmd_Argv(1));
	if (!f)

	fileinfo = COM_LoadHunkFile (Cmd_Argv(1));
	if (!fileinfo)
// 2001-09-12 Returning information about loaded file by Maddes  end
	{
		Con_Printf ("couldn't exec %s\n",Cmd_Argv(1));
		return;
	}
	f = (char *)fileinfo->data;	// 2001-09-12 Returning information about loaded file by Maddes
	Con_Printf ("execing %s\n",Cmd_Argv(1));

	Cbuf_InsertText (f);
	Hunk_FreeToLowMark (mark);
}
*/

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))
int NUMCOMMANDS2;
int		keys_cursor;
int		bind_grab;

void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}

void M_Menu_Keys_f2 (void)
{
	key_dest = key_menu;
	m_state = m_keys2;
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



// For mods

void M_Keys_Draw2 (void)
{
	int		i;	
	int		keys[2];
	char	*name;
	int		x, y;
	
	qpic_t	*p;

	NUMCOMMANDS2 = (sizeof(extrabinds)/sizeof(extrabinds[0]));
	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	if (bind_grab)
		M_Print (12, 32, "Press a key or button for this action");
	else
		M_Print (18, 32, "Enter to change, backspace to clear");

// search for known bindings
	for (i=0 ; i<NUMCOMMANDS2 ; i++)
	{
		y = 48 + 8*i;

		M_Print (16, y, extrabinds[i][1]);

		M_FindKeysForCommand (extrabinds[i][0], keys);

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
	case K_JOY1:
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


void M_Keys_Key2 (int k)
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
			sprintf (cmd, "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), extrabinds[keys_cursor][0]);
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
			keys_cursor = NUMCOMMANDS2-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor++;
		if (keys_cursor >= NUMCOMMANDS2)
			keys_cursor = 0;
		break;

	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (extrabinds[keys_cursor][0], keys);
		S_LocalSound ("misc/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (extrabinds[keys_cursor][0]);
		bind_grab = true;
		break;

	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
		S_LocalSound ("misc/menu2.wav");
		M_UnbindCommand (extrabinds[keys_cursor][0]);
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

//#ifndef	_WIN32
#ifndef PROTO
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
#else
char *quitMessage [] =
{
/* .........1.........2.... */
  " Why are you leaving me ",
  "  for that cruddy OS    ",
  "    you live with??     ",
  "                        ",

  " Ok why don't you just  ",
  "   get the heck out of  ",
  " this game you loser.   ",
  "                        ",

  " Is this game too coool ",
  "  for your face trying  ",
  "        to quit?        ",
  "                        ",

  " Man, I oughta poke  you",
  "   for trying to quit!  ",
  "     Press Y to get     ",
  "        poked out.      ",

  " Press Y to quit like a ",
  "   addicted cig smoker. ",
  "  Press N to quit like  ",
  "    a big loser in life ",

  "   If you press Y to    ",
  "  quit, I will          ",
  "  format ntfs on your   ",
  "      ext3 partition!   ",

  " IF YOU PRESS y YOU LOS ",
  " YOUR CHANCES IN WINNING",
  " A NEW CAR!!!!!!!!!!!!!!",
  "   so prease press N..  ",

  "                        ",
  "    GET OUT OF HERE     ",
  "                        ",
  "                        "
};
#endif
//#endif

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

	if (menu_quitscreen->value == 1){
	
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

	}

	else if (menu_quitscreen->value == 2){
	
	M_DrawTextBox (0, 0, 38, 23);
	M_PrintWhite (16, 12,  "   ENGOO version 2.70              \n\n");
	M_PrintWhite (16, 28,  "                                   \n");
	M_Print (16, 36,	   " Based on QIP Engine by Maddes and \n");
	M_Print (16, 44,       "                               \n");
	M_Print (16, 52,       "                                   \n");
	M_PrintWhite (16, 60,       "  Programming by                   \n");
	M_Print (16, 68,	   "                  leilei           \n");
	M_Print (16, 76,       "                    \n");
	M_PrintWhite (16, 84,  " LOTS of HELP from             \n");
	M_Print (16, 92,       " LordHavoc       - Lighting\n");
	M_Print (16, 100,      " Spike           - Memory\n");
	M_Print (16, 108,	   " Fitz            - Fog\n");
	M_Print (16, 116,      " Qbism           - Optimizations\n");
	M_Print (16, 124,      " Mankrip         - Optimizations\n");
	M_Print (16, 132,      " Andrewj         - Dithering\n");
	M_Print (16, 140,      " Tomaz           - Snow/Rain and Bots\n");
	M_Print (16, 148,      " MH              - Vid code /interpol\n");
	M_Print (16, 156, " Siggi           - 2D Scaling\n");
	M_Print (16, 164, " Taniwha         - Aspect\n");
	M_PrintWhite (16, 172, " This engine is licensed under the\n");
	M_PrintWhite (16, 180, " GNU GPL v2. Read COPYING for more.\n");
	
	}
	else	// old classic dos version threats
	{
	M_DrawTextBox (56, 76, 24, 4);
	M_Print (64, 84,  quitMessage[msgNumber*4+0]);
	M_Print (64, 92,  quitMessage[msgNumber*4+1]);
	M_Print (64, 100, quitMessage[msgNumber*4+2]);
	M_Print (64, 108, quitMessage[msgNumber*4+3]);

	}
}



void M_Quit_Draw_Credits (void)
{
	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

//#ifdef _WIN32
	
//ndif
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	}
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	}
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	}
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
	if(gamemode != GAME_LASER_ARENA){
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	}
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
		if (loadscreen->value)
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

// preset functions here, because we declared lots of cvars already.
extern cvar_t *r_particletrans;
extern cvar_t *r_depthoffield;
extern cvar_t *s_underwater;

extern cvar_t *cl_bobmodel_side;
extern cvar_t *cl_bobmodel_up;
extern cvar_t *cl_bobmodel_speed;
extern cvar_t *cl_bob;



void Preset_Q101 (void) // Quake 1.01
{
	Cvar_Set(cl_bobmodel,		"0");	// thrust
	Cvar_Set(cl_bobmodel_side,	"0.3");
	Cvar_Set(cl_bobmodel_up,	"0.15");
	Cvar_Set(cl_bob,		"0.02");
	Cvar_Set(cl_bobmodel_speed,	"7");
	Cvar_Set(cl_leanmodel,		"0");
	Cvar_Set(cl_followmodel,	"0");
	Cvar_Set(cl_bobfall,		"0");
	Cvar_Set(cl_gundraw,		"0");
	Cvar_Set(r_shading,			"1");	// uniform normal shading
	Cvar_Set(r_menucolor,		"1");	// brown menu
	Cvar_Set(s_pitchin,			"0");	// normal sound pitch
	Cvar_Set(s_oldspatial,		"1");	// long distance spatial
	Cvar_Set(r_flares,			"0");	// don't flares
	Cvar_Set(r_particleset,		"0");	// normal particles
	Cvar_Set(r_particletrans,	"0");	// don't have blend particles
	Cvar_Set(r_particleblood,	"0");	// don't use new blood
	Cvar_Set(r_coloredlights,	"0");	// don't have colored lightin
	Cvar_Set(r_filter,			"0");	// don't filter textures
	Cvar_Set(r_muzzlehack,		"0");	// don't hack the muzzleflash
	Cvar_Set(r_lerpmodels,		"0");	// don't interpolate
	Cvar_Set(r_particlespray,	"0");	// don't spray particles
	Cvar_Set(r_shadowhack,		"0");	// don't model shadows
	Cvar_Set(r_shadedither,		"0");	// don't dither
	Cvar_Set(cl_sbar,			"1");	// use old status bar
	Cvar_Set(d_mipdetail,		"0");	// normal detail
	Cvar_Set(s_underwater,		"0");	// don't sound pitch in water
	Cvar_Set(r_wateralpha,		"1");	// don't blend water
	Cvar_Set(r_waterquality,	"0");	// don't do water fx
	Cvar_Set(r_depthoffield,	"0");	// and the depth of field	
	Cvar_Set(r_lowworld,		"0");
	Cvar_Set(r_dynamic,			"1");	
	Cvar_Set(r_shinygrays,		"0");	// don't shine
	Cvar_Set(cl_diecam,			"0");	
	Cvar_Set(s_gibs,			"0");	
	Cvar_Set(s_playerdeath,		"0");	
	Cvar_Set(s_blood,			"0");	
	fullbrights = 32;	// use the standard colormap fullbrights
	overbrights = 1;	// normal overbrights
	GrabColorMap();		// regenerate colormap
}


void Preset_Q107 (void) // Quake 1.07
{
	Preset_Q101();	// old preset, but...
	Cvar_Set(r_menucolor,		"15");	// dotty menu
	Cvar_Set(s_oldspatial,		"0");	// short distance spatial
}

void Preset_GLQ (void) // GLQuake
{
	Preset_Q101();	// old preset, but...
	Cvar_Set(r_filter,			"1");	// filter textures
	Cvar_Set(r_particleset,		"1");	// glquake particleset (if available)
	Cvar_Set(r_menucolor,		"16");	// alpha blend black menu
	Cvar_Set(r_dynamic,			"0");	// we don't have flashblend yet :(
	Cvar_Set(v_gamma,				"1");	// glquake doesn't do gamma lol
//	Cvar_Set(v_intensity,			"0");	
//	Cvar_Set(contrast,			"0");	
	fullbrights = 0;	// no fullbrights
	overbrights = 0;	// no overbrights
	GrabColorMap();		// regenerate colormap
}

void Preset_Q64 (void) 
{
	Preset_Q101();	// old preset, but...
	Cvar_Set(r_filter,			"2");	// filter textures slantedly
	Cvar_Set(r_particleset,		"1");	// glquake particleset (if available)
	Cvar_Set(r_menucolor,		"16");	// alpha blend black menu
	Cvar_Set(d_mipdetail,		"64");	// low detail because n64 sucks
	Cvar_Set(r_coloredlights,	"1");	
	Cvar_Set(r_dynamic,			"0");	// we don't have flashblend yet :(
	Cvar_Set(r_shading,			"0");	// simple shading	
	fullbrights = 0;	// no fullbrights
	overbrights = 0;	// no overbrights
	GrabColorMap();		// regenerate colormap
}


void Preset_U (void) // U
{
	Cvar_Set(cl_bobmodel,		"3");	// fig 8
	Cvar_Set(cl_leanmodel,		"0");
	Cvar_Set(cl_followmodel,	"0");
	Cvar_Set(cl_bobmodel_side,	"0.3");
	Cvar_Set(cl_bobmodel_up,	"0.15");
	Cvar_Set(cl_bobmodel_speed,	"7");
	Cvar_Set(r_lowworld,		"0");
	Cvar_Set(r_depthoffield,	"0");	// and the depth of field
	Cvar_Set(r_shading,			"2");	// point shading
	Cvar_Set(r_menucolor,		"16");	
	Cvar_Set(s_pitchin,			"1.4");	// normal sound pitch
	Cvar_Set(s_oldspatial,		"1");	// long distance spatial
	Cvar_Set(r_flares,			"2");	// don't flares
	Cvar_Set(r_particleset,		"2");	// normal particles
	Cvar_Set(r_particletrans,	"1");	// don't have blend particles
	Cvar_Set(r_particleblood,	"0");	// don't use new blood
	Cvar_Set(r_coloredlights,	"2");	// don't have colored lightin
	Cvar_Set(r_filter,			"1");	// don't filter textures
	Cvar_Set(r_muzzlehack,		"1");	// don't hack the muzzleflash
	Cvar_Set(r_lerpmodels,		"1");	// don't interpolate
	Cvar_Set(r_particlespray,	"0");	// don't spray particles
	Cvar_Set(r_shadowhack,		"0");	// don't model shadows
	Cvar_Set(r_shadedither,		"1");	// don't dither
	Cvar_Set(cl_sbar,			"0");	// use old status bar
	Cvar_Set(d_mipdetail,		"0");	// normal detail
	Cvar_Set(s_underwater,		"0");	// don't sound pitch in water
	Cvar_Set(r_wateralpha,		"0.6");	// don't blend water
	Cvar_Set(r_waterquality,	"0");	// don't do water fx
	Cvar_Set(r_dynamic,			"1");	
	Cvar_Set(cl_diecam,			"3");	
	Cvar_Set(s_gibs,			"0");	
	Cvar_Set(s_playerdeath,		"1");	
	Cvar_Set(s_blood,			"0");	
	fullbrights = 32;	// use the standard colormap fullbrights
	overbrights = 1;	// normal overbrights
	GrabColorMap();		// regenerate colormap
}


void Preset_Lei (void) // leilei - The kind of settings I prefer myself
{
	Cvar_Set(cl_bobmodel,		"3");	// fig 8
	Cvar_Set(cl_leanmodel,		"1");
	Cvar_Set(cl_followmodel,	"1");
	Cvar_Set(cl_bobfall,		"1");
	Cvar_Set(cl_gundraw,		"1");
	Cvar_Set(cl_bobmodel_side,	"0.3");
	Cvar_Set(cl_bobmodel_up,	"0.15");
	Cvar_Set(cl_bobmodel_speed,	"7");
	Cvar_Set(r_depthoffield,	"0");	// and the depth of field
	Cvar_Set(r_shading,			"2");	// point shading
	Cvar_Set(r_menucolor,		"0");	
	Cvar_Set(s_pitchin,			"2");	// normal sound pitch
	Cvar_Set(s_oldspatial,		"1");	// long distance spatial
	Cvar_Set(r_flares,			"2");	// don't flares
	Cvar_Set(r_particleset,		"2");	// normal particles
	Cvar_Set(r_particletrans,	"1");	// don't have blend particles
	Cvar_Set(r_particleblood,	"9");	// don't use new blood
	Cvar_Set(r_coloredlights,	"2");	// don't have colored lightin
	Cvar_Set(r_filter,			"0");	// don't filter textures
	Cvar_Set(r_muzzlehack,		"1");	// don't hack the muzzleflash
	Cvar_Set(r_lerpmodels,		"1");	// interpolate
	Cvar_Set(r_particlespray,	"0");	// don't spray particles
	Cvar_Set(r_shadowhack,		"1");	// don't model shadows
	Cvar_Set(r_shadedither,		"1");	// don't dither
	Cvar_Set(cl_sbar,			"1");	// use old status bar
	Cvar_Set(d_mipdetail,		"0");	// normal detail
	Cvar_Set(s_underwater,		"1");	// don't sound pitch in water
	Cvar_Set(r_wateralpha,		"0.3");	// don't blend water
	Cvar_Set(r_waterquality,	"1");	// don't do water fx
	Cvar_Set(r_lowworld,		"0");
	Cvar_Set(r_dynamic,			"1");	
	Cvar_Set(cl_diecam,			"3");	
	Cvar_Set(s_gibs,			"1");	
	Cvar_Set(s_playerdeath,		"1");	
	Cvar_Set(s_blood,			"1");
	Cvar_Set(r_shinygrays,		"1");	// do shine
	fullbrights = 32;	// use the standard colormap fullbrights
	overbrights = 1;	// normal overbrights
	GrabColorMap();		// regenerate colormap
}



void Preset_D (void) 
{
	Cvar_Set(cl_bobmodel,		"1");	// Arc
	Cvar_Set(cl_bob,		"0.07");
	Cvar_Set(cl_leanmodel,		"0");
	Cvar_Set(cl_followmodel,	"0");
	Cvar_Set(cl_bobmodel_side,	"0.6");
	Cvar_Set(cl_bobmodel_up,	"-0.5");
	Cvar_Set(cl_bobmodel_speed,	"2");
	Cvar_Set(r_lowworld,		"0");
	Cvar_Set(r_shading,			"0");	// uniform normal shading
	Cvar_Set(r_menucolor,		"17");	// brown menu
	Cvar_Set(s_pitchin,			"1");	// normal sound pitch
	Cvar_Set(s_oldspatial,		"1");	// long distance spatial
	Cvar_Set(r_flares,			"0");	// don't flares
	Cvar_Set(r_particleset,		"0");	// normal particles
	Cvar_Set(r_particletrans,	"0");	// don't have blend particles
	Cvar_Set(r_particleblood,	"0");	// don't use new blood
	Cvar_Set(r_coloredlights,	"0");	// don't have colored lightin
	Cvar_Set(r_filter,			"0");	// don't filter textures
	Cvar_Set(r_muzzlehack,		"0");	// don't hack the muzzleflash
	Cvar_Set(r_lerpmodels,		"0");	// don't interpolate
	Cvar_Set(r_particlespray,	"0");	// don't spray particles
	Cvar_Set(r_shadowhack,		"0");	// don't model shadows
	Cvar_Set(r_shadedither,		"0");	// don't dither
	Cvar_Set(r_shinygrays,		"0");	// don't shine
	Cvar_Set(cl_sbar,			"1");	// use old status bar
	Cvar_Set(d_mipdetail,		"0");	// normal detail
	Cvar_Set(s_underwater,		"0");	// don't sound pitch in water
	Cvar_Set(r_wateralpha,		"1");	// don't blend water
	Cvar_Set(r_waterquality,	"0");	// don't do water fx
	Cvar_Set(r_depthoffield,	"0");	// and the depth of field	
	Cvar_Set(r_dynamic,			"1");	
	Cvar_Set(cl_diecam,			"0");	
	Cvar_Set(s_gibs,			"0");	
	Cvar_Set(s_playerdeath,		"1");	
	Cvar_Set(s_blood,			"0");	
	fullbrights = 32;	// use the standard colormap fullbrights
	overbrights = 0;	// no overbrights
	GrabColorMap();		// regenerate colormap
}

void Preset_Xtreem (void) // Everything set high
{
	Cvar_Set(cl_bobmodel,		"3");	// thrust
	Cvar_Set(cl_bobmodel_side,	"0.3");
	Cvar_Set(cl_bobmodel_up,	"0.15");
	Cvar_Set(cl_bob,		"0.02");
	Cvar_Set(cl_bobmodel_speed,	"7");
	Cvar_Set(cl_leanmodel,		"1");
	Cvar_Set(cl_followmodel,	"1");
	Cvar_Set(r_shading,			"2");	// dithered shading
	Cvar_Set(r_menucolor,		"1");	// brown menu
	Cvar_Set(s_pitchin,			"2");	// normal sound pitch
	Cvar_Set(s_oldspatial,		"1");	// long distance spatial
	Cvar_Set(r_flares,			"2");	// don't flares
	Cvar_Set(r_particleset,		"2");	// normal particles
	Cvar_Set(r_particletrans,	"1");	// don't have blend particles
	Cvar_Set(r_particleblood,	"8");	// don't use new blood
	Cvar_Set(r_coloredlights,	"3");	// don't have colored lightin
	Cvar_Set(r_filter,			"1");	// don't filter textures
	Cvar_Set(r_muzzlehack,		"1");	// don't hack the muzzleflash
	Cvar_Set(r_lerpmodels,		"1");	// don't interpolate
	Cvar_Set(r_particlespray,	"0");	// don't spray particles
	Cvar_Set(r_shadowhack,		"1");	// don't model shadows
	Cvar_Set(r_shadedither,		"1");	// don't dither
	Cvar_Set(cl_sbar,			"1");	// use old status bar
	Cvar_Set(d_mipdetail,		"-1");	// normal detail
	Cvar_Set(s_underwater,		"1");	// don't sound pitch in water
	Cvar_Set(r_wateralpha,		"0.3");	// don't blend water
	Cvar_Set(r_waterquality,	"2");	// don't do water fx
	Cvar_Set(r_depthoffield,	"1");	// and the depth of field
	Cvar_Set(r_lowworld,		"0");
	Cvar_Set(r_dynamic,			"1");	
	Cvar_Set(s_gibs,			"1");	
	Cvar_Set(s_blood,			"1");		
	

	fullbrights = 32;	// use the standard colormap fullbrights
	overbrights = 1;	// normal overbrights
	GrabColorMap();		// regenerate colormap
}


void Preset_Crap (void) // Everything set low or possibly beyond low
{
	Cvar_Set(cl_bobmodel,		"0");	// thrust
	Cvar_Set(cl_bobmodel_side,	"0.3");
	Cvar_Set(cl_bobmodel_up,	"0.15");
	Cvar_Set(cl_bob,		"0.02");
	Cvar_Set(cl_bobmodel_speed,	"7");
	Cvar_Set(cl_leanmodel,		"0");
	Cvar_Set(cl_followmodel,	"0");
	Cvar_Set(r_lowworld,		"1");
	Cvar_Set(r_shading,			"0");	// dithered shading
	Cvar_Set(r_menucolor,		"15");	// brown menu
	Cvar_Set(s_pitchin,			"0");	// normal sound pitch
	Cvar_Set(s_oldspatial,		"0");	// long distance spatial
	Cvar_Set(r_flares,			"0");	// don't flares
	Cvar_Set(r_particleset,		"0");	// normal particles
	Cvar_Set(r_particletrans,	"0");	// don't have blend particles
	Cvar_Set(r_particleblood,	"0");	// don't use new blood
	Cvar_Set(r_coloredlights,	"0");	// don't have colored lightin
	Cvar_Set(r_filter,			"0");	// don't filter textures
	Cvar_Set(r_muzzlehack,		"0");	// don't hack the muzzleflash
	Cvar_Set(r_lerpmodels,		"0");	// don't interpolate
	Cvar_Set(r_particlespray,	"0");	// don't spray particles
	Cvar_Set(r_shadowhack,		"0");	// don't model shadows
	Cvar_Set(r_shadedither,		"0");	// don't dither
	Cvar_Set(cl_sbar,			"1");	// use old status bar
	Cvar_Set(d_mipdetail,		"9");	// normal detail
	Cvar_Set(s_underwater,		"0");	// don't sound pitch in water
	Cvar_Set(r_wateralpha,		"1");	// don't blend water
	Cvar_Set(r_waterquality,	"0");	// don't do water fx
	Cvar_Set(r_depthoffield,	"0");	// and the depth of field
	Cvar_Set(r_dynamic,			"0");	
	Cvar_Set(cl_diecam,			"0");	
	Cvar_Set(s_gibs,			"0");	
	Cvar_Set(s_blood,			"0");	
	

	fullbrights = 32;	// use the standard colormap fullbrights
	overbrights = 1;	// normal overbrights
	GrabColorMap();		// regenerate colormap
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
	Cmd_AddCommand ("menu_keys2", M_Menu_Keys_f2);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
	Cmd_AddCommand ("help", M_Menu_Help_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);

	Cmd_AddCommand ("preset_q101",	Preset_Q101);
	Cmd_AddCommand ("preset_q107",	Preset_Q107);
	Cmd_AddCommand ("preset_glq",	Preset_GLQ);
	Cmd_AddCommand ("preset_d",	Preset_D);
	Cmd_AddCommand ("preset_u",	Preset_U);
	Cmd_AddCommand ("preset_q64",	Preset_Q64);

	menu_quitscreen = Cvar_Get ("menu_quitscreen", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
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

	case m_keys2:
		M_Keys_Draw2 ();
		break;

//	case m_broken:
//		M_Broken_Draw ();
//		break;


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

	case m_keys2:
		M_Keys_Key2 (key);
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


