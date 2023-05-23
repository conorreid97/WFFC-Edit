#include "ToolMain.h"
#include "resource.h"
#include "MFCFrame.h"

#include <vector>
#include <sstream>

//
//ToolMain Class
ToolMain::ToolMain()
{

	m_currentChunk = 0;		//default value
	m_selectedObject = -1;	//initial selection ID
	m_sceneGraph.clear();	//clear the vector for the scenegraph
	//m_sceneAINodes.clear();
	m_databaseConnection = NULL;

	//zero input commands
	m_toolInputCommands.forward		= false;
	m_toolInputCommands.back		= false;
	m_toolInputCommands.left		= false;
	m_toolInputCommands.right		= false;
	m_toolInputCommands.updateObject = false;
	m_toolInputCommands.mouseState_LB = Released;
	m_toolInputCommands.mouseState_LB = Released;
	m_toolInputCommands.terrainDir = 1;
	m_toolInputCommands.tool = Picking;
	m_toolInputCommands.innerRadius = 15;
	m_toolInputCommands.outerRadius = 25;
	m_toolInputCommands.camSpeed = 0.30;
	
	for (int i = 0; i < 5; i++) {
		posVectorX.push_back(0);
		posVectorY.push_back(0);
	}
	// set camera
	camType = 1;

	bFocus = false;
	bCamSpline = false;
	bWireframe = false;
	
}


ToolMain::~ToolMain()
{
	sqlite3_close(m_databaseConnection);		//close the database connection
}


int ToolMain::getCurrentSelectionID()
{

	return m_selectedObject;
}

void ToolMain::onActionInitialise(HWND handle, int width, int height)
{
	//window size, handle etc for directX
	m_width		= width;
	m_height	= height;
	m_toolHandle = handle;
	
	CMyFrame* window = (CMyFrame*)CWnd::FromHandle(m_toolHandle);
	CRect viewRect;
	window->GetWindowRect(viewRect);

	m_d3dRenderer.Initialize(handle, m_width, m_height);

	//database connection establish
	int rc;
	rc = sqlite3_open_v2("database/test.db",&m_databaseConnection, SQLITE_OPEN_READWRITE, NULL);

	if (rc) 
	{
		TRACE("Can't open database");
		//if the database cant open. Perhaps a more catastrophic error would be better here
	}
	else 
	{
		TRACE("Opened database successfully");
	}

	onActionLoad();

	
}

void ToolMain::onActionFocusCamera()
{
	if (m_selectedObject != -1) {
		m_d3dRenderer.cam1.Focus(XMFLOAT3(m_sceneGraph[m_selectedObject - 1].posX, m_sceneGraph[m_selectedObject - 1].posY, m_sceneGraph[m_selectedObject - 1].posZ),
			XMFLOAT3(m_sceneGraph[m_selectedObject - 1].scaX, m_sceneGraph[m_selectedObject - 1].scaY, m_sceneGraph[m_selectedObject - 1].scaZ), m_selectedObject, m_toolHandle);
	}
}

void ToolMain::onArcBall() {
	// If an object is selected change to arcball
	if (m_selectedObject != -1) {
		setCamType(2);
		m_d3dRenderer.FocusTool();
	}
	else {
		MessageBox(m_toolHandle, L"Make sure to select an object before opening the inspector.", L"Error", MB_OK);

	}
	
}

void ToolMain::onCamSpline()
{
	if (m_toolInputCommands.CameraSet) {
		setCamType(3);
	}
	else {
		MessageBox(m_toolHandle, L"Make sure to set a camera.", L"Error", MB_OK);

	}
}

void ToolMain::onActivateCamSpline()
{
	if (!bCamSpline) {
		bCamSpline = true;
		// only start the spline when all aspects have been set
		if (m_toolInputCommands.AINode && m_toolInputCommands.PathStart && m_toolInputCommands.PathNode1 && m_toolInputCommands.PathEnd) {
			m_d3dRenderer.CamSplineTool();
		}
		else {
			MessageBox(m_toolHandle, L"Make sure to set an AI Node, Path Start, 2 Path Nodes and a Path End before starting AI Spline. This can be done through object manipulation window", L"Error", MB_OK);
		}
	}
	else {
		bCamSpline = false;
	}
}

