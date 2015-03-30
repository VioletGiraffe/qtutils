#pragma once

#include <assert.h>
#include <vector>

template <typename CoeffType>
class CImageInterpolationKernelBase
{
public:
	virtual ~CImageInterpolationKernelBase() {}

	virtual CoeffType coeff(int x, int y) const = 0;
	virtual int size() const = 0;
};

template <typename CoeffType>
class CImageInterpolationKernel : public CImageInterpolationKernelBase<CoeffType>
{
public:
	explicit CImageInterpolationKernel(int s): _size(s) {
		_kernel.resize(s);
		for(int i = 0; i < size(); ++i)
		{
			_kernel[i].resize(s);
			for(int k = 0; k < size(); ++k)
				_kernel[i][k] = 0.0f;
		}
	}

	CoeffType coeff(int x, int y) const override
	{
		assert(x < _size && y < _size);
		return _kernel[x][y];
	}

	int size() const override { return _size; }

protected:
	void normalizeKernel() {
		CoeffType sum = CoeffType(0.0);
		for (int i = 0; i < size(); ++i)
			for (int k = 0; k < size(); ++k)
				sum += _kernel[i][k];

		for (int i = 0; i < size(); ++i)
			for (int k = 0; k < size(); ++k)
				_kernel[i][k] /= sum;
	}

	int _size;
	std::vector<std::vector<CoeffType> > _kernel;
};

class CBicubicKernel : public CImageInterpolationKernel<float>
{
public:
	CBicubicKernel(int s, float a);
};

class CTriangularKernel : public CImageInterpolationKernel<float>
{
public:
	CTriangularKernel();
};

class CBellBicubicKernel : public CImageInterpolationKernel<float>
{
public:
	explicit CBellBicubicKernel(int s);
};

class CLanczosKernel : public CImageInterpolationKernel<float>
{
public:
	CLanczosKernel(int s, int a = 2);
};
