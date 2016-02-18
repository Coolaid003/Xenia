/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2016 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/ui/vulkan/vulkan_device.h"

#include <gflags/gflags.h>

#include <cinttypes>
#include <mutex>
#include <string>

#include "xenia/base/assert.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/base/profiling.h"
#include "xenia/ui/vulkan/vulkan.h"
#include "xenia/ui/vulkan/vulkan_immediate_drawer.h"
#include "xenia/ui/vulkan/vulkan_util.h"
#include "xenia/ui/window.h"

namespace xe {
namespace ui {
namespace vulkan {

VulkanDevice::VulkanDevice(VulkanInstance* instance) : instance_(instance) {
  if (FLAGS_vulkan_validation) {
    /*DeclareRequiredLayer("VK_LAYER_GOOGLE_unique_objects",
                         Version::Make(0, 0, 0), true);*/
    DeclareRequiredLayer("VK_LAYER_LUNARG_threading", Version::Make(0, 0, 0),
                         true);
    /*DeclareRequiredLayer("VK_LAYER_LUNARG_mem_tracker", Version::Make(0, 0,
       0),
                         true);*/
    DeclareRequiredLayer("VK_LAYER_LUNARG_object_tracker",
                         Version::Make(0, 0, 0), true);
    DeclareRequiredLayer("VK_LAYER_LUNARG_draw_state", Version::Make(0, 0, 0),
                         true);
    DeclareRequiredLayer("VK_LAYER_LUNARG_param_checker",
                         Version::Make(0, 0, 0), true);
    DeclareRequiredLayer("VK_LAYER_LUNARG_swapchain", Version::Make(0, 0, 0),
                         true);
    DeclareRequiredLayer("VK_LAYER_LUNARG_device_limits",
                         Version::Make(0, 0, 0), true);
    DeclareRequiredLayer("VK_LAYER_LUNARG_image", Version::Make(0, 0, 0), true);
  }
}

VulkanDevice::~VulkanDevice() {
  if (handle) {
    vkDestroyDevice(handle, nullptr);
    handle = nullptr;
  }
}

bool VulkanDevice::Initialize(DeviceInfo device_info) {
  // Gather list of enabled layer names.
  auto layers_result = CheckRequirements(required_layers_, device_info.layers);
  auto& enabled_layers = layers_result.second;

  // Gather list of enabled extension names.
  auto extensions_result =
      CheckRequirements(required_extensions_, device_info.extensions);
  auto& enabled_extensions = extensions_result.second;

  // We wait until both extensions and layers are checked before failing out so
  // that the user gets a complete list of what they have/don't.
  if (!extensions_result.first || !layers_result.first) {
    FatalVulkanError(
        "Layer and extension verification failed; aborting initialization");
    return false;
  }

  // Query supported features so we can make sure we have what we need.
  VkPhysicalDeviceFeatures supported_features;
  vkGetPhysicalDeviceFeatures(device_info.handle, &supported_features);
  VkPhysicalDeviceFeatures enabled_features = {0};
  bool any_features_missing = false;
#define ENABLE_AND_EXPECT(name)                                  \
  if (!supported_features.name) {                                \
    any_features_missing = true;                                 \
    FatalVulkanError("Vulkan device is missing feature " #name); \
  } else {                                                       \
    enabled_features.name = VK_TRUE;                             \
  }
  ENABLE_AND_EXPECT(geometryShader);
  ENABLE_AND_EXPECT(depthClamp);
  ENABLE_AND_EXPECT(alphaToOne);
  ENABLE_AND_EXPECT(multiViewport);
  // TODO(benvanik): add other features.
  if (any_features_missing) {
    XELOGE(
        "One or more required device features are missing; aborting "
        "initialization");
    return false;
  }

  // Pick a queue.
  // Any queue we use must support both graphics and presentation.
  // TODO(benvanik): use multiple queues (DMA-only, compute-only, etc).
  if (device_info.queue_family_properties.empty()) {
    FatalVulkanError("No queue families available");
    return false;
  }
  uint32_t ideal_queue_family_index = UINT_MAX;
  uint32_t queue_count = 1;
  for (size_t i = 0; i < device_info.queue_family_properties.size(); ++i) {
    auto queue_flags = device_info.queue_family_properties[i].queueFlags;
    if (!device_info.queue_family_supports_present[i]) {
      // Can't present from this queue, so ignore it.
      continue;
    }
    if (queue_flags & VK_QUEUE_GRAPHICS_BIT) {
      // Can do graphics and present - good!
      ideal_queue_family_index = static_cast<uint32_t>(i);
      // TODO(benvanik): pick a higher queue count?
      queue_count = 1;
      break;
    }
  }
  if (ideal_queue_family_index == UINT_MAX) {
    FatalVulkanError(
        "No queue families available that can both do graphics and present");
    return false;
  }

  VkDeviceQueueCreateInfo queue_info;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = nullptr;
  queue_info.flags = 0;
  queue_info.queueFamilyIndex = ideal_queue_family_index;
  queue_info.queueCount = queue_count;
  std::vector<float> queue_priorities(queue_count);
  queue_info.pQueuePriorities = queue_priorities.data();

  VkDeviceCreateInfo create_info;
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.queueCreateInfoCount = 1;
  create_info.pQueueCreateInfos = &queue_info;
  create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
  create_info.ppEnabledLayerNames = enabled_layers.data();
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(enabled_extensions.size());
  create_info.ppEnabledExtensionNames = enabled_extensions.data();
  create_info.pEnabledFeatures = &enabled_features;

  auto err = vkCreateDevice(device_info.handle, &create_info, nullptr, &handle);
  switch (err) {
    case VK_SUCCESS:
      // Ok!
      break;
    case VK_ERROR_INITIALIZATION_FAILED:
      FatalVulkanError("Device initialization failed; generic");
      return false;
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      FatalVulkanError(
          "Device initialization failed; requested extension not present");
      return false;
    case VK_ERROR_LAYER_NOT_PRESENT:
      FatalVulkanError(
          "Device initialization failed; requested layer not present");
      return false;
    default:
      FatalVulkanError(std::string("Device initialization failed; unknown: ") +
                       to_string(err));
      return false;
  }

  device_info_ = std::move(device_info);
  queue_family_index_ = ideal_queue_family_index;

  // Get the primary queue used for most submissions/etc.
  vkGetDeviceQueue(handle, queue_family_index_, 0, &primary_queue_);

  XELOGVK("Device initialized successfully!");
  return true;
}

VkDeviceMemory VulkanDevice::AllocateMemory(
    const VkMemoryRequirements& requirements, VkFlags required_properties) {
  // Search memory types to find one matching our requirements and our
  // properties.
  uint32_t type_index = UINT_MAX;
  for (uint32_t i = 0; i < device_info_.memory_properties.memoryTypeCount;
       ++i) {
    const auto& memory_type = device_info_.memory_properties.memoryTypes[i];
    if (((requirements.memoryTypeBits >> i) & 1) == 1) {
      // Type is available for use; check for a match on properties.
      if ((memory_type.propertyFlags & required_properties) ==
          required_properties) {
        type_index = i;
        break;
      }
    }
  }
  if (type_index == UINT_MAX) {
    XELOGE("Unable to find a matching memory type");
    return nullptr;
  }

  // Allocate the memory.
  VkMemoryAllocateInfo memory_info;
  memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memory_info.pNext = nullptr;
  memory_info.allocationSize = requirements.size;
  memory_info.memoryTypeIndex = type_index;
  VkDeviceMemory memory = nullptr;
  auto err = vkAllocateMemory(handle, &memory_info, nullptr, &memory);
  CheckResult(err, "vkAllocateMemory");
  return memory;
}

}  // namespace vulkan
}  // namespace ui
}  // namespace xe
