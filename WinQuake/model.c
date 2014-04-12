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
// model.c -- model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"
#include "r_local.h"

#ifdef QSB
#define	MAX_MOD_KNOWN	4096
#else
#define	MAX_MOD_KNOWN	256
#endif
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

// values for model_t's needload
#define NL_PRESENT		0
#define NL_NEEDS_LOADED	1
#define NL_UNREFERENCED	2

/*
===================
Mod_ClearAll
===================
*/
void Mod_ClearAll (void)
{
	int		i;
	model_t	*mod;


	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++) {
		mod->needload = NL_UNREFERENCED;
//FIX FOR CACHE_ALLOC ERRORS:
		if (mod->type == mod_sprite) mod->cache.data = NULL;
	}
}

/*
==================
Mod_FindName

==================
*/
model_t *Mod_FindName (char *name)
{
	int		i;
	model_t	*mod;
	model_t	*avail = NULL;
//#ifndef ITSFIX
	if (!name[0])
		Sys_Error ("Mod_ForName: NULL name");
//#endif

//
// search the currently loaded models
//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!strcmp (mod->name, name) )
			break;
		if (mod->needload == NL_UNREFERENCED)
			if (!avail || mod->type != mod_alias)
				avail = mod;
	}

	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
		{
			if (avail)
			{
				mod = avail;
				if (mod->type == mod_alias)
					if (Cache_Check (&mod->cache))
						Cache_Free (&mod->cache);
			}
			else
				Sys_Error ("mod_numknown == MAX_MOD_KNOWN");
		}
		else
			mod_numknown++;
		strcpy (mod->name, name);
		mod->needload = NL_NEEDS_LOADED;
	}

	return mod;
}

/*
==================
Mod_TouchModel

==================
*/
void Mod_TouchModel (char *name)
{
	model_t	*mod;

	mod = Mod_FindName (name);

	if (mod->needload == NL_PRESENT)
	{
		if (mod->type == mod_alias)
			Cache_Check (&mod->cache);
	}
}

/*
==================
Mod_LoadModel

Loads a model into the cache
==================
*/
model_t *Mod_LoadModel (model_t *mod, qboolean crash)
{
	unsigned	*buf;
	byte		stackbuf[1024];	// avoid dirtying the cache heap
	loadedfile_t	*fileinfo;	// 2001-09-12 Returning information about loaded file by Maddes

	if (mod->type == mod_alias)
	{
		if (Cache_Check (&mod->cache))
		{
			mod->needload = NL_PRESENT;
			return mod;
		}
	}
	else
	{
		if (mod->needload == NL_PRESENT)
			return mod;
	}

//
// because the world is so huge, load it one piece at a time
//

//
// load the file
//
// 2001-09-12 Returning information about loaded file by Maddes  start

	fileinfo = COM_LoadStackFile (mod->name, stackbuf, sizeof(stackbuf));
	if (!fileinfo)
// 2001-09-12 Returning information about loaded file by Maddes  end
	{
		if (crash)
			Sys_Error ("Mod_NumForName: %s not found", mod->name);
		return NULL;
	}

	buf = (unsigned *)fileinfo->data;	// 2001-09-12 Returning information about loaded file by Maddes

//
// allocate a new model
//
	COM_FileBase (mod->name, loadname);

	loadmodel = mod;

//
// fill it in
//

// call the apropriate loader
	mod->needload = NL_PRESENT;
	mod->dontshadow = 1;	
	switch (LittleLong(*(unsigned *)buf))
	{
	case IDPOLYHEADER:
		Mod_LoadAliasModel (mod, buf);
		mod->dontshadow = 0;	
		break;

	case IDSPRITEHEADER:
		Mod_LoadSpriteModel (mod, buf);
		
		break;

	default:
		Mod_LoadBrushModel (mod, buf, fileinfo);	// 2001-09-12 .ENT support by Maddes
		
		break;
	}

	return mod;
}


/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/


void R_LightsHere(vec3_t org, int radius, vec3_t color, int key)
{
	//entity_t *anty;
	dlight_t *dl;
	int lnum = key;

	
//	Con_Printf("got fed %f %f %f %i %i\n", org[0], org[1], org[2], radius, key);

//	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	if (dl = CL_AllocDlight (lnum)){
		//	dl = CL_AllocDlight (lnum);
			VectorCopy (org, dl->origin);
			VectorCopy (color, dl->color);
			VectorCopy (color, dl->flashcolor);
		//	dl->flashcolor[0] = 1;
		//	dl->flashcolor[1] = 1;
		//	dl->flashcolor[2] = 1;
			dl->flashcolor[3] = 0.1f;
		//	dl->color[0] = 1f;
		//	dl->color[1] = 1f;
		//	dl->color[2] = 1f;
			dl->unmark = 1;	// important
			dl->radius  = radius;
			
						
			
		//dl->radius = radius;
			dl->die = cl.time + 0xFFFFFF;
		//	dl->decay = 0;
			
		//	R_FlareTest(org, 12, 23, 33, 255, 0, NULL);
	}


};
// Yoinked directly from FTEQW by Spike. We need to parse this just for coronas
// only......or maybe...............point...................lightsbutthatsforanotherday