void ToolMain::onActivateScaling()
{
	if (!m_toolInputCommands.bScaleManip) {
		m_toolInputCommands.bScaleManip = true;
		bMoveManip = false;
		bRotManip = false;
	}
	else {
		m_toolInputCommands.bScaleManip = false;
	}
}

void ToolMain::onActivateMove()
{
	if (!bMoveManip) {
		bMoveManip = true;
		m_toolInputCommands.bScaleManip = false;
		bRotManip = false;
	}
	else {
		bMoveManip = false;
	}
}

void ToolMain::onActivateRotate()
{
	if (!bRotManip) {
		bRotManip = true;
		m_toolInputCommands.bScaleManip = false; 
		bMoveManip = false;
	}
	else {
		bRotManip = false;
	}
}

void ToolMain::onActivateTerrainEdit()
{
	if (m_toolInputCommands.tool != TerrainEdit) {
		m_toolInputCommands.tool = TerrainEdit;
	}
	else {
		m_toolInputCommands.tool = Picking;
	}
}

void ToolMain::onActionLoad()
{
	//load current chunk and objects into lists
	if (!m_sceneGraph.empty())		//is the vector empty
	{
		m_sceneGraph.clear();		//if not, empty it
	}

	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	sqlite3_stmt *pResultsChunk;

	//OBJECTS IN THE WORLD
	//prepare SQL Text
	sqlCommand = "SELECT * from Objects";				//sql command which will return all records from the objects table.
	//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0 );
	
	//loop for each row in results until there are no more rows.  ie for every row in the results. We create and object
	while (sqlite3_step(pResults) == SQLITE_ROW)
	{	
		SceneObject newSceneObject;
		newSceneObject.ID = sqlite3_column_int(pResults, 0);
		newSceneObject.chunk_ID = sqlite3_column_int(pResults, 1);
		newSceneObject.model_path		= reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		newSceneObject.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		newSceneObject.posX = sqlite3_column_double(pResults, 4);
		newSceneObject.posY = sqlite3_column_double(pResults, 5);
		newSceneObject.posZ = sqlite3_column_double(pResults, 6);
		newSceneObject.rotX = sqlite3_column_double(pResults, 7);
		newSceneObject.rotY = sqlite3_column_double(pResults, 8);
		newSceneObject.rotZ = sqlite3_column_double(pResults, 9);
		newSceneObject.scaX = sqlite3_column_double(pResults, 10);
		newSceneObject.scaY = sqlite3_column_double(pResults, 11);
		newSceneObject.scaZ = sqlite3_column_double(pResults, 12);
		newSceneObject.render = sqlite3_column_int(pResults, 13);
		newSceneObject.collision = sqlite3_column_int(pResults, 14);
		newSceneObject.collision_mesh = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 15));
		newSceneObject.collectable = sqlite3_column_int(pResults, 16);
		newSceneObject.destructable = sqlite3_column_int(pResults, 17);
		newSceneObject.health_amount = sqlite3_column_int(pResults, 18);
		newSceneObject.editor_render = sqlite3_column_int(pResults, 19);
		newSceneObject.editor_texture_vis = sqlite3_column_int(pResults, 20);
		newSceneObject.editor_normals_vis = sqlite3_column_int(pResults, 21);
		newSceneObject.editor_collision_vis = sqlite3_column_int(pResults, 22);
		newSceneObject.editor_pivot_vis = sqlite3_column_int(pResults, 23);
		newSceneObject.pivotX = sqlite3_column_double(pResults, 24);
		newSceneObject.pivotY = sqlite3_column_double(pResults, 25);
		newSceneObject.pivotZ = sqlite3_column_double(pResults, 26);
		newSceneObject.snapToGround = sqlite3_column_int(pResults, 27);
		newSceneObject.AINode = sqlite3_column_int(pResults, 28);
		newSceneObject.audio_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 29));
		newSceneObject.volume = sqlite3_column_double(pResults, 30);
		newSceneObject.pitch = sqlite3_column_double(pResults, 31);
		newSceneObject.pan = sqlite3_column_int(pResults, 32);
		newSceneObject.one_shot = sqlite3_column_int(pResults, 33);
		newSceneObject.play_on_init = sqlite3_column_int(pResults, 34);
		newSceneObject.play_in_editor = sqlite3_column_int(pResults, 35);
		newSceneObject.min_dist = sqlite3_column_double(pResults, 36);
		newSceneObject.max_dist = sqlite3_column_double(pResults, 37);
		newSceneObject.camera = sqlite3_column_int(pResults, 38);
		newSceneObject.path_node = sqlite3_column_int(pResults, 39);
		newSceneObject.path_node_start = sqlite3_column_int(pResults, 40);
		newSceneObject.path_node_end = sqlite3_column_int(pResults, 41);
		newSceneObject.parent_id = sqlite3_column_int(pResults, 42);
		newSceneObject.editor_wireframe = sqlite3_column_int(pResults, 43);
		newSceneObject.name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));

		newSceneObject.light_type = sqlite3_column_int(pResults, 45);
		newSceneObject.light_diffuse_r = sqlite3_column_double(pResults, 46);
		newSceneObject.light_diffuse_g = sqlite3_column_double(pResults, 47);
		newSceneObject.light_diffuse_b = sqlite3_column_double(pResults, 48);
		newSceneObject.light_specular_r = sqlite3_column_double(pResults, 49);
		newSceneObject.light_specular_g = sqlite3_column_double(pResults, 50);
		newSceneObject.light_specular_b = sqlite3_column_double(pResults, 51);
		newSceneObject.light_spot_cutoff = sqlite3_column_double(pResults, 52);
		newSceneObject.light_constant = sqlite3_column_double(pResults, 53);
		newSceneObject.light_linear = sqlite3_column_double(pResults, 54);
		newSceneObject.light_quadratic = sqlite3_column_double(pResults, 55);
	

		//send completed object to scenegraph
		m_sceneGraph.push_back(newSceneObject);
	}

	//THE WORLD CHUNK
	//prepare SQL Text
	sqlCommand = "SELECT * from Chunks";				//sql command which will return all records from  chunks table. There is only one tho.
														//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResultsChunk, 0);


	sqlite3_step(pResultsChunk);
	m_chunk.ID = sqlite3_column_int(pResultsChunk, 0);
	m_chunk.name = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 1));
	m_chunk.chunk_x_size_metres = sqlite3_column_int(pResultsChunk, 2);
	m_chunk.chunk_y_size_metres = sqlite3_column_int(pResultsChunk, 3);
	m_chunk.chunk_base_resolution = sqlite3_column_int(pResultsChunk, 4);
	m_chunk.heightmap_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 5));
	m_chunk.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 6));
	m_chunk.tex_splat_alpha_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 7));
	m_chunk.tex_splat_1_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 8));
	m_chunk.tex_splat_2_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 9));
	m_chunk.tex_splat_3_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 10));
	m_chunk.tex_splat_4_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 11));
	m_chunk.render_wireframe = sqlite3_column_int(pResultsChunk, 12);
	m_chunk.render_normals = sqlite3_column_int(pResultsChunk, 13);
	m_chunk.tex_diffuse_tiling = sqlite3_column_int(pResultsChunk, 14);
	m_chunk.tex_splat_1_tiling = sqlite3_column_int(pResultsChunk, 15);
	m_chunk.tex_splat_2_tiling = sqlite3_column_int(pResultsChunk, 16);
	m_chunk.tex_splat_3_tiling = sqlite3_column_int(pResultsChunk, 17);
	m_chunk.tex_splat_4_tiling = sqlite3_column_int(pResultsChunk, 18);

	
	//Process REsults into renderable
	m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	//build the renderable chunk 
	m_d3dRenderer.BuildDisplayChunk(&m_chunk);

}

