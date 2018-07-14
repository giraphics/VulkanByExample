#include  "Circle.h"

static char* PIPELINE_RECT_FILLED = "RectFilled";
static char* PIPELINE_RECT_OUTLINE = "RectOutline";

#define VERTEX_BUFFER_BIND_IDX 0

AbstractModelFactory* Circle::GetRenderScemeFactory()
{
    return new CircleMultiDrawFactory(static_cast<VulkanApp*>(m_Scene->GetApplication()));
}

static const Vertex rectFilledVertices[] =
{
    { glm::vec3(1, 0, 0),	glm::vec3(0.f, 0.f, 0.f), 0 },
    { glm::vec3(0, 0, 0),	glm::vec3(1.f, 0.f, 0.f), 0 },
    { glm::vec3(1, 1, 0),	glm::vec3(0.f, 1.f, 0.f), 0 },
    { glm::vec3(1, 1, 0),	glm::vec3(0.f, 1.f, 0.f), 0 },
    { glm::vec3(0, 0, 0),	glm::vec3(1.f, 0.f, 0.f), 0 },
    { glm::vec3(0, 1, 0),	glm::vec3(1.f, 1.f, 0.f), 0 },
};

static const Vertex rectOutlineVertices[] =
{
    { glm::vec3(0, 0, 0),	glm::vec3(0.f, 0.f, 0.f) },
    { glm::vec3(1, 0, 0),	glm::vec3(1.f, 0.f, 0.f) },
    { glm::vec3(1, 1, 0),	glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3(0, 1, 0),	glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3(0, 0, 0),	glm::vec3(0.f, 0.f, 0.f) },
};

CircleMultiDrawFactory::CircleMultiDrawFactory(VulkanApp* p_VulkanApp)
{
    assert(p_VulkanApp);
    m_VulkanApplication = p_VulkanApp;

    memset(&UniformBuffer, 0, sizeof(UniformBuffer));

    CDS = NULL;// std::make_shared<CubeDescriptorSet>(m_VulkanApplication);
    UniformBuffer = NULL;
}

CircleMultiDrawFactory::~CircleMultiDrawFactory()
{
    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        ModelVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = m_ModelList.size();
        if (!modelSize) continue;

        // Destroy Vertex Buffer
        for (int j = 0; j < modelSize; j++)
        {
            if (m_ModelList.at(j)->GetRefShapeType() == SHAPE_CIRCLE_MULTIDRAW)
            {
                RectangleModel* model = (static_cast<RectangleModel*>(m_ModelList.at(j)));
                if (!model) return;

                vkDestroyBuffer(m_VulkanApplication->m_hDevice, model->m_VertexBuffer.m_Buffer, NULL);
                vkFreeMemory(m_VulkanApplication->m_hDevice, model->m_VertexBuffer.m_Memory, NULL);
            }
        }
    }

    delete CDS;
    //// Destroy descriptors
    //for (int i = 0; i < descLayout.size(); i++) {
    //       vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
    //}
    //descLayout.clear();

    //vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
    //vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);

    vkUnmapMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory);
    vkDestroyBuffer(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory, NULL);
}

void CircleMultiDrawFactory::Setup(VkCommandBuffer& p_CommandBuffer)
{
    if (!CDS)
    {
        //CDS = std::make_shared<RectangleDescriptorSet>(m_VulkanApplication);
        CDS = new CircleDescriptorSet(m_VulkanApplication);
        CDS->CreateDescriptor();
        UniformBuffer = CDS->UniformBuffer;
    }

    CreateVertexBuffer();

    CreateGraphicsPipeline();

    // Build the push constants
    createPushConstants();

//    m_VulkanApplication->CreateCommandBuffers(); // Create command buffers

    RecordCommandBuffer(p_CommandBuffer);
}

void CircleMultiDrawFactory::Update()
{
    VulkanHelper::WriteMemory(m_VulkanApplication->m_hDevice,
        UniformBuffer->m_MappedMemory,
        UniformBuffer->m_MappedRange,
        UniformBuffer->m_BufObj.m_MemoryFlags,
        &m_ProjectViewMatrix, sizeof(m_ProjectViewMatrix));
}

