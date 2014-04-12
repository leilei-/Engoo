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
along with this program; f not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// wad.c

#include "quakedef.h"

int			wad_numlumps;
lumpinfo_t	*wad_lumps;
byte		*wad_base;
byte		*wad_extra;	// leilei - optional, but extra wad stuff!

void SwapPic (qpic_t *pic);

/*
==================
W_CleanupName

Lowercases name and pads with spaces and a terminating 0 to the length of
lumpinfo_t->name.
Used so lumpname lookups can proceed rapidly by comparing 4 chars at a time
Space padding is so names can be printed nicely in tables.
Can safely be performed in place.
==================
*/
void W_CleanupName (char *in, char *out)
{
	int		i;
	int		c;

	for (i=0 ; i<16 ; i++ )
	{
		c = in[i];
		if (!c)
			break;

		if (c >= 'A' && c <= 'Z')
			c += ('a' - 'A');
		out[i] = c;
	}

	for ( ; i< 16 ; i++ )
		out[i] = 0;
}



/*
====================
W_LoadWadFile
====================
*/
void W_LoadWadFile (char *filename)
{
	lumpinfo_t		*lump_p;
	wadinfo_t		*header;
	unsigned		i;
	int				infotableofs;
	loadedfile_t	*fileinfo;	// 2001-09-12 Returning information about loaded file by Maddes

	fileinfo = COM_LoadHunkFile (filename);
	if (!fileinfo)
		Sys_Error ("W_LoadWadFile: couldn't load %s\n\nAre you sure you are running this engine with \na valid game dataset? Check your working target\ndirectory to ensure that it is correct.", filename);
	wad_base = fileinfo->data;	// 2001-09-12 Returning information about loaded file by Maddes

	header = (wadinfo_t *)wad_base;

	if (header->identification[0] != 'W'
	|| header->identification[1] != 'A'
	|| header->identification[2] != 'D'
	|| header->identification[3] != '2')
		Sys_Error ("Wad file %s doesn't have WAD2 id\n",filename);

	wad_numlumps = LittleLong(header->numlumps);
	infotableofs = LittleLong(header->infotableofs);
	wad_lumps = (lumpinfo_t *)(wad_base + infotableofs);

	for (i=0, lump_p = wad_lumps ; i<wad_numlumps ; i++,lump_p++)
	{
		lump_p->filepos = LittleLong(lump_p->filepos);
		lump_p->size = LittleLong(lump_p->size);
			
	
		W_CleanupName (lump_p->name, lump_p->name);
		if (lump_p->type == TYP_QPIC){
			SwapPic ( (qpic_t *)(wad_base + lump_p->filepos));
	// leilei - quick palette translation
			/*
			if (rmap_ready){
				int bah;
				for (bah = 0; bah < lump_p->size; bah++)
				{ }	
				//	 lump_p->size[bah] = coltranslate[lump_p->size[bah]];
				
				}
				*/
	// leilei - quick palette translation

		}
	}
}


// leilei - this one is for 'extra' interface stuff
// it is not required by the engine, but will load it
// if it finds it
void W_LoadWadFileExtra (char *filename)
{
	lumpinfo_t		*lump_p;
	wadinfo_t		*header;
	unsigned		i;
	int				infotableofs;
	loadedfile_t	*fileinfo;	// 2001-09-12 Returning information about loaded file by Maddes

	fileinfo = COM_LoadHunkFile (filename);
	if (!fileinfo){
		Con_Printf ("W_LoadWadFileExtra: couldn't load %s\n\nNot panicking, moving on", filename);
		hasextra = 0;
			return;
	}
	wad_extra = fileinfo->data;	// 2001-09-12 Returning information about loaded file by Maddes

	header = (wadinfo_t *)wad_extra;

	if (header->identification[0] != 'W'
	|| header->identification[1] != 'A'
	|| header->identification[2] != 'D'
	|| header->identification[3] != '2')
		Sys_Error ("Wad file %s doesn't have WAD2 id\n",filename);

	wad_numlumps = LittleLong(header->numlumps);
	infotableofs = LittleLong(header->infotableofs);
	wad_lumps = (lumpinfo_t *)(wad_extra + infotableofs);

	for (i=0, lump_p = wad_lumps ; i<wad_numlumps ; i++,lump_p++)
	{
		lump_p->filepos = LittleLong(lump_p->filepos);
		lump_p->size = LittleLong(lump_p->size);
		W_CleanupName (lump_p->name, lump_p->name);
		if (lump_p->type == TYP_QPIC)
			SwapPic ( (qpic_t *)(wad_extra + lump_p->filepos));
	}
		hasextra = 1;
}



