// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_TSTRING_H_
#define COMET_COMET_CORE_TYPE_TSTRING_H_

#include "comet/core/c_array.h"
#include "comet/core/c_string.h"
#include "comet/core/essentials.h"
#include "comet/core/file_system/slash_helper.h"
#include "comet/core/hash.h"
#include "comet/core/memory/allocator/allocator.h"

namespace comet {
namespace internal {
memory::Allocator* GetTStringAllocator();

class TStringAllocator : public memory::Allocator {
 public:
  static TStringAllocator& Get();

  TStringAllocator() = default;
  TStringAllocator(const TStringAllocator&) = delete;
  TStringAllocator(TStringAllocator&&) = delete;
  TStringAllocator& operator=(const TStringAllocator&) = delete;
  TStringAllocator& operator=(TStringAllocator&&) = delete;

  void* AllocateAligned(usize size, memory::Alignment align) override;
  void Deallocate(void* ptr) override;
};
}  // namespace internal

void InitializeTStrings();
void DestroyTStrings();
void AttachTStringAllocator(memory::Allocator* allocator);
void DetachTStringAllocator();

const auto kSSOCapacityThreshold{15};

class CTStringView;

class TString {
 public:
  TString() = default;

  template <typename TChar>
  TString(const TChar* str, usize length) : length_{length} {
    Allocate(length_);
    Copy(GetTStr(), str, length_);
    GetTStr()[length_] = COMET_TCHAR('\0');
  }

  template <typename TChar>
  explicit TString(TChar c) : TString{&c, 1} {}

  template <typename TChar>
  explicit TString(const TChar* str) : TString{str, comet::GetLength(str)} {}

  template <typename TChar>
  TString(usize length, TChar c) : length_{length} {
    Allocate(length_);
    Copy(GetTStr(), &c, length_);
    GetTStr()[length_] = COMET_TCHAR('\0');
  }

  TString(const TString& other);
  TString(const TString& other, usize pos, usize length = kInvalidIndex);

  template <typename TChar>
  TString(const TChar* start, const TChar* end)
      : length_{static_cast<usize>(end - start)} {
    Allocate(length_);
    Copy(GetTStr(), start, length_);
    GetTStr()[length_] = COMET_TCHAR('\0');
  }

  TString(TString&& other) noexcept;
  TString& operator=(const TString& other);
  TString& operator=(const CTStringView& other);
  TString& operator=(const tchar* other);
  TString& operator=(TString&& other) noexcept;
  ~TString();

  void Reserve(usize capacity);
  void Resize(usize length);
  void Clear();

  template <typename TChar>
  TString& Append(usize length, TChar c) {
    auto new_length{length + length_};

    // Use >= to take the null terminator into account.
    if (new_length >= capacity_) {
      Reserve(new_length);
    }

    auto offset{length_ - 1};

    for (usize i{0}; i < length; ++i) {
      GetTStr()[offset++] = c;
    }

    length_ = new_length;
    GetTStr()[length_] = COMET_TCHAR('\0');
    return *this;
  }

  TString& Append(const TString& str);
  TString& Append(const TString& str, usize offset,
                  usize length = kInvalidIndex);

  template <typename TChar>
  TString& Append(const TChar* str, usize length) {
    auto new_length{length_ + length};

    if (new_length > capacity_) {
      Reserve(new_length);
    }

    Copy(GetTStr(), str, length, length_);
    length_ = new_length;
    GetTStr()[length_] = COMET_TCHAR('\0');
    return *this;
  }

  template <typename TChar>
  TString& Append(const TChar* str) {
    return Append(str, comet::GetLength(str));
  }

  template <typename TChar>
  TString& Append(std::initializer_list<TChar> cs) {
    return Append(cs.begin(), cs.size());
  }

  TString GenerateSubString(usize offset = 0,
                            usize count = kInvalidIndex) const;
  bool IsContained(tchar c) const;
  usize GetIndex(tchar c) const;
  usize GetLastIndexOf(tchar c, usize offset = kInvalidIndex) const noexcept;
  usize GetNthToLastIndexOf(tchar c, usize count = 0,
                            usize offset = kInvalidIndex) const noexcept;
  friend void Swap(TString& str1, TString& str2);
  tchar& operator[](usize index);
  const tchar& operator[](usize index) const;
  const tchar* GetCTStr() const noexcept;
  tchar* GetTStr() noexcept;
  usize GetLength() const noexcept;
  usize GetLengthWithNullTerminator() const noexcept;
  usize GetCapacity() const noexcept;
  bool IsEmpty() const noexcept;
  const tchar& GetFirst() const noexcept;
  const tchar& GetLast() const noexcept;
  operator const tchar*() const noexcept;

#ifdef COMET_DEBUG
  void AllowAlloc() noexcept;
  void DisallowAlloc() noexcept;
#endif  // COMET_DEBUG