void CircleMultiDrawFactory::ResizeWindow(int width, int height)
{
    return;
    /*
    CreateGraphicsPipeline(true);

//    m_VulkanApplication->CreateCommandBuffers(); // Create command buffers

    RecordCommandBuffer();
    */
}

void CircleMultiDrawFactory::CreateRectOutlinePipeline()
{
    // Compile the vertex shader
    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceVert.spv"); // Relative path to binary output dir
                                                                                                                                      // Setup the vertex shader stage create info structures
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    // Compile the fragment shader
    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceFrag.spv"); // Relative path to binary output dir
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
    // Check the size where every necessrry
    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)m_VertexInputBinding[PIPELINE_OUTLINE].size();
    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_OUTLINE].data();
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)m_VertexInputAttribute[PIPELINE_OUTLINE].size();
    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_OUTLINE].data();

    // Setup input assembly
    // We will be rendering 1 triangle using triangle strip topology
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport to the maximum widht and height of the window
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_VulkanApplication->m_swapChainExtent.width;
    viewport.height = (float)m_VulkanApplication->m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Setup scissor rect
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_VulkanApplication->m_swapChainExtent;

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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthClampEnable = true;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.pNext = NULL;
    depthStencilStateInfo.flags = 0;
    depthStencilStateInfo.depthTestEnable = true;
    depthStencilStateInfo.depthWriteEnable = true;
    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateInfo.back.compareMask = 0;
    depthStencilStateInfo.back.reference = 0;
    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.writeMask = 0;
    depthStencilStateInfo.minDepthBounds = 0;
    depthStencilStateInfo.maxDepthBounds = 0;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.front = depthStencilStateInfo.back;

    // Setup multi sampling. In our first example we will be using single sampling mode
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = NUM_SAMPLES;

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
    pipelineLayoutInfo.setLayoutCount = (uint32_t)CDS->descLayout.size();
    pipelineLayoutInfo.pSetLayouts = CDS->descLayout.data();

    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkResult vkResult = vkCreatePipelineLayout(m_VulkanApplication->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
        assert(false);
    }

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
    pipelineInfo.layout = graphicsPipelineLayout;
    pipelineInfo.renderPass = m_VulkanApplication->m_hRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    vkResult = vkCreateGraphicsPipelines(m_VulkanApplication->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
        assert(false);
    }

    m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE] = qMakePair(graphicsPipeline, graphicsPipelineLayout);

    // Cleanup
    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, vertShader, nullptr);
}

