#pragma once
#include "pch.h"
#include "InputCommands.h"
using namespace DirectX;


class ArcballCamera
{
public: 

	ArcballCamera();

	void setCameraParams(const XMFLOAT3& eye, const XMFLOAT3& target, const XMFLOAT3& up);

	void Update(InputCommands* m_InputCommands, float scale);

	XMMATRIX getViewMatrix();

	void setEye(XMVECTOR pos) { 
		XMVECTOR m_pos = XMLoadFloat3(&m_eye); 
		m_pos = pos;
	}
	XMVECTOR getEye() { return XMVectorSet(m_eye.x, m_eye.y, m_eye.z, 1.0); }

	template <typename T>
	const T& clamp(const T& value, const T& minValue, const T& maxValue)
	{
		return value < minValue ? minValue : (value > maxValue ? maxValue : value);
	}

private:
	// camera properties
	XMFLOAT3 m_eye;
	XMFLOAT3 m_target;
	XMFLOAT3 m_up;

	DirectX::SimpleMath::Matrix        m_view;

	// camera state
	float m_camOrientationX;	// rotation angle around the y-axis
	float m_camOrientationY;	// rotation angle around the x-axis
	float m_radius;	// Distance from eye to target

};

