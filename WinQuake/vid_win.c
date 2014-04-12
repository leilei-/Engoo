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
// vid_win.c -- Win32 video driver

#include "quakedef.h"
#include "winquake.h"
#include "d_local.h"
#include "resource.h"
#include <ddraw.h>

// true if the ddraw driver started up OK
qboolean vid_usingddraw = false;

// main application window
HWND hWndWinQuake = NULL;

// compatibility
HWND mainwindow = NULL;
int			vidmodetweak = 0;	// leilei - this is always 0 for the widescreen stuff. No modern computer will ever run
								// 320x200/320x400 natively ever again, and that's a given.
int	wellistretcheditooutokay;	// leilei - window stretch hack
byte gammatable[256];
extern	int reflectavailable;	// leilei
float	usingstretch;	// fteqw
float	usingstretchh;	// leilei
float	usingstretchv;	// leilei
int		windreshack		=	3;	// leilei
int		imsizingawindowactually;	// leilei
int		iamnotsizingthewindow;
int	d_con_indirect = 0;
int	yeahimconsoled;
int		thatmode;
int		eased;	// leilei - for console and menu, not just pause
static void Check_Gamma (void)
{
	int i;
	float gamm = 0.7f;
	float f, inf;

	if ((i = COM_CheckParm ("-gamma")) == 0)
		gamm = 1.0f;
	else gamm = Q_atof (com_argv [i + 1]);

	for (i = 0; i < 256; i++)
	{
		f = pow ((i + 1) / 256.0, gamm);
		inf = f * 255 + 0.5;

		if (inf < 0) inf = 0;
		if (inf > 255) inf = 255;

		gammatable[i] = (unsigned char)inf;
	}
}


/*
=================================================================================================================

				DIRECTDRAW VIDEO DRIVER

=================================================================================================================
*/

LPDIRECTDRAW dd_Object = NULL;
HINSTANCE hInstDDraw = NULL;

LPDIRECTDRAWSURFACE dd_FrontBuffer = NULL;
LPDIRECTDRAWSURFACE dd_BackBuffer = NULL;

LPDIRECTDRAWCLIPPER dd_Clipper = NULL;

