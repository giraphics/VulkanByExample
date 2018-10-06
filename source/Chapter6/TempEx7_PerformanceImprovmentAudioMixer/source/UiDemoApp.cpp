#include "UIDemoApp.h"
#include "UIDemo.h"
#include "UiMetalPaintEngine.h"
#include "Rectangle/Rect.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication qtApp(argc, argv);

    UIDemoApp* app = new UIDemoApp();
    app->EnableDepthBuffer(true);
    app->EnableWindowResize(true);
    app->Initialize();
    app->m_pWindow->show();
    qtApp.exec();

    delete app;
    return 0;
}

UIDemoApp::UIDemoApp()
{
    VulkanHelper::GetInstanceLayerExtensionProperties();
}

UIDemoApp::~UIDemoApp()
{
    delete m_Scene;
    delete m_ScenePainterEngine;
}

void UIDemoApp::Configure()
{
    SetApplicationName("Metal performance test");
    SetWindowDimension(1200 , 800);

#ifdef _WIN32
    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Add Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
   #ifdef _WIN64
   #else
   #endif
#elif __APPLE__
    AddInstanceExtension("VK_KHR_surface");
    AddInstanceExtension("VK_MVK_macos_surface");
#endif

    // m_SceneVector.push_back(std::make_shared<Scene>(this));
    m_Scene = new Scene(this);//m_SceneVector[0].get();
    m_ScenePainterEngine = new Scene(this);//m_SceneVector[0].get();

    InitMetalEngine();

//    Node* m_Parent = new Rectangl(m_Scene, NULL, BoundingRegion(300, 300, 200, 200), "Node 1", SHAPE::SHAPE_RECTANGLE_INSTANCED);
//    m_Parent->SetColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
//    m_Parent->SetDefaultColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
//    m_Parent->SetMemPoolIdx(0);

//    Node* m_Parent1 = new Rectangl(m_ScenePainterEngine, NULL, BoundingRegion(400, 400, 200, 200), "Node 1", SHAPE::SHAPE_RECTANGLE_INSTANCED);
//    m_Parent1->SetColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
//    m_Parent1->SetDefaultColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
//    m_Parent1->SetMemPoolIdx(1);
//    m_Parent1->SetZOrder(10);

     m_UIDemo.Grid(m_Scene, m_windowDim.width, m_windowDim.height);             // Grid demo
//     m_UIDemo.ProgressBarFunc(m_Scene);                                         // Progress bar
     //m_UIDemo.MixerView(m_Scene, m_windowDim.width, m_windowDim.height);        // Mixer View demo
     //m_UIDemo.InitTransformationConformTest(m_Scene);                           // Transformation test demo
}

void UIDemoApp::Setup()
{
    m_Scene->SetUpProjection();
    m_ScenePainterEngine->SetUpProjection();
    m_Scene->Setup();
    m_ScenePainterEngine->Setup();

    RecordRenderPass(1, SG_STATE_SETUP); // Parminder: Double check if this is required

    // At least update the scene once so that in case UpdateMeAndMyChildren() is being used it has all transformation readily available
    m_Scene->Update();
    m_ScenePainterEngine->Update();
}

void UIDemoApp::Update()
{
    m_UIDemo.UpdateTransformationConformTest();
    m_Scene->Update();
    m_ScenePainterEngine->Update();
}

bool UIDemoApp::Render()
{
    QRectF rect;
    m_MetalPaintEngine->drawRects(&rect, 1);

    RecordRenderPass(1, SG_STATE_RENDER);

    return VulkanApp::Render();
}

void UIDemoApp::mousePressEvent(QMouseEvent* p_Event)
{
    m_Scene->mousePressEvent(p_Event);
}

void UIDemoApp::mouseReleaseEvent(QMouseEvent* p_Event)
{
    m_Scene->mouseReleaseEvent(p_Event);
}

void UIDemoApp::mouseMoveEvent(QMouseEvent* p_Event)
{
    m_Scene->mouseMoveEvent(p_Event);
}

void UIDemoApp::ResizeWindow(int p_Width, int p_Height)
{
    VulkanApp::ResizeWindow(p_Width, p_Height);

    RecordRenderPass(3, SG_STATE_RESIZE, p_Width, p_Height);
}

bool UIDemoApp::InitMetalEngine()
{
    if (!m_MetalPaintEngine)
    {
        m_MetalPaintEngine.reset(new UiMetalPaintEngine());

        return m_MetalPaintEngine->Init(m_ScenePainterEngine);
    }

    return true;
}

void UIDemoApp::RecordRenderPass(int p_Argcount, ...)
{
    va_list list;
    va_start(list, p_Argcount);
    SCENE_GRAPH_STATES currentState = SG_STATE_NONE;
    QSize resizedDimension;

    for (int i = 0; i < p_Argcount; ++i)
    {
        switch (i)
        {
        case 0:
            currentState = static_cast<SCENE_GRAPH_STATES>(va_arg(list, int));
            break;

        case 1:
            resizedDimension.setWidth(va_arg(list, int));
            break;

        case 2:
            resizedDimension.setHeight(va_arg(list, int));
            break;

        default:
        {
            if (currentState == SG_STATE_NONE)
            {
                va_end(list);
                return;
            }
            break;
        }
        }
    }
    va_end(list);

    // Specify the clear color value
    VkClearValue clearColor[2];
    clearColor[0].color.float32[0] = 0.0f;
    clearColor[0].color.float32[1] = 0.0f;
    clearColor[0].color.float32[2] = 0.0f;
    clearColor[0].color.float32[3] = 0.0f;

    // Specify the depth/stencil clear value
    clearColor[1].depthStencil.depth = 1.0f;
    clearColor[1].depthStencil.stencil = 0;

    // Offset to render in the frame buffer
    VkOffset2D   renderOffset = { 0, 0 };
    // Width & Height to render in the frame buffer
    VkExtent2D   renderExtent = m_swapChainExtent;

    // For each command buffers in the command buffer list
    for (size_t i = 0; i < m_hCommandBufferList.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // Indicate that the command buffer can be resubmitted to the queue
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // Begin command buffer
        vkBeginCommandBuffer(m_hCommandBufferList[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_hRenderPass;
        renderPassInfo.framebuffer = m_hFramebuffers[i];
        renderPassInfo.renderArea.offset = renderOffset;
        renderPassInfo.renderArea.extent = renderExtent;
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearColor;

        // Begin render pass
        vkCmdBeginRenderPass(m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        switch (currentState)
        {
        case SG_STATE_SETUP:
        case SG_STATE_RENDER:
            m_Scene->Render(m_hCommandBufferList[i]);
            m_ScenePainterEngine->Render(m_hCommandBufferList[i]);
            break;

        case SG_STATE_RESIZE:
            m_Scene->Resize(m_hCommandBufferList[i], resizedDimension.width(), resizedDimension.height());
            m_ScenePainterEngine->Resize(m_hCommandBufferList[i], resizedDimension.width(), resizedDimension.height());
            break;

        default:
            break;
        }

        // End the Render pass
        vkCmdEndRenderPass(m_hCommandBufferList[i]);

        // End the Command buffer
        VkResult vkResult = vkEndCommandBuffer(m_hCommandBufferList[i]);
        if (vkResult != VK_SUCCESS)
        {
            VulkanHelper::LogError("vkEndCommandBuffer() failed!");
            assert(false);
        }
    }
}
