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
#ifdef GLOBOT
#include "bot.h"

globot_t	globot;	// This struct is used to store global stuff that aint client specific

/*
======
Random

This function only returns a random number in the range 0 - 1
======
*/
float Random (void)
{
	return (rand ()&0x7fff) / ((float)0x7fff);
}

/*
===========
RandomRange

This function is pretty much the same as Random except
that it returns a value in the range min - max
===========
*/
float RandomRange (float min, float max) 
{
	return (Random() * (max - min)) + min;
}

/*
=======
Nextent

This function is used to cycle thro all avaliable clients
(this is just a recoded PF_nextent)
=======
*/
edict_t *Nextent (edict_t *edict)
{
	int		e;
	edict_t	*ent;
	
	e = NUM_FOR_EDICT (edict);	// Get the edictnum

	while (1)					// Loop until we get a return
	{
		e++;					// Increase e with 1

		if (e == sv.num_edicts)	// If gone through all edict's
			return sv.edicts;	// then return

		ent = EDICT_NUM (e);	// Get the edict from the new edictnum

		if (!ent->free)			// If it exists
			return ent;			// then return it
	}
}

/*
=========
Traceline

This function is used to decide if a bot can see his enemy or not
(this is just a recoded PF_traceline)
=========
*/
qboolean Traceline (vec3_t v1, vec3_t v2, edict_t *ent, edict_t *enemy)
{
	trace_t	trace;

	trace = SV_Move (v1, vec3_origin, vec3_origin, v2, true, ent);

	if (trace.fraction == 1)	// If the trace found its way to the enemy
		return true;			// Then the bot can see him

	return false;				// Otherwise he cant
}

