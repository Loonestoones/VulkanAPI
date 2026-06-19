#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "KevDev_types.h"

namespace KevDevVK {

    void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags);


    VkSemaphore CreateSemaphore(VkDevice Device);

    void ImageMemBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format,
					 VkImageLayout OldLayout, VkImageLayout NewLayout);

    VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format,
						    VkImageAspectFlags AspectFlags);


    
VkSampler CreateTextureSampler(VkDevice Device, VkFilter MinFilter, VkFilter MaxFilter,
							   VkSamplerAddressMode AddressMode);

}
