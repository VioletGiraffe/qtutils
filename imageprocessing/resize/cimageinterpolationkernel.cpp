#include "cimageinterpolationkernel.h"
#define _USE_MATH_DEFINES
#include <math.h>

static float bicubic(int i, int size, float a)
{
	const float x = (((float)i/(size-1))*2.0f - 1.0f) * 1.95f;
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

static float triang(int i, int size)
{
	const float x = (((float)i-(float)size)/2.0f) / (size/2.0f);
	return x <= 0 ? x + 1.0f : 1.0f - x;
}

static float bellBicubic(int i, int size)
{
	const float f = ((float)i / size - 0.5f) * 1.5f; //[-3/2;+3/2]
	if( f > -1.5 && f < -0.5 )
	{
		return( 0.5 * pow(f + 1.5, 2.0));
	}
	else if( f > -0.5 && f < 0.5 )
	{
		return 3.0 / 4.0 - ( f * f );
	}
	else if( ( f > 0.5 && f < 1.5 ) )
	{
		return( 0.5 * pow(f - 1.5, 2.0));
	}
	else
		return 0.0f;
}

static float lanczos(int a, int i, int size)
{
	const float x = ((float)i / size - 0.5f) * a;
	if (fabs(x) < 0.0001f)
		return 1.0f;
	else if(fabs(x) < a && fabs(x) > 0)
		return a * sin(M_PI * x) * sin(M_PI * x / a) / M_PI / M_PI / x / x;
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

CLanczosKernel::CLanczosKernel(int a /*= 2*/): CImageInterpolationKernel(4)
{
	for (int i = 0; i < size(); ++i)
		for (int k = 0; k < size(); ++k)
			_kernel[i][k] = lanczos(a, i, size())*lanczos(k, i, size());

	normalizeKernel();
}
