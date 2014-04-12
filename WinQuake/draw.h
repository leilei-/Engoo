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

// draw.h -- these are the only functions outside the refresh allowed
// to touch the vid buffer

extern	qpic_t		*draw_disc;	// also used on sbar
extern unsigned char d_15to8table[65536];
void Draw_Init (void);
void Draw_Character (int x, int y, int num);
void Draw_DebugChar (char num);
void Draw_Pic (int x, int y, qpic_t *pic);
void Draw_TransPic (int x, int y, qpic_t *pic);
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation);
void Draw_ConsoleBackground (int lines);
void Draw_BeginDisc (void);
void Draw_EndDisc (void);
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FadeScreen (void);
void Draw_String (int x, int y, char *str);
qpic_t *Draw_PicFromWad (char *name);
qpic_t *Draw_CachePic (char *path);
// Scaled up versions

void Draw_Pic_Scaled (int x, int y, qpic_t *pic);
void Draw_PicCropped (int x, int y, qpic_t *pic);
void Draw_PicCropped_Scaled (int x, int y, qpic_t *pic);

void Draw_TransPic_Scaled (int x, int y, qpic_t *pic);
void Draw_TransPicTranslate_Scaled (int x, int y, qpic_t *pic, byte *translation);
void Draw_TransPicCropped (int x, int y, qpic_t *pic);
void Draw_TransPicCropped_Scaled (int x, int y, qpic_t *pic);

void Draw_Fill_Scaled (int x, int y, int w, int h, int c);

void Draw_Character_Scaled (int x, int y, unsigned int num);
void Draw_SmallString_Scaled (int x, int y, const char *str);
//void Draw_BigCharacter_Scaled (int x, int y, int num);
// BestColor

#define APART(c)			(((c)>>24)&0xff)
#define RPART(c)			(((c)>>16)&0xff)
#define GPART(c)			(((c)>>8)&0xff)
#define BPART(c)			((c)&0xff)










// leilei's palette remappy stuffs

int	rmap_ready;
int rmap_models;
int rmap_maps;
int rmap_pics;
int rmap_particles;
extern byte coltranslate[256];				// TranslateToCustomPal 






