#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <iostream>
#include "HelloTriangleApp.h"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;


int WinMain()
{
	HelloTriangleApp app;

	try
	{
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//int WinMainOld()
//{
//	SDL_Init(SDL_INIT_EVENTS);
//
//	SDL_Window* g_pWindow = SDL_CreateWindow("Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1080, 720, SDL_WINDOW_VULKAN);
//
//	u32 extensionCount = 0;
//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//
//	std::cout << extensionCount << " extensions supported.\n";
//
//	glm::mat4 matrix;
//	glm::vec4 vec;
//
//	auto test = matrix * vec;
//
//	bool HasQuit = false;
//
//	while (!HasQuit)
//	{
//		SDL_Event event;
//
//		while (SDL_PollEvent(&event))
//		{
//			if (event.type == SDL_QUIT)
//			{
//				HasQuit = true;
//			}
//		}
//	}
//
//	SDL_DestroyWindow(g_pWindow);
//
//	SDL_Quit();
//
//	return 0;
//}