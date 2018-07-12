// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include <cmath>

namespace math
{
	constexpr float32 mEps = 1e-6f;
	constexpr float32 mE = 2.71828182845904523536f;
	constexpr float32 mLn2 = 0.693147180559945309417f;
	constexpr float32 mPI = 3.14159265358979323846f;
	constexpr float32 m2PI = 2.f * mPI;
	constexpr float32 m1divPI = 0.318309886183790671538f;
	constexpr float32 m1div2PI = 1.f / m2PI;
	constexpr float32 mPIdiv2 = 1.57079632679489661923f;
	constexpr float32 mPIdiv4 = 0.785398163397448309616f;
	constexpr float32 mSqrt2 = 1.41421356237309504880f;
	constexpr float32 m1divSqrt2 = 0.707106781186547524401f;

	constexpr float64 mEpsf64 = 1e-15;
	constexpr float64 mEf64 = 2.71828182845904523536;
	constexpr float64 mLn2f64 = 0.693147180559945309417;
	constexpr float64 mPIf64 = 3.14159265358979323846;
	constexpr float64 m2PIf64 = 2 * mPIf64;
	constexpr float64 m1divPIf64 = 0.318309886183790671538;
	constexpr float64 m1div2PIf64 = 1.0 / m2PIf64;
	constexpr float64 mPIdiv2f64 = 1.57079632679489661923;
	constexpr float64 mPIdiv4f64 = 0.785398163397448309616;
	constexpr float64 mSqrt2f64 = 1.41421356237309504880;
	constexpr float64 m1divSqrt2f64 = 0.707106781186547524401;

	template <
		typename LeftValTy,
		typename RightValTy,
		typename EpsilonType> bool ScalarNearEqual(
			LeftValTy left,
			RightValTy right,
			EpsilonType epsilon)
	{
		return Less(abs(Diff(right, left)), epsilon);
	}
}