void CircleMultiDrawFactory::CreateRectFillPipeline()
{
    // Compile the vertex shader
    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceVert.spv"); // Relative path to binary output dir
                                                                                                                                      // Setup the vertex shader stage create info structures
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    // Compile the fragment shader
    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceFrag.spv"); // Relative path to binary output dir
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
    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)m_VertexInputBinding[PIPELINE_FILLED].size();// sizeof(m_VertexInputBinding[PIPELINE_FILLED]) / sizeof(VkVertexInputBindingDescription);
    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_FILLED].data();
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)m_VertexInputAttribute[PIPELINE_FILLED].size();// sizeof(m_VertexInputAttribute[PIPELINE_FILLED]) / sizeof(VkVertexInputAttributeDescription);
    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_FILLED].data();

    // Setup input assembly
    // We will be rendering 1 triangle using triangle strip topology
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport to the maximum widht and height of the window
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_VulkanApplication->m_swapChainExtent.width;
    viewport.height = (float)m_VulkanApplication->m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Setup scissor rect
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_VulkanApplication->m_swapChainExtent;

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
    rasterizer.cullMode = VK_CULL_MODE_NONE/*VK_CULL_MODE_BACK_BIT*/;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthClampEnable = true;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.pNext = NULL;
    depthStencilStateInfo.flags = 0;
    depthStencilStateInfo.depthTestEnable = true;
    depthStencilStateInfo.depthWriteEnable = true;
    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateInfo.back.compareMask = 0;
    depthStencilStateInfo.back.reference = 0;
    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilStateInfo.back.writeMask = 0;
    depthStencilStateInfo.minDepthBounds = 0;
    depthStencilStateInfo.maxDepthBounds = 0;
    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo.front = depthStencilStateInfo.back;

    // Setup multi sampling. In our first example we will be using single sampling mode
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = NUM_SAMPLES;

    // Setup color output masks.
    // Set to write out RGBA components
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;

    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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

    // Setup the push constant range
    const unsigned pushConstantRangeCount = 1;
    VkPushConstantRange pushConstantRanges[pushConstantRangeCount] = {};
    pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRanges[0].offset = 0;
    pushConstantRanges[0].size = 16 + sizeof(glm::mat4);
    //pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //pushConstantRanges[1].offset = 16;
    //pushConstantRanges[1].size = sizeof(glm::mat4);

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)CDS->descLayout.size();
    pipelineLayoutInfo.pSetLayouts = CDS->descLayout.data();

    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkResult vkResult = vkCreatePipelineLayout(m_VulkanApplication->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
        assert(false);
    }

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
    pipelineInfo.layout = graphicsPipelineLayout;
    pipelineInfo.renderPass = m_VulkanApplication->m_hRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    vkResult = vkCreateGraphicsPipelines(m_VulkanApplication->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (vkResult != VK_SUCCESS)
    {
        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
        assert(false);
    }

    m_GraphicsPipelineMap[PIPELINE_RECT_FILLED] = qMakePair(graphicsPipeline, graphicsPipelineLayout);

    // Cleanup
    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, vertShader, nullptr);
}

void CircleMultiDrawFactory::CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap)
{
    if (p_ClearGraphicsPipelineMap)
    {
        const VkDevice& device = m_VulkanApplication->m_hDevice;
        QMap<QString, QPair<VkPipeline, VkPipelineLayout> >::iterator i;
        for (i = m_GraphicsPipelineMap.begin(); i != m_GraphicsPipelineMap.end(); ++i)
        {
            vkDestroyPipeline(m_VulkanApplication->m_hDevice, i.value().first, nullptr);
            vkDestroyPipelineLayout(device, i.value().second, nullptr);
        }

        m_GraphicsPipelineMap.clear();
    }

    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
    if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_FILLED))
    {
        graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].first;
        graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].second;
        return;
    }

    CreateRectFillPipeline();
    CreateRectOutlinePipeline();
}

void CircleMultiDrawFactory::createPushConstants()
{
    return;
    VkCommandBuffer copyCmd;
    VulkanHelper::AllocateCommandBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, &copyCmd);
    VulkanHelper::BeginCommandBuffer(copyCmd);

    enum ColorFlag {
        RED = 1,
        GREEN = 2,
        BLUE = 3,
        MIXED_COLOR = 4,
    };

    float mixerValue = 0.3f;
    unsigned constColorRGBFlag = BLUE;

    // Create push constant data, this contain a constant
    // color flag and mixer value for non-const color
    unsigned pushConstants[2] = {};
    pushConstants[0] = constColorRGBFlag;
    memcpy(&pushConstants[1], &mixerValue, sizeof(float));

    // Check if number of push constants does not exceed the allowed size
    int maxPushContantSize = m_VulkanApplication->m_physicalDeviceInfo.prop.limits.maxPushConstantsSize;
    if (sizeof(pushConstants) > maxPushContantSize) {
        assert(0);
        printf("Push constand size is greater than expected, max allow size is %d", maxPushContantSize);
    }

    //for each (VulkanDrawable* drawableObj in drawableList)
    //{
    //    vkCmdPushConstants(cmdPushConstant, drawableObj->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants);
    //}

    VulkanHelper::EndCommandBuffer(copyCmd);
    VulkanHelper::SubmitCommandBuffer(m_VulkanApplication->m_hGraphicsQueue, copyCmd);
    vkFreeCommandBuffers(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, 1, &copyCmd);

    //CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdPushConstant);
    //CommandBufferMgr::beginCommandBuffer(cmdPushConstant);


    //CommandBufferMgr::endCommandBuffer(cmdPushConstant);
    //CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdPushConstant);
}

