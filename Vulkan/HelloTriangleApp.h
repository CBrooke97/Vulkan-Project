#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vector>
#include <optional>
#include <vulkan/vulkan.h>
#include <vulkan/vk_platform.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static std::vector<char> readFile(const std::string& filename);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messagSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

struct QueueFamilyIndices {
	std::optional<u32> graphicsFamily;
	std::optional<u32> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	};
};

const std::vector<const char*> g_deviceExtensions
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApp;

void setupDebugMessenger(HelloTriangleApp& app);

class HelloTriangleApp
{
public:
	void Run() {
		InitWindow();
		InitVulkan();
		MainLoop();
		Cleanup();
	}

	VkInstance& GetInstance() { return m_vkInstance; }
	VkDebugUtilsMessengerEXT& GetDebugMessenger() { return m_debugMessenger; }

private:
	void InitWindow();

	void InitVulkan();

	void MainLoop();

	void Cleanup();

	void createSurface();

	void createInstance();

	bool checkValidationLayerSupport();

	void pickPhysicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	void createLogicalDevice();

	void createSwapchain();

	void createImageViews();

	void createRenderPass();

	void createGraphicsPipeline();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkShaderModule createShaderModule(const std::vector<char>& bytecode);

	// Queue families are essentially the render command queues.
	// These are split into families to handle different kinds of operations.
	// For example, a memory upload family, a compute command family etc.
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	std::vector<const char*> getRequiredExtensions();

	const std::vector<const char*> validationLayers
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const u32 m_windWidth { 1280 };
	const u32 m_windHeight { 720 };
	SDL_Window* m_pWindow { nullptr };
	VkInstance m_vkInstance{ };
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_vkSurfaceKHR;
	VkSwapchainKHR m_vkSwapchainKHR;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;

	VkPipelineLayout m_pipelineLayout;

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_logicalDevice;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
};
