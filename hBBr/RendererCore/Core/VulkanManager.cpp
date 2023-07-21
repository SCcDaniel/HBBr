﻿#include "VulkanManager.h"
#include <vector>
#include <array>
#include "pugixml.hpp"
#include "HString.h"
#include "ConsoleDebug.h"

//导入Vulkan静态库
#pragma comment(lib ,"vulkan-1.lib")

PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT  fvkDestroyDebugReportCallbackEXT = nullptr;

bool VulkanManager::debugMarkerActive = false;
bool VulkanManager::extensionPresent = false;
PFN_vkDebugMarkerSetObjectTagEXT VulkanManager::vkDebugMarkerSetObjectTag = VK_NULL_HANDLE;
PFN_vkDebugMarkerSetObjectNameEXT VulkanManager::vkDebugMarkerSetObjectName = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerBeginEXT VulkanManager::vkCmdDebugMarkerBegin = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerEndEXT VulkanManager::vkCmdDebugMarkerEnd = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerInsertEXT VulkanManager::vkCmdDebugMarkerInsert = VK_NULL_HANDLE;

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT	flags,
	VkDebugReportObjectTypeEXT	obj_type,
	uint64_t	src_obj,
	size_t	location,
	int32_t	msg_code,
	const char* layer_prefix,
	const char* msg,
	void* user_data
)
{
	HString title;
	HString color;
	bool bError = false;
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		title = "INFO: "; color = "255,255,255";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		title = "WARNING: "; color = "255,255,0";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		title = "PERFORMANCE WARNING: "; color = "255,175,0";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		title = "ERROR: "; color = "255,0,0";
		bError = true;
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		title = "DEBUG: "; color = "255,255,255";
	}
	//if (flags & VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT) {
	//	ConsoleDebug::print_endl(DString(": "));
	//}
	ConsoleDebug::print_endl(title + HString("@[") + layer_prefix + "]", color);
	ConsoleDebug::print_endl(msg, color);
	if(bError)
		MessageOut(HString(title + HString("@[") + layer_prefix + "]\n" + msg).c_str(),false,true);
	return false;
}


VulkanManager::VulkanManager()
{
#if defined(_WIN32)
	_currentPlatform = EPlatform::Windows;
#elif defined(__ANDROID__)
	_currentPlatform = EPlatform::Android;
#elif defined(__linux__)
	_currentPlatform = EPlatform::Linux;
#endif
	_bDebugEnable = false;
	_graphicsQueueFamilyIndex = -1;
	_swapchainBufferCount = 3;
	_enable_VK_KHR_display = false;
}

VulkanManager::~VulkanManager()
{
	if (_gpuDevice)
		vkDestroyDevice(_device,nullptr);
	if (_bDebugEnable)
		fvkDestroyDebugReportCallbackEXT(_instance, _debugReport, nullptr);
	if (_instance != VK_NULL_HANDLE)
		vkDestroyInstance(_instance, nullptr);
}

