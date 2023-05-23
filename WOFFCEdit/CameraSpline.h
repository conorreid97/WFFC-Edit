#pragma once
//#include "DisplayObject.h"
#include "pch.h"


using namespace DirectX;

class CameraSpline
{
private:

	// Determine timings
	const float timeDelta = 0.001f;;
	float currentTime;
	float t;

	// Determine segments
	float segment;
	int iSegment;
	float tParam;

	XMVECTOR splineCamPos;
	XMVECTOR splineAIPos;

public:
	CameraSpline();
	~CameraSpline();

	XMFLOAT3 AIUpdate();
	XMFLOAT3 CamUpdate();

	XMVECTOR p0, p1, p2, p3;
};

