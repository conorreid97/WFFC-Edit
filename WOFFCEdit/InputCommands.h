#pragma once

enum MouseState {
	Pressed,
	Released,
	Held
};

enum ToolState {
	Picking,
	TerrainEdit
};

struct InputCommands
{
	MouseState mouseState_LB;
	MouseState mouseState_RB;
	ToolState tool;

	

	// Mouse
	bool rotRight;
	bool rotLeft;
	int mouse_X;
	int mouse_Y;
	int prev_mouse_X;
	int prev_mouse_Y;
	bool mouse_LB_Down;
	bool mouse_RB_Down;
	bool drag;
	int wheelScroll;

	// Keyboard
	bool forward;	// W
	bool back;		// S
	bool right;		// D
	bool left;		// A

	bool multiSelect; // control
	bool upArrow;
	bool downArrow;
	bool leftArrow;
	bool rightArrow;
	bool enter;
	bool backspace;

	///// Tools
	// Terrain
	int terrainDir;
	bool endTerrainEdit;
	float innerRadius;
	float outerRadius;

	// Duplicate
	bool duplicate;

	// Focus
	bool focus;
	bool finishedLerp;
	bool bScaleManip;
	float camSpeed;

	// Object manip dialog
	bool updateObject;

	// AI Nodes
	bool AINode;
	bool PathNode1;
	bool PathNode2;
	bool PathStart;
	bool PathEnd;
};