void VulkanManager::InitInstance(bool bEnableDebug)
{
	_bDebugEnable = bEnableDebug;
	//layers & extensions
	std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	std::vector<const char*> layers;

	//列举支持的layers和extensions
	{
		uint32_t count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		std::vector<VkLayerProperties> availableLaters(count);
		vkEnumerateInstanceLayerProperties(&count, availableLaters.data());
		ConsoleDebug::print_endl("----------Instance Layer Properties---------");
		for (uint32_t i = 0; i < count; i++)
		{
			char layerName[256];
			memcpy(layerName, availableLaters[i].layerName, 256);
			//_instance_layers.push_back(layerName);
			ConsoleDebug::print_endl(availableLaters[i].layerName + HString("  |  ") + availableLaters[i].description, "150,150,150");

			uint32_t ecount = 0;
			vkEnumerateInstanceExtensionProperties(availableLaters[i].layerName, &ecount, nullptr);
			std::vector<VkExtensionProperties> availableExts(ecount);
			vkEnumerateInstanceExtensionProperties(availableLaters[i].layerName, &ecount, availableExts.data());
			ConsoleDebug::print_endl("\tInstance Extension Properties---------");
			for (uint32_t i = 0; i < ecount; i++)
			{
				char extName[256];
				memcpy(extName, availableExts[i].extensionName, 256);
				ConsoleDebug::print_endl(HString("\t") + extName, "150,150,150");
				if (strcmp(availableExts[i].extensionName, VK_KHR_DISPLAY_EXTENSION_NAME) == 0)
				{
					layers.push_back(availableLaters[i].layerName);
					extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
					_enable_VK_KHR_display = true;
				}
			}
			ConsoleDebug::print_endl("\t------------------");
		}
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "hBBr";
	appInfo.pEngineName = "hBBr Engine";
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;

#if defined(_WIN32)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
	extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
	if (_bDebugEnable)
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
		debugCallbackCreateInfo.flags =
			VK_DEBUG_REPORT_WARNING_BIT_EXT
			| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
			| VK_DEBUG_REPORT_ERROR_BIT_EXT
			;
		createInfo.pNext = &debugCallbackCreateInfo;
	}
	createInfo.enabledExtensionCount = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = (uint32_t)layers.size();
	createInfo.ppEnabledLayerNames = layers.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		MessageOut(
			"Cannot find a compatible Vulkan installable client "
			"driver (ICD). Please make sure your driver supports "
			"Vulkan before continuing. The call to vkCreateInstance failed.", true);
	}
	else if (result != VK_SUCCESS) {
		MessageOut(
			"The call to vkCreateInstance failed. Please make sure "
			"you have a Vulkan installable client driver (ICD) before continuing.", true);
	}
}