 private:
  void Allocate(usize capacity);
  void Deallocate();

  usize length_{0};
#ifdef COMET_DEBUG
  bool is_alloc_allowed_{true};
#endif  // COMET_DEBUG
  usize capacity_{kSSOCapacityThreshold};
  tchar* str_{nullptr};
  tchar sso_[kSSOCapacityThreshold + 1]{COMET_TCHAR('\0')};
};

#ifndef COMET_DEBUG
#define COMET_ALLOW_STR_ALLOC(str)
#define COMET_DISALLOW_STR_ALLOC(str)
#else
#define COMET_ALLOW_STR_ALLOC(str) str.AllowAlloc()
#define COMET_DISALLOW_STR_ALLOC(str) str.DisallowAlloc()
#endif  // !COMET_DEBUG

class CTStringView {
 public:
  CTStringView(const TString& str) noexcept : CTStringView{str.GetCTStr()} {}

  constexpr CTStringView(const tchar* str) noexcept
      : str_{str}, length_{comet::GetLength(str)} {}

  constexpr CTStringView(const tchar* str, usize length) noexcept
      : str_{str}, length_{length} {}

  constexpr CTStringView(const CTStringView& other) noexcept = default;
  constexpr CTStringView(CTStringView&& other) noexcept = default;

  constexpr CTStringView& operator=(const CTStringView& other) = default;
  constexpr CTStringView& operator=(CTStringView&& other) noexcept = default;

  ~CTStringView() = default;

  constexpr const tchar& operator[](usize index) const { return str_[index]; }

  TString GenerateSubString(usize offset = 0,
                            usize count = kInvalidIndex) const;

  bool IsContained(tchar c) const {
    return comet::IsContained(str_, length_, c);
  }

  usize GetIndex(tchar c) const { return comet::GetIndex(str_, length_, c); }

  usize GetLastIndexOf(tchar c, usize offset) const noexcept {
    return comet::GetLastIndexOf(GetCTStr(), length_, c, offset);
  }

  usize GetNthToLastIndexOf(tchar c, usize count, usize offset) const noexcept {
    return comet::GetNthToLastIndexOf(GetCTStr(), length_, c, count, offset);
  }

  constexpr usize GetLength() const noexcept { return length_; }

  constexpr usize GetLengthWithNullTerminator() const noexcept {
    return length_ + 1;
  }

  constexpr bool IsEmpty() const noexcept { return length_ == 0; }

  constexpr const tchar* GetCTStr() const noexcept { return str_; }

  const tchar& GetFirst() const noexcept { return str_[0]; }

  const tchar& GetLast() const noexcept { return str_[length_ - 1]; }

  constexpr operator const tchar*() const noexcept { return GetCTStr(); }

