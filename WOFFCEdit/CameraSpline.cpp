#include "CameraSpline.h"

CameraSpline::CameraSpline()
{

	p0 = XMFLOAT3(-10.0f, 0.0f, 0.0f);
	p1 = XMFLOAT3(-5.0f, 5.0f, 0.0f);
	p2 = XMFLOAT3(5.0f, -5.0f, 0.0f);
	p3 = XMFLOAT3(10.0f, 0.0f, 0.0f);

	a1 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	a2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	a3 = XMFLOAT3(0.0f, 0.0f, 0.0f);

	b1 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	b2 = XMFLOAT3(0.0f, 0.0f, 0.0f);

	c = XMFLOAT3(0.0f, 0.0f, 0.0f);


	currentTime = 0.0f;
	t = 0.0f;

	segment = 0.0f;
	iSegment = 0;
	tParam = 0.0f;

	splineCamPos = XMFLOAT3(0.0, 0.0, 0.0);

}

CameraSpline::~CameraSpline()
{
}

XMFLOAT3 CameraSpline::catmullRomSpline(float time)
{
	// wrap time around
	t = fmod(time, 1.0f);

	// number of segments = number of control points - 1 = 3
	segment = t * 2;
	iSegment = (int)segment;
	tParam = (segment - iSegment) / 2;

	// compute intermediate control points
	a1.x = ((2 * p1.x) + (p2.x - p0.x) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0.x * tParam * (1.0f - tParam);
	a1.y = ((2 * p1.y) + (p2.y - p0.y) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0.y * tParam * (1.0f - tParam);
	a1.z = ((2 * p1.z) + (p2.z - p0.z) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0.z * tParam * (1.0f - tParam);

	a2.x = ((2 * p2.x) + (p3.x - p1.x) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1.x * tParam * (1.0f - tParam);
	a2.y = ((2 * p2.y) + (p3.y - p1.y) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1.y * tParam * (1.0f - tParam);
	a2.z = ((2 * p2.z) + (p3.z - p1.z) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1.z * tParam * (1.0f - tParam);

	a3.x = ((2 * p3.x) + (p2.x - p1.x) * (1.0f - tParam)) * (tParam * tParam) + p2.x * (2 * tParam - tParam * tParam);
	a3.y = ((2 * p3.y) + (p2.y - p1.y) * (1.0f - tParam)) * (tParam * tParam) + p2.y * (2 * tParam - tParam * tParam);
	a3.z = ((2 * p3.z) + (p2.z - p1.z) * (1.0f - tParam)) * (tParam * tParam) + p2.z * (2 * tParam - tParam * tParam);

	// compute B1 and B2
	b1.x = ((2 * a1.x) + (a2.x - p0.x) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0.x * tParam * (1.0f - tParam);
	b1.y = ((2 * a1.y) + (a2.y - p0.y) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0.y * tParam * (1.0f - tParam);
	b1.z = ((2 * a1.z) + (a2.z - p0.z) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0.z * tParam * (1.0f - tParam);

	b2.x = ((2 * a2.x) + (a3.x - p1.x) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1.x * tParam * (1.0f - tParam);
	b2.y = ((2 * a2.y) + (a3.y - p1.y) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1.y * tParam * (1.0f - tParam);
	b2.z = ((2 * a2.z) + (a3.z - p1.z) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1.z * tParam * (1.0f - tParam);

	// compute C
	c.x = ((2 * b1.x) + (b2.x - a1.x) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + a1.x * tParam * (1.0f - tParam);
	c.y = ((2 * b1.y) + (b2.y - a1.y) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + a1.y * tParam * (1.0f - tParam);
	c.z = ((2 * b1.z) + (b2.z - a1.z) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + a1.z * tParam * (1.0f - tParam);

	// convert XMVECTOR to XMFLOAT3
	//XMFLOAT3 result;
	//XMStoreFloat3(&result, c);

	return c;
}

XMFLOAT3 CameraSpline::update()
{

	// Increment the current time
	currentTime += timeDelta;

	// Wrap the time around to keep it within the range [0, 1)
	if (currentTime >= 1.0f)
	{
		currentTime = 0.0f;
	}

	// update camera to spline pos
	splineCamPos = catmullRomSpline(currentTime);

	return splineCamPos;
}