void VulkanManager::InitDevice(VkSurfaceKHR surface)
{
	//--------------Get GPU
	{
		uint32_t devicesCount = 0;
		vkEnumeratePhysicalDevices(_instance, &devicesCount, nullptr);
		std::vector<VkPhysicalDevice> gpu_list(devicesCount);
		vkEnumeratePhysicalDevices(_instance, &devicesCount, gpu_list.data());
		ConsoleDebug::print_endl("-----------GPU List-----------", "0,255,0");
		ConsoleDebug::print_endl("Found " + HString::FromUInt(devicesCount) + " GPU(s)", "222,255,255");
		_gpuProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		for (uint32_t i = 0; i < devicesCount; i++)
		{
			vkGetPhysicalDeviceProperties2(gpu_list[i], &_gpuProperties);
			ConsoleDebug::print_endl("Device Name:" + HString(_gpuProperties.properties.deviceName), "200,255,255");
			ConsoleDebug::print_endl("Device ID:" + HString::FromUInt(_gpuProperties.properties.deviceID), "150,150,150");

			if (IsGPUDeviceSuitable(gpu_list[i]))
			{
				_gpuDevice = gpu_list[i];
				break;
			}
			//
		}
		ConsoleDebug::print_endl("---------------------------", "0,255,0");
		if (_gpuDevice == VK_NULL_HANDLE)
		{
			MessageOut("Can not find any gpu device.exit.",true);
		}
		vkGetPhysicalDeviceProperties2(_gpuDevice, &_gpuProperties);
		vkGetPhysicalDeviceMemoryProperties(_gpuDevice, &_gpuMemoryProperties);
	}
	//------------------Queue Family
	VkQueueFamilyProperties graphicsQueueFamilyProperty{};
	VkQueueFamilyProperties transferQueueFamilyProperty{};
	{
		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpuDevice, &family_count, nullptr);
		std::vector<VkQueueFamilyProperties> family_property_list(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpuDevice, &family_count, family_property_list.data());
		bool bFound_Graphics = false;
		//bool bFound_Transfer = false;
		for (uint32_t i = 0; i < family_count; i++) {
			//一般情况下Graphics queue同时支持多种功能,不需要另外获取Transfer等不同queue
			//但是据听说有的Graphics queue并不支持Present,所以需要同时判断Present支持
			if (bFound_Graphics == false && family_property_list[i].queueCount > 0 && family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				bFound_Graphics = true;
				_graphicsQueueFamilyIndex = i;
				graphicsQueueFamilyProperty = family_property_list[i];
				if (surface != VK_NULL_HANDLE)
				{
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(_gpuDevice, i, surface, &presentSupport);
					if (presentSupport) {
						break;
					}
				}
			}
			//if (bFound_Transfer == false && family_property_list[i].queueCount > 0 && family_property_list[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			//{
			//	bFound_Transfer = true;
			//	_transferQueueFamilyIndex = i;
			//	transferQueueFamilyProperty = family_property_list[i];
			//	break;
			//}
		}
		//if (!bFound_Graphics && !bFound_Transfer)
		if (!bFound_Graphics)
		{
			MessageOut("Vulkan ERROR: Queue family supporting graphics not found.Exit.",true);
		}
	}
	std::vector <const char*> layers;
	std::vector <const char*> extensions;
	//列举Device支持的Layers和Extensions
	{
		uint32_t count = 0;
		vkEnumerateDeviceLayerProperties(_gpuDevice, &count, nullptr);
		std::vector<VkLayerProperties> availableLaters(count);
		vkEnumerateDeviceLayerProperties(_gpuDevice, &count, availableLaters.data());
		ConsoleDebug::print_endl("----------Device Layer Properties---------");
		for (uint32_t i = 0; i < count; i++) {
			char layerName[256];
			memcpy(layerName, availableLaters[i].layerName, 256);
			//_device_layers.push_back(layerName);
			ConsoleDebug::print_endl(availableLaters[i].layerName + HString("  |  ") + availableLaters[i].description, "150,150,150");
			//
			uint32_t ecount = 0;
			vkEnumerateDeviceExtensionProperties(_gpuDevice, availableLaters[i].layerName, &ecount, nullptr);
			std::vector<VkExtensionProperties> availableExts(ecount);
			vkEnumerateDeviceExtensionProperties(_gpuDevice, availableLaters[i].layerName, &ecount, availableExts.data());
			ConsoleDebug::print_endl("\tInstance Extension Properties---------");
			for (uint32_t i = 0; i < ecount; i++)
			{
				char extName[256];
				memcpy(extName, availableExts[i].extensionName, 256);
				ConsoleDebug::print_endl(HString("\t") + extName, "150,150,150");
				//Debug Marker
				{
					if (strcmp(availableExts[i].extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
					{
						extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
						debugMarkerActive = true;
						break;
					}
				}

			}
			ConsoleDebug::print_endl("\t------------------");
		}
	}
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	extensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
	//允许深度/模板图像的图像存储屏障仅设置了深度或模板位之一，而不是两者都设置。
	extensions.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
	if (_bDebugEnable)
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
		//RenderDoc支持
		layers.push_back("VK_LAYER_RENDERDOC_Capture");//
		extensions.push_back("VK_EXT_debug_marker");
		extensions.push_back("VK_EXT_tooling_info");
	}
	//---------------------Create Device
	std::array<std::vector<float>, 1> prior;
	prior[0].resize(graphicsQueueFamilyProperty.queueCount, 0);
	//prior[1].resize(transferQueueFamilyProperty.queueCount, 0);

	std::vector<VkDeviceQueueCreateInfo> queue_create_info;
	VkDeviceQueueCreateInfo device_queue_create_info = {};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex = _graphicsQueueFamilyIndex;
	device_queue_create_info.queueCount = graphicsQueueFamilyProperty.queueCount;
	device_queue_create_info.pQueuePriorities = prior[0].data();
	queue_create_info.push_back(device_queue_create_info);
	//if (_graphicsQueueFamilyIndex != _transferQueueFamilyIndex)
	//{
	//	VkDeviceQueueCreateInfo device_transfer_queue_create_info = {};
	//	device_transfer_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//	device_transfer_queue_create_info.queueFamilyIndex = _transferQueueFamilyIndex;
	//	device_transfer_queue_create_info.queueCount = transferQueueFamilyProperty.queueCount;
	//	device_transfer_queue_create_info.pQueuePriorities = prior[1].data();
	//	queue_create_info.push_back(device_transfer_queue_create_info);
	//}
	vkGetPhysicalDeviceFeatures(_gpuDevice, &_gpuFeatures);
	//开启vk的gpu特殊功能
	_gpuVk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	_gpuVk12Features.separateDepthStencilLayouts = VK_TRUE;

	VkDeviceCreateInfo device_create_info = {};
	memset(&device_create_info, 0, sizeof(VkDeviceCreateInfo));
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info.size());
	device_create_info.pQueueCreateInfos = queue_create_info.data();
	device_create_info.ppEnabledLayerNames = layers.data();
	device_create_info.enabledLayerCount = (uint32_t)layers.size();
	device_create_info.ppEnabledExtensionNames = extensions.data();
	device_create_info.enabledExtensionCount = (uint32_t)extensions.size();
	device_create_info.pEnabledFeatures = &_gpuFeatures;
	device_create_info.pNext = &_gpuVk12Features;
	auto result = vkCreateDevice(_gpuDevice, &device_create_info, nullptr, &_device);
	if(result!= VK_SUCCESS) 
		MessageOut((HString("Create Device Failed.") + GetVkResult(result)).c_str() , true);
	vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_graphicsQueue);
	//vkGetDeviceQueue(_device, _transferQueueFamilyIndex, 0, &_transfer_Queue);
	if (_enable_VK_KHR_display)
	{
		uint32_t displayCount = 0;
		vkGetPhysicalDeviceDisplayPropertiesKHR(_gpuDevice, &displayCount, nullptr);
		std::vector<VkDisplayPropertiesKHR> displayPro(displayCount);
		vkGetPhysicalDeviceDisplayPropertiesKHR(_gpuDevice, &displayCount, displayPro.data());
		ConsoleDebug::print_endl("-----------Display info-----------", "0,255,0");
		for (uint32_t i = 0; i < displayCount; i++)
		{
			ConsoleDebug::print_endl(HString("-----------Display ") + HString::FromInt(i) + " -----------", "200,255,255");
			ConsoleDebug::print_endl("Display Name:" + HString(displayPro[i].displayName), "200,255,255");
			ConsoleDebug::print_endl("Display Width:" + HString::FromInt(displayPro[i].physicalResolution.width), "200,255,255");
			ConsoleDebug::print_endl("Display Height:" + HString::FromInt(displayPro[i].physicalResolution.height), "200,255,255");
			uint32_t count = 0;
			vkGetDisplayModePropertiesKHR(_gpuDevice, displayPro[i].display, &count, nullptr);
			std::vector<VkDisplayModePropertiesKHR> displayMode(count);
			vkGetDisplayModePropertiesKHR(_gpuDevice, displayPro[i].display, &count, displayMode.data());
			for (uint32_t i = 0; i < count; i++)
			{
				ConsoleDebug::print_endl("Display Refresh Rate:" + HString::FromInt(displayMode[i].parameters.refreshRate), "200,255,255");
			}
		}
	}
	if (debugMarkerActive)
	{
		//Debug Marker
		vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(_device, "vkDebugMarkerSetObjectTagEXT");
		vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(_device, "vkDebugMarkerSetObjectNameEXT");
		vkCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(_device, "vkCmdDebugMarkerBeginEXT");
		vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(_device, "vkCmdDebugMarkerEndEXT");
		vkCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(_device, "vkCmdDebugMarkerInsertEXT");
	}
}