 private:
  const tchar* str_{nullptr};
  usize length_{0};
};

std::ostream& operator<<(std::ostream& stream, const TString& str);
std::ostream& operator<<(std::ostream& stream, const CTStringView& str);
bool operator==(const TString& str1, const TString& str2);
bool operator==(const TString& str1, const CTStringView& str2);
bool operator==(const CTStringView& str1, const TString& str2);
bool operator==(const CTStringView& str1, const CTStringView& str2);
bool operator==(const TString& str1, const tchar* str2);
bool operator==(const tchar* str1, const TString& str2);
bool operator==(const CTStringView& str1, const tchar* str2);
bool operator==(const tchar* str1, const CTStringView& str2);
bool operator==(const TString& str, tchar c);
bool operator==(tchar c, const TString& str);
bool operator==(const CTStringView& str, tchar c);
bool operator==(tchar c, const CTStringView& str);
bool operator!=(const TString& str1, const TString& str2);
bool operator!=(const TString& str1, const CTStringView& str2);
bool operator!=(const CTStringView& str1, const TString& str2);
bool operator!=(const CTStringView& str1, const CTStringView& str2);
bool operator!=(const TString& str1, const tchar* str2);
bool operator!=(const tchar* str1, const TString& str2);
bool operator!=(const CTStringView& str1, const tchar* str2);
bool operator!=(const tchar* str1, const CTStringView& str2);
bool operator!=(const TString& str, tchar c);
bool operator!=(tchar c, const TString& str);
bool operator!=(const CTStringView& str, tchar c);
bool operator!=(tchar c, const CTStringView& str);
bool operator<(const TString& str1, const TString& str2);
bool operator<(const TString& str1, const CTStringView& str2);
bool operator<(const CTStringView& str1, const TString& str2);
bool operator<(const CTStringView& str1, const CTStringView& str2);
bool operator<(const TString& str1, const tchar* str2);
bool operator<(const tchar* str1, const TString& str2);
bool operator<(const CTStringView& str1, const tchar* str2);
bool operator<(const tchar* str1, const CTStringView& str2);
bool operator<(const TString& str, tchar c);
bool operator<(tchar c, const TString& str);
bool operator<(const CTStringView& str, tchar c);
bool operator<(tchar c, const CTStringView& str);
bool operator<=(const TString& str1, const TString& str2);
bool operator<=(const TString& str1, const CTStringView& str2);
bool operator<=(const CTStringView& str1, const TString& str2);
bool operator<=(const CTStringView& str1, const CTStringView& str2);
bool operator<=(const TString& str1, const tchar* str2);
bool operator<=(const tchar* str1, const TString& str2);
bool operator<=(const CTStringView& str1, const tchar* str2);
bool operator<=(const tchar* str1, const CTStringView& str2);
bool operator<=(const TString& str, tchar c);
bool operator<=(tchar c, const TString& str);
bool operator<=(const CTStringView& str, tchar c);
bool operator<=(tchar c, const CTStringView& str);
bool operator>(const TString& str1, const TString& str2);
bool operator>(const TString& str1, const CTStringView& str2);
bool operator>(const CTStringView& str1, const TString& str2);
bool operator>(const CTStringView& str1, const CTStringView& str2);
bool operator>(const TString& str1, const tchar* str2);
bool operator>(const tchar* str1, const TString& str2);
bool operator>(const CTStringView& str1, const tchar* str2);
bool operator>(const tchar* str1, const CTStringView& str2);
bool operator>(const TString& str, tchar c);
bool operator>(tchar c, const TString& str);
bool operator>(const CTStringView& str, tchar c);
bool operator>(tchar c, const CTStringView& str);
bool operator>=(const TString& str1, const TString& str2);
bool operator>=(const TString& str1, const CTStringView& str2);
bool operator>=(const CTStringView& str1, const TString& str2);
bool operator>=(const CTStringView& str1, const CTStringView& str2);
bool operator>=(const TString& str1, const tchar* str2);
bool operator>=(const tchar* str1, const TString& str2);
bool operator>=(const CTStringView& str1, const tchar* str2);
bool operator>=(const tchar* str1, const CTStringView& str2);
bool operator>=(const TString& str, tchar c);
bool operator>=(tchar c, const TString& str);
bool operator>=(const CTStringView& str, tchar c);
bool operator>=(tchar c, const CTStringView& str);
TString operator+(const TString& str1, const TString& str2);
TString operator+(const TString& str1, const CTStringView& str2);
TString operator+(const CTStringView& str1, const TString& str2);
TString operator+(const CTStringView& str1, const CTStringView& str2);
TString operator+(const TString& str1, const tchar* str2);
TString operator+(const tchar* str1, const TString& str2);
TString operator+(const CTStringView& str1, const tchar* str2);
TString operator+(const tchar* str1, const CTStringView& str2);
TString operator+(const TString& str, tchar c);
TString operator+(tchar c, const TString& str);
TString& operator+=(TString& str1, const TString& str2);
TString& operator+=(TString& str1, const CTStringView& str2);
TString& operator+=(TString& str1, const tchar* str2);
TString& operator+=(TString& str, tchar c);
TString operator/(const TString& str1, const TString& str2);
TString operator/(const TString& str1, const CTStringView& str2);
TString operator/(const CTStringView& str1, const TString& str2);
TString operator/(const CTStringView& str1, const CTStringView& str2);
TString operator/(const TString& str1, const tchar* str2);
TString operator/(const tchar* str1, const TString& str2);
TString operator/(const CTStringView& str1, const tchar* str2);
TString operator/(const tchar* str1, const CTStringView& str2);
TString operator/(const TString& str, tchar c);
TString operator/(tchar c, const TString& str);
TString& operator/=(TString& str1, const TString& str2);
TString& operator/=(TString& str1, const CTStringView& str2);
TString& operator/=(TString& str1, const tchar* str2);
TString& operator/=(TString& str, tchar c);

HashValue GenerateHash(const TString& value);
HashValue GenerateHash(const CTStringView& value);

#define COMET_CTSTRING_VIEW(str) (CTStringView{COMET_TCHAR(str)})
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_TSTRING_H_