void CircleMultiDrawFactory::RecordCommandBuffer(VkCommandBuffer& p_CommandBuffer)
{
    //// Specify the clear color value
    //VkClearValue clearColor[2];
    //clearColor[0].color.float32[0] = 0.0f;
    //clearColor[0].color.float32[1] = 0.0f;
    //clearColor[0].color.float32[2] = 0.0f;
    //clearColor[0].color.float32[3] = 0.0f;

    //// Specify the depth/stencil clear value
    //clearColor[1].depthStencil.depth = 1.0f;
    //clearColor[1].depthStencil.stencil = 0;

    //// Offset to render in the frame buffer
    //VkOffset2D   renderOffset = { 0, 0 };
    //// Width & Height to render in the frame buffer
    //VkExtent2D   renderExtent = m_VulkanApplication->m_swapChainExtent;

    //// For each command buffers in the command buffer list
    //for (size_t i = 0; i < m_VulkanApplication->m_hCommandBufferList.size(); i++)
    //{
    //    VkCommandBufferBeginInfo beginInfo = {};
    //    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //    // Indicate that the command buffer can be resubmitted to the queue
    //    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    //    // Begin command buffer
    //vkBeginCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i], &beginInfo);
    //Render(p_CommandBuffer);
    //    VkRenderPassBeginInfo renderPassInfo = {};
    //    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //    renderPassInfo.renderPass = m_VulkanApplication->m_hRenderPass;
    //    renderPassInfo.framebuffer = m_VulkanApplication->m_hFramebuffers[i];
    //    renderPassInfo.renderArea.offset = renderOffset;
    //    renderPassInfo.renderArea.extent = renderExtent;
    //    renderPassInfo.clearValueCount = 2;
    //    renderPassInfo.pClearValues = clearColor;

    //    // Begin render pass
    //    vkCmdBeginRenderPass(m_VulkanApplication->m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    Render(p_CommandBuffer); // consider using shared ptr/smart pointers

    //                                                          // End the Render pass
    //    vkCmdEndRenderPass(m_VulkanApplication->m_hCommandBufferList[i]);

    //    // End the Command buffer
    //    VkResult vkResult = vkEndCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i]);
    //    if (vkResult != VK_SUCCESS)
    //    {
    //        VulkanHelper::LogError("vkEndCommandBuffer() failed!");
    //        assert(false);
    //    }
    //}
}

void CircleMultiDrawFactory::CreateVertexBuffer()
{
    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        if (pipelineIdx == PIPELINE_FILLED)
        {
            //VkVertexInputBindingDescription		m_VertexInputBinding[1];   // 0 for (position and color)
            //VkVertexInputAttributeDescription	m_VertexInputAttribute[2]; // Why 2 = 2(for position and color)

            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(3); // Why 2 = 2(for position and color

                                                           // Indicates the rate at which the information will be
                                                           // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(Vertex);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);

            m_VertexInputAttribute[pipelineIdx][2].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][2].location = 2;
            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32_UINT;
            m_VertexInputAttribute[pipelineIdx][2].offset = offsetof(struct Vertex, m_DrawType);
        }
        else if (pipelineIdx == PIPELINE_OUTLINE)
        {
            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
            m_VertexInputAttribute[pipelineIdx].resize(2); // Why 2 = 2(for position and color

                                                           // Indicates the rate at which the information will be
                                                           // injected for vertex input.
            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(Vertex);

            // The VkVertexInputAttribute interpreting the data.
            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][0].location = 0;
            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
            m_VertexInputAttribute[pipelineIdx][1].location = 1;
            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);
        }
    }
}

