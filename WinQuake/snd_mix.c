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
// snd_mix.c -- portable code to mix sounds for snd_dma.c

#include "quakedef.h"

#ifdef _WIN32
#include "winquake.h"
#else
#define DWORD	unsigned long
#endif

#define	PAINTBUFFER_SIZE	2048
portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
int		snd_scaletable[32][256];
int 	*snd_p, snd_linear_count, snd_vol;
short	*snd_out;
extern cvar_t	*s_underwater;
void Snd_WriteLinearBlastStereo16 (void);
int		playersnd;		// leilei - dsp effects
#if	!id386
void Snd_WriteLinearBlastStereo16 (void)
{
	int		i;
	int		val;

	for (i=0 ; i<snd_linear_count ; i+=2)
	{
		val = (snd_p[i]*snd_vol)>>8;
		if (val > 0x7fff)
			snd_out[i] = 0x7fff;
		else if (val < (short)0x8000)
			snd_out[i] = (short)0x8000;
		else
			snd_out[i] = val;

		val = (snd_p[i+1]*snd_vol)>>8;
		if (val > 0x7fff)
			snd_out[i+1] = 0x7fff;
		else if (val < (short)0x8000)
			snd_out[i+1] = (short)0x8000;
		else
			snd_out[i+1] = val;
	}
}
#endif
void SND_PaintDumb(int count);
void S_TransferStereo16 (int endtime)
{
	int		lpos;
	int		lpaintedtime;
	DWORD	*pbuf;
#ifdef _WIN32
	int		reps;
	DWORD	dwSize,dwSize2;
	DWORD	*pbuf2;
	HRESULT	hresult;
#endif

	snd_vol = volume->value*256;

	snd_p = (int *) paintbuffer;
	lpaintedtime = paintedtime;

#ifdef _WIN32
	if (pDSBuf)
	{
		reps = 0;

		while ((hresult = pDSBuf->lpVtbl->Lock(pDSBuf, 0, gSndBufSize, &pbuf, &dwSize,
									   &pbuf2, &dwSize2, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("S_TransferStereo16: DS::Lock Sound Buffer Failed\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("S_TransferStereo16: DS: couldn't restore buffer\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}
		}
	}
	else
#endif
	{
		pbuf = (DWORD *)shm->buffer;
	}

	while (lpaintedtime < endtime)
	{
	// handle recirculating buffer issues
		lpos = lpaintedtime & ((shm->samples>>1)-1);

		snd_out = (short *) pbuf + (lpos<<1);

		snd_linear_count = (shm->samples>>1) - lpos;
		if (lpaintedtime + snd_linear_count > endtime)
			snd_linear_count = endtime - lpaintedtime;

		snd_linear_count <<= 1;

	// write a linear blast of samples
		Snd_WriteLinearBlastStereo16 ();

		snd_p += snd_linear_count;
		lpaintedtime += (snd_linear_count>>1);
	}

#ifdef _WIN32
	if (pDSBuf)
		pDSBuf->lpVtbl->Unlock(pDSBuf, pbuf, dwSize, NULL, 0);
#endif
}

void S_TransferPaintBuffer(int endtime)
{
	int 	out_idx;
	int 	count;
	int 	out_mask;
	int 	*p;
	int 	step;
	int		val;
	int		snd_vol;
	DWORD	*pbuf;
#ifdef _WIN32
	int		reps;
	DWORD	dwSize,dwSize2;
	DWORD	*pbuf2;
	HRESULT	hresult;
#endif

	if (shm->samplebits == 16 && shm->channels == 2)
	{
		S_TransferStereo16 (endtime);
		return;
	}

	p = (int *) paintbuffer;
	count = (endtime - paintedtime) * shm->channels;
	out_mask = shm->samples - 1;
	out_idx = paintedtime * shm->channels & out_mask;
	step = 3 - shm->channels;
	snd_vol = volume->value*256;

#ifdef _WIN32
	if (pDSBuf)
	{
		reps = 0;

		while ((hresult = pDSBuf->lpVtbl->Lock(pDSBuf, 0, gSndBufSize, &pbuf, &dwSize,
									   &pbuf2,&dwSize2, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("S_TransferPaintBuffer: DS::Lock Sound Buffer Failed\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("S_TransferPaintBuffer: DS: couldn't restore buffer\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}
		}
	}
	else
#endif
	{
		pbuf = (DWORD *)shm->buffer;
	}

	if (shm->samplebits == 16)
	{
		short *out = (short *) pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = val;
			out_idx = (out_idx + 1) & out_mask;
		}
	}
	else if (shm->samplebits == 8)
	{
		unsigned char *out = (unsigned char *) pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = (val>>8) + 128;
			out_idx = (out_idx + 1) & out_mask;
		}
	}

#ifdef _WIN32
	if (pDSBuf) {
		DWORD dwNewpos, dwWrite;
		int il = paintedtime;
		int ir = endtime - paintedtime;

		ir += il;

		pDSBuf->lpVtbl->Unlock(pDSBuf, pbuf, dwSize, NULL, 0);

		pDSBuf->lpVtbl->GetCurrentPosition(pDSBuf, &dwNewpos, &dwWrite);

//		if ((dwNewpos >= il) && (dwNewpos <= ir))
//			Con_Printf("%d-%d p %d c\n", il, ir, dwNewpos);
	}
#endif
}

extern cvar_t		*temp1;
/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

void SND_PaintChannelFrom8 (channel_t *ch, sfxcache_t *sc, int endtime);
void SND_PaintChannelFrom16 (channel_t *ch, sfxcache_t *sc, int endtime);

void SND_PaintChannelFrom8_II (channel_t *ch, sfxcache_t *sc, int endtime);
void SND_PaintChannelFrom16_II (channel_t *ch, sfxcache_t *sc, int endtime);



void S_PaintChannels(int endtime)
{
	int 	i;
	int 	end;
	channel_t *ch;
	sfxcache_t	*sc;
	int		ltime, count;
	int poop;	
	while (paintedtime < endtime)
	{
	// if paintbuffer is smaller than DMA buffer
		end = endtime;
		if (endtime - paintedtime > PAINTBUFFER_SIZE)
			end = paintedtime + PAINTBUFFER_SIZE;

	// clear the paint buffer
		Q_memset(paintbuffer, 0, (end - paintedtime) * sizeof(portable_samplepair_t));

	// paint in the channels.
		ch = channels;
		for (i=0; i<total_channels ; i++, ch++)
		{
			
			if (!ch->sfx)
				continue;
			if (!ch->leftvol && !ch->rightvol)
				continue;
			sc = S_LoadSound (ch->sfx);
			if (!sc)
				continue;
			
	if (ch->step < 5)
			ch->step = 256; // ambient suck hack
	if (sc->loopstart > 1)
				ch->step = 256; // loop suck hack

			
			ltime = paintedtime;

			while (ltime < end)
			{	// paint up to end
				if (ch->end < end)
					count = ch->end - ltime;
				else
					count = end - ltime;

				if (count > 0)
				{

					if (sc->width == 1)
						SND_PaintChannelFrom8_II(ch, sc, count);
					else
						SND_PaintChannelFrom16_II(ch, sc, count);



					// lol?
			//		SND_PaintChannelFrom8_II(ch, sc, count + 8);
			//		SND_PaintChannelFrom8_II(ch, sc, count + 16);

#ifdef DUMB
				//		SND_PaintDumb(count);
					
					/*
	long duh_sigrenderer_generate_samples(
	DUH_SIGRENDERER *sigrenderer,
	float volume, float delta,
	long size, sample_t **samples);

					*/

#endif


					ltime += count;
				}

			// if at end of loop, restart
				if (ltime >= ch->end)
				{
					if (sc->loopstart >= 0)
					{
//						ch->pos = sc->loopstart;
//						ch->end = ltime + sc->length - ch->pos;

						// leilei - HACK!
						ch->pos =  sc->loopstart >> 8;	
						ch->end = ltime + sc->length - ch->pos;
					}
					else
					{	// channel just stopped
						ch->sfx = NULL;
						break;
					}
				}
			}

		}

	// transfer out according to DMA format
		S_TransferPaintBuffer(end);
		paintedtime = end;
	}
}

void SND_InitScaletable (void)
{
	int		i, j;

	for (i=0 ; i<32 ; i++)
		for (j=0 ; j<256 ; j++)
			snd_scaletable[i][j] = ((signed char)j) * i * 8;
}


#if	!id386

void SND_PaintChanneledFrom8 (channel_t *ch, sfxcache_t *sc, int count)
{
//	int 	data;
	int		*lscale, *rscale;
	unsigned char *sfx;
	int		i;

	if (ch->leftvol > 255)
		ch->leftvol = 255;
	if (ch->rightvol > 255)
		ch->rightvol = 255;
	lscale = snd_scaletable[ch->leftvol >> 3];
	rscale = snd_scaletable[ch->rightvol >> 3];
	if (sc->stereo)
	{
		// LordHavoc: stereo sound support, and optimizations
		sfx = (unsigned char *)sc->data + ch->pos * 2;

		for (i=0 ; i<count ; i++)
		{
			paintbuffer[i].left += lscale[*sfx++];
			paintbuffer[i].right += rscale[*sfx++];
		}
		
	}
	else
	{
	//	sfx = (unsigned char *)sc->data + ch->pos;
	    sfx = (signed char *)sc->data;  //AJA - pitch shift

		for (i=0 ; i<count ; i++)
		{
			paintbuffer[i].left += lscale[*sfx];
			paintbuffer[i].right += rscale[*sfx++];


		}
	
		

	}
	ch->pos += count;
}

#endif	// !id386

extern cvar_t *temp2;
extern cvar_t *temp3;

void
 SND_PaintChannelFrom8_II(channel_t *ch, sfxcache_t *sc, int count)
 {
     int data;
	 unsigned char dater;
     int *lscale, *rscale;
	 int lscale2, rscale2;
     unsigned char *sfx;
     int i;
	 int	verb;	// leilei - lame processing test
	 int ah;
	 float	arb;
	 float	sterp;
     if (ch->leftvol > 255)
       ch->leftvol = 255;
     if (ch->rightvol > 255)
       ch->rightvol = 255;
 //	if (ch->step < 5)
//			ch->step = 256; // ambient suck hack
     lscale = snd_scaletable[ch->leftvol >> 3];
     rscale = snd_scaletable[ch->rightvol >> 3];


	 sterp = ch->step * host_timescale->value;
	if (sc->stereo > 1)
	{
		
		// LordHavoc: stereo sound support, and optimizations


	// leilei - VERY BAD attempt at stereo sound support
    
	sfx = (unsigned char *)sc->data;
	
	sfx = (unsigned char *)sc->data;
     for (i = 0; i < count; i++)
     {
		
			data = (sfx[ch->pos >> 8]);
		dater = (sfx[ch->pos * 2 >> 8]);
	//		paintbuffer[i].left += lscale[*sfx++];
	//		paintbuffer[i].right += rscale[*sfx++];
        
		paintbuffer[i].left += lscale[dater++];
		
		paintbuffer[i].right += rscale[dater++];
		
		

	//ch->step *= host_timescale->value; // slowmo sounds hack	
      ch->pos += (sterp);
	  
      if ((ch->pos >> 8) >= sc->length) break;


	}
	}
	else
	{
    sfx = (signed char *)sc->data;  //@@@
 
     for (i = 0; i < count; i++)
     {

      //@@@
      data = sfx[ch->pos >> 8];
  //      paintbuffer[i].left += lscale[data];
//        paintbuffer[i].right += rscale[data];

 			// dumb DSP effects test test


	  // This makes it sound muffled. Might use for underwater?
		//if (playersnd == 2){
		if ((sc->flags == 2 || playersnd == 2) && s_underwater->value > 1){
	
		for (verb = 1; verb > 8; verb++)
		{

					lscale2 = lscale[data];// / verb;
					rscale2 = rscale[data];// / verb;
			paintbuffer[i].left  += lscale2;
			paintbuffer[i].right += rscale2;
		}
		}
		else
		{
        paintbuffer[i].left += lscale[data];
        paintbuffer[i].right += rscale[data];
		}
      //@@@


	/*	{
			int ehhh;
			for (ehhh=0; ehhh<count; ehhh++){
				if (ehhh > count+8)
					paintbuffer[ehhh+8].left += 0;
				else
				paintbuffer[ehhh+8].left += paintbuffer[ehhh].left * 0.2;
			}
		}*/
      ch->pos += sterp;
      if ((ch->pos >> 8) >= sc->length) break;
    }
	
	}

 } 


void SND_PaintChannelFrom16_II(channel_t *ch, sfxcache_t *sc, int count)
 {
   int data;
   int left, right;
   int leftvol, rightvol;
   signed short *sfx;
   float sterp;
   int i;
   int	verb, arb; // leilei
 
   leftvol = ch->leftvol;
   rightvol = ch->rightvol;
   	sterp = ch->step * host_timescale->value;
	if (ch->step < 5)
			ch->step = 256; // ambient suck hack

  sfx = (signed short *)sc->data;  //@@@
	
   for (i = 0; i < count; i++)
   {

    data = sfx[ch->pos >> 8];  //@@@

     left = (data * leftvol) >> 8;
     right = (data * rightvol) >> 8;
//     paintbuffer[i].left += left;
 //    paintbuffer[i].right += right;

 //	leilei - dsp "effects"

	 		if ((sc->flags == 2 || playersnd == 2) && s_underwater->value > 1){
				
		for (verb = 22; verb < 58; verb++)
		{
			arb = 1.2 / verb;
	//	    paintbuffer[i+verb].left += (left * (arb));
	//		paintbuffer[i+verb].right += (right * (arb));
			paintbuffer[i+verb].left += left * 0.05;
		  paintbuffer[i+verb].right += right * 0.04;
		}
		}
		else
		{
		  paintbuffer[i].left += left;
		  paintbuffer[i].right += right;
		}

    //@@@
    ch->pos += sterp;
    if ((ch->pos >> 8) >= sc->length) break;
  }
} 

union {
	short s16[8192];
	char s8[16384];
} suffer;

void SND_PaintDumb(int count)
{
   int data;
   int left, right;
   int leftvol, rightvol;
   signed short *sfx;
   //DUH		*sptr [2048];
   int i;
   int	verb, arb; // leilei
	
//   leftvol = ch->leftvol;
  // rightvol = ch->rightvol;

	// put the dumb thing in
//	duh_render(sr,16,0,1 ,0, 2048, sptr);
//	duh_start_sigrenderer(duh, 0, 1, 0);
//	duh_sigrenderer_generate_samples(sr, 0, 1, ltime, 0);
//	duh_render(44100,16,0,1 ,0, 16384, suffer);
//	duh_start_sigrenderer(duh, 0, 1, 0);
//	duh_sigrenderer_generate_samples(sr, 0, 1, count, 0);
	
	Con_Printf("painting dumb\n");	
	


   for (i = 0; i < count; i++)
   {

//    data = sfx[ch->pos >> 8];  //@@@

  //   left = (data * leftvol) >> 8;
  //   right = (data * rightvol) >> 8;
//
//	 paintbuffer[i].left += left;
//	  paintbuffer[i].right += right;

    //@@@
   // ch->pos += ch->step;
 //   if ((ch->pos >> 8) >= sc->length) break;
  }
} 
