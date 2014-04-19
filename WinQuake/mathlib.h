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
// mathlib.h

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

struct mplane_s;

extern vec3_t vec3_origin;
extern	int nanmask;

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);
#define CLAMP(min, x, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x)) //johnfitz

vec_t _DotProduct (vec3_t v1, vec3_t v2);
void _VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorCopy (vec3_t in, vec3_t out);

int VectorCompare (vec3_t v1, vec3_t v2);
vec_t Length (vec3_t v);
void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
float VectorNormalize (vec3_t v);		// returns vector length
void VectorInverse (vec3_t v);
void VectorScale (vec3_t in, vec_t scale, vec3_t out);
int Q_log2(int val);

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

void FloorDivMod (double numer, double denom, int *quotient,
		int *rem);
fixed16_t Invert24To16(fixed16_t val);
int GreatestCommonDivisor (int i1, int i2);

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
float	anglemod(float a);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  start
#define bound(min,num,max) (num >= min ? (num < max ? num : max) : min)
int _mathlib_temp_int1, _mathlib_temp_int2, _mathlib_temp_int3;
float _mathlib_temp_float1, _mathlib_temp_float2, _mathlib_temp_float3;
vec3_t _mathlib_temp_vec1, _mathlib_temp_vec2, _mathlib_temp_vec3;
#if !defined _WIN32 || defined __MINGW32__
#define min(val1,val2) (val2 < val1 ? val2 : val1)
#define max(val1,val2) (val2 > val1 ? val2 : val1)
#endif
// 2001-11-15 DarkPlaces general builtin functions by LordHavoc  end

void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );	// 2001-12-10 Reduced compiler warnings by Jeff Ford

#define VectorScalarMult(a,b,c) {c[0]=a[0]*b;c[1]=a[1]*b;c[2]=a[2]*b;}	// 2001-12-15 Oriented sprites fix by Atomizer

//static fixed_t FixedMul(fixed_t a, fixed_t b)
//{
//  return (fixed_t)((long long) a*b >> FRACBITS);
//}

#define MIN(x, y)		((x) <= (y) ? (x) : (y))
#define MAX(x, y)		((x) >= (y) ? (x) : (y))
#define MID(min, val, max)	MAX(min, MIN(val, max))


#define VectorMAMAM(scale1, b1, scale2, b2, scale3, b3, c) ((c)[0] = (scale1) * (b1)[0] + (scale2) * (b2)[0] + (scale3) * (b3)[0],(c)[1] = (scale1) * (b1)[1] + (scale2) * (b2)[1] + (scale3) * (b3)[1],(c)[2] = (scale1) * (b1)[2] + (scale2) * (b2)[2] + (scale3) * (b3)[2])




#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

#define VectorClear(a) ((a)[0]=(a)[1]=(a)[2]=0)

#define VectorInterpolate(v1, _frac, v2, v)							\
do {																\
	_mathlib_temp_float1 = _frac;									\
																	\
	(v)[0] = (v1)[0] + _mathlib_temp_float1 * ((v2)[0] - (v1)[0]);	\
	(v)[1] = (v1)[1] + _mathlib_temp_float1 * ((v2)[1] - (v1)[1]);	\
	(v)[2] = (v1)[2] + _mathlib_temp_float1 * ((v2)[2] - (v1)[2]);	\
} while (0);

#define AngleInterpolate(v1, _frac, v2, v)									\
do {																		\
	_mathlib_temp_float1 = _frac;											\
	VectorSubtract((v2), (v1), _mathlib_temp_vec1);							\
																			\
	if (_mathlib_temp_vec1[0] > 180) _mathlib_temp_vec1[0] -= 360;			\
	else if (_mathlib_temp_vec1[0] < -180) _mathlib_temp_vec1[0] += 360;	\
	if (_mathlib_temp_vec1[1] > 180) _mathlib_temp_vec1[1] -= 360;			\
	else if (_mathlib_temp_vec1[1] < -180) _mathlib_temp_vec1[1] += 360;	\
	if (_mathlib_temp_vec1[2] > 180) _mathlib_temp_vec1[2] -= 360;			\
	else if (_mathlib_temp_vec1[2] < -180) _mathlib_temp_vec1[2] += 360;	\
																			\
	(v)[0] = (v1)[0] + _mathlib_temp_float1 * _mathlib_temp_vec1[0];		\
	(v)[1] = (v1)[1] + _mathlib_temp_float1 * _mathlib_temp_vec1[1];		\
	(v)[2] = (v1)[2] + _mathlib_temp_float1 * _mathlib_temp_vec1[2];		\
} while (0);

#define FloatInterpolate(f1, _frac, f2)			\
	(_mathlib_temp_float1 = _frac,				\
	(f1) + _mathlib_temp_float1 * ((f2) - (f1)))

#define PlaneDist(point, plane) (														\
	(plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)	\
)

#define PlaneDiff(point, plane) (																			\
	(((plane)->type < 3) ? (point)[(plane)->type] - (plane)->dist: DotProduct((point), (plane)->normal) - (plane)->dist) 	\
)

#define Crap(x,y) (abs(x[0]*y[0])+abs(x[1]*y[1])+abs(x[2]*y[2]))