void VulkanManager::InitDebug()
{
	if (_bDebugEnable)
	{
		fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
		if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT)
		{
			MessageOut("Vulkan ERROR: Cant fetch debug function pointers.");
		}
		fvkCreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, nullptr, &_debugReport);
	}
}

uint32_t VulkanManager::FindMemoryTypeIndex(const VkMemoryRequirements* memory_requirements, const VkMemoryPropertyFlags required_properties)
{
	for (uint32_t i = 0; i < _gpuMemoryProperties.memoryTypeCount; ++i) {
		if (memory_requirements->memoryTypeBits & (1 << i)) {
			if ((_gpuMemoryProperties.memoryTypes[i].propertyFlags & required_properties) == required_properties) {
				return i;
			}
		}
	}
	MessageOut("Cound not find memory type.");
	return UINT32_MAX;
}

bool VulkanManager::IsGPUDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		deviceFeatures.geometryShader;
}

void VulkanManager::CreateSurface(void* hWnd , VkSurfaceKHR& newSurface)
{
#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR info={};
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hwnd = (HWND)hWnd;
	info.hinstance = GetModuleHandle(NULL);
	auto result = vkCreateWin32SurfaceKHR(_instance, &info, nullptr, &newSurface);
	if (result != VK_SUCCESS)
	{
		MessageOut("Create Win32 surface failed.exit.",true);
	}
#endif
}