void R_LoadRTLights(void)
{
	dlight_t *dl;
	char fname[MAX_QPATH];
	loadedfile_t *file;
	char *end;
	int style;
	vec3_t org;
	float radius;
	char carbmarpnarm;
	float corona;
	
	vec3_t rgb;

	COM_StripExtension(cl.worldmodel->name, fname);
	strncat(fname, ".rtlights", MAX_QPATH-1);

	file = COM_LoadTempFile(fname);

		
	if (!file)
		return;
	while(1)
	{
		end = strchr(file->data, '\n');
		if (!end)
			end = file->data + strlen(file->data);
		if (end == file->data)
			break;
		*end = '\0';

		file->data = COM_Parse(file->data);
		org[0] = atof(com_token);
		file->data = COM_Parse(file->data);
		org[1] = atof(com_token);
		file->data = COM_Parse(file->data);
		org[2] = atof(com_token);

		file->data = COM_Parse(file->data);
		radius = atof(com_token);

		file->data = COM_Parse(file->data);
		rgb[0] = atof(com_token);
		file->data = COM_Parse(file->data);
		rgb[1] = atof(com_token);
		file->data = COM_Parse(file->data);
		rgb[2] = atof(com_token);
		//R_LightsHere(org,666, rgb);
		file->data = COM_Parse(file->data);
		style = atoi(com_token);

		file->data = COM_Parse(file->data);
		style = atof(com_token);

		file->data = COM_Parse(file->data);
		corona = atof(com_token);
	
		// We got a corona, so let's make it happen.
		if (corona){
				//Con_Printf("we got a %f %f %f %f %f %f %f %i %f %f in here\n", org[0], org[1], org[2], radius, rgb[0], rgb[1], rgb[2], style, style, corona);
			R_FlareTest(org,11,(int)rgb[0] * 255,(int)rgb[1] * 255,(int)rgb[2] * 255,0,NULL);
		
//						R_LightsHere(org, radius, rgb);
		}
	// testing the wlights using rtlight data. I want to really load the entities list.
		{
//		if (radius){
			

		}	
		file->data = end+1;
	}
}




/*
=================
Mod_LoadTextures
=================
*/
extern cvar_t *temp2;



void Mod_LoadTextures (lump_t *l)
{
	int		i, j, pixels, num, max, altmax, ao;
	miptex_t	*mt;
	texture_t	*tx, *tx2;
		int pb;
	texture_t	*anims[10];
	texture_t	*altanims[10];
	dmiptexlump_t *m;
	qboolean alphaed;

	if (!l->filelen)
	{
		loadmodel->textures = NULL;
		return;
	}
	m = (dmiptexlump_t *)(mod_base + l->fileofs);

	m->nummiptex = LittleLong (m->nummiptex);

	loadmodel->numtextures = m->nummiptex;
	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures) , loadname);

	for (i=0 ; i<m->nummiptex ; i++)
	{
		m->dataofs[i] = LittleLong(m->dataofs[i]);
		if (m->dataofs[i] == -1)
			continue;
		mt = (miptex_t *)((byte *)m + m->dataofs[i]);
		mt->width = LittleLong (mt->width);
		mt->height = LittleLong (mt->height);
		for (j=0 ; j<MIPLEVELS ; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);

		if ( (mt->width & 15) || (mt->height & 15) )
			Sys_Error ("Texture %s is not 16 aligned", mt->name);
	
	{
		if (!mt->offsets[0])	//external hl texture.
			{
			int pb = 1; // pixelbuffer stuff, but we only use 8bit software

			pixels = mt->width*pb*mt->height/64*85;

			tx = Hunk_AllocName (sizeof(texture_t) +pixels, mt->name );	//allocate enough to cover it.
			tx->pixbytes = pb;
			loadmodel->textures[i] = tx;

			memcpy (tx->name, mt->name, sizeof(tx->name));
			tx->width = mt->width;
			tx->height = mt->height;
			for (j=0 ; j<MIPLEVELS ; j++)
				tx->offsets[j] = 0;//mt->offsets[j] + sizeof(texture_t) - sizeof(miptex_t);
			// the pixels immediately follow the structures
			memset ( tx+1, 7, pixels);	//set it all to 7 for no particular reason.

			continue;
			}
		
			else
			{

		pixels = mt->width*mt->height/64*85;
		tx = Hunk_AllocName (sizeof(texture_t) +pixels, loadname );
		loadmodel->textures[i] = tx;

		memcpy (tx->name, mt->name, sizeof(tx->name));
		tx->width = mt->width;
		tx->height = mt->height;
		for (j=0 ; j<MIPLEVELS ; j++)
			tx->offsets[j] = mt->offsets[j] + sizeof(texture_t) - sizeof(miptex_t);
		// the pixels immediately follow the structures

	
		if (loadmodel->fromgame == fg_halflife)
		
				//
				// HLTEX - reading as 24bit
				//
				{	
			if (coloredlights == 2){
				int k;
				byte *in;
				byte *out;
				out = (byte *)(tx+1);
				in = W_ConvertWAD3Texture(mt);
				tx->offsets[0] = (char *)out - (char *)tx;for (j = 0; j < mt->width*mt->height; j++, in+=4)
				{if (in[3] == 0)	*out++ = 255;	else *out++ = BestColor(in[0],in[1],in[2], 0, 239);	}
			
		
				in = out-mt->width*mt->height;	//shrink mips.
				tx->offsets[1] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=2)	for (k = 0; k < tx->width; k+=2) *out++ = in[k + tx->width*j];
				tx->offsets[2] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=4)	for (k = 0; k < tx->width; k+=4) *out++ = in[k + tx->width*j];
				tx->offsets[3] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=8)	for (k = 0; k < tx->width; k+=8) *out++ = in[k + tx->width*j];				
					
				
			}

				//
				// HLTEX - reading as 24bit, converting to 8bit
				//
			else
			{
				int k;
				byte *in;
				byte *out;
				out = (byte *)(tx+1);
				in = W_ConvertWAD3Texture(mt);
				tx->offsets[0] = (char *)out - (char *)tx;for (j = 0; j < mt->width*mt->height; j++, in+=4)
			
				{if (in[3] == 0)	*out++ = 255;	else *out++ = BestColor(in[0],in[1],in[2], 0, 239);	}
				in = out-mt->width*mt->height;	//shrink mips.
				tx->offsets[1] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=2)	for (k = 0; k < tx->width; k+=2) *out++ = in[k + tx->width*j];
				tx->offsets[2] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=4)	for (k = 0; k < tx->width; k+=4) *out++ = in[k + tx->width*j];
				tx->offsets[3] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=8)	for (k = 0; k < tx->width; k+=8) *out++ = in[k + tx->width*j];
							
			}
				}
