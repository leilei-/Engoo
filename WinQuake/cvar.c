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
// cvar.c -- dynamic variable tracking
// 2001-09-18 New cvar system by Maddes
//            completly new file

#include "quakedef.h"

cvar_t	*cvar_list = NULL;
cvar_t	*cvar_last = NULL;

char	new_valstring[CVAR_MAX_VALSTRING];

/*
*********************************************************************
   L O C A L   F U N C T I O N S
*********************************************************************
*/

/*
============
Cvar_PutInList
============
*/
void Cvar_PutInList (cvar_t *var)
{
	if (!(cvar_list))
	{
		cvar_list = var;
	}

	if (cvar_last)
	{
		cvar_last->next = var;
		var->prev = cvar_last;
	}

	cvar_last = var;
}

/*
=========
Cvar_RemoveFromList
=========
*/
void Cvar_RemoveFromList (cvar_t *var)
{
	// take out of list
	if (var->prev)
	{
		var->prev->next = var->next;
	}
	if (var->next)
	{
		var->next->prev = var->prev;
	}

	// special case: first var of list
	if (cvar_list == var)
	{
		cvar_list = var->next;
	}

	// special case: last var of list
	if (cvar_last == var)
	{
		cvar_last = var->prev;
	}
}

/*
============
Cvar_Display

same cvar display for all cvar commands
============
*/
void Cvar_Display (cvar_t *var, qboolean long_display)
{
	Con_Printf ("%c%c%c%c%c%c%c ",
			(var->flags & CVAR_ORIGINAL) ? 'O' : (var->flags & CVAR_PROGS_CREATED) ? 'P' : (var->flags & CVAR_USER_CREATED) ? 'U' : ' ',
			(var->flags & CVAR_ARCHIVE) ? 'A' : ' ',
			(var->flags & CVAR_ROM) ? 'R' : ' ',
			(var->flags & CVAR_NOTIFY) ? 'N' : ' ',
			(var->flags & CVAR_SERVERINFO) ? 'S' : (var->flags & CVAR_USERINFO) ? 'U' : ' ',
			(var->rangecheck) ? 'C' : ' ',
			(var->description) ? 'D' : ' ');

	Con_Printf ("\"%s\" is \"%s\"", var->name, var->string);

	if ((long_display) && (var->rangecheck))
	{
		char	val[32];
		int		i;

		if (var->minvalue == (int)var->minvalue)
		{
			sprintf (val, "%i", (int)var->minvalue);
		}
		else
		{
			sprintf (val, "%1f", var->minvalue);
			for (i=strlen(val)-1 ; i>0 && val[i]=='0' && val[i-1]!='.' ; i--)
			{
				val[i] = 0;
			}
		}
		Con_Printf (" (%s-", val);

		if (var->maxvalue == (int)var->maxvalue)
		{
			sprintf (val, "%i", (int)var->maxvalue);
		}
		else
		{
			sprintf (val, "%1f", var->maxvalue);
			for (i=strlen(val)-1 ; i>0 && val[i]=='0' && val[i-1]!='.' ; i--)
			{
				val[i] = 0;
			}
		}
		Con_Printf ("%s)", val);
	}

	Con_Printf ("\n");
}

// 2000-01-09 CvarList command by Maddes  start
/*
=========
Cvar_List_f
=========
*/
void Cvar_List_f (void)
{
	cvar_t	*var;
	int		i;
	char 	*partial;
	int		len;
	int		count;

	if (Cmd_Argc() > 1)
	{
		partial = Cmd_Argv (1);
		len = strlen(partial);
	}
	else
	{
		partial = NULL;
		len = 0;
	}

	count=0;
	for (var=cvar_list, i=0 ; var ; var=var->next, i++)
	{
		if (partial && Q_strncasecmp (partial, var->name, len))
		{
			continue;
		}
		Cvar_Display (var, false);
		count++;
	}

	Con_Printf ("------------\n");
	if (partial)
	{
		Con_Printf ("%i beginning with \"%s\" out of ", count, partial);
	}
	Con_Printf ("%i variables\n", i);
}
// 2000-01-09 CvarList command by Maddes  end

