#pragma once
#include "../../../common/VulkanApp.h"
class Cube;
class MeshDemoApp : public VulkanApp
{
public:
	MeshDemoApp();
	virtual ~MeshDemoApp();

	// Core virtual methods used by derived classes
	void Configure();
	void Setup();
	void Update();

private:
	Cube* m_Cube;
};