typedef HRESULT (WINAPI *DIRECTDRAWCREATEPROC) (GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
DIRECTDRAWCREATEPROC QDirectDrawCreate = NULL;

unsigned int ddpal[256];

unsigned char *vidbuf = NULL;


int dd_window_width = 640;
int dd_window_height = 480;
RECT SrcRect;
RECT DstRect;

void DD_UpdateRects (int width, int height)
{
	POINT p;

	p.x = 0;
	p.y = 0;

	// first we need to figure out where on the primary surface our window lives
	ClientToScreen (hWndWinQuake, &p);
	GetClientRect (hWndWinQuake, &DstRect);
	OffsetRect (&DstRect, p.x, p.y);
	SetRect (&SrcRect, 0, 0, width, height);
}
extern float oldgammavalue;

void VID_CreateDDrawDriver (int width, int height, unsigned char *palette, unsigned char **buffer, int *rowbytes)
{
	HRESULT hr;
	DDSURFACEDESC ddsd;

	vid_usingddraw = false;
	dd_window_width = width;
	dd_window_height = height;

	vidbuf = (unsigned char *) malloc (width * height);
	buffer[0] = vidbuf;
	rowbytes[0] = width;


	if (!(hInstDDraw = LoadLibrary ("ddraw.dll"))) return;
	if (!(QDirectDrawCreate = (DIRECTDRAWCREATEPROC) GetProcAddress (hInstDDraw, "DirectDrawCreate"))) return;

	if (FAILED (hr = QDirectDrawCreate (NULL, &dd_Object, NULL))) return;
	if (FAILED (hr = dd_Object->lpVtbl->SetCooperativeLevel (dd_Object, hWndWinQuake, DDSCL_NORMAL))) return;

	// the primary surface in windowed mode is the full screen
	memset (&ddsd, 0, sizeof (ddsd));
	ddsd.dwSize = sizeof (ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;

	// ...and create it
	if (FAILED (hr = dd_Object->lpVtbl->CreateSurface (dd_Object, &ddsd, &dd_FrontBuffer, NULL))) return;

	// not using a clipper will slow things down and switch aero off
	if (FAILED (hr = IDirectDraw_CreateClipper (dd_Object, 0, &dd_Clipper, NULL))) return;
	if (FAILED (hr = IDirectDrawClipper_SetHWnd (dd_Clipper, 0, hWndWinQuake))) return;
	if (FAILED (hr = IDirectDrawSurface_SetClipper (dd_FrontBuffer, dd_Clipper))) return;

	// the secondary surface is an offscreen surface that is the currect dimensions
	// this will be blitted to the correct location on the primary surface (which is the full screen) during our draw op
	memset (&ddsd, 0, sizeof (ddsd));
	ddsd.dwSize = sizeof (ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth = width;
	ddsd.dwHeight = height;

	if (FAILED (hr = IDirectDraw_CreateSurface (dd_Object, &ddsd, &dd_BackBuffer, NULL))) return;
        
	// direct draw is working now
	vid_usingddraw = true;

	// create a palette
	oldgammavalue = 1; // leilei - GAMMA hack 
	Check_Gamma ();
	VID_SetPalette (palette);

	// create initial rects
	DD_UpdateRects (dd_window_width, dd_window_height);

	// leilei - water reflection
#ifdef WATERREFLECTIONS
	vid.reflectbuffer = (unsigned char *) malloc (vid.width * vid.height);
	memset (vid.reflectbuffer, 0xff, vid.width * vid.height);
	reflectavailable = 1;	// :)
#endif
}


/*
=================================================================================================================

				GDI VIDEO DRIVER

=================================================================================================================
*/

// common bitmap definition
typedef struct dibinfo
{
	BITMAPINFOHEADER	header;
	RGBQUAD				acolors[256];
} dibinfo_t;


static HGDIOBJ previously_selected_GDI_obj = NULL;
HBITMAP hDIBSection;
unsigned char *pDIBBase = NULL;
HDC hdcDIBSection = NULL;
HDC hdcGDI = NULL;


void VID_CreateGDIDriver (int width, int height, unsigned char *palette, unsigned char **buffer, int *rowbytes)
{
	dibinfo_t   dibheader;
	BITMAPINFO *pbmiDIB = (BITMAPINFO *) &dibheader;
	int i;

	hdcGDI = GetDC (hWndWinQuake);
	memset (&dibheader, 0, sizeof (dibheader));

	// fill in the bitmap info
	pbmiDIB->bmiHeader.biSize          = sizeof (BITMAPINFOHEADER);
	pbmiDIB->bmiHeader.biWidth         = width;
	pbmiDIB->bmiHeader.biHeight        = height;
	pbmiDIB->bmiHeader.biPlanes        = 1;
	pbmiDIB->bmiHeader.biCompression   = BI_RGB;
	pbmiDIB->bmiHeader.biSizeImage     = 0;
	pbmiDIB->bmiHeader.biXPelsPerMeter = 0;
	pbmiDIB->bmiHeader.biYPelsPerMeter = 0;
	pbmiDIB->bmiHeader.biClrUsed       = 256;
	pbmiDIB->bmiHeader.biClrImportant  = 256;
	pbmiDIB->bmiHeader.biBitCount      = 8;

	// fill in the palette
	for (i = 0; i < 256; i++)
	{
		// d_8to24table isn't filled in yet so this is just for testing
		dibheader.acolors[i].rgbRed   = palette[i * 3];
		dibheader.acolors[i].rgbGreen = palette[i * 3 + 1];
		dibheader.acolors[i].rgbBlue  = palette[i * 3 + 2];
	}

	// create the DIB section
	hDIBSection = CreateDIBSection (hdcGDI,
							pbmiDIB,
							DIB_RGB_COLORS,
							&pDIBBase,
							NULL,
							0);

	// set video buffers
	if (pbmiDIB->bmiHeader.biHeight > 0)
	{
		// bottom up
		buffer[0] = pDIBBase + (height - 1) * width;
		rowbytes[0] = -width;
	}
	else
	{
		// top down
		buffer[0] = pDIBBase;
		rowbytes[0] = width;
	}

	// clear the buffer
	memset (pDIBBase, 0xff, width * height);

	if ((hdcDIBSection = CreateCompatibleDC (hdcGDI)) == NULL)
		Sys_Error ("DIB_Init() - CreateCompatibleDC failed\n");

	if ((previously_selected_GDI_obj = SelectObject (hdcDIBSection, hDIBSection)) == NULL)
		Sys_Error ("DIB_Init() - SelectObject failed\n");



	// create a palette
	Check_Gamma ();
	VID_SetPalette (palette);
}


void VID_UnloadAllDrivers (void)
{
	// shut down ddraw
	if (vidbuf)
	{
		free (vidbuf);
		vidbuf = NULL;
	}

	if (dd_Clipper)
	{
		IDirectDrawClipper_Release (dd_Clipper);
		dd_Clipper = NULL;
	}

	if (dd_FrontBuffer)
	{
		IDirectDrawSurface_Release (dd_FrontBuffer);
		dd_FrontBuffer = NULL;
	}

	if (dd_BackBuffer)
	{
		IDirectDrawSurface_Release (dd_BackBuffer);
		dd_BackBuffer = NULL;
	}

	if (dd_Object)
	{
		IDirectDraw_Release (dd_Object);
		dd_Object = NULL;
	}

	if (hInstDDraw)
	{
		FreeLibrary (hInstDDraw);
		hInstDDraw = NULL;
	}

	QDirectDrawCreate = NULL;

	// shut down gdi
	if (hdcDIBSection)
	{
		SelectObject (hdcDIBSection, previously_selected_GDI_obj);
		DeleteDC (hdcDIBSection);
		hdcDIBSection = NULL;
	}

	if (hDIBSection)
	{
		DeleteObject (hDIBSection);
		hDIBSection = NULL;
		pDIBBase = NULL;
	}

	if (hdcGDI)
	{
		// if hdcGDI exists then hWndWinQuake must also be valid
		ReleaseDC (hWndWinQuake, hdcGDI);
		hdcGDI = NULL;
	}

	// not using ddraw now
	vid_usingddraw = false;
	reflectavailable = 0;	// leilei - :(
#ifdef WATERREFLECTIONS
	if (vid.reflectbuffer)
		vid.reflectbuffer = NULL;
#endif
}



// compatibility
qboolean		DDActive;

// not used any more
void VID_LockBuffer (void) {}
void VID_UnlockBuffer (void) {}
int VID_ForceUnlockedAndReturnState (void) {return 0;}
void VID_ForceLockState (int lk) {}


#define MAX_MODE_LIST	36
#define VID_ROW_SIZE	3

extern int		Minimized;

HWND WINAPI InitializeWindow (HINSTANCE hInstance, int nCmdShow);

int			DIBWidth, DIBHeight;
RECT		WindowRect;
DWORD		WindowStyle, ExWindowStyle;

int			window_center_x, window_center_y, window_x, window_y, window_width, window_height;
RECT		window_rect;

static DEVMODE	gdevmode;
static qboolean	startwindowed = 0, windowed_mode_set;
static int		firstupdate = 1;
static qboolean	vid_initialized = false, vid_palettized;
static int		vid_fulldib_on_focus_mode;
static qboolean	force_minimized, in_mode_set, is_mode0x13, force_mode_set;
static int		windowed_mouse;
static qboolean	palette_changed, syscolchg, vid_mode_set, hide_window, pal_is_nostatic;
static HICON	hIcon;

viddef_t	vid;				// global video state

#define MODE_WINDOWED			0
#define MODE_SETTABLE_WINDOW	2
#define NO_MODE					(MODE_WINDOWED - 1)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 3)

cvar_t	*vid_ddraw;

// Note that 0 is MODE_WINDOWED
cvar_t	*vid_mode;
// Note that 0 is MODE_WINDOWED
cvar_t	*_vid_default_mode;
// Note that 3 is MODE_FULLSCREEN_DEFAULT
cvar_t	*_vid_default_mode_win;
cvar_t	*vid_wait;
cvar_t	*vid_nopageflip;
cvar_t	*_vid_wait_override;
cvar_t	*vid_config_x;
cvar_t	*vid_config_y;
cvar_t	*vid_stretch_by_2;
cvar_t	*_windowed_mouse;
cvar_t	*vid_fullscreen_mode;
cvar_t	*vid_windowed_mode;
cvar_t	*block_switch;
cvar_t	*vid_window_x;
cvar_t	*vid_window_y;

// leilei additions that may or may not be implemented
cvar_t	*vid_window_width;
cvar_t	*vid_window_height;


cvar_t	*vid_aspectforce;
cvar_t	*vid_aspectfilter; // 0 - off, 1 - 4:3, 2 - 5:4, 3 - 16:9, 4 - 16:10
cvar_t	*vid_depth;	


int			vid_modenum = NO_MODE;
int			vid_testingmode, vid_realmode;
double		vid_testendtime;
int			vid_default = MODE_WINDOWED;
static int	windowed_default;

modestate_t	modestate = MS_UNINIT;

static byte		*vid_surfcache;
static int		vid_surfcachesize;

unsigned char	vid_curpal[256*3];

unsigned short	d_8to16table[256];

#ifdef statictest
unsigned		d_8to24table[256];
#else
unsigned 	d_8to24table[256];
#endif

int     mode;

typedef struct
{
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			fullscreen;
	char		modedesc[13];
	float		aspect;
	int			dummy;
} vmode_t;

static vmode_t	modelist[MAX_MODE_LIST];
static int		nummodes;
static int		modenum2;
static vmode_t	*pcurrentmode;

int		aPage;					// Current active display page
int		vPage;					// Current visible display page
int		waitVRT = true;			// True to wait for retrace on flip

static vmode_t	badmode;

static byte	backingbuf[48*24];

void VID_MenuDraw (void);
void VID_MenuKey (int key);

LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AppActivate (BOOL fActive, BOOL minimize);

// video commands
int VID_NumModes (void);
vmode_t *VID_GetModePtr (int modenum);
void VID_TestMode_f (void);
void VID_ForceMode_f (void);
void VID_Minimize_f (void);
void VID_Fullscreen_f (void);
void VID_Windowed_f (void);
void VID_DescribeModes_f (void);
void VID_DescribeMode_f (void);
void VID_NumModes_f (void);
void VID_DescribeCurrentMode_f (void);
char *VID_GetExtModeDescription (int mode);
char *VID_GetModeDescription (int mode);
char *VID_GetModeDescription2 (int mode);
char *VID_GetModeDescriptionMemCheck (int mode);
void VID_CheckModedescFixup (int mode);


/*
================
VID_RememberWindowPos
================
*/
void VID_RememberWindowPos (void)
{
	RECT	rect;

	if (GetWindowRect (hWndWinQuake, &rect))
	{
		if ((rect.left < GetSystemMetrics (SM_CXSCREEN)) &&
				(rect.top < GetSystemMetrics (SM_CYSCREEN))  &&
				(rect.right > 0)                             &&
				(rect.bottom > 0))
		{
			Cvar_SetValue (vid_window_x, (float) rect.left);
			Cvar_SetValue (vid_window_y, (float) rect.top);
		}
	}
}


/*
================
VID_CheckWindowXY
================
*/
void VID_CheckWindowXY (void)
{
	if (((int) vid_window_x->value > (GetSystemMetrics (SM_CXSCREEN) - 160)) ||
			((int) vid_window_y->value > (GetSystemMetrics (SM_CYSCREEN) - 120)) ||
			((int) vid_window_x->value < 0)									   ||
			((int) vid_window_y->value < 0))
	{
		Cvar_SetValue (vid_window_x, 0.0);
		Cvar_SetValue (vid_window_y, 0.0);
	}
}

/*
================
VID_CheckWindowSize
================
*/
void VID_CheckWindowSize (void)
{
	if (((int) vid_window_width->value > (GetSystemMetrics (SM_CXSIZEFRAME))) ||
			((int) vid_window_height->value > (GetSystemMetrics (SM_CYSIZEFRAME))) ||
			((int) vid_window_width->value < 0)									   ||
			((int) vid_window_height->value < 0))
	{
		Cvar_SetValue (vid_window_width, 0.0);
		Cvar_SetValue (vid_window_height, 0.0);
		
	}
}
/*
================
VID_UpdateWindowStatus
================
*/
void VID_UpdateWindowStatus (void)
{
	window_rect.left = window_x;
	window_rect.top = window_y;
	window_rect.right = window_x + window_width;
	window_rect.bottom = window_y + window_height;
	window_center_x = (window_rect.left + window_rect.right) / 2;
	window_center_y = (window_rect.top + window_rect.bottom) / 2;
	IN_UpdateClipCursor ();
}


/*
================
ClearAllStates
================
*/
void ClearAllStates (void)
{
	int		i;

	// send an up event for each key, to make sure the server clears them all
	for (i = 0; i < 256; i++)
	{
		Key_Event (i, false);
	}

	Key_ClearStates ();
	IN_ClearStates ();
}


/*
================
VID_CheckAdequateMem
================
*/
qboolean VID_CheckAdequateMem (int width, int height)
{
	// there will always be enough ;)
	return true;
}


/*
================
VID_AllocBuffers
================
*/
qboolean VID_AllocBuffers (int width, int height)
{
	int		tsize, tbuffersize;

	tbuffersize = width * height * sizeof (*d_pzbuffer);
	tsize = D_SurfaceCacheForRes (width, height);
	tbuffersize += tsize;

	vid_surfcachesize = tsize;

	if (d_pzbuffer)
	{
		D_FlushCaches ();
		Z_Free (mainzone, d_pzbuffer);
		d_pzbuffer = NULL;
	}
	d_pzbuffer = malloc(tbuffersize);
//	d_pzbuffer = Z_Malloc (mainzone, tbuffersize);
	vid_surfcache = (byte *) d_pzbuffer +
					width * height * sizeof (*d_pzbuffer);

	return true;
}


void initFatalError (void)
{
	exit (EXIT_FAILURE);
}


void registerAllMemDrivers (void)
{
}


void VID_InitModes (HINSTANCE hInstance)
{
	WNDCLASS		wc;
	HDC				hdc;
	int				i;
	hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON2));
	/* Register the frame class */
	wc.style         = CS_OWNDC;
	wc.lpfnWndProc   = (WNDPROC) MainWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = 0;
	wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = "WinQuake";

	if (!RegisterClass (&wc))
		Sys_Error ("Couldn't register window class");
#ifdef EGAHACK
	modelist[0].type = MS_WINDOWED;
	modelist[0].width = 320;
	modelist[0].height = 200;
	strcpy (modelist[0].modedesc, "320x200x4");
	modelist[0].modenum = MODE_WINDOWED;
	modelist[0].fullscreen = 0;
	modelist[0].aspect = modelist[0].width / modelist[0].height;
	modelist[0].dummy = 0;

	modelist[1].type = MS_WINDOWED;
	modelist[1].width = 640;
	modelist[1].height = 200;
	strcpy (modelist[1].modedesc, "640x200x4");
	modelist[1].modenum = MODE_WINDOWED + 1;
	modelist[1].fullscreen = 0;
	modelist[1].aspect = modelist[1].width / modelist[1].height;
	modelist[1].dummy = 0;

	modelist[2].type = MS_WINDOWED;
	modelist[2].width = 640;
	modelist[2].height = 350;
	strcpy (modelist[2].modedesc, "640x350x4");
	modelist[2].modenum = MODE_WINDOWED + 2;
	modelist[2].fullscreen = 0;
	modelist[2].aspect = modelist[2].width / modelist[2].height;
	modelist[2].dummy = 0;
#else
	modelist[0].type = MS_WINDOWED;
	modelist[0].width = 320;
	modelist[0].height = 240;
	strcpy (modelist[0].modedesc, "320x240");
	modelist[0].modenum = MODE_WINDOWED;
	modelist[0].fullscreen = 0;
	modelist[0].aspect = modelist[0].width / modelist[0].height;
	modelist[0].dummy = 0;

	modelist[1].type = MS_WINDOWED;
	modelist[1].width = 640;
	modelist[1].height = 480;
	strcpy (modelist[1].modedesc, "640x480");
	modelist[1].modenum = MODE_WINDOWED + 1;
	modelist[1].fullscreen = 0;
	modelist[1].aspect = modelist[1].width / modelist[1].height;
	modelist[1].dummy = 0;

	modelist[2].type = MS_WINDOWED;
	modelist[2].width = 800;
	modelist[2].height = 600;
	strcpy (modelist[2].modedesc, "800x600");
	modelist[2].modenum = MODE_WINDOWED + 2;
	modelist[2].fullscreen = 0;
	modelist[2].aspect = modelist[2].width / modelist[2].height;
	modelist[2].dummy = 0;

	modelist[3].type = MS_WINDOWED;
	modelist[3].width = 960;
	modelist[3].height = 480;
	strcpy (modelist[3].modedesc, "960x480");
	modelist[3].modenum = MODE_WINDOWED + 3;
	modelist[3].fullscreen = 0;
	modelist[3].aspect = modelist[3].width / modelist[3].height;
	modelist[3].dummy = 0;


	modelist[4].type = MS_WINDOWED;
	modelist[4].width = 1280;
	modelist[4].height = 720;
	strcpy (modelist[4].modedesc, "1280x720");
	modelist[4].modenum = MODE_WINDOWED + 4;
	modelist[4].fullscreen = 0;
	modelist[4].aspect = modelist[4].width / modelist[4].height;
	modelist[4].dummy = 0;

	modelist[5].type = MS_WINDOWED;
	modelist[5].width = 1600;
	modelist[5].height = 900;		// for stretching only...
	strcpy (modelist[5].modedesc, "Custom Res");
	modelist[5].modenum = MODE_WINDOWED + 5;
	modelist[5].fullscreen = 0;
	modelist[5].aspect = 0;
	modelist[5].dummy = 1;

	modelist[6].type = MS_WINDOWED;
	modelist[6].width = 640;
	modelist[6].height = 480;		// for stretching only...
	strcpy (modelist[6].modedesc, "Custom Res");
	modelist[6].modenum = MODE_WINDOWED + 6;
	modelist[6].fullscreen = 0;
	modelist[6].aspect = 0;
	modelist[6].dummy = 1;

#endif

	// automatically stretch the default mode up if > 640x480 desktop resolution
	hdc = GetDC (NULL);

	if ((GetDeviceCaps (hdc, HORZRES) > 800) && !COM_CheckParm ("-noautostretch"))
	{
		vid_default = MODE_WINDOWED + 2;
	}
	else if ((GetDeviceCaps (hdc, HORZRES) > 640) && !COM_CheckParm ("-noautostretch"))
	{
		vid_default = MODE_WINDOWED + 1;
	}
	else
	{
		vid_default = MODE_WINDOWED;
	}

	// always start at the lowest mode then switch to the higher one if selected
	vid_default = MODE_WINDOWED;

	//windowed_default = vid_default;
	windowed_default = 0;
	ReleaseDC (NULL, hdc);
	nummodes = 7;	// reserve space for windowed mode
}


/*
=================
VID_GetDisplayModes
=================
*/
void VID_GetDisplayModes (void)
{
	DEVMODE	devmode;
	int		i, j, modenum, cmodes, existingmode, originalnummodes, lowestres;
	int		numlowresmodes, bpp, done;
	int		cstretch, istretch, mstretch;
	BOOL	stat;

	// enumerate > 8 bpp modes
	originalnummodes = nummodes;
	modenum = 0;
	lowestres = 99999;

	do
	{
		stat = EnumDisplaySettings (NULL, modenum, &devmode);

		if ((devmode.dmPelsWidth <= MAXWIDTH) &&
				(devmode.dmPelsHeight <= MAXHEIGHT) &&
				(devmode.dmPelsWidth >= 320) &&
				(devmode.dmPelsHeight >= 240) &&
				(nummodes < MAX_MODE_LIST))
		{
			devmode.dmFields = DM_BITSPERPEL |
							   DM_PELSWIDTH |
							   DM_PELSHEIGHT;

			if (ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) ==
					DISP_CHANGE_SUCCESSFUL)
			{
				modelist[nummodes].type = MS_FULLDIB;
				modelist[nummodes].width = devmode.dmPelsWidth;
				modelist[nummodes].height = devmode.dmPelsHeight;
				modelist[nummodes].modenum = 0;
				modelist[nummodes].fullscreen = 1;
				modelist[nummodes].aspect = devmode.dmPelsWidth / devmode.dmPelsHeight;
				modelist[nummodes].dummy = 0;	// fullscreen modes are never dynamic like a window

				sprintf (modelist[nummodes].modedesc, "%dx%d",
						 devmode.dmPelsWidth, devmode.dmPelsHeight);

				// see is the mode already there
				// (same dimensions but different refresh rate)
				for (i = originalnummodes, existingmode = 0; i < nummodes; i++)
				{
					if ((modelist[nummodes].width == modelist[i].width) &&
							(modelist[nummodes].height == modelist[i].height))
					{
						existingmode = 1;
						break;
					}
				}

				// if it's not add it to the list
				if (!existingmode && !modelist[nummodes].dummy)
				{
					if (modelist[nummodes].width < lowestres)
						lowestres = modelist[nummodes].width;

					nummodes++;
				}
			}
		}

		modenum++;
	}
	while (stat);

	if (nummodes != originalnummodes)
		vid_default = MODE_FULLSCREEN_DEFAULT;
	else Con_SafePrintf ("No fullscreen DIB modes found\n");
}


