#pragma once

struct InputCommands
{
	bool forward;
	bool back;
	bool right;
	bool left;
	bool rotRight;
	bool rotLeft;
	// mouse x position
	float mousePosX;
	// mouse y position
	float mousePosY;
	//mouse x drag last frame?
	float mousePosXPrev;
	// mouse y drag
	float mousePosYPrev;
	// mouse l button
	bool LMButtonDown;
	// mouse r button
	bool RMButtonDown;
	
};
