//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "DisplayObject.h"
#include <cmath>
#include <string>


using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

XMVECTOR controlPoints[] = {
	XMVectorSet(-1.0f, 0.0f, 0.0f, 1.0f),
	XMVectorSet(-0.5f, 0.5f, 0.0f, 1.0f),
	XMVectorSet(0.5f, -0.5f, 0.0f, 1.0f),
	XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f)
};

Game::Game()

{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
	m_displayList.clear();
	
	//initial Settings
	//modes
	m_grid = false;
	m_rebuildDisplayList = false;
	//functional
	m_movespeed = 0.30;

	cameraType = 2;
	bCamPath = false;
	camView = cam1.GetViewMatrix();
	

	bFog = true;
	
	switchCam = false;
	currentCamPos = XMVectorSet(0.0, 0.0, 0.0, 1.0);
	targetPos = XMVectorSet(0.0, 0.0, 0.0, 1.0);

	bWireframe = false;
	m_prevSelected = -1;
	// arcball lerp
	m_LerpRemain = 0.5;
	m_ArcBallLerp = 0.5;
	arcballpos = XMFLOAT3(0.0, 0.0, 0.0);
	scale = 0.0f;
}

Game::~Game()
{

#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	GetClientRect(window, &m_ScreenDimensions);

	

#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif

	// set my variables
	bSelected = false;
	selectedID = -1;
	for (int i = 0; i < 15; i++) {
		selectedID_List.push_back(-1);
		vectorVictor.push_back(XMFLOAT3(0.0,0.0,0.0));
	}

}

void Game::SetGridState(bool state)
{
	m_grid = state;
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick(InputCommands *Input, std::vector<SceneObject>* SceneGraph)
{
	//copy over the input commands so we have a local version to use elsewhere.
	m_InputCommands = *Input;
    m_timer.Tick([&]()
    {
        Update(m_timer, SceneGraph);
    });

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

    Render();

}

// Updates the world.
void Game::Update(DX::StepTimer const& timer, std::vector<SceneObject>* SceneGraph)
{
	if (cameraType == 1) {
		camView = cam1.GetViewMatrix();
		cam1.update(&m_InputCommands, timer);
	}
	else if (cameraType == 2) {
		
		camView = arcBallCam.getViewMatrix();
		arcBallCam.Update(&m_InputCommands, scale);
	}
	else if (cameraType == 3) {
		camView = cam2.GetViewMatrix();
		cam2.update(&m_InputCommands, timer);
	}
	if (m_InputCommands.tool == TerrainEdit) {
		TerrainHighlight();
	}

	

   // m_batchEffect->SetView(cam1.m_view);
	m_batchEffect->SetView(camView);
    m_batchEffect->SetWorld(Matrix::Identity);
	//m_displayChunk.m_terrainEffect->SetView(cam1.m_view);
	m_displayChunk.m_terrainEffect->SetView(camView);
	m_displayChunk.m_terrainEffect->SetWorld(Matrix::Identity);

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

   // reset selected list
	if (!m_InputCommands.multiSelect)
	{
		for (int i = 15; i < selectedID_List.size(); i++) {
			selectedID_List.pop_back();
		}
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}

	Clear();

	m_deviceResources->PIXBeginEvent(L"Render");
	auto context = m_deviceResources->GetD3DDeviceContext();

	if (m_grid)
	{
		// Draw procedurally generated dynamic grid
		const XMVECTORF32 xaxis = { 512.f, 0.f, 0.f };
		const XMVECTORF32 yaxis = { 0.f, 0.f, 512.f };
		DrawGrid(xaxis, yaxis, g_XMZero, 512, 512, Colors::Gray);
	}

	//CAMERA POSITION ON HUD
	m_sprites->Begin();
	WCHAR   Buffer[256];
	std::wstring varPos = L"Cam X: " + std::to_wstring(cam1.m_camPosition.x) + L"Cam Y: " + std::to_wstring(cam1.m_camPosition.y) + L"Cam Z: " + std::to_wstring(cam1.m_camPosition.z);
	m_font->DrawString(m_sprites.get(), varPos.c_str(), XMFLOAT2(100, 10), Colors::Yellow);

	std::wstring varRot = L"Cam X Rot: " + std::to_wstring(cam1.m_camOrientation.x) + L"Cam Y Rot: " + std::to_wstring(cam1.m_camOrientation.y) + L"Cam Z Rot: " + std::to_wstring(cam1.m_camOrientation.z);
	m_font->DrawString(m_sprites.get(), varRot.c_str(), XMFLOAT2(100, 50), Colors::Yellow);

	m_sprites->End();

	//RENDER OBJECTS FROM SCENEGRAPH
	int numRenderObjects = m_displayList.size();
	for (int i = 0; i < numRenderObjects; i++)
	{
		m_deviceResources->PIXBeginEvent(L"Draw model");
		const XMVECTORF32 scale = { m_displayList[i].m_scale.x, m_displayList[i].m_scale.y, m_displayList[i].m_scale.z };
		const XMVECTORF32 translate = { m_displayList[i].m_position.x, m_displayList[i].m_position.y, m_displayList[i].m_position.z };

		//convert degrees into radians for rotation matrix
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y * 3.1415 / 180,
			m_displayList[i].m_orientation.x * 3.1415 / 180,
			m_displayList[i].m_orientation.z * 3.1415 / 180);

		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		if (bWireframe) {
			m_displayList[i].m_model->Draw(context, *m_states, local, camView, m_projection, true);	//last variable in draw,  make TRUE for wireframe

		}
		else {
			m_displayList[i].m_model->Draw(context, *m_states, local, camView, m_projection, m_displayList[i].m_wireframe);	//last variable in draw,  make TRUE for wireframe

		}
		m_deviceResources->PIXEndEvent();
	}
	m_deviceResources->PIXEndEvent();

	//RENDER TERRAIN
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullNone());
	
	if (bWireframe) {
		context->RSSetState(m_states->Wireframe());		
	}
	//Render the batch,  This is handled in the Display chunk becuase it has the potential to get complex
	m_displayChunk.RenderBatch(m_deviceResources);

	m_deviceResources->Present();


}


// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void XM_CALLCONV Game::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
    m_deviceResources->PIXBeginEvent(L"Draw grid");

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullCounterClockwise());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        float fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();

    m_deviceResources->PIXEndEvent();
}
void Game::SetScreenDim(RECT in)
{
	m_ScreenDimensions = in;
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::BuildDisplayList(std::vector<SceneObject> * SceneGraph)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();

	if (!m_displayList.empty())		//is the vector empty
	{
		m_displayList.clear();		//if not, empty it
	}

	//for every item in the scenegraph
	int numObjects = SceneGraph->size();
	for (int i = 0; i < numObjects; i++)
	{
		//create a temp display object that we will populate then append to the display list.
		DisplayObject newDisplayObject;
		
		//load model
		std::wstring modelwstr = StringToWCHART(SceneGraph->at(i).model_path);							//convect string to Wchar
		newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str(), *m_fxFactory, true);	//get DXSDK to load model "False" for LH coordinate system (maya)

		//Load Texture
		std::wstring texturewstr = StringToWCHART(SceneGraph->at(i).tex_diffuse_path);								//convect string to Wchar
		HRESULT rs;
		rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

		//if texture fails.  load error default
		if (rs)
		{
			CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
		}

		//apply new texture to models effect
		newDisplayObject.m_model->UpdateEffects([&](IEffect* effect) //This uses a Lambda function,  if you dont understand it: Look it up.
		{	
			auto lights = dynamic_cast<BasicEffect*>(effect);
			if (lights)
			{
				lights->SetTexture(newDisplayObject.m_texture_diffuse);			
			}
		});

		newDisplayObject.m_ID = SceneGraph->at(i).ID;
		
		//set position
		newDisplayObject.m_position.x = SceneGraph->at(i).posX;
		newDisplayObject.m_position.y = SceneGraph->at(i).posY;
		newDisplayObject.m_position.z = SceneGraph->at(i).posZ;
		
		//setorientation
		newDisplayObject.m_orientation.x = SceneGraph->at(i).rotX;
		newDisplayObject.m_orientation.y = SceneGraph->at(i).rotY;
		newDisplayObject.m_orientation.z = SceneGraph->at(i).rotZ;

		//set scale
		newDisplayObject.m_scale.x = SceneGraph->at(i).scaX;
		newDisplayObject.m_scale.y = SceneGraph->at(i).scaY;
		newDisplayObject.m_scale.z = SceneGraph->at(i).scaZ;

		//set wireframe / render flags
		newDisplayObject.m_render		= SceneGraph->at(i).editor_render;
		newDisplayObject.m_wireframe	= SceneGraph->at(i).editor_wireframe;

		newDisplayObject.m_light_type		= SceneGraph->at(i).light_type;
		newDisplayObject.m_light_diffuse_r	= SceneGraph->at(i).light_diffuse_r;
		newDisplayObject.m_light_diffuse_g	= SceneGraph->at(i).light_diffuse_g;
		newDisplayObject.m_light_diffuse_b	= SceneGraph->at(i).light_diffuse_b;
		newDisplayObject.m_light_specular_r = SceneGraph->at(i).light_specular_r;
		newDisplayObject.m_light_specular_g = SceneGraph->at(i).light_specular_g;
		newDisplayObject.m_light_specular_b = SceneGraph->at(i).light_specular_b;
		newDisplayObject.m_light_spot_cutoff = SceneGraph->at(i).light_spot_cutoff;
		newDisplayObject.m_light_constant	= SceneGraph->at(i).light_constant;
		newDisplayObject.m_light_linear		= SceneGraph->at(i).light_linear;
		newDisplayObject.m_light_quadratic	= SceneGraph->at(i).light_quadratic;
		
		m_displayList.push_back(newDisplayObject);

		
		
		// Colour the Start and End of the AI Path
		if (SceneGraph->at(i).path_node_start || SceneGraph->at(i).path_node_end) {
			DisplayObject  selectedHighlight = ColourObject(newDisplayObject, Colors::Yellow);
		}
		// Colour the control points for the AI Path
		if (SceneGraph->at(i).path_node) {
			DisplayObject  selectedHighlight = ColourObject(newDisplayObject, Colors::OrangeRed);
		}
		if (SceneGraph->at(i).AINode) {
			DisplayObject  selectedHighlight = ColourObject(newDisplayObject, Colors::Coral);
		}
		if (SceneGraph->at(i).camera) {
			DisplayObject selectedHighlight = ColourObject(newDisplayObject, Colors::Green);
			cam2.m_camPosition = XMFLOAT3(SceneGraph->at(i).posX - 5, SceneGraph->at(i).posY + 2, SceneGraph->at(i).posZ);
			
			
		}
		// Colour the selected object
		if (SceneGraph->at(i).ID == selectedID) {
			
			DisplayObject  selectedHighlight = ColourObject(newDisplayObject, Colors::DarkCyan);;

			m_displayList.push_back(selectedHighlight);
		}
	}	
	m_rebuildDisplayList = false;
}

