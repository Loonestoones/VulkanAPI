#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "KevDev_vulkan_core.h"
#include "KevDev_vulkan_shader.h"
#include "KevDev_vulkan_graphics_pipeline.h"
#include "KevDev_vulkan_simple_mesh.h"

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

        VulkanApp()
        {
        }

        ~VulkanApp()
        {
                m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());
                m_vkCore.DestroyFramebuffers(m_frameBuffers);
                vkDestroyShaderModule(m_vkCore.GetDevice(), m_vs, nullptr);
                vkDestroyShaderModule(m_vkCore.GetDevice(), m_fs, nullptr);
		delete m_pPipeline;
                vkDestroyRenderPass(m_vkCore.GetDevice(), m_renderPass, NULL);
		m_mesh.Destroy(m_device);
        }

        void Init(const char* pAppName, GLFWwindow* pWindow)
        {
		m_pWindow = pWindow;
                m_vkCore.Init(pAppName, pWindow);
		m_device = m_vkCore.GetDevice();
                m_numImages = m_vkCore.GetNumImages();
                m_pQueue = m_vkCore.GetQueue();
                m_renderPass = m_vkCore.CreateSimpleRenderPass();
                m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);
                CreateShaders();
		CreateVertexBuffer();
		CreateUniformBuffers();
		CreatePipeline();
		CreateCommandBuffers();
                RecordCommandBuffers();
		//DefaultCreateCameraPers();
		// object ready to receive callbacks
		//KevDevVK::glfw_vulkan_set_callbacks(m_pWindow, this);

        }

        void RenderScene()
        {
                u32 ImageIndex = m_pQueue->AcquireNextImage();

		UpdateUniformBuffers(ImageIndex);

                m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);

                m_pQueue->Present(ImageIndex);

                m_pQueue->WaitIdle();
        }

private:
        void CreateCommandBuffers()
        {
                m_cmdBufs.resize(m_numImages);
                m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.data());

                printf("Created command buffers\n");
        }

	void CreateVertexBuffer()
	{
		struct Vertex {
			Vertex(const glm::vec3& p, const glm::vec2& t)
			{
				Pos = p;
				Tex = t;
			}

			glm::vec3 Pos;
			glm::vec2 Tex;
		};

		std::vector<Vertex> Vertices = {
			Vertex({-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}),	// top left
			Vertex({1.0f, -1.0f, 0.0f},   {0.0f, 1.0f}),	// top right
			Vertex({0.0f,  1.0f, 0.0f},   {1.0f, 1.0f}) 	// bottom middle
		};

		m_mesh.m_vertexBufferSize = sizeof(Vertices[0]) * Vertices.size();
		m_mesh.m_vb = m_vkCore.CreateVertexBuffer(Vertices.data(), m_mesh.m_vertexBufferSize);
	}

	struct UniformData {
	    glm::mat4 WVP;
	};

	void CreateUniformBuffers()
	{
	    m_uniformBuffers = m_vkCore.CreateUniformBuffers(sizeof(UniformData));
	}

        void CreateShaders()
        {
            // Load pre-compiled SPIR-V shaders from binary files
            m_vs = KevDevVK::CreateShaderModuleFromBinary(m_vkCore.GetDevice(), "shader/test.vert.spv");

            m_fs = KevDevVK::CreateShaderModuleFromBinary(m_vkCore.GetDevice(), "shader/test.frag.spv");
        }

	void CreatePipeline()
	{
	    m_pPipeline = new KevDevVK::GraphicsPipeline(m_device, m_pWindow, m_renderPass, m_vs, m_fs,
		    &m_mesh, m_numImages, m_uniformBuffers, sizeof(UniformData));
	}

        void RecordCommandBuffers()
        {
                VkClearColorValue ClearColor = { 1.0f, 0.0f, 0.0f, 0.0f };
                VkClearValue ClearValue;
                ClearValue.color = ClearColor;

                VkRenderPassBeginInfo RenderPassBeginInfo = {
                        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .pNext = NULL,
                        .renderPass = m_renderPass,
                        .renderArea = {
                                .offset = {
                                        .x = 0,
                                        .y = 0
                                },
                                .extent = {
                                        .width = WINDOW_WIDTH,
                                        .height = WINDOW_HEIGHT
                                }
                        },
                        .clearValueCount = 1,
                        .pClearValues = &ClearValue
                };

                for (uint i = 0; i < m_cmdBufs.size(); i++) {
                        KevDevVK::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

                        RenderPassBeginInfo.framebuffer = m_frameBuffers[i];

                        vkCmdBeginRenderPass(m_cmdBufs[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			m_pPipeline->Bind(m_cmdBufs[i], i);

			u32 VertexCount = 3;
			u32 InstanceCount = 1;
			u32 FirstVertex = 0;
			u32 FirstInstance = 0;

			vkCmdDraw(m_cmdBufs[i], VertexCount, InstanceCount, FirstVertex, FirstInstance);

                        vkCmdEndRenderPass(m_cmdBufs[i]);

                        VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
                        CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
                }

                printf("Command buffers recorded\n");
        }


	void UpdateUniformBuffers(uint32_t ImageIndex)
	{		
		static float foo = 0.0f;
		glm::mat4 Rotate = glm::mat4(1.0);
		Rotate = glm::rotate(Rotate, glm::radians(foo), glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)));
		foo += 0.001f;

		glm::mat4 VP = m_pGameCamera->GetVPMatrix();

		glm::mat4 WVP = VP * Rotate;
		m_uniformBuffers[ImageIndex].Update(m_device, &WVP, sizeof(WVP));
	}

	GLFWwindow* m_pWindow = nullptr;
        KevDevVK::VulkanCore m_vkCore;
        KevDevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = nullptr;
        int m_numImages = 0;
        std::vector<VkCommandBuffer> m_cmdBufs;
        VkRenderPass m_renderPass;
        std::vector<VkFramebuffer> m_frameBuffers;
        VkShaderModule m_vs;
        VkShaderModule m_fs;
	KevDevVK::GraphicsPipeline* m_pPipeline = nullptr;
	KevDevVK::SimpleMesh m_mesh;
	std::vector<KevDevVK::BufferAndMemory> m_uniformBuffers;
	GLMCameraFirstPerson* m_pGameCamera = nullptr;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
};


#define APP_NAME "North Sea Engine"

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

        GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, NULL, NULL);

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

