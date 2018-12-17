// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "math\MathBase.h"
#include "atc\StaticOperators.h"

namespace math
{
	//Vector definition

	template <
		uint32 coordinates,
		typename ValTy> class VectorBase
	{
	public:
		ValTy coord[coordinates];
		VectorBase() {}
	};
	template <typename ValTy> class VectorBase<1, ValTy>
	{
	public:
		union
		{
			ValTy x;
			ValTy coord[1];
		};
		VectorBase() {}
		VectorBase(ValTy x)
		{
			this->x = x;
		}
	};
	template <typename ValTy> class VectorBase<2, ValTy>
	{
	public:
		union
		{
			struct { ValTy x, y; };
			ValTy coord[2];
		};
		VectorBase() {}
		VectorBase(ValTy x, ValTy y)
		{
			this->x = x;
			this->y = y;
		}
	};
	template <typename ValTy> class VectorBase<3, ValTy>
	{
	public:
		union
		{
			struct { ValTy x, y, z; };
			ValTy coord[3];
		};
		VectorBase() {}
		VectorBase(ValTy x, ValTy y, ValTy z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
	};
	template <typename ValTy> class VectorBase<4, ValTy>
	{
	public:
		union
		{
			struct { ValTy x, y, z, w; };
			ValTy coord[4];
		};
		VectorBase() {}
		VectorBase(ValTy x, ValTy y, ValTy z, ValTy w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
	};

	template <
		uint32 coordinates,
		typename ValTy> class Vector
		: public VectorBase<coordinates, ValTy>
	{
		using VectorBase::VectorBase;
	public:
		Vector() {}
		Vector(ValTy value)
		{
			*this = value;
		}
		template <
			uint32 rightCoordinates,
			typename RightValTy> Vector(
				Vector<rightCoordinates, RightValTy> &v)
		{
			*this = v;
		}
		inline ValTy &operator[] (uint32 idx)
		{
			return coord[idx];
		}
		inline Vector operator-()
		{
			Vector v;
			Assign<coordinates>(
				Negative<ValTy>,
				v.coord,
				coord);
			return v;
		}
		template <
			uint32 rightCoordinates,
			typename RightValTy> inline bool operator==(
				Vector<rightCoordinates, RightValTy> &v)
		{
			return TestProductAnd<Min(coordinates, rightCoordinates)>(
				Equal<ValTy, RightValTy>,
				coord,
				v.coord);
		}
		template <
			uint32 rightCoordinates,
			typename RightValTy> inline bool operator!=(
				Vector<rightCoordinates, RightValTy> &v)
		{
			return !(*this == v);
		}
		template <
			uint32 rightCoordinates,
			typename RightValTy> inline Vector &operator=(
				Vector<rightCoordinates, RightValTy> &v)
		{
			Assign<Min(coordinates, rightCoordinates)>(coord, v.coord);
			return *this;
		}
		inline Vector &operator=(ValTy value)
		{
			ForEach<coordinates>(
				[value] (ValTy &arg) -> void {arg = value; },
				coord);
			return *this;
		}
		template <
			uint32 rightCoordinates,
			typename RightValTy> inline Vector &operator+=(
				Vector<rightCoordinates, RightValTy> &v)
		{
			ForEach<Min(coordinates, rightCoordinates)>(
				Add<ValTy, RightValTy>,
				coord,
				v.coord);
			return *this;
		}
		template <
			uint32 rightCoordinates,
			typename RightValTy> inline Vector &operator-=(
				Vector<rightCoordinates, RightValTy> &v)
		{
			ForEach<Min(coordinates, rightCoordinates)>(
				Sub<ValTy, RightValTy>,
				coord,
				v.coord);
			return *this;
		}
		inline Vector &operator+=(ValTy value)
		{
			ForEach<coordinates>(
				[value] (ValTy &arg) -> void {arg += value; },
				coord);
			return *this;
		}
		inline Vector &operator-=(ValTy value)
		{
			ForEach<coordinates>(
				[value] (ValTy &arg) -> void {arg -= value; },
				coord);
			return *this;
		}
		inline Vector &operator*=(ValTy value)
		{
			ForEach<coordinates>(
				[value] (ValTy &arg) -> void {arg *= value; },
				coord);
			return *this;
		}
		inline Vector &operator/=(ValTy value)
		{
			ForEach<coordinates>(
				[value] (ValTy &arg) -> void {arg /= value; },
				coord);
			return *this;
		}
	};
	template <
		uint32 leftCoordinates,
		typename LeftValTy,
		uint32 rightCoordinates,
		typename RightValTy> inline auto operator+(
			Vector<leftCoordinates, LeftValTy> a,
			Vector<rightCoordinates, RightValTy> &b)
		-> Vector<Min(leftCoordinates, rightCoordinates),
		GetResultingType<LeftValTy, RightValTy>>
	{
		return a += b;
	}
	template <
		uint32 leftCoordinates,
		typename LeftValTy,
		uint32 rightCoordinates,
		typename RightValTy> inline auto operator-(
			Vector<leftCoordinates, LeftValTy> a,
			Vector<rightCoordinates, RightValTy> &b)
		->Vector<Min(leftCoordinates, rightCoordinates),
		GetResultingType<LeftValTy, RightValTy>>
	{
		return a -= b;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline
		ReturnTypeIf<Vector<coordinates, ValTy>, IsScalar<ArgType>> operator+(
			Vector<coordinates, ValTy> v,
			ArgType value)
	{
		return v += value;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline
		ReturnTypeIf<Vector<coordinates, ValTy>, IsScalar<ArgType>> operator+(
			ArgType value,
			Vector<coordinates, ValTy> v)
	{
		return v += value;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline
		ReturnTypeIf<Vector<coordinates, ValTy>, IsScalar<ArgType>> operator-(
			Vector<coordinates, ValTy> v,
			ArgType value)
	{
		return v /= value;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline
		ReturnTypeIf<Vector<coordinates, ValTy>, IsScalar<ArgType>> operator-(
			ArgType value,
			Vector<coordinates, ValTy> v)
	{
		ForEach<coordinates>(
			[value] (ValTy &arg) -> void {arg = value - arg; },
			coord);
		return v;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline
		ReturnTypeIf<Vector<coordinates, ValTy>, IsScalar<ArgType>> operator*(
			Vector<coordinates, ValTy> v,
			ArgType value)
	{
		return v *= value;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline
		ReturnTypeIf<Vector<coordinates, ValTy>, IsScalar<ArgType>> operator*(
			ArgType value,
			Vector<coordinates, ValTy> v)
	{
		return v *= value;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline Vector<coordinates, ValTy> operator/(
			Vector<coordinates, ValTy> v,
			ArgType value)
	{
		return v /= value;
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ArgType> inline Vector<coordinates, ValTy> operator/(
			ArgType value,
			Vector<coordinates, ValTy> v)
	{
		ForEach<coordinates>(
			[value] (ValTy &arg) -> void {arg = value / arg; },
			coord);
		return v;
	}

	template <
		uint32 coordinates,
		typename ValTy,
		typename EpsilonType> inline bool VectorNearEqual(
			Vector<coordinates, ValTy> &a,
			Vector<coordinates, ValTy> &b,
			EpsilonType epsilon)
	{
		return TestProductAnd<coordinates>(
			[epsilon] (ValTy left, ValTy right)
			-> bool {return ScalarNearEqual(left, right, epsilon); },
			a.coord,
			b.coord);
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorAbsolute(Vector<coordinates, ValTy> v)
	{
		ForEach<coordinates>([] (ValTy &arg) -> void {arg = abs(arg); }, v.coord);
		return v;
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorFloor(Vector<coordinates, ValTy> v)
	{
		ForEach<coordinates>([] (ValTy &arg) -> void {arg = floor(arg); }, v.coord);
		return v;
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorCeil(Vector<coordinates, ValTy> v)
	{
		ForEach<coordinates>([] (ValTy &arg) -> void {arg = ceil(arg); }, v.coord);
		return v;
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorTrunc(Vector<coordinates, ValTy> v)
	{
		ForEach<coordinates>([] (ValTy &arg) -> void {arg = trunc(arg); }, v.coord);
		return v;
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorRound(Vector<coordinates, ValTy> v)
	{
		ForEach<coordinates>([] (ValTy &arg) -> void {arg = round(arg); }, v.coord);
		return v;
	}
	template <uint32 coordinates, typename ValTy>
	inline ValTy VectorDot(
		Vector<coordinates, ValTy> &a,
		Vector<coordinates, ValTy> &b)
	{
		return Accumulate<coordinates>(
			Product<ValTy, ValTy>,
			Add<ValTy, ValTy>,
			static_cast<ValTy>(0),
			a.coord,
			b.coord);
	}
	template <uint32 coordinates, typename ValTy>
	inline ValTy VectorLength(Vector<coordinates, ValTy> &v)
	{
		return sqrt(VectorDot(v, v));
	}
	template <uint32 coordinates, typename ValTy>
	inline ValTy VectorLength2(Vector<coordinates, ValTy> &v)
	{
		return VectorDot(v, v);
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorNormalize(Vector<coordinates, ValTy> v)
	{
		return v /= VectorLength(v);
	}
	template <uint32 coordinates, typename ValTy>
	inline ReturnTypeIf<Vector<2, ValTy>, coordinates == 2> VectorCross(
		Vector<coordinates, ValTy> &a,
		Vector<coordinates, ValTy> &b)
	{
		return Vector<2, ValTy>(
			a[0] * b[1] - a[1] * b[0],
			a[0] * b[1] - a[1] * b[0]);
	}
	template <uint32 coordinates, typename ValTy>
	inline ReturnTypeIf<Vector<3, ValTy>, coordinates >= 3> VectorCross(
		Vector<coordinates, ValTy> &a,
		Vector<coordinates, ValTy> &b)
	{
		return Vector<3, ValTy>(
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0]);
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorClamp(
		Vector<coordinates, ValTy> v,
		Vector<coordinates, ValTy> &vMin,
		Vector<coordinates, ValTy> &vMax)
	{
		ForEach<coordinates>([] (ValTy &arg, ValTy argMin, ValTy argMax)
			-> void {arg = Min(Max(arg, argMin), argMax); },
			v.coord,
			vMin.coord,
			vMax.coord);
		return v;
	}
	template <uint32 coordinates, typename ValTy>
	inline ValTy VectorAngleBetweenVectors(
		Vector<coordinates, ValTy> &a,
		Vector<coordinates, ValTy> &b)
	{
		return acos(VectorDot(a, b)
			/ sqrt(VectorLength2(a)*VectorLength2(b)));
	}
	template <uint32 coordinates, typename ValTy>
	inline ValTy VectorAngleBetweenNormals(
		Vector<coordinates, ValTy> &a,
		Vector<coordinates, ValTy> &b)
	{
		return acos(VectorDot(a, b));
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorClosestPointOnLine(
		Vector<coordinates, ValTy> &linePoint1,
		Vector<coordinates, ValTy> &linePoint2,
		Vector<coordinates, ValTy> &point)
	{
		Vector<coordinates, ValTy> dir = linePoint2 - linePoint1;
		ValTy lineLength = VectorLength(dir),
			distance = VectorDot(point - linePoint1, dir /= lineLength);
		if (distance < static_cast<ValTy>(0)) return linePoint1;
		else if (distance > lineLength) return linePoint2;
		return linePoint1 + dir * distance;
	}
	template <uint32 coordinates, typename ValTy>
	inline Vector<coordinates, ValTy> VectorClosestPointOnInfiniteLine(
		Vector<coordinates, ValTy> &linePoint1,
		Vector<coordinates, ValTy> &linePoint2,
		Vector<coordinates, ValTy> &point)
	{
		Vector<coordinates, ValTy> dirNormal
			= VectorNormalize(linePoint2 - linePoint1);
		return linePoint1 + dirNormal * VectorDot(point - linePoint1, dirNormal);
	}
	template <typename ValTy>
	inline bool VectorLinesIntersect(
		Vector<2, ValTy> &line1Point1,
		Vector<2, ValTy> &line1Point2,
		Vector<2, ValTy> &line2Point1,
		Vector<2, ValTy> &line2Point2,
		Vector<2, ValTy> *intersectPoint)
	{
		Vector<2, float64> v1 = line1Point2 - line1Point1,
			v2 = line2Point2 - line2Point1;
		float64 c = v1[0] * v2[1] - v1[1] * v2[0];
		if (ScalarNearEqual(
			c,
			static_cast<float64>(0),
			static_cast<float64>(mEps)))
			return false;
		c = static_cast<float64>(1) / c;
		float64 c1 = v2[0] * (line1Point1[1] - line2Point1[1]) - v2[1] * (line1Point1[0] - line2Point1[0]);
		(*intersectPoint)[0] = static_cast<ValTy>(line1Point1[0] + c1*c*v1[0]);
		(*intersectPoint)[1] = static_cast<ValTy>(line1Point1[1] + c1*c*v1[1]);
		return true;
	}
	template <uint32 coordinates, typename ValTy>
	inline bool VectorCCWTestLH(
		Vector<coordinates, ValTy> &triPoint1,
		Vector<coordinates, ValTy> &triPoint2,
		Vector<coordinates, ValTy> &triPoint3)
	{
		static_assert(coordinates >= 2, "Invalid vector size");
		return GreaterEqual(
			(triPoint2[0] - triPoint1[0])*(triPoint3[1] - triPoint1[1])
			- (triPoint3[0] - triPoint1[0])*(triPoint2[1] - triPoint1[1]),
			static_cast<ValTy>(0));
	}
	template <uint32 coordinates, typename ValTy>
	inline bool VectorCCWTestRH(
		Vector<coordinates, ValTy> &triPoint1,
		Vector<coordinates, ValTy> &triPoint2,
		Vector<coordinates, ValTy> &triPoint3)
	{
		static_assert(coordinates >= 2, "Invalid vector size");
		return LessEqual(
			(triPoint2[0] - triPoint1[0])*(triPoint3[1] - triPoint1[1])
			- (triPoint3[0] - triPoint1[0])*(triPoint2[1] - triPoint1[1]),
			static_cast<ValTy>(0));
	}
	template <
		uint32 coordinates,
		typename ValTy,
		typename ...Args>
		inline Vector<coordinates, ValTy> VectorPermute(
			Vector<coordinates, ValTy> &v, Args... args)
	{
		static_assert(coordinates == sizeof...(args), "Invalid parameter count");
		uint32 idxs[coordinates] = {args...};
		Vector<coordinates, ValTy> result;
		Assign<coordinates>(
			[&v] (uint32 idx) -> ValTy {return v[idx]; },
			result.coord,
			idxs);
		return result;
	}

	typedef Vector<2, float32> Vector2f;
	typedef Vector<2, uint32> Vector2u;
	typedef Vector<2, int32> Vector2i;
	typedef Vector<3, float32> Vector3f;
	typedef Vector<3, uint32> Vector3u;
	typedef Vector<3, int32> Vector3i;
	typedef Vector<4, float32> Vector4f;
	typedef Vector<4, uint32> Vector4u;
	typedef Vector<4, int32> Vector4i;

	// Matrix definition

	template <
		uint32 rows,
		uint32 columns,
		typename ValTy> class Matrix
	{
	public:
		typedef Vector<columns, ValTy> RowType;

		RowType r[rows];
		Matrix() {}
		Matrix(ValTy value)
		{
			*this = value;
		}
		Matrix(Matrix &m)
		{
			*this = m;
		}
		inline RowType &operator[] (uint32 idx)
		{
			return r[idx];
		}
		inline Matrix operator-()
		{
			Matrix m;
			Assign<coordinates>(
				Negative<ValTy>,
				r,
				m.r);
			return m;
		}
		inline bool operator==(Matrix &m)
		{
			return TestProductAnd<rows>(
				Equal<RowType, RowType>,
				r,
				m.r);
		}
		inline bool operator!=(Matrix &m)
		{
			return !(*this == m);
		}
		inline Matrix &operator=(Matrix &m)
		{
			Assign<rows>(r, m.r);
			return *this;
		}
		inline Matrix &operator=(ValTy value)
		{
			ForEach<rows>([value] (RowType &arg) -> void {arg = value; }, r);
			return *this;
		}
		template <
			uint32 rowsRight,
			uint32 columnsRight>
			Matrix<rows, columnsRight, ValTy> operator*(
				Matrix<rowsRight, columnsRight, ValTy> &m)
		{
			static_assert(columns == rowsRight, "Incorrect matrix size");
			Matrix<rowsRight, columnsRight, ValTy> result(static_cast<ValTy>(0));
			for (uint32 i = 0; i < rows; i++)
			{
				for (uint32 j = 0; j < columnsRight; j++)
				{
					for (uint32 k = 0; k < rows; k++)
						result[i][j] += r[i][k] * m[k][j];
				}
			}
			return result;
		}
		template <
			uint32 coordinates,
			typename RightValTy,
			typename ResultValType = GetResultingType<ValTy, RightValTy>>
			inline Vector<rows, ResultValType> operator*(
				Vector<coordinates, RightValTy> &v)
		{
			static_assert(coordinates == columns, "Incorrect vector/matrix size");
			Vector<rows, ResultValType> result;
			Assign<rows>(
				[&v] (RowType &arg) -> ResultValType {return VectorDot(arg, v); },
				result.coord,
				r);
			return result;
		}
		inline Matrix &operator+=(Matrix &m)
		{
			ForEach<rows>(
				Add<RowType, RowType>,
				r,
				m.r);
			return *this;
		}
		inline Matrix &operator-=(Matrix &m)
		{
			ForEach<rows>(
				Sub<RowType, RowType>,
				r,
				m.r);
			return *this;
		}
		inline Matrix &operator+=(ValTy value)
		{
			ForEach<rows>([value] (RowType &arg) -> void {arg += value; }, r);
			return *this;
		}
		inline Matrix &operator-=(ValTy value)
		{
			ForEach<rows>([value] (RowType &arg) -> void {arg -= value; }, r);
			return *this;
		}
		inline Matrix &operator*=(ValTy value)
		{
			ForEach<rows>([value] (RowType &arg) -> void {arg *= value; }, r);
			return *this;
		}
		inline Matrix &operator/=(ValTy value)
		{
			ForEach<rows>([value] (RowType &arg) -> void {arg /= value; }, r);
			return *this;
		}
	};
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline Matrix<rows, columns, ValTy> operator+(
			Matrix<rows, columns, ValTy> a,
			Matrix<rows, columns, ValTy> &b)
	{
		return a += b;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline Matrix<rows, columns, ValTy> operator-(
			Matrix<rows, columns, ValTy> a,
			Matrix<rows, columns, ValTy> &b)
	{
		return a -= b;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline Matrix<rows, columns, ValTy> operator+(
			Matrix<rows, columns, ValTy> m,
			ArgType value)
	{
		return m += value;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline Matrix<rows, columns, ValTy> operator+(
			ArgType value,
			Matrix<rows, columns, ValTy> m)
	{
		return m += value;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline Matrix<rows, columns, ValTy> operator-(
			Matrix<rows, columns, ValTy> m,
			ArgType value)
	{
		return m -= value;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline Matrix<rows, columns, ValTy> operator-(
			ArgType value,
			Matrix<rows, columns, ValTy> m)
	{
		ForEach<rows>(
			[value] (Matrix<rows, columns, ValTy>::RowType &arg)
			-> void {arg = value - arg; },
			coord);
		return m;
	}
	template <
		uint32 coordinates,
		typename LeftValTy,
		uint32 rows,
		uint32 columns,
		typename RightValTy,
		typename ResultValType = GetResultingType<LeftValTy, RightValTy>>
		inline Vector<columns, ResultValType> operator*(
			Vector<coordinates, LeftValTy> &v,
			Matrix<rows, columns, RightValTy> &m)
	{
		static_assert(coordinates == rows, "Incorrect vector/matrix size");
		Vector<columns, ResultValType> result(static_cast<ResultValType>(0));
		for (uint32 i = 0; i < columns; i++)
			for (uint32 j = 0; j < rows; j++)
				result[i] += v[j] * m[j][i];
		return result;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline ReturnTypeIf<Matrix<rows, columns, ValTy>, IsScalar<ArgType>> operator*(
			Matrix<rows, columns, ValTy> m,
			ArgType value)
	{
		return m *= value;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline ReturnTypeIf<Matrix<rows, columns, ValTy>, IsScalar<ArgType>> operator*(
			ArgType value,
			Matrix<rows, columns, ValTy> m)
	{
		return m *= value;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline Matrix<rows, columns, ValTy> operator/(
			Matrix<rows, columns, ValTy> m,
			ArgType value)
	{
		return m /= value;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename ArgType>
		inline Matrix<rows, columns, ValTy> operator/(
			ArgType value,
			Matrix<rows, columns, ValTy> m)
	{
		ForEach<rows>(
			[value] (Matrix<rows, columns, ValTy>::RowType &arg)
			-> void {arg = value / arg; },
			coord);
		return m;
	}

	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		typename EpsilonType> inline bool MatrixNearEqual(
			Matrix<rows, columns, ValTy> &a,
			Matrix<rows, columns, ValTy> &b,
			EpsilonType epsilon)
	{
		return TestProductAnd<rows>(
			[epsilon] (Matrix<rows, columns, ValTy>::RowType &left,
				Matrix<rows, columns, ValTy>::RowType &right)
			-> bool {return abs(Diff(left, right)) < epsilon; },
			a.r,
			b.r);
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixSetIdentity(Matrix<rows, columns, ValTy> *m)
	{
		(*m) = static_cast<ValTy>(0);
		for (uint32 i = 0; i < Min(rows, columns); i++)
			(*m)[i][i] = static_cast<ValTy>(1);
	}
	inline void MatrixScale2d(
		float32 scaleX,
		float32 scaleY,
		float32 originX,
		float32 originY,
		Matrix<3, 2, float32> *m)
	{
		(*m)[0][0] = scaleX;
		(*m)[0][1] = 0.0f;
		(*m)[1][0] = 0.0f;
		(*m)[1][1] = scaleY;
		(*m)[2][0] = (1.0f - scaleX)*originX;
		(*m)[2][1] = (1.0f - scaleY)*originY;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixScale3d(
			ValTy scaleX,
			ValTy scaleY,
			ValTy scaleZ,
			Matrix<rows, columns, ValTy> *m)
	{
		static_assert(Min(rows, columns) >= 3, "Invalid matrix size");
		MatrixSetIdentity(m);
		(*m)[0][0] = scaleX;
		(*m)[1][1] = scaleY;
		(*m)[2][2] = scaleZ;
	}
	inline void MatrixRotate2d(
		float32 angle,
		float32 originX,
		float32 originY,
		Matrix<3, 2, float32> *m)
	{
		float32 c1 = cos(angle), c2 = sin(angle);
		(*m)[0][0] = (*m)[1][1] = c1;
		(*m)[0][1] = -c2;
		(*m)[1][0] = c2;
		(*m)[2][0] = (1.0f - (*m)[0][0])*originX - (*m)[1][0] * originY;
		(*m)[2][1] = (1.0f - (*m)[1][1])*originY - (*m)[0][1] * originX;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixRotate3d(
			ValTy angleX,
			ValTy angleY,
			ValTy angleZ,
			Matrix<rows, columns, ValTy> *m)
	{
		static_assert(Min(rows, columns) >= 3, "Invalid matrix size");
		MatrixSetIdentity(m);
		ValTy c1 = cos(angleX), c2 = cos(angleY), c3 = cos(angleZ),
			c4 = sin(angleX), c5 = sin(angleY), c6 = sin(angleZ);
		(*m)[0][0] = c2*c3;
		(*m)[0][1] = c4*c5*c3 + c1*c6;
		(*m)[0][2] = c1*c5*c3 + c4*c6;
		(*m)[1][0] = -c2*c6;
		(*m)[1][1] = -c4*c5*c6 + c1*c6;
		(*m)[1][2] = c1*c5*c6 + c4*c3;
		(*m)[2][0] = c6;
		(*m)[2][1] = -c4*c2;
		(*m)[2][2] = c1*c2;
	}
	inline void MatrixSkew2d(
		float32 angleX,
		float32 angleY,
		float32 originX,
		float32 originY,
		Matrix<3, 2, float32> *m)
	{
		float32 c1 = tan(angleX), c2 = tan(angleY);
		(*m)[0][0] = (*m)[1][1] = static_cast<float32>(1);
		(*m)[0][1] = c2;
		(*m)[1][0] = c1;
		(*m)[2][0] = -originX*c1;
		(*m)[2][1] = -originY*c2;
	}
	inline void MatrixTranslate2d(
		float32 x,
		float32 y,
		Matrix<3, 2, float32> *m)
	{
		(*m)[0][0] = 1.0f;
		(*m)[0][1] = 0.0f;
		(*m)[1][0] = 0.0f;
		(*m)[1][1] = 1.0f;
		(*m)[2][0] = x;
		(*m)[2][1] = y;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixTranslate3d(
			ValTy x,
			ValTy y,
			ValTy z,
			Matrix<rows, columns, ValTy> *m)
	{
		static_assert(Min(rows, columns) >= 3, "Invalid matrix size");
		MatrixSetIdentity(m);
		(*m)[0][3] = x;
		(*m)[1][3] = y;
		(*m)[2][3] = z;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixTransform3d(
			ValTy scaleX,
			ValTy scaleY,
			ValTy angleX,
			ValTy angleY,
			ValTy angleZ,
			ValTy translateX,
			ValTy translateY,
			ValTy translateZ,
			Matrix<rows, columns, ValTy> *m)
	{
		static_assert(rows == columns && Min(rows, columns) >= 3, "Invalid matrix size");
		Matrix<rows, columns, ValTy> mt;
		MatrixScale3d(scaleX, scaleY, scaleZ, m);
		MatrixRotate3d(angleX, angleY, angleZ, &mt);
		(*m) = (*m)*mt;
		MatrixTranslate3d(translateX, translateY, translateZ, &mt);
		(*m) = (*m)*mt;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixLookAtLH(
			Vector<columns, ValTy> position,
			Vector<columns, ValTy> focus,
			Vector<columns, ValTy> upDirection,
			Matrix<rows, columns, ValTy> *m)
	{
		static_assert(Min(rows, columns) >= 3, "Invalid matrix size");
		MatrixSetIdentity(m);
		Vector<columns, ValTy> cz = VectorNormalize(focus - position);
		Vector<columns, ValTy> cx = VectorNormalize(VectorCross(upDirection, cz));
		Vector<columns, ValTy> cy = VectorCross(cz, cx);
		(*m)[0][0] = cx.coord[0];
		(*m)[1][0] = cx.coord[0];
		(*m)[2][0] = cx.coord[0];
		(*m)[0][1] = cx.coord[1];
		(*m)[1][1] = cx.coord[1];
		(*m)[2][1] = cx.coord[1];
		(*m)[0][2] = cx.coord[2];
		(*m)[1][2] = cx.coord[2];
		(*m)[2][2] = cx.coord[2];
		(*m)[3][0] = -VectorDot(cx, position);
		(*m)[3][1] = -VectorDot(cy, position);
		(*m)[3][2] = -VectorDot(cz, position);
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixLookAtRH(
			Vector<columns, ValTy> position,
			Vector<columns, ValTy> focus,
			Vector<columns, ValTy> upDirection,
			Matrix<rows, columns, ValTy> *m)
	{
		Swap(position, focus);
		MatrixLookAtLH(position, focus, upDirection, m);
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixPerspectiveFovLH(
			ValTy fov,
			ValTy aspect,
			ValTy nearZ,
			ValTy farZ,
			Matrix<rows, columns, ValTy> *m)
	{
		static_assert(Min(rows, columns) >= 4, "Invalid matrix size");
		MatrixSetIdentity(m);
		ValTy c1 = tan(static_cast<ValTy>(0.5)*fov),
			c2 = static_cast<ValTy>(1) / (farZ - nearZ);
		(*m)[0][0] /= (aspect * c1);
		(*m)[1][1] /= c1;
		(*m)[2][2] = (nearZ + farZ)*c2;
		(*m)[2][3] = static_cast<ValTy>(1);
		(*m)[3][2] = -static_cast<ValTy>(2)*nearZ*farZ*c2;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixPerspectiveFovRH(
			ValTy fov,
			ValTy aspect,
			ValTy nearZ,
			ValTy farZ,
			Matrix<rows, columns, ValTy> *m)
	{
		MatrixPerspectiveFovLH(fov, aspect, nearZ, farZ, m);
		(*m)[2][3] = static_cast<ValTy>(-1);
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixOrthographicLH(
			ValTy left,
			ValTy top,
			ValTy right,
			ValTy bottom,
			Matrix<rows, columns, ValTy> *m)
	{
		static_assert(Min(rows, columns) >= 3, "Invalid matrix size");
		MatrixSetIdentity(m);
		(*m)[0][0] = static_cast<ValTy>(2) / (right - left);
		(*m)[1][1] = static_cast<ValTy>(2) / (bottom - top);
		(*m)[0][2] = -(right + left) / (right - left);
		(*m)[1][2] = -(top + bottom) / (top - bottom);
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		inline void MatrixOrthographicRH(
			ValTy left,
			ValTy top,
			ValTy right,
			ValTy bottom,
			Matrix<rows, columns, ValTy> *m)
	{
		MatrixOrthographicLH(left, top, right, bottom, m);
		Negate((*m)[1][2]);
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		uint32 n = Min(rows, columns)>
		ReturnTypeIf<ValTy, n >= 5> MatrixDet(
			Matrix<rows, columns, ValTy> &m)
	{
		ValTy result = static_cast<ValTy>(0);
		Matrix<n - 1, n - 1, ValTy> subm;
		for (uint32 i = 0; i < n; i++)
		{
			for (uint32 j = 1; j < n; j++)
			{
				for (uint32 k = 0; k < n; k++)
					if (k < i) subm[j - 1][k] = m[j][k];
					else if (k > i) subm[j - 1][k - 1] = m[j][k];
			}
			if (i & 1) result -= m[0][i] * MatrixDet(subm);
			else result += m[0][i] * MatrixDet(subm);
		}
		return result;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		uint32 n = Min(rows, columns)>
		inline ReturnTypeIf<ValTy, n == 1> MatrixDet(
			Matrix<rows, columns, ValTy> &m)
	{
		return m[0][0];
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		uint32 n = Min(rows, columns)>
		inline ReturnTypeIf<ValTy, n == 2> MatrixDet(
			Matrix<rows, columns, ValTy> &m)
	{
		return m[0][0] * m[1][1] - m[0][1] * m[1][0];
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		uint32 n = Min(rows, columns)>
		inline ReturnTypeIf<ValTy, n == 3> MatrixDet(
			Matrix<rows, columns, ValTy> &m)
	{
		return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0]
			+ m[0][2] * m[1][0] * m[2][1] - m[0][2] * m[1][1] * m[2][0]
			- m[0][1] * m[1][0] * m[2][2] - m[0][0] * m[1][2] * m[2][1];
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy,
		uint32 n = Min(rows, columns)>
		ReturnTypeIf<ValTy, n == 4> MatrixDet(
			Matrix<rows, columns, ValTy> &m)
	{
		return m[0][3] * m[1][2] * m[2][1] * m[3][0] - m[0][2] * m[1][3] * m[2][1] * m[3][0] -
			m[0][3] * m[1][1] * m[2][2] * m[3][0] + m[0][1] * m[1][3] * m[2][2] * m[3][0] +
			m[0][2] * m[1][1] * m[2][3] * m[3][0] - m[0][1] * m[1][2] * m[2][3] * m[3][0] -
			m[0][3] * m[1][2] * m[2][0] * m[3][1] + m[0][2] * m[1][3] * m[2][0] * m[3][1] +
			m[0][3] * m[1][0] * m[2][2] * m[3][1] - m[0][0] * m[1][3] * m[2][2] * m[3][1] -
			m[0][2] * m[1][0] * m[2][3] * m[3][1] + m[0][0] * m[1][2] * m[2][3] * m[3][1] +
			m[0][3] * m[1][1] * m[2][0] * m[3][2] - m[0][1] * m[1][3] * m[2][0] * m[3][2] -
			m[0][3] * m[1][0] * m[2][1] * m[3][2] + m[0][0] * m[1][3] * m[2][1] * m[3][2] +
			m[0][1] * m[1][0] * m[2][3] * m[3][2] - m[0][0] * m[1][1] * m[2][3] * m[3][2] -
			m[0][2] * m[1][1] * m[2][0] * m[3][3] + m[0][1] * m[1][2] * m[2][0] * m[3][3] +
			m[0][2] * m[1][0] * m[2][1] * m[3][3] - m[0][0] * m[1][2] * m[2][1] * m[3][3] -
			m[0][1] * m[1][0] * m[2][2] * m[3][3] + m[0][0] * m[1][1] * m[2][2] * m[3][3];
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		Matrix<columns, rows, ValTy> MatrixTranspose(
			Matrix<rows, columns, ValTy> m)
	{
		for (uint32 i = 1; i < rows; i++)
		{
			for (uint32 j = 0; j < i; j++)
				Swap(m[i][j], m[j][i]);
		}
		return m;
	}
	template <
		uint32 rows,
		uint32 columns,
		typename ValTy>
		Matrix<rows, columns, ValTy> MatrixInverse(
			Matrix<rows, columns, ValTy> &m,
			ValTy *pDet = nullptr)
	{
		static_assert(rows == columns, "Invalid matrix size");
		constexpr uint32 n = rows;
		ValTy det = MatrixDet(m);
		if (pDet != nullptr) *pDet = det;
		if (det != static_cast<ValTy>(0))
		{
			Matrix<n, n, ValTy> cfm;
			Matrix<n - 1, n - 1, ValTy> subm;
			for (uint32 i = 0; i < n; i++)
			{
				for (uint32 j = 0; j < n; j++)
				{
					for (uint32 k = 0; k < n; k++)
					{
						for (uint32 p = 0; p < n; p++)
							if (k < i && p < j) subm[k][p] = m[k][p];
							else if (k < i && p > j) subm[k][p - 1] = m[k][p];
							else if (k > i && p < j) subm[k - 1][p] = m[k][p];
							else if (k > i && p > j) subm[k - 1][p - 1] = m[k][p];
					}
					if ((i + j) & 1) cfm[i][j] = -MatrixDet(subm);
					else cfm[i][j] = MatrixDet(subm);
				}
			}
			m = MatrixTranspose(cfm);
			return m *= (static_cast<ValTy>(1) / det);
		}
		else return m;
	}

	typedef Matrix<2, 2, float32> Matrix2x2f;
	typedef Matrix<2, 2, uint32> Matrix2x2u;
	typedef Matrix<2, 2, int32> Matrix2x2i;
	typedef Matrix<3, 2, float32> Matrix3x2f;
	typedef Matrix<3, 2, uint32> Matrix3x2u;
	typedef Matrix<3, 2, int32> Matrix3x2i;
	typedef Matrix<3, 3, float32> Matrix3x3f;
	typedef Matrix<3, 3, uint32> Matrix3x3u;
	typedef Matrix<3, 3, int32> Matrix3x3i;
	typedef Matrix<4, 4, float32> Matrix4x4f;
	typedef Matrix<4, 4, uint32> Matrix4x4u;
	typedef Matrix<4, 4, int32> Matrix4x4i;

	template <typename ValTy> class Rect
	{
	public:
		ValTy left, top, right, bottom;
		Rect() {}
		Rect(
			ValTy left,
			ValTy top,
			ValTy right,
			ValTy bottom)
			: left(left),
			top(top),
			right(right),
			bottom(bottom) {}
	};

	template <typename ValTy> inline ValTy ToRadians(ValTy degrees)
	{
		return degrees*(static_cast<ValTy>(mPIf64) / static_cast<ValTy>(180));
	}
	template <typename ValTy> inline ValTy ToDegrees(ValTy radians)
	{
		return radians*(static_cast<ValTy>(180) / static_cast<ValTy>(mPIf64));
	}
}
