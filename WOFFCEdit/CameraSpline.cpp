#include "CameraSpline.h"

CameraSpline::CameraSpline()
{


	p0 = XMVectorSet(-10.0f, 0.0f, 0.0f, 1.0f);
	p1 = XMVectorSet(-5.0f, 5.0f, 0.0f, 1.0f);
	p2 = XMVectorSet(5.0f, -5.0f, 0.0f, 1.0f);
	p3 = XMVectorSet(10.0f, 0.0f, 0.0f, 1.0f);

	currentTime = 0.0f;
	t = 0.0f;

	segment = 0.0f;
	iSegment = 0;
	tParam = 0.0f;

	splineCamPos = XMVectorSet(0.0, 0.0, 0.0,1.0);

}

CameraSpline::~CameraSpline()
{
}


XMFLOAT3 CameraSpline::CamUpdate()
{

	// Increment the current time
	currentTime += timeDelta;

	// Wrap the time around to keep it within the range [0, 1)
	if (currentTime >= 1.0f)
	{
		currentTime = 0.0f;
	}

	// update camera to spline pos
	//splineCamPos = catmullRomSpline(currentTime);

	splineCamPos = XMVectorCatmullRom(p0, p1, p2, p3, currentTime);
	XMFLOAT3 fSplinePos = XMFLOAT3(0.0, 0.0, 0.0);

	XMStoreFloat3(&fSplinePos, splineCamPos);
	fSplinePos = XMFLOAT3(fSplinePos.x - 5, fSplinePos.y + 5, fSplinePos.z);
	return fSplinePos;
}

XMFLOAT3 CameraSpline::AIUpdate()
{

	// Increment the current time
	currentTime += timeDelta;

	// Wrap the time around to keep it within the range [0, 1)
	if (currentTime >= 1.0f)
	{
		currentTime = 0.0f;
	}

	// update camera to spline pos
	//splineCamPos = catmullRomSpline(currentTime);

	splineAIPos = XMVectorCatmullRom(p0, p1, p2, p3, currentTime);
	XMFLOAT3 fSplinePos = XMFLOAT3(0.0, 0.0, 0.0);
	
	XMStoreFloat3(&fSplinePos, splineAIPos);
	
	return fSplinePos;
}