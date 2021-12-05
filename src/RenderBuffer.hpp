#pragma once
#include "Debug.hpp"
#include "Vector3.hpp"
namespace BEbraEngine {

	struct RenderBuffer
	{
		virtual void setData(void* data, size_t range, size_t offset) = 0;
		virtual void destroy() = 0;
	};

	struct RenderBufferView {
		std::shared_ptr<RenderBuffer> buffer;
		size_t availableRange{};
		size_t offset{};
		void setData(void* data, size_t range, size_t offsetofData = 0) {
			if (range > availableRange) {
				Debug::log("the range is higher than acceptable: " + std::to_string(range) + " > " +  std::to_string(availableRange));
			}
			buffer->setData(data, range, offset + offsetofData);
		}
	};
}