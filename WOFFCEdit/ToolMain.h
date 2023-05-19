#pragma once

#include <afxext.h>
#include "pch.h"
#include "Game.h"
#include "sqlite3.h"
#include "SceneObject.h"
//#include "DirectXHelpers.h"
#include "InputCommands.h"
#include <vector>

//XMVECTOR AIControlPoints[] = {
//	XMVectorSet(-1.0f, 0.0f, 0.0f, 1.0f),
//	XMVectorSet(-0.5f, 0.5f, 0.0f, 1.0f),
//	XMVectorSet(0.5f, -0.5f, 0.0f, 1.0f),
//	XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f)
//};

class ToolMain
{
public: //methods
	ToolMain();
	~ToolMain();

	//onAction - These are the interface to MFC
	int		getCurrentSelectionID();										//returns the selection number of currently selected object so that It can be displayed.
	void	onActionInitialise(HWND handle, int width, int height);			//Passes through handle and hieght and width and initialises DirectX renderer and SQL LITE
	void	onActionFocusCamera();
	void    onActivateCamSpline();
	void	onActivateScaling();
	void	onActivateMove();
	void	onActivateRotate();
	void	onActivateFog();
	void	onActionLoad();													//load the current chunk
	void	onActionDuplicate();
	int		getID(int ID);
	afx_msg	void	onActionSave();											//save the current chunk
	afx_msg void	onActionSaveTerrain();									//save chunk geometry

	void	Tick(MSG *msg);
	void	UpdateInput(MSG *msg);

	void DeleteObject();

	void MouseUpdate();
	void CamSplineUpdate();
	void ObjectUpdate();
	void TerrainUpdate();

	void setCamType(int _cam) { camType = _cam; }

public:	//variables
	std::vector<SceneObject>    m_sceneGraph;	//our scenegraph storing all the objects in the current chunk
	ChunkObject					m_chunk;		//our landscape chunk
	int m_selectedObject;						//ID of current Selection

	// Focus
	bool bFocus;


private:	//methods
	void	onContentAdded();


		
private:	//variables
	HWND	m_toolHandle;		//Handle to the  window
	Game	m_d3dRenderer;		//Instance of D3D rendering system for our tool
	InputCommands m_toolInputCommands;		//input commands that we want to use and possibly pass over to the renderer
	CRect	WindowRECT;		//Window area rectangle. 
	char	m_keyArray[256];
	sqlite3 *m_databaseConnection;	//sqldatabase handle

	int m_width;		//dimensions passed to directX
	int m_height;
	int m_currentChunk;			//the current chunk of thedatabase that we are operating on.  Dictates loading and saving. 
	
	std::vector<int>posVectorX;
	std::vector<int>posVectorY;

	// Splines
	CameraSpline camSpline;
	
	// drag 
	bool bDragging;

	// camera types
	int camType;

	// Object Manipulation
	bool bScaleManip;
	bool bMoveManip;
	bool bRotManip;

	// ai spline
	bool bCamSpline;
	int numSegments = 10;
	std::vector<XMVECTOR> intermediatePoints;
	XMVECTOR p;

	XMFLOAT3 p0;
	XMFLOAT3 p1;
	XMFLOAT3 p2;
	XMFLOAT3 p3;

	XMVECTOR controlPoints[4];

	float pX;
	float pY;
	float pZ;
};