#ifndef EGAHACK
				else

					if (rmap_ready){
					// Read and remap inappropriately
						int k;
				byte *in;
				byte *out;
				out = (byte *)(tx+1);
				in = RemapTex(mt);
			
				tx->offsets[0] = (char *)out - (char *)tx;for (j = 0; j < mt->width*mt->height; j++, in+=4)
				{if (in[3] == 0)		*out++ = 255;	else{ *out++ =	BestColor(in[0],in[1],in[2], 0, 239);	}}
				in = out-mt->width*mt->height;	//shrink mips.
				//out = (byte *)(tx+1);
				tx->offsets[1] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=2)	for (k = 0; k < tx->width; k+=2) *out++ = in[k + tx->width*j];
				tx->offsets[2] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=4)	for (k = 0; k < tx->width; k+=4) *out++ = in[k + tx->width*j];
				tx->offsets[3] = (char *)out - (char *)tx;for (j = 0; j < tx->height; j+=8)	for (k = 0; k < tx->width; k+=8) *out++ = in[k + tx->width*j];

					}
#endif	
					else{

			{
					memcpy ( tx+1, mt+1, pixels);	// bring in ye olde quake textures
					
					
			}

					}
					
				}
		}
		if (!strncmp(mt->name,"sky",3))
			R_InitSky (tx);
			// Grab a color to sample for our particle system.
				{
					tx->pcolor = FindOurColor(tx);
				//y	if (tx->pcolor < host_fullbrights)
				//	Con_Printf("%s has color %i\n", tx->name, tx->pcolor);
			//		else
//						Con_Printf("%s has color %i AND IS FREAKING FULLBRIGHT\n", tx->name, tx->pcolor);
				}
	}

//
// sequence the animations
//
	for (i=0 ; i<m->nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;
		if (tx->anim_next)
			continue;	// already sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
			Sys_Error ("Bad animating texture %s", tx->name);

		for (j=i+1 ; j<m->nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp (tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
				Sys_Error ("Bad animating texture %s", tx->name);
		}

#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			
			if (!tx2)
			tx2 = anims[j+1];		// leilei - TFN bb2 workaround
		//		Sys_Error ("Missing frame %i of %s",j, tx->name); 
			tx2->anim_total = max * ANIM_CYCLE; 
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[ (j+1)%max ];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
	//		tx2 = altanims[0];
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = altanims[ (j+1)%altmax ];
			if (max)
				tx2->alternate_anims = anims[0];
		}
	}

}



// For the truecolor renderer only
void Mod_LoadTextures32 (lump_t *l)
{
	int		i, j, pixels, num, max, altmax, ao;
	miptex_t	*mt;
	texture_t	*tx, *tx2;

	texture_t	*anims[10];
	texture_t	*altanims[10];
	int pb;
	dmiptexlump_t *m;

			
	pb = 4;

	if (!l->filelen)
	{
		loadmodel->textures = NULL;
		return;
	}
	m = (dmiptexlump_t *)(mod_base + l->fileofs);

	m->nummiptex = LittleLong (m->nummiptex);

	loadmodel->numtextures = m->nummiptex;
	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures) , loadname);

	for (i=0 ; i<m->nummiptex ; i++)
	{
		m->dataofs[i] = LittleLong(m->dataofs[i]);
		if (m->dataofs[i] == -1)
			continue;
		mt = (miptex_t *)((byte *)m + m->dataofs[i]);
		mt->width = LittleLong (mt->width);
		mt->height = LittleLong (mt->height);

		for (j=0 ; j<MIPLEVELS ; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);


		if ( (mt->width & 15) || (mt->height & 15) )
			Sys_Error ("Texture %s is not 16 aligned", mt->name);
	
	
		
		if (!mt->offsets[0])	//external hl texture.
			{

			pixels = mt->width*4*mt->height/64*85;
/*			
			tx = Hunk_AllocName (sizeof(texture_t) +pixels, mt->name );	//allocate enough to cover it.
			tx->pixbytes = pb;

			loadmodel->textures[i] = tx;

			memcpy (tx->name, mt->name, sizeof(tx->name));
			tx->width = mt->width;
			tx->height = mt->height;
			for (j=0 ; j<MIPLEVELS ; j++)
				tx->offsets[j] = mt->offsets[j] + sizeof(texture_t) - sizeof(miptex_t);
			// the pixels immediately follow the structures
			memset ( tx+1, 7, pixels);	//set it all to 7 for no particular reason.
*/
			continue;
			}
		
			else if (loadmodel->fromgame == fg_halflife) // interna hl texture
			{
				//
				// HLTEX - reading as 24bit
				//
			

			// Reading as 24bit strictly for 15bit dither mode
			
			
			pixels = (mt->width*pb*mt->height)*85/64;
		
			tx = Hunk_AllocName (sizeof(texture_t) +pixels, loadname );
			tx->pixbytes = pb;
			loadmodel->textures[i] = tx;
			
			memcpy (tx->name, mt->name, sizeof(tx->name));
			tx->width = mt->width;
			tx->height = mt->height;

			for (j=0 ; j<MIPLEVELS ; j++)
				tx->offsets[j] = (mt->offsets[j]-sizeof(miptex_t))*4 + sizeof(texture_t);

			// the pixels immediately follow the structures
				
				memcpy ( tx+1, W_ConvertWAD3TextureFTE(mt,&mt->width,&mt->height, 0), pixels);

				
			}

				// NON HALFLIFE (Quake) Textures
			else{
				{
					// TODO: Upconvert Quake to 32-bit (should be easy, but i'm not debugging that right now)
			int pb = 4;
			
			pixels = (mt->width*pb*mt->height)*85/64;
		
			tx = Hunk_AllocName (sizeof(texture_t) +pixels, loadname );
			tx->pixbytes = 4;
			loadmodel->textures[i] = tx;
			
			memcpy (tx->name, mt->name, sizeof(tx->name));
			tx->width = mt->width;
			tx->height = mt->height;
			for (j=0 ; j<MIPLEVELS ; j++)
				tx->offsets[j] = (mt->offsets[j]-sizeof(miptex_t))*4 + sizeof(texture_t);
			// the pixels immediately follow the structures
				memcpy ( tx+1, W_ConvertWAD3TextureFTEQ(mt,	&mt->width,&mt->height), pixels);			
			}
	
	
					
					
				
		}
		if (!strncmp(mt->name,"sky",3))
			R_InitSky (tx);
	}