void VulkanManager::DestroySurface(VkSurfaceKHR& surface)
{
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(_instance, surface, nullptr);
	}
}

VkExtent2D VulkanManager::GetSurfaceSize(VkSurfaceKHR& surface)
{
	const int SwapchainBufferCount = 3;
	VkExtent2D surfaceSize = {};
	VkSurfaceCapabilitiesKHR _surfaceCapabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpuDevice, surface, &_surfaceCapabilities);
	if (_surfaceCapabilities.currentExtent.width < UINT32_MAX && _surfaceCapabilities.currentExtent.width>0) {
		surfaceSize.width = _surfaceCapabilities.currentExtent.width;
		surfaceSize.height = _surfaceCapabilities.currentExtent.height;
	}
	else {
		surfaceSize.width = _surfaceCapabilities.maxImageExtent.width < (uint32_t)surfaceSize.width ? _surfaceCapabilities.maxImageExtent.width : (uint32_t)surfaceSize.width;
		surfaceSize.width = _surfaceCapabilities.minImageExtent.width > (uint32_t)surfaceSize.width ? _surfaceCapabilities.minImageExtent.width : (uint32_t)surfaceSize.width;
		surfaceSize.height = _surfaceCapabilities.maxImageExtent.height < (uint32_t)surfaceSize.height ? _surfaceCapabilities.maxImageExtent.height : (uint32_t)surfaceSize.height;
		surfaceSize.height = _surfaceCapabilities.minImageExtent.height > (uint32_t)surfaceSize.height ? _surfaceCapabilities.minImageExtent.height : (uint32_t)surfaceSize.height;
	}
	_swapchainBufferCount = _surfaceCapabilities.minImageCount > SwapchainBufferCount ? _surfaceCapabilities.minImageCount : SwapchainBufferCount;
	_swapchainBufferCount = _surfaceCapabilities.maxImageCount < _swapchainBufferCount ? _surfaceCapabilities.maxImageCount : _swapchainBufferCount;
	return surfaceSize;
}

void VulkanManager::CheckSurfaceSupport(VkSurfaceKHR& surface, VkSurfaceFormatKHR& surfaceFormat)
{
	//Check Support
	VkBool32 IsSupportSurface = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(_gpuDevice, _graphicsQueueFamilyIndex, surface, &IsSupportSurface);
	if (!IsSupportSurface)
	{
		MessageOut("Vulkan ERROR: The GPU is Not Support Surface.",true);
	}
	{
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM,VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		uint32_t avail_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpuDevice, surface, &avail_count, NULL);
		std::vector<VkSurfaceFormatKHR> avail_format;
		avail_format.resize((int)avail_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpuDevice, surface, &avail_count, avail_format.data());
		if (avail_count == 1)
		{
			if (avail_format[0].format == VK_FORMAT_UNDEFINED)
			{
				surfaceFormat.format = requestSurfaceImageFormat[0];
				surfaceFormat.colorSpace = requestSurfaceColorSpace;
			}
			else
			{
				// No point in searching another format
				surfaceFormat = avail_format[0];
			}
		}
		else
		{
			// Request several formats, the first found will be used
			for (int request_i = 0; request_i < _countof(requestSurfaceImageFormat); request_i++)
				for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
					if (avail_format[avail_i].format == requestSurfaceImageFormat[request_i] && avail_format[avail_i].colorSpace == requestSurfaceColorSpace)
					{
						surfaceFormat = avail_format[avail_i];
						return;
					}
			// If none of the requested image formats could be found, use the first available
			surfaceFormat = avail_format[0];
		}
	}
}