/*
=============
W_GetLumpinfo
=============
*/
lumpinfo_t	*W_GetLumpinfo (char *name)
{
	int		i;
	lumpinfo_t	*lump_p;
	char	clean[16];

	W_CleanupName (name, clean);

	for (lump_p=wad_lumps, i=0 ; i<wad_numlumps ; i++,lump_p++)
	{
		if (!strcmp(clean, lump_p->name))
			return lump_p;
	}

//	Sys_Error ("W_GetLumpinfo: %s not found", name);
	return NULL;
}

void *W_GetLumpName (char *name)
{
	lumpinfo_t	*lump;

	lump = W_GetLumpinfo (name);

	return (void *)(wad_base + lump->filepos);
}

void *W_GetLumpName2 (char *name)
{
	lumpinfo_t	*lump;

	lump = W_GetLumpinfo (name);

	return (void *)(wad_extra + lump->filepos);
}

void *W_GetLumpNum (int num)
{
	lumpinfo_t	*lump;

	if (num < 0 || num > wad_numlumps)
		Sys_Error ("W_GetLumpNum: bad number: %i", num);

	lump = wad_lumps + num;

	return (void *)(wad_base + lump->filepos);
}

/*
=============================================================================

automatic byte swapping

=============================================================================
*/

void SwapPic (qpic_t *pic)
{
	/*// leilei - quick palette translation
	if (rmap_ready){
		int bah;
		for (bah = 0; bah < pic->width*pic->height; bah++)
			pic->data[bah] = coltranslate[pic->data[bah]];
		
		}
	// leilei - quick palette translation
*/
	pic->width = LittleLong(pic->width);
	pic->height = LittleLong(pic->height);

}















int image_width, image_height;
// based on original code by LordHavoc
#define TEXWAD_MAXIMAGES 16384
typedef struct
{
	char name[16];
	FILE *file;
	int position;
	int size;
} texwadlump_t;

texwadlump_t	texwadlump[TEXWAD_MAXIMAGES];

/*
====================
W_LoadTextureWadFile
====================
*/
void W_LoadTextureWadFile (char *filename, int complain)
{
	lumpinfo_t		*lumps, *lump_p;
	wadinfo_t		header;
	unsigned		i, j;
	int				infotableofs;
	FILE			*file;
	int				numlumps;

	COM_FOpenFile (filename, &file, NULL);
	if (!file)
	{
		if (complain)
			Con_Printf ("W_LoadTextureWadFile: couldn't find %s\n", filename);
		return;
	}

	if (fread(&header, sizeof(wadinfo_t), 1, file) != 1)
	{Con_Printf ("W_LoadTextureWadFile: unable to read wad header");return;}
	
	if(header.identification[0] != 'W'
	|| header.identification[1] != 'A'
	|| header.identification[2] != 'D'
	|| header.identification[3] != '3')
	{Con_Printf ("W_LoadTextureWadFile: Wad file %s doesn't have WAD3 id\n",filename);return;}

	numlumps = LittleLong(header.numlumps);
	if (numlumps < 1 || numlumps > TEXWAD_MAXIMAGES)
	{Con_Printf ("W_LoadTextureWadFile: invalid number of lumps (%i)\n", numlumps);return;}
	infotableofs = LittleLong(header.infotableofs);
	if (fseek(file, infotableofs, SEEK_SET))
	{Con_Printf ("W_LoadTextureWadFile: unable to seek to lump table");return;}
	if (!(lumps = malloc(sizeof(lumpinfo_t)*numlumps)))
	{Con_Printf ("W_LoadTextureWadFile: unable to allocate temporary memory for lump table");return;}

	if (fread(lumps, sizeof(lumpinfo_t), numlumps, file) != (unsigned)numlumps)
	{Con_Printf ("W_LoadTextureWadFile: unable to read lump table");return;}
	
	for (i=0, lump_p = lumps ; i<(unsigned)numlumps ; i++,lump_p++)
	{
		W_CleanupName (lump_p->name, lump_p->name);
		for (j = 0;j < TEXWAD_MAXIMAGES;j++)
		{
			if (texwadlump[j].name[0]) // occupied slot, check the name
			{
				if (!strcmp(lump_p->name, texwadlump[j].name)) // name match, replace old one
					break;
			}
			else // empty slot
				break;
		}
		if (j >= TEXWAD_MAXIMAGES)
			break; // abort loading

		W_CleanupName (lump_p->name, texwadlump[j].name);
		texwadlump[j].file = file;
		texwadlump[j].position = LittleLong(lump_p->filepos);
		texwadlump[j].size = LittleLong(lump_p->disksize);
	}
	free(lumps);
	// leaves the file open
}