DisplayObject Game::ColourObject(DisplayObject object, XMVECTOR colour ) {
	DisplayObject  selectedHighlight = object;

	selectedHighlight.m_ID = -1;
	selectedHighlight.m_wireframe = true;

	// highlight object
	selectedHighlight.m_model->UpdateEffects([&](IEffect* effect)
		{
			auto fog = dynamic_cast<IEffectFog*>(effect);
			if (fog)
			{
				fog->SetFogEnabled(true);
				fog->SetFogStart(0);
				fog->SetFogEnd(0);
				fog->SetFogColor(colour);
			}
		});
	return selectedHighlight;
}

void Game::BuildDisplayChunk(ChunkObject * SceneChunk)
{
	//populate our local DISPLAYCHUNK with all the chunk info we need from the object stored in toolmain
	//which, to be honest, is almost all of it. Its mostly rendering related info so...
	m_displayChunk.PopulateChunkData(SceneChunk);		//migrate chunk data
	m_displayChunk.LoadHeightMap(m_deviceResources);
	m_displayChunk.m_terrainEffect->SetProjection(m_projection);
	m_displayChunk.InitialiseBatch();
}

void Game::SaveDisplayChunk(ChunkObject * SceneChunk)
{
	m_displayChunk.SaveHeightMap();			//save heightmap to file.
}

