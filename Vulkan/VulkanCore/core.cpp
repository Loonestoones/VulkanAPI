#include <vector>
#include <assert.h>
#include <vulkan/vulkan_core.h>

#include "KevDev_vulkan_core.h"
#include "KevDev_vulkan_util.h"
#include "KevDev_types.h"
#include "KevDev_util.h"
#include "KevDev_vulkan_device.h"
#include "KevDev_vulkan_wrapper.h" 

namespace KevDevVK {

    VulkanCore::VulkanCore()
    {
    }

    VulkanCore::~VulkanCore()
    {
	printf("-------------------------------\n");

	vkFreeCommandBuffers(m_device, m_cmdBufPool, 1, &m_copyCmdBuf);

	vkDestroyCommandPool(m_device, m_cmdBufPool, nullptr);

	m_queue.Destroy();

	for (int i = 0; i < m_imageViews.size(); i++) {
	    vkDestroyImageView(m_device, m_imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

	vkDestroyDevice(m_device, nullptr);

	PFN_vkDestroySurfaceKHR vkDestroySurface = VK_NULL_HANDLE;
	vkDestroySurface = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkDestroySurfaceKHR");
	if (!vkDestroySurface) {
	    OGLDEV_ERROR("Cannot find address of vkDestroyDebugUtilsMessenger\n");
	    exit(1);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	printf("GLFW window surface destroyed\n");

	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
	vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
	if (!vkDestroyDebugUtilsMessenger) {
	    OGLDEV_ERROR("Cannot find adress of vkDestroyDebugUtilsMessenger\n");
	    exit(1);
	}

	printf("Debug callback destroyed\n");

	vkDestroyInstance(m_instance, nullptr);
	printf("Vulkan instance destroyed\n");
    }

    void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow)
    {
	m_pWindow = pWindow;
	CreateInstance(pAppName);
	CreateDebugCallback();
	CreateSurface();
	m_physDevices.Init(m_instance, m_surface);
	m_queueFamily = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
	CreateDevice();
	CreateSwapChain();
	CreateCommandBufferPool();
	m_queue.Init(m_device, m_swapChain, m_queueFamily, 0);
	CreateCommandBuffers(1, &m_copyCmdBuf);
    }


    
const VkImage& VulkanCore::GetImage(int Index) const
{
	if (Index >= m_images.size()) {
		OGLDEV_ERROR("Invalid image index %d\n", Index);
		exit(1);
	}

	return m_images[Index];
}


const VkImageView& VulkanCore::GetImageView(int Index) const
{
	if (Index >= m_imageViews.size()) {
		OGLDEV_ERROR("Invalid image view index %d\n", Index);
		exit(1);
	}

	return m_imageViews[Index];
}


    

    void VulkanCore::UpdateInstanceVersion()
{
	u32 InstanceVersion = 0;

	VkResult res = vkEnumerateInstanceVersion(&InstanceVersion);
	CHECK_VK_RESULT(res, "vkEnumerateInstanceVersion");

	m_instanceVersion.Major = VK_API_VERSION_MAJOR(InstanceVersion);
	m_instanceVersion.Minor = VK_API_VERSION_MINOR(InstanceVersion);
	m_instanceVersion.Patch = VK_API_VERSION_PATCH(InstanceVersion);

	printf("Vulkan loader supports version %d.%d.%d\n", 
		   m_instanceVersion.Major, m_instanceVersion.Minor, m_instanceVersion.Patch);
}




    void VulkanCore::CreateInstance(const char* pAppName)
    {
	std::vector<const char*> Layers = {
	    "VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> Extensions = {
	    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
	    "VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
	    "VK_MVK_macos_surface",
#endif
#if defined (__linux__)
	    "VK_KHR_xcb_surface",
#endif
	    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

	const VkApplicationInfo AppInfo = {
	    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    .pNext = nullptr,
	    .pApplicationName = pAppName,
	    .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0 , 0),
	    .pEngineName = "North Sea Engine",
	    .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
	    .apiVersion = VK_MAKE_API_VERSION(0, m_instanceVersion.Major, m_instanceVersion.Minor, 0)
	};

	VkInstanceCreateInfo CreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    .pNext = nullptr,
	    .flags = 0,
	    .pApplicationInfo = &AppInfo,
	    .enabledLayerCount = (u32)(Layers.size()),
	    .ppEnabledLayerNames = Layers.data(),
	    .enabledExtensionCount = (u32)(Extensions.size()),
	    .ppEnabledExtensionNames = Extensions.data()
	};

	VkResult res = vkCreateInstance(&CreateInfo, nullptr, &m_instance);
	CHECK_VK_RESULT(res, "Create instance");
	printf("Vulkan instance created\n");
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
	printf("Debug callback: %s\n", pCallbackData->pMessage);
	printf("	Severity %s\n", GetDebugSeverityStr(Severity));
	printf("	Type %s\n", GetDebugType(Type));
	printf("	Objects ");
	
	for (u32 i = 0; i < pCallbackData->objectCount; i++) {
	    printf("%llx ", pCallbackData->pObjects[i].objectHandle);
	}

	return VK_FALSE;
    } // calling function should not be aborted
      //

    void VulkanCore::CreateDebugCallback()
    {
	VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	    .pNext = nullptr,
	    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	    .pfnUserCallback = &DebugCallback,
	    .pUserData = nullptr
	};

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
	vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
	if (!vkCreateDebugUtilsMessenger) {
	    OGLDEV_ERROR("Cannot find address of vkCreateDebugUtilsMessenger\n");
	    exit(1);
	}

	VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, nullptr, &m_debugMessenger);
	CHECK_VK_RESULT(res, "debug utils messenger");