byte *W_ConvertWAD3Texture(miptex_t *tex)
{
	byte	*in, *data, *out, *pal;
	int		d, p;

	in		= (byte *)((int) tex + tex->offsets[0]);
	data	= out = malloc(tex->width * tex->height * 4);

	if (!data)
		return NULL;

	image_width		= tex->width;
	image_height	= tex->height;
	pal				= in + (((image_width * image_height) * 85) >> 6);
	pal				+= 2;

	for (d = 0;d < image_width * image_height;d++)
	{
		p = *in++;
		if (tex->name[0] == '{' && p == 255)
			out[0] = out[1] = out[2] = out[3] = 0;
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = 255;
		}
		out += 4;
	}
	return data;
}


byte *W_ConvertWAD3Texture2(miptex_t *tex)
{
	byte	*in, *data, *out, *pal;
	int		d, p;

	in		= (byte *)((int) tex + tex->offsets[0]);
	data	= out = malloc(tex->width * tex->height * 3);

	if (!data)
		return NULL;

	image_width		= tex->width;
	image_height	= tex->height;
	pal				= in + (((image_width * image_height) * 85) >> 6);
	pal				+= 2;

	for (d = 0;d < image_width * image_height;d++)
	{
		p = *in++;
		if (tex->name[0] == '{' && p == 255)
			out[0] = out[1] = out[2] = out[3] = 0;
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
		}
		out += 3;
	}
	return data;
}

byte *W_ConvertWAD3TextureFTE(miptex_t *tex, int *width, int *height, int alphaed)	//returns rgba
{	
	byte *in, *data, *out, *pal;
	int d, p;

	int alpha = 0;

//	if (tex->name[0] == '{')
//		alpha = 1;
//	else if (!strncmp(tex->name, "window", 6) || !strncmp(tex->name, "glass", 5))
//		alpha = 2;

//use malloc here if you want, but you'll have to free it again... NUR!

	data = out = Hunk_TempAllocMore(((tex->width*4 * tex->height) * 85)/64); //sw mip

	if (!data)
		return NULL;

	in = (byte *)tex + tex->offsets[0];

	*width = tex->width;
	*height = tex->height;
	pal = in + (((tex->width * tex->height) * 85) >> 6);
	pal += 2;

	for (d = 0;d < (tex->width * tex->height* 85)/64;d++)	//sw mip
	{
		p = *in++;
		if (alpha==1 && p == 255)	//only allow alpha on '{' textures
			out[0] = out[1] = out[2] = out[3] = 0;
		else if (alpha == 2)
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = (out[0]+out[1]+out[2])/3;
		}
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = 255;
		}
		out += 4;
	}
	return data;
}

extern byte		*host_origpal;	
// just for quake palette...
byte *W_ConvertWAD3TextureFTEQ(miptex_t *tex, int *width, int *height, int alphaed)	//returns rgba
{	
	byte *in, *data, *out, *pal;
	int d, p;

	int alpha = 0;

//	if (tex->name[0] == '{')
//		alpha = 1;
//	else if (!strncmp(tex->name, "window", 6) || !strncmp(tex->name, "glass", 5))
//		alpha = 2;

//use malloc here if you want, but you'll have to free it again... NUR!

	data = out = Hunk_TempAllocMore(((tex->width*4 * tex->height) * 85)/64); //sw mip

	if (!data)
		return NULL;

	in = (byte *)tex + tex->offsets[0];

	*width = tex->width;
	*height = tex->height;

	pal = host_origpal;

	for (d = 0;d < (tex->width * tex->height* 85)/64;d++)	//sw mip
	{
		p = *in++;
		if (alpha==1 && p == 255)	//only allow alpha on '{' textures
			out[0] = out[1] = out[2] = out[3] = 0;
		else if (alpha == 2)
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = (out[0]+out[1]+out[2])/3;
		}
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = 255;
		}
		out += 4;
	}
	return data;
}



byte *W_ConvertWAD3Texture24(miptex_t *tex, int cal)
{
	byte	*in, *data, *out, *pal;
	int		d, p;

	in		= (byte *)((int) tex + tex->offsets[0]);
	data	= out = malloc(tex->width * tex->height * 3);

	if (!data)
		return NULL;

	image_width		= tex->width;
	image_height	= tex->height;
	pal				= in + (((image_width * image_height) * 85) >> 6);
	pal				+= 2;

	for (d = 0;d < image_width * image_height;d++)
	{
		p = *in++;
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = 0;// pal[p+1];
			out[2] = 0;// pal[p+2];
		}
		out += 3;
		//out ++;
	}
	return data;
}





extern byte *host_otherpal;

