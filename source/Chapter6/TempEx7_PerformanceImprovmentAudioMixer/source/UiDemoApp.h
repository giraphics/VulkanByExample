#pragma once
#include "../../../common/VulkanApp.h"
#include "../TempEx7_SceneGraph/Scene.h"
#include "UIDemo.h"

class UiMetalPaintEngine;
//class RectangleInstancingScheme;
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
    virtual void ResizeWindow(int p_Width, int p_Height);

    virtual bool InitMetalEngine();

protected:
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);

private:
    void RecordRenderPass(int p_Argcount, ...);

private:
    Scene* m_Scene;
    Scene* m_ScenePainterEngine;
    UIDemo m_UIDemo;
    QScopedPointer<UiMetalPaintEngine> m_MetalPaintEngine;
};
