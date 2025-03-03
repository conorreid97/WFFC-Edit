#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
#include "SceneObject.h"
#include "InputCommands.h"
#include <vector>

class ObjectManipDialogue :	public CDialogEx
{
	DECLARE_DYNAMIC(ObjectManipDialogue)


public:
	ObjectManipDialogue(CWnd* pParent, std::vector<SceneObject>* SceneGraph);
	ObjectManipDialogue(CWnd* pParent = NULL);
	virtual ~ObjectManipDialogue();

	void SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<int>* SceneAINodes, std::vector<int>* ScenePathNodes, int* Selection, InputCommands* input);	//passing in pointers to the data the class will operate on.

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void End();		//kill the dialogue
	afx_msg void Select();	//Item has been selected

	std::vector<SceneObject>* m_sceneGraph;
	std::vector<int>* m_sceneAINodes;
	std::vector<int>* m_scenePathNodes;
	int* m_currentSelection;
	InputCommands* m_Inputcommands;
	ToolMain m_ToolSystem;
	bool triggerAINode;
	int prevAINodeID;
	DECLARE_MESSAGE_MAP()

public:
	CListBox m_listBox;
	virtual BOOL OnInitDialog() override;
	virtual void PostNcDestroy();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnd();
	afx_msg void OnLbnSelchangeList1();

	float m_PosX;
	float m_PosY;
	float m_PosZ;

	float m_RotX;
	float m_RotY;
	float m_RotZ;

	float m_ScaleX;
	float m_ScaleY;
	float m_ScaleZ;

	int m_AINodeCheck;
	int m_PathStartCheck;
	int m_PathNodeCheck;
	int m_PathNode1Check;
	int m_PathEndCheck;

	int m_CameraCheck;

	int m_Current = 0;
	bool m_IsSelected;
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedAinode();
	afx_msg void OnBnClickedPathstart();
	afx_msg void OnBnClickedPathnode();
	afx_msg void OnBnClickedPathend();
	afx_msg void OnBnClickedCamera();
};

INT_PTR CALLBACK SelectProc(HWND   hwndDlg, UINT   uMsg, WPARAM wParam, LPARAM lParam);