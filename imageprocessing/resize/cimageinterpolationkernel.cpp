#include "cimageinterpolationkernel.h"
#define _USE_MATH_DEFINES
#include <math.h>

inline float bicubic(int i, int size, float a)
{
	const float x = ((float)i / size - 0.5f) * 2.0f * 2.0f; //[-2;+2]
	const float fabsx = fabs(x);
	if (fabsx <= 1.0f)
		return (a+2)*fabsx*fabsx*fabsx - (a+3)*fabsx*fabsx + 1.0f;
	else if(fabsx > 1 && fabsx < 2)
		return a*fabsx*fabsx*fabsx -5*a*fabsx*fabsx + 8*a*fabsx - 4*a;
	else
		return 0;
}

CBicubicKernel::CBicubicKernel(int s, float a): CImageInterpolationKernel(s)
{
	for (int i = 0; i < size(); ++i)
		for (int k = 0; k < size(); ++k)
			_kernel[i][k] = bicubic(i, size(), a)*bicubic(k, size(), a);

	normalizeKernel();
}

inline float triang(int i, int size)
{
	const float x = ((float)i / size - 0.5f) * 2.0f; //[-1;+1]
	return x <= 0.0f ? x + 1.0f : 1.0f - x;
}

inline float bellBicubic(int i, int size)
{
	const float f = ((float)i / size - 0.5f) * 2.0f * 1.5f; //[-3/2;+3/2]
	if( f > -1.5f && f < -0.5f )
	{
		return( 0.5f * (float)pow(f + 1.5, 2.0));
	}
	else if( f > -0.5f && f < 0.5f )
	{
		return 3.0f / 4.0f - ( f * f );
	}
	else if( ( f > 0.5f && f < 1.5f ) )
	{
		return( 0.5f * (float)pow(f - 1.5, 2.0));
	}
	else
		return 0.0f;
}

inline float lanczos(int a, int i, int size)
{
	const float x = ((float)i / size - 0.5f) * 2.0f * a; //[-a; a]
	if (fabs(x) < 0.0001f)
		return 1.0f;
	else if(fabs(x) < a && fabs(x) > 0)
		return (float)(a * sin(M_PI * x) * sin(M_PI * x / a) / M_PI / M_PI / x / x);
	else
		return 0.0f;
}

CTriangularKernel::CTriangularKernel(): CImageInterpolationKernel(4)
{
	for (int i = 0; i < size(); ++i)
		for (int k = 0; k < size(); ++k)
			_kernel[i][k] = triang(i, size())*triang(k, size());

	normalizeKernel();
}

CBellBicubicKernel::CBellBicubicKernel(int s): CImageInterpolationKernel(s)
{
	for (int i = 0; i < size(); ++i)
		for (int k = 0; k < size(); ++k)
			_kernel[i][k] = bellBicubic(i, size())*bellBicubic(k, size());

	normalizeKernel();
}

CLanczosKernel::CLanczosKernel(int s, int a /*= 2*/): CImageInterpolationKernel(s)
{
	for (int i = 0; i < size(); ++i)
		for (int k = 0; k < size(); ++k)
			_kernel[i][k] = lanczos(a, i, size())*lanczos(a, k, size());

	normalizeKernel();
}