void VID_DestroyWindow (void)
{
	if (modestate == MS_FULLDIB)
		ChangeDisplaySettings (NULL, CDS_FULLSCREEN);

	VID_UnloadAllDrivers ();
}




void VID_SetAspect (void)
{
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
}



qboolean VID_SetWindowedMode (int modenum)
{
	HDC				hdc;
	qboolean		stretched;
	int				lastmodestate;
	LONG			wlong;

	if (!windowed_mode_set)
	{
		if (COM_CheckParm ("-resetwinpos"))
		{
			Cvar_SetValue (vid_window_x, 0.0);
			Cvar_SetValue (vid_window_y, 0.0);
		}

		windowed_mode_set;
	}

	VID_CheckModedescFixup (modenum);
	lastmodestate = modestate;
	VID_DestroyWindow ();

	WindowRect.top = WindowRect.left = 0;
	WindowRect.right = modelist[modenum].width;
	WindowRect.bottom = modelist[modenum].height;
	DIBWidth = modelist[modenum].width;
	DIBHeight = modelist[modenum].height;
/*	WindowStyle = WS_OVERLAPPEDWINDOW | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX |
				  WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPSIBLINGS |
				  WS_CLIPCHILDREN | WS_THICKFRAME;*/

		WindowStyle = WS_OVERLAPPEDWINDOW |  WS_CAPTION | WS_SYSMENU | WS_SIZEBOX |
				  WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPSIBLINGS |
				  WS_CLIPCHILDREN | WS_THICKFRAME;

	//	WindowStyle = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
	ExWindowStyle = 0;
	AdjustWindowRectEx (&WindowRect, WindowStyle, FALSE, 0);

	// the first time we're called to set the mode, create the window we'll use
	// for the rest of the session
	if (!vid_mode_set)
	{
		hWndWinQuake = CreateWindowEx
		(
			ExWindowStyle,
			"WinQuake",
			"WinQuake",
			WindowStyle,
			0, 0,
			WindowRect.right - WindowRect.left,
			WindowRect.bottom - WindowRect.top,
			NULL,
			NULL,
			global_hInstance,
			NULL
		);

		if (!hWndWinQuake)
			Sys_Error ("Couldn't create DIB window");

		// compatibility
		mainwindow = hWndWinQuake;

		// done
		vid_mode_set = true;
	}
	else
	{
		SetWindowLong (hWndWinQuake, GWL_STYLE, WindowStyle | WS_VISIBLE);
		SetWindowLong (hWndWinQuake, GWL_EXSTYLE, ExWindowStyle);
	}

	if (!SetWindowPos (hWndWinQuake,
					   NULL,
					   0, 0,
					   WindowRect.right - WindowRect.left,
					   WindowRect.bottom - WindowRect.top,
					   SWP_NOCOPYBITS | SWP_NOZORDER |
					   SWP_HIDEWINDOW))
	{
		Sys_Error ("Couldn't resize DIB window");
	}

	if (hide_window)
		return true;

	// position and show the DIB window
	VID_CheckWindowXY ();
	VID_CheckWindowSize ();
	SetWindowPos (hWndWinQuake, NULL, (int) vid_window_x->value,
				  (int) vid_window_y->value, 0, 0,
				  SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);

	if (force_minimized)
		ShowWindow (hWndWinQuake, SW_MINIMIZE);
	else ShowWindow (hWndWinQuake, SW_SHOWDEFAULT);

	UpdateWindow (hWndWinQuake);
	modestate = MS_WINDOWED;
	vid_fulldib_on_focus_mode = 0;

	vid.numpages = 1;

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;

	vid.maxlowwidth = LOW_WIDTH;
	vid.maxlowheight = LOW_HEIGHT;


	vid.height = vid.conheight = DIBHeight;
	vid.width = vid.conwidth = DIBWidth;
	if (!yeahimconsoled){
	vid.vconheight = DIBHeight;
	vid.vconwidth = DIBWidth;}
	VID_SetAspect();
	SendMessage (hWndWinQuake, WM_SETICON, (WPARAM) TRUE, (LPARAM) hIcon);
	SendMessage (hWndWinQuake, WM_SETICON, (WPARAM) FALSE, (LPARAM) hIcon);

	return true;
}


