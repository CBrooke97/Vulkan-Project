#define NOMINMAX

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "HelloTriangleApp.h"
#include <assert.h>
#include <vector>
#include <set>
#include <algorithm>
#include <limits>
#include <fstream>

void HelloTriangleApp::InitWindow()
{
	SDL_Init(SDL_INIT_EVENTS);

	m_pWindow = SDL_CreateWindow("Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_windWidth, m_windHeight, SDL_WINDOW_VULKAN);

	//u32 extensionCount = 0;
	//vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	//std::cout << extensionCount << " extensions supported.\n";
}

void HelloTriangleApp::InitVulkan()
{
	createInstance();
	setupDebugMessenger(*this);
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createImageViews();
	createGraphicsPipeline();
}

void HelloTriangleApp::MainLoop()
{
	bool HasQuit = false;

	while (!HasQuit)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				HasQuit = true;
			}
		}
	}
}

void HelloTriangleApp::Cleanup()
{
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
	}

	for (auto imageView : m_swapchainImageViews) 
	{
		vkDestroyImageView(m_logicalDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_logicalDevice, m_vkSwapchainKHR, nullptr);

	vkDestroyDevice(m_logicalDevice, nullptr);
	
	vkDestroySurfaceKHR(m_vkInstance, m_vkSurfaceKHR, nullptr);

	vkDestroyInstance(m_vkInstance, nullptr);

	SDL_DestroyWindow(m_pWindow);

	SDL_Quit();
}

void HelloTriangleApp::createSurface()
{
	if (SDL_Vulkan_CreateSurface(m_pWindow, m_vkInstance, &m_vkSurfaceKHR) == SDL_FALSE)
	{
		throw std::runtime_error("Failed to create valid surface!");
	}
}

void HelloTriangleApp::createInstance()
{
	//Check if we want validation layers + if they are supported.
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested, but not available.");
	}

	VkApplicationInfo appInfo{ };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{ };

	// If validation layers are enabled, add them to the create info.
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	memset(&createInfo, 0, sizeof(VkInstanceCreateInfo));

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> sdlExtensionNames = getRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<u32>(sdlExtensionNames.size());
	createInfo.ppEnabledExtensionNames = sdlExtensionNames.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else 
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);

	assert(result == VK_SUCCESS);
}

bool HelloTriangleApp::checkValidationLayerSupport()
{
	u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) 
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void HelloTriangleApp::pickPhysicalDevice()
{
	u32 deviceCount{ 0 };
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan Support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);

	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

	for (const VkPhysicalDevice& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}


// Example function for how to check if device supports features we require.
// 
//bool HelloTriangleApp::isDeviceSuitable(VkPhysicalDevice device)
//{
//	// Queries basice properties
//	// Name, type, supported VK version etc.
//	VkPhysicalDeviceProperties deviceProperties;
//	vkGetPhysicalDeviceProperties(device, &deviceProperties);
//
//	// Queries supported features
//	// Texture compression, 64bit float, multi-viewport rendering etc.
//	VkPhysicalDeviceFeatures deviceFeatures;
//	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
//
//	// We want out GPU to be dedicated and support geometry shaders.
//	// Return a check for that.
//	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
//		deviceFeatures.geometryShader;
//}

bool HelloTriangleApp::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) 
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool HelloTriangleApp::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(g_deviceExtensions.begin(), g_deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void HelloTriangleApp::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{ };

	VkDeviceCreateInfo createInfo{ };
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(g_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = g_deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
}

void HelloTriangleApp::createSwapchain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// Recommended to request 1 more image than the minimum for swap chain.
	u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// Make sure we do not exceed the maximum support number of images for the swap chain.
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_vkSurfaceKHR;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	// If the graphics queue family and present queue family are not the same queue family,
	// then we can allow concurrent use of images between these queues.
	// This requires two distinct family queues.
	if (indices.graphicsFamily != indices.presentFamily) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	// If the queues are the same, which is usually the case on most hardware,
	// we need to use exclsive mode due to the minimum distinct queue requirement.
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// Prevents alpha channel from blending this window with other windows.
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;

	// Enabling clipped means pixels obscured by other windows will be clipped.
	createInfo.clipped = VK_TRUE;

	// This parameter is only used when the window is resized.
	// During resizing, a new window is created and a reference to the old one is required for initialization.
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_vkSwapchainKHR) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain!");
	}


	// Retrieve the swapchain images from the newly created swapchain
	// and store them for later use.
	vkGetSwapchainImagesKHR(m_logicalDevice, m_vkSwapchainKHR, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_logicalDevice, m_vkSwapchainKHR, &imageCount, m_swapchainImages.data());

	// Store format and extent for later use too.
	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
}

