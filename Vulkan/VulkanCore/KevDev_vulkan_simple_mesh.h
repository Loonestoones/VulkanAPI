#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "KevDev_types.h"
#include "KevDev_vulkan_core.h"

namespace KevDevVK {

struct SimpleMesh {
	BufferAndMemory m_vb;
	size_t m_vertexBufferSize = 0;
	VulkanTexture* m_pTex = nullptr;

	void Destroy(VkDevice Device)
	{
		m_vb.Destroy(Device);

		if (m_pTex) {
		    m_pTex->Destroy(Device);
		    delete m_pTex;
		}
	}
};

}
