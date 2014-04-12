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

#ifndef GloBot
#define GloBot

typedef struct
{
	struct edict_s	*enemy;			// This holds what client the bot is currently in a fight with
	int				chase;			// This holds what client the bot is following when he cant see anyone. Its used to get him to run around... bad hack i know
	int				ClientNo;		// This holds the bots client number
	float			connecttime;	// The server time when he connected
	qboolean		menudone;		// Used for Team Fortress to see if we have passed the class menu or not
	qboolean		isbot;			// This keeps track if a client is a bot or a human
	qboolean		Active;			// And this if client has joined the game or not

} bot_t;

typedef struct
{
	struct edict_s	*world;			// This holds the world entity
	byte			MaxClients;		// And this is the maximum allowed number of client's
	qboolean		botactive[64];	// And this keeps track of what bots that has already joined
} globot_t;

extern	globot_t		globot;		// This struct is used to store global stuff that aint client specific

//bot_misc.c
float Random (void);
float RandomRange (float min, float max) ;

//bot_setup.c
void NextFreeClient (void);

#endif // GloBot