	printf("Debug utils messenger created\n");
    }

    void VulkanCore::CreateSurface()
    {
	if (glfwCreateWindowSurface(m_instance, m_pWindow, nullptr, &m_surface)) {
	    fprintf(stderr, "Error creating GLFW window surface\n");
	    exit(1);
	}

	printf("GLFW window surface created\n");
    }

void VulkanCore::CreateDevice()
{
    float qPriorities[] = { 1.0f };

    VkDeviceQueueCreateInfo qInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_queueFamily,
        .queueCount = 1,
        .pQueuePriorities = qPriorities
    };

    std::vector<const char*> DevExts = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
    };

    // 1. Properly check for support
    bool DeviceSupportsDynamicRendering = m_physDevices.Selected().IsExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    
    // If we are on 1.3+, we don't need to add the extension string, 
    // but we still need to enable the FEATURE.
    if (!DeviceSupportsDynamicRendering) {
        printf("The system doesn't support dynamic rendering\n");
        exit(1);
    }

    // 2. Setup the Feature Chain
    // Use the 1.3 struct for dynamic rendering if on a 1.3+ driver
    VkPhysicalDeviceVulkan13Features Features13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = nullptr,
        .dynamicRendering = VK_TRUE // This is the magic toggle
    };

    VkPhysicalDeviceVulkan12Features Features12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &Features13, // Chain 1.3 features here
        .descriptorIndexing = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE
    };

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.geometryShader = VK_TRUE;
    DeviceFeatures.tessellationShader = VK_TRUE;
    DeviceFeatures.multiDrawIndirect = VK_TRUE;

    VkDeviceCreateInfo DeviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &Features12, // Start of the chain
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &qInfo,
        .enabledExtensionCount = (u32)DevExts.size(),
        .ppEnabledExtensionNames = DevExts.data(),
        .pEnabledFeatures = &DeviceFeatures
    };

    VkResult res = vkCreateDevice(m_physDevices.Selected().m_physDevice, &DeviceCreateInfo, nullptr, &m_device);
    CHECK_VK_RESULT(res, "Create device\n");

    printf("\nDevice created with Dynamic Rendering enabled!\n");
}



static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& PresentModes)
{
	for (int i = 0; i < PresentModes.size(); i++) {
		if (PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return PresentModes[i];
		}
	}

	// Fallback to FIFO which is always supported
	return VK_PRESENT_MODE_FIFO_KHR;
}


