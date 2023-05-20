#include "stdafx.h"
#include "ToolMain.h"
#include "ObjectManipDialogue.h"


IMPLEMENT_DYNAMIC(ObjectManipDialogue, CDialogEx)

//Message map.  Just like MFCMAIN.cpp.  This is where we catch button presses etc and point them to a handy dandy method.
BEGIN_MESSAGE_MAP(ObjectManipDialogue, CDialogEx)
	//ON_COMMAND(IDOK, &ObjectManip::End)					//ok button
	ON_BN_CLICKED(IDOK, &ObjectManipDialogue::OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST1, &ObjectManipDialogue::Select)	//listbox
	ON_BN_CLICKED(IDCANCEL, &ObjectManipDialogue::OnEnd)

	//ON_EN_CHANGE(IDC_EDIT4, &ObjectManip::OnEnChangeEdit4)
	ON_BN_CLICKED(IDC_CHECK1, &ObjectManipDialogue::OnBnClickedCheck1)
END_MESSAGE_MAP()


ObjectManipDialogue::ObjectManipDialogue(CWnd* pParent, std::vector<SceneObject>* SceneGraph)	:	CDialogEx(IDD_DIALOG2, pParent)
{

	m_sceneGraph = SceneGraph;
}

ObjectManipDialogue::ObjectManipDialogue(CWnd* pParent)	:	CDialogEx(IDD_DIALOG2, pParent)
{
	m_PosX = 0;
	m_PosY = 0;
	m_PosZ = 0;

	m_RotX = 0;
	m_RotY = 0;
	m_RotZ = 0;

	m_ScaleX = 0;
	m_ScaleY = 0;
	m_ScaleZ = 0;

	m_Current = 0;
	m_IsSelected = false;

	triggerAINode = false;
}

ObjectManipDialogue::~ObjectManipDialogue()
{
}

void ObjectManipDialogue::SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<SceneObject>* SceneAINodes, int* Selection, InputCommands* input)
{
	m_sceneGraph = SceneGraph;
	m_currentSelection = Selection - 1;
	m_Inputcommands = input;

	m_sceneAINodes = SceneAINodes;

	m_Current = *Selection - 1;
	//roll through all the objects in the scene graph and put an entry for each in the listbox
	int numSceneObjects = m_sceneGraph->size();
	if (m_Current > -1)
	{
		m_PosX = SceneGraph->at(*Selection - 1).posX;
		m_PosY = SceneGraph->at(*Selection - 1).posY;
		m_PosZ = SceneGraph->at(*Selection - 1).posZ;

		m_RotX = SceneGraph->at(*Selection - 1).rotX;
		m_RotY = SceneGraph->at(*Selection - 1).rotY;
		m_RotZ = SceneGraph->at(*Selection - 1).rotZ;

		m_ScaleX = SceneGraph->at(*Selection - 1).scaX;
		m_ScaleY = SceneGraph->at(*Selection - 1).scaY;
		m_ScaleZ = SceneGraph->at(*Selection - 1).scaZ;

		SceneGraph->at(*Selection - 1).editor_pivot_vis = true;
	}
}

void ObjectManipDialogue::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_PosX, m_PosX);
	DDX_Text(pDX, IDC_PosY, m_PosY);
	DDX_Text(pDX, IDC_PosZ, m_PosZ);

	DDX_Text(pDX, IDC_RotX, m_RotX);
	DDX_Text(pDX, IDC_RotY, m_RotY);
	DDX_Text(pDX, IDC_RotZ, m_RotZ);

	DDX_Text(pDX, IDC_ScaleX, m_ScaleX);
	DDX_Text(pDX, IDC_ScaleY, m_ScaleY);
	DDX_Text(pDX, IDC_ScaleZ, m_ScaleZ);


	DDV_MinMaxFloat(pDX, m_ScaleX, 0.1, 10);
	DDV_MinMaxFloat(pDX, m_ScaleY, 0.1, 10);
	DDV_MinMaxFloat(pDX, m_ScaleZ, 0.1, 10);


}

void ObjectManipDialogue::End()
{
	DestroyWindow();	//destory the window properly.  INcluding the links and pointers created.  THis is so the dialogue can start again. 

}

void ObjectManipDialogue::Select()
{
	CString currentSelectionValue;
}


BOOL ObjectManipDialogue::OnInitDialog()
{
	CDialogEx::OnInitDialog();



	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void ObjectManipDialogue::PostNcDestroy()
{
	DestroyWindow();
}

void ObjectManipDialogue::OnBnClickedOk()
{
	UpdateData(true);

	UpdateData(false);

	if (!m_Inputcommands->updateObject)
	{

		m_sceneGraph->at(m_Current).posX = m_PosX;
		m_sceneGraph->at(m_Current).posY = m_PosY;
		m_sceneGraph->at(m_Current).posZ = m_PosZ;

		m_sceneGraph->at(m_Current).rotX = m_RotX;
		m_sceneGraph->at(m_Current).rotY = m_RotY;
		m_sceneGraph->at(m_Current).rotZ = m_RotZ;

		m_sceneGraph->at(m_Current).scaX = m_ScaleX;
		m_sceneGraph->at(m_Current).scaY = m_ScaleY;
		m_sceneGraph->at(m_Current).scaZ = m_ScaleZ;


		m_Inputcommands->updateObject = true;
	}
}

void ObjectManipDialogue::OnEnd()
{
	DestroyWindow();	//destory the window properly.  INcluding the links and pointers created.  THis is so the dialogue can start again. 

}




void ObjectManipDialogue::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
	m_sceneGraph->at(m_Current).AINode = true;
	// need to loop through and find duplicates
	m_sceneAINodes->push_back(m_sceneGraph->at(m_Current));
	m_ToolSystem.onActionAINode();
	triggerAINode = true;
	//m_Inputcommands->updateObject = true;

	
}
