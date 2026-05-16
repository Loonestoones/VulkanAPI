#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "KevDev_vulkan_util.h"
#include "KevDev_vulkan_device.h"
#include "KevDev_vulkan_wrapper.h"
#include "KevDev_vulkan_queue.h"


namespace KevDevVK {

    class BufferAndMemory {
public:
	BufferAndMemory() {}

	VkBuffer m_buffer = NULL;
	VkDeviceMemory m_mem = NULL;
	VkDeviceSize m_allocationSize = 0;

	void Update(VkDevice Device, const void* pData, size_t Size);

	void Destroy(VkDevice Device);
};


enum RenderPassType {
	RenderPassTypeDefault = 0x0,
	RenderPassTypeFirst = 0x01,
	RenderPassTypeLast = 0x02,
	RenderPassTypeOffscreen = 0x04,
	RenderPassTypeOffscreenInternal = 0x08,
};



    class VulkanCore {
	public:
	    VulkanCore();

	    ~VulkanCore();

	    void Init(const char* pAppName, GLFWwindow* pWindow);

	    VkRenderPass CreateSimpleRenderPass();

	    std::vector<VkFramebuffer> CreateFramebuffers(VkRenderPass RenderPass);

	    void DestroyFramebuffers(std::vector<VkFramebuffer>& Framebuffers);

	    VkDevice& GetDevice() { return m_device; }

	    int GetNumImages() const { return (int)m_images.size(); }

	    const VkImage& GetImage(int Index) const;

	    const VkImageView& GetDepthView(int Index) const;

	    const VkImageView& GetImageView(int Index) const;

	    VulkanQueue* GetQueue() { return &m_queue; }
	    void CreateCommandBuffers(u32 Count, VkCommandBuffer* pCmdBufs);
	    void FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs);

	    std::vector<BufferAndMemory> CreateUniformBuffers(size_t Size);

	    void GetFramebufferSize(int& Width, int& Height) const;

	    BufferAndMemory CreateVertexBuffer(const void* pVertices, size_t Size);

	private:
	    void UpdateInstanceVersion();
	    void CreateInstance(const char* pAppName);
	    void CreateDebugCallback();
	    void CreateSurface();
	    void CreateDevice();
	    void CreateSwapChain();
	    void CreateCommandBufferPool();	
	    BufferAndMemory CreateUniformBuffer(size_t Size);
	    BufferAndMemory CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);

	    u32 GetMemoryTypeIndex(u32 memTypeBits, VkMemoryPropertyFlags memPropFlags);

	    void CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size);

	    BufferAndMemory CreateBuffer(VkDevice Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);

	    VkInstance m_instance = VK_NULL_HANDLE;
	    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	    GLFWwindow* m_pWindow = nullptr;
	    VkSurfaceKHR m_surface;
	    VulkanPhysicalDevices m_physDevices;
	    u32 m_queueFamily = 0;
	    VkDevice m_device;

	    struct {
		int Major = 0;
		int Minor = 0;
		int Patch = 0;
	    } m_instanceVersion;

	    VkSurfaceFormatKHR m_swapChainSurfaceFormat = {};
	    VkSwapchainKHR m_swapChain;
	    std::vector<VkImage> m_images;
	    std::vector<VkImageView> m_imageViews;
	    VkCommandPool m_cmdBufPool = VK_NULL_HANDLE;
	    VulkanQueue m_queue;
	    VkCommandBuffer m_copyCmdBuf = VK_NULL_HANDLE;
	    std::vector<VkFramebuffer> m_frameBuffers;

    };
}

