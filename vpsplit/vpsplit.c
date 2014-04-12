/*
==============================================================================
Title: "VisPatch file splitter"
(c) 2001 by Matthias "Maddes" Buecher

Authors:
Matthias "Maddes" Buecher <maddes@go.to>
http://www.quake-info-pool.net/

Most of this code is easy to comprehend and there should not be too many
questions about how it works.  The heart of the program has been slighty
commented out to help you understand the technical operations being done.

DISCLAIMER WARNING:
WE DO NOT ACCEPT RESPONSIBILITY FOR ANY EFFECTS, POSITIVE OR NEGATIVE,
THAT THIS CODE MAY CAUSE TOWARDS YOUR COMPUTER.
WE DO NOT SUPPORT MODIFICATIONS OF THIS CODE, USE ONLY AT YOUR OWN RISK.
THIS PATCH HAS NOT BEEN ENDORSED BY THE ID SOFTWARE CORPORATION.
THIS PROGRAM IS FREE OF CHARGE. IF YOU HAVE RECEIVED THIS PROGRAM THROUGH
PAYMENT, PLEASE CONTACT US.
THANK YOU.
==============================================================================
*/

/* Compatible headers */
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* needed to compile under Linux */
#ifndef _WIN32
#define stricmp strcasecmp
#endif

/* constants */
#define VERSION		"1.00 (2001-12-31)"
#define PARM_CHAR	'-'
#define BUFLEN		1024
#define VISPATCH_MAPNAME_LENGTH	32

/*
structure definitions
*/
typedef unsigned char       BYTE;

/* VisPatch file */
typedef struct vispatch_s {
	char	mapname[VISPATCH_MAPNAME_LENGTH];	/* map for which these data are for, always use strncpy and strncmp for this field */
	int		filelen;		/* length of data after VisPatch header (VIS+Leafs) */
} vispatch_t;

/* global data definition */
char in_filename[BUFLEN+1];
FILE *in_handle;

char *extension;			/* pointer to extension string */

int flag_error;				/* error occured */
int flag_warning;			/* warning occured */

// taken from Quake's COMMON.C
int		(*LittleLong) (int l);
int		(*BigLong) (int l);


/*
******************************************************************************
*
*   G E N E R A L   F U N C T I O N S
*
******************************************************************************
*/

