#include "Application.h"
#include "atc\StaticOperators.h"
#include "atc\Function.h"
#include "math\VectorMath.h"
#include <vector>
#include <numeric>
#include <math.h>
#include <tuple>
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
int main()
{
	Vector<2, float32> p1(500, 300), p2(300, 400), p3(295, 500), p4(0, 700);

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
