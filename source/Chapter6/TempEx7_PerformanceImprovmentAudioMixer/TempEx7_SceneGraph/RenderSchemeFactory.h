#pragma once

 #include "Scene.h"
 #include "../common/VulkanHelper.h"

class VulkanApp;

// Rename to RenderSchemeBase
class RenderSchemeFactory
{
public:
    RenderSchemeFactory() {}
    virtual ~RenderSchemeFactory() {}

    virtual void Setup();
    virtual void Update();
    virtual void UpdateDirty(); // XXX
    virtual void Render();
    virtual void Prepare(Scene* p_Scene);
    virtual void UpdateNodeList(Node* p_Item);
    virtual void RemoveNodeList(Node* p_Item);
    virtual void ResizeWindow(int p_Width, int p_Height);

    GETSET(glm::mat4x4, ProjectViewMatrix);
    GETSET(VulkanApp*, VulkanApplication)

protected:
    QMap<QString, QPair<VkPipeline, VkPipelineLayout> > m_GraphicsPipelineMap;
};
