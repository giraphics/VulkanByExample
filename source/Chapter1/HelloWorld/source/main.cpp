#include "HelloVulkanApp.h"

VulkanApp* g_pApp = NULL;

int main()
{
    // Create a new instance of HelloVulanApp object
    g_pApp = new HelloVulkanApp();

    if (g_pApp)
    {
        // Initialize the application
        if (g_pApp->Init())
        {
            // Enter the rendering loop
            g_pApp->Run();

            // Call application to cleanup
            g_pApp->Cleanup();
        }
    }
    return 0;
}
