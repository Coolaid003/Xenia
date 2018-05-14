/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2018 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <gflags/gflags.h>

#include "xenia/app/emulator_window.h"
#include "xenia/gpu/vulkan/vulkan_graphics_system.h"

#include "xenia/ui/vulkan/vulkan_instance.h"
#include "xenia/ui/vulkan/vulkan_provider.h"
#include "xenia/ui/opengl/opengl_provider.h"
#include <QVulkanWindow>

DEFINE_string(apu, "any", "Audio system. Use: [any, nop, xaudio2]");
DEFINE_string(gpu, "any", "Graphics system. Use: [any, vulkan, null]");
DEFINE_string(hid, "any", "Input system. Use: [any, nop, winkey, xinput]");

DEFINE_string(target, "", "Specifies the target .xex or .iso to execute.");
DEFINE_bool(fullscreen, false, "Toggles fullscreen");

namespace xe {
namespace app {

EmulatorWindow::EmulatorWindow() {}

bool EmulatorWindow::Setup() {
  // TODO(DrChat): Pass in command line arguments.
  emulator_ = std::make_unique<xe::Emulator>(L"");

  // Initialize the graphics backend.
  // TODO(DrChat): Pick from gpu command line flag.
  if (!InitializeVulkan()) {
    return false;
  }

  auto graphics_factory = [&](cpu::Processor* processor,
                              kernel::KernelState* kernel_state) {
    auto graphics = std::make_unique<gpu::vulkan::VulkanGraphicsSystem>();
    if (graphics->Setup(processor, kernel_state,
                        graphics_provider_->CreateOffscreenContext())) {
      return std::unique_ptr<gpu::vulkan::VulkanGraphicsSystem>(nullptr);
    }

    return graphics;
  };

  X_STATUS result = emulator_->Setup(nullptr, graphics_factory, nullptr);
  return result == X_STATUS_SUCCESS;
}

bool EmulatorWindow::InitializeVulkan() {
  auto provider = xe::ui::vulkan::VulkanProvider::Create(nullptr);
  auto device = provider->device();

  // Create a Qt wrapper around our vulkan instance.
  vulkan_instance_ = std::make_unique<QVulkanInstance>();
  vulkan_instance_->setVkInstance(*provider->instance());

  graphics_window_ = std::make_unique<QVulkanWindow>();
  graphics_window_->setVulkanInstance(vulkan_instance_.get());

  graphics_provider_ = std::move(provider);
  return true;
}

}  // namespace app
}  // namespace xe
