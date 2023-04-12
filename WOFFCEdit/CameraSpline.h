#pragma once
//#include "DisplayObject.h"
#include "pch.h"


using namespace DirectX;

class CameraSpline
{
private:

	XMVECTOR p0, p1, p2, p3;

	XMVECTOR a1, a2, a3;

	XMVECTOR b1, b2;

	XMVECTOR c;

	// Determine timings
	const float timeDelta = 0.001f;;
	float currentTime;
	float t;

	// Determine segments
	float segment;
	int iSegment;
	float tParam;

	XMFLOAT3 splineCamPos;


public:
	CameraSpline();
	~CameraSpline();

	XMFLOAT3 catmullRomSpline(float time);

	XMFLOAT3 update();

};