void ToolMain::onActionDuplicate()
{
	SceneObject dupObject;
	int dupID = 0;
	int targElement = 0;

	for (int i = 0; i < m_sceneGraph.size(); i++) {
		if (m_sceneGraph.at(i).ID == m_selectedObject) {
			targElement = i;
		}
		dupID = max(dupID, m_sceneGraph[i].ID);
	}
	// Set new object to the same as selected object
	// Give new object a valid ID
	// Increase position of new object
	// Add new object to scenegraph
	dupObject = m_sceneGraph.at(targElement);
	dupObject.ID = dupID + 1;
	dupObject.posY += 2;
	m_sceneGraph.push_back(dupObject);

	m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
}

void::ToolMain::onActionAINode() {
	if (m_sceneGraph.size() > 0) {
		m_sceneAINodes.push_back(m_sceneGraph.at(m_selectedObject - 1).ID);
	}
}

int ToolMain::getID(int ID)
{
	for (size_t i = 0; i < m_sceneGraph.size(); i++) {
		if (m_sceneGraph[i].ID == ID) {
			return i;
		}
	}
	return -1;
}

void ToolMain::onActionSave()
{
	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	

	//OBJECTS IN THE WORLD Delete them all
	//prepare SQL Text
	sqlCommand = "DELETE FROM Objects";	 //will delete the whole object table.   Slightly risky but hey.
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);
	sqlite3_step(pResults);
	
	//Populate with our new objects
	std::wstring sqlCommand2;
	int numObjects = m_sceneGraph.size();	//Loop thru the scengraph.

	for (int i = 0; i < numObjects; i++)
	{
		std::stringstream command;
		command << "INSERT INTO Objects " 
			<<"VALUES(" << m_sceneGraph.at(i).ID << ","
			<< m_sceneGraph.at(i).chunk_ID  << ","
			<< "'" << m_sceneGraph.at(i).model_path <<"'" << ","
			<< "'" << m_sceneGraph.at(i).tex_diffuse_path << "'" << ","
			<< m_sceneGraph.at(i).posX << ","
			<< m_sceneGraph.at(i).posY << ","
			<< m_sceneGraph.at(i).posZ << ","
			<< m_sceneGraph.at(i).rotX << ","
			<< m_sceneGraph.at(i).rotY << ","
			<< m_sceneGraph.at(i).rotZ << ","
			<< m_sceneGraph.at(i).scaX << ","
			<< m_sceneGraph.at(i).scaY << ","
			<< m_sceneGraph.at(i).scaZ << ","
			<< m_sceneGraph.at(i).render << ","
			<< m_sceneGraph.at(i).collision << ","
			<< "'" << m_sceneGraph.at(i).collision_mesh << "'" << ","
			<< m_sceneGraph.at(i).collectable << ","
			<< m_sceneGraph.at(i).destructable << ","
			<< m_sceneGraph.at(i).health_amount << ","
			<< m_sceneGraph.at(i).editor_render << ","
			<< m_sceneGraph.at(i).editor_texture_vis << ","
			<< m_sceneGraph.at(i).editor_normals_vis << ","
			<< m_sceneGraph.at(i).editor_collision_vis << ","
			<< m_sceneGraph.at(i).editor_pivot_vis << ","
			<< m_sceneGraph.at(i).pivotX << ","
			<< m_sceneGraph.at(i).pivotY << ","
			<< m_sceneGraph.at(i).pivotZ << ","
			<< m_sceneGraph.at(i).snapToGround << ","
			<< m_sceneGraph.at(i).AINode << ","
			<< "'" << m_sceneGraph.at(i).audio_path << "'" << ","
			<< m_sceneGraph.at(i).volume << ","
			<< m_sceneGraph.at(i).pitch << ","
			<< m_sceneGraph.at(i).pan << ","
			<< m_sceneGraph.at(i).one_shot << ","
			<< m_sceneGraph.at(i).play_on_init << ","
			<< m_sceneGraph.at(i).play_in_editor << ","
			<< m_sceneGraph.at(i).min_dist << ","
			<< m_sceneGraph.at(i).max_dist << ","
			<< m_sceneGraph.at(i).camera << ","
			<< m_sceneGraph.at(i).path_node << ","
			<< m_sceneGraph.at(i).path_node_start << ","
			<< m_sceneGraph.at(i).path_node_end << ","
			<< m_sceneGraph.at(i).parent_id << ","
			<< m_sceneGraph.at(i).editor_wireframe << ","
			<< "'" << m_sceneGraph.at(i).name << "'" << ","

			<< m_sceneGraph.at(i).light_type << ","
			<< m_sceneGraph.at(i).light_diffuse_r << ","
			<< m_sceneGraph.at(i).light_diffuse_g << ","
			<< m_sceneGraph.at(i).light_diffuse_b << ","
			<< m_sceneGraph.at(i).light_specular_r << ","
			<< m_sceneGraph.at(i).light_specular_g << ","
			<< m_sceneGraph.at(i).light_specular_b << ","
			<< m_sceneGraph.at(i).light_spot_cutoff << ","
			<< m_sceneGraph.at(i).light_constant << ","
			<< m_sceneGraph.at(i).light_linear << ","
			<< m_sceneGraph.at(i).light_quadratic

			<< ")";
		std::string sqlCommand2 = command.str();
		rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand2.c_str(), -1, &pResults, 0);
		sqlite3_step(pResults);	
	}
	MessageBox(NULL, L"Objects Saved", L"Notification", MB_OK);
}