void Game::Lerp(DX::StepTimer const& t)
{
	/////Lerp
	// get destination of selected object
	//decrease the lerp time remaining
	// 
	// ATTENTION
	// 
	/////////
	XMVECTOR dest = m_displayList.at(selectedID).m_position;
	m_LerpRemain -= t.GetElapsedSeconds();
	float lerp = (m_ArcBallLerp - m_LerpRemain) / m_LerpRemain;
	XMVectorLerp(XMVectorSet(0.0,0.0,0.0,1.0), dest, lerp);
}

void Game::ColourControlPoint(int id)
{
	/////// Colour Control Points
	//
	///////

	DisplayObject newDisplayObject;
	newDisplayObject = m_displayList.at(id);
	DisplayObject  selectedHighlight = newDisplayObject;

	selectedHighlight.m_ID = -1;
	selectedHighlight.m_wireframe = true;

	// highlight object
	selectedHighlight.m_model->UpdateEffects([&](IEffect* effect)
		{
			auto fog = dynamic_cast<IEffectFog*>(effect);
			if (fog)
			{
				fog->SetFogEnabled(true);
				fog->SetFogStart(0);
				fog->SetFogEnd(0);
				fog->SetFogColor(Colors::IndianRed);
			}
		});
}

void Game::CamSplineTool()
{
	if (!bCamPath) {

		bCamPath = true;
	}
	else {
		bCamPath = false;
	}
}

void Game::FocusTool()
{
	// switch to arcball
	cameraType = 2;
	m_LerpRemain = 0.5;
	m_ArcBallLerp = 0.5;

	currentCamPos = cam1.getCamPos();
	targetPos = m_displayList[selectedID].m_position;

	switchCam = true;

	Vector3 targetOffset = Vector3(m_displayList[selectedID].m_position.x, m_displayList[selectedID].m_position.y, m_displayList[selectedID].m_position.z + 10);
	
	
	XMStoreFloat3(&arcballpos, arcBallCam.getEye());
	
	arcBallCam.setCameraParams(XMFLOAT3(0.0, 3.7, -3.5), m_displayList[selectedID].m_position, XMFLOAT3(0.0, 1.0, 0.0));

	
}

void Game::TerrainHighlight()
{
	//reset highlighted
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			m_displayChunk.m_bHighlight[i][j] = false;
		}
	}

	//intersection point and bool to check if we have an intersection
	Vector3 IntersectionPoint;
	bool intersection = false;

	//setup near and far planes of frustum with mouse X and mouse y passed down from Toolmain.
	const XMVECTOR nearSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 1.0f, 1.0f);

	//Unproject the points on the near and far plane
	const XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, camView, m_world);
	const XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, camView, m_world);

	//get the line cast from the mouse
	const XMVECTOR lineCast = XMVector3Normalize(farPoint - nearPoint);

	//loop through quads to check for line intersection
	for (size_t i = 0; i < TERRAINRESOLUTION - 1; i++)
	{
		if (intersection)
			break;
		for (size_t j = 0; j < TERRAINRESOLUTION - 1; j++)
		{
			XMVECTOR v1 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i][j].position);
			XMVECTOR v2 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i][j + 1].position);
			XMVECTOR v3 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i + 1][j + 1].position);
			XMVECTOR v4 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i + 1][j].position);

			//get plane from vertices
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(v2 - v1, v3 - v1));
			float d = -XMVectorGetX(XMVector3Dot(normal, v1));
			XMVECTOR plane = XMVectorSetW(normal, d);

			//get intersection point
			XMVECTOR intersects = XMPlaneIntersectLine(plane, nearPoint, farPoint);

			if (!XMVector3Equal(intersects, XMVectorZero()))
			{
				//convert intersection point to vector3
				Vector3 point;
				XMStoreFloat3(&point, intersects);

				// check if the point is inside the quad
				if (point.x >= std::min(XMVectorGetX(v1), XMVectorGetX(v2)) && point.x <= std::max(XMVectorGetX(v1), XMVectorGetX(v2)) &&
					point.z >= std::min(XMVectorGetZ(v1), XMVectorGetZ(v4)) && point.z <= std::max(XMVectorGetZ(v1), XMVectorGetZ(v4)))
				{
					//store point of intersection
					IntersectionPoint = point;
					intersection = true;
					break;
				}
			}

		}
	}

	//if line did not intersect terrain, return
	if (!intersection)
		return;

	//loop through vertices and check if they are within a certain radius of the intersection point
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			//get distance between vertex and intersection point (ignoring y axis)
			const float distance = Vector3::Distance(Vector3(IntersectionPoint.x, 0, IntersectionPoint.z), Vector3(m_displayChunk.m_terrainGeometry[i][j].position.x, 0, m_displayChunk.m_terrainGeometry[i][j].position.z));
			const int outerRadius = m_InputCommands.outerRadius;
			const int innerRadius = m_InputCommands.innerRadius;
			if (distance < outerRadius && distance> innerRadius)
			{
				m_displayChunk.m_bHighlight[i][j] = true;
			}
		}
	}
}


