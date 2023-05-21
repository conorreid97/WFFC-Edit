#include "Camera.h"

Camera::Camera()
{
	m_movespeed = 0.30;
	m_camRotRate = 2.0;

	//camera
	m_camPosition.x = 0.0f;
	m_camPosition.y = 3.7f;
	m_camPosition.z = -3.5f;

	m_camOrientation.x = 0;
	m_camOrientation.y = 0;
	m_camOrientation.z = 0;

	m_camLookAt.x = 0.0f;
	m_camLookAt.y = 0.0f;
	m_camLookAt.z = 0.0f;

	m_camLookDirection.x = 0.0f;
	m_camLookDirection.y = 0.0f;
	m_camLookDirection.z = 0.0f;

	m_camRight.x = 0.0f;
	m_camRight.y = 0.0f;
	m_camRight.z = 0.0f;

	m_camOrientation.x = 0.0f;
	m_camOrientation.y = 0.0f;
	m_camOrientation.z = 0.0f;

	//bCamPath = false;
	//m_LerpRemain = 0.0;
}

Camera::~Camera()
{
}

void Camera::update(InputCommands* m_InputCommands, DX::StepTimer const& t)
{
	//TODO  any more complex than this, and the camera should be abstracted out to somewhere else
//camera motion is on a plane, so kill the 7 component of the look direction
	Vector3 planarMotionVector = m_camLookDirection;
	planarMotionVector.y = 0.0;
	inputCommands = m_InputCommands;
	if (m_LerpRemain > 0) {
		//m_InputCommands->finishedLerp = false;
		Lerp(t);
	}
	/*else {
		m_InputCommands->finishedLerp = true;
	}*/

	if (m_camPosition.x >= m_Dest.x) {
		m_InputCommands->finishedLerp = true;
	}

	if (m_InputCommands->rotRight)
	{
		m_camPosition.y -= (m_camRotRate / 3);
	}
	if (m_InputCommands->rotLeft)
	{
		m_camPosition.y += (m_camRotRate / 3);
	}

	if (m_InputCommands->mouse_RB_Down && m_InputCommands->drag) {
		if (m_InputCommands->mouse_X < m_InputCommands->prev_mouse_X) {
			m_camOrientation.y -= m_camRotRate;
		}
		else if (m_InputCommands->mouse_X > m_InputCommands->prev_mouse_X) {
			m_camOrientation.y += m_camRotRate;
		}
		else if (m_InputCommands->mouse_Y > m_InputCommands->prev_mouse_Y) {
			m_camOrientation.x -= m_camRotRate * 2;
		}
		else if (m_InputCommands->mouse_Y < m_InputCommands->prev_mouse_Y) {
			m_camOrientation.x += m_camRotRate *2;
		}
	}

	// lock the camera when reaches the bottom and top
	if (m_camOrientation.x <= -90) {
		m_camOrientation.x = -90;
	}
	if (m_camOrientation.x >= 90) {
		m_camOrientation.x = 90;
	}

	// parametric equations of a sphere
	m_camLookDirection.x = cos((m_camOrientation.y) * 3.1415 / 180) * cos((m_camOrientation.x) * 3.1415 / 180);
	m_camLookDirection.y = sin((m_camOrientation.x) * 3.1415 / 180);
	m_camLookDirection.z = sin((m_camOrientation.y) * 3.1415 / 180) * cos((m_camOrientation.x) * 3.1415 / 180);
	m_camLookDirection.Normalize();


	//create right vector from look Direction
	m_camLookDirection.Cross(Vector3::UnitY, m_camRight);

	//process input and update stuff
	if (m_InputCommands->forward)
	{
		m_camPosition += m_camLookDirection * m_movespeed;
	}
	if (m_InputCommands->back)
	{
		m_camPosition -= m_camLookDirection * m_movespeed;
	}
	if (m_InputCommands->right)
	{
		m_camPosition += m_camRight * m_movespeed;
	}
	if (m_InputCommands->left)
	{
		m_camPosition -= m_camRight * m_movespeed;
	}

	//update lookat point
	m_camLookAt = m_camPosition + m_camLookDirection;

	//m_camPosition.x = m_camLookAt.x + 5.0f;

	//apply camera vectors
	m_view = Matrix::CreateLookAt(m_camPosition, m_camLookAt, Vector3::UnitY);

}


void Camera::Lerp(DX::StepTimer const& t)
{
	m_LerpRemain -= t.GetElapsedSeconds();
	float lerpFac = (m_Lerp - m_LerpRemain) / m_Lerp;
	m_camPosition = Vector3::Lerp(m_Origin, m_Dest, lerpFac);
	
}

void Camera::Focus(XMFLOAT3 pos, XMFLOAT3 scale, int id, HWND toolHandle)
{
	if (id != -1) {
		m_LerpRemain = 0.5;
		m_Lerp = 0.5;
		m_camOrientation = Vector3(-30, 0, 0);
		m_Dest = pos - (XMFLOAT3(2.5, -2, 0) * scale);
		m_Origin = m_camPosition;
	}
	else {
		MessageBox(toolHandle, L"Make sure to select an object before opening the inspector.", L"Error", MB_OK);

	}

}




