// tools/assetc/asset/asset_manager.h
#ifndef COMET_TOOL_ASSETC_ASSET_PROCESSOR_H_
#define COMET_TOOL_ASSETC_ASSET_PROCESSOR_H_

#include "comet/core.h"
#include "exporter/asset_exporter.h"

namespace comet {
namespace tool {
namespace assetc {
struct AssetcConfig {
  CTStringView asset_root{};
  CTStringView resource_root{};
  memory::Allocator* allocator{nullptr};
  bool force{false};
  usize worker_count{0};
};

class AssetProcessor {
 public:
  AssetProcessor() = default;
  AssetProcessor(const AssetProcessor&) = delete;
  AssetProcessor(AssetProcessor&&) = delete;
  AssetProcessor& operator=(const AssetProcessor&) = delete;
  AssetProcessor& operator=(AssetProcessor&&) = delete;
  ~AssetProcessor() = default;

  void Initialize(const AssetcConfig& config);
  void Shutdown();

  void RefreshLibraryMetadataFile();
  void Refresh();

  const TString& GetAssetsRootPath() const noexcept;
  const TString& GetResourcesRootPath() const noexcept;

 private:
  void RefreshLibrary();
  void RefreshFolder(CTStringView asset_abs_path);
  void RefreshAsset(CTStringView asset_abs_path);

  bool IsRefreshNeeded(CTStringView asset_abs_path,
                       CTStringView metadata_file_path) const;

  bool is_initialized_{false};
  bool is_force_refresh_{false};

  memory::Allocator* allocator_{nullptr};

  TString root_asset_path_{};
  TString root_resource_path_{};
  TString library_meta_path_{};

  Array<memory::UniquePtr<AssetExporter>> exporters_{};
};

}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_TOOL_ASSETC_ASSET_PROCESSOR_H_