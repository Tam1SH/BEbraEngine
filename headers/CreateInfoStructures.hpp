#pragma once
#include "stdafx.h"
#include "platform.hpp"
#include "RenderBuffer.hpp"
#include "Vector3.hpp"

using BE_STD::vector;

namespace BEbraEngine {


	class LightDescriptorInfo {
	public:
		enum class Type {
			Point,
			Direction
		};
	public:
		RenderBufferView* bufferView;
		Type type;
	};

	class RenderObjectInfo {
	public:
		RenderBufferView* bufferView;
	};

	class VulkanDescriptorSetInfo {
	public:
		RenderBufferView* bufferView;
		VkSampler sampler;
		VkImageView imageView;
	};
	class VulkanDescriptorPoolInfo {
	public:
		vector<VkDescriptorPoolSize> types;
		VkDescriptorSetLayout layout;
	};
}