/*
==========
CalcAngles

This function is used to decide if the bot should turn around or not
(this is just a recoded PF_vectoangles)
==========
*/
void CalcAngles (vec3_t oldvector, vec3_t newvector)
{
	float	forward;
	float	yaw, pitch;
	
	if (oldvector[1] == 0 && oldvector[0] == 0)
	{
		yaw = 0;
		if (oldvector[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(oldvector[1], oldvector[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (oldvector[0]*oldvector[0] + oldvector[1]*oldvector[1]);
		pitch = (int) (atan2(oldvector[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	newvector[0] = pitch;
	newvector[1] = yaw;
	newvector[2] = 0;
}

/*
=======
BotInit

This function is used to calcualte the maximum
amount of clients and to set some default values
=======
*/
void BotInit (void)
{
	client_t	*client;
	int			i;

	globot.world		= PROG_TO_EDICT(pr_global_struct->world);	// Find the world entity
	globot.MaxClients	= 0;										// Reset the MaxClients counter

	for (i=0, client=svs.clients; i<svs.maxclients; i++, client++)	// Keep looping as long as there are clients
	{
		globot.MaxClients++;					// Increase MaxClients with 1

		client->edict->bot.ClientNo	= -1;		// We dont want anyone to have a client number until they have connected
		client->edict->bot.menudone	= false;	// Of course we have not gone past the Team Fortress menu yet
		client->edict->bot.Active	= false;	// Of course noone is active... the game havent started yet :)
		client->edict->bot.isbot	= false;	// And noone is a bot either... or human...
	}

	for (i=1; i<16; i++)
		globot.botactive[i] = false;

	if (globot.MaxClients > 16)					// We dont allow more then 16 players right now
		globot.MaxClients = 16;
}

/*
===========
PickBotName

This function is used to give each bot a different name
TODO: Read and parse names from a file
===========
*/
char *PickBotName (int r)
{
	if		(r == 1)	return "windshield wiper";
	else if	(r == 2)	return "nosefart";
	else if (r == 3)	return "WILDEBEEST";
	else if (r == 4)	return "Human";
	else if (r == 5)	return "Aye";
	else if (r == 6)	return "Utrvis";
	else if (r == 7)	return "Whoop";
	else if (r == 8)	return "BurninNgtWear";
	else if (r == 9)	return "nonsensicalous";
	else if (r == 10)	return "a dog";
	else if (r == 11)	return "Damn, I";
	else if (r == 12)	return "Hard Man";
	else if (r == 13)	return "complete as whole";
	else if (r == 14)	return "AN UPSET";
	else if (r == 15)	return "Starter";

	return "GloBot";
}

/*
============
UpdateClient

This function is used to "fake" a real
client so the bot shows up on the scoreboard
============
*/
void UpdateClient (client_t *client, int ClientNo)
{
	MSG_WriteByte	(&sv.reliable_datagram, svc_updatename);
	MSG_WriteByte	(&sv.reliable_datagram, ClientNo);
	MSG_WriteString	(&sv.reliable_datagram, client->name);
	MSG_WriteByte	(&sv.reliable_datagram, svc_updatefrags);
	MSG_WriteByte	(&sv.reliable_datagram, ClientNo);
	MSG_WriteShort	(&sv.reliable_datagram, client->old_frags);
	MSG_WriteByte	(&sv.reliable_datagram, svc_updatecolors);
	MSG_WriteByte	(&sv.reliable_datagram, ClientNo);
	MSG_WriteByte	(&sv.reliable_datagram, client->colors);
}

/*
==========
BotConnect

This function is used to connect the bot's
==========
*/
void BotConnect (client_t *client, int ClientNo, int color, char *name)
{
	edict_t	*self	= PROG_TO_EDICT(pr_global_struct->self);	// Make a backup of the current QC self
	edict_t	*bot	= client->edict;
	int		randombot;

	bot->bot.isbot			= true;								// And yes this is a bot
	bot->bot.Active			= true;								// and hes active
	bot->bot.enemy			= bot;								// Now why is he chasing himself?
	bot->bot.connecttime	= sv.time;
	bot->bot.ClientNo		= ClientNo;							// Now we get a clientnumber

	randombot = ceil (RandomRange (0, 15));

	while (globot.botactive[randombot])
		randombot = ceil (RandomRange (0, 15));

	globot.botactive[randombot] = true;

	if (name[0] != '0')
		strcpy (client->name, name);
	else
		strcpy (client->name, PickBotName (randombot));

	if (color != 666)
	{
		client->colors		= color * 14 + color;				// The bot must have a color
		bot->v.team			= color + 1;						// And to be in a team
	}
	else
	{
		client->colors		= randombot * 14 + randombot;		// The bot must have a color
		bot->v.team			= randombot + 1;					// And to be in a team
	}
	
	client->old_frags		= 0;								// And since he just joined he cant have got any frags yet

	bot->v.colormap			= ClientNo;							// Without this he wont be using any colored clothes
	bot->v.netname			= client->name - pr_strings;		// Everyone wants a name

	UpdateClient (client, ClientNo);							// Update the scoreboard

	pr_global_struct->self	= EDICT_TO_PROG(bot);				// Update the QC self to be the bot

	PR_ExecuteProgram (pr_global_struct->SetNewParms);			// Now call some QC functions
	PR_ExecuteProgram (pr_global_struct->ClientConnect);		// Now call some more QC functions
	PR_ExecuteProgram (pr_global_struct->PutClientInServer);	// Now call yet some more QC functions

	pr_global_struct->self	= EDICT_TO_PROG (self);				// Get back to the backup
}


/*
==========
LoserConnect

Create a player to split onto....
==========
*/
int	splitmeup;
client_t		*splitplayer;			// leilei - splitscreen
void LoserConnect (client_t *client, int ClientNo, int color, char *name)
{
	edict_t	*self	= PROG_TO_EDICT(pr_global_struct->self);	// Make a backup of the current QC self
	edict_t	*bot	= client->edict;
	int		randombot;
	splitmeup = 2;
	bot->bot.isbot			= 35;								// And yes this is a human who ignores some builtins
	bot->bot.Active			= true;								// and hes active
	bot->bot.enemy			= bot;								// Now why is he chasing himself?
	bot->bot.connecttime	= sv.time;
	bot->bot.ClientNo		= ClientNo;							// Now we get a clientnumber
	
	randombot = ceil (RandomRange (0, 15));

	while (globot.botactive[randombot])
		randombot = ceil (RandomRange (0, 15));

	globot.botactive[randombot] = true;

	if (name[0] != '0')
		strcpy (client->name, name);
	else
		strcpy (client->name, "Scud");

	if (color != 666)
	{
		client->colors		= color * 16 + color;				// The bot must have a color
		bot->v.team			= color + 1;						// And to be in a team
	}
	else
	{
		client->colors		= randombot * 16 + randombot;		// The bot must have a color
		bot->v.team			= randombot + 1;					// And to be in a team
	}
	client->old_frags		= 0;								// And since he just joined he cant have got any frags yet

	bot->v.colormap			= ClientNo;							// Without this he wont be using any colored clothes
	bot->v.netname			= client->name - pr_strings;		// Everyone wants a name

	UpdateClient (client, ClientNo);							// Update the scoreboard

	pr_global_struct->self	= EDICT_TO_PROG(bot);				// Update the QC self to be the bot

	PR_ExecuteProgram (pr_global_struct->SetNewParms);			// Now call some QC functions
	PR_ExecuteProgram (pr_global_struct->ClientConnect);		// Now call some more QC functions
	PR_ExecuteProgram (pr_global_struct->PutClientInServer);	// Now call yet some more QC functions

	pr_global_struct->self	= EDICT_TO_PROG (self);				// Get back to the backup
	splitplayer = client;
}
/*
==============
NextFreeClient

This function is used to find an empty client spot
==============
*/
void NextFreeClient (void)
{
	client_t	*client;
	int			i, color;
	char		name[32];

	if (Cmd_Argc() == 2)
	{
		color = Q_atoi(Cmd_Argv(1));
		sprintf (name, "0");
	}
	else if (Cmd_Argc() == 3)
	{
		color = Q_atoi(Cmd_Argv(1));
		sprintf (name, "%s", Cmd_Argv(2));
	}
	else
	{
		color = 666;
		sprintf (name, "0");
	}

	for (i=0, client=svs.clients; i<svs.maxclients; i++, client++)	// Keep looping as long as there are free client slots
	{
		if (!client->edict->bot.Active)				// We found a free client slot
		{
			BotConnect (client, i, color, name);	// so why not connect a bot?
			return;									// We are done now
		}
	}

	SV_BroadcastPrintf ("Unable to connect a bot, server is full.\n");	// No free client slots = no more bots allowed
}



// for splitscreen's genesis
void NextFreeLoser (void)
{
	client_t	*client;
	int			i, color;
	char		name[32];
	if (splitmeup){
		SV_BroadcastPrintf ("Unable to connect a loser, you already got one.\n");	// No free client slots = no more bots allowed
	return;
	}
	if (Cmd_Argc() == 2)
	{
		color = Q_atoi(Cmd_Argv(1));
		sprintf (name, "0");
	}
	else if (Cmd_Argc() == 3)
	{
		color = Q_atoi(Cmd_Argv(1));
		sprintf (name, "%s", Cmd_Argv(2));
	}
	else
	{
		color = 666;
		sprintf (name, "0");
	}

	for (i=0, client=svs.clients; i<svs.maxclients; i++, client++)	// Keep looping as long as there are free client slots
	{
		if (!client->edict->bot.Active)				// We found a free client slot
		{
			LoserConnect (client, i, color, name);	// so why not connect a bot?
			return;									// We are done now
		}
	}

	SV_BroadcastPrintf ("Unable to connect a loser, server is full.\n");	// No free client slots = no more bots allowed
}

/*
=======
MoveBot

This function is used to get the bot to actually
move and shoot and kill and destroy and wreak havoc and
slaughter and... oh sorry
=======
*/
void MoveBot (client_t *client, qboolean fire, edict_t *enemy)
{
	if (fire)								// If he has an enemy
	{
		vec3_t	origin;

		client->edict->v.button0	= 1;	// Shoot, slaughter, kill, destroy!!!!

		VectorSubtract (enemy->v.origin, client->edict->v.origin, origin);

		if (Length(origin) > 200	||					// If further away then 200 units from enemy
			(int)client->edict->v.weapon & 4096)		// or is using the axe
			client->cmd.forwardmove	= 400;				// Then chase after the enemy
		else											// If closer then 200 units to enemy and not using the axe
			client->cmd.forwardmove	= -400;				// Then stay at a distance

		client->cmd.sidemove		+= (rand()&127)-64;	// Make him strafe

		if (client->cmd.sidemove > 400)					// If strafing to fast
			client->cmd.sidemove = 400;					// Then limit the strafe speed
		else if (client->cmd.sidemove < -400)			// If strafing to fast
			client->cmd.sidemove = -400;				// Then limit the strafe speed
	}
	else												// If he has no enemy 
	{
		client->edict->v.button0	= 0;				// Then why bother to shoot?
		client->cmd.forwardmove		= 400;				// Let him run
	}

	switch ((int)skill->value)
	{
		case 1:		// Medium
			client->edict->v.v_angle[0]		= client->edict->v.angles[0] + (rand()&15)-8;
			client->edict->v.v_angle[1]		= client->edict->v.angles[1] + (rand()&15)-8;	// Adjust him to aim where he looks and make it not 100% accurate
			client->edict->v.v_angle[2]		= client->edict->v.angles[2] + (rand()&15)-8;
			break;

		case 2:		// Hard
			client->edict->v.v_angle[0]		= client->edict->v.angles[0] + (rand()&7)-4;
			client->edict->v.v_angle[1]		= client->edict->v.angles[1] + (rand()&7)-4;	// Adjust him to aim where he looks and make it not 100% accurate
			client->edict->v.v_angle[2]		= client->edict->v.angles[2] + (rand()&7)-4;
			break;

		case 3:		// Nightmare
			client->edict->v.v_angle[0]		= client->edict->v.angles[0] + (rand()&3)-2;
			client->edict->v.v_angle[1]		= client->edict->v.angles[1] + (rand()&3)-2;	// Adjust him to aim where he looks and make it not 100% accurate
			client->edict->v.v_angle[2]		= client->edict->v.angles[2] + (rand()&3)-2;
			break;

		default:	// Easy
			client->edict->v.v_angle[0]		= client->edict->v.angles[0] + (rand()&31)-16;
			client->edict->v.v_angle[1]		= client->edict->v.angles[1] + (rand()&31)-16;	// Adjust him to aim where he looks and make it not 100% accurate
			client->edict->v.v_angle[2]		= client->edict->v.angles[2] + (rand()&31)-16;
			break;
	}
}

/*
==============
SearchForEnemy

This function is used to search for
something for the bot to shoot at
==============
*/
void SearchForEnemy (client_t *client)
{
	edict_t	*bot = client->edict;
	edict_t	*nmy = bot->bot.enemy;					
	vec3_t	eyes1, eyes2, origin, test;
	int		test2;
	int		num;

	if (nmy != bot			&&	// If he has an enemy that aint himself
		nmy->v.health > 0)		// and has some health
	{
		VectorAdd (nmy->v.origin, nmy->v.view_ofs, eyes1);	// We want the origin of the enemies eyes
		VectorAdd (bot->v.origin, bot->v.view_ofs, eyes2);	// We want the origin of the bots eyes

		if (Traceline (eyes1, eyes2, bot, nmy))				// If the bot can see his enemy
		{
			VectorSubtract (eyes1, eyes2, origin);			// Get a nice vector
			CalcAngles (origin, test);						// And use it to see in what direction the enemy is
			test2 = test[1] - bot->v.angles[1];				// Another shortcut

			if (test2 > -80 && test2 < 80)					// If enemy is in front of the bot so he can see him
			{														
				VectorCopy (test, bot->v.angles);			// Then turn towards the enemy
				MoveBot (client, true, nmy);				// and start running and shooting
				return;										// We are done here now...
			}
		}
	}

	// Guess we had no enemy or couldnt see him anymore

	bot->bot.enemy = bot;				// Set enemy to the bot himself again
	nmy	= Nextent(globot.world);		// Prepare to loop through clients
	num	= 0;

	while (num < globot.MaxClients)		// Keep looping as long as there are clients
	{
		if (nmy->bot.Active		&&		// Enemy is playing 
			nmy != bot			&&		// and is not the bot himself
			nmy->v.health > 0	&&		// and is alive
			nmy->v.team != bot->v.team)	// and in another team
		{
			VectorAdd (nmy->v.origin, nmy->v.view_ofs, eyes1);	// We want the origin of the clients eyes
			VectorAdd (bot->v.origin, bot->v.view_ofs, eyes2);	// We want the origin of the bots eyes

			if (Traceline (eyes1, eyes2, bot, nmy))		// If the bot can see his client
			{
				VectorSubtract (eyes1, eyes2, origin);	// Get a nice vector
				CalcAngles (origin, test);				// And use it to see in what direction the client is
				test2 = test[1] - bot->v.angles[1];		// Another shortcut
				if (test2 > -60 && test2 < 60)			// If client is in front of the bot so he can see him
				{
					bot->bot.enemy = nmy;				// Then set him as the enemy
					bot->bot.chase = -1;				// and stop chasing
					VectorCopy (test, bot->v.angles);	// Then turn to the enemy
					MoveBot (client, true, nmy);		// and start running, jumping and shooting
					return;								// We are done here now...
				}
			}
		}
		num++;
		nmy = Nextent(nmy);	// If the client was'nt visible then continue the loop with the next client
	}

	// Guess we found no enemies

	bot->v.button0 = 0;		// So why waste ammo?

	num = bot->bot.chase;	// Are we already chasing someone?

	if (num == -1)			// If we arent
		num = (ceil (rand()%globot.MaxClients))	+ NUM_FOR_EDICT(globot.world);	// Then find someone to chase

	nmy = EDICT_NUM(num);			// And find that client

	if (nmy->bot.Active		&&		// Enemy is playing 
		nmy != bot			&&		// and is not the bot himself
		nmy->v.health > 0	&&		// and is alive
		nmy->v.team != bot->v.team)	// and in another team
	{
		bot->bot.chase = num;									// Then start chasing him
		VectorSubtract (nmy->v.origin, bot->v.origin, origin);	// Get a nice vector	

		CalcAngles (origin, test);		// And use it to see in what direction the client is
		bot->v.angles[0] = 0;			// This is reset so it doesnt look like hes running into the floor if the chase client is below him
		bot->v.angles[1] = test[1];		// Then turn the bot that way
		bot->v.angles[2] = 0;
		MoveBot (client, false, nmy);	// And get him running
		return;
	}

	bot->bot.chase = -1;	// Guess we found noone to chase either... so we'll stand here and see if we find anyone the next frame
}

/*
============
SwitchWeapon

This is the function where the bot checks what ammo and weapons he has...
And switches to the best weapon avaliable...
============
*/
void SwitchWeapon (edict_t *bot)
{
	int	items	= (int)bot->v.items;
	int weapon	= (int)bot->v.weapon;

	if (bot->v.ammo_rockets	>= 1		&&	// If the bot has some rockets
		items	& IT_ROCKET_LAUNCHER	&&	// and the Rocket Launcher
		weapon != IT_ROCKET_LAUNCHER)		// and aint using it
	{
		bot->v.impulse = 7;					// Then use the Rocket Launcher
		return;
	}

	if (bot->v.ammo_cells >= 1			&&	// If the bot has some cells
		bot->v.waterlevel <= 1			&&	// and is not in water
		items	& IT_LIGHTNING			&&	// and the Lightning Gun
		weapon != IT_LIGHTNING)				// and aint using it
	{
		bot->v.impulse = 8;					// Then use the Lightning Gun
		return;
	}

	if (bot->v.ammo_nails >= 2			&&	// If the bot has some nails
		items	& IT_SUPER_NAILGUN		&&	// and the Super Nailgun
		weapon != IT_SUPER_NAILGUN)			// and aint using it
	{
		bot->v.impulse = 5;					// Then use the Super Nailgun
		return;
	}

	if (bot->v.ammo_rockets >= 1		&&	// If the bot has some rockets
		items	& IT_GRENADE_LAUNCHER	&&	// and the Grenade Launcher
		weapon != IT_GRENADE_LAUNCHER)		// and aint using it
	{
		bot->v.impulse = 6;					// Then use the Grenade Launcher
		return;
	}

	if (bot->v.ammo_nails >= 1			&&	// If the bot has some nails
		items	& IT_NAILGUN			&&	// and the Nailgun
		weapon != IT_NAILGUN)				// and aint using it
	{
		bot->v.impulse = 4;					// Then use the Nailgun
		return;
	}

	if (bot->v.ammo_shells >= 2			&&	// If the bot has some sheels
		items	& IT_SUPER_SHOTGUN		&&	// and the Super Shotgun
		weapon != IT_SUPER_SHOTGUN)			// and aint using it
	{
		bot->v.impulse = 3;					// Then use the Super Shotgun
		return;
	}

	if (bot->v.ammo_shells >= 1			&&	// If the bot has some shells
		items	& IT_SHOTGUN			&&	// and the Shotgun
		weapon != IT_SHOTGUN)				// and aint using it
	{
		bot->v.impulse = 2;					// Then use the Shotgun
		return;
	}
}

/*
===========
BotPreFrame

This function is where everything starts...
From here we search for enemies to hunt and shoot
and make the bot to respawn if killed...
===========
*/
void BotPreFrame (client_t *client)
{
	edict_t	*bot = client->edict;

	client->cmd.forwardmove	= 0;	// Stop all bots from running

	SwitchWeapon (bot);

	if (bot->v.deadflag == DEAD_NO)	// If the bot is alive
		SearchForEnemy (client);	// Then search for an enemy or someone to chase
	else
	{
		bot->v.button0	= 0;		// If dead then clear all buttons
		bot->v.button1	= 0;
		bot->v.button2	= 0;
	}

	if (bot->v.deadflag == DEAD_RESPAWNABLE)	// If dead and ready to respawn
		bot->v.button1 = 1;						// Then respawn
}

/*
============
BotPostFrame

This function is used to check
if the bot is running into a wall
============
*/
void BotPostFrame (client_t *client)
{
	edict_t	*bot = client->edict;

	if (bot->bot.chase != -1)		// If we are chasing someone
	{
		if ((bot->v.velocity[0] < 20 && bot->v.velocity[0] > -20) &&	// And if our speed is slow
			(bot->v.velocity[1] < 20 && bot->v.velocity[1] > -20))		// (running against a wall)
		{
			bot->bot.chase = -1;	// Then find a new client to chase
		}
	}

	// This last piece here is only used by Team Fortress

	if (!bot->bot.menudone)								// If we have not gone past the class selection menu
	{
		if (bot->v.health > 5)							// If we have more health then 5
			bot->bot.menudone  = true;					// Then this is not Team Fortress so there is no menu

	// Looks like we are playing Team Fortress after all

		else if (sv.time > bot->bot.connecttime + 2)	// If bot has been active more then 2 seconds then choose a random class and leave the menu
			bot->v.impulse = 10;
		else if (sv.time > bot->bot.connecttime + 1 &&	// If bot has been active between 1 and 2 secs
				 teamplay->value)						// and we are playing teamplay
			bot->v.impulse = 5;							// Then choose a random team
	}
}

/*
========
Bot_Init

This function is what allows us to use
the command "addbot" from the console
========
*/
void Bot_Init (void)
{
	Cmd_AddCommand ("addbot", NextFreeClient);
	Cmd_AddCommand ("addburp", NextFreeLoser);
}
#endif