#pragma once
#ifndef __MODULESCENEWORLD_H__
#define __MODULESCENEWORLD_H__

#include "Module.h"
#include "Globals.h"
#include "ImGui\imgui.h"
#include "WindowManager.h"


class SceneWorld : public WindowManager
{
public:

	SceneWorld();
	virtual ~SceneWorld();

	bool Start();
	update_status Update(float dt);
	void ShowSceneWorld();
	bool CleanUp();

	//void OpenClose();
	//bool IsOpen();


private:


};

#endif //__MODULEHIERARCHY_H__