void HelloTriangleApp::createImageViews()
{
	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];

		// Defines the image as 1D, 2D, 3D or a cubemap
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		// Inits the image format to the same as the swapchain
		createInfo.format = m_swapchainImageFormat;

		// This allows mapping of each channel, as you may want to map them all to the same channel
		// for monochrome images. This initialization just leaves them as default mapping.
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// The subresource range describes the images purpose and which parts can be accessed.
		// In our case, we access them as colour targets without mipmaps or layers.
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void HelloTriangleApp::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("Shaders/CompiledShaders/vert.spv");
	auto fragShaderCode = readFile("Shaders/CompiledShaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	// Create the Vertex Shader portion of the pipeline
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{ };
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

	// Specify the shader module to attach to the pipeline stage
	vertShaderStageInfo.module = vertShaderModule;

	// Specify the entry point for that module.
	vertShaderStageInfo.pName = "main";

	// Create the Fragment Shader portion of the pipeline
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Specify the shader module to attach to the pipeline stage
	fragShaderStageInfo.module = fragShaderModule;

	// Specify the entry point for that module.
	fragShaderStageInfo.pName = "main";

	// Store these Pipeline Stage create infos in an array for use in pipeline creation.
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Pipelines in Vulkan are generally immutable, but some parts can be dynamic.
	// For that to be the case, we have to specify which we want to be dynamic.
	// Having these states be dynamic *can* reduce complexity later down the lane,
	// as opposed to creating a new pipeline representing each state.
	std::vector<VkDynamicState> dynamicStates
	{
		// Viewport and scissor rect can both be dynamic.
		// It is often recommended to do so. 
		// These are setup later.
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{ };
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Structure used to describe vertex data format.
	// Specifies bindings, that being spacing between data and whether it is instanced.
	// Specifices attributes, the extra data like Vertex Colours.
	// Leaving this empty for now, as we are hardcoding the vertex data in the shader.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	// Structure used to describe the geometry (topology) and primitive restart.
	// Primitve restart allows STRIP modes to break up geometry when enabled.
	// We are using LIST mode so this is disabled.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport is the area being drawn to.
	// They describe the transformation of the image to the framebuffer.
	VkViewport viewport{ };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchainExtent.width;
	viewport.height = (float)m_swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor rect is used by the rasterizer to clip pixels outside of it.
	// Essentially defines the area of the viewport that can be drawn to.
	// If the viewport is bigger than the scissor rect, pixels outside the bounds
	// will be clipped by the rasterizer.
	VkRect2D scissor{ };
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;

	// Here we describe the Viewport with the above information.
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// If we were creating non-dynamic viewport and scissor,
	// they would have to be assigned as below.
	// This would make them immutable along with the pipeline.
	// viewportState.pViewports = &viewport;
	// viewportState.pScissors = &scissor;

	// Setup the structure which describes rasterizer state.
	// Depth clamp will clamp fragments beyond the near and far planes
	// rather than discarding them. Can be useful for shadow maps.
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;

	// This will cull all geometry passed to the rasterizer. We do not want this.
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	// Determines how geometry from the vertex shader is converted to fragments.
	// Modes other than fill require enabling specific GPU features.
	// VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
	// VK_POLYGON_MODE_LINE : polygon edges are drawn as lines
	// VK_POLYGON_MODE_POINT : polygon vertices are drawn as point
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	// Determines the thickness of lines in fragments. 
	// Max value is hardware dependent, we generally want value of 1.
	// Value greater then 1 also requires enabling GPU features.
	rasterizer.lineWidth = 1.0f;

	// Cull mode determines face culling.
	// frontFace specifies which vertices to consider forward facing,
	// and their winding order.
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	// These values allow the rasterizer to bias depth values.
	// Can be useful for shadow maps.
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// The below structure describes the multisampling state of the pipeline.
	// Multisampling is a means of anti-aliasing (MSAA).
	// Having the hardware do it is more efficient.
	// Requires enabling of GPU features.
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// We can destroy the shader modules after we are done linking them to the piepline.
	vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);
}

VkSurfaceFormatKHR HelloTriangleApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) 
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) 
	{
		return capabilities.currentExtent;
	}
	else 
	{
		int width, height;
		SDL_Vulkan_GetDrawableSize(m_pWindow, &width, &height);

		VkExtent2D actualExtent
		{
			static_cast<u32>(width),
			static_cast<u32>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkShaderModule HelloTriangleApp::createShaderModule(const std::vector<char>& bytecode)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytecode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

QueueFamilyIndices HelloTriangleApp::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices familyIndices;

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// Use a bitwise & to check if queue family supports Graphics_Bit.
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			familyIndices.graphicsFamily = i;
		}

		if (familyIndices.isComplete())
		{
			break;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_vkSurfaceKHR, &presentSupport);

		if (presentSupport)
		{
			familyIndices.presentFamily = i;
		}

		i++;
	}

	return familyIndices;
}

SwapChainSupportDetails HelloTriangleApp::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_vkSurfaceKHR, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vkSurfaceKHR, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vkSurfaceKHR, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vkSurfaceKHR, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vkSurfaceKHR, &presentModeCount, details.presentModes.data());
	}

	return details;
}

std::vector<const char*> HelloTriangleApp::getRequiredExtensions()
{
	u32 sdlExtensionCount;

	SDL_bool sdl_result = SDL_Vulkan_GetInstanceExtensions(m_pWindow, &sdlExtensionCount, nullptr);

	std::vector<const char*> sdlExtensionNames(sdlExtensionCount);

	sdl_result = SDL_Vulkan_GetInstanceExtensions(m_pWindow, &sdlExtensionCount, sdlExtensionNames.data());

	if (enableValidationLayers)
	{
		sdlExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return sdlExtensionNames;
}

std::vector<char> readFile(const std::string& filename)
{
	// 'ate' allows us to start reading from the end of the file. 
	// This can be used to determine the file size at the start,
	// for use in allocating a buffer size.
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	// Create a buffer of the same size as the bytecode.
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	// Return to the start of the file and read the data into the buffer.
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
									  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
									  const VkAllocationCallbacks* pAllocator, 
									  VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
								   VkDebugUtilsMessengerEXT debugMessenger, 
								   const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) 
	{
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messagSeverity, 
											 VkDebugUtilsMessageTypeFlagsEXT messageType, 
											 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
											 void* pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void setupDebugMessenger(HelloTriangleApp& app)
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(app.GetInstance(), &createInfo, nullptr, &app.GetDebugMessenger()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}
