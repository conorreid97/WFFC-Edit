//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SceneObject.h"
#include "DisplayObject.h"
#include "DisplayChunk.h"
#include "ChunkObject.h"
#include "InputCommands.h"
#include "CameraSpline.h"
#include "Camera.h"
#include "ArcballCamera.h"
#include <vector>


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
public:

	Game();
	~Game();

	// Initialization and management
	void Initialize(HWND window, int width, int height);
	void SetGridState(bool state);

	// Basic game loop
	void Tick(InputCommands * Input, std::vector<SceneObject>* SceneGraph);
	void Render();

	// Rendering helpers
	void Clear();

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	// change to return list of ints
	int MousePicking();
	void MoveObject();

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int height);

	//tool specific
	int GetSelectedID() { return selectedID; }
	void SetRebuildDisplayList(bool b) { m_rebuildDisplayList = b; }
	bool GetRebuildDisplayList() { return m_rebuildDisplayList; }
	void BuildDisplayList(std::vector<SceneObject> * SceneGraph); //note vector passed by reference 
	DisplayObject ColourObject(DisplayObject object, XMVECTOR colour); // Colours a given object with a given colour
	void BuildDisplayChunk(ChunkObject *SceneChunk);
	void SaveDisplayChunk(ChunkObject *SceneChunk);	//saves geometry et al
	void Lerp(DX::StepTimer const& t);
	// ai spline
	void ColourControlPoint(int);

	void CamSplineTool();
	void FocusTool();
	void TerrainHighlight();
	void ClearDisplayList();

	void UpdateObjectData(SceneObject* scene, int i);
	// terrain editor
	void StartTerrainEdit();
	void EditTerrain();
	void EndTerrainEdit();
	float m_ChunkY[TERRAINRESOLUTION][TERRAINRESOLUTION];	// create a 2d array the same size as the chunk
	std::vector<std::pair<int, int>> m_pointVec;	// adds edited points to a vector for undo / redo
	RECT		m_ScreenDimensions;
	Camera cam1;
	//Camera getCam1() { return cam1; }

#ifdef DXTK_AUDIO
	void NewAudioDevice();
#endif

	// wireframe mode
	bool bWireframe;

private:

	void Update(DX::StepTimer const& timer, std::vector<SceneObject>* SceneGraph);

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color);

	//tool specific
	std::vector<DisplayObject>			m_displayList;
	DisplayChunk						m_displayChunk;
	InputCommands						m_InputCommands;
	
	// Camera
	
	ArcballCamera arcBallCam;
	float m_LerpRemain;
	float m_ArcBallLerp;
	XMFLOAT3 arcballpos;
	//functionality
	float m_movespeed;
	bool m_rebuildDisplayList;
	int m_prevSelected;
	// Dynamic cameras
	int cameraType;
	XMMATRIX camView;


	


	//control variables
	bool m_grid;							//grid rendering on / off
	// Device resources.
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_shape;
    std::unique_ptr<DirectX::Model>                                         m_model;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture2;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;


	// bool for deselecting
	bool bSelected;
	int selectedID;

	std::vector<int>selectedID_List;
	//depth check
	XMFLOAT3 currentPos;
	XMFLOAT3 SelectedVector;
	XMFLOAT3 prevPos;
	std::vector<XMFLOAT3> vectorVictor;


public:

	// Spline
	CameraSpline camSpline;
	bool bCamPath;
	int getCamType() { return cameraType; }
	void setCamType(int _cam) { cameraType = _cam; }
	void SetScreenDim(RECT in);
	// Object Manipulation

	// Fog
	bool bFog;

	// switch to arcball
	bool switchCam;
	XMVECTOR currentCamPos;
	XMVECTOR targetPos;
};

std::wstring StringToWCHART(std::string s);