qboolean VID_SetFullDIBMode (int modenum)
{
	HDC				hdc;
	int				lastmodestate;
	VID_DestroyWindow ();

	gdevmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	gdevmode.dmPelsWidth = modelist[modenum].width;
	gdevmode.dmPelsHeight = modelist[modenum].height;
	gdevmode.dmSize = sizeof (gdevmode);

	if (ChangeDisplaySettings (&gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		Sys_Error ("Couldn't set fullscreen DIB mode");

	lastmodestate = modestate;
	modestate = MS_FULLDIB;
	vid_fulldib_on_focus_mode = modenum;
	WindowRect.top = WindowRect.left = 0;

	WindowRect.right = modelist[modenum].width;
	WindowRect.bottom = modelist[modenum].height;

	DIBWidth = modelist[modenum].width;
	DIBHeight = modelist[modenum].height;

	WindowStyle = WS_POPUP | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	ExWindowStyle = 0;

	AdjustWindowRectEx (&WindowRect, WindowStyle, FALSE, 0);

	SetWindowLong (hWndWinQuake, GWL_STYLE, WindowStyle | WS_VISIBLE);
	SetWindowLong (hWndWinQuake, GWL_EXSTYLE, ExWindowStyle);

	if (!SetWindowPos (hWndWinQuake,
					   NULL,
					   0, 0,
					   WindowRect.right - WindowRect.left,
					   WindowRect.bottom - WindowRect.top,
					   SWP_NOCOPYBITS | SWP_NOZORDER))
	{
		Sys_Error ("Couldn't resize DIB window");
	}

	// position and show the DIB window
	SetWindowPos (hWndWinQuake, HWND_TOPMOST, 0, 0, 0, 0,
				  SWP_NOSIZE | SWP_SHOWWINDOW | SWP_DRAWFRAME);
	ShowWindow (hWndWinQuake, SW_SHOWDEFAULT);
	UpdateWindow (hWndWinQuake);

	vid.numpages = 1;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;

	vid.maxlowwidth = LOW_WIDTH;
	vid.maxlowheight = LOW_HEIGHT;



#ifdef SCALED2D
	vid.height = vid.conheight = DIBHeight;
	vid.width = vid.conwidth = DIBWidth;
	if (!yeahimconsoled){
	vid.vconheight = DIBHeight;
	vid.vconwidth = DIBWidth;}
#else
	vid.height = vid.conheight = DIBHeight;
	vid.width = vid.conwidth = DIBWidth;
#endif
	VID_SetAspect();
	// needed because we're not getting WM_MOVE messages fullscreen on NT
	//window_x = 0;
	//window_y = 0;

	return true;
}


void VID_RestoreOldMode (int original_mode)
{
	static qboolean	inerror = false;

	if (inerror)
		return;

	in_mode_set = false;
	inerror = true;
	// make sure mode set happens (video mode changes)
	vid_modenum = original_mode - 1;

	if (!VID_SetMode (original_mode, vid_curpal))
	{
		vid_modenum = MODE_WINDOWED - 1;

		if (!VID_SetMode (windowed_default, vid_curpal))
			Sys_Error ("Can't set any video mode");
	}

	inerror = false;
}


void VID_SetDefaultMode (void)
{
	if (vid_initialized)
		VID_SetMode (0, vid_curpal);

	IN_DeactivateMouse ();
}


int VID_SetMode (int modenum, unsigned char *palette)
{
	int				original_mode, temp, dummy;
	qboolean		stat;
	MSG				msg;
	HDC				hdc;
	//thatmode = modenum;
	if (modenum == -666 || modenum == -667 && vid_windowed_mode->value)
	{
		if (window_width > 320 && window_height > 200)
		{
		modelist[6].type = MS_WINDOWED;
		modelist[6].width = window_width;
		modelist[6].height = window_height;
		modelist[6].modenum = MODE_WINDOWED + 6;
		modelist[6].fullscreen = 0;
		modelist[5].type = MS_WINDOWED;
		modelist[5].width = window_width;
		modelist[5].height = window_height;
		modelist[5].modenum = MODE_WINDOWED + 5;
		modelist[5].fullscreen = 0;
if (modenum == -667)
		modenum = MODE_WINDOWED + 5;	// we really picked 5 but odn't tell anyone!!!
else
		modenum = MODE_WINDOWED + 6;	// we really picked 5 but odn't tell anyone!!!
		windreshack *= -1;
		}
	}

	while ((modenum >= nummodes) || (modenum < 0))
	{
		if (vid_modenum == NO_MODE)
		{
			if (modenum == vid_default)
			{
				modenum = windowed_default;
			}
			else
			{
				modenum = vid_default;
			}

			Cvar_SetValue (vid_mode, (float) modenum);
		}
		else
		{
			Cvar_SetValue (vid_mode, (float) vid_modenum);
			return 0;
		}
	}

	if (!force_mode_set && (modenum == vid_modenum))
		return true;

	// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	if (!imsizingawindowactually);
	scr_disabled_for_loading = true;
	in_mode_set = true;
	CDAudio_Pause ();
	//S_ClearBuffer ();

	if (vid_modenum == NO_MODE)
		original_mode = windowed_default;
	else
		original_mode = vid_modenum;

	// Set either the fullscreen or windowed mode
	if (modelist[modenum].type == MS_WINDOWED || vid_windowed_mode->value)
	{
		if (_windowed_mouse->value)
		{
			stat = VID_SetWindowedMode (modenum);
			if (!cl.paused){
			IN_ActivateMouse ();
			IN_HideMouse ();}
		}
		else
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
			stat = VID_SetWindowedMode (modenum);
		}
	}
	else
	{
		stat = VID_SetFullDIBMode (modenum);
		if (!cl.paused){
		IN_ActivateMouse ();
		IN_HideMouse ();
		}
	}

	// shutdown any old driver that was active
	VID_UnloadAllDrivers ();

	// because we have set the background brush for the window to NULL (to avoid flickering when re-sizing the window on the desktop),
	// we clear the window to black when created, otherwise it will be empty while Quake starts up.
	// this also prevents a screen flash to while when switching drivers.  it still flashes, but at least it's black now
	hdc = GetDC (hWndWinQuake);
	if (!imsizingawindowactually);
//	PatBlt (hdc, 0, 0, WindowRect.right, WindowRect.bottom, BLACKNESS);

	StretchBlt (hdc, 0, 0, WindowRect.right, WindowRect.bottom, hdc, 0, 0, WindowRect.right, WindowRect.bottom, BLACKNESS);

	ReleaseDC (hWndWinQuake, hdc);

	// create the new driver
	vid_usingddraw = false;

	// attempt to create a direct draw driver
	
	if (vid_ddraw->value){
	
		VID_CreateDDrawDriver (DIBWidth, DIBHeight, palette, &vid.buffer, &vid.rowbytes);
		
	}

	// create a gdi driver if directdraw failed or if we preferred not to use it
	if (!vid_usingddraw)
	{
		// because directdraw may have been partially created we must shut it down again first
		VID_UnloadAllDrivers ();

		// now create the gdi driver
		
		VID_CreateGDIDriver (DIBWidth, DIBHeight, palette, &vid.buffer, &vid.rowbytes);
		reflectavailable = 0;	// leilei - :(
	}
	


	// if ddraw failed to come up we disable the cvar too
	if (vid_ddraw->value && !vid_usingddraw) Cvar_Set (vid_ddraw, "0");


	
#ifdef EXPREND
	vid.shadowbuffer = (unsigned char *) malloc (vid.width * vid.height);
	memset (vid.shadowbuffer, 0xff, vid.width * vid.height);		// buffer for shadows
#endif
	// set the rest of the buffers we need (why not just use one single buffer instead of all this crap? oh well, it's Quake...)
	vid.direct = (unsigned char *) vid.buffer;
	vid.conbuffer = vid.buffer;

	// more crap for the console
	vid.conrowbytes = vid.rowbytes;

	window_width = vid.width;
	window_height = vid.height;


	VID_UpdateWindowStatus ();
	CDAudio_Resume ();
	scr_disabled_for_loading = temp;

	if (!stat)
	{
		VID_RestoreOldMode (original_mode);
		return false;
	}

	if (hide_window)
		return true;

	// now we try to make sure we get the focus on the mode switch, because
	// sometimes in some systems we don't.  We grab the foreground, then
	// finish setting up, pump all our messages, and sleep for a little while
	// to let messages finish bouncing around the system, then we put
	// ourselves at the top of the z order, then grab the foreground again,
	// Who knows if it helps, but it probably doesn't hurt
	if (!force_minimized)
		SetForegroundWindow (hWndWinQuake);

	hdc = GetDC (NULL);

	if (GetDeviceCaps (hdc, RASTERCAPS) & RC_PALETTE)
		vid_palettized = true;
	else
		vid_palettized = false;

	VID_SetPalette (palette);
	ReleaseDC (NULL, hdc);
	vid_modenum = modenum;
	Cvar_SetValue (vid_mode, (float) vid_modenum);

	if (!VID_AllocBuffers (vid.width, vid.height))
	{
		// couldn't get memory for this mode; try to fall back to previous mode
		VID_RestoreOldMode (original_mode);
		return false;
	}

	D_InitCaches (vid_surfcache, vid_surfcachesize);

	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
	if (imsizingawindowactually)
	Sleep (0); //very short 'sleep' for windowed mode
	else{
	Sleep (100);
	}
	imsizingawindowactually = 0;
	if (!force_minimized)
	{
		SetWindowPos (hWndWinQuake, HWND_TOP, 0, 0, 0, 0,
			 		  SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
					  SWP_NOCOPYBITS);

		SetForegroundWindow (hWndWinQuake);
	}

	// fix the leftover Alt from any Alt-Tab or the like that switched us away
	ClearAllStates ();

	if (!msg_suppress_1)
		Con_SafePrintf ("%s\n", VID_GetModeDescription (vid_modenum));

	VID_SetPalette (palette);
	
	in_mode_set = false;
	
	vid.recalc_refdef = 1;
	VID_SetAspect();	

	SCR_StretchInit();	
	SCR_StretchRefresh();	
	SCR_CvarCheck();
	iamnotsizingthewindow = 0;
	modenum2 = modenum;
	
	return true;
}


void VID_SetPalette (unsigned char *palette)
{
	int i;
	unsigned char *pal = palette;

	if (!Minimized)
	{
		if (vid_usingddraw)
		{
			// incoming palette is 3 component
			for (i = 0; i < 256; i++, pal += 3)
			{
				PALETTEENTRY *p = (PALETTEENTRY *) &ddpal[i];

				p->peRed = gammatable[pal[2]];
				p->peGreen = gammatable[pal[1]];
				p->peBlue = gammatable[pal[0]];
				p->peFlags = 255;
			}
		}
		else
		{
			HDC			hdc;
			RGBQUAD		colors[256];

			if (hdcDIBSection)
			{
				// incoming palette is 3 component
				for (i = 0; i < 256; i++, pal += 3)
				{
					PALETTEENTRY *p = (PALETTEENTRY *) &ddpal[i];

					colors[i].rgbRed   = gammatable[pal[0]];
					colors[i].rgbGreen = gammatable[pal[1]];
					colors[i].rgbBlue  = gammatable[pal[2]];
					colors[i].rgbReserved = 0;

					p->peRed = gammatable[pal[2]];
					p->peGreen = gammatable[pal[1]];
					p->peBlue = gammatable[pal[0]];
					p->peFlags = 255;
				}

				colors[0].rgbRed = 0;
				colors[0].rgbGreen = 0;
				colors[0].rgbBlue = 0;
				colors[255].rgbRed = 0xff;
				colors[255].rgbGreen = 0xff;
				colors[255].rgbBlue = 0xff;

				if (SetDIBColorTable (hdcDIBSection, 0, 256, colors) == 0)
				{
					Con_SafePrintf ("DIB_SetPalette() - SetDIBColorTable failed\n");
				}
			}
		}
	}

	memcpy (vid_curpal, palette, sizeof (vid_curpal));
}


void VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette (palette);
}