byte *RemapTex(miptex_t *tex)
{
	byte	*in, *data, *out, *pal;
	int		d, p;

	in		= (byte *)((int) tex + tex->offsets[0]);
	data	= out = malloc(tex->width * tex->height * 4);

	if (!data)
		return NULL;

	image_width		= tex->width;
	image_height	= tex->height;
	//pal				= in + (((image_width * image_height) * 85) >> 6);
	pal				= host_otherpal;
//	pal				+= 2;
	
	for (d = 0;d < image_width * image_height;d++)
	{
		p = *in++;
		if (tex->name[0] == '{' && p == 255)
			out[0] = out[1] = out[2] = out[3] = 0;
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = 255;
		}
		out += 4;
	}
	return data;
}



int FindOurColor(miptex_t *tex)
{
	byte	*in, *data, *out, *pal;
	int		d, p, color;

	in		= (byte *)((int) tex + tex->offsets[0]);
	data	= out = malloc(tex->width * tex->height * 4);

	if (!data)
		return 0;

	image_width		= tex->width;
	image_height	= tex->height;
	//pal				= in + (((image_width * image_height) * 85) >> 6);
	pal				= host_otherpal;
//	pal				+= 2;


	for (d = 0;d < image_width * image_height;d++)
	{
		p = *in++;
		{
			//p *= 3;
			if (p > host_fullbrights){
			color = p;	// we got a fullbright so we can sue it.
			return color;
			}
			else
				color = p;
		//	out[0] = pal[p];
		//	out[1] = pal[p+1];
		//	out[2] = pal[p+2];
		//	out[3] = 255;
		}
	//	out += 4;
	}

	return color;
}


byte *RemapTexEGA(miptex_t *tex)
{
	byte	*in, *data, *out, *pal;
	int		d, p;

	in		= (byte *)((int) tex + tex->offsets[0]);
	data	= out = malloc(tex->width * tex->height * 4);

	if (!data)
		return NULL;

	image_width		= tex->width;
	image_height	= tex->height;
	pal				= host_otherpal;
	
	for (d = 0;d < image_width * image_height;d++)
	{
		p = *in++;
		p = coltranslate[p];
		out[0] = p;
		out++;
	}
	return data;
}


char wads[4096];
void Mod_ParseInfoFromEntityLump(char *data)	//actually, this should be in the model code.
{
	extern model_t *loadmodel;
	char key[128];
	char skyname[64];
	float skyrotate = 0;
	vec3_t skyaxis = {0, 0, 0};

	wads[0] = '\0';

#ifndef CLIENTONLY
	if (isDedicated)	//don't bother
		return;
#endif

	// this hack is necessary to ensure Quake 2 maps get their
	// default skybox
	if (loadmodel->fromgame == fg_quake2)
		strcpy(skyname, "unit1_");
	else
		skyname[0] = '\0';

	if (data)
	if ((data=COM_Parse(data)))	//read the map info.
	if (com_token[0] == '{')
	while (1)
	{
		if (!(data=COM_Parse(data)))
			break; // error
		if (com_token[0] == '}')
			break; // end of worldspawn
		if (com_token[0] == '_')
			strcpy(key, com_token + 1);	//_ vars are for comments/utility stuff that arn't visible to progs. Ignore them.
		else
			strcpy(key, com_token);
		if (!((data=COM_Parse(data))))
			break; // error		
		if (!strcmp("wad", key)) // for HalfLife maps
		{
			if (loadmodel->fromgame == fg_halflife)
			{
				strncat(wads, ";", 4095);	//cache it for later (so that we don't play with any temp memory yet)
				strncat(wads, com_token, 4095);	//cache it for later (so that we don't play with any temp memory yet)
			}
		}
		else if (!strcmp("skyname", key)) // for HalfLife maps
		{
			strncpy(skyname, com_token, sizeof(skyname));
		}
		else if (!strcmp("sky", key)) // for Quake2 maps
		{
			strncpy(skyname, com_token, sizeof(skyname));
		}
		else if (!strcmp("skyrotate", key))
		{
			skyrotate = atof(com_token);
		}
		else if (!strcmp("skyaxis", key))
		{
			char *s;
			strncpy(key, com_token, sizeof(key));
			s = COM_Parse(key);
			if (s)
			{
				skyaxis[0] = atof(s);
				s = COM_Parse(s);
				if (s)
				{
					skyaxis[1] = atof(s);
					COM_Parse(s);
					if (s)
						skyaxis[2] = atof(s);
				}
			}
		}
	}

	skyrotate = VectorNormalize(skyaxis);
//	R_SetSky(skyname, skyrotate, skyaxis);
}


