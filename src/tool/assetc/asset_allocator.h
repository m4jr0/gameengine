#ifndef COMET_TOOL_ASSETC_ASSET_ALLOCATOR_H_
#define COMET_TOOL_ASSETC_ASSET_ALLOCATOR_H_

#include <new>

#include "comet/core.h"
#include "comet/core/essentials.h"

namespace comet {
namespace tool {
namespace asset {

class AssetAllocator final : public memory::Allocator {
 public:
  void* AllocateAligned(usize size, memory::Alignment align) override {
    return ::operator new(size, std::align_val_t{align});
  }

  void Deallocate(void* ptr) override {
    ::operator delete(ptr, std::align_val_t{alignof(std::max_align_t)});
  }
};

}  // namespace asset
}  // namespace tool
}  // namespace comet

#endif  // COMET_TOOL_ASSETC_ASSET_ALLOCATOR_H_