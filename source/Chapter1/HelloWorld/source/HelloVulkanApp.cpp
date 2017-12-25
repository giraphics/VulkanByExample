#include "HelloVulkanApp.h"

HelloVulkanApp::HelloVulkanApp()
{
    m_hPipelineLayout = VK_NULL_HANDLE;
    m_hGraphicsPipeline = VK_NULL_HANDLE;
}

bool HelloVulkanApp::Configure()
{
    SetApplicationName("Hello World!");
    return (true);
}

bool HelloVulkanApp::Update()
{
    return true;
}

bool HelloVulkanApp::Setup()
{
    bool result = true;

    CreateGraphicsPipeline();

    // The following code records the commands to draw
    // a quad in a blue background

    // Background color (Blue)
    // The color values are defined in this order (Red,Green,Blue,Alpha)
    VkClearValue clearColor = { 0.0f, 0.0f, 1.0f, 1.0f };
    // Offset to render in the frame buffer
    VkOffset2D   renderOffset = { 0, 0 };
    // Width & Height to render in the frame buffer
    VkExtent2D   renderExtent = m_swapChainExtent;

    // For each command buffers in the command buffer list
    for (size_t i = 0; i < m_hCommandBufferList.size() && (result == true); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // Indicate that the command buffer can be resubmitted to the queue
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // Step 1: Begin command buffer
        vkBeginCommandBuffer(m_hCommandBufferList[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_hRenderPass;
        renderPassInfo.framebuffer = m_hSwapChainFramebufferList[i];
        renderPassInfo.renderArea.offset = renderOffset;
        renderPassInfo.renderArea.extent = renderExtent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // Step 2: Begin render pass
        vkCmdBeginRenderPass(m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Step 3: Bind graphics pipeline
        vkCmdBindPipeline(m_hCommandBufferList[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_hGraphicsPipeline);

        // Step 4: Draw a quad using 3 vertices 
        vkCmdDraw(m_hCommandBufferList[i], 3, 1, 0, 0);

        // Step 5: End the Render pass
        vkCmdEndRenderPass(m_hCommandBufferList[i]);

        // Step 6: End the Command buffer
        VkResult vkResult = vkEndCommandBuffer(m_hCommandBufferList[i]);
        if (vkResult != VK_SUCCESS)
        {
            LogError("vkEndCommandBuffer() failed!");
            result = false;
        }
    }
    return (result);
}

bool HelloVulkanApp::Render()
{ 
    return VulkanApp::Render(); 
}

void HelloVulkanApp::Close()
{
    vkDestroyPipeline(m_hDevice, m_hGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_hDevice, m_hPipelineLayout, nullptr);
}

bool HelloVulkanApp::CreateGraphicsPipeline()
{
    bool result = true;

    // Compile the vertex shader
	VkShaderModule vertShader = CreateShader("../source/QuadVert.spv"); // Relative path to binary output dir

	// Setup the vertex shader stage create info structures
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    // Compile the fragment shader
	VkShaderModule fragShader = CreateShader("../source/QuadFrag.spv"); // Relative path to binary output dir

	// Setup the fragment shader stage create info structures
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // Setup the vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    // Setup input assembly
    // We will be rendering 1 triangle using triangle strip topology
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport to the maximum widht and height of the window
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapChainExtent.width;
    viewport.height = (float)m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Setup scissor rect
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChainExtent;

    // Setup view port state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Setup the rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Setup multi sampling. In our first example we will be using single sampling mode
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Setup color output masks.
    // Set to write out RGBA components
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    // Setup color blending
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VkResult vkResult = vkCreatePipelineLayout(m_hDevice, &pipelineLayoutInfo, nullptr, &m_hPipelineLayout);

    if (vkResult != VK_SUCCESS)
    {
        LogError("vkCreatePipelineLayout() failed!");
        result = false;
    }
    else
    {
        // Create graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = m_hPipelineLayout;
        pipelineInfo.renderPass = m_hRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        vkResult = vkCreateGraphicsPipelines(m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_hGraphicsPipeline);
        if (vkResult != VK_SUCCESS)
        {
            LogError("vkCreateGraphicsPipelines() failed!");
            result = false;
        }
    }

    // Cleanup
    vkDestroyShaderModule(m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_hDevice, vertShader, nullptr);

    return (result);
}
