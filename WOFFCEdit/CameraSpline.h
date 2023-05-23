#pragma once
//#include "DisplayObject.h"
#include "pch.h"


using namespace DirectX;

class CameraSpline
{
private:

	

	XMFLOAT3 a1, a2, a3;

	XMFLOAT3 b1, b2;

	XMFLOAT3 c;

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

	XMFLOAT3 catmullRomSpline(float time);

	XMFLOAT3 AIUpdate();
	XMFLOAT3 CamUpdate();

	XMVECTOR p0, p1, p2, p3;
	//XMFLOAT3 p0, p1, p2, p3;
};

