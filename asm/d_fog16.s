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
//
// d_fog16.s
// x86 assembly-language horizontal 8-bpp span-drawing code, with 16-pixel
// subdivision, exclusively for use with fog.
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"
#include "d_ifacea.h"

#if	id386

//----------------------------------------------------------------------
// 8-bpp horizontal span drawing code for polygons, with no transparency and
// 16-pixel subdivision.
//
// Assumes there is at least one span in pspans, and that every span
// contains at least one pixel
//----------------------------------------------------------------------

	.data

	.text
// out-of-line, rarely-needed clamping code

LClampHigh0:
	movl	C(bbextents),%esi
	jmp		LClampReFogtry0
LClampHighOrLow0:
	jg		LClampHigh0
	xorl	%esi,%esi
	jmp		LClampReFogtry0

LClampHigh1:
	movl	C(bbextentt),%edx
	jmp		LClampReFogtry1
LClampHighOrLow1:
	jg		LClampHigh1
	xorl	%edx,%edx
	jmp		LClampReFogtry1

LClampLow2:
	movl	$4096,%ebp
	jmp		LClampReFogtry2
LClampHigh2:
	movl	C(bbextents),%ebp
	jmp		LClampReFogtry2

LClampLow3:
	movl	$4096,%ecx
	jmp		LClampReFogtry3
LClampHigh3:
	movl	C(bbextentt),%ecx
	jmp		LClampReFogtry3

LClampLow4:
	movl	$4096,%eax
	jmp		LClampReFogtry4
LClampHigh4:
	movl	C(bbextents),%eax
	jmp		LClampReFogtry4

LClampLow5:
	movl	$4096,%ebx
	jmp		LClampReFogtry5
LClampHigh5:
	movl	C(bbextentt),%ebx
	jmp		LClampReFogtry5


#define pspans	4+16

	.align 4
.globl C(D_DrawSpans16_Fog)
C(D_DrawSpans16_Fog):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

//
// set up scaled-by-16 steps, for 16-long segments; also set up cacheblock
// and span list pointers
//
// TODO: any overlap from rearranging?
	flds	C(d_sdivzstepu)
	fmuls	fp_16
	movl	C(cacheblock),%edx
	flds	C(d_tdivzstepu)
	fmuls	fp_16
	movl	pspans(%esp),%ebx	// point to the first span descriptor
	flds	C(d_zistepu)
	fmuls	fp_16
	movl	%edx,pbase			// pbase = cacheblock
	fstps	zi16stepu
	fstps	tdivz16stepu
	fstps	sdivz16stepu

