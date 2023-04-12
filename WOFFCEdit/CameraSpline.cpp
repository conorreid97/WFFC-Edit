#include "CameraSpline.h"

CameraSpline::CameraSpline()
{

	p0 = XMVectorSet(-10.0f, 0.0f, 0.0f, 1.0f);
	p1 = XMVectorSet(-5.0f, 5.0f, 0.0f, 1.0f);
	p2 = XMVectorSet(5.0f, -5.0f, 0.0f, 1.0f);
	p3 = XMVectorSet(10.0f, 0.0f, 0.0f, 1.0f);

	a1 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	a2 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	a3 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	b1 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	b2 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	c = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);


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
	a1 = ((2 * p1) + (p2 - p0) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0 * tParam * (1.0f - tParam);
	a2 = ((2 * p2) + (p3 - p1) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1 * tParam * (1.0f - tParam);
	a3 = ((2 * p3) + (p2 - p1) * (1.0f - tParam)) * (tParam * tParam) + p2 * (2 * tParam - tParam * tParam);

	// compute B1 and B2
	b1 = ((2 * a1) + (a2 - p0) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p0 * tParam * (1.0f - tParam);
	b2 = ((2 * a2) + (a3 - p1) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + p1 * tParam * (1.0f - tParam);

	// compute C
	c = ((2 * b1) + (b2 - a1) * tParam) * ((1.0f - tParam) * (1.0f - tParam)) + a1 * tParam * (1.0f - tParam);

	// convert XMVECTOR to XMFLOAT3
	XMFLOAT3 result;
	XMStoreFloat3(&result, c);

	return result;
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
