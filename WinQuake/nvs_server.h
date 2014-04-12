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
// nvs_server.h
// 2000-04-30 NVS COMMON by Maddes

extern cvar_t	*nvs_required;
extern cvar_t	*nvs_svc_enable;

extern ddef_t	*pr_field_nvs_svc;

qboolean NVS_RejectClient (void);

void NVS_Init_Server_Cvars (void);

void NVS_Init_Server (void);

// 2000-05-02 NVS SVC by Maddes  start
extern msg_lookup_t te_lookup[];
extern msg_lookup_t limit_lookup[];	// 2001-09-20 Configurable limits by Maddes
extern msg_lookup_t version_lookup[];
extern msg_lookup_t svc_lookup[];

void NVS_InitSVCMsg(int dest, int svc_message, int sub_message, client_t *client);
void PF_NVS_InitSVCMsg(void);

void NVS_WriteChar (int dest, int c, sizebuf_t *sb);
void NVS_WriteByte (int dest, int c, sizebuf_t *sb);
void NVS_WriteShort (int dest, int c, sizebuf_t *sb);
void NVS_WriteLong (int dest, int c, sizebuf_t *sb);
void NVS_WriteFloat (int dest, float f, sizebuf_t *sb);
void NVS_WriteString (int dest, char *s, sizebuf_t *sb);
void NVS_WriteCoord (int dest, float f, sizebuf_t *sb);
void NVS_WriteAngle (int dest, float f, sizebuf_t *sb);

#define	MSG_BROADCAST	0		// unreliable to all
#define	MSG_ONE			1		// reliable to one (msg_entity)
#define	MSG_ALL			2		// reliable to all
#define	MSG_INIT		3		// write to the init string
// 2000-05-02 NVS SVC by Maddes  end