//
// sequence the animations
//
	for (i=0 ; i<m->nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;
		if (tx->anim_next)
			continue;	// already sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
			Sys_Error ("Bad animating texture %s", tx->name);

		for (j=i+1 ; j<m->nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp (tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
				Sys_Error ("Bad animating texture %s", tx->name);
		}

#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			
			if (!tx2)
			tx2 = anims[j+1];		// leilei - TFN bb2 workaround
		//		Sys_Error ("Missing frame %i of %s",j, tx->name); 
			tx2->anim_total = max * ANIM_CYCLE; 
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[ (j+1)%max ];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
	//		tx2 = altanims[0];
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = altanims[ (j+1)%altmax ];
			if (max)
				tx2->alternate_anims = anims[0];
		}
	}
}


/*
=================
Mod_LoadLighting
Mostly a lazy modified FTE snippet
=================
*/
extern	int truecolor;
void Mod_LoadLighting (lump_t *l)
{
		

	int		i, poo;
	byte	*in, *out, *data;
	byte	d;
	char	litname[1024];
	loadedfile_t	*fileinfo;	// 2001-09-12 Returning information about loaded file by Maddes

	loadmodel->lightdata = NULL;

	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
	if (coloredlights && external_lit || truecolor)
	{
		strcpy(litname, loadmodel->name);
		COM_StripExtension(loadmodel->name, litname);
		COM_DefaultExtension(litname, ".lit");
		fileinfo = COM_LoadHunkFile(litname);
		if (fileinfo)
		{
			Con_DPrintf("%s loaded from %s\n", litname, fileinfo->path->pack ? fileinfo->path->pack->filename : fileinfo->path->filename);
			data = fileinfo->data;	
			if (data[0] == 'Q' && data[1] == 'L' && data[2] == 'I' && data[3] == 'T')
			{
				i = LittleLong(((int *)data)[1]);
				if (i == 1)
				{
					loadmodel->lightdata = data + 8;	
					return;
				}
				else
					Con_Printf("Unknown .LIT file version (%d)\n", i);
			}
			else
				Con_Printf("Corrupt .LIT file (old version?), ignoring\n");
					
		}

		if (loadmodel->fromgame == fg_halflife || loadmodel->fromgame == fg_quake2 || loadmodel->fromgame == fg_quake3)	//half-life levels use 24 bit anyway.
		{
			loadmodel->lightdata = Hunk_AllocName ( l->filelen, loadname);
			memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
		}
		else if (!external_lit->string)
		{
			loadmodel->lightdata = Hunk_AllocName ( l->filelen, loadname);
			memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
	
				
			
		}
		else
		{
		//expand to 24 bit
			int i;
			qbyte *dest, *src = mod_base + l->fileofs;
			loadmodel->lightdata = Hunk_AllocName ( l->filelen*3, loadname);
			dest = loadmodel->lightdata;
			for (i = 0; i<l->filelen; i++)
			{
				dest[0] = *src;
				dest[1] = *src;
				dest[2] = *src;

				src++;
				dest+=3;
		
			}
				
	
		}
	}
	else
	{
		if (loadmodel->fromgame == fg_halflife || loadmodel->fromgame == fg_quake2 || loadmodel->fromgame == fg_quake3)
		{
			int i;
			qbyte *out;
			qbyte *in;
			out = loadmodel->lightdata = Hunk_AllocName ( l->filelen/3, loadname);
			in = mod_base + l->fileofs;	//24 bit to luminance.
			for (i = 0; i < l->filelen; i+=3)
				*out++ = ((int)in[i] + (int)in[i] + (int)in[i])/3;

		}
		else
			if (loadmodel->fromgame == fg_quakeold) // Quake v0.x lightmaps lack overbrights
		{
			int i;
			qbyte *out;
			qbyte *in;
			out = loadmodel->lightdata = Hunk_AllocName ( l->filelen, loadname);
			in = mod_base + l->fileofs;	
			for (i = 0; i < l->filelen; i+=1)
				*out++ = ((int)in[i])>>1;
			
		}
		else
		{//standard Quake
			loadmodel->lightdata = Hunk_AllocName ( l->filelen, loadname);
			memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
			
		}
	}


}



// old funciton
void Mod_FLoadLighting (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
	loadmodel->lightdata = Hunk_AllocName ( l->filelen, loadname);
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
}


