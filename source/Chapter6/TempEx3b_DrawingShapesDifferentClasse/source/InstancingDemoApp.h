#pragma once
#include "../../../common/VulkanApp.h"
#include "../TempEx2_SceneGraph/Scene3D.h"
#include "UIDemo.h"

class Rectangl;
class Circle;
class UIDemoApp : public VulkanApp
{
public:
    UIDemoApp();
    virtual ~UIDemoApp();

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
    Rectangl* m_Cube1;
    Rectangl* m_Cube2;
    Rectangl* m_Cube3;
    Rectangl* m_Cube4;
    Rectangl* item1;
    Rectangl* item2;
    Rectangl* item3;
    Rectangl* item4;
    Circle* m_Cube5;
    UIDemo m_UIDemo;
};
