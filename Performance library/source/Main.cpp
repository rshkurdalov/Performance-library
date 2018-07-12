#include "Application.h"
#include "atc\StaticOperators.h"
#include "atc\Function.h"
#include "math\VectorMath.h"
#include <vector>
#include <numeric>
#include <math.h>
using namespace atc;

using namespace kernel;
using namespace util;
using namespace graphics;
using namespace ui;

float32 binOp(float32 *a, float32 *b)
{
	return (*a)*(*b);
}
float32 sequenceOp(float32 a, float32 b)
{
	return a + b;
}

Vector2f bezier2(Vector2f point1, Vector2f point2, Vector2f point3, float32 t)
{
	Vector2f ret((1.0f - t)*((1.0f - t)*point1 + (2.0f*t)*point2) + (t*t)*point3);
	return Vector2f((1.0f - t)*((1.0f - t)*point1 + (2.0f*t)*point2) + (t*t)*point3);
}

float32 distToBezier(Vector2f p0, Vector2f p1, Vector2f p2, Vector2f p)
{
	float32 c1 = (p0.x - 2.0f * p1.x + p2.x),
		c2 = (p0.y - 2.0f * p1.y + p2.y),
		c3;
	float32 k0 = 4.0f * ((p1.x - p0.x)*(p0.x - p.x) + (p1.y - p0.y)*(p0.y - p.y)),
		k1 = 4.0f * (c1 * (p0.x - p.x) + c2 * (p0.y - p.y) + 2.0f * ((p1.x - p0.x)*(p1.x - p0.x) + (p1.y - p0.y)*(p1.y - p0.y))),
		k2 = 12.0f * (c1*(p1.x - p0.x) + c2 * (p1.y - p0.y)),
		k3 = 4.0f * (c1 * c1 + c2 * c2);
	float32 t1, t2, t3;
	if (k3 == 0 || k0 == 0)
	{
		k0 = k0;
	}
	k0 /= k3;
	k1 /= k3;
	k2 /= k3;
	float32 q, r, d;
	q = (3.0f*k1 - (k2*k2)) / 9.0f;
	r = (k2 * (9.0f*k1 - 2.0f*(k2*k2)) - (27.0f*k0)) / 54.0f;
	d = q * q * q + r * r;
	c1 = k2 / 3.0f;
	if (d > 0.0f)
	{
		t1 = -c1 + cbrt(r + sqrt(d)) + cbrt(r - sqrt(d));
		return VectorLength(p - bezier2(p0, p1, p2, t1));
	}
	else
	{
		if (d == 0.0f)
		{
			c2 = cbrt(r);
			t1 = 2.0f*c2 - c1;
			t2 = -c1 - c2;
			return Min(
				VectorLength(p - bezier2(p0, p1, p2, t1)),
				VectorLength(p - bezier2(p0, p1, p2, t2)));
		}
		else
		{
			q = -q;
			c3 = acos(r / (q*sqrt(q)));
			c2 = 2.0f*sqrt(q);
			t1 = c2 * cos(c3 / 3.0f) - c1;
			t2 = c2 * cos((c3 + 2.0f*mPI) / 3.0f) - c1;
			t3 = c2 * cos((c3 + 4.0f*mPI) / 3.0f) - c1;
			return Min(
				VectorLength(p - bezier2(p0, p1, p2, t1)),
				VectorLength(p - bezier2(p0, p1, p2, t2)),
				VectorLength(p - bezier2(p0, p1, p2, t3)));
		}
	}
}

int main()
{
	Vector<2, float32> p1(500, 300), p2(300, 400), p3(295, 500), p4(100, 700);

	Matrix<3, 3, float32> m(2), k(m);
	m[0][0] = 3; m[0][1] = 5;
	m[1][0] = 8; m[1][2] = 7;
	m[2][1] = 14; m[2][2] = 17;
	k = m;
	m = MatrixInverse(m);

	float64 result=10; 
	
	std::vector<float32> v = {1,3,5};
	float32 acRes = Accumulate<3>(
		Add<float32, float32>,
		1.0f,
		v.data());
	Add(result, result + result);

	Application app;
	app.Initialize();
	app.Run();
	app.Shutdown();
	return 0;
}
