#pragma once
#include "../../../common/VulkanApp.h"
#include "../TempEx2_SceneGraph/Scene.h"
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
    virtual void Update();
    virtual bool Render();

    void ResizeWindow(int p_Width, int p_Height);

protected:
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);

private:
    void RecordRenderPass(int p_Argcount, ...);

private:
    Scene* m_Scene;
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
