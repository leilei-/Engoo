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
// nvs_server_data.c
// 2000-05-02 NVS SVC by Maddes

#include "quakedef.h"

/*
The conversion tables consist of boolean values, that define which data is send
to a client of the corresponding NVS version.

The version tables hold a entry for every change in the SVC message.
An entry contains the NVS version the change occured, plus its number of
data/writes, message size in bytes, pointer to the conversion table.

Determining the message size:
byte/char/angle		= 1 byte
short/coord/entity	= 2 bytes
long			= 4 bytes
float			= 4 bytes
string			= unknown (assume typical length, e.g. names are 16 bytes + 1 EOS)

Now add all send writes for a version together and use the result as the message size.
Remember that you only add the writes which are true in corresponding version table line.
*/

/*
*******************************************************************
* S V C   M E S S A G E S   W I T H O U T   S U B M E S S A G E S *
*******************************************************************
*/

/*
SVC_PARTICLE
[byte] svc_particle [coord] origin [coord] origin [coord] origin [char] velocity [char] velocity [char] velocity [byte] count [byte] color
1 +2+2+2 +1+1+1 +1 +1
*/

qboolean svc_particle_000_conversion[] = { true,  true,  true,  true, true,  true,  true,  true, true};

msg_version_t svc_particle_versions[] =
{
	{0.00, 9, 12, svc_particle_000_conversion}		// 0.0 is end mark of table
};

/*
SVC_SETANGLE
[byte] svc_setangle [byte] angle [byte] angle [byte] angle [float] newangle [float] newangle [float] newangle
1 +1+1+1 +4+4+4
*/
qboolean svc_setangle_050_conversion[] = { true, false, false, false,  true,  true,  true };
qboolean svc_setangle_000_conversion[] = { true,  true,  true,  true, false, false, false };

msg_version_t svc_setangle_versions[] =
{
	{0.50, 7, 1+4+4+4, svc_setangle_050_conversion},
	{0.00, 4, 1+1+1+1, svc_setangle_000_conversion}		// 0.0 is end mark of table
};

/*
SVC LookUp Table
*/
msg_lookup_t svc_lookup[] =
{
	{svc_particle, svc_particle_versions},
	{svc_setangle, svc_setangle_versions},
	{-1, NULL}						// -1 is end mark of table
};



/*
*************************************************************
* S V C   M E S S A G E S   W I T H   S U B M E S S A G E S *
*************************************************************

Note: each has it own lookup table

*/

/*
*************************************************
* S V C _ T E M P E N T I T Y   M E S S A G E S *
*************************************************
*/

/*
TE_EXPLOSION
[byte] svc_temp_entity [byte] te_explosion [coord] x [coord] y [coord] z [coord] red [coord] green [coord] blue [coord] alpha
1+1 +2+2+2 +2+2+2+2
*/
qboolean te_explosion_050_conversion[] = {true, true, true, true, true,  true,  true,  true,  true};
qboolean te_explosion_000_conversion[] = {true, true, true, true, true, false, false, false, false};

msg_version_t te_explosion_versions[] =
{
	{0.50, 9, 1+1+2+2+2+2+2+2+2, te_explosion_050_conversion},
	{0.00, 5, 1+1+2+2+2, te_explosion_000_conversion}		// 0.0 is end mark of table
};

/*
SVC_TEMP_ENTITY LookUp Table
*/
msg_lookup_t te_lookup[] =
{
	{TE_EXPLOSION, te_explosion_versions},
	{-1, NULL}						// -1 is end mark of table
};

// 2001-09-20 Configurable limits by Maddes  start
/*
***************************************
* S V C _ L I M I T   M E S S A G E S *
***************************************
*/

// 2001-09-20 Configurable entity limits by Maddes  start
/*
LIM_ENTITIES
[byte] svc_limit [byte] lim_entities [short] edicts [short] static edicts [short] temp edicts
1+1 +2+2+2
*/
qboolean lim_entities_000_conversion[] = {  true,  true,  true,  true,  true};

msg_version_t lim_entities_versions[] =
{
	{0.00, 5, 1+1+2+2+2, lim_entities_000_conversion}	// 0.0 is end mark of table
													// HACK!!! Using version 0.00 for lowest version the
													// client must now when sending this command on connect
};
// 2001-09-20 Configurable entity limits by Maddes  end

/*
SVC_LIMIT LookUp Table
*/
msg_lookup_t limit_lookup[] =
{
	{LIM_ENTITIES, lim_entities_versions},		// 2001-09-20 Configurable entity limits by Maddes
	{-1, NULL}						// -1 is end mark of table
};
// 2001-09-20 Configurable limits by Maddes  end

/*
*******************************************************
* S V C _ E X T R A _ V E R S I O N   M E S S A G E S *
*******************************************************
*/

/*
VERSION_TYPE
[byte] svc_extra_version [byte] version_type [float] version
1+1 +4
*/
/*
qboolean version_any_XYY_conversion[] = { true,  true,  true};
qboolean version_any_000_conversion[] = {false, false, false};

msg_version_t version_any_versions[] =
{
	{X.YY, 3, 1+1+4, version_any_XYY_conversion},
	{0.00, 0, 0, version_any_000_conversion},
};
*/

/*
SVC_EXTRA_VERSION LookUp Table
*/
msg_lookup_t version_lookup[] =
{
//	{VERSION_TYPE, version_any_versions},
	{-1, NULL}						// -1 is end mark of table
};
