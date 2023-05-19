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

	bool forward;
	bool back;
	bool right;
	bool left;
	bool rotRight;
	bool rotLeft;
	int mouse_X;
	int mouse_Y;
	int prev_mouse_X;
	int prev_mouse_Y;
	bool mouse_LB_Down;
	bool mouse_RB_Down;
	bool multiSelect;
	bool drag;
	bool upArrow;
	bool downArrow;
	bool leftArrow;
	bool rightArrow;
	bool enter;
	bool backspace;
	bool duplicate;
	int terrainDir;
	bool endTerrainEdit;

};