void Game::UpdateObjectData(SceneObject* scene, int i)
{
	m_displayList.at(i).m_position = Vector3(scene->posX, scene->posY, scene->posZ);
	m_displayList.at(i).m_orientation = Vector3(scene->rotX, scene->rotY, scene->rotZ);
	m_displayList.at(i).m_scale = Vector3(scene->scaX, scene->scaY, scene->scaZ);
}

void Game::StartTerrainEdit()
{
	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < 128; j++) {
			m_ChunkY[i][j] = m_displayChunk.m_terrainGeometry[i][j].position.y;
		}
	}
}

void Game::EditTerrain()
{
	// point of intersection and bool to check if there is an intersection or not
	Vector3 InterPoint;
	bool bInter = false;

	const XMVECTOR nearSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 1.0f, 1.0f);

	//Unproject the points on the near and far plane
	const XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, camView, m_world);
	const XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, camView, m_world);

	const XMVECTOR rayCast = XMVector3Normalize(farPoint - nearPoint);

	for (size_t i = 0; i < TERRAINRESOLUTION - 1; i++) {
		if (bInter) {
			break;
		}

		for (size_t j = 0; j < TERRAINRESOLUTION - 1; j++) {
			// store the points of the terrain chunk
			XMVECTOR p1 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i][j].position);
			XMVECTOR p2 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i][j + 1].position);
			XMVECTOR p3 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i + 1][j + 1].position);
			XMVECTOR p4 = XMLoadFloat3(&m_displayChunk.m_terrainGeometry[i + 1][j].position);

			// get the plane from the points of the terrain chunk
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(p2 - p1, p3 - p1));
			float d = -XMVectorGetX(XMVector3Dot(normal, p1));
			XMVECTOR plane = XMVectorSetW(normal, d);

			// get the point of intersection
			XMVECTOR intersect = XMPlaneIntersectLine(plane, nearPoint, farPoint);

			if (!XMVector3Equal(intersect, XMVectorZero())) {
				Vector3 point;
				XMStoreFloat3(&point, intersect);

				if (point.x >= std::min(XMVectorGetX(p1), XMVectorGetX(p2)) && point.x <= std::max(XMVectorGetX(p1), XMVectorGetX(p2)) &&
					point.z >= std::min(XMVectorGetZ(p1), XMVectorGetZ(p4)) && point.z <= std::max(XMVectorGetZ(p1), XMVectorGetZ(p4)))
				{
					InterPoint = point;
					bInter = true;
					break;
				}
			}
		}
	}

	if (!bInter) {
		return;
	}

	for (int i = 0; i < TERRAINRESOLUTION; i++) {
		for (int j = 0; j < TERRAINRESOLUTION; j++) {
			// Get the distance between the vertex and point of intersection, but ignore the y axis
			const float dist = Vector3::Distance(Vector3(InterPoint.x, 0, InterPoint.z), 
													Vector3(m_displayChunk.m_terrainGeometry[i][j].position.x, 
														0, m_displayChunk.m_terrainGeometry[i][j].position.z));
			const int outRadius = m_InputCommands.outerRadius;
			const int inRadius = m_InputCommands.innerRadius;
			if (dist < outRadius) {
				if (dist < inRadius) {
					m_displayChunk.m_terrainGeometry[i][j].position.y += 0.25f * m_InputCommands.terrainDir;
				}
				else {
					m_displayChunk.m_terrainGeometry[i][j].position.y += 0.25f * m_InputCommands.terrainDir * (1 - ((dist - inRadius) / 10.f));
				}
				// min the terrain can dig
				if (m_displayChunk.m_terrainGeometry[i][j].position.y < 0) {
					m_displayChunk.m_terrainGeometry[i][j].position.y = 0;
				}
				// max the terrain can go
				else if (m_displayChunk.m_terrainGeometry[i][j].position.y > 64) {
					m_displayChunk.m_terrainGeometry[i][j].position.y = 64;
				}

				// recalculate normals
				m_displayChunk.CalculateTerrainNormal(i, j);
			
			}
		}
	}
}

