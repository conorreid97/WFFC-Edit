#include "ArcballCamera.h"

ArcballCamera::ArcballCamera()
{
	m_eye = XMFLOAT3(0.0, 30.7, -3.5);
	m_target = XMFLOAT3(0.0, 3.7, -1.0);
	m_up = XMFLOAT3(0.0, 1.0, 0.0);
	m_camOrientationX = float(500.0);
	m_camOrientationY = float(100.0);
	m_radius = float(5.0);
}

void ArcballCamera::setCameraParams(const XMFLOAT3& eye, const XMFLOAT3& target, const XMFLOAT3& up)
{
	m_eye = eye;
	m_target = target;
	m_up = up;
}

void ArcballCamera::Update(InputCommands* m_InputCommands)
{
	// update camera position based on rotation angles
	float sinPhi = sinf(m_camOrientationY);
	float cosPhi = cosf(m_camOrientationY);
	float sinTheta = sinf(m_camOrientationX);
	float cosTheta = cosf(m_camOrientationX);
	
	if (m_InputCommands->mouse_RB_Down) {
		if (m_InputCommands->mouse_X < m_InputCommands->prev_mouse_X) {
			m_camOrientationX -= 0.05f;//m_InputCommands->mouse_X /*deltaX*/ * 0.0001f;
			m_camOrientationY = clamp(m_camOrientationY, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);
		}
		else if (m_InputCommands->mouse_X > m_InputCommands->prev_mouse_X) {
			m_camOrientationX += 0.05f;//m_InputCommands->mouse_X /*deltaX*/ * 0.0001f;
			m_camOrientationY = clamp(m_camOrientationY, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);
		}
		if (m_InputCommands->mouse_Y < m_InputCommands->prev_mouse_Y) {
			m_camOrientationY -= 0.05f;//m_InputCommands->mouse_Y /*deltaY * */ * 0.0001f;
			m_camOrientationX = clamp(m_camOrientationX, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);
		}
		else if (m_InputCommands->mouse_Y > m_InputCommands->prev_mouse_Y) {
			m_camOrientationY += 0.05f;// m_InputCommands->mouse_Y /*deltaY * */ * 0.0001f;
			m_camOrientationX = clamp(m_camOrientationX, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);
		}
		
	}
	// Clamp the rotation angles to prevent camera flipping
	//m_camOrientationY = clamp(m_camOrientationY, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);


	XMFLOAT3 eye;
	eye = XMFLOAT3(0.0, 3.7, -3.5);
	XMFLOAT3 target = XMFLOAT3(0.0, 0.0, 0.0);
	
	eye.x = m_target.x + m_radius * sinPhi * cosTheta;
	eye.y = m_target.y + m_radius * cosPhi;
	eye.z = m_target.z + m_radius * sinPhi * sinTheta;

	m_eye = eye;
}

XMMATRIX ArcballCamera::getViewMatrix()
{
	XMVECTOR eyePos = XMLoadFloat3(&m_eye);
	XMVECTOR targetPos = XMLoadFloat3(&m_target);
	XMVECTOR upDir = XMLoadFloat3(&m_up);


	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(eyePos, targetPos, upDir);
	return m_view;/*XMMatrixLookAtLH(eyePos, targetPos, upDir)*/

}