void VID_Init_Cvars (void)
{
	vid_ddraw = Cvar_Get ("vid_ddraw", "1", CVAR_ORIGINAL);
	vid_mode = Cvar_Get ("vid_mode", "0", CVAR_ORIGINAL);
	vid_wait = Cvar_Get ("vid_wait", "0", CVAR_ORIGINAL);
	vid_nopageflip = Cvar_Get ("vid_nopageflip", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	_vid_wait_override = Cvar_Get ("_vid_wait_override", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	_vid_default_mode = Cvar_Get ("_vid_default_mode", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	_vid_default_mode_win = Cvar_Get ("_vid_default_mode_win", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_config_x = Cvar_Get ("vid_config_x", "800", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_config_y = Cvar_Get ("vid_config_y", "600", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_stretch_by_2 = Cvar_Get ("vid_stretch_by_2", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	_windowed_mouse = Cvar_Get ("_windowed_mouse", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_fullscreen_mode = Cvar_Get ("vid_fullscreen_mode", "3", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_windowed_mode = Cvar_Get ("vid_windowed_mode", "1", CVAR_ARCHIVE|CVAR_ORIGINAL);
	block_switch = Cvar_Get ("block_switch", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_window_x = Cvar_Get ("vid_window_x", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_window_y = Cvar_Get ("vid_window_y", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	vid_window_width = Cvar_Get ("vid_window_width", "0", CVAR_ORIGINAL);	// leilei
	vid_window_height = Cvar_Get ("vid_window_height", "0", CVAR_ORIGINAL); // leilei
}


void VID_Init (unsigned char *palette)
{
	int		i, bestmatch, bestmatchmetric, t, dr, dg, db;
	int		basenummodes;
	byte	*ptmp;
	static qboolean firsttime = true;

	Check_Gamma ();

	if (firsttime)
	{
	VID_Init_Cvars();

		Cmd_AddCommand ("vid_testmode", VID_TestMode_f);
		Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
		Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
		Cmd_AddCommand ("vid_describemode", VID_DescribeMode_f);
		Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);
		Cmd_AddCommand ("vid_forcemode", VID_ForceMode_f);
		Cmd_AddCommand ("vid_windowed", VID_Windowed_f);
		Cmd_AddCommand ("vid_fullscreen", VID_Fullscreen_f);
		Cmd_AddCommand ("vid_minimize", VID_Minimize_f);

		VID_InitModes (global_hInstance);
		basenummodes = nummodes;
		VID_GetDisplayModes ();
	}

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;

	vid.maxlowwidth = LOW_WIDTH;
	vid.maxlowheight = LOW_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *) vid.colormap + 2048));
	vid_testingmode = 0;

#if 0
	// GDI doesn't let us remap palette index 0, so we'll remap color
	// mappings from that black to another one
	bestmatchmetric = 256 * 256 * 3;

	for (i = 1; i < 256; i++)
	{
		dr = palette[0] - palette[i*3];
		dg = palette[1] - palette[i*3+1];
		db = palette[2] - palette[i*3+2];
		t = (dr * dr) + (dg * dg) + (db * db);

		if (t < bestmatchmetric)
		{
			bestmatchmetric = t;
			bestmatch = i;

			if (t == 0)
				break;
		}
	}

	for (i = 0, ptmp = vid.colormap; i < (1 << (VID_CBITS + 8)); i++, ptmp++)
	{
		if (*ptmp == 0)
			*ptmp = bestmatch;
	}
#endif

	if (COM_CheckParm("-startwindowed"))
	{
		startwindowed = 1;
		vid_default = windowed_default;
	}

	if (hwnd_dialog)
		DestroyWindow (hwnd_dialog);

	// sound initialization has to go here, preceded by a windowed mode set,
	// so there's a window for DirectSound to work with but we're not yet
	// fullscreen so the "hardware already in use" dialog is visible if it
	// gets displayed
	// keep the window minimized until we're ready for the first real mode set
	hide_window = true;
	VID_SetMode (MODE_WINDOWED, palette);
	hide_window = false;
	if (firsttime) S_Init ();
	vid_initialized = true;

	SCR_StretchInit();

	force_mode_set = true;
	VID_SetMode (vid_default, palette);
	force_mode_set = false;
	vid_realmode = vid_modenum;
	VID_SetPalette (palette);
	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
	strcpy (badmode.modedesc, "Bad mode");

	firsttime = false;
}


void VID_Shutdown (void)
{
	HDC				hdc;
	int				dummy;

	if (vid_initialized)
	{
		if (modestate == MS_FULLDIB)
			ChangeDisplaySettings (NULL, CDS_FULLSCREEN);

		PostMessage (HWND_BROADCAST, WM_PALETTECHANGED, (WPARAM) hWndWinQuake, (LPARAM) 0);
		PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, (WPARAM) 0, (LPARAM) 0);
		AppActivate (false, false);

		VID_DestroyWindow ();

		if (hwnd_dialog) DestroyWindow (hwnd_dialog);
		if (hWndWinQuake) DestroyWindow (hWndWinQuake);

		vid_testingmode = 0;
		vid_initialized = 0;
	}
}


/*
================
FlipScreen
================
*/

extern cvar_t *temp2;
int		beepeepee;
extern byte transTable[256][256];

void FlipScreen (vrect_t *rects)
{
	int numrects = 0;
	float	fate = temp2->value;
	if (reflectpass)
			return;	// DONT DO THAT!!!

	//beepeepee = 8;
	if (beepeepee == 8) // direct paletted case 
	while (rects)
	{
		if (vid_usingddraw)
		{
			int x, y;
			HRESULT hr = S_OK;
			unsigned char *src = NULL;
			unsigned int *dst = NULL;
			

			if (dd_BackBuffer)
			{
				RECT TheRect;
				RECT sRect, dRect;
				DDSURFACEDESC ddsd;

				memset (&ddsd, 0, sizeof (ddsd));
				ddsd.dwSize = sizeof (DDSURFACEDESC);

				// lock the correct subrect
				TheRect.left = rects->x;
				TheRect.right = rects->x + rects->width;
				TheRect.top = rects->y;
				TheRect.bottom = rects->y + rects->height;

				if ((hr = IDirectDrawSurface_Lock (dd_BackBuffer, &TheRect, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR, NULL)) == DDERR_WASSTILLDRAWING) return;

				src = (unsigned char *) vidbuf + rects->y * vid.rowbytes + rects->x;
				dst = (unsigned int *) ddsd.lpSurface;
#ifdef VOODOO
				
#endif


				// convert pitch to unsigned int addressable
				ddsd.lPitch >>= 2;

				// because we created a 32 bit backbuffer we need to copy from the 8 bit memory buffer to it before flipping
				if (!(rects->width & 15))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 16, psrc += 16, pdst += 16)
						{
							
							pdst[0] = psrc[0];
							pdst[1] = psrc[1];
							pdst[2] = psrc[2];
							pdst[3] = psrc[3];

							pdst[4] = psrc[4];
							pdst[5] = psrc[5];
							pdst[6] = psrc[6];
							pdst[7] = psrc[7];

							pdst[8] = psrc[8];
							pdst[9] = psrc[9];
							pdst[10] = psrc[10];
							pdst[11] = psrc[11];

							pdst[12] = psrc[12];
							pdst[13] = psrc[13];
							pdst[14] = psrc[14];
							pdst[15] = psrc[15];
						}
					}
				}
				else if (!(rects->width % 10))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 10, psrc += 10, pdst += 10)
						{
							pdst[0] = psrc[0];
							pdst[1] = psrc[1];
							pdst[2] = psrc[2];
							pdst[3] = psrc[3];
							pdst[4] = psrc[4];
							pdst[5] = psrc[5];
							pdst[6] = psrc[6];
							pdst[7] = psrc[7];
							pdst[8] = psrc[8];
							pdst[9] = psrc[9];
						}
					}
				}
				else if (!(rects->width & 7))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 8, psrc += 8, pdst += 8)
						{
							pdst[0] = psrc[0];
							pdst[1] = psrc[1];
							pdst[2] = psrc[2];
							pdst[3] = psrc[3];

							pdst[4] = psrc[4];
							pdst[5] = psrc[5];
							pdst[6] = psrc[6];
							pdst[7] = psrc[7];
						}
					}
				}
				else if (!(rects->width % 5))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 5, psrc += 5, pdst += 5)
						{
							pdst[0] = psrc[0];
							pdst[1] = psrc[1];
							pdst[2] = psrc[2];
							pdst[3] = psrc[3];
							pdst[4] = psrc[4];
						}
					}
				}
				else if (!(rects->width & 3))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 4, psrc += 4, pdst += 4)
						{
							pdst[0] = psrc[0];
							pdst[1] = psrc[1];
							pdst[2] = psrc[2];
							pdst[3] = psrc[3];
						}
					}
				}
				else
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						for (x = 0; x < rects->width; x++)
						{
							dst[x] = src[x];
						}
					}
				}

				IDirectDrawSurface_Unlock (dd_BackBuffer, NULL);

				// correctly offset source
				sRect.left = SrcRect.left + rects->x;
				sRect.right = SrcRect.left + rects->x + rects->width;
				sRect.top = SrcRect.top + rects->y;
				sRect.bottom = SrcRect.top + rects->y + rects->height;

				// correctly offset dest
				dRect.left = DstRect.left + rects->x;
				dRect.right = DstRect.left + rects->x + rects->width;
				dRect.top = DstRect.top + rects->y;
				dRect.bottom = DstRect.top + rects->y + rects->height;

				// copy to front buffer
				IDirectDrawSurface_Blt (dd_FrontBuffer, &dRect, dd_BackBuffer, &sRect, 0, NULL);
			}
		}
		else if (hdcDIBSection)
		{
			BitBlt
			(
				hdcGDI,
				rects->x, rects->y,
				rects->x + rects->width,
				rects->y + rects->height,
				hdcDIBSection,
				rects->x, rects->y,
				SRCCOPY
			);
		}

		numrects++;
		rects = rects->pnext;
	}
	else	// 8-to-32 lookup paletted case
		{
	while (rects)
	{
		if (vid_usingddraw)
		{
			int x, y;
			HRESULT hr = S_OK;
			unsigned char *src = NULL;
			unsigned int *dst = NULL;
			


			if (dd_BackBuffer)
			{
				RECT TheRect;
				RECT sRect, dRect;
				DDSURFACEDESC ddsd;

				memset (&ddsd, 0, sizeof (ddsd));
				ddsd.dwSize = sizeof (DDSURFACEDESC);

				// lock the correct subrect
				TheRect.left = rects->x;
				TheRect.right = rects->x + rects->width;
				TheRect.top = rects->y;
				TheRect.bottom = rects->y + rects->height;

				if ((hr = IDirectDrawSurface_Lock (dd_BackBuffer, &TheRect, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR, NULL)) == DDERR_WASSTILLDRAWING) return;

				src = (unsigned char *) vidbuf + rects->y * vid.rowbytes + rects->x;
				dst = (unsigned int *) ddsd.lpSurface;

				// convert pitch to unsigned int addressable
				ddsd.lPitch >>= 2;

				// because we created a 32 bit backbuffer we need to copy from the 8 bit memory buffer to it before flipping
				if (!(rects->width & 15))
				{

					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 16, psrc += 16, pdst += 16)
						{
#ifdef THIRTYTWOBITHACK
							pdst[0] = psrc[0];
							pdst[1] = psrc[1];
							pdst[2] = psrc[2];
							pdst[3] = psrc[3];
							pdst[4] = psrc[4];
							pdst[5] = psrc[5];
							pdst[6] = psrc[6];
							pdst[7] = psrc[7];
							pdst[8] = psrc[8];
							pdst[9] = psrc[9];
							pdst[10] = psrc[10];
							pdst[11] = psrc[11];
							pdst[12] = psrc[12];
							pdst[13] = psrc[13];
							pdst[14] = psrc[14];
							pdst[15] = psrc[15];
#else 
							
							pdst[0] = ddpal[psrc[0]];
							pdst[1] = ddpal[psrc[1]];
							pdst[2] = ddpal[psrc[2]];
							pdst[3] = ddpal[psrc[3]];

							pdst[4] = ddpal[psrc[4]];
							pdst[5] = ddpal[psrc[5]];
							pdst[6] = ddpal[psrc[6]];
							pdst[7] = ddpal[psrc[7]];

							pdst[8] = ddpal[psrc[8]];
							pdst[9] = ddpal[psrc[9]];
							pdst[10] = ddpal[psrc[10]];
							pdst[11] = ddpal[psrc[11]];

							pdst[12] = ddpal[psrc[12]];
							pdst[13] = ddpal[psrc[13]];
							pdst[14] = ddpal[psrc[14]];
							pdst[15] = ddpal[psrc[15]];
#endif							
						}
					}
					
#ifdef VOODOO
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;
						vec3_t		colar;

						for (x = 0; x < rects->width; x += 16, psrc += 16, pdst += 16)
						{
							int fat;

									//pdst[0] = ddpal[psrc[0]];
								for (fat =0; fat < 16; fat++){
								
									if (fat < 13){
									psrc[fat] = transTable[transTable[psrc[fat+3]][psrc[fat+1]]][psrc[fat]];
									psrc[fat+1] = transTable[psrc[fat+3]][psrc[fat+1]];
									psrc[fat+2] = transTable[psrc[fat+2]][psrc[fat+3]];
									psrc[fat+3] = transTable[psrc[fat+3]][transTable[psrc[fat+1]][psrc[fat+2]]];

									}
										pdst[fat] = ddpal[psrc[fat]];
								//	if (fat < 14)
								//	psrc[fat+2] = transTable[psrc[fat]][psrc[fat+2]];
								//	if (fat < 13)
								//	psrc[fat+3] = transTable[psrc[fat]][psrc[fat+3]];
					

								//	pdst[fat+1] = pdst[fat];

								//	pdst[fat+1] = pdst[fat];
								//	pdst[fat+2] = pdst[fat];
								//	pdst[fat+3] = pdst[fat];
				

								}

  
						/*	pdst[0] = ddpal[psrc[0]];
							pdst[1] = ddpal[psrc[0]];
							pdst[2] = ddpal[psrc[2]];
							pdst[3] = ddpal[psrc[2]];

							pdst[4] = ddpal[psrc[4]];
							pdst[5] = ddpal[psrc[5]];
							pdst[6] = ddpal[psrc[6]];
							pdst[7] = ddpal[psrc[7]];

							pdst[8] = ddpal[psrc[8]];
							pdst[9] = ddpal[psrc[9]];
							pdst[10] = ddpal[psrc[10]];
							pdst[11] = ddpal[psrc[11]];

							pdst[12] = ddpal[psrc[12]];
							pdst[13] = ddpal[psrc[13]];
							pdst[14] = ddpal[psrc[14]];
							pdst[15] = ddpal[psrc[15]];
							*/
						}
					}
#endif

				}
				else if (!(rects->width % 10))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 10, psrc += 10, pdst += 10)
						{
							pdst[0] = ddpal[psrc[0]];
							pdst[1] = ddpal[psrc[1]];
							pdst[2] = ddpal[psrc[2]];
							pdst[3] = ddpal[psrc[3]];
							pdst[4] = ddpal[psrc[4]];

							pdst[5] = ddpal[psrc[5]];
							pdst[6] = ddpal[psrc[6]];
							pdst[7] = ddpal[psrc[7]];
							pdst[8] = ddpal[psrc[8]];
							pdst[9] = ddpal[psrc[9]];
						}
					}
				}
				else if (!(rects->width & 7))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 8, psrc += 8, pdst += 8)
						{
							pdst[0] = ddpal[psrc[0]];
							pdst[1] = ddpal[psrc[1]];
							pdst[2] = ddpal[psrc[2]];
							pdst[3] = ddpal[psrc[3]];

							pdst[4] = ddpal[psrc[4]];
							pdst[5] = ddpal[psrc[5]];
							pdst[6] = ddpal[psrc[6]];
							pdst[7] = ddpal[psrc[7]];
						}
					}
				}
				else if (!(rects->width % 5))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 5, psrc += 5, pdst += 5)
						{
							pdst[0] = ddpal[psrc[0]];
							pdst[1] = ddpal[psrc[1]];
							pdst[2] = ddpal[psrc[2]];
							pdst[3] = ddpal[psrc[3]];
							pdst[4] = ddpal[psrc[4]];
						}
					}
				}
				else if (!(rects->width & 3))
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						byte *psrc = src;
						unsigned int *pdst = dst;

						for (x = 0; x < rects->width; x += 4, psrc += 4, pdst += 4)
						{
							pdst[0] = ddpal[psrc[0]];
							pdst[1] = ddpal[psrc[1]];
							pdst[2] = ddpal[psrc[2]];
							pdst[3] = ddpal[psrc[3]];
						}
					}
				}
				else
				{
					for (y = 0; y < rects->height; y++, src += vid.rowbytes, dst += ddsd.lPitch)
					{
						for (x = 0; x < rects->width; x++)
						{
							dst[x] = ddpal[src[x]];
						}
					}
				}

				IDirectDrawSurface_Unlock (dd_BackBuffer, NULL);

				// correctly offset source
				sRect.left = SrcRect.left + rects->x;
				sRect.right = SrcRect.left + rects->x + rects->width;
				sRect.top = SrcRect.top + rects->y;
				sRect.bottom = SrcRect.top + rects->y + rects->height;

				// correctly offset dest
				dRect.left = DstRect.left + rects->x;
				dRect.right = DstRect.left + rects->x + rects->width;
				dRect.top = DstRect.top + rects->y;
				dRect.bottom = DstRect.top + rects->y + rects->height;

				// copy to front buffer
				IDirectDrawSurface_Blt (dd_FrontBuffer, &dRect, dd_BackBuffer, &sRect, 0, NULL);
			}
		}
		else if (hdcDIBSection)
		{
			BitBlt
			(
				hdcGDI,
				rects->x, rects->y,
				rects->x + rects->width,
				rects->y + rects->height,
				hdcDIBSection,
				rects->x, rects->y,
				SRCCOPY
			);
		}

		numrects++;
		rects = rects->pnext;
	}

	}

}


