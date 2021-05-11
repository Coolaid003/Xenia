/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_VFS_DEVICES_STFS_CONTAINER_DEVICE_H_
#define XENIA_VFS_DEVICES_STFS_CONTAINER_DEVICE_H_

#include <map>
#include <memory>
#include <string>

#include "xenia/base/math.h"
#include "xenia/base/string_util.h"
#include "xenia/kernel/util/xex2_info.h"
#include "xenia/vfs/device.h"
#include "xenia/vfs/devices/stfs_xbox.h"

namespace xe {
namespace vfs {

// https://free60project.github.io/wiki/STFS.html

class StfsContainerEntry;

class StfsContainerDevice : public Device {
 public:
  StfsContainerDevice(const std::string_view mount_path,
                      const std::filesystem::path& host_path);
  ~StfsContainerDevice() override;

  bool Initialize() override;
  void Dump(StringBuffer* string_buffer) override;
  Entry* ResolvePath(const std::string_view path) override;

  const std::string& name() const override { return name_; }
  uint32_t attributes() const override { return 0; }
  uint32_t component_name_max_length() const override { return 40; }

  uint32_t total_allocation_units() const override {
    return uint32_t(data_size() / sectors_per_allocation_unit() /
                    bytes_per_sector());
  }
  uint32_t available_allocation_units() const override { return 0; }
  uint32_t sectors_per_allocation_unit() const override { return 8; }
  uint32_t bytes_per_sector() const override { return 0x200; }

  // Gives rough estimate of the size of the data in this container
  // TODO: use allocated_block_count inside volume-descriptor?
  size_t data_size() const {
    if (header_.header.header_size) {
      if (header_.metadata.volume_type == XContentVolumeType::kStfs &&
          header_.metadata.volume_descriptor.stfs.is_valid()) {
        return header_.metadata.volume_descriptor.stfs.total_block_count *
               kSectorSize;
      }
      return files_total_size_ -
             xe::round_up(header_.header.header_size, kSectorSize);
    }
    return files_total_size_ - sizeof(StfsHeader);
  }

 private:
  const uint32_t kSectorSize = 0x1000;
  const uint32_t kBlocksPerHashLevel[3] = {170, 28900, 4913000};

  enum class Error {
    kSuccess = 0,
    kErrorOutOfMemory = -1,
    kErrorReadError = -10,
    kErrorFileMismatch = -30,
    kErrorDamagedFile = -31,
    kErrorTooSmall = -32,
  };

  enum class SvodLayoutType {
    kUnknown = 0x0,
    kEnhancedGDF = 0x1,
    kXSF = 0x2,
    kSingleFile = 0x4,
  };

  XContentPackageType ReadMagic(const std::filesystem::path& path);
  bool ResolveFromFolder(const std::filesystem::path& path);

  Error OpenFiles();
  void CloseFiles();

  Error ReadHeaderAndVerify(FILE* header_file);

  Error ReadSVOD();
  Error ReadEntrySVOD(uint32_t sector, uint32_t ordinal,
                      StfsContainerEntry* parent);
  void BlockToOffsetSVOD(size_t sector, size_t* address, size_t* file_index);

  Error ReadSTFS();
  size_t BlockToOffsetSTFS(uint64_t block_index) const;
  uint32_t BlockToHashBlockNumberSTFS(uint32_t block_index,
                                      uint32_t hash_level) const;
  size_t BlockToHashBlockOffsetSTFS(uint32_t block_index,
                                    uint32_t hash_level) const;

  const StfsHashEntry* GetBlockHash(uint32_t block_index);

  std::string name_;
  std::filesystem::path host_path_;

  std::map<size_t, FILE*> files_;
  size_t files_total_size_;

  size_t base_offset_;
  size_t magic_offset_;
  std::unique_ptr<Entry> root_entry_;
  StfsHeader header_;
  SvodLayoutType svod_layout_;
  uint32_t blocks_per_hash_table_;
  uint32_t block_step[2];

  std::unordered_map<size_t, StfsHashTable> cached_hash_tables_;
};

}  // namespace vfs
}  // namespace xe

#endif  // XENIA_VFS_DEVICES_STFS_CONTAINER_DEVICE_H_
