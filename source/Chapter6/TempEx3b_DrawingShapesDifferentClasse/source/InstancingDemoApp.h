#pragma once
#include "../../../common/VulkanApp.h"
#include "../TempEx2_SceneGraph/Scene3D.h"
#include "UIDemo.h"

class RectangleModel;
class Circle;
class InstancingDemoApp : public VulkanApp
{
public:
    InstancingDemoApp();
    virtual ~InstancingDemoApp();

    // Core virtual methods used by derived classes
    virtual void Configure();
    virtual void Setup();
    void SetupPrivate();
    virtual void Update();
    virtual bool Render();
    void RecordRenderPass();
    void ResizeWindow(int width, int height);

	virtual void mousePressEvent(QMouseEvent* p_Event);
	virtual void mouseReleaseEvent(QMouseEvent* p_Event);
	virtual void mouseMoveEvent(QMouseEvent* p_Event);

private:
	Scene3D* m_Scene;
    RectangleModel* m_Cube1;
    RectangleModel* m_Cube2;
    RectangleModel* m_Cube3;
    RectangleModel* m_Cube4;
    RectangleModel* item1;
    RectangleModel* item2;
    RectangleModel* item3;
    RectangleModel* item4;
    Circle* m_Cube5;
    UIDemo m_UIDemo;
};
    