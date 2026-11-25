// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "assetc_pch.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <cstdio>
#include <cstdlib>
#include <filesystem>
////////////////////////////////////////////////////////////////////////////////

#include "asset_processor.h"
#include "comet/core.h"

namespace comet {
namespace tool {
namespace assetc {
namespace {
enum class ArgumentResult { Success, Help, Invalid };

struct Arguments {
  std::filesystem::path asset_path{};
  std::filesystem::path resource_path{};
  bool force{false};
};

void PrintUsage() {
  std::puts("Usage: assetc [-a PATH] [-r PATH] [-f] [-h]");
  std::puts("  -f, --force       Reprocess assets even when unchanged");
  std::puts("  -h, --help        Show this help");
  std::puts("  -a, --assets      Asset input directory (default: assets)");
  std::puts("  -r, --resources   Compiled resource directory (default: resources)");
}

ArgumentResult ParseArguments(int argc, char** argv, Arguments& arguments) {
  command_line::Option options[]{
      {"help", false, false, nullptr, 'h'},
      {"force", false, false, nullptr, 'f'},
      {"assets", true, false, nullptr, 'a'},
      {"resources", true, false, nullptr, 'r'},
  };
  const schar* invalid_argument{nullptr};
  const auto result{command_line::Parse(
      argc, argv, options, sizeof(options) / sizeof(options[0]),
      &invalid_argument)};
  if (result != command_line::ParseResult::Success) {
    std::fprintf(stderr, "Invalid argument: %s\n",
                 invalid_argument != nullptr ? invalid_argument : "(null)");
    return ArgumentResult::Invalid;
  }
  if (options[0].is_present) {
    return ArgumentResult::Help;
  }
  // Convert UTF-8 paths to native paths.
  arguments.asset_path = std::filesystem::u8path(
      options[2].is_present ? options[2].value : "assets");
  arguments.resource_path = std::filesystem::u8path(
      options[3].is_present ? options[3].value : "resources");
  arguments.force = options[1].is_present;
  return ArgumentResult::Success;
}

job::SchedulerConfig GenerateSchedulerConfig(memory::Allocator* allocator) {
  job::SchedulerConfig scheduler_config{};
  scheduler_config.allocators.job_queue_allocator = allocator;
  scheduler_config.allocators.worker_allocator = allocator;
  scheduler_config.allocators.fiber_queue_allocator = allocator;
  scheduler_config.allocators.fiber_object_allocator = allocator;
  scheduler_config.allocators.fiber_stack_allocator = allocator;
  scheduler_config.allocators.fiber_life_cycle_allocator = allocator;
  scheduler_config.allocators.counter_queue_allocator = allocator;
  scheduler_config.allocators.counter_allocator = allocator;

  scheduler_config.job_queue_capacity = 1024;
  scheduler_config.counter_count = 256;
  scheduler_config.large_fiber_count = 32;
  scheduler_config.gigantic_fiber_count = 4;
  scheduler_config.fiber_life_cycle_queue_capacity = 128;
  scheduler_config.forced_fiber_worker_count = 0;
  scheduler_config.forced_io_worker_count = 2;
  scheduler_config.default_io_worker_count = 2;
  scheduler_config.promotion_interval = 1000;

  return scheduler_config;
}

void InitializeServices(memory::Allocator* allocator,
                        job::SchedulerConfig& scheduler_config) {
  COMET_INITIALIZE_ALLOCATION_TRACKING();
  thread::Thread::AttachMainThread();
  COMET_LOG_INITIALIZE();

  memory::AttachDefaultAllocator(allocator);

  InitializeFileSystem({
      .get_scratch_allocator = []() -> memory::Allocator* {
        return &memory::GetDefaultAllocator();
      },
  });

  auto& scheduler{job::Scheduler::Get()};
  scheduler.AttachConfig(scheduler_config);
  scheduler.Initialize();
}

void ShutdownServices() {
  auto& scheduler{job::Scheduler::Get()};
  scheduler.Shutdown();
  scheduler.DetachConfig();

  ShutdownFileSystem();
  memory::DetachDefaultAllocator();

  COMET_LOG_DESTROY();
  thread::Thread::DetachMainThread();
  COMET_DESTROY_ALLOCATION_TRACKING();
}

void ProcessAssets(const Arguments& arguments, memory::Allocator* allocator) {
  AssetcConfig config{};
  config.asset_root = CTStringView{arguments.asset_path.c_str()};
  config.resource_root = CTStringView{arguments.resource_path.c_str()};
  config.allocator = allocator;
  config.force = arguments.force;

  AssetProcessor processor{};
  processor.Initialize(config);
  processor.Refresh();
  processor.Shutdown();
}
}  // namespace

int Run(int argc, char** argv) {
  Arguments arguments{};
  const auto result{ParseArguments(argc, argv, arguments)};

  if (result == ArgumentResult::Help) {
    PrintUsage();
    return EXIT_SUCCESS;
  }

  if (result == ArgumentResult::Invalid) {
    return EXIT_FAILURE;
  }

  memory::SystemAllocator allocator{};
  auto scheduler_config{GenerateSchedulerConfig(&allocator)};
  InitializeServices(&allocator, scheduler_config);
  ProcessAssets(arguments, &allocator);
  ShutdownServices();
  return EXIT_SUCCESS;
}
}  // namespace assetc
}  // namespace tool
}  // namespace comet

int main(int argc, char** argv) {
  return comet::tool::assetc::Run(argc, argv);
}
