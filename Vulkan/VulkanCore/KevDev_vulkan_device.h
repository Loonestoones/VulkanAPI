#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "KevDev_types.h"

namespace KevDevVK {

struct PhysicalDevice {
	VkPhysicalDevice m_physDevice;
	VkPhysicalDeviceProperties m_devProps;
	std::vector<VkQueueFamilyProperties> m_qFamilyProps;
	std::vector<VkBool32> m_qSupportsPresent;
	std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
	VkSurfaceCapabilitiesKHR m_surfaceCaps;
	VkPhysicalDeviceMemoryProperties m_memProps;
	std::vector<VkPresentModeKHR> m_presentModes;
	VkPhysicalDeviceFeatures m_features;
	VkFormat m_depthFormat;
	struct {
		int Variant = 0;
		int Major = 0;
		int Minor = 0;
		int Patch = 0;
	} m_apiVersion;
	std::vector<VkExtensionProperties> m_extensions;

	bool IsExtensionSupported(const char* pExt) const;
};


class VulkanPhysicalDevices {
public:
	VulkanPhysicalDevices() {}
	~VulkanPhysicalDevices() {}

	void Init(const VkInstance& Instance, const VkSurfaceKHR& Surface);

	u32 SelectDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent);

	const PhysicalDevice& Selected() const;

private:

	void GetDeviceAPIVersion(int DeviceIndex);

	void GetExtensions(int DeviceIndex);

	std::vector<PhysicalDevice> m_devices;

	int m_devIndex = -1;
};

}