LSpanLoop:
//
// set up the initial s/z, t/z, and 1/z on the FP stack, and generate the
// initial s and t values
//
// FIXME: pipeline FILD?
	fildl	espan_t_v(%ebx)
	fildl	espan_t_u(%ebx)

	fld		%st(1)			// dv | du | dv
	fmuls	C(d_sdivzstepv)	// dv*d_sdivzstepv | du | dv
	fld		%st(1)			// du | dv*d_sdivzstepv | du | dv
	fmuls	C(d_sdivzstepu)	// du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
	fld		%st(2)			// du | du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
	fmuls	C(d_tdivzstepu)	// du*d_tdivzstepu | du*d_sdivzstepu |
							//  dv*d_sdivzstepv | du | dv
	fxch	%st(1)			// du*d_sdivzstepu | du*d_tdivzstepu |
							//  dv*d_sdivzstepv | du | dv
	faddp	%st(0),%st(2)	// du*d_tdivzstepu |
							//  du*d_sdivzstepu + dv*d_sdivzstepv | du | dv
	fxch	%st(1)			// du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fld		%st(3)			// dv | du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fmuls	C(d_tdivzstepv)	// dv*d_tdivzstepv |
							//  du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fxch	%st(1)			// du*d_sdivzstepu + dv*d_sdivzstepv |
							//  dv*d_tdivzstepv | du*d_tdivzstepu | du | dv
	fadds	C(d_sdivzorigin)	// sdivz = d_sdivzorigin + dv*d_sdivzstepv +
							//  du*d_sdivzstepu; stays in %st(2) at end
	fxch	%st(4)			// dv | dv*d_tdivzstepv | du*d_tdivzstepu | du |
							//  s/z
	fmuls	C(d_zistepv)		// dv*d_zistepv | dv*d_tdivzstepv |
							//  du*d_tdivzstepu | du | s/z
	fxch	%st(1)			// dv*d_tdivzstepv |  dv*d_zistepv |
							//  du*d_tdivzstepu | du | s/z
	faddp	%st(0),%st(2)	// dv*d_zistepv |
							//  dv*d_tdivzstepv + du*d_tdivzstepu | du | s/z
	fxch	%st(2)			// du | dv*d_tdivzstepv + du*d_tdivzstepu |
							//  dv*d_zistepv | s/z
	fmuls	C(d_zistepu)		// du*d_zistepu |
							//  dv*d_tdivzstepv + du*d_tdivzstepu |
							//  dv*d_zistepv | s/z
	fxch	%st(1)			// dv*d_tdivzstepv + du*d_tdivzstepu |
							//  du*d_zistepu | dv*d_zistepv | s/z
	fadds	C(d_tdivzorigin)	// tdivz = d_tdivzorigin + dv*d_tdivzstepv +
							//  du*d_tdivzstepu; stays in %st(1) at end
	fxch	%st(2)			// dv*d_zistepv | du*d_zistepu | t/z | s/z
	faddp	%st(0),%st(1)	// dv*d_zistepv + du*d_zistepu | t/z | s/z

	flds	fp_64k			// fp_64k | dv*d_zistepv + du*d_zistepu | t/z | s/z
	fxch	%st(1)			// dv*d_zistepv + du*d_zistepu | fp_64k | t/z | s/z
	fadds	C(d_ziorigin)		// zi = d_ziorigin + dv*d_zistepv +
							//  du*d_zistepu; stays in %st(0) at end
							// 1/z | fp_64k | t/z | s/z
//
// calculate and clamp s & t
//
	fdivr	%st(0),%st(1)	// 1/z | z*64k | t/z | s/z

//
// point %edi to the first pixel in the span
//
	movl	C(d_viewbuffer),%ecx
	movl	espan_t_v(%ebx),%eax
	movl	%ebx,pspantemp	// preserve spans pointer

	
	movl	C(tadjust),%edx
	movl	C(sadjust),%esi
	movl	C(d_scantable)(,%eax,4),%edi	// v * screenwidth
	addl	%ecx,%edi
	movl	espan_t_u(%ebx),%ecx




	addl	%ecx,%edi				// pdest = &pdestspan[scans->u];
	movl	espan_t_count(%ebx),%ecx

//
// now start the FDIV for the end of the span
//
	cmpl	$16,%ecx
	ja		LSetupNotLast1

	decl	%ecx
	jz		LCleanup1		// if only one pixel, no need to start an FDIV
	movl	%ecx,spancountminus1

// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z

	fildl	spancountminus1

	flds	C(d_tdivzstepu)	// C(d_tdivzstepu) | spancountminus1
	flds	C(d_zistepu)		// C(d_zistepu) | C(d_tdivzstepu) | spancountminus1
	fmul	%st(2),%st(0)	// C(d_zistepu)*scm1 | C(d_tdivzstepu) | scm1
	fxch	%st(1)			// C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
	fmul	%st(2),%st(0)	// C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
	fxch	%st(2)			// scm1 | C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1
	fmuls	C(d_sdivzstepu)	// C(d_sdivzstepu)*scm1 | C(d_zistepu)*scm1 |
							//  C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_zistepu)*scm1 | C(d_sdivzstepu)*scm1 |
							//  C(d_tdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)

	flds	fp_64k
	fdiv	%st(1),%st(0)	// this is what we've gone to all this trouble to
							//  overlap
	jmp		LFDIVInFlight1

