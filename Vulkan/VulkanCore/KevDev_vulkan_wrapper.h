#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "KevDev_types.h"

namespace KevDevVK {

    void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags);


    VkSemaphore CreateSemaphore(VkDevice Device);

}
