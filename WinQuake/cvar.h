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
// cvar.h
// 2001-09-18 New cvar system by Maddes
//            completly new file

/*
cvar_t variables are used to hold scalar or string variables that can be changed
or displayed at the console or prog code as well as accessed directly in C code.

to create a Cvar, define a pointer for it

    cvar_t *my_cvar;

and find a good place to initialize it with the Cvar_Get function.

    my_cvar = Cvar_Get ("my_cvar", "1", CVAR_ARCHIVE);

the defined flags are list in cvar.h and begin with CVAR_

C code usually just references a cvar in place:
if ( r_draworder->value )

Interpreted prog code can access cvars with the cvar(name) or
cvar_set (name, value) internal functions:
teamplay = cvar("teamplay");
cvar_set ("registered", "1");

The user can access cvars from the console in two ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0

The "set" command works similar. But when used with a value and the cvar does
not exist, then the cvar is created. The "seta" command additionally sets the
archive flag, so it will be stored in config.cfg.

Cvars are no more restricted from having the same names as commands, because of the "set" command.
*/

// bit flags for cvars
#define CVAR_NONE			0		//cvar has no flags
#define CVAR_ARCHIVE		1		//cvar will be stored in config.cfg
#define CVAR_ROM			2		//cvar is readonly
#define	CVAR_NOTIFY			4		//cvar changes will be broadcasted to all players
#define	CVAR_SERVERINFO		8		//cvar will be send to clients (net_dgrm.c)
#define	CVAR_USERINFO		16		//cvar will be send to server (QW-like)

#define CVAR_ORIGINAL		1024	//cvar from the original engine (stored in config.cfg also without seta for compatibility reasons)
#define CVAR_USER_CREATED	2048	//cvar created by user
#define CVAR_PROGS_CREATED	4096	//cvar created by progs.dat


typedef char *(*cvar_rangecheck) (struct cvar_s *var, char *value, qboolean showinfo);

typedef void (*cvar_callback) (struct cvar_s *var);

typedef struct cvar_s {
	char			*name;
	char			*string;
	float			value;
	int				flags;

	struct cvar_s	*prev;
	struct cvar_s	*next;

	// EVIL! cvar_callback	*callback; won't work here
	void			(*callback) (struct cvar_s *var);

	// EVIL! cvar_rangecheck	*rangecheck; won't work here
	char			*(*rangecheck) (struct cvar_s *var, char *value, qboolean showinfo);
	float			minvalue;
	float			maxvalue;

	char			*description;
} cvar_t;


extern cvar_t	*cvar_list;


void Cvar_Init (void);

cvar_t *Cvar_Get (char *name, char *string, int flags);

cvar_t *Cvar_Free (cvar_t *var);

cvar_t *Cvar_FindVar (char *name);

void Cvar_Set (cvar_t *var, char *value);
// equivelant to "<name> <variable>" typed at the console

void Cvar_SetValue (cvar_t *var, float value);
// expands value to a string and calls Cvar_Set

qboolean Cvar_Command (void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void Cvar_WriteVariables (FILE *f, qboolean new_rc_file);
// Writes lines containing "set variable value" for all variables
// with the archive flag set to true.

char *Cvar_CompleteVariable (char *partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

void Cvar_SetCallback (cvar_t *var, cvar_callback callback);

#define CVAR_MAX_VALSTRING	256
void Cvar_SetRangecheck (cvar_t *var, cvar_rangecheck rangecheck, float minvalue, float maxvalue);

char *Cvar_RangecheckBool (cvar_t *var, char *value, qboolean showinfo);
char *Cvar_RangecheckInt (cvar_t *var, char *value, qboolean showinfo);
char *Cvar_RangecheckFloat (cvar_t *var, char *value, qboolean showinfo);

void Cvar_SetDescription (cvar_t *var, char *description);

// 2001-12-15 Enhanced console command completion by Fett/Maddes  start
int Cvar_CompleteCountPossible (char *partial);
void Cvar_CompletePrintPossible (char *partial);
// 2001-12-15 Enhanced console command completion by Fett/Maddes  end