/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void CalcSurfaceExtents (msurface_t *s)
{
	float	mins[2], maxs[2], val;
	int		i,j, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;

	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];

		for (j=0 ; j<2 ; j++)
		{
			val = v->position[0] * tex->vecs[j][0] +
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{
		bmins[i] = floor(mins[i]/16);
		bmaxs[i] = ceil(maxs[i]/16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;
		if ( !(tex->flags & TEX_SPECIAL) && s->extents[i] > 256)
			Sys_Error ("Bad surface extents");
	}
}


/*
=================
Mod_LoadFaces
=================
*/
void Mod_LoadFaces (lump_t *l)
{
	dface_t		*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;
	
	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);
		out->flags = 0;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);
		if (side)
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;

		out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

		CalcSurfaceExtents (out);

	// lighting info
	// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
				if (i == -1)
			out->samples = NULL;
		if (i == -1)
			out->samples = NULL;
		else if (coloredlights)
		{
			if (loadmodel->fromgame == fg_halflife)
				out->samples = loadmodel->lightdata + i;
			else
			out->samples = loadmodel->lightdata + i * 3;
		}
		else
		{
			if (loadmodel->fromgame == fg_halflife)
				out->samples = loadmodel->lightdata + i/3;
			else if (loadmodel->fromgame == fg_quakeold)
				out->samples = loadmodel->lightdata + i;
			else
				out->samples = loadmodel->lightdata + i;
		}
	// set the drawing flags flag
	//	if (!strncmp(out->texinfo->texture->name,"light",5) || !strncmp(out->texinfo->texture->name,"dem",3)|| !strncmp(out->texinfo->texture->name,"rune",4))	// light hack
	//	{
	//		out->flags |= ( SURF_FLARE);
	//		continue;
	//	}
		// for the d_mipdetail 64 setting - keep buttons and other small textures sharp while allowing the rest to blur out
				if (out->texinfo->texture->height < 33 && out->texinfo->texture->width < 33)
		{
			out->flags |= ( SURF_DONTMIP64);
			continue;
		}
		if (!strncmp(out->texinfo->texture->name,"sky",3))	// sky
		{
			out->flags |= (SURF_DRAWSKY | SURF_DRAWTILED);
			continue;
		}

		if (!strncmp(out->texinfo->texture->name,"*",1))		// turbulent
		{
			out->flags |= (SURF_DRAWTURB | SURF_DRAWTILED | SURF_DRAWTRANSLUCENT); // no lava/water check. we're trying to emulate dp/glq behavior here
			if (strncmp(out->texinfo->texture->name, "*lava", 3))
					out->flags |= SURF_MIRROR;	// mirror not the lava
			//out->flags |= (SURF_DRAWTILED | SURF_DRAWTRANSLUCENT); // no lava/water check. we're trying to emulate dp/glq behavior here
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			continue;
		}

//		if (!strncmp(out->texinfo->texture->name,"window02_1",10))	// sky
//		{
//			out->flags |= ( SURF_MIRROR| SURF_DRAWTILED);
//			continue;
//		}

	}
	
}

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

/*
=================
Mod_LoadAliasFrame
=================
*/
void * Mod_LoadAliasFrame (void * pin, int *pframeindex, int numv,
	trivertx_t *pbboxmin, trivertx_t *pbboxmax, aliashdr_t *pheader, char *name)
{
	trivertx_t		*pframe, *pinframe;
	int				i, j;
	daliasframe_t	*pdaliasframe;

	pdaliasframe = (daliasframe_t *)pin;

	strcpy (name, pdaliasframe->name);

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about
	// endianness
		pbboxmin->v[i] = pdaliasframe->bboxmin.v[i];
		pbboxmax->v[i] = pdaliasframe->bboxmax.v[i];
	}

	pinframe = (trivertx_t *)(pdaliasframe + 1);
	pframe = Hunk_AllocName (numv * sizeof(*pframe), loadname);

	*pframeindex = (byte *)pframe - (byte *)pheader;

	for (j=0 ; j<numv ; j++)
	{
		int		k;

	// these are all byte values, so no need to deal with endianness
		pframe[j].lightnormalindex = pinframe[j].lightnormalindex;

		for (k=0 ; k<3 ; k++)
		{
			pframe[j].v[k] = pinframe[j].v[k];
		}
	}

	pinframe += numv;

	return (void *)pinframe;
}


/*
=================
Mod_LoadAliasGroup
=================
*/
void * Mod_LoadAliasGroup (void * pin, int *pframeindex, int numv,
	trivertx_t *pbboxmin, trivertx_t *pbboxmax, aliashdr_t *pheader, char *name)
{
	daliasgroup_t		*pingroup;
	maliasgroup_t		*paliasgroup;
	int					i, numframes;
	daliasinterval_t	*pin_intervals;
	float				*poutintervals;
	void				*ptemp;

	pingroup = (daliasgroup_t *)pin;

	numframes = LittleLong (pingroup->numframes);

	paliasgroup = Hunk_AllocName (sizeof (maliasgroup_t) +
			(numframes - 1) * sizeof (paliasgroup->frames[0]), loadname);

	paliasgroup->numframes = numframes;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about endianness
		pbboxmin->v[i] = pingroup->bboxmin.v[i];
		pbboxmax->v[i] = pingroup->bboxmax.v[i];
	}

	*pframeindex = (byte *)paliasgroup - (byte *)pheader;

	pin_intervals = (daliasinterval_t *)(pingroup + 1);

	poutintervals = Hunk_AllocName (numframes * sizeof (float), loadname);

	paliasgroup->intervals = (byte *)poutintervals - (byte *)pheader;

	for (i=0 ; i<numframes ; i++)
	{
		*poutintervals = LittleFloat (pin_intervals->interval);
		if (*poutintervals <= 0.0){
			*poutintervals = 0.1; // leilei - force
		//	Sys_Error ("Mod_LoadAliasGroup: interval<=0, %s sucks", loadname);
		}
		poutintervals++;
		pin_intervals++;
	}

	ptemp = (void *)pin_intervals;

	for (i=0 ; i<numframes ; i++)
	{
				paliasgroup->frames[i].numframes = numframes; // Manoel Kasimier - model interpolation
		ptemp = Mod_LoadAliasFrame (ptemp,
									&paliasgroup->frames[i].frame,
									numv,
									&paliasgroup->frames[i].bboxmin,
									&paliasgroup->frames[i].bboxmax,
									pheader, name);
	}

	return ptemp;
}



/*
=============
Mod_AvgPixel
leilei
based on John Carmack quake2 code
=============
*/
void Mod_AvgPixel (model_t *mod, int skinnum, int count, byte *skin)
{
	int		r,g,b;
	int		i;
	int		vis;
	int		pix;
	int		bestcolor;
	int		fullbright;

	if (coloredlights){
//	mod->avgcol[0+skinnum] = 1;
//	mod->avgcol[1+skinnum] = 1;
//	mod->avgcol[2+skinnum] = 1;
			return;
	}
	
	vis = 0;
	r = g = b = 0;
	fullbright = 0;
	for (i=0 ; i<count ; i++)
	{
		pix = skin[i];
		
		r += host_basepal[pix*3];
		g += host_basepal[pix*3+1];
		b += host_basepal[pix*3+2];
		vis++;
	}
		
	r /= vis;
	g /= vis;
	b /= vis;

	// error diffusion
//	r += d_red;
//	g += d_green;
//	b += d_blue;
//	mod->avgcol[0+skinnum] = r / 255;
//	mod->avgcol[1+skinnum] = g / 255;
//	mod->avgcol[2+skinnum] = b / 255;
//	return pal;
}


