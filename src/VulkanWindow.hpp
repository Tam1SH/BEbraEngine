#pragma once
#include "stdafx.h"
#include "Vector2.hpp"
#include "BaseRenderWindow.hpp"
#include "AbstractRender.hpp"

#undef CreateWindow
namespace BEbraEngine {
	class VulkanRender;
}
namespace BEbraEngine {

	class VulkanWindow : public BaseWindow {
	public:

		void CreateWindow(const Vector2& size, const std::string& title = "BEbraEngine") override;

		VulkanWindow(AbstractRender* render);

		void onResizeCallback(int width, int height) override;

		Vector2 GetDrawableSize();

		void SetVulkanRender(VulkanRender* render);

		void onUpdate() override;

		~VulkanWindow();

	private:
		VulkanRender* render;

	};
}

