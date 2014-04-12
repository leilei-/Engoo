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
// nvs_server.c
// 2000-04-30 NVS COMMON by Maddes

#include "quakedef.h"

// 2000-04-30 NVS COMMON by Maddes
cvar_t	*nvs_required;
cvar_t	*nvs_svc_enable;

ddef_t	*pr_field_nvs_svc;

/*
=====================
NVS_RejectClient

checks client's CMAX version against required NVS version
=====================
*/
// 2000-04-30 NVS HANDSHAKE SRV<->CL/QC<->CL by Maddes
qboolean NVS_RejectClient (void)
{
	// check client against required version
	if (host_client->nvs_cmax < nvs_required->value)	// client not compliant
	{
		Con_DPrintf ("Client rejected! His executable/settings doesn't support the required NVS version.\n");

		// tell client and force him to disconnect
		SV_ClientPrintf ("\nRejected! Your executable/settings doesn't support the required NVS version.\n\n");
		cmd_source = src_client;
		Host_Version_f ();
		SV_DropClient (false);
		return true;
	}

	return false;
}


/*
=====================
Host_NVS_Request_f

Command: NVS_REQUEST <version>
=====================
*/
// 2000-04-30 NVS HANDSHAKE SRV<->CL/QC<->CL by Maddes
void Host_NVS_Request_f (void)
{
	float	value;

	if (cmd_source == src_command)	// this a client remote only command
	{
		Con_Printf("%s is not valid from the console\n", Cmd_Argv(0));
		return;
	}

	if (Cmd_Argc() < 2)		// not enough arguments
	{
		SV_ClientPrintf("%s: not enough arguments\n", Cmd_Argv(0));
		SV_ClientPrintf("Syntax: %s <version>\n", Cmd_Argv(0));
		return;
	}

	Con_DPrintf("Server received NVS client version %s from client %i\n", Cmd_Argv(1), NUM_FOR_EDICT(host_client->edict));

// 2001-12-24 Keeping full backwards compatibility by Maddes  start
	if (sv_compatibility->value)	// do not reply, like the original Quake executable
	{
		return;
	}
// 2001-12-24 Keeping full backwards compatibility by Maddes  end

	// get and check value
	value = Q_atof(Cmd_Argv(1));
	if (value < 0)
	{
		SV_ClientPrintf ("Only positive NVS versions are accepted.\n");
		return;
	}

	// determine and store client NVS versions
	host_client->nvs_cmax = value;
	host_client->nvs_cclc = (value < MAX_NVS_VERSION) ? value : MAX_NVS_VERSION;
	host_client->nvs_csvc = (value < nvs_current_ssvc->value) ? value : nvs_current_ssvc->value;

	// check client against required version
	if (NVS_RejectClient())
	{
		return;
	}

	// tell client the NVS versions, only when necessary or when client changes during a running game
	// NECESSARY (GOOD) HACK: This is a special case were not the client's SVC is of importance
	if (host_client->nvs_cclc || nvs_current_ssvc->value || host_client->spawned)
	{
		MSG_WriteByte (&host_client->message, svc_extra_version);
		MSG_WriteByte (&host_client->message, VERSION_NVS);
		MSG_WriteFloat (&host_client->message, nvs_current_ssvc->value);
		MSG_WriteFloat (&host_client->message, host_client->nvs_csvc);
		MSG_WriteFloat (&host_client->message, host_client->nvs_cclc);

		Con_DPrintf("Server sends NVS versions SSVC %1.2f CSVC %1.2f CCLC %1.2f to client %i\n", nvs_current_ssvc->value, host_client->nvs_csvc, host_client->nvs_cclc, NUM_FOR_EDICT(host_client->edict));
	}
}