/*
=================
Mod_LoadAliasSkin
=================
*/
void * Mod_LoadAliasSkin (void * pin, int *pskinindex, int skinsize,
	aliashdr_t *pheader)
{
	int		i;
	byte	*pskin, *pinskin;
	unsigned short	*pusskin;

	pskin = Hunk_AllocName (skinsize * r_pixbytes, loadname);
	pinskin = (byte *)pin;
	*pskinindex = (byte *)pskin - (byte *)pheader;

	Q_memcpy (pskin, pinskin, skinsize);

	pinskin += skinsize;

	return ((void *)pinskin);
}


/*
=================
Mod_LoadAliasSkinGroup
=================
*/
void * Mod_LoadAliasSkinGroup (void * pin, int *pskinindex, int skinsize,
	aliashdr_t *pheader)
{
	daliasskingroup_t		*pinskingroup;
	maliasskingroup_t		*paliasskingroup;
	int						i, numskins;
	daliasskininterval_t	*pinskinintervals;
	float					*poutskinintervals;
	void					*ptemp;

	pinskingroup = (daliasskingroup_t *)pin;

	numskins = LittleLong (pinskingroup->numskins);

	paliasskingroup = Hunk_AllocName (sizeof (maliasskingroup_t) +
			(numskins - 1) * sizeof (paliasskingroup->skindescs[0]),
			loadname);

	paliasskingroup->numskins = numskins;

	*pskinindex = (byte *)paliasskingroup - (byte *)pheader;

	pinskinintervals = (daliasskininterval_t *)(pinskingroup + 1);

	poutskinintervals = Hunk_AllocName (numskins * sizeof (float),loadname);

	paliasskingroup->intervals = (byte *)poutskinintervals - (byte *)pheader;

	for (i=0 ; i<numskins ; i++)
	{
		*poutskinintervals = LittleFloat (pinskinintervals->interval);
		if (*poutskinintervals <= 0)
			Sys_Error ("Mod_LoadAliasSkinGroup: interval<=0");

		poutskinintervals++;
		pinskinintervals++;
	}

	ptemp = (void *)pinskinintervals;

	for (i=0 ; i<numskins ; i++)
	{
		ptemp = Mod_LoadAliasSkin (ptemp,
				&paliasskingroup->skindescs[i].skin, skinsize, pheader);
	}

	return ptemp;
}


/*
=================
Mod_LoadAliasModel
=================
*/
void Mod_LoadAliasModel (model_t *mod, void *buffer)
{
	int					i;
	mdl_t				*pmodel, *pinmodel;
	stvert_t			*pstverts, *pinstverts;
	aliashdr_t			*pheader;
	mtriangle_t			*ptri;
	dtriangle_t			*pintriangles;
	int					version, numframes, numskins;
	int					size;
	daliasframetype_t	*pframetype;
	daliasskintype_t	*pskintype;
	maliasskindesc_t	*pskindesc;
	int					skinsize;
	int					start, end, total;

	start = Hunk_LowMark ();

	pinmodel = (mdl_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != ALIAS_VERSION)
		Sys_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);

//
// allocate space for a working header, plus all the data except the frames,
// skin and group info
//
	size = 	sizeof (aliashdr_t) + (LittleLong (pinmodel->numframes) - 1) *
			 sizeof (pheader->frames[0]) +
			sizeof (mdl_t) +
			LittleLong (pinmodel->numverts) * sizeof (stvert_t) +
			LittleLong (pinmodel->numtris) * sizeof (mtriangle_t);

	pheader = Hunk_AllocName (size, loadname);
	pmodel = (mdl_t *) ((byte *)&pheader[1] +
			(LittleLong (pinmodel->numframes) - 1) *
			 sizeof (pheader->frames[0]));

//	mod->cache.data = pheader;
	mod->flags = LittleLong (pinmodel->flags);

//
// endian-adjust and copy the data, starting with the alias model header
//
	pmodel->boundingradius = LittleFloat (pinmodel->boundingradius);
	pmodel->numskins = LittleLong (pinmodel->numskins);
	pmodel->skinwidth = LittleLong (pinmodel->skinwidth);
	pmodel->skinheight = LittleLong (pinmodel->skinheight);

	if (pmodel->skinheight > MAX_LBM_HEIGHT)
		Sys_Error ("model %s has a skin taller than %d", mod->name,
				   MAX_LBM_HEIGHT);

	pmodel->numverts = LittleLong (pinmodel->numverts);

	if (pmodel->numverts <= 0)
		Sys_Error ("model %s has no vertices", mod->name);

	if (pmodel->numverts > MAXALIASVERTS)
		Sys_Error ("model %s has too many vertices", mod->name);

	pmodel->numtris = LittleLong (pinmodel->numtris);

	if (pmodel->numtris <= 0)
		Sys_Error ("model %s has no triangles", mod->name);

	pmodel->numframes = LittleLong (pinmodel->numframes);
	pmodel->size = LittleFloat (pinmodel->size) * ALIAS_BASE_SIZE_RATIO;
	mod->synctype = LittleLong (pinmodel->synctype);
	mod->numframes = pmodel->numframes;

	for (i=0 ; i<3 ; i++)
	{
		pmodel->scale[i] = LittleFloat (pinmodel->scale[i]);
		pmodel->scale_origin[i] = LittleFloat (pinmodel->scale_origin[i]);
		pmodel->eyeposition[i] = LittleFloat (pinmodel->eyeposition[i]);
	}

	numskins = pmodel->numskins;
	numframes = pmodel->numframes;

	if (pmodel->skinwidth & 0x03)
		Sys_Error ("Mod_LoadAliasModel: skinwidth not multiple of 4");

	pheader->model = (byte *)pmodel - (byte *)pheader;

