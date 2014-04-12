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
// nvs_client.c
// 2000-04-30 NVS COMMON by Maddes

#include "quakedef.h"

// 2000-04-30 NVS COMMON by Maddes
cvar_t	*nvs_current_csvc;
cvar_t	*nvs_current_cclc;

/*
=====================
Host_NVS_Max_Client_f

Command: NVS_MAX_CLIENT <version>
=====================
*/
// 2000-04-30 NVS HANDSHAKE SRV<->CL/QC<->CL by Maddes
void Host_NVS_Max_Client_f (void)
{
	float	value;

	if (Cmd_Argc() < 2)			// not enough arguments
	{
		Con_Printf("%s: Maximum possible NVS client version is %1.2f", Cmd_Argv(0), nvs_current_cclc->maxvalue);
		if (nvs_current_cclc->maxvalue != MAX_NVS_VERSION)
		{
			Con_Printf(", highest maximum is %1.2f", MAX_NVS_VERSION);
		}
		Con_Printf("\nSyntax: %s <version>\n", Cmd_Argv(0));
		return;
	}

	value = Q_atof(Cmd_Argv(1));

	if (value > MAX_NVS_VERSION)
	{
		Con_Printf("%s: NVS client version %1.2f is not supported, highest version is %1.2f\n", Cmd_Argv(0), value, MAX_NVS_VERSION);
		return;
	}

	if (value < 0)
	{
		value = MAX_NVS_VERSION;
	}

	nvs_current_cclc->maxvalue = value;
	Con_Printf("%s: Maximum possible NVS client version set to %1.2f, change will take effect on next connect\n", Cmd_Argv(0), nvs_current_cclc->maxvalue);
}

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
=====================
NVS_Init_Client_Cvars

Register NVS cvars
=====================
*/
void NVS_Init_Client_Cvars (void)
{
	nvs_current_csvc = Cvar_Get ("nvs_current_csvc", "0", CVAR_ROM);
	Cvar_SetRangecheck (nvs_current_csvc, NULL, 0, MAX_NVS_VERSION);
	Cvar_SetDescription (nvs_current_csvc, "Contains the current client's NVS version for SVC messages, which are send from the server to this client. Maximum can be set with NVS_MAX_CLIENT command.");
	Cvar_Set(nvs_current_csvc, nvs_current_csvc->string);	// do rangecheck

	nvs_current_cclc = Cvar_Get ("nvs_current_cclc", "0", CVAR_ROM);
	Cvar_SetRangecheck (nvs_current_cclc, NULL, 0, MAX_NVS_VERSION);
	Cvar_SetDescription (nvs_current_cclc, "Contains the current client's NVS version for CLC messages, which are send from this client to the server.");
	Cvar_Set(nvs_current_cclc, nvs_current_cclc->string);	// do rangecheck
}
// 2001-09-18 New cvar system by Maddes (Init)  end

/*
=====================
NVS_Init_Client

Register NVS commands and initialize variables
=====================
*/
// 2000-04-30 NVS COMMON by Maddes
void NVS_Init_Client (void)
{
// 2001-09-18 New cvar system by Maddes (Init)  start
/*
	nvs_current_csvc = Cvar_Get ("nvs_current_csvc", "0", CVAR_ROM);
	Cvar_SetRangecheck (nvs_current_csvc, NULL, 0, MAX_NVS_VERSION);
	Cvar_Set(nvs_current_csvc, nvs_current_csvc->string);	// do rangecheck

	nvs_current_cclc = Cvar_Get ("nvs_current_cclc", "0", CVAR_ROM);
	Cvar_SetRangecheck (nvs_current_cclc, NULL, 0, MAX_NVS_VERSION);
	Cvar_Set(nvs_current_cclc, nvs_current_cclc->string);	// do rangecheck
*/
// 2001-09-18 New cvar system by Maddes (Init)  end

	Cmd_AddCommand ("nvs_max_client", Host_NVS_Max_Client_f);
}