/*
=====================
Host_NVS_Max_Server_f

Command: NVS_MAX_SERVER <version>
=====================
*/
// 2000-04-30 NVS HANDSHAKE SRV<->QC/SRV<->CL by Maddes
void Host_NVS_Max_Server_f (void)
{
	float	value;

	if (Cmd_Argc() < 2)			// not enough arguments
	{
		Con_Printf("%s: Maximum possible NVS server version is %1.2f", Cmd_Argv(0), nvs_current_ssvc->maxvalue);
		if (nvs_current_ssvc->maxvalue != MAX_NVS_VERSION)
		{
			Con_Printf(", highest maximum is %1.2f", MAX_NVS_VERSION);
		}
		Con_Printf("\nSyntax: %s <version>\n", Cmd_Argv(0));
		return;
	}

	value = Q_atof(Cmd_Argv(1));

	if (value > MAX_NVS_VERSION)
	{
		Con_Printf("%s: NVS server version %1.2f is not supported, highest version is %1.2f\n", Cmd_Argv(0), value, MAX_NVS_VERSION);
		return;
	}

	if (value < 0)
	{
		value = MAX_NVS_VERSION;
	}

	nvs_current_ssvc->maxvalue = value;
	Con_Printf("%s: Maximum possible NVS server version set to %1.2f, change will take effect on next level\n", Cmd_Argv(0), nvs_current_ssvc->maxvalue);
}

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
=====================
NVS_Init_Server_Cvars

Register NVS cvars
=====================
*/
void NVS_Init_Server_Cvars (void)
{
	nvs_required = Cvar_Get ("nvs_required", "0", CVAR_NOTIFY);
	Cvar_SetRangecheck (nvs_required, Cvar_RangecheckFloat, 0, MAX_NVS_VERSION);
	Cvar_SetDescription (nvs_required, "Contains the required NVS version a client must support to join this server. Note that the PROGS.DAT may do similar checks which are done without this cvar.");
	Cvar_Set(nvs_required, nvs_required->string);	// do rangecheck

	nvs_svc_enable = Cvar_Get ("nvs_svc_enable", "0", CVAR_NONE);
	Cvar_SetRangecheck (nvs_svc_enable, Cvar_RangecheckBool, 0, 1);
	Cvar_SetDescription (nvs_svc_enable, "As long as NVS is not a standard this cvar will default to 0, which disables nearly all enhanced SVC messages from this server to the client, only the NVS handshake SVC message and all enhanced client messages are enabled. This cvar will be removed when the NVS becomes a standard. Also see SV_COMPATIBILITY.");
	Cvar_Set(nvs_svc_enable, nvs_svc_enable->string);	// do rangecheck
}
// 2001-09-18 New cvar system by Maddes (Init)  end

/*
=====================
NVS_Init_Server

Register NVS commands and initialize variables
=====================
*/
// 2000-04-30 NVS COMMON by Maddes
void NVS_Init_Server (void)
{
// 2001-09-18 New cvar system by Maddes (Init)  start
/*
	nvs_required = Cvar_Get ("nvs_required", "0", CVAR_NOTIFY);
	Cvar_SetRangecheck (nvs_required, Cvar_RangecheckFloat, 0, MAX_NVS_VERSION);
	Cvar_Set(nvs_required, nvs_required->string);	// do rangecheck
*/
// 2001-09-18 New cvar system by Maddes (Init)  end

	Cmd_AddCommand ("nvs_max_server", Host_NVS_Max_Server_f);
	Cmd_AddCommand ("nvs_request", Host_NVS_Request_f);

	sv.nvs_msgwrites = 0;			// 2000-05-02 NVS SVC by Maddes
}