void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	int		i, j, reps, repshift;
	vrect_t	rect;

	if (!vid_initialized)
		return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	if (!vid.direct) return;

	for (i = 0; i < (height << repshift); i += reps)
	{
		for (j = 0; j < reps; j++)
		{
			memcpy (&backingbuf[(i + j) * 24],
					vid.direct + x + ((y << repshift) + i + j) * vid.rowbytes,
					width);

			memcpy (vid.direct + x + ((y << repshift) + i + j) * vid.rowbytes,
					&pbitmap[(i >> repshift) * width],
					width);
		}
	}

	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height << repshift;
	rect.pnext = NULL;

	FlipScreen (&rect);
}


void D_EndDirectRect (int x, int y, int width, int height)
{
	int		i, j, reps, repshift;
	vrect_t	rect;

	if (!vid_initialized)
		return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	if (!vid.direct) return;

	for (i = 0; i < (height << repshift); i += reps)
	{
		for (j = 0; j < reps; j++)
		{
			memcpy (vid.direct + x + ((y << repshift) + i + j) * vid.rowbytes,
					&backingbuf[(i + j) * 24],
					width);
		}
	}

	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height << repshift;
	rect.pnext = NULL;

	FlipScreen (&rect);
}


void VID_Update (vrect_t *rects)
{
	vrect_t	rect;
	RECT	trect;

	if (!vid_palettized && palette_changed)
	{
		palette_changed = false;
		rect.x = 0;
		rect.y = 0;
		rect.width = vid.width;
		rect.height = vid.height;
		rect.pnext = NULL;
		rects = &rect;
	}

	if (firstupdate)
	{
		if (modestate == MS_WINDOWED)
		{
			GetWindowRect (hWndWinQuake, &trect);

			if ((trect.left != (int) vid_window_x->value) ||
					(trect.top  != (int) vid_window_y->value))
			{
				if (COM_CheckParm ("-resetwinpos"))
				{
					Cvar_SetValue (vid_window_x, 0.0);
					Cvar_SetValue (vid_window_y, 0.0);
				}

				VID_CheckWindowXY ();
				
				SetWindowPos (hWndWinQuake, NULL, (int) vid_window_x->value,
							  (int) vid_window_y->value, 0, 0,
							   SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
				VID_CheckWindowSize ();
//							  SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
				
			}
		}

		if ((_vid_default_mode_win->value != vid_default) &&
				(!startwindowed || (_vid_default_mode_win->value < MODE_FULLSCREEN_DEFAULT)))
		{
			firstupdate = 0;

			if (COM_CheckParm ("-resetwinpos"))
			{
				Cvar_SetValue (vid_window_x, 0.0);
				Cvar_SetValue (vid_window_y, 0.0);
			}

			if ((_vid_default_mode_win->value < 0) ||
					(_vid_default_mode_win->value >= nummodes))
			{
				Cvar_SetValue (_vid_default_mode_win, (float)windowed_default);
			}

			Cvar_SetValue (vid_mode, _vid_default_mode_win->value);
		}
	}


	// We've drawn the frame; copy it to the screen
	FlipScreen (rects);
	
	// check for a driver change
	if ((vid_ddraw->value && !vid_usingddraw) || (!vid_ddraw->value && vid_usingddraw))
	{
		// reset the mode
		force_mode_set = true;
		iamnotsizingthewindow = 1;
		VID_SetMode ((int) vid_mode->value, vid_curpal);
		force_mode_set = false;

		// store back
		if (vid_usingddraw)
			Con_SafePrintf ("loaded DirectDraw driver\n");
		else Con_SafePrintf ("loaded GDI driver\n");
		iamnotsizingthewindow = 0;
	}

	if (vid_testingmode)
	{
		if (realtime >= vid_testendtime)
		{
			iamnotsizingthewindow = 1;
			VID_SetMode (vid_realmode, vid_curpal);
			vid_testingmode = 0;
			iamnotsizingthewindow = 0;
		}
	}
	else
	{
		if ((int) vid_mode->value != vid_realmode)
		{
			iamnotsizingthewindow = 1;
			VID_SetMode ((int) vid_mode->value, vid_curpal);
			Cvar_SetValue (vid_mode, (float) vid_modenum);
			// so if mode set fails, we don't keep on
			//  trying to set that mode
			vid_realmode = vid_modenum;
			iamnotsizingthewindow = 0;
		}
	}

	// handle the mouse state when windowed if that's changed
	if (modestate == MS_WINDOWED)
	{
		if ((int) _windowed_mouse->value != windowed_mouse)
		{
			if (_windowed_mouse->value)
			{
				IN_ActivateMouse ();
				IN_HideMouse ();
			}
			else
			{
				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}

			windowed_mouse = (int) _windowed_mouse->value;
		}
	}
}


//==========================================================================

byte        scantokey[128] =
{
	//  0           1       2       3       4       5       6       7
	//  8           9       A       B       C       D       E       F
	0 ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13,    K_CTRL, 'a',  's',     // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	'\'',    '`',    K_SHIFT, '\\',  'z',    'x',    'c',    'v',     // 2
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT, '*',
	K_ALT, ' ',   0 ,    K_F1, K_F2, K_F3, K_F4, K_F5,  // 3
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0 , K_HOME,
	K_UPARROW, K_PGUP, '-', K_LEFTARROW, '5', K_RIGHTARROW, '+', K_END, //4
	K_DOWNARROW, K_PGDN, K_INS, K_DEL, 0, 0,             0,              K_F11,
	K_F12, 0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0,       // 5
	0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0,
	0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0,        // 6
	0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0,
	0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0         // 7
};

/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int MapKey (int key)
{
	key = (key >> 16) & 255;

	if (key > 127)
		return 0;

	return scantokey[key];
}

void AppActivate (BOOL fActive, BOOL minimize)
/****************************************************************************
*
* Function:     AppActivate
* Parameters:   fActive - True if app is activating
*
* Description:  If the application is activating, then swap the system
*               into SYSPAL_NOSTATIC mode so that our palettes will display
*               correctly.
*
****************************************************************************/
{
	HDC			hdc;
	int			i, t;
	static BOOL	sound_active;
	ActiveApp = fActive;

	// messy, but it seems to work
	if (vid_fulldib_on_focus_mode)
	{
		Minimized = minimize;

		if (Minimized)
			ActiveApp = false;
	}

	if (vid_initialized)
	{
		// yield the palette if we're losing the focus
		hdc = GetDC (NULL);

		if (!Minimized)
			VID_SetPalette (vid_curpal);

		scr_fullupdate = 0;
		ReleaseDC (NULL, hdc);
	}

	// enable/disable sound on focus gain/loss
	if (!ActiveApp && sound_active)
	{
		S_BlockSound ();
		S_ClearBuffer ();
		sound_active = false;
	}
	else if (ActiveApp && !sound_active)
	{
		S_UnblockSound ();
		S_ClearBuffer ();
		sound_active = true;
	}

	// minimize/restore fulldib windows/mouse-capture normal windows on demand
	if (!in_mode_set)
	{
		if (ActiveApp)
		{
			if (vid_fulldib_on_focus_mode)
			{
				if (vid_initialized)
				{
					msg_suppress_1 = true;	// don't want to see normal mode set message
					VID_SetMode (vid_fulldib_on_focus_mode, vid_curpal);
					msg_suppress_1 = false;
					t = in_mode_set;
					in_mode_set = true;
					AppActivate (true, false);
					in_mode_set = t;
				}

				IN_ActivateMouse ();
				IN_HideMouse ();
			}
			else if ((modestate == MS_WINDOWED) && _windowed_mouse->value)
			{
				IN_ActivateMouse ();
				IN_HideMouse ();
			}
		}

		if (!ActiveApp)
		{
			if (modestate == MS_FULLDIB)
			{
				if (vid_initialized)
				{
					force_minimized = true;
					i = vid_fulldib_on_focus_mode;
					msg_suppress_1 = true;	// don't want to see normal mode set message
					VID_SetMode (windowed_default, vid_curpal);
					msg_suppress_1 = false;
					vid_fulldib_on_focus_mode = i;
					force_minimized = false;
					// we never seem to get WM_ACTIVATE inactive from this mode set, so we'll
					// do it manually
					t = in_mode_set;
					in_mode_set = true;
					AppActivate (false, true);
					in_mode_set = t;
				}

				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}
			else if ((modestate == MS_WINDOWED) && _windowed_mouse->value)
			{
				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}
		}
	}
}


/*
================
VID_HandlePause
================
*/
void VID_HandlePause (qboolean pause)
{
	if ((modestate == MS_WINDOWED) && _windowed_mouse->value)
	{
		if (pause)
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
		}
		else
		{
			IN_ActivateMouse ();
			IN_HideMouse ();
		}
	}
}