void ToolMain::onActionSaveTerrain()
{
	m_d3dRenderer.SaveDisplayChunk(&m_chunk);
}

void ToolMain::Tick(MSG *msg)
{
	// get the window from MFC	
	CWnd* window = CWnd::FromHandle(m_toolHandle);
	CRect viewRect;
	window->GetWindowRect(viewRect);

	// Update camera
	if (camType == 1) {
		m_d3dRenderer.setCamType(1);
	}
	else if (camType == 2) {
		m_d3dRenderer.setCamType(2);
	}
	if (camType == 3) {
		m_d3dRenderer.setCamType(3);
	}

	//if (m_selectedObject != -1 && m_sceneGraph.at(m_selectedObject -1).camera) {
	//	//DeleteObject();
	//}

	// update the wireframe mode
	m_d3dRenderer.bWireframe = bWireframe;

	TerrainUpdate();

	ObjectUpdate();

	MouseUpdate();

	CamSplineUpdate();

	

	// update object manipulation dialogue
	if (m_toolInputCommands.updateObject) {
		m_d3dRenderer.UpdateObjectData(&m_sceneGraph.at(m_selectedObject), m_selectedObject);
		
		m_toolInputCommands.updateObject = false;
		m_d3dRenderer.SetRebuildDisplayList(true);
	}

	
	// update the display list if manipilation is active
	if (m_toolInputCommands.bScaleManip || bMoveManip || bRotManip || bCamSpline) {
		m_d3dRenderer.SetRebuildDisplayList(true);
	}



	//Renderer Update Call
	m_d3dRenderer.Tick(&m_toolInputCommands, &m_sceneGraph);
	if (m_d3dRenderer.GetRebuildDisplayList()) {
		m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	}
	
	//m_selectedObject = m_d3dRenderer.GetSelectedID();
}

