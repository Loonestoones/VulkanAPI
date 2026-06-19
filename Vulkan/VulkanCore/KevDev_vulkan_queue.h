
#pragma once

#include <stdio.h>

#include <vulkan/vulkan.h>
#include <vector>
#include "KevDev_types.h"

namespace KevDevVK{

class VulkanQueue {

public:
	VulkanQueue() {}
	~VulkanQueue() {}

	void Init(VkDevice Device, VkSwapchainKHR SwapChain, u32 QueueFamily, u32 QueueIndex);

	void Destroy();

	u32 AcquireNextImage();

	void SubmitSync(VkCommandBuffer CmdBuf);

	void SubmitAsync(VkCommandBuffer CmdBuf);

	void SubmitAsync(VkCommandBuffer* pCmdBufs, int NumCmdBufs);

void Present(u32 ImageIndex);

	void WaitIdle();

	VkQueue GetHandle() const { return m_queue; }

private:

	void CreateSyncObjects();

	VkDevice m_device = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
	VkQueue m_queue = VK_NULL_HANDLE;
	std::vector<VkSemaphore> m_imageAvailableSems;
	std::vector<VkSemaphore> m_renderFinishedSems;
	std::vector<VkFence> m_inFlightFences;
	u32 m_numImages = 0;
	u32 m_frameIndex = 0;
	u32 m_acquiredImageIndex = 0;
};

}
