#include "MFCMain.h"
#include "resource.h"

HINSTANCE hInst;

BEGIN_MESSAGE_MAP(MFCMain, CWinApp)
	ON_COMMAND(ID_FILE_QUIT,	&MFCMain::MenuFileQuit)
	ON_COMMAND(ID_FILE_SAVETERRAIN, &MFCMain::MenuFileSaveTerrain)
	ON_COMMAND(ID_EDIT_SELECT, &MFCMain::MenuEditSelect)
	ON_COMMAND(ID_EDIT_OBJECTMANIPULATION, &MFCMain::MenuObjectManip)
	ON_COMMAND(ID_BUTTON40001,	&MFCMain::ToolBarButton1)
	ON_COMMAND(ID_Button40006, &MFCMain::ToolBarButton2)
	ON_COMMAND(ID_BUTTON40007, &MFCMain::ToolBarButton3)
	ON_COMMAND(ID_BUTTON40008, &MFCMain::ToolBarButton4)
	ON_COMMAND(ID_BUTTON40009, &MFCMain::ToolBarButton5)
	ON_COMMAND(ID_BUTTON40010, &MFCMain::ToolBarButton6)
	ON_COMMAND(ID_BUTTON40011, &MFCMain::ToolBarButton7)
	ON_COMMAND(ID_BUTTON40012, &MFCMain::ToolBarButton8)
	ON_COMMAND(ID_BUTTON40013, &MFCMain::ToolBarButton9)
	ON_COMMAND(ID_BUTTON40014, &MFCMain::ToolBarButton10)

	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TOOL, &CMyFrame::OnUpdatePage)
END_MESSAGE_MAP()

BOOL MFCMain::InitInstance()
{
	//instanciate the mfc frame
	m_frame = new CMyFrame();
	m_pMainWnd = m_frame;

	RECT screenSize;
	const HWND window = GetDesktopWindow();
	GetWindowRect(window, &screenSize);


	m_frame->Create(	NULL,
					_T("World Of Flim-Flam Craft Editor"),
					WS_OVERLAPPEDWINDOW,
					CRect(0, 0, screenSize.right, screenSize.bottom),
					NULL,
					NULL,
					0,
					NULL
				);

	//show and set the window to run and update. 
	m_frame->ShowWindow(SW_SHOW);
	m_frame->UpdateWindow();


	//get the rect from the MFC window so we can get its dimensions
	m_toolHandle = m_frame->m_DirXView.GetSafeHwnd();				//handle of directX child window
	m_frame->m_DirXView.GetClientRect(&WindowRECT);
	m_width		= WindowRECT.Width();
	m_height	= WindowRECT.Height();

	m_ToolSystem.onActionInitialise(m_toolHandle, m_width, m_height);

	return TRUE;
}

int MFCMain::Run()
{
	MSG msg;
	BOOL bGotMsg;

	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		if (true)
		{
			bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		}
		else
		{
			bGotMsg = (GetMessage(&msg, NULL, 0U, 0U) != 0);
		}

		if (bGotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			m_ToolSystem.UpdateInput(&msg);
		}
		else
		{	
			int ID = m_ToolSystem.getCurrentSelectionID();
			std::wstring statusString = L"Selected Object: " + std::to_wstring(ID);
			m_ToolSystem.Tick(&msg);

			//send current object ID to status bar in The main frame
			m_frame->m_wndStatusBar.SetPaneText(1, statusString.c_str(), 1);	
		}
	}

	return (int)msg.wParam;
}

void MFCMain::MenuFileQuit()
{
	//will post message to the message thread that will exit the application normally
	PostQuitMessage(0);
}

void MFCMain::MenuFileSaveTerrain()
{
	m_ToolSystem.onActionSaveTerrain();
}

void MFCMain::MenuEditSelect()
{
	//SelectDialogue m_ToolSelectDialogue(NULL, &m_ToolSystem.m_sceneGraph);		//create our dialoguebox //modal constructor
	//m_ToolSelectDialogue.DoModal();	// start it up modal

	//modeless dialogue must be declared in the class.   If we do local it will go out of scope instantly and destroy itself
	m_ToolSelectDialogue.Create(IDD_DIALOG1);	//Start up modeless
	m_ToolSelectDialogue.ShowWindow(SW_SHOW);	//show modeless
	m_ToolSelectDialogue.SetObjectData(&m_ToolSystem.m_sceneGraph, &m_ToolSystem.m_selectedObject);
}

void MFCMain::MenuObjectManip()
{
	if (m_ToolSystem.m_selectedObject > -1) {
		m_ToolObjectManipDialogue.SetObjectData(&m_ToolSystem.m_sceneGraph, &m_ToolSystem.m_sceneAINodes, &m_ToolSystem.m_scenePathNodes, &m_ToolSystem.m_selectedObject, &m_ToolSystem.m_toolInputCommands);
		m_ToolObjectManipDialogue.Create(IDD_DIALOG2);	//Start up modeless
		m_ToolObjectManipDialogue.ShowWindow(SW_SHOW);	//show modeless
	}
	else {
		MessageBox(m_toolHandle, L"Make sure to select an object before opening the inspector.", L"Error", MB_OK);
	}
}

void MFCMain::ToolBarButton1()
{
	m_ToolSystem.onActionSave();
	m_ToolSystem.onActionSaveTerrain();
	//m_ToolSystem.onActivateCamSpline();
}

// spline cam button
void MFCMain::ToolBarButton2()
{
	// this is going to switch to spline cam
	if (m_ToolSystem.getCamType() == 1) {
		m_ToolSystem.onCamSpline();
	}
	else {
		m_ToolSystem.setCamType(1);
	}
}

// AI Spline
void MFCMain::ToolBarButton3()
{
	m_ToolSystem.onActivateCamSpline();
}
// focus
void MFCMain::ToolBarButton4()
{
	if (!m_ToolSystem.bFocus)
	{
		m_ToolSystem.bFocus = true;
		//m_ToolSystem.setCamType(2);
		m_ToolSystem.onActionFocusCamera();
	}
	else {
		m_ToolSystem.bFocus = false;
		m_ToolSystem.setCamType(1);

	}
}
// scaling
void MFCMain::ToolBarButton5()
{
	m_ToolSystem.onActivateScaling();
	
}
// moving
void MFCMain::ToolBarButton6()
{
	m_ToolSystem.onActivateMove();
	
}
//rotate
void MFCMain::ToolBarButton7()
{
	m_ToolSystem.onActivateRotate();
	
}
//terrain
void MFCMain::ToolBarButton8()
{
	m_ToolSystem.onActivateTerrainEdit();
}
// arcball cam
void MFCMain::ToolBarButton9()
{
	if (m_ToolSystem.getCamType() == 1) {
		m_ToolSystem.onArcBall();
	}
	else {
		m_ToolSystem.setCamType(1);
	}
}
// wireframe
void MFCMain::ToolBarButton10()
{
	if (!m_ToolSystem.bWireframe) {
		m_ToolSystem.bWireframe = true;
	}
	else {
		m_ToolSystem.bWireframe = false;
	}
}


MFCMain::MFCMain()
{
}


MFCMain::~MFCMain()
{
}