/*
=========
Cvar_Set_f

used for "set" and "seta" command
=========
*/
void Cvar_Set_f (void)
{
	cvar_t	*var;
	int		flags;

	if (Cmd_Argc() < 2 || Cmd_Argc() > 3)
	{
		Con_Printf ("Syntax: %s <cvar> [value]\n", Cmd_Argv(0));
		return;
	}

	var = Cvar_FindVar (Cmd_Argv(1));

	if (Cmd_Argc() == 2)	// just display the cvar
	{
		if (!var)
		{
			Con_Printf ("Variable \"%s\" does not exist\n", Cmd_Argv(1));
			return;
		}
		Cvar_Display (var, true);
		return;
	}

	flags = CVAR_NONE;
	if (!Q_strcasecmp (Cmd_Argv(0), "seta"))
	{
		flags |= CVAR_ARCHIVE;
	}

	if (!var)	// create a user cvar
	{
		flags |= CVAR_USER_CREATED;
		var = Cvar_Get (Cmd_Argv(1), Cmd_Argv(2), flags);
		return;
	}

	if (var->flags & CVAR_ROM)	// check for user-protected cvar
	{
		Con_Printf ("Variable \"%s\" is read-only\n", var->name);
		return;
	}

	Cvar_Set (var, Cmd_Argv(2));
	var->flags |= flags;
}

/*
=========
Cvar_UnSet_f

used for "unset" command
=========
*/
void Cvar_UnSet_f (void)
{
	cvar_t	*var;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Syntax: %s <cvar>\n", Cmd_Argv(0));
		return;
	}

	var = Cvar_FindVar (Cmd_Argv(1));
	if (!var)
	{
		Con_Printf ("Variable \"%s\" does not exist\n", Cmd_Argv(1));
		return;
	}

	if (!(var->flags & CVAR_USER_CREATED))
	{
		Con_Printf ("Variable \"%s\" is not user created\n", var->name);
		return;
	}

	var = Cvar_Free (var);
}

/*
============
Cvar_Help_f
============
*/
void Cvar_Help_f (void)
{
	cvar_t	*var;
	char	*dummy = NULL;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Syntax: %s <cvar>\n", Cmd_Argv(0));

		Con_Printf ("\nOutput:\n");
		Con_Printf ("1234567 \"cvarname\" is \"value\"\n");
		Con_Printf ("\\------ (O)riginal Quake, engine( ), (P)ROGS.DAT or (U)ser cvar\n");
		Con_Printf (" \\----- (A)rchived in config.cfg\n");
		Con_Printf ("  \\---- Read-only\n");
		Con_Printf ("   \\--- (N)otification send to all clients on change\n");
		Con_Printf ("    \\-- (S)erverinfo or (U)serinfo\n");
		Con_Printf ("     \\- Range (c)hecked\n");
		Con_Printf ("      \\ Description available (use CVARHELP)\n");

		return;
	}

	var = Cvar_FindVar (Cmd_Argv(1));
	if (!var)
	{
		Con_Printf ("Variable \"%s\" does not exist\n", Cmd_Argv(1));
		return;
	}

	Cvar_Display (var, false);

	if (var->description)
	{
		Con_Printf("Desc: %s\n", var->description);
	}
	else
	{
		Con_Printf("Desc: No information available.\n");
	}

	if (var->rangecheck)
	{
		var->rangecheck (var, dummy, true);
	}
}

/*
*********************************************************************
   G L O B A L   F U N C T I O N S
*********************************************************************
*/

/*
============
Cvar_Init
============
*/
void Cvar_Init (void)
{
	Cmd_AddCommand ("set", Cvar_Set_f);
	Cmd_AddCommand ("seta", Cvar_Set_f);
	Cmd_AddCommand ("unset", Cvar_UnSet_f);
	Cmd_AddCommand ("cvarlist", Cvar_List_f);	// 2000-01-09 CvarList command by Maddes
	Cmd_AddCommand ("cvarhelp", Cvar_Help_f);
}

/*
============
Cvar_Get
============
*/
cvar_t *Cvar_Get (char *name, char *string, int flags)
{
	cvar_t	*var;

	var = Cvar_FindVar (name);
	if (!var)	// Var does not exist, create it
	{
		var = Z_Malloc (mainzone, sizeof(cvar_t));	// 2001-09-20 Enhanced zone handling by Maddes
		var->name = Z_Malloc (mainzone, strlen (name) + 1);	// 2001-09-20 Enhanced zone handling by Maddes
		strcpy (var->name, name);
		var->string = NULL;		// no value yet
		var->value = 0;
		var->flags = CVAR_NONE;
		var->prev = NULL;
		var->next = NULL;

		var->callback = NULL;

		var->rangecheck = NULL;
		var->minvalue = 0;
		var->maxvalue = 0;

		var->description = NULL;

		Cvar_PutInList (var);

		Cvar_Set (var, string);
	}

	// always throw out flags
	if (!(flags & CVAR_ROM))	// keep ARCHIVE flag if not ROM
	{
		flags |= var->flags & CVAR_ARCHIVE;
	}
	var->flags = flags;
	return var;
}