void VulkanManager::CreateSwapchain(VkSurfaceKHR& surface, VkSurfaceFormatKHR& surfaceFormat , VkSwapchainKHR &newSwapchain)
{
	//ConsoleDebug::print_endl("Create Swapchain KHR.");
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	//if (_winInfo.vsync)
	{
		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, nullptr);
		std::vector<VkPresentModeKHR> presentModes(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, presentModes.data());
		for (int i = 0; i < presentModes.size(); i++)
		{
			//if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			//{
			//	present_mode = presentModes[i];//这个垂直同步在Nvidia有bug ?
			//	//break;
			//}
			if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
			{
				present_mode = presentModes[i];//垂直同步
				break;
			}
		}
	}

	//Get surface size 
	VkExtent2D _surfaceSize = GetSurfaceSize(surface);
	if (_surfaceSize.width <= 0 && _surfaceSize.height <= 0)
	{
		return;
	}
	{
		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = surface;
		info.minImageCount = _swapchainBufferCount;
		info.imageFormat = surfaceFormat.format;
		info.imageColorSpace = surfaceFormat.colorSpace;
		info.imageExtent = _surfaceSize;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT	//支持在RenderPass中作为color附件，并且在subpass中进行传递
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT					//支持复制到其他图像
			;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices = nullptr;
		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//是否半透明，用于组合其他表面,这里我们不需要
		info.presentMode = present_mode;
		info.clipped = VK_TRUE;//是否不渲染看不见的位置
		//替补swapchain,在重置这个swapchain的时候会比较有用，但是事实上我们可以用更暴力的方法重置swapchain，也就是完全重写
		info.oldSwapchain = VK_NULL_HANDLE;
		auto result = vkCreateSwapchainKHR(_device, &info, nullptr, &newSwapchain);
		if (result != VK_SUCCESS)
		{
			MessageOut((HString("Create Swapchain KHR fail.") + GetVkResult(result)).c_str(),true);
		}
		if (newSwapchain)
			vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, nullptr);
	}
}

void VulkanManager::DestroySwapchain(VkSwapchainKHR& swapchain)
{
	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_device, swapchain, nullptr);
	}
}

void VulkanManager::CreateImage(uint32_t width , uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, VkImage& image)
{
	VkExtent2D texSize = {};
	texSize.width = width;
	texSize.height = height;
	VkImageCreateInfo	create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.flags = 0;
	create_info.format = format;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.usage = usageFlags;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.arrayLayers = 1;
	create_info.mipLevels = 1;
	create_info.extent.depth = 1;
	create_info.extent.width = texSize.width;
	create_info.extent.height = texSize.height;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	create_info.pQueueFamilyIndices = nullptr;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	auto result = vkCreateImage(_device, &create_info, nullptr, &image);
	if (result != VK_SUCCESS)
	{
		MessageOut("Create vulkan image failed.",false,false);
	}
}

void VulkanManager::CreateImageViewAndMemory(VkImage& inImage, VkFormat format, VkImageAspectFlags aspectFlags, VkDeviceMemory& imageViewMemory, VkImageView& imageView, VkMemoryPropertyFlags memoryPropertyFlag)
{
	if (inImage == VK_NULL_HANDLE)
	{
		MessageOut("Create vulkan image view failed.VkImage is NULL.", false, false);
		return;
	}
	VkMemoryRequirements mem_requirement;
	vkGetImageMemoryRequirements(_device, inImage, &mem_requirement);
	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = mem_requirement.size;
	memory_allocate_info.memoryTypeIndex = FindMemoryTypeIndex(&mem_requirement, memoryPropertyFlag);
	auto err = vkAllocateMemory(_device, &memory_allocate_info, nullptr, &imageViewMemory);
	if (VK_SUCCESS != err) {
		MessageOut("Create vulkan image view failed.VkImage is NULL.", false, false);
	}
	err = vkBindImageMemory(_device, inImage, imageViewMemory, 0);
	if (VK_SUCCESS != err) {
		MessageOut("Create vulkan image view failed.VkImage is NULL.", false, false);
	}
	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.flags = 0;
	image_view_create_info.image = inImage;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = format;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.aspectMask = aspectFlags;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.layerCount = 1;
	image_view_create_info.subresourceRange.levelCount = 1;
	vkCreateImageView(_device, &image_view_create_info, nullptr, &imageView);
}

