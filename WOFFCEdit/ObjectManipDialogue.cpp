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
	ON_BN_CLICKED(IDC_AINode, &ObjectManipDialogue::OnBnClickedAinode)
	ON_BN_CLICKED(IDC_PathStart, &ObjectManipDialogue::OnBnClickedPathstart)
	ON_BN_CLICKED(IDC_PathNode, &ObjectManipDialogue::OnBnClickedPathnode)
	ON_BN_CLICKED(IDC_PathEnd, &ObjectManipDialogue::OnBnClickedPathend)
	ON_BN_CLICKED(IDC_Camera, &ObjectManipDialogue::OnBnClickedCamera)
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

void ObjectManipDialogue::SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<int>* SceneAINodes, std::vector<int>* ScenePathNodes, int* Selection, InputCommands* input)
{
	m_sceneGraph = SceneGraph;
	m_currentSelection = Selection - 1;
	m_Inputcommands = input;

	m_sceneAINodes = SceneAINodes;
	m_scenePathNodes = ScenePathNodes;

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

		m_AINodeCheck = SceneGraph->at(*Selection - 1).AINode;
		m_PathStartCheck = SceneGraph->at(*Selection - 1).path_node_start;
		m_PathNodeCheck = SceneGraph->at(*Selection - 1).path_node;
		m_PathEndCheck = SceneGraph->at(*Selection - 1).path_node_end;

		m_CameraCheck = SceneGraph->at(*Selection - 1).camera;

		//// ATTENTION
		if (m_sceneAINodes->size() > 1) {
			m_sceneAINodes->pop_back();
		}
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

	DDX_Check(pDX, IDC_AINode, m_AINodeCheck);
	DDX_Check(pDX, IDC_PathStart, m_PathStartCheck);
	DDX_Check(pDX, IDC_PathNode, m_PathNodeCheck);
	DDX_Check(pDX, IDC_PathEnd, m_PathEndCheck);

	DDX_Check(pDX, IDC_Camera, m_CameraCheck);
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
}


void ObjectManipDialogue::OnBnClickedAinode()
{
	

	// TODO: Add your control notification handler code here
	if (!m_Inputcommands->AINode) {
		m_sceneGraph->at(m_Current).AINode = true;
		m_Inputcommands->AINode = true;
		m_sceneGraph->at(prevAINodeID).AINode = false;
		m_sceneAINodes->push_back(m_Current);
	}
	else {
		m_sceneGraph->at(m_Current).AINode = false;
		if (prevAINodeID != m_Current) {
			m_sceneGraph->at(prevAINodeID).AINode = false;
			m_sceneGraph->at(m_Current).AINode = true;
		}
		m_Inputcommands->AINode = false;
		
	}
	prevAINodeID = m_Current;
	
}


void ObjectManipDialogue::OnBnClickedPathstart()
{
	m_sceneGraph->at(m_Current).path_node_start = true;

	if (!m_Inputcommands->PathStart) {
		m_sceneGraph->at(m_Current).path_node_start = true;
		m_Inputcommands->PathStart = true;
		m_sceneGraph->at(prevAINodeID).path_node_start = false;
	}
	else {
		m_sceneGraph->at(m_Current).path_node_start = false;
		if (prevAINodeID != m_Current) {
			m_sceneGraph->at(prevAINodeID).path_node_start = false;
			m_sceneGraph->at(m_Current).path_node_start = true;
		}
		m_Inputcommands->PathStart = false;

	}
	prevAINodeID = m_Current;
	// TODO: Add your control notification handler code here
}


void ObjectManipDialogue::OnBnClickedPathnode()
{
	// TODO: Add your control notification handler code here
	m_sceneGraph->at(m_Current).path_node = true;
	
	if (m_scenePathNodes->size() > 1) {
		m_sceneGraph->at(m_scenePathNodes->back()).path_node = false;
		m_scenePathNodes->pop_back();
	}
	m_scenePathNodes->push_back(m_Current);
	m_Inputcommands->PathNode1 = true;
	//m_ToolSystem.onActionPathNode(m_Current);
}


void ObjectManipDialogue::OnBnClickedPathend()
{
	// TODO: Add your control notification handler code here
	m_sceneGraph->at(m_Current).path_node_end = true;

	if (!m_Inputcommands->PathEnd) {
		m_sceneGraph->at(m_Current).path_node_end = true;
		m_Inputcommands->PathEnd = true;
		m_sceneGraph->at(prevAINodeID).path_node_end = false;
	}
	else {
		m_sceneGraph->at(m_Current).path_node_end = false;
		if (prevAINodeID != m_Current) {
			m_sceneGraph->at(prevAINodeID).path_node_end = false;
			m_sceneGraph->at(m_Current).path_node_end = true;
		}
		m_Inputcommands->PathEnd = false;

	}
	prevAINodeID = m_Current;
}


void ObjectManipDialogue::OnBnClickedCamera()
{
	// TODO: Add your control notification handler code here

	if (!m_Inputcommands->CameraSet) {
		m_sceneGraph->at(m_Current).camera = true;
		m_Inputcommands->CameraSet = true;
		m_sceneGraph->at(prevAINodeID).camera = false;
	}
	else {
		m_sceneGraph->at(m_Current).camera = false;
		if (prevAINodeID != m_Current) {
			m_sceneGraph->at(prevAINodeID).camera = false;
			m_sceneGraph->at(m_Current).camera = true;
		}
	}
}
