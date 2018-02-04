#pragma once
#include "../../../common/VulkanApp.h"

class Cube;
class MyFirst3DAppWithDepthBuffer : public VulkanApp
{
public:
    MyFirst3DAppWithDepthBuffer();
    virtual ~MyFirst3DAppWithDepthBuffer();

    // Core virtual methods used by derived classes
    virtual void Configure();
    virtual void Setup();
    virtual void Update();

private:
    Cube* m_Cube;
};