static u32 ChooseNumImages(const VkSurfaceCapabilitiesKHR& Capabilities)
{
	u32 RequestedNumImages = Capabilities.minImageCount + 1;

	int FinalNumImages = 0;

	if ((Capabilities.maxImageCount > 0) && (RequestedNumImages > Capabilities.maxImageCount)) {
		FinalNumImages = Capabilities.maxImageCount;
	}
	else {
		FinalNumImages = RequestedNumImages;
	}

	return FinalNumImages;
}


static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& SurfaceFormats)
{
	for (int i = 0; i < SurfaceFormats.size(); i++) {
		if ((SurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB) &&
			(SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
			return SurfaceFormats[i];
		}
	}

	return SurfaceFormats[0];
}


VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags,
	VkImageViewType ViewType, u32 LayerCount, u32 mipLevels)
{
    VkImageViewCreateInfo ViewInfo =
    {
	.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.image = Image,
	.viewType = ViewType,
	.format = Format,
	.components = {
	    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
	    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
	    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
	    .a = VK_COMPONENT_SWIZZLE_IDENTITY
	},
	.subresourceRange = {
	    .aspectMask = AspectFlags,
	    .baseMipLevel = 0,
	    .levelCount = mipLevels,
	    .baseArrayLayer = 0,
	    .layerCount = LayerCount
	}
    };

    VkImageView ImageView;
    VkResult res = vkCreateImageView(Device, &ViewInfo, nullptr, &ImageView);
    CHECK_VK_RESULT(res, "vkCreateImageView");
    return ImageView;
}



void VulkanCore::CreateSwapChain()
{
	const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_physDevices.Selected().m_surfaceCaps;

	u32 NumImages = ChooseNumImages(SurfaceCaps);

	const std::vector<VkPresentModeKHR>& PresentModes = m_physDevices.Selected().m_presentModes;
	VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);

	m_swapChainSurfaceFormat = ChooseSurfaceFormatAndColorSpace(m_physDevices.Selected().m_surfaceFormats);

	VkSwapchainCreateInfoKHR SwapChainCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = m_surface,
		.minImageCount = NumImages,
		.imageFormat = m_swapChainSurfaceFormat.format,
		.imageColorSpace = m_swapChainSurfaceFormat.colorSpace,
		.imageExtent = SurfaceCaps.currentExtent,
		.imageArrayLayers = 1,
		.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &m_queueFamily,
		.preTransform = SurfaceCaps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = PresentMode,
		.clipped = VK_TRUE
	};

	VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChain);
	CHECK_VK_RESULT(res, "vkCreateSwapchainKHR\n");

	printf("Swap chain created\n");

	uint NumSwapChainImages = 0;
	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, NULL);
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");
	assert(NumImages <= NumSwapChainImages);
	
	printf("Requested %d images, created %d images\n", NumImages, NumSwapChainImages);

	m_images.resize(NumSwapChainImages);

	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, m_images.data());
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");

	int LayerCount = 1;
	int MipLevels = 1;
	m_imageViews.resize(NumSwapChainImages);
	for (u32 i = 0; i < NumSwapChainImages; i++) {
		m_imageViews[i] = CreateImageView(m_device, m_images[i], m_swapChainSurfaceFormat.format, 
		VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, LayerCount, MipLevels);
	}
}




void VulkanCore::CreateCommandBufferPool()
{
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {
	.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.queueFamilyIndex = m_queueFamily
    };

    VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, nullptr, &m_cmdBufPool);
    CHECK_VK_RESULT(res, "vkCreateCommandPool\n");

    printf("Command buffer pool created\n");
}



void VulkanCore::CreateCommandBuffers(u32 count, VkCommandBuffer* cmdBufs)
{
    VkCommandBufferAllocateInfo cmdBufAllocInfo = {
	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	.pNext = nullptr,
	.commandPool = m_cmdBufPool,
	.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	.commandBufferCount = count
    };

    VkResult res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, cmdBufs);
    CHECK_VK_RESULT(res, "vkAllocateCommandBuffers\n");

    printf("%d command buffers created\n", count);
}


