#pragma once
#include "../../../common/VulkanApp.h"
#include "../DrawingShapes_SceneGraph/Scene.h"
#include "UIDemo.h"

class Rectangl;
class Circle;
class UIDemoApp : public VulkanApp
{
public:
    UIDemoApp();
    virtual ~UIDemoApp();

    virtual void Configure();
    virtual void Setup();
    virtual void Update();
    virtual bool Render();
    virtual void ResizeWindow(int p_Width, int p_Height);

protected:
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);

private:
    void RecordRenderPass(int p_Argcount, ...);

private:
    //std::vector<std::shared_ptr<Scene>> m_SceneVector;
    Scene* m_Scene;
    UIDemo m_UIDemo;
};