HString VulkanManager::GetVkResult(VkResult code)
{
	HString text = "Code: ";
	switch (code)
	{
	case VK_SUCCESS:
		text += "VK_SUCCESS";
		break;
	case VK_NOT_READY:
		text += "VK_NOT_READY";
		break;
	case VK_TIMEOUT:
		text += "VK_TIMEOUT";
		break;
	case VK_EVENT_SET:
		text += "VK_EVENT_SET";
		break;
	case VK_EVENT_RESET:
		text += "VK_EVENT_RESET";
		break;
	case VK_INCOMPLETE:
		text += "VK_INCOMPLETE";
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		text += "VK_ERROR_OUT_OF_HOST_MEMORY";
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		text += "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		text += "VK_ERROR_INITIALIZATION_FAILED";
		break;
	case VK_ERROR_DEVICE_LOST:
		text += "VK_ERROR_DEVICE_LOST";
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		text += "VK_ERROR_MEMORY_MAP_FAILED";
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		text += "VK_ERROR_LAYER_NOT_PRESENT";
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		text += "VK_ERROR_EXTENSION_NOT_PRESENT";
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		text += "VK_ERROR_FEATURE_NOT_PRESENT";
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		text += "VK_ERROR_INCOMPATIBLE_DRIVER";
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		text += "VK_ERROR_TOO_MANY_OBJECTS";
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		text += "VK_ERROR_FORMAT_NOT_SUPPORTED";
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		text += "VK_ERROR_FRAGMENTED_POOL";
		break;
	case VK_ERROR_UNKNOWN:
		text += "VK_ERROR_UNKNOWN";
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		text += "VK_ERROR_OUT_OF_POOL_MEMORY";
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		text += "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		break;
	case VK_ERROR_FRAGMENTATION:
		text += "VK_ERROR_FRAGMENTATION";
		break;
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		text += "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		text += "VK_ERROR_SURFACE_LOST_KHR";
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		text += "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		break;
	case VK_SUBOPTIMAL_KHR:
		text += "VK_SUBOPTIMAL_KHR";
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		text += "VK_ERROR_OUT_OF_DATE_KHR";
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		text += "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		text += "VK_ERROR_VALIDATION_FAILED_EXT";
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		text += "VK_ERROR_INVALID_SHADER_NV";
		break;
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		text += "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		break;
	case VK_ERROR_NOT_PERMITTED_EXT:
		text += "VK_ERROR_NOT_PERMITTED_EXT";
		break;
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		text += "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		break;
	case VK_THREAD_IDLE_KHR:
		text += "VK_THREAD_IDLE_KHR";
		break;
	case VK_THREAD_DONE_KHR:
		text += "VK_THREAD_DONE_KHR";
		break;
	case VK_OPERATION_DEFERRED_KHR:
		text += "VK_OPERATION_DEFERRED_KHR";
		break;
	case VK_OPERATION_NOT_DEFERRED_KHR:
		text += "VK_OPERATION_NOT_DEFERRED_KHR";
		break;
	case VK_PIPELINE_COMPILE_REQUIRED_EXT:
		text += "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		break;
	default:
		text = "[Unknow???]";
		break;
	}
	return text;
}

void MessageOut(const char* msg, bool bExit, bool bMessageBox)
{
#if defined(_WIN32)
	printf(msg);
	if(bMessageBox)
		MessageBoxA(NULL, msg, "message", MB_ICONERROR);
#else
	printf(msg);
	fflush(stdout);
#endif
	if (bExit)
		exit(EXIT_FAILURE);
}