void VulkanCore::FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs)
{
	m_queue.WaitIdle();
	vkFreeCommandBuffers(m_device, m_cmdBufPool, Count, pCmdBufs);
}


VkRenderPass VulkanCore::CreateSimpleRenderPass()
{
    VkAttachmentDescription AttachDesc = {
	.flags = 0,
	.format = m_swapChainSurfaceFormat.format,
	.samples = VK_SAMPLE_COUNT_1_BIT,
	.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference AttachRef = {
	.attachment = 0,
	.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription SubpassDesc = {
	.flags = 0,
	.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
	.inputAttachmentCount = 0,
	.pInputAttachments = nullptr,
	.colorAttachmentCount = 1,
	.pColorAttachments = &AttachRef,
	.pResolveAttachments = nullptr,
	.pDepthStencilAttachment = nullptr,
	.preserveAttachmentCount = 0,
	.pPreserveAttachments = nullptr
    };

    VkRenderPassCreateInfo RenderPassCreateInfo = {
	.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.attachmentCount = 1,
	.pAttachments = &AttachDesc,
	.subpassCount = 1,
	.pSubpasses = &SubpassDesc,
	.dependencyCount = 0,
	.pDependencies = nullptr
    };

    VkRenderPass RenderPass;
    
    VkResult res = vkCreateRenderPass(m_device, &RenderPassCreateInfo, nullptr, &RenderPass);
    CHECK_VK_RESULT(res, "vkCreateRenderPass\n");

    printf("Created a simple render pass\n");

    return RenderPass;
}


std::vector<VkFramebuffer> VulkanCore::CreateFramebuffers(VkRenderPass RenderPass)
{
    m_frameBuffers.resize(m_images.size());

    int WindowWidth, WindowHeight;
    glfwGetWindowSize(m_pWindow, &WindowWidth, &WindowHeight);

    VkResult res;

    for (uint i = 0; i < m_images.size(); i++) {
	VkFramebufferCreateInfo fbCreateInfo = {};
	fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCreateInfo.renderPass = RenderPass;
	fbCreateInfo.attachmentCount = 1;
	fbCreateInfo.pAttachments = &m_imageViews[i];
	fbCreateInfo.width = WindowWidth;
	fbCreateInfo.height = WindowHeight;
	fbCreateInfo.layers = 1;
res = vkCreateFramebuffer(m_device, &fbCreateInfo, nullptr, &m_frameBuffers[i]);
	CHECK_VK_RESULT(res, "vkCreateFramebuffer\n");
    }

    printf("Framebuffers created\n");

    return m_frameBuffers;
}



void VulkanCore::DestroyFramebuffers(std::vector<VkFramebuffer>& Framebuffers)
{
	for (int i = 0; i < Framebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device, Framebuffers[i], NULL);
	}
}


BufferAndMemory VulkanCore::CreateVertexBuffer(const void* pVertices, size_t Size)
{
    // step 1: create the staging buffer
    VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    BufferAndMemory StagingVB = CreateBuffer(Size, Usage, MemProps);

    //step 2: Map the memory of the stage buffer
    void* pMem = nullptr;
    VkDeviceSize Offset = 0;
    VkMemoryMapFlags Flags = 0;
    VkResult res = vkMapMemory(m_device, StagingVB.m_mem, Offset,
			    StagingVB.m_allocationSize, Flags, &pMem);
    CHECK_VK_RESULT(res, "vkMapMemory\n");

    //step 3: copy vertices to the starting buffer
    memcpy(pMem, pVertices, Size);

    //step 4: unmap/release the mapped moery
    vkUnmapMemory(m_device, StagingVB.m_mem);

    //step 5: create final buffer
    Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    BufferAndMemory VB = CreateBuffer(Size, Usage, MemProps);

    //step 6: copy staging buffer to the final buffer
    CopyBuffer(VB.m_buffer, StagingVB.m_buffer, Size);

    //step 7: release resources of the staging bufer
    StagingVB.Destroy(m_device);

    return VB;
};


BufferAndMemory VulkanCore::CreateUniformBuffer(size_t Size)
{
    BufferAndMemory Buffer;

    VkBufferUsageFlags Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
				    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer = CreateBuffer(Size, Usage, MemProps);

    return Buffer;
}


BufferAndMemory VulkanCore::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage,
					VkMemoryPropertyFlags Properties)
{
    VkBufferCreateInfo vbCreateInfo = {
	.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	.size = Size,
	.usage = Usage,
	.sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    BufferAndMemory Buf;

    //step 1: create a buffer
    VkResult res = vkCreateBuffer(m_device, &vbCreateInfo, nullptr, &Buf.m_buffer);
    CHECK_VK_RESULT(res, "vkCreateBuffer\n");
    printf("Buffer created\n");

    //step 2: get buffer mem reqs
    VkMemoryRequirements MemReqs = {};
    vkGetBufferMemoryRequirements(m_device, Buf.m_buffer, &MemReqs);
    printf("Buffer requires %d bytes\n", (int)MemReqs.size);

    Buf.m_allocationSize = MemReqs.size;

    //step 3: get mem type index
    u32 MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, Properties);
    printf("Memory type index %d\n", MemoryTypeIndex);

    //step 4: allocate mem
    VkMemoryAllocateInfo MemAllocInfo = {
	.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
	.allocationSize = MemReqs.size,
	.memoryTypeIndex = MemoryTypeIndex
    };

    res = vkAllocateMemory(m_device, &MemAllocInfo, nullptr, &Buf.m_mem);
    CHECK_VK_RESULT(res, "vkAllocateMemory error &d\n");
    

    //step 5: bind memory
    res = vkBindBufferMemory(m_device, Buf.m_buffer, Buf.m_mem, 0);
    CHECK_VK_RESULT(res, "vkBindBufferMemory error %d\n");

    return Buf;
}


