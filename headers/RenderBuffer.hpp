#pragma once
#include "Debug.hpp"
#include "Vector3.hpp"
namespace BEbraEngine {

	class RenderBuffer
	{
	public:
		virtual void setData(const void* data, uint32_t range, uint32_t offset = 0) = 0;
		virtual void destroy() = 0;
	};

	class RenderBufferView {
	public:
		std::shared_ptr<RenderBuffer> buffer;
		uint32_t availableRange{};
		uint32_t offset{};
		void setData(const void* data, uint32_t range, uint32_t offsetofData = 0) {
			if (range > availableRange) {
				DEBUG_LOG1("the range is higher than acceptable: " + std::to_string(range) + " > " + std::to_string(availableRange));
			}
			else
				buffer->setData(data, range, offset + offsetofData);
		}
	};
}