/*
===================================================================

MAIN WINDOW

===================================================================
*/

LONG CDAudio_MessageHandler (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* main window procedure */
LONG WINAPI MainWndProc (
	HWND    hWnd,
	UINT    uMsg,
	WPARAM  wParam,
	LPARAM  lParam)
{
	LONG			lRet = 0;
	int				fwKeys, xPos, yPos, fActive, fMinimized, temp;
	HDC				hdc;
	PAINTSTRUCT		ps;
	static int		recursiveflag;

	switch (uMsg)
	{
	case WM_CREATE:
		break;
	case WM_SYSCOMMAND:

		// Check for maximize being hit
		switch (wParam & ~0x0F)
		{
			/*
		case SC_MAXIMIZE:

			// if minimized, bring up as a window before going fullscreen,
			// so MGL will have the right state to restore
//			if (Minimized)
//			{
//				force_mode_set = true;
//				VID_SetMode (vid_modenum, vid_curpal);
//				force_mode_set = false;
//			}
		window_x = (int) LOWORD (lParam);
		window_y = (int) HIWORD (lParam);
		VID_UpdateWindowStatus ();

		// notify the driver that the screen has moved (currently ddraw only)
		if (vid_usingddraw)
			DD_UpdateRects (dd_window_width, dd_window_height);
		
		//	return 0;
			//VID_SetMode ((int) vid_fullscreen_mode->value, vid_curpal);
			break;
			*/
	//	case SC_SIZE:

	//		break;
	
	  case SC_SCREENSAVE:
		case SC_MONITORPOWER:

			if (modestate != MS_WINDOWED)
			{
				// don't call DefWindowProc() because we don't want to start
				// the screen saver fullscreen
				break;
			}

			// fall through windowed and allow the screen saver to start
		default:

			if (!in_mode_set)
			{
				S_BlockSound ();
				S_ClearBuffer ();
			}

			lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);

			if (!in_mode_set)
			{
				S_UnblockSound ();
			}
		}

		break;
		
	case WM_MOVE:
		window_x = (int) LOWORD (lParam);
		window_y = (int) HIWORD (lParam);
		VID_UpdateWindowStatus ();

		if ((modestate == MS_WINDOWED) && !in_mode_set && !Minimized)
			VID_RememberWindowPos ();

		// notify the driver that the screen has moved (currently ddraw only)
		if (vid_usingddraw)
			DD_UpdateRects (dd_window_width, dd_window_height);

		break;
		
	case WM_SIZE:
		//if ()
		if (!iamnotsizingthewindow)
		{
		Minimized = false;
		imsizingawindowactually = 1;
		window_width = (int) LOWORD (lParam);
		window_height = (int) HIWORD (lParam);
		VID_UpdateWindowStatus ();
		if (windreshack > 1)
		VID_SetMode(-666,  vid_curpal);	
		else
		VID_SetMode(-667,  vid_curpal);	
		// notify the driver that the screen has moved (currently ddraw only)
		if (vid_usingddraw)
			DD_UpdateRects (dd_window_width * 2, dd_window_height);
		else
		{
			BitBlt
			(
				hdcGDI,
				window_x, window_y,
				window_x + window_width,
				window_y + window_height,
				hdcDIBSection,
				window_x, window_y,
				SRCCOPY
			);
		}
		if (!(wParam & SIZE_RESTORED))
		{
			if (wParam & SIZE_MINIMIZED)
				Minimized = true;
			
		}
		}
		break;
	
	case WM_SYSCHAR:
		// keep Alt-Space from happening
		break;
	case WM_ACTIVATE:
		fActive = LOWORD (wParam);
		fMinimized = (BOOL) HIWORD (wParam);
		AppActivate (!(fActive == WA_INACTIVE), fMinimized);
		// fix the leftover Alt from any Alt-Tab or the like that switched us away
		ClearAllStates ();

		if (!in_mode_set)
		{
			VID_SetPalette (vid_curpal);
		}

		break;
	case WM_PAINT:
		hdc = BeginPaint (hWnd, &ps);

		if (!in_mode_set && host_initialized)
			SCR_UpdateWholeScreen ();

		EndPaint (hWnd, &ps);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:

		if (!in_mode_set)
			Key_Event (MapKey (lParam), true);

		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:

		if (!in_mode_set)
			Key_Event (MapKey (lParam), false);

		break;
		// this is complicated because Win32 seems to pack multiple mouse events into
		// one update sometimes, so we always check all states and look for events
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:

		if (!in_mode_set)
		{
			temp = 0;

			if (wParam & MK_LBUTTON)
				temp |= 1;

			if (wParam & MK_RBUTTON)
				temp |= 2;

			if (wParam & MK_MBUTTON)
				temp |= 4;

			IN_MouseEvent (temp);
		}

		break;
		// JACK: This is the mouse wheel with the Intellimouse
		// Its delta is either positive or neg, and we generate the proper
		// Event.
	case WM_MOUSEWHEEL:

		if ((short) HIWORD (wParam) > 0)
		{
			Key_Event (K_MWHEELUP, true);
			Key_Event (K_MWHEELUP, false);
		}
		else
		{
			Key_Event (K_MWHEELDOWN, true);
			Key_Event (K_MWHEELDOWN, false);
		}

		break;
	case WM_DISPLAYCHANGE:

		if (!in_mode_set && (modestate == MS_WINDOWED) && !vid_fulldib_on_focus_mode)
		{
			force_mode_set = true;
			iamnotsizingthewindow = 1;
			VID_SetMode (vid_modenum, vid_curpal);
			force_mode_set = false;
			iamnotsizingthewindow = 0;
		}

		break;
	case WM_CLOSE:

		// this causes Close in the right-click task bar menu not to work, but right
		// now bad things happen if Close is handled in that case (garbage and a
		// crash on Win95)
		if (!in_mode_set)
		{
			if (MessageBox (hWndWinQuake, "Are you sure you want to quit?", "Confirm Exit",
							MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES)
			{
				Sys_Quit ();
			}
		}

		break;
	case MM_MCINOTIFY:
		lRet = CDAudio_MessageHandler (hWnd, uMsg, wParam, lParam);
		break;
	default:
		/* pass all unhandled messages to DefWindowProc */
		lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
		break;
	}

	/* return 0 if handled message, 1 if not */
	return lRet;
}


extern void M_Menu_Options_f (void);
extern void M_Print (int cx, int cy, char *str);
extern void M_PrintWhite (int cx, int cy, char *str);
extern void M_DrawCharacter (int cx, int line, int num);
extern void M_DrawTransPic (int x, int y, qpic_t *pic);
extern void M_DrawPic (int x, int y, qpic_t *pic);

static int	vid_line, vid_wmodes;

typedef struct
{
	int		modenum;
	char	*desc;
	int		iscur;
	int		width;
} modedesc_t;

#define MAX_COLUMN_SIZE		12		// leilei - extended
#define MODE_AREA_HEIGHT	(MAX_COLUMN_SIZE + 6)
#define MAX_MODEDESCS		(MAX_COLUMN_SIZE*3)

static modedesc_t	modedescs[MAX_MODEDESCS];

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	qpic_t		*p;
	char		*ptr;
	int			lnummodes, i, j, k, column, row, dup, dupmode;
	char		temp[100];
	vmode_t		*pv;
	modedesc_t	tmodedesc;
	if(gamemode != GAME_LASER_ARENA){
	p = Draw_CachePic ("gfx/vidmodes.lmp");
	M_DrawPic ((320 - p->width) / 2, 4, p);
	}

	for (i = 0; i < 3; i++)
	{
		ptr = VID_GetModeDescriptionMemCheck (i);
		modedescs[i].modenum = modelist[i].modenum;
		modedescs[i].desc = ptr;
		modedescs[i].iscur = 0;

		if (vid_modenum == i)
			modedescs[i].iscur = 1;
	}

	vid_wmodes = 0;
	lnummodes = VID_NumModes ();

	for (i = 4; i < lnummodes; i++)
	{
		ptr = VID_GetModeDescriptionMemCheck (i);
		pv = VID_GetModePtr (i);

		// we only have room for 15 fullscreen modes, so don't allow
		// 360-wide modes, because if there are 5 320-wide modes and
		// 5 360-wide modes, we'll run out of space
		if (ptr && ((pv->width != 360) || COM_CheckParm ("-allow360")))
		{
			dup = 0;

			for (j = 3; j < vid_wmodes; j++)
			{
				
				if (!strcmp (modedescs[j].desc, ptr))
				{
					dup = 1;
					dupmode = j;
					break;
				}
			}

			if (dup || (vid_wmodes < MAX_MODEDESCS))
			{
				if (!dup || COM_CheckParm ("-noforcevga"))
				{
					if (dup)
					{
						k = dupmode;
					}
					else
					{
						k = vid_wmodes;
					}

					modedescs[k].modenum = i;
					modedescs[k].desc = ptr;
					modedescs[k].iscur = 0;
					modedescs[k].width = pv->width;

					if (i == vid_modenum)
						modedescs[k].iscur = 1;

					if (!dup)
						vid_wmodes++;
				}
			}
		}
	}

	// sort the modes on width (to handle picking up oddball dibonly modes
	// after all the others)
	for (i = 3; i < (vid_wmodes - 1); i++)
	{
		for (j = (i + 1); j < vid_wmodes; j++)
		{
			if (modedescs[i].width > modedescs[j].width)
			{
				tmodedesc = modedescs[i];
				modedescs[i] = modedescs[j];
				modedescs[j] = tmodedesc;
			}
		}
	}

	M_Print (13 * 8, 36, "Video Modes");	// leilei - it's all going to be modes.for both.
	column = 16;
	row = 36 + 2 * 8;

	// retired window exclusive modes
/*
	for (i = 0; i < 5; i++)
	{
		if (modedescs[i].iscur)
			M_PrintWhite (column, row, modedescs[i].desc);
		else
			M_Print (column, row, modedescs[i].desc);

		column += 13 * 8;
	}
*/
	if (vid_wmodes > 0)
	{
		M_Print (10 * 8, 36 + 4 * 8, "Available Resolutions");
		column = 16;
		row = 36 + 6 * 8;

		for (i = 3; i < vid_wmodes; i++)
		{
			if (modedescs[i].iscur)
				M_PrintWhite (column, row, modedescs[i].desc);
			else
				M_Print (column, row, modedescs[i].desc);

			column += 13 * 8;

			if (((i - 3) % VID_ROW_SIZE) == (VID_ROW_SIZE - 1))
			{
				column = 16;
				row += 8;
			}
		}
	}

	// line cursor
	if (vid_testingmode)
	{
		sprintf (temp, "TESTING %s",
				 modedescs[vid_line].desc);
		M_Print (13 * 8, 36 + MODE_AREA_HEIGHT * 8 + 8 * 4, temp);
		M_Print (9 * 8, 36 + MODE_AREA_HEIGHT * 8 + 8 * 6,
				 "Please wait 5 seconds...");
	}
	else
	{
		M_Print (9 * 8, 36 + MODE_AREA_HEIGHT * 8 + 8,
				 "Press Enter to set mode");
		M_Print (6 * 8, 36 + MODE_AREA_HEIGHT * 8 + 8 * 3,
				 "T to test mode for 5 seconds");
		ptr = VID_GetModeDescription2 (vid_modenum);

		if (ptr)
		{
			sprintf (temp, "D to set default: %s", ptr);
			M_Print (2 * 8, 36 + MODE_AREA_HEIGHT * 8 + 8 * 5, temp);
		}

		ptr = VID_GetModeDescription2 ((int) _vid_default_mode_win->value);

		if (ptr)
		{
			sprintf (temp, "Current default: %s", ptr);
			M_Print (3 * 8, 36 + MODE_AREA_HEIGHT * 8 + 8 * 6, temp);
		}

		M_Print (15 * 8, 36 + MODE_AREA_HEIGHT * 8 + 8 * 8,
				 "Esc to exit");
		row = 36 + 2 * 8 + (vid_line / VID_ROW_SIZE) * 8;
		column = 8 + (vid_line % VID_ROW_SIZE) * 13 * 8;

		if (vid_line >= 3)
			row += 3 * 8;

		M_DrawCharacter (column, row, 12 + ((int) (realtime * 4) & 1));
	}
	// Color Depth (Dummy menu option)
		M_Print (2 * 8, 36 + 2 * 8, "Depth: 8bpp");
	// Windowed mode...
	if (vid_windowed_mode->value)
			M_Print (15 * 8, 36 + 2 * 8, "Windowed:Yes");
	else 
			M_Print (15 * 8, 36 + 2 * 8, "Windowed:No");
	// Aspecting (Dummy menu option)
		M_Print (28 * 8, 36 + 2 * 8, "Aspect:Auto");
	
}