/*
=====================
NVS_InitSVCMsg

Check for modified message and determine conversion tables for clients
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_InitSVCMsg(int dest, int svc_message, int sub_message, client_t *client)
{
	int		i;
	int		search_message;
	msg_lookup_t	*lookup_tab;
	msg_version_t	*version_tab;

	// initialize global data
	sv.nvs_msgserver = NULL;
	sv.nvs_msgwrites = 0;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		svs.clients[i].nvs_msgconversion = NULL;

		if ( (!svs.clients[i].active)			// not active
		|| ((client) && (client != &svs.clients[i])) )	// not selected
		{
			svs.clients[i].nvs_msgignore = true;	// ignore
		}
		else
		{
			svs.clients[i].nvs_msgignore = false;
		}
	}

	// determine which lookup table to use for svc/sub message
	switch(svc_message)
	{
	case svc_temp_entity:
		lookup_tab = te_lookup;
		search_message = sub_message;
		break;

// 2001-09-20 Configurable limits by Maddes  start
	case svc_limit:
		lookup_tab = limit_lookup;
		search_message = sub_message;
		break;
// 2001-09-20 Configurable limits by Maddes  end

	case svc_extra_version:
		lookup_tab = version_lookup;
		search_message = sub_message;
		break;

	default:
		lookup_tab = svc_lookup;
		search_message = svc_message;
		break;
	}

	// find svc/sub message in lookup table
	for ( ; (lookup_tab->cmd >= 0) && (lookup_tab->cmd != search_message) ; lookup_tab++)
	{
	}
	if (lookup_tab->cmd < 0)			// message not found in lookup table
	{
		return;
	}

	// find server's SSVC in version table
	sv.nvs_msgserver = lookup_tab->version_tab;
	for ( ; sv.nvs_msgserver->nvs_version && sv.nvs_msgserver->nvs_version > nvs_current_ssvc->value ; sv.nvs_msgserver++)
	{
	}
	sv.nvs_msgwrites = sv.nvs_msgserver->numwrites;		// will be decreased through write command

	// find signon's SSVC in version table (last entry 0.00)
	sv.nvs_msgsignon = sv.nvs_msgserver;
	for ( ; sv.nvs_msgsignon->nvs_version ; sv.nvs_msgsignon++)
	{
	}

	// find clients' versions
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].nvs_msgignore)
		{
			continue;		// not active or selected client
		}

		// find client's CSVC in version table
		version_tab = sv.nvs_msgserver;
		for ( ; version_tab->nvs_version && version_tab->nvs_version > svs.clients[i].nvs_csvc ; version_tab++)
		{
		}

		if ( (!version_tab->numwrites)
		|| ((dest == MSG_BROADCAST) && (svs.clients[i].datagram.cursize > MAX_DATAGRAM-version_tab->numbytes)) )
		{
			svs.clients[i].nvs_msgignore = true;
		}
		else
		{
			svs.clients[i].nvs_msgconversion = version_tab->conversion_tab;
		}
	}
}


/*
=====================
PF_NVS_InitSVCMsg

Call NVS_InitSVCMsg from PROGS.DAT/QuakeC
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void PF_NVS_InitSVCMsg(void)
{
	edict_t		*ent;
	int		entnum;
	client_t	*client;

	client = NULL;

	ent = G_EDICT(OFS_PARM3);
	entnum = NUM_FOR_EDICT(ent);
	if (entnum == 0)				// world means all clients
	{
		client = NULL;
	}
	else if (entnum < 1 || entnum > svs.maxclients)
	{
		PR_RunError ("PF_NVS_InitSVCMsg: entity not a client");
	}
	else
	{
		client = &svs.clients[entnum-1];
	}

	NVS_InitSVCMsg(G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), G_FLOAT(OFS_PARM2), client);
}


/*
=====================
NVS_CheckClient

Checks if client or write command for client should be ignored
=====================
*/
// 2000-05-02 NVS SVC by Maddes
qboolean NVS_CheckClient (client_t *client)
{
	if (client->nvs_msgignore)
	{
		return false;		// client should be ignored (not active, not selected or don't know SVC message)
	}

	// if NVS enhanced message then perform necessary checks
	if (sv.nvs_msgwrites)
	{
		if (!(client->nvs_msgconversion[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites]))
		{
			return false;		// client does not await this data
		}
	}

	return true;
}