/*
=========
Cvar_Free

used for "unset" command
=========
*/
cvar_t *Cvar_Free (cvar_t *var)
{
	Cvar_RemoveFromList (var);

	if (var->string)
	{
		Z_Free (mainzone, var->string);	// 2001-09-20 Enhanced zone handling by Maddes
	}

	if (var->name)
	{
		Z_Free (mainzone, var->name);	// 2001-09-20 Enhanced zone handling by Maddes
	}

	Z_Free (mainzone, var);	// 2001-09-20 Enhanced zone handling by Maddes

	return NULL;
}

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (char *name)
{
	cvar_t	*var;

	for ( var=cvar_list ; var ; var=var->next )
	{
		if (!Q_strcasecmp (name, var->name))
		{
			return var;
		}
	}

	return NULL;
}

/*
============
Cvar_Set
============
*/
void Cvar_Set (cvar_t *var, char *value)
{
	char	*newvalue;

	if (var->rangecheck)
	{
		newvalue = var->rangecheck (var, value, false);
	}
	else
	{
		newvalue = value;
	}

	if (var->string)
	{
		if (!strcmp (newvalue, var->string))
		{
			return;		// do nothing on same value
		}
		Z_Free (mainzone, var->string);	// 2001-09-20 Enhanced zone handling by Maddes
	}

	var->string = Z_Malloc (mainzone, strlen(newvalue) + 1);	// 2001-09-20 Enhanced zone handling by Maddes
	strcpy (var->string, newvalue);

	var->value = Q_atof (var->string);

	// notify clients of change
	if (var->flags & CVAR_NOTIFY)
	{
		if (sv.active)
		{
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
		}
	}

	if (var->callback)
	{
		var->callback (var);
	}
}