/*
================
VID_MenuKey
================
*/
void VID_MenuKey (int key)
{
	if (vid_testingmode)
		return;

	switch (key)
	{
	case K_ESCAPE:
		S_LocalSound ("misc/menu1.wav");
		M_Menu_Options_f ();
		break;
	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line = ((vid_line / VID_ROW_SIZE) * VID_ROW_SIZE) +
				   ((vid_line + 2) % VID_ROW_SIZE);

		if (vid_line >= vid_wmodes)
			vid_line = vid_wmodes - 1;

		break;
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line = ((vid_line / VID_ROW_SIZE) * VID_ROW_SIZE) +
				   ((vid_line + 4) % VID_ROW_SIZE);

		if (vid_line >= vid_wmodes)
			vid_line = (vid_line / VID_ROW_SIZE) * VID_ROW_SIZE;

		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line -= VID_ROW_SIZE;
		
		if (vid_line < 0)
		{
			vid_line += ((vid_wmodes + (VID_ROW_SIZE - 1)) /
						  VID_ROW_SIZE) * VID_ROW_SIZE;

			while (vid_line >= vid_wmodes)
				vid_line -= VID_ROW_SIZE;
		}

		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line += VID_ROW_SIZE;

		if (vid_line >= vid_wmodes)
		{
			vid_line -= ((vid_wmodes + (VID_ROW_SIZE - 1)) /
						  VID_ROW_SIZE) * VID_ROW_SIZE;

			while (vid_line < 0)
				vid_line += VID_ROW_SIZE;
		}

		break;
	case K_ENTER:
		S_LocalSound ("misc/menu1.wav");

		// Bit depth setting (not implemented yet)

		// window setting
		if (vid_line == 1){
			
				if (!vid_windowed_mode->value)
					vid_windowed_mode->value = 1;
				else
					vid_windowed_mode->value = 0;
				Cvar_SetValue (vid_windowed_mode, vid_windowed_mode->value);
				//VID_SetMode (modenum2, vid_curpal);
		}

		// Aspect forcing (not implemented yet)


		// mode picking
		else if (vid_line > 2) VID_SetMode (modedescs[vid_line].modenum, vid_curpal);
		break;
	case 'T':
	case 't':
		S_LocalSound ("misc/menu1.wav");
		// have to set this before setting the mode because WM_PAINT
		// happens during the mode set and does a VID_Update, which
		// checks vid_testingmode
		vid_testingmode = 1;
		vid_testendtime = realtime + 5.0;

		if (!VID_SetMode (modedescs[vid_line].modenum, vid_curpal))
		{
			vid_testingmode = 0;
		}

		break;
	case 'D':
	case 'd':
		S_LocalSound ("misc/menu1.wav");
		firstupdate = 0;
		Cvar_SetValue (_vid_default_mode_win, (float)vid_modenum);
		break;
	default:
		break;
	}
}


/*
=================
VID_NumModes
=================
*/
int VID_NumModes (void)
{
	return nummodes;
}


/*
=================
VID_GetModePtr
=================
*/
vmode_t *VID_GetModePtr (int modenum)
{
	if ((modenum >= 0) && (modenum < nummodes))
		return &modelist[modenum];
	else
		return &badmode;
}


/*
=================
VID_CheckModedescFixup
=================
*/
void VID_CheckModedescFixup (int mode)
{
	int		x, y;

#if 0
	if (mode == MODE_SETTABLE_WINDOW)
	{
		x = (int) vid_config_x->value;
		y = (int) vid_config_y->value;
		sprintf (modelist[mode].modedesc, "%dx%d", x, y);
		modelist[mode].width = x;
		modelist[mode].height = y;
	}
#endif
}


/*
=================
VID_GetModeDescriptionMemCheck
=================
*/
char *VID_GetModeDescriptionMemCheck (int mode)
{
	char		*pinfo;
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);
	pv = VID_GetModePtr (mode);
	pinfo = pv->modedesc;

	if (VID_CheckAdequateMem (pv->width, pv->height))
	{
		return pinfo;
	}
	else
	{
		return NULL;
	}
}


/*
=================
VID_GetModeDescription
=================
*/
char *VID_GetModeDescription (int mode)
{
	char		*pinfo;
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);
	pv = VID_GetModePtr (mode);
	pinfo = pv->modedesc;
	return pinfo;
}


/*
=================
VID_GetModeDescription2

Tacks on "windowed" or "fullscreen"
=================
*/
char *VID_GetModeDescription2 (int mode)
{
	static char	pinfo[40];
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);
	pv = VID_GetModePtr (mode);

	if (modelist[mode].type == MS_FULLSCREEN)
	{
		sprintf (pinfo, "%s fullscreen", pv->modedesc);
	}
	else if (modelist[mode].type == MS_FULLDIB)
	{
		sprintf (pinfo, "%s fullscreen", pv->modedesc);
	}
	else
	{
		sprintf (pinfo, "%s windowed", pv->modedesc);
	}

	return pinfo;
}


// KJB: Added this to return the mode driver name in description for console

char *VID_GetExtModeDescription (int mode)
{
	static char	pinfo[40];
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);
	pv = VID_GetModePtr (mode);

	if (modelist[mode].type == MS_FULLDIB)
	{
		sprintf (pinfo, "%s fullscreen", pv->modedesc);
	}
	else
	{
		sprintf (pinfo, "%s windowed", pv->modedesc);
	}

	return pinfo;
}


/*
=================
VID_DescribeCurrentMode_f
=================
*/
void VID_DescribeCurrentMode_f (void)
{
	Con_Printf ("%s\n", VID_GetExtModeDescription (vid_modenum));
}


/*
=================
VID_NumModes_f
=================
*/
void VID_NumModes_f (void)
{
	if (nummodes == 1)
		Con_Printf ("%d video mode is available\n", nummodes);
	else
		Con_Printf ("%d video modes are available\n", nummodes);
}


/*
=================
VID_DescribeMode_f
=================
*/
void VID_DescribeMode_f (void)
{
	int		modenum;
	modenum = Q_atoi (Cmd_Argv (1));
	Con_Printf ("%s\n", VID_GetExtModeDescription (modenum));
}


/*
=================
VID_DescribeModes_f
=================
*/
void VID_DescribeModes_f (void)
{
	int			i, lnummodes;
	char		*pinfo;
	qboolean	na;
	vmode_t		*pv;
	na = false;
	lnummodes = VID_NumModes ();

	for (i = 0; i < lnummodes; i++)
	{
		pv = VID_GetModePtr (i);
		pinfo = VID_GetExtModeDescription (i);

		if (VID_CheckAdequateMem (pv->width, pv->height))
		{
			Con_Printf ("%2d: %s\n", i, pinfo);
		}
		else
		{
			Con_Printf ("**: %s\n", pinfo);
			na = true;
		}
	}

	if (na)
	{
		Con_Printf ("\n[**: not enough system RAM for mode]\n");
	}
}


/*
=================
VID_TestMode_f
=================
*/
void VID_TestMode_f (void)
{
	int		modenum;
	double	testduration;

	if (!vid_testingmode)
	{
		modenum = Q_atoi (Cmd_Argv (1));

		if (VID_SetMode (modenum, vid_curpal))
		{
			vid_testingmode = 1;
			testduration = Q_atof (Cmd_Argv (2));

			if (testduration == 0)
				testduration = 5.0;

			vid_testendtime = realtime + testduration;
		}
	}
}


/*
=================
VID_Windowed_f
=================
*/
void VID_Windowed_f (void)
{
	iamnotsizingthewindow = 0;
	VID_SetMode ((int) vid_windowed_mode->value, vid_curpal);
	iamnotsizingthewindow = 0;
}


/*
=================
VID_Fullscreen_f
=================
*/
void VID_Fullscreen_f (void)
{
	iamnotsizingthewindow = 1;
	VID_SetMode ((int) vid_fullscreen_mode->value, vid_curpal);
	iamnotsizingthewindow = 0;
}


/*
=================
VID_Minimize_f
=================
*/
void VID_Minimize_f (void)
{
	// we only support minimizing windows; if you're fullscreen,
	// switch to windowed first
	if (modestate == MS_WINDOWED)
		ShowWindow (hWndWinQuake, SW_MINIMIZE);
}



/*
=================
VID_ForceMode_f
=================
*/
void VID_ForceMode_f (void)
{
	int		modenum;
	double	testduration;

	if (!vid_testingmode)
	{
		modenum = Q_atoi (Cmd_Argv (1));
		force_mode_set = 1;
		VID_SetMode (modenum, vid_curpal);
		force_mode_set = 0;
	}
}


