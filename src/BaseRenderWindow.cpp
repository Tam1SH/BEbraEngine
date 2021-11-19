#include "stdafx.h"
#include "BaseRenderWindow.hpp"
#include "Input.hpp"
#include "Vector2.hpp"
#include <SDL_vulkan.h>
#ifdef BEBRA_USE_SDL
namespace BEbraEngine {
	void BaseWindow::Vulkan_CreateSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		SDL_Vulkan_CreateSurface(handle, instance, surface);
	}
	std::vector<const char*> BaseWindow::Vulkan_GetInstanceExtensions()
	{
		unsigned int count;
		SDL_Vulkan_GetInstanceExtensions(handle, &count, nullptr);

		std::vector<const char*> extensions = { };
		size_t additional_extension_count = extensions.size();
		extensions.resize(additional_extension_count + count);

		SDL_Vulkan_GetInstanceExtensions(handle, &count, extensions.data() + additional_extension_count);
		return extensions;
	}
	void BaseWindow::SetWindowSize(const Vector2& newSize) const noexcept
	{
		SDL_SetWindowSize(handle, newSize.x, newSize.y);
	}
	void BaseWindow::update() {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			SDL_GetMouseState(Input::mouse_x, Input::mouse_y);

			//    ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
				auto size = GetWindowSize();
				this->onResizeCallback(size.x,size.y);
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
				_isClose = true;
			}
		}

		Input::state = SDL_GetKeyboardState(NULL);

	}
	void BaseWindow::_onCreateWindow(int w, int h, const SurfaceType& type, const char* title)
	{
		SDL_WindowFlags flag{};
		if (type == Vulkan) {
			flag = SDL_WINDOW_VULKAN;
		}
		
		handle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flag | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
		Input::SetWindow(handle);
	}
	void BaseWindow::onCreateWindow(const Vector2& size, const SurfaceType& type, const std::string& title = "BEbraEngine")
	{
		_isClose = false;
		_onCreateWindow(size.x, size.y, type, title.c_str());
	}



	Vector2 BaseWindow::GetWindowSize() const noexcept
	{
		int w = 0, h = 0;
		SDL_GetWindowSize(handle, &w, &h);
		return Vector2(w, h);
	}

	int BaseWindow::Width() const noexcept
	{
		return GetWindowSize().x;
	}

	int BaseWindow::Height() const noexcept
	{
		return GetWindowSize().y;
	}

	void BaseWindow::SetPosition(const Vector2& position) const noexcept
	{
		SDL_SetWindowPosition(handle, position.x, position.y);
	}

	Vector2 BaseWindow::GetPosition() const noexcept
	{
		int x = 0, y = 0;
		SDL_GetWindowPosition(handle, &x, &y);
		return Vector2(x, y);
	}

	BaseWindow::BaseWindow()
	{
		SDL_Init(SDL_INIT_VIDEO);
	}

	BaseWindow::~BaseWindow()
	{
		SDL_DestroyWindow(handle);
		SDL_Quit();
	}

	bool BaseWindow::isClose() {
		return _isClose;
	}



}
#endif