/*
============
Cvar_SetValue

expands value to a string and calls Cvar_Set
============
*/
void Cvar_SetValue (cvar_t *var, float value)
{
	char	valstring[32];

// 1999-09-07 weird cvar zeros fix by Maddes  start
	int	i;

	if (value == (int)value)
	{
		sprintf (valstring, "%i", (int)value);
	}
	else
	{
		sprintf (valstring, "%1f", value);	// no leading spaces
		for (i=strlen(valstring)-1 ; i>0 && valstring[i]=='0' && valstring[i-1]!='.' ; i--)	// no trailing zeroes
		{
			valstring[i] = 0;
		}
	}
// 1999-09-07 weird cvar zeros fix by Maddes  end

	Cvar_Set (var, valstring);
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (void)
{
	cvar_t	*var;

	var = Cvar_FindVar (Cmd_Argv(0));
	if (!var)
	{
		return false;
	}

	if (Cmd_Argc() == 1)	// just display the cvar
	{
		Cvar_Display (var, true);
	}
	else
	{
		if (var->flags & CVAR_ROM)	// check for user-protected cvar
		{
			Con_Printf ("Variable \"%s\" is read-only\n", var->name);
		}
		else
		{
			Cvar_Set (var, Cmd_Argv(1));
		}
	}

	return true;
}

/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (FILE *f, qboolean new_rc_file)
{
	cvar_t	*var;

	for (var=cvar_list ; var ; var=var->next)
	{
		if (!(var->flags & CVAR_ARCHIVE))	// only cvars that should be archived
		{
			continue;
		}

		if (new_rc_file)
		{
			fprintf (f, "seta %s \"%s\"\n", var->name, var->string);
		}
		else if (var->flags & CVAR_ORIGINAL)		// original id cvars
		{
			fprintf (f, "%s \"%s\"\n", var->name, var->string);
		}
	}
}

/*
============
Cvar_CompleteVariable

attempts to match a partial variable name for command line completion
returns NULL if nothing fits
============
*/
char *Cvar_CompleteVariable (char *partial)
{
	cvar_t	*var;
	int		len;

	len = strlen(partial);

	if (!len)
	{
		return NULL;
	}

	// check variables
	for (var=cvar_list ; var ; var=var->next)
	{
		if (!(Q_strncasecmp (partial,var->name, len)))
		{
			return var->name;
		}
	}

	return NULL;
}

/*
============
Cvar_SetRangecheck
============
*/
void Cvar_SetRangecheck (cvar_t *var, cvar_rangecheck rangecheck, float minvalue, float maxvalue)
{
	var->rangecheck = rangecheck;
	var->minvalue = minvalue;
	var->maxvalue = maxvalue;
}

/*
============
Cvar_SetCallback
============
*/
void Cvar_SetCallback (cvar_t *var, cvar_callback callback)
{
	var->callback = callback;
}

/*
============
Cvar_RangecheckBool
============
*/
char *Cvar_RangecheckBool (cvar_t *var, char *value, qboolean showinfo)
{
	float	newvalue;

	if (showinfo)
	{
		Con_Printf("Range: BOOL 0/1\n");
		return value;
	}

	newvalue = Q_atof (value);

	if (newvalue)
	{
		return "1";
	}

	return "0";
}

/*
============
Cvar_RangecheckInt
============
*/
char *Cvar_RangecheckInt (cvar_t *var, char *value, qboolean showinfo)
{
	float	newvalue;

	if (showinfo)
	{
		Con_Printf("Range: INT %i - %i\n", (int)var->minvalue, (int)var->maxvalue);
		return value;
	}

	newvalue = Q_atof (value);

	if ( (newvalue != (int)newvalue)
	     || (newvalue < var->minvalue)
	     || (newvalue > var->maxvalue) )
	{
		if (newvalue < var->minvalue)
		{
			newvalue = var->minvalue;
		}
		else if (newvalue > var->maxvalue)
		{
			newvalue = var->maxvalue;
		}

		sprintf (new_valstring, "%i", (int)newvalue);

		return new_valstring;
	}

	return value;
}

/*
============
Cvar_RangecheckFloat
============
*/
char *Cvar_RangecheckFloat (cvar_t *var, char *value, qboolean showinfo)
{
	float	newvalue;
	int		i;

	if (showinfo)
	{
		Con_Printf("Range: FLOAT %1f - %1f\n", var->minvalue, var->maxvalue);
		return value;
	}

	newvalue = Q_atof (value);

	// check limits of new value
	if ( (newvalue < var->minvalue)
	     || (newvalue > var->maxvalue) )
	{
		if (newvalue < var->minvalue)
		{
			newvalue = var->minvalue;
		}
		else
		{
			newvalue = var->maxvalue;
		}

		if (newvalue == (int)newvalue)
		{
			sprintf (new_valstring, "%i", (int)newvalue);
		}
		else
		{
			sprintf (new_valstring, "%1f", newvalue);
			for (i=strlen(new_valstring)-1 ; i>0 && new_valstring[i]=='0' && new_valstring[i-1]!='.' ; i--)
			{
				new_valstring[i] = 0;
			}
		}

		return new_valstring;
	}

	return value;
}

/*
============
Cvar_SetDescription
============
*/
void Cvar_SetDescription (cvar_t *var, char *description)
{
	var->description = description;
}

// 2001-12-15 Enhanced console command completion by Fett/Maddes  start
/*
============
Cvar_CompleteCountPossible
============
*/
int Cvar_CompleteCountPossible (char *partial)
{
	cvar_t	*var;
	int	len;
	int	h;

	h=0;

	len = strlen(partial);

	if (!len)
		return 0;

	// Loop through the cvars and count all partial matches
	for (var=cvar_list ; var ; var=var->next)
		if (!Q_strncasecmp (partial,var->name, len))
			h++;

	return h;
}

/*
============
Cvar_CompletePrintPossible
============
*/
void Cvar_CompletePrintPossible (char *partial)
{
	cvar_t	*var;
	int		len;
	int		maxcnt, cnt;
	int		con_linewidth;
	char	*sout;

	len = strlen(partial);

	// Determine the width of the console - 1
	con_linewidth = (vid.width >> 3) - 3;
	maxcnt = con_linewidth / 20;	// entries per line
	cnt = 0;

	// Loop through the cvar list and print all matches
	for (var=cvar_list ; var ; var=var->next)
	{
		if (!Q_strncasecmp (partial,var->name, len))
		{
			sout = Pad_CompletePrint(var->name);
			Con_Printf ("%s", sout);

			cnt++;
			if (cnt >= maxcnt)
			{
				cnt = 0;
				Con_Printf ("\n");
			}
		}
	}
	if (cnt)
	{
		Con_Printf ("\n");
	}
	Con_Printf ("\n");
}
// 2001-12-15 Enhanced console command completion by Fett/Maddes  end