void CircleMultiDrawFactory::UpdateModelList(Model3D *p_Item)
{
    RectangleModel* rectangle = dynamic_cast<RectangleModel*>(p_Item);
    assert(rectangle);

    // Note: Based on the draw type push the model in respective pipelines
    // Keep the draw type loose couple with the pipeline type,
    // they may be in one-to-one correspondence but that is not necessary.
    switch (rectangle->GetDrawType())
    {
    case RectangleModel::FILLED:
        m_PipelineTypeModelVector[PIPELINE_FILLED].push_back(p_Item);
        break;

    case RectangleModel::OUTLINE:
        m_PipelineTypeModelVector[PIPELINE_OUTLINE].push_back(p_Item);
        break;

    case RectangleModel::ROUNDED:
        // TODO
        break;

    default:
        break;
    }
}

void CircleMultiDrawFactory::Prepare(Scene3D* p_Scene)
{
    // Update the uniform
    //if (!CDS)
    //{
    //	CDS = std::make_shared<RectangleDescriptorSet>(m_VulkanApplication);
    //	CDS->CreateDescriptor();
    //	UniformBuffer = CDS->UniformBuffer;
    //}

    //VulkanHelper::WriteMemory(m_VulkanApplication->m_hDevice,
    //	UniformBuffer->m_MappedMemory,
    //	UniformBuffer->m_MappedRange,
    //	UniformBuffer->m_BufObj.m_MemoryFlags,
    //	&m_Transform, sizeof(m_Transform));
}

void CircleMultiDrawFactory::Render(VkCommandBuffer& p_CmdBuffer)
{
    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
    {
        ModelVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
        const int modelSize = m_ModelList.size();
        if (!modelSize) continue;

        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
        if (pipelineIdx == PIPELINE_FILLED)
        {
            if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_FILLED))
            {
                graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].first;
                graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].second;
            }
        }
        else if (pipelineIdx == PIPELINE_OUTLINE)
        {
            if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_OUTLINE))
            {
                graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE].first;
                graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE].second;
            }
        }
        else
        {
            assert(false);
        }

        vkCmdBindDescriptorSets(p_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout, 0, 1, CDS->descriptorSet.data(), 0, NULL);
        vkCmdBindPipeline(p_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        for (int j = 0; j < modelSize; j++)
        {
            RectangleModel* model = (static_cast<RectangleModel*>(m_ModelList.at(j)));
            if (!model) continue;

            //////////////////////////////////////////////////////////////////////////////////
            struct pushConst
            {
                glm::vec4 inColor;
                glm::mat4 modelMatrix;
            }PC;

            PC.inColor = model->GetColor();
            //PC.modelMatrix = /*(*model->GetScene()->GetProjection()) * (*model->GetScene()->GetView()) */ model->GetTransformedModel();
            //PC.modelMatrix = (*GetProjection()) * (*GetView()) model->GetModel();// GetTransformedModel();

            // Check if number of push constants does not exceed the allowed size
            int maxPushContantSize = m_VulkanApplication->m_physicalDeviceInfo.prop.limits.maxPushConstantsSize;
            if (sizeof(PC) > maxPushContantSize) {
                printf("Push constand size is greater than expected, max allow size is %d", maxPushContantSize);
                assert(0);
            }

            vkCmdPushConstants(p_CmdBuffer, graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &PC);
            ////////////////////////////////////////////////////////////////////////////////

            if (model->GetDrawType() == RectangleModel::FILLED)
            {

                // Specify vertex buffer information
                const VkDeviceSize offsets[1] = { 0 };
                vkCmdBindVertexBuffers(p_CmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &model->m_VertexBuffer.m_Buffer, offsets);

                // Draw the Cube
                const int vertexCount = sizeof(rectFilledVertices) / sizeof(Vertex);
                vkCmdDraw(p_CmdBuffer, vertexCount, /*INSTANCE_COUNT*/1, 0, 0);
            }
            else if (model->GetDrawType() == RectangleModel::OUTLINE)
            {
                // Specify vertex buffer information
                const VkDeviceSize offsets[1] = { 0 };
                vkCmdBindVertexBuffers(p_CmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &model->m_VertexBuffer.m_Buffer, offsets);

                // Draw the Cube
                const int vertexCount = sizeof(rectOutlineVertices) / sizeof(Vertex);
                vkCmdDraw(p_CmdBuffer, vertexCount, /*INSTANCE_COUNT*/1, 0, 0);

            }
        }

    }
}
