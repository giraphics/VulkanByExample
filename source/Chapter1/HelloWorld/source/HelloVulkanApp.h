#pragma once
#include "../../../common/VulkanApp.h"

class HelloVulkanApp :public VulkanApp
{
public:
    HelloVulkanApp();
    ~HelloVulkanApp() {}

    bool Configure();
    bool Setup();
    bool Update();
    bool Render();
    void Close();

private:
    // Store app specific objects
    VkPipelineLayout            m_hPipelineLayout;
    VkPipeline                  m_hGraphicsPipeline;

    bool CreateGraphicsPipeline();

};