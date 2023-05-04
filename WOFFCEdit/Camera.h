#pragma once
#include "pch.h"
#include "InputCommands.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera
{
public:
	Camera();
	~Camera();

	void update(InputCommands* m_InputCommands);

	DirectX::SimpleMath::Vector3 GetOrientation() { return m_camOrientation; }
	DirectX::SimpleMath::Matrix GetViewMatrix() { return m_view; }

	

	DirectX::SimpleMath::Matrix        m_view;

	//camera
	DirectX::SimpleMath::Vector3		m_camPosition;
	DirectX::SimpleMath::Vector3		m_camOrientation;
	DirectX::SimpleMath::Vector3		m_camLookAt;
	DirectX::SimpleMath::Vector3		m_camLookDirection;
	DirectX::SimpleMath::Vector3		m_camRight;
	float m_camRotRate;
	float m_movespeed;

	XMVECTOR getCamPos(){ return XMVectorSet(m_camPosition.x, m_camPosition.y, m_camPosition.z, 1.0f); }
};