// taken from Quake's COMMON.C
int LongSwap (int l)
{
	BYTE	b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

// taken from Quake's COMMON.C
int LongNoSwap (int l)
{
	return l;
}


/*
==============================================================================
transforms a string to lowercase
==============================================================================
*/
void strlower(char *text)
{
	int i;
	int length;

	length = strlen(text);
	for (i=0; i<length; i++)
		text[i] = (char)tolower(text[i]);
}

/*
==============================================================================
gets the extension out of the path
==============================================================================
*/
char *get_extension(const char *const filename)
{
	char *dot;
	char *slash;
	char *backslash;

	dot = strrchr(filename, '.');
	slash = strrchr(filename, '/');
	backslash = strrchr(filename, '\\');

	if ((dot) && (dot > slash) && (dot > backslash))
	{
		return dot;
	}

	return NULL;		// no extension found
}

/*
******************************************************************************
*
*   U S E R   I N T E R F A C E
*
******************************************************************************
*/

/*
==============================================================================
displays the help text
==============================================================================
*/
void display_help(char *call)
{
	/* syntax */
	printf("Usage: VPSplit [-?] <vispatchfile>\n");
	printf("  -?           : displays this help text\n");

	/* example */
	printf("\nExample:\n");
	printf("%s \"example.vis\"\n", call);
	printf("Extracts the data for each single map inside the vispatch file \"example.vis\"\n");

	/* contact */
	printf("\nHow to contact the author:\n");
	printf("Matthias \"Maddes\" Buecher <maddes@go.to>\n");
	printf("http://www.quake-info-pool.net/\n");
}

/*
==============================================================================
checks if string begins with a parameter character
==============================================================================
*/
int chk_paramchar(char *text)
{
	/* check for parameter character at beginning */
	if (text[0] == PARM_CHAR)
	{
		strlower(text);
		return 1;
	}
	else
		return 0;
}

/*
==============================================================================
checks all parameters
when help parameter is found it displays the help text and leaves the function
checks if the ini file is present
displays all used parameters and asks for confirmation
==============================================================================
*/
void chk_param(int argc, char *argv[])
{
	/* define local variables */
	int i;			/* iteration variable */
	int flag_help;		/* display help */

	/* initialize local variables */
	flag_error = 0;
	flag_warning = 0;
	flag_help = 0;

	/* parse parameters */
	for (i=1; i<argc; i++)
	{
		if (chk_paramchar(argv[i]))
		{
			if (!strcmp(argv[i],"-?") || !stricmp(argv[i],"-h") || !stricmp(argv[i],"-help"))	/* help parameter */
			{
				flag_help = 1;
				flag_error = 1;
				break;
			}
			else				/* unknown parameter */
			{
				printf("Error on parameter %i: \"%s\" is an unknown parameter\n", i, argv[i]);
				flag_help = 1;
				flag_error = 2;
			}
		}
		else	/* should be the filename */
		{
			if (!*in_filename)
			{
				strcpy(in_filename, argv[i]);
			}
			else
			{
				printf("Error on parameter %i: \"%s\", a vispatch file was already named\n", i, argv[i]);
				flag_help = 1;
				flag_error = 2;
			}
		}
	}

	/* check if bsp file exists */
	if (!*in_filename)
	{
		printf("Error: no vispatch file named\n");
		flag_error = 2;
		flag_help = 1;
	}
	else
	{
		in_handle = fopen(in_filename, "rb");
		if (!in_handle)
		{
			// check extension
			extension = get_extension(in_filename);
			if ( (!extension) || (stricmp(extension, ".vis")) || (stricmp(extension, ".dat")))
			{
				strcat(in_filename, ".vis");
				in_handle = fopen(in_filename, "rb");
			}
		}

		if (!in_handle)
		{
			printf("Error: could not open vispatch file \"%s\" - %s\n", in_filename, strerror(errno));
			flag_error = 2;
		}
		else
		{
			fclose(in_handle);
			in_handle = NULL;
		}
	}

	/* asked for help */
	if (flag_error > 1 || flag_warning)
	{
		printf("\n");
	}

	if (flag_help)
	{
		display_help(argv[0]);
	}

	if (!flag_error)
	{
		printf("VisPatch file: \"%s\"\n", in_filename);

		printf("\n");
	}
}

/*
******************************************************************************
*
*   F I L E S   R E A D I N G   &   W R I T I N G
*
******************************************************************************
*/

void process_file(void)
{
	int			in_filelength;
	BYTE		*in_data;
	int			i;
	int			offset;
	vispatch_t	*header;
	char		out_filename[VISPATCH_MAPNAME_LENGTH+5];	// + ".vis" + EoS
	FILE		*out_handle;
	int			out_filelength;
	long		filelen;

	/* open vispatch file */
	in_handle = fopen(in_filename, "rb");
	if (!in_handle)
	{
		printf("Error: could not open vispatch file \"%s\" - %s\n", in_filename, strerror(errno));
		flag_error = 2;
		return;
	}

	/* get file length */
	if ( fseek(in_handle, 0, SEEK_END) != 0)
	{
		printf("Error: couldn't seek file end - %s\n", strerror(errno));
		flag_error = 2;
		return;
	}
	in_filelength = ftell(in_handle);
	if ( in_filelength < 0)
	{
		printf("Error: couldn't get file length - %s\n", strerror(errno));
		flag_error = 2;
		return;
	}
	if ( fseek(in_handle, 0, SEEK_SET) != 0)
	{
		printf("Error: couldn't seek file start - %s\n", strerror(errno));
		flag_error = 2;
		return;
	}

	/* allocate memory for file */
	in_data = malloc(in_filelength);
	if (!in_data)
	{
		printf("Error: couldn't allocate memory for vispatch file\n");
		flag_error = 2;
		return;
	}

	/* read file into memory */
	i = fread(in_data, 1, in_filelength, in_handle);
	if ( i != in_filelength)
	{
		printf("Error: read %i bytes from vispatch file, expected %i\n", i, in_filelength);
		flag_error = 2;
		return;
	}

	/* close file */
	fclose(in_handle);

	/* extract map data one by one */
	offset = 0;
	while (offset < in_filelength)
	{
			/* get vispatch header */
			header = (vispatch_t *)(in_data + offset);
			filelen = LittleLong(header->filelen);

			/* clean up original header */
			strncpy(header->mapname, header->mapname, VISPATCH_MAPNAME_LENGTH);	// remove junk from original header

			/* get output name for map data */
			strncpy(out_filename, header->mapname, VISPATCH_MAPNAME_LENGTH);
			out_filename[VISPATCH_MAPNAME_LENGTH] = 0;
			extension = get_extension(out_filename);
			if (extension)
			{
				extension[0] = 0;
			}
			strcat(out_filename, ".vis");

			/* map output length */
			out_filelength = filelen + sizeof(struct vispatch_s);
			printf("\"%.32s\" at %i: \"%s\" with %i(%i) bytes\n", header->mapname, offset, out_filename, filelen, out_filelength);

			/* open output file */
			out_handle = fopen(out_filename, "wb");
			if (!out_handle)
			{
				printf("Error: could not create output file \"%s\" - %s\n", out_filename, strerror(errno));
				flag_error = 2;
				break;
			}

			/* write output file */
			i = fwrite(header, 1, out_filelength, out_handle);
			if ( i != out_filelength)
			{
				printf("Error: wrote %i bytes in %s, expected %i\n", i, out_filename, out_filelength);
				flag_error = 2;
				return;
			}

			/* close output file */
			fclose(out_handle);

			/* next map date */
			offset += out_filelength;
	};
}

/*
******************************************************************************
*
*   M A I N   F U N C T I O N   ( S T A R T   O F   P R O G R A M )
*
******************************************************************************
*/

/*
==============================================================================
here the program starts and all sub functions are called
==============================================================================
*/
int main(int argc, char *argv[])
{
	/* initialize variables */
	flag_error = 0;
	flag_warning = 0;
	*in_filename = 0;

	// taken from Quake's COMMON.C
	BYTE	swaptest[2] = {1,0};

	// set the byte swapping variables in a portable manner
	if ( *(short *)swaptest == 1)
	{
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
	}
	else
	{
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
	}

	/* display program header lines */
	printf("VisPatch File Splitter\n");
	printf("(c) 2001 by Matthias \"Maddes\" Buecher\n");
	printf("Version %s\n\n", VERSION);

	/* check parameters */
	chk_param(argc, argv);
	if (flag_error)
	{
		exit(flag_error);
	}

	/* here we go with the "real" program */
	process_file();
	if (flag_error)
	{
		exit(flag_error);
	}

	printf("Finished\n");

	return 0;
}