u32 VulkanCore::GetMemoryTypeIndex(u32 MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags)
{
	const VkPhysicalDeviceMemoryProperties& MemProps = m_physDevices.Selected().m_memProps;

	for (uint i = 0; i < MemProps.memoryTypeCount; i++) {
		const VkMemoryType& MemType = MemProps.memoryTypes[i];
		uint CurBitmask = (1 << i);
		bool IsCurMemTypeSupported = (MemTypeBitsMask & CurBitmask);
		bool HasRequiredMemProps = ((MemType.propertyFlags & ReqMemPropFlags) == ReqMemPropFlags);

		if (IsCurMemTypeSupported && HasRequiredMemProps) {
			return i;
		}
	}

	printf("Cannot find memory type for type %x requested mem props %x\n", MemTypeBitsMask, ReqMemPropFlags);
	exit(1);
	return -1;
}


void VulkanCore::CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size)
{
    BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferCopy BufferCopy = {
	.srcOffset = 0,
	.dstOffset = 0,
	.size = Size
    };

    vkCmdCopyBuffer(m_copyCmdBuf, Src, Dst, 1, &BufferCopy);

    vkEndCommandBuffer(m_copyCmdBuf);

    m_queue.SubmitSync(m_copyCmdBuf);

    m_queue.WaitIdle();
}    


void VulkanCore::GetFramebufferSize(int& Width, int& Height) const
{
    glfwGetWindowSize(m_pWindow, &Width, &Height);
}

std::vector<BufferAndMemory> VulkanCore::CreateUniformBuffers(size_t Size)
{
    std::vector<BufferAndMemory> UniformBuffers;

    UniformBuffers.resize(m_images.size());

    for (int i = 0; i < UniformBuffers.size(); i++) {
	UniformBuffers[i] = CreateUniformBuffer(Size);
    }

    return UniformBuffers;
}

void BufferAndMemory::Update(VkDevice Device, const void* pData, size_t Size)
{
    void* pMem = nullptr;
    VkResult res = vkMapMemory(Device, m_mem, 0, Size, 0 , &pMem);
    CHECK_VK_RESULT(res, "vkMapMemory");
    memcpy(pMem, pData, Size);
    vkUnmapMemory(Device, m_mem);
}

}
