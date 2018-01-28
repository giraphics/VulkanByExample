#pragma once
#include "VulkanHelper.h"


// Base class for Vulkan application
class VulkanApp : public VulkanHelper
{
public:
    VulkanApp();
	virtual ~VulkanApp();

    void Initialize(); // Initialize the Vulkan application
};