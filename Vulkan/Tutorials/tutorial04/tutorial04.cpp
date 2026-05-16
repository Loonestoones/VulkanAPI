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
	    CreateCommandBuffers();
	}

	void RenderScene()
	{
	}

    private:
	void CreateCommandBuffers()
	{
	    m_cmdBufs.resize(m_numImages);
	    m_vkCore.CreateCommandBuffers(m_numImages, &m_cmdBufs.data());
	}

	KevDevVK::VulkanCore m_vkCore;
	int m_numImages = 0;
	std::vector<VkCommandBuffer> m_cmdBufs;
};

#define APP_NAME "Tutorial 03"

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
