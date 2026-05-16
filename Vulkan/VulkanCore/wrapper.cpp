#include "KevDev_vulkan_util.h"
#include "KevDev_vulkan_wrapper.h"

namespace KevDevVK {

    void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags)
    {
	VkCommandBufferBeginInfo BeginInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .pNext = nullptr,
	    .flags = UsageFlags,
	    .pInheritanceInfo = nullptr
	};

	VkResult res = vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
	CHECK_VK_RESULT(res, "vkBeginCommandBuffer\n");

    }


    VkSemaphore CreateSemaphore(VkDevice Device)
    {
	VkSemaphoreCreateInfo CreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	    .pNext = nullptr,
	    .flags = 0
	};

	VkSemaphore Semaphore;
	VkResult Res = vkCreateSemaphore(Device, &CreateInfo, nullptr, &Semaphore);
	CHECK_VK_RESULT(Res, "vkCreateSemaphore");
	return Semaphore;
    }

}
