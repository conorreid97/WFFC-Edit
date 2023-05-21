#pragma once
#include "pch.h"
#include "InputCommands.h"
#include "StepTimer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera
{
public:
	Camera();
	~Camera();

	void update(InputCommands* m_InputCommands, DX::StepTimer const& t);

	DirectX::SimpleMath::Vector3 GetOrientation() { return m_camOrientation; }
	DirectX::SimpleMath::Matrix GetViewMatrix() { return m_view; }

	void Lerp(DX::StepTimer const& t);
	void Focus(XMFLOAT3 pos, XMFLOAT3 scale, int id, HWND toolHandle);
	

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

	// set input commands and set focus in cpp
	InputCommands* inputCommands;
private:
	float m_LerpRemain;
	float m_Lerp;
	DirectX::SimpleMath::Vector3 m_Origin;
	DirectX::SimpleMath::Vector3 m_Dest;
};
