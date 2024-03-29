﻿module;
#include <SDL_vulkan.h>
#include <SDL.h>

export module Window_impl;
import Window;
import Input;
import <vector>;
import <string>;
using std::vector;
using std::string;

namespace BEbraEngine {
	void Window::vulkan_CreateSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		SDL_Vulkan_CreateSurface(handle, instance, surface);

	}
	vector<const char*> Window::vulkan_GetInstanceExtensions()
	{
		unsigned int count;
		SDL_Vulkan_GetInstanceExtensions(handle, &count, nullptr);

		vector<const char*> extensions = { };
		size_t additional_extension_count = extensions.size();
		extensions.resize(additional_extension_count + count);

		SDL_Vulkan_GetInstanceExtensions(handle, &count, extensions.data() + additional_extension_count);
		return extensions;
	}
	void Window::setWindowSize(const Vector2& newSize) const noexcept
	{
		SDL_SetWindowSize(handle, newSize.x, newSize.y);
	}
	string Window::getName() const noexcept
	{
		return SDL_GetWindowTitle(handle);
	}
	void Window::update() {

		SDL_Event event;


		Input::InputState state{};
		int x{}, y{};




		//SDL_WarpMouseInWindow(handle, getSize().x / 2, getSize().y / 2);
		while (SDL_PollEvent(&event)) {

			//    ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
				auto size = getSize();

				this->onResizeCallback(size.x, size.y);
				//break;
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
				_isClose = true;
			}

		}

		SDL_GetMouseState(&x, &y);

		//float deltaX = (float)x - getSize().x / 2;
		//float deltaY = getSize().y / 2 - (float)y;
		//*Input::lastX = deltaX;
		//*Input::lastY = deltaY;
		state.x = x;
		state.y = y;

		Input::update(state);
		Input::state = SDL_GetKeyboardState(NULL);


	}
	void Window::_onCreateWindow(int w, int h, const SurfaceType& type, const char* title)
	{
		SDL_Init(SDL_INIT_VIDEO);
		SDL_WindowFlags flag{};
		if (type == SurfaceType::Vulkan) {
			flag = SDL_WINDOW_VULKAN;
		}
		handle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flag | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

		//SDL_SetRelativeMouseMode(SDL_TRUE);
		//SDL_ShowCursor(0);
		//SDL_SetWindowGrab(handle, SDL_TRUE);

		Input::setWindow(handle);
	}
	void Window::onCreateWindow(const Vector2& size, const SurfaceType& type, const std::string& title = "BEbraEngine")
	{
		_isClose = false;
		this->type = type;
		_onCreateWindow(size.x, size.y, type, title.c_str());
	}



	Vector2 Window::getSize() const noexcept
	{
		int w = 0, h = 0;
		SDL_GetWindowSize(handle, &w, &h);
		return Vector2(w, h);
	}

	int Window::width() const noexcept
	{
		return getSize().x;
	}

	int Window::height() const noexcept
	{
		return getSize().y;
	}

	void Window::setPosition(const Vector2& position) const noexcept
	{
		SDL_SetWindowPosition(handle, position.x, position.y);
	}

	Vector2 Window::getPosition() const noexcept
	{
		int x = 0, y = 0;
		SDL_GetWindowPosition(handle, &x, &y);
		return Vector2(x, y);
	}

	Window::Window()
	{
		
	}

	Window::~Window()
	{
		if (handle) {
			SDL_DestroyWindow(handle);
			SDL_Quit();
		}

	}

	bool Window::isClose() {
		return _isClose;
	}

	bool Window::isCollapsed()
	{
		return SDL_GetWindowFlags(handle) & SDL_WINDOW_MINIMIZED;
	}
	void Window::setFullScreen(FullScreenType type)
	{
		int flags{};
		if (type == FullScreenType::FullScreenOnWindow)
			flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
		if (type == FullScreenType::FullScreen)
			flags = SDL_WINDOW_FULLSCREEN;
		if (!SDL_SetWindowFullscreen(handle, flags)) throw std::exception();

	}




}