void ToolMain::UpdateInput(MSG * msg)
{

	switch (msg->message)
	{
		//Global inputs,  mouse position and keys etc
	case WM_KEYDOWN:
		m_keyArray[msg->wParam] = true;
		break;

	case WM_KEYUP:
		m_keyArray[msg->wParam] = false;
		break;

	case WM_MOUSEMOVE:
		
		bDragging = true;

		m_toolInputCommands.mouse_X = GET_X_LPARAM(msg->lParam);
		m_toolInputCommands.mouse_Y = GET_Y_LPARAM(msg->lParam);

		posVectorX.push_back(m_toolInputCommands.mouse_X);
		posVectorY.push_back(m_toolInputCommands.mouse_Y);

		m_toolInputCommands.prev_mouse_X = posVectorX[posVectorX.size() - 2];
		m_toolInputCommands.prev_mouse_Y = posVectorY[posVectorY.size() - 2];

		break;

	case WM_LBUTTONDOWN:	//mouse button down,  you will probably need to check when its up too
		//set some flag for the mouse button in inputcommands
		m_toolInputCommands.mouse_LB_Down = true;
		m_toolInputCommands.mouseState_LB = Pressed;
		break;
	case WM_LBUTTONUP:
		m_toolInputCommands.mouse_LB_Down = false;
		m_toolInputCommands.mouseState_LB = Released;
		if (m_toolInputCommands.tool == TerrainEdit) {
			m_toolInputCommands.endTerrainEdit = true;
		}
		break;
	case WM_RBUTTONDOWN:
		m_toolInputCommands.mouse_RB_Down = true;
		break;
	case WM_RBUTTONUP:
		m_toolInputCommands.mouse_RB_Down = false;
		break;
	case WM_MOUSEWHEEL:
		m_toolInputCommands.wheelScroll = GET_WHEEL_DELTA_WPARAM(msg->wParam);
		

		// mouse scroll wheel
		// depending on what mode the user is in either increase brush size or increase cam speed
		if (m_toolInputCommands.tool == Picking) {
			if (m_toolInputCommands.wheelScroll > 0) {
				m_toolInputCommands.camSpeed += 0.1f;
			}
			else {
				m_toolInputCommands.camSpeed -= 0.1f;
			}
			if (m_toolInputCommands.camSpeed <= 0.1) {
				m_toolInputCommands.camSpeed = 0.1;
			}
		}
		if (m_toolInputCommands.tool == TerrainEdit) {
			if (m_toolInputCommands.wheelScroll > 0) {
				m_toolInputCommands.innerRadius += 1.0f;
				m_toolInputCommands.outerRadius += 1.0f;
			}
			else {
				m_toolInputCommands.innerRadius -= 1.0f;
				m_toolInputCommands.outerRadius -= 1.0f;
			}
			// min inner radius
			if (m_toolInputCommands.innerRadius <= 1.0) {
				m_toolInputCommands.innerRadius = 1.0;
			}
			// max inner radius
			if (m_toolInputCommands.innerRadius >= 40.0) {
				m_toolInputCommands.innerRadius = 40.0;
			}
			// min outer radius
			if (m_toolInputCommands.outerRadius <= 5.0) {
				m_toolInputCommands.outerRadius = 5.0f;
			}
			// max outer radius
			if (m_toolInputCommands.outerRadius >= 55.f) {
				m_toolInputCommands.outerRadius = 55.f;
			}
		}
		break;
	}

	//here we update all the actual app functionality that we want.  This information will either be used int toolmain, or sent down to the renderer (Camera movement etc
	//WASD movement
	if (m_keyArray['W'])
	{
		m_toolInputCommands.forward = true;
	}
	else m_toolInputCommands.forward = false;
	
	if (m_keyArray['S'])
	{
		m_toolInputCommands.back = true;
	}
	else m_toolInputCommands.back = false;
	if (m_keyArray['A'])
	{
		m_toolInputCommands.left = true;
	}
	else m_toolInputCommands.left = false;

	if (m_keyArray['D'])
	{
		m_toolInputCommands.right = true;
	}
	else m_toolInputCommands.right = false;
	//rotation
	if (m_keyArray['E'])
	{
		m_toolInputCommands.rotLeft = true;
	}
	else m_toolInputCommands.rotLeft = false;
	if (m_keyArray['Q'])
	{
		m_toolInputCommands.rotRight = true;
	}
	else m_toolInputCommands.rotRight = false;
	// Duplicating
	if (m_keyArray['C'])
	{
		if (!m_toolInputCommands.duplicate) {
			// function call to duplicate
			onActionDuplicate();
		}
		m_toolInputCommands.duplicate = true;
	}
	else m_toolInputCommands.duplicate = false;
	// Focus Camera
	if (m_keyArray['F']) {
		onActionFocusCamera();
	}
	else m_toolInputCommands.focus = false;
	if (m_keyArray[17]) {	// 17 is the number for the ctrl button
		m_toolInputCommands.multiSelect = true;
	}
	else m_toolInputCommands.multiSelect = false;
	if (m_keyArray[38]) {	// 38 is the number for the up button
		m_toolInputCommands.upArrow = true;
	}
	else m_toolInputCommands.upArrow = false;
	if (m_keyArray[40]) {	// 40 is the number for the down button
		m_toolInputCommands.downArrow = true;
	}
	else m_toolInputCommands.downArrow = false;
	if (m_keyArray[37]) {	//37 is the number for the left button
		m_toolInputCommands.leftArrow = true;
	}
	else m_toolInputCommands.leftArrow = false;
	if (m_keyArray[39]) {	// 39 is the number for the righ button
		m_toolInputCommands.rightArrow = true;
	}
	else m_toolInputCommands.rightArrow = false;
	if (m_keyArray[13]) {	// 13 is the number for the enter button
		m_toolInputCommands.enter = true;
	}
	else m_toolInputCommands.enter = false;
	if (m_keyArray[8]) {	// 8 is the number for the backspace button
		m_toolInputCommands.backspace = true;
	}
	else m_toolInputCommands.backspace = false;
	// Object Deletion
	if (m_keyArray[VK_DELETE]) {
		DeleteObject();
	}
	//WASD
}
void ToolMain::DeleteObject()
{
	if (m_selectedObject != -1){
		// create new scenegraph 
		std::vector<SceneObject> newSceneGraph;
		// fill new scenegraph with everything apart from object
		for (int i = 0; i < m_sceneGraph.size(); i++){
			if (m_sceneGraph.at(i).ID != m_selectedObject){
				newSceneGraph.push_back(m_sceneGraph.at(i));
			}
		}
		// update scene graph
		m_sceneGraph.clear();
		m_sceneGraph = newSceneGraph;

		m_selectedObject = -1;

		m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	}
}
void ToolMain::MouseUpdate()
{
	if (bDragging) {
		m_toolInputCommands.drag = true;
	}
	else if (!bDragging) {
		m_toolInputCommands.drag = false;
	}
	// set the dragging to false if mouse isnt currently moving 
	if (posVectorX.back() == posVectorX[posVectorX.size() - 5]) {
		bDragging = false;
	}
	else if (posVectorY.back() == posVectorY[posVectorY.size() - 5]) {
		bDragging = false;
	}

	// set the right mouse button up if mouse moves off of screen
	if (posVectorX.back() > m_width || posVectorX.back() < 0) {
		m_toolInputCommands.mouse_RB_Down = false;
	}
	else if (posVectorY.back() > m_height || posVectorY.back() < 0)
	{
		m_toolInputCommands.mouse_RB_Down = false;
	}
	// Perform the mouse picking
	if (m_toolInputCommands.mouse_LB_Down)
	{
		m_selectedObject = m_d3dRenderer.MousePicking();
		m_toolInputCommands.mouse_LB_Down = false;
	}
}

void ToolMain::CamSplineUpdate()
{
	if (m_d3dRenderer.bCamPath) {

		for (int i = 0; i < m_sceneGraph.size(); i++) {
			if (m_sceneGraph.at(i).path_node_start) {
				camSpline.p1 = XMVectorSet(m_sceneGraph[i].posX, m_sceneGraph[i].posY, m_sceneGraph[i].posZ, 1.0f); // 5
			}
			
			if (m_sceneGraph.at(i).path_node_end) {
				camSpline.p2 = XMVectorSet(m_sceneGraph[i].posX, m_sceneGraph[i].posY, m_sceneGraph[i].posZ, 1.0f); // 6
			}
		}
		// control points
		camSpline.p0 = XMVectorSet(m_sceneGraph[m_scenePathNodes[0]].posX, m_sceneGraph[m_scenePathNodes[0]].posY, m_sceneGraph[m_scenePathNodes[0]].posZ, 1.0f);
		camSpline.p3 = XMVectorSet(m_sceneGraph[m_scenePathNodes[1]].posX, m_sceneGraph[m_scenePathNodes[1]].posY, m_sceneGraph[m_scenePathNodes[1]].posZ, 1.0f);
			
		XMFLOAT3 AIPos = XMFLOAT3(0.0, 0.0, 0.0);
		XMFLOAT3 CamPos = XMFLOAT3(0.0, 0.0, 0.0);
		AIPos = camSpline.AIUpdate();
		CamPos = camSpline.CamUpdate();

		// update the position
		if (m_selectedObject != -1) {
			for (int i = 0; i < m_sceneAINodes.size(); i++) {
				m_sceneGraph[m_sceneAINodes[i]].posX = AIPos.x;
				m_sceneGraph[m_sceneAINodes[i]].posY = AIPos.y;
				m_sceneGraph[m_sceneAINodes[i]].posZ = AIPos.z;

				m_d3dRenderer.cam2.m_camPosition = CamPos;
				m_d3dRenderer.cam2.m_camOrientation - XMFLOAT3(1.0, 0.0, 0.0);
			}
		}
		
	}
}