//
// load the skins
//
	skinsize = pmodel->skinheight * pmodel->skinwidth;

	if (numskins < 1)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of skins: %d\n", numskins);

	pskintype = (daliasskintype_t *)&pinmodel[1];

	pskindesc = Hunk_AllocName (numskins * sizeof (maliasskindesc_t),
								loadname);

	pheader->skindesc = (byte *)pskindesc - (byte *)pheader;

	for (i=0 ; i<numskins ; i++)
	{
		aliasskintype_t	skintype;

		skintype = LittleLong (pskintype->type);
		pskindesc[i].type = skintype;

		if (skintype == ALIAS_SKIN_SINGLE)
		{
			pskintype = (daliasskintype_t *)
					Mod_LoadAliasSkin (pskintype + 1,
									   &pskindesc[i].skin,
									   skinsize, pheader);
		}
		else
		{
			pskintype = (daliasskintype_t *)
					Mod_LoadAliasSkinGroup (pskintype + 1,
											&pskindesc[i].skin,
											skinsize, pheader);
		}
//			Mod_AvgPixel(mod, numskins, 56, &pskindesc[i].pcachespot);
	}

//
// set base s and t vertices
//
	pstverts = (stvert_t *)&pmodel[1];
	pinstverts = (stvert_t *)pskintype;

	pheader->stverts = (byte *)pstverts - (byte *)pheader;

	for (i=0 ; i<pmodel->numverts ; i++)
	{
		pstverts[i].onseam = LittleLong (pinstverts[i].onseam);
	// put s and t in 16.16 format
		pstverts[i].s = LittleLong (pinstverts[i].s) << 16;
		pstverts[i].t = LittleLong (pinstverts[i].t) << 16;
	}
#ifdef MHINTERPOL
	R_CheckAliasVerts (pmodel->numverts);
#endif
#ifdef INTERPOL7
	R_CheckAliasVerts (pmodel->numverts);
#endif
//
// set up the triangles
//
	ptri = (mtriangle_t *)&pstverts[pmodel->numverts];
	pintriangles = (dtriangle_t *)&pinstverts[pmodel->numverts];

	pheader->triangles = (byte *)ptri - (byte *)pheader;

	for (i=0 ; i<pmodel->numtris ; i++)
	{
		//int		j;

		ptri[i].facesfront = LittleLong (pintriangles[i].facesfront);

	//	leilei - unrolled
		{
			ptri[i].vertindex[0] =
					LittleLong (pintriangles[i].vertindex[0]);

			ptri[i].vertindex[1] =
					LittleLong (pintriangles[i].vertindex[1]);

			ptri[i].vertindex[2] =
					LittleLong (pintriangles[i].vertindex[2]);
		}
	}

//
// load the frames
//
	if (numframes < 1)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of frames: %d\n", numframes);

	pframetype = (daliasframetype_t *)&pintriangles[pmodel->numtris];

	for (i=0 ; i<numframes ; i++)
	{
		aliasframetype_t	frametype;

		frametype = LittleLong (pframetype->type);
		pheader->frames[i].type = frametype;

		if (frametype == ALIAS_SINGLE)
		{
			pframetype = (daliasframetype_t *)
					Mod_LoadAliasFrame (pframetype + 1,
										&pheader->frames[i].frame,
										pmodel->numverts,
										&pheader->frames[i].bboxmin,
										&pheader->frames[i].bboxmax,
										pheader, pheader->frames[i].name);
		}
		else
		{
			pframetype = (daliasframetype_t *)
					Mod_LoadAliasGroup (pframetype + 1,
										&pheader->frames[i].frame,
										pmodel->numverts,
										&pheader->frames[i].bboxmin,
										&pheader->frames[i].bboxmax,
										pheader, pheader->frames[i].name);
		}
	}

	mod->type = mod_alias;

// FIXME: do this right
	mod->mins[0] = mod->mins[1] = mod->mins[2] = -16;
	mod->maxs[0] = mod->maxs[1] = mod->maxs[2] = 16;


	

//
// move the complete, relocatable alias model to the cache
//
	end = Hunk_LowMark ();
	total = end - start;

	Cache_Alloc (&mod->cache, total, loadname);
	if (!mod->cache.data)
		return;
	memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);


}

//=============================================================================