LCleanup1:
// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z
	jmp		LFDIVInFlight1

	.align	4
LSetupNotLast1:
// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z

	fadds	zi16stepu
	fxch	%st(2)
	fadds	sdivz16stepu
	fxch	%st(2)
	flds	tdivz16stepu
	faddp	%st(0),%st(2)
	flds	fp_64k
	fdiv	%st(1),%st(0)	// z = 1/1/z
							// this is what we've gone to all this trouble to
							//  overlap
LFDIVInFlight1:

	addl	s,%esi
	addl	t,%edx
	movl	C(bbextents),%ebx
	movl	C(bbextentt),%ebp
	cmpl	%ebx,%esi
	ja		LClampHighOrLow0
LClampReFogtry0:
	movl	%esi,s
	movl	pbase,%ebx
	shll	$16,%esi
	cmpl	%ebp,%edx
	movl	%esi,sfracf
	ja		LClampHighOrLow1
LClampReFogtry1:
	movl	%edx,t
	movl	s,%esi					// sfrac = scans->sfrac;
	shll	$16,%edx
	movl	t,%eax					// tfrac = scans->tfrac;
	sarl	$16,%esi
	movl	%edx,tfracf

//
// calculate the texture starting address
//
	sarl	$16,%eax
	movl	C(cachewidth),%edx
	imull	%edx,%eax				// (tfrac >> 16) * cachewidth
	addl	%ebx,%esi
	addl	%eax,%esi				// psource = pbase + (sfrac >> 16) +
									//           ((tfrac >> 16) * cachewidth);
//
// determine whether last span or not
//
	cmpl	$16,%ecx
	jna		LLastSegment

//
// not the last segment; do full 16-wide segment
//
LNotLastSegment:

//
// advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
// get there
//

// pick up after the FDIV that was left in flight previously

	fld		%st(0)			// duplicate it
	fmul	%st(4),%st(0)	// s = s/z * z
	fxch	%st(1)
	fmul	%st(3),%st(0)	// t = t/z * z
	fxch	%st(1)
	fistpl	snext
	fistpl	tnext
	movl	snext,%eax
	movl	tnext,%edx

	movb	(%esi),%bl	// get first source texel
	subl	$16,%ecx		// count off this segments' pixels
	movl	C(sadjust),%ebp
	movl	%ecx,counttemp	// remember count of remaining pixels

	movl	C(tadjust),%ecx
	movb	%bl,(%edi)	// store first dest pixel

	addl	%eax,%ebp
	addl	%edx,%ecx

	movl	C(bbextents),%eax
	movl	C(bbextentt),%edx

	cmpl	$4096,%ebp
	jl		LClampLow2
	cmpl	%eax,%ebp
	ja		LClampHigh2
LClampReFogtry2:

	cmpl	$4096,%ecx
	jl		LClampLow3
	cmpl	%edx,%ecx
	ja		LClampHigh3
LClampReFogtry3:

	movl	%ebp,snext
	movl	%ecx,tnext

	subl	s,%ebp
	subl	t,%ecx

//
// set up advancetable
//
	movl	%ecx,%eax
	movl	%ebp,%edx
	sarl	$20,%eax			// tstep >>= 16;
	jz		LZero
	sarl	$20,%edx			// sstep >>= 16;
	movl	C(cachewidth),%ebx
	imull	%ebx,%eax
	jmp		LSetUp1

LZero:
	sarl	$20,%edx			// sstep >>= 16;
	movl	C(cachewidth),%ebx