void Game::EndTerrainEdit()
{
	//std::vector<float> oldChunkYPoint;
	//std::vector<float> newChunkYpoint;

	//for (int i = 0; i < m_pointVec.size(); i++) {
	//	newChunkYpoint.push_back(m_displayChunk.m_terrainGeometry[m_pointVec[i].first][m_pointVec[i].second].position.y);
	//	oldChunkYPoint.push_back(m_ChunkY[m_pointVec[i].first][m_pointVec[i].second]);

	//}

	//TerrainEditCommand
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif


#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    m_fxFactory = std::make_unique<EffectFactory>(device);
	m_fxFactory->SetDirectory(L"database/data/"); //fx Factory will look in the database directory
	m_fxFactory->SetSharing(false);	//we must set this to false otherwise it will share effects based on the initial tex loaded (When the model loads) rather than what we will change them to.

    m_sprites = std::make_unique<SpriteBatch>(context);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
                VertexPositionColor::InputElementCount,
                shaderByteCode, byteCodeLength,
                m_batchInputLayout.ReleaseAndGetAddressOf())
        );
    }

    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");

//    m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

    // SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
    m_model = Model::CreateFromSDKMESH(device, L"tiny.sdkmesh", *m_fxFactory);
	

    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
    );

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
    );

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_batchEffect->SetProjection(m_projection);
	
}

void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_font.reset();
    m_shape.reset();
    m_model.reset();
    m_texture1.Reset();
    m_texture2.Reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

std::wstring StringToWCHART(std::string s)
{

	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

int Game::MousePicking()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	float selectedDistance = INFINITY;
	float closestPick = INFINITY;

	//m_prevSelected = selectedID;
	//selectedID = -1;

	//setup near and far planes of frustum with mouse X and mouse y passed down from Toolmain. 
		//they may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 1.0f, 1.0f);

	//Loop through entire display list of objects and pick with each in turn. 
	for (int i = 0; i < m_displayList.size(); i++)
	{
		//Get the scale factor and translation of the object
		const XMVECTORF32 scale = { m_displayList[i].m_scale.x,		m_displayList[i].m_scale.y,		m_displayList[i].m_scale.z };
		const XMVECTORF32 translate = { m_displayList[i].m_position.x,		m_displayList[i].m_position.y,	m_displayList[i].m_position.z };

		//convert euler angles into a quaternion for the rotation of the object
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y * 3.1415 / 180, m_displayList[i].m_orientation.x * 3.1415 / 180,
			m_displayList[i].m_orientation.z * 3.1415 / 180);

		//create set the matrix of the selected object in the world based on the translation, scale and rotation.
		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		//Unproject the points on the near and far plane, with respect to the matrix we just created.
		XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, camView, local);

		XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, camView, local);

		XMVECTOR pickingVector = farPoint - nearPoint;
		pickingVector = XMVector3Normalize(pickingVector);

		for (size_t j = 0; j < m_displayList[i].m_model.get()->meshes.size(); j++) {
			if (m_displayList[i].m_model.get()->meshes[j]->boundingBox.Intersects(nearPoint, pickingVector, selectedDistance) && m_displayList[i].m_ID != -1)
			{
				if (selectedDistance < closestPick)
				{
					selectedID = m_displayList[i].m_ID;
					closestPick = selectedDistance;
				}
			}
			// Deselect
			else if (m_displayList[i].m_model.get()->meshes[j]->boundingBox.Intersects(nearPoint, pickingVector, selectedDistance) && m_displayList[i].m_ID == -1){
				selectedID = -1;
			}
		
		}
	}

	m_rebuildDisplayList = true;
	//if we got a hit.  return it.  
	return selectedID;
}

void Game::MoveObject()
{

	m_displayList[selectedID].m_scale += Vector3(1.0f, 1.0f, 1.0f);
}

