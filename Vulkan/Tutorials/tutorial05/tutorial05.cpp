#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "KevDev_vulkan_core.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

class VulkanApp
{
    public:
	VulkanApp() : m_vkCore()
	{
	}

	// desctructor below, consturctor above
	~VulkanApp()
	{
	}

	void Init(const char* pAppName, GLFWwindow* pWindow)
	{
	    m_vkCore.Init(pAppName, pWindow);
	    m_numImages = m_vkCore.GetNumImages();
	    m_pQueue = m_vkCore.GetQueue();
	    CreateCommandBuffers();
	    RecordCommandBuffers();
	}

	void RenderScene()
	{
	    u32 ImageIndex = m_pQueue->AcquireNextImage();

	    m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);

	    m_pQueue->Present(ImageIndex);
	}

    private:
	void CreateCommandBuffers()
	{
	    m_cmdBufs.resize(m_numImages);
	    m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.data());
	}

	void RecordCommandBuffers()
	{
	    VkClearColorValue ClearColor = { 1.0f, 0.0f, 0.0f, 0.0f };

	    VkImageSubresourceRange ImageRange = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	    };

	    for (uint i = 0; i < m_cmdBufs.size(); i++) {
		KevDevVK::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		vkCmdClearColorImage(m_cmdBufs[i], m_vkCore.GetImage(i), VK_IMAGE_LAYOUT_GENERAL, &ClearColor, 1, &ImageRange);

		VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
		CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
	    }

	    printf("Command buffers rescorded\n");
	}

	KevDevVK::VulkanCore m_vkCore;
	KevDevVK::VulkanQueue* m_pQueue = nullptr;
	int m_numImages = 0;
	std::vector<VkCommandBuffer> m_cmdBufs;
};

#define APP_NAME "Tutorial 05"

int main(int argc, char* argv[])
{
	if (!glfwInit()) {
		return 1;
	}

	if (!glfwVulkanSupported()) {
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, nullptr, nullptr);
	
	if (!pWindow) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(pWindow, GLFW_KeyCallback);

	VulkanApp App;
	App.Init(APP_NAME, pWindow);

	while (!glfwWindowShouldClose(pWindow)) {
		App.RenderScene();
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
