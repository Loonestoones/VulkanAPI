#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "KevDev_vulkan_core.h"
#include "KevDev_vulkan_shader.h"

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
                vkDestroyRenderPass(m_vkCore.GetDevice(), m_renderPass, NULL);
        }

        void Init(const char* pAppName, GLFWwindow* pWindow)
        {
                m_vkCore.Init(pAppName, pWindow);
                m_numImages = m_vkCore.GetNumImages();
                m_pQueue = m_vkCore.GetQueue();
                m_renderPass = m_vkCore.CreateSimpleRenderPass();
                m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);
                CreateCommandBuffers();
                RecordCommandBuffers();
                CreateShaders();
        }

        void RenderScene()
        {
                u32 ImageIndex = m_pQueue->AcquireNextImage();

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

        void CreateShaders()
        {
            // Load pre-compiled SPIR-V shaders from binary files
            m_vs = KevDevVK::CreateShaderModuleFromBinary(m_vkCore.GetDevice(), "shader/test.vert.spv");

            m_fs = KevDevVK::CreateShaderModuleFromBinary(m_vkCore.GetDevice(), "shader/test.frag.spv");
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

                        vkCmdEndRenderPass(m_cmdBufs[i]);

                        VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
                        CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
                }

                printf("Command buffers recorded\n");
        }

        KevDevVK::VulkanCore m_vkCore;
        KevDevVK::VulkanQueue* m_pQueue = NULL;
        int m_numImages = 0;
        std::vector<VkCommandBuffer> m_cmdBufs;
        VkRenderPass m_renderPass;
        std::vector<VkFramebuffer> m_frameBuffers;
        VkShaderModule m_vs;
        VkShaderModule m_fs;
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
