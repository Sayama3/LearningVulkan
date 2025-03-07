#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#define TRYC_MSG(test, message)            \
	if constexpr ((test) != true) {        \
		throw std::runtime_error(message); \
	}
#define TRYC(test)                       \
	if constexpr ((test) != true) {      \
		throw std::runtime_error(#test); \
	}

#define TRY_MSG(test, message)             \
	if ((test) != true) {                  \
		throw std::runtime_error(message); \
	}
#define TRY(test)                        \
	if ((test) != true) {                \
		throw std::runtime_error(#test); \
	}

#define TRY_VK_MSG(vk_action, message) TRY_MSG((vk_action) == VK_SUCCESS, message)
#define TRY_VK(vk_action) TRY_MSG((vk_action) == VK_SUCCESS, #vk_action)

static constexpr uint32_t WIDTH = 800;
static constexpr uint32_t HEIGHT = 600;

static const std::vector<const char *> c_ValidationLayers = {"VK_LAYER_KHRONOS_validation",};
static const std::vector<const char*> c_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#ifdef NDEBUG
static constexpr bool c_EnableValidationLayers = false;
#else
static constexpr bool c_EnableValidationLayers = true;
#endif


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	[[nodiscard]] bool IsComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	SwapChainSupportDetails() = default;
	~SwapChainSupportDetails() = default;
	SwapChainSupportDetails(const SwapChainSupportDetails&) = default;
	SwapChainSupportDetails& operator=(const SwapChainSupportDetails&) = default;
	SwapChainSupportDetails(SwapChainSupportDetails&& other) : SwapChainSupportDetails() {
	    swap(other);
	}
	SwapChainSupportDetails& operator=(SwapChainSupportDetails&& other) noexcept {
	    swap(other);
		return *this;
	}
	void swap(SwapChainSupportDetails& other) noexcept {
        std::swap(capabilities, other.capabilities);
        std::swap(formats, other.formats);
        std::swap(presentModes, other.presentModes);
	}

	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats{};
	std::vector<VkPresentModeKHR> presentModes{};
};

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		TRY_MSG(deviceCount > 0, "failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : devices) {
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0) {
			m_PhysicalDevice = candidates.rbegin()->second;
		} else {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

		createInfo.enabledExtensionCount = static_cast<uint32_t>(c_DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = c_DeviceExtensions.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		// Retro-compatibility with pre-1.3 vulkan drivers
		if constexpr (c_EnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(c_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = c_ValidationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		TRY_VK_MSG(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device), "failed to create logical device!");
		vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	}

	void createSurface() {
		TRY_VK_MSG(glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface), "failed to create window surface!");
	}
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_PhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // May need two for VR
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Some of the one I might use later:
		// VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);
		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // should be used for blending with other windows

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // VK_TRUE = Do not draw pixel hidden by another window.

		createInfo.oldSwapchain = VK_NULL_HANDLE; // If we need to recreate a swapchain cause the window was resize.

		TRY_VK_MSG(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain), "failed to create swap chain!");

		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(m_Window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyDevice(m_Device, nullptr);
		if constexpr (c_EnableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_Instance, debugMessenger, nullptr);
		}
		vkDestroyInstance(m_Instance, nullptr);
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

private:

	int rateDeviceSuitability(const VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		// Application can't function without geometry shaders
		if (!deviceFeatures.geometryShader) {
			return 0;
		}

		const bool extensionsSupported = checkDeviceExtensionSupport(device);
		if (!extensionsSupported) {
			return 0;
		}

		QueueFamilyIndices indices = findQueueFamilies(device);
		if (!indices.IsComplete()) {
			return 0;
		}

		// SwapChain Extension must be checked BEFORE the support of the swap chain (if any is needed).
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		const bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		if (!swapChainAdequate) {
			return 0;
		}

		return score;
	}

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT)) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.IsComplete()) {
				break;
			}
			++i;
		}


		return indices;
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char *layerName: c_ValidationLayers) {
			bool layerFound = false;

			for (const auto &layerProperties: availableLayers) {
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

	bool checkExtensionsSupport(const char **const additional_layers = nullptr, const int additional_layers_count = 0) {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		for (int i = 0; i < additional_layers_count; ++i) {
			bool layerFound = false;

			for (const auto &extension: extensions) {
				if (strcmp(additional_layers[i], extension.extensionName) == 0) {
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

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		TRY_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities));

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
				return availablePresentMode;
			}
		}
		// The only guaranteed to be available. As a backup if all else fail.
		return VK_PRESENT_MODE_FIFO_KHR;
	}


	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			int width, height;
			glfwGetFramebufferSize(m_Window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	std::vector<const char *> getRequiredInstanceExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (c_EnableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void createInstance() {
		// ==================== Validating Vulkan Drivers ====================
		if constexpr (c_EnableValidationLayers) {
			TRY_MSG(checkValidationLayerSupport(), "validation layers requested, but not available!");
		}

		{
			uint32_t glfwExtensionCount = 0;
			const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			TRY_MSG(checkExtensionsSupport(glfwExtensions, glfwExtensionCount), "GLFW Extensions are not available.");
		}

		// ==================== VkApplicationInfo ====================
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// ==================== VkInstanceCreateInfo ====================
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		// -------------------- Extensions --------------------
		auto extensionsNames = getRequiredInstanceExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsNames.size());
		createInfo.ppEnabledExtensionNames = extensionsNames.data();
		// -------------------- Layers --------------------
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if constexpr (c_EnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(c_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = c_ValidationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		// ==================== VkInstanceCreation ====================
		TRY_VK_MSG(vkCreateInstance(&createInfo, nullptr, &m_Instance), "failed to create a Vulkan Instance!");
	}

	bool checkDeviceExtensionSupport(const VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(c_DeviceExtensions.begin(), c_DeviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void setupDebugMessenger() {
		if constexpr (!c_EnableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		TRY_VK_MSG(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &debugMessenger), "failed to set up debug messenger!")
	}

	static std::string severityToString(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {return "ERROR"; }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {return "WARNING"; }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {return "INFO"; }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {return "VERBOSE"; }
		return"Unknown";
	}
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *pUserData) {
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			std::cerr << "[" << severityToString(messageSeverity) << "] [VULKAN] " << pCallbackData->pMessage << std::endl;
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::cout << "[" << severityToString(messageSeverity) << "] [VULKAN] " << pCallbackData->pMessage << std::endl;
		}
		return VK_FALSE;
	}

public:
private:
	GLFWwindow* m_Window{nullptr};
	VkInstance m_Instance{VK_NULL_HANDLE};
	VkPhysicalDevice m_PhysicalDevice{VK_NULL_HANDLE};
	VkDevice m_Device{VK_NULL_HANDLE};
	VkQueue m_GraphicsQueue{VK_NULL_HANDLE};
	VkQueue m_PresentQueue{VK_NULL_HANDLE};
	VkSurfaceKHR m_Surface{VK_NULL_HANDLE};
	VkSwapchainKHR m_SwapChain{VK_NULL_HANDLE};
	VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
	VkFormat m_SwapChainImageFormat{};
	VkExtent2D m_SwapChainExtent{};
	std::vector<VkImage> m_SwapChainImages;
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