/*
=====================
NVS_CheckCounter

Handles write counter
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_CheckCounter (void)
{
	if (sv.nvs_msgwrites)
	{
		sv.nvs_msgwrites--;
	}
}


/*
=====================
NVS_WriteChar
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteChar (int dest, int c, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteChar (sb, c);
			}
			else
			{
				MSG_WriteChar (&sv.signon, c);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteChar (&svs.clients[i].datagram, c);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteChar (sb, c);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteChar (&svs.clients[i].message, c);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteChar: bad destination");
		break;
	}

	NVS_CheckCounter();
}


/*
=====================
NVS_WriteByte
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteByte (int dest, int c, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteByte (sb, c);
			}
			else
			{
				MSG_WriteByte (&sv.signon, c);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteByte (&svs.clients[i].datagram, c);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteByte (sb, c);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteByte (&svs.clients[i].message, c);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteByte: bad destination");
		break;
	}

	NVS_CheckCounter();
}


/*
=====================
NVS_WriteShort
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteShort (int dest, int c, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteShort (sb, c);
			}
			else
			{
				MSG_WriteShort (&sv.signon, c);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteShort (&svs.clients[i].datagram, c);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteShort (sb, c);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteShort (&svs.clients[i].message, c);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteShort: bad destination");
		break;
	}

	NVS_CheckCounter();
}


/*
=====================
NVS_WriteLong
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteLong (int dest, int c, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteLong (sb, c);
			}
			else
			{
				MSG_WriteLong (&sv.signon, c);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteLong (&svs.clients[i].datagram, c);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteLong (sb, c);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteLong (&svs.clients[i].message, c);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteLong: bad destination");
		break;
	}

	NVS_CheckCounter();
}


/*
=====================
NVS_WriteFloat
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteFloat (int dest, float f, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteFloat(sb, f);
			}
			else
			{
				MSG_WriteFloat (&sv.signon, f);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteFloat (&svs.clients[i].datagram, f);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteFloat (sb, f);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteFloat (&svs.clients[i].message, f);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteFloat: bad destination");
		break;
	}

	NVS_CheckCounter();
}


/*
=====================
NVS_WriteString
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteString (int dest, char *s, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteString(sb, s);
			}
			else
			{
				MSG_WriteString (&sv.signon, s);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteString (&svs.clients[i].datagram, s);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteString (sb, s);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteString (&svs.clients[i].message, s);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteString: bad destination");
		break;
	}

	NVS_CheckCounter();
}


/*
=====================
NVS_WriteCoord
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteCoord (int dest, float f, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteCoord(sb, f);
			}
			else
			{
				MSG_WriteCoord (&sv.signon, f);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteCoord (&svs.clients[i].datagram, f);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteCoord (sb, f);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteCoord (&svs.clients[i].message, f);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteCoord: bad destination");
		break;
	}

	NVS_CheckCounter();
}


/*
=====================
NVS_WriteAngle
=====================
*/
// 2000-05-02 NVS SVC by Maddes
void NVS_WriteAngle (int dest, float f, sizebuf_t *sb)
{
	int	i;

	switch (dest)
	{
	case MSG_INIT:
		if (sv.nvs_msgsignon->conversion_tab[sv.nvs_msgserver->numwrites-sv.nvs_msgwrites])
		{
			if (sb)				// special signon
			{
				MSG_WriteAngle(sb, f);
			}
			else
			{
				MSG_WriteAngle (&sv.signon, f);
			}
		}
		break;

	case MSG_ONE:
	case MSG_ALL:
	case MSG_BROADCAST:
		for (i=0 ; i<svs.maxclients ; i++)
		{
			if (NVS_CheckClient(&svs.clients[i]))
			{
				if (dest == MSG_BROADCAST)	// unreliable
				{
					MSG_WriteAngle (&svs.clients[i].datagram, f);
				}
				else if (dest == MSG_ONE && sb)	// special reliable MSG_ONE
				{
					MSG_WriteAngle (sb, f);
				}
				else				// reliable MSG_ONE, MSG_ALL
				{
					MSG_WriteAngle (&svs.clients[i].message, f);
				}
			}
		}
		break;

	default:
		Host_Error ("NVS_WriteAngle: bad destination");
		break;
	}

	NVS_CheckCounter();
}
