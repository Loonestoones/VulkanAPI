#pragma once

#include <vulkan/vulkan.h>

namespace KevDevVK {

VkShaderModule CreateShaderModuleFromBinary(VkDevice device, const char* pFilename);

VkShaderModule CreateShaderModuleFromText(VkDevice device, const char* pFilename);

VkShaderModule CreateShaderModuleFromSPIRV(VkDevice device, const char* pFilename);

}

