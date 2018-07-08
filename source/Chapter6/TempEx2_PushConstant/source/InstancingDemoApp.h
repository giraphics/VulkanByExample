#pragma once
#include "../../../common/VulkanApp.h"
#include "../TempEx2_SceneGraph/Scene3D.h"

class RectangleModel;
class InstancingDemoApp : public VulkanApp
{
public:
    InstancingDemoApp();
    virtual ~InstancingDemoApp();

    // Core virtual methods used by derived classes
    virtual void Configure();
    virtual void Setup();
    virtual void Update();
    virtual bool Render();
    void ResizeWindow(int width, int height);

	virtual void mousePressEvent(QMouseEvent* p_Event);
	virtual void mouseReleaseEvent(QMouseEvent* p_Event);
	virtual void mouseMoveEvent(QMouseEvent* p_Event);

private:
	void Grid(Scene3D* m_Scene);
	void MixerView(Scene3D* m_Scene);
//	void ProgressBarFunc(Scene3D* m_Scene);

private:
	Scene3D* m_Scene;
    RectangleModel* m_Cube1;
    RectangleModel* m_Cube2;
};
    