LSetUp1:

	addl	%edx,%eax			// add in sstep
								// (tstep >> 16) * cachewidth + (sstep >> 16);
	movl	tfracf,%edx
	movl	%eax,advancetable+4	// advance base in t
	addl	%ebx,%eax			// ((tstep >> 16) + 1) * cachewidth +
								//  (sstep >> 16);
	shll	$12,%ebp			// left-justify sstep fractional part
	movl	sfracf,%ebx
	shll	$12,%ecx			// left-justify tstep fractional part
	movl	%eax,advancetable	// advance extra in t

	movl	%ecx,tstep
	addl	%ecx,%edx			// advance tfrac fractional part by tstep frac

	sbbl	%ecx,%ecx			// turn tstep carry into -1 (0 if none)
	addl	%ebp,%ebx			// advance sfrac fractional part by sstep frac
	adcl	advancetable+4(,%ecx,4),%esi	// point to next source texel

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	(%esi),%al
	addl	%ebp,%ebx
	movb	%al,1(%edi)
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,2(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,3(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	//	movl	%edx,%ecx
//	movl C(colormap),%esi
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,4(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,5(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,6(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,7(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi


//
// start FDIV for end of next segment in flight, so it can overlap
//
	movl	counttemp,%ecx
	cmpl	$16,%ecx			// more than one segment after this?
	ja		LSetupNotLast2	// yes

	decl	%ecx
	jz		LFDIVInFlight2	// if only one pixel, no need to start an FDIV
	movl	%ecx,spancountminus1
	fildl	spancountminus1

	flds	C(d_zistepu)		// C(d_zistepu) | spancountminus1
	fmul	%st(1),%st(0)	// C(d_zistepu)*scm1 | scm1
	flds	C(d_tdivzstepu)	// C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
	fmul	%st(2),%st(0)	// C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
	fxch	%st(1)			// C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1 | scm1
	faddp	%st(0),%st(3)	// C(d_tdivzstepu)*scm1 | scm1
	fxch	%st(1)			// scm1 | C(d_tdivzstepu)*scm1
	fmuls	C(d_sdivzstepu)	// C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1
	flds	fp_64k			// 64k | C(d_sdivzstepu)*scm1
	fxch	%st(1)			// C(d_sdivzstepu)*scm1 | 64k
	faddp	%st(0),%st(4)	// 64k

	fdiv	%st(1),%st(0)	// this is what we've gone to all this trouble to
							//  overlap
							
	jmp		LFDIVInFlight2

	.align	4
LSetupNotLast2:
	fadds	zi16stepu
	fxch	%st(2)
	fadds	sdivz16stepu
	fxch	%st(2)
	flds	tdivz16stepu
	faddp	%st(0),%st(2)
	flds	fp_64k
	fdiv	%st(1),%st(0)	// z = 1/1/z
							// this is what we've gone to all this trouble to
							//  overlap
LFDIVInFlight2:
	movl	%ecx,counttemp

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,8(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	




	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,9(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,10(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,11(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,12(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,13(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,14(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	$16,%edi
	movl	%edx,tfracf
	movl	snext,%edx
	movl	%ebx,sfracf
	movl	tnext,%ebx
	movl	%edx,s
	movl	%ebx,t

	movl	counttemp,%ecx		// retrieve count

//
// determine whether last span or not
//
	cmpl	$16,%ecx				// are there multiple segments remaining?
	movb	%al,-1(%edi)
	ja		LNotLastSegment		// yes

//
// last segment of scan
//
LLastSegment:

//
// advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
// get there. The number of pixels left is variable, and we want to land on the
// last pixel, not step one past it, so we can't run into arithmetic problems
//
	testl	%ecx,%ecx
	jz		LNoSteps		// just draw the last pixel and we're done

// pick up after the FDIV that was left in flight previously


	fld		%st(0)			// duplicate it
	fmul	%st(4),%st(0)	// s = s/z * z
	fxch	%st(1)
	fmul	%st(3),%st(0)	// t = t/z * z
	fxch	%st(1)
	fistpl	snext
	fistpl	tnext

	movb	(%esi),%al		// load first texel in segment
	movl	C(tadjust),%ebx
	movb	%al,(%edi)		// store first pixel in segment
	movl	C(sadjust),%eax

	addl	snext,%eax
	addl	tnext,%ebx

	movl	C(bbextents),%ebp
	movl	C(bbextentt),%edx

	cmpl	$4096,%eax
	jl		LClampLow4
	cmpl	%ebp,%eax
	ja		LClampHigh4
LClampReFogtry4:
	movl	%eax,snext

	cmpl	$4096,%ebx
	jl		LClampLow5
	cmpl	%edx,%ebx
	ja		LClampHigh5
LClampReFogtry5:

	cmpl	$1,%ecx			// don't bother
	je		LOnlyOneStep	// if two pixels in segment, there's only one step,
							//  of the segment length
	subl	s,%eax
	subl	t,%ebx

	addl	%eax,%eax		// convert to 15.17 format so multiply by 1.31
	addl	%ebx,%ebx		//  reciprocal yields 16.48

	imull	reciprocal_table_16-8(,%ecx,4)	// sstep = (snext - s) /
											//  (spancount-1)
	movl	%edx,%ebp

	movl	%ebx,%eax
	imull	reciprocal_table_16-8(,%ecx,4)	// tstep = (tnext - t) /
											//  (spancount-1)
LSetEntryvec:
//
// set up advancetable
//
	movl	entryvec_table_16(,%ecx,4),%ebx
	movl	%edx,%eax
	movl	%ebx,jumptemp		// Fogtry point into code for RET later
	movl	%ebp,%ecx
	sarl	$16,%edx			// tstep >>= 16;
	movl	C(cachewidth),%ebx
	sarl	$16,%ecx			// sstep >>= 16;
	imull	%ebx,%edx
	addl	%ecx,%edx			// add in sstep
								// (tstep >> 16) * cachewidth + (sstep >> 16);
	movl	tfracf,%ecx
	movl	%edx,advancetable+4	// advance base in t
	addl	%ebx,%edx			// ((tstep >> 16) + 1) * cachewidth +
								//  (sstep >> 16);
	shll	$16,%ebp			// left-justify sstep fractional part
	movl	sfracf,%ebx
	shll	$16,%eax			// left-justify tstep fractional part
	movl	%edx,advancetable	// advance extra in t

	movl	%eax,tstep
	movl	%ecx,%edx
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi

	jmp		*jumptemp			// jump to the number-of-pixels handler

//----------------------------------------

LNoSteps:
	movb	(%esi),%al		// load first texel in segment
	subl	$15,%edi			// adjust for hardwired offset
	jmp		LEndSpan


LOnlyOneStep:
	subl	s,%eax
	subl	t,%ebx
	movl	%eax,%ebp
	movl	%ebx,%edx
	jmp		LSetEntryvec

//----------------------------------------

.globl	Fogtry2_16, Fogtry3_16, Fogtry4_16, Fogtry5_16
.globl	Fogtry6_16, Fogtry7_16, Fogtry8_16, Fogtry9_16
.globl	Fogtry10_16, Fogtry11_16, Fogtry12_16, Fogtry13_16
.globl	Fogtry14_16, Fogtry15_16, Fogtry16_16

Fogtry2_16:
	subl	$14,%edi		// adjust for hardwired offsets
	movb	(%esi),%al
	jmp		LFogtry2_16

//----------------------------------------

Fogtry3_16:
	subl	$13,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	jmp		LFogtry3_16

//----------------------------------------

Fogtry4_16:
	subl	$12,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry4_16

//----------------------------------------

Fogtry5_16:
	subl	$11,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry5_16

//----------------------------------------

Fogtry6_16:
	subl	$10,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry6_16

//----------------------------------------

Fogtry7_16:
	subl	$9,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry7_16

//----------------------------------------

Fogtry8_16:
	subl	$8,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry8_16

//----------------------------------------

Fogtry9_16:
	subl	$7,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry9_16

//----------------------------------------

Fogtry10_16:
	subl	$6,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry10_16

//----------------------------------------

Fogtry11_16:
	subl	$5,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry11_16

//----------------------------------------

Fogtry12_16:
	subl	$4,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry12_16

//----------------------------------------

Fogtry13_16:
	subl	$3,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry13_16

//----------------------------------------

Fogtry14_16:
	subl	$2,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry14_16

//----------------------------------------

Fogtry15_16:
	decl	%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LFogtry15_16

//----------------------------------------

Fogtry16_16:
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,1(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx

LFogtry15_16:
	sbbl	%ecx,%ecx
	movb	%al,2(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx

LFogtry14_16:
	sbbl	%ecx,%ecx
	movb	%al,3(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx

LFogtry13_16:
	sbbl	%ecx,%ecx
	movb	%al,4(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx

LFogtry12_16:
	sbbl	%ecx,%ecx
	movb	%al,5(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx

LFogtry11_16:
	sbbl	%ecx,%ecx
	movb	%al,6(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx

LFogtry10_16:
	sbbl	%ecx,%ecx
	movb	%al,7(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LFogtry9_16:
	sbbl	%ecx,%ecx
	movb	%al,8(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LFogtry8_16:
	sbbl	%ecx,%ecx
	movb	%al,9(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LFogtry7_16:
	sbbl	%ecx,%ecx
	movb	%al,10(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LFogtry6_16:
	sbbl	%ecx,%ecx
	movb	%al,11(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LFogtry5_16:
	sbbl	%ecx,%ecx
	movb	%al,12(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LFogtry4_16:
	sbbl	%ecx,%ecx
	movb	%al,13(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
LFogtry3_16:
	movb	%al,14(%edi)
	movb	(%esi),%al
LFogtry2_16:

LEndSpan:

//
// clear s/z, t/z, 1/z from FP stack
//
	fstp %st(0)
	fstp %st(0)
	fstp %st(0)

	movl	pspantemp,%ebx				// restore spans pointer
	movl	espan_t_pnext(%ebx),%ebx	// point to next span
	testl	%ebx,%ebx			// any more spans?
	movb	%al,15(%edi)
	jnz		LSpanLoop			// more spans

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret

//----------------------------------------------------------------------
// Code patching routines
//----------------------------------------------------------------------
	.data

	.align 4
LFogPatchTable8:
/*
	.long	FogPatch0-4
	.long	FogPatch1-4
	.long	FogPatch2-4
	.long	FogPatch3-4
	.long	FogPatch4-4
	.long	FogPatch5-4
	.long	FogPatch6-4
	.long	FogPatch7-4
	.long	FogPatch8-4
	.long	FogPatch9-4
	.long	FogPatch10-4
	.long	FogPatch11-4
	.long	FogPatch12-4
	.long	FogPatch13-4
	.long	FogPatch14-4
	.long	FogPatch15-4
	.long	FogPatch16-4
	.long	FogPatch17-4
	.long	FogPatch18-4
	.long	FogPatch19-4
	.long	FogPatch20-4
	.long	FogPatch21-4
	.long	FogPatch22-4
	.long	FogPatch23-4
	.long	FogPatch24-4
	.long	FogPatch25-4
	.long	FogPatch26-4
	.long	FogPatch27-4
	.long	FogPatch28-4
	.long	FogPatch29-4
	.long	FogPatch30-4
	.long	FogPatch31-4
	*/
	.text

	.align 4

.globl C(R_FogPatch)
C(R_FogPatch):
	pushl	%ebx

	movl	C(colormap),%eax
	movl	$LFogPatchTable8,%ebx
	movl	$32,%ecx
LPatchLoop8:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LPatchLoop8

	popl	%ebx

	ret

#endif	// id386