void ToolMain::ObjectUpdate()
{
	if (m_selectedObject == -1) {
		return;
	}
	if (m_toolInputCommands.bScaleManip) {
			
		if (m_toolInputCommands.upArrow) {
			m_sceneGraph[m_selectedObject - 1].scaX += 0.2;
			m_sceneGraph[m_selectedObject - 1].scaY += 0.2;
			m_sceneGraph[m_selectedObject - 1].scaZ += 0.2;
			m_d3dRenderer.scale += 0.01;
		}	
		if (m_toolInputCommands.downArrow) {
			m_sceneGraph[m_selectedObject - 1].scaX -= 0.2;
			m_sceneGraph[m_selectedObject - 1].scaY -= 0.2;
			m_sceneGraph[m_selectedObject - 1].scaZ -= 0.2;
			m_d3dRenderer.scale -= 0.01;
			//m_d3dRenderer.MoveObject();
		}
	}

	if (bMoveManip) {
		if (!m_toolInputCommands.multiSelect) {	// multiSelect is the ctrl button
			if (m_toolInputCommands.upArrow) {
				m_sceneGraph[m_selectedObject - 1].posX += 0.2;
			}
			if (m_toolInputCommands.downArrow) {
				m_sceneGraph[m_selectedObject - 1].posX -= 0.2;
			}
			if (m_toolInputCommands.leftArrow) {
				m_sceneGraph[m_selectedObject - 1].posZ -= 0.2;
			}
			if (m_toolInputCommands.rightArrow) {
				m_sceneGraph[m_selectedObject - 1].posZ += 0.2;
			}
		}
		
		if (m_toolInputCommands.multiSelect) {	// multiSelect is the ctrl button
			if (m_toolInputCommands.upArrow) {
				m_sceneGraph[m_selectedObject - 1].posY += 0.2;
			}
			if (m_toolInputCommands.downArrow) {
				m_sceneGraph[m_selectedObject - 1].posY -= 0.2;
			}
		}
	
	}

	if (bRotManip) {
		if (!m_toolInputCommands.multiSelect) {
			if (m_toolInputCommands.upArrow) {
				m_sceneGraph[m_selectedObject - 1].rotX += 0.5;
			}
			if (m_toolInputCommands.downArrow) {
				m_sceneGraph[m_selectedObject - 1].rotX -= 0.5;
			}
			if (m_toolInputCommands.leftArrow) {
				m_sceneGraph[m_selectedObject - 1].rotZ -= 0.5;
			}
			if (m_toolInputCommands.rightArrow) {
				m_sceneGraph[m_selectedObject - 1].rotZ += 0.5;
			}
		}
		if (m_toolInputCommands.multiSelect) {	// multiSelect is the ctrl button
			if (m_toolInputCommands.rightArrow) {
				m_sceneGraph[m_selectedObject - 1].rotY += 0.2;
			}
			if (m_toolInputCommands.leftArrow) {
				m_sceneGraph[m_selectedObject - 1].rotY -= 0.2;
			}
		}

	}
}

void ToolMain::TerrainUpdate()
{
	if (m_toolInputCommands.tool == TerrainEdit) {
		if (m_toolInputCommands.mouseState_LB == Pressed)
		{
			m_toolInputCommands.mouseState_LB = Held;
		}
		if (m_toolInputCommands.mouseState_LB == Held) {
			// if ctrl is held switch the terrain direction to negative
			if (m_toolInputCommands.multiSelect) {
				m_toolInputCommands.terrainDir = -1;
			}
			else {
				m_toolInputCommands.terrainDir = 1;
			}
			m_d3dRenderer.EditTerrain();
		}
		if (m_toolInputCommands.endTerrainEdit) {
			m_toolInputCommands.endTerrainEdit = false;
		}
	}
}
