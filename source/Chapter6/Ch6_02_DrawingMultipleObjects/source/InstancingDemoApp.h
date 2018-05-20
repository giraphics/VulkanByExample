#pragma once
#include "../../../common/VulkanApp.h"
#include "../../../common/SceneGraph/Scene3D.h"

class RectangleFactory;
class InstancingDemoApp : public VulkanApp
{
public:
    InstancingDemoApp();
    virtual ~InstancingDemoApp();

    // Core virtual methods used by derived classes
    virtual void Configure();
    virtual void Setup();
    virtual void Update();
    void ResizeWindow(int width, int height);
private:
	void Grid(Scene3D* m_Scene);

private:
	RectangleFactory* m_CubeFactory;
    //Rect* m_Cube;
	Scene3D* m_Scene;
	Model3D* m_Cube;
};