/*
=================
Mod_LoadSpriteFrame
=================
*/
void * Mod_LoadSpriteFrame (void * pin, mspriteframe_t **ppframe, int version)
{
	dspriteframe_t		*pinframe;
	mspriteframe_t		*pspriteframe;
	int					i, width, height, size, origin[2];
	unsigned short		*ppixout;
	byte				*ppixin;

	pinframe = (dspriteframe_t *)pin;

	width = LittleLong (pinframe->width);
	height = LittleLong (pinframe->height);
	size = width * height;

	pspriteframe = Hunk_AllocName (sizeof (mspriteframe_t) + size*r_pixbytes,
								   loadname);

	Q_memset (pspriteframe, 0, sizeof (mspriteframe_t) + size);
	*ppframe = pspriteframe;

	pspriteframe->width = width;
	pspriteframe->height = height;
	origin[0] = LittleLong (pinframe->origin[0]);
	origin[1] = LittleLong (pinframe->origin[1]);

	pspriteframe->up = origin[1];
	pspriteframe->down = origin[1] - height;
	pspriteframe->left = origin[0];
	pspriteframe->right = width + origin[0];

	if (r_pixbytes == 1)
	{
		if (version == SPRITE32_VERSION)
		{	//downgrade quality

		//	ppixin = (unsigned char *)(pinframe + 1);
		//	ppixout = (unsigned char *)&pspriteframe->p.d.data[0];
		//	ppixin = (unsigned char *)(pinframe + 1);
		//	ppixout = (unsigned char *)&pspriteframe->pixels[0];
		//	ppixout = &pspriteframe->pixels[0];

			for (i=0 ; i<size ; i++)
			{
		//		if (ppixin[i*4+3] < 128)
		//			ppixout[i] = 255;	//transparent.
			//	else
			//		ppixout[i] = palmap[ppixin[i*4]][ppixin[i*4+1]][ppixin[i*4+2]];
			}
			size *= 4;
		}
		else
		//	Q/_memcpy (&pspriteframe->p.d.data[0], (qbyte *)(pinframe + 1), size);
		Q_memcpy (&pspriteframe->pixels[0], (byte *)(pinframe + 1), size);
		// leilei - quick palette translation
	if (rmap_ready){
		int bah;
		for (bah = 0; bah < pspriteframe->width*pspriteframe->height; bah++)
			pspriteframe->pixels[bah] = coltranslate[pspriteframe->pixels[bah]];
		
		}


	}
	else if (r_pixbytes == 2)
	{
		ppixin = (byte *)(pinframe + 1);
		ppixout = (unsigned short *)&pspriteframe->pixels[0];

		for (i=0 ; i<size ; i++)
			ppixout[i] = d_8to16table[ppixin[i]];
	}
	else
	{
		Sys_Error ("Mod_LoadSpriteFrame: driver set invalid r_pixbytes: %d\n",
				 r_pixbytes);
	}

	return (void *)((byte *)pinframe + sizeof (dspriteframe_t) + size);
}


/*
=================
Mod_LoadSpriteGroup
=================
*/
void * Mod_LoadSpriteGroup (void * pin, mspriteframe_t **ppframe, int version)
{
	dspritegroup_t		*pingroup;
	mspritegroup_t		*pspritegroup;
	int					i, numframes;
	dspriteinterval_t	*pin_intervals;
	float				*poutintervals;
	void				*ptemp;

	pingroup = (dspritegroup_t *)pin;

	numframes = LittleLong (pingroup->numframes);

	pspritegroup = Hunk_AllocName (sizeof (mspritegroup_t) +
				(numframes - 1) * sizeof (pspritegroup->frames[0]), loadname);

	pspritegroup->numframes = numframes;

	*ppframe = (mspriteframe_t *)pspritegroup;

	pin_intervals = (dspriteinterval_t *)(pingroup + 1);

	poutintervals = Hunk_AllocName (numframes * sizeof (float), loadname);

	pspritegroup->intervals = poutintervals;

	for (i=0 ; i<numframes ; i++)
	{
		*poutintervals = LittleFloat (pin_intervals->interval);
		if (*poutintervals <= 0.0)
			Sys_Error ("Mod_LoadSpriteGroup: interval<=0");

		poutintervals++;
		pin_intervals++;
	}

	ptemp = (void *)pin_intervals;

	for (i=0 ; i<numframes ; i++)
	{
		ptemp = Mod_LoadSpriteFrame (ptemp, &pspritegroup->frames[i], version);
	}

	return ptemp;
}


/*
=================
Mod_LoadSpriteModel
=================
*/
void Mod_LoadSpriteModel (model_t *mod, void *buffer)
{
	int					i;
	int					version;
	dsprite_t			*pin;
	msprite_t			*psprite;
	int					numframes;
	int					size;
	dspriteframetype_t	*pframetype;
	int					spr32;		// leilei 
	pin = (dsprite_t *)buffer;

	version = LittleLong (pin->version);

	if (version != 32)
	if (version != SPRITE_VERSION)
		Sys_Error ("%s has wrong version number"
				 "(%i should be %i)", mod->name, version, SPRITE_VERSION);

	if (version == 32)
			spr32 = 1;	// yep it's a sprite32.

	numframes = LittleLong (pin->numframes);

	size = sizeof (msprite_t) +	(numframes - 1) * sizeof (psprite->frames);

	psprite = Hunk_AllocName (size, loadname);

	mod->cache.data = psprite;

	psprite->type = LittleLong (pin->type);
	psprite->maxwidth = LittleLong (pin->width);
	psprite->maxheight = LittleLong (pin->height);
	psprite->beamlength = LittleFloat (pin->beamlength);
	mod->synctype = LittleLong (pin->synctype);
	psprite->numframes = numframes;

	mod->mins[0] = mod->mins[1] = -psprite->maxwidth/2;
	mod->maxs[0] = mod->maxs[1] = psprite->maxwidth/2;
	mod->mins[2] = -psprite->maxheight/2;
	mod->maxs[2] = psprite->maxheight/2;

//
// load the frames
//
	if (numframes < 1)
		Sys_Error ("Mod_LoadSpriteModel: Invalid # of frames: %d\n", numframes);

	mod->numframes = numframes;
	mod->flags = 0;

	pframetype = (dspriteframetype_t *)(pin + 1);

	for (i=0 ; i<numframes ; i++)
	{
		spriteframetype_t	frametype;

		frametype = LittleLong (pframetype->type);
		psprite->frames[i].type = frametype;

		if (frametype == SPR_SINGLE)
		{
			pframetype = (dspriteframetype_t *)
					Mod_LoadSpriteFrame (pframetype + 1,
										 &psprite->frames[i].frameptr, version);
		}
		else
		{
			pframetype = (dspriteframetype_t *)
					Mod_LoadSpriteGroup (pframetype + 1,
										 &psprite->frames[i].frameptr, version);
		}
	}

	mod->type = mod_sprite;
}

//=============================================================================

/*
================
Mod_Print
================
*/
void Mod_Print (void)
{
	int		i;
	model_t	*mod;

	Con_Printf ("Cached models:\n");
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		Con_Printf ("%8p : %s",mod->cache.data, mod->name);
		if (mod->needload & NL_UNREFERENCED)
			Con_Printf (" (!R)");
		if (mod->needload & NL_NEEDS_LOADED)
			Con_Printf (" (!P)");
		Con_Printf ("\n");
	}
}
