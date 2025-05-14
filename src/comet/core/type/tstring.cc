// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "tstring.h"

#include "comet/core/c_string.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/generator.h"
#include "comet/core/hash.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/math/math_common.h"

namespace comet {
namespace internal {
thread_local memory::Allocator* tls_tstring_allocator{nullptr};

memory::Allocator* GetTStringAllocator() {
  return tls_tstring_allocator != nullptr ? tls_tstring_allocator
                                          : &TStringAllocator::Get();
}

TStringAllocator& TStringAllocator::Get() {
  static TStringAllocator singleton{};
  return singleton;
}

void* TStringAllocator::AllocateAligned(usize size, memory::Alignment align) {
  // TODO(m4jr0): Implement specific TString allocator.
  return memory::AllocateAligned(size, align, memory::kEngineMemoryTagTString);
}

void TStringAllocator::Deallocate(void* p) {
  // TODO(m4jr0): Implement specific TString allocator.
  memory::Deallocate(p);
}
}  // namespace internal

void InitializeTStrings() { internal::TStringAllocator::Get().Initialize(); }

void DestroyTStrings() { internal::TStringAllocator::Get().Destroy(); }

void AttachTStringAllocator(memory::Allocator* handle) {
  COMET_ASSERT(handle != nullptr, "TString allocator provided is null!");
  internal::tls_tstring_allocator = handle;
}

void DetachTStringAllocator() {
  COMET_ASSERT(internal::tls_tstring_allocator != nullptr,
               "No TString allocator handle has been provided!");
  internal::tls_tstring_allocator = nullptr;
}

#ifdef COMET_WIDE_TCHAR
TString::TString(std::string_view str) {
  length_ = comet::GetLength(str.data());
  Allocate(length_);
  Copy(GetTStr(), str.data(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
}
#else
TString::TString(std::wstring_view str) {
  length_ = comet::GetLength(str.data());
  Allocate(length_);
  Copy(GetTStr(), str.data(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
}
#endif  // COMET_WIDE_TCHAR

TString::TString(const TString& other)
    : length_{other.length_}
#ifdef COMET_DEBUG
      ,
      is_alloc_allowed_{other.is_alloc_allowed_}
#endif  // COMET_DEBUG
{
  Allocate(other.capacity_);
  Copy(GetTStr(), other.GetCTStr(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
}

TString::TString(const TString& other, usize pos, usize length)
    : TString{other.GenerateSubString(pos, length)} {}

TString::TString(TString&& other) noexcept { Swap(*this, other); }

TString& TString::operator=(const TString& other) {
  if (other.length_ > capacity_) {
    Deallocate();
    Allocate(other.length_);
  }

  length_ = other.length_;
  Copy(GetTStr(), other.GetCTStr(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
  return *this;
}

TString& TString::operator=(TString&& other) noexcept {
  Swap(*this, other);
  return *this;
}

TString& TString::operator=(const CTStringView& other) {
  const auto length{other.GetLength()};

  if (length > capacity_) {
    Deallocate();
    Allocate(length);
  }

  length_ = length;
  Copy(GetTStr(), other.GetCTStr(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
  return *this;
}

TString::~TString() { Destroy(); }

void TString::Destroy() { Deallocate(); }

TString& TString::operator=(const tchar* other) {
  return operator=(CTStringView{other});
}

void TString::Reserve(usize capacity) {
  if (capacity <= capacity_) {
    return;
  }

  Allocate(capacity);
}

void TString::Resize(usize length) {
  length_ = length;

  if (length_ > capacity_) {
    Allocate(length_);
  }

  GetTStr()[length_] = COMET_TCHAR('\0');
}

void TString::Clear() {
  if (length_ == 0) {
    return;
  }

  length_ = 0;
  GetTStr()[0] = COMET_TCHAR('\0');
}

TString& TString::Append(const TString& str) {
  return Append(str.GetCTStr(), str.GetLength());
}

TString& TString::Append(const TString& str, usize offset, usize length) {
  auto effective_length{str.GetLength()};

  if (length != kInvalidIndex) {
    effective_length -= length;
  }

  return Append(str.GetCTStr() + offset, length);
}

TString TString::GenerateSubString(usize offset, usize count) const {
  TString new_str{};
  auto length{count == kInvalidIndex ? length_ - offset : count};
  new_str.Reserve(length);
  new_str.length_ = length;
  GetSubString(new_str.GetTStr(), GetCTStr(), length_, offset, count);
  return new_str;
}

bool TString::IsContained(tchar c) const {
  return comet::IsContained(GetCTStr(), length_, c);
}

bool TString::IsContained(const tchar* str) const {
  if (str == nullptr) {
    return false;
  }

  return IsContained(str, comet::GetLength(str));
}

bool TString::IsContained(const tchar* str, usize len) const {
  if (str == nullptr || len == 0 || len > length_) {
    return false;
  }

  // TODO(m4jr0): Implement version that supports length as a parameter.
  return comet::IsContained(GetCTStr(), str);
}

bool TString::IsContained(const TString& str) const {
  return IsContained(str.GetCTStr(), str.GetLength());
}

bool TString::IsContainedInsensitive(tchar c) const {
  return comet::IsContainedInsensitive(GetCTStr(), &c);
}

bool TString::IsContainedInsensitive(const tchar* str) const {
  if (str == nullptr) {
    return false;
  }

  return IsContainedInsensitive(str, comet::GetLength(str));
}

bool TString::IsContainedInsensitive(const tchar* str, usize len) const {
  if (str == nullptr || len == 0 || len > length_) {
    return false;
  }

  // TODO(m4jr0): Implement version that supports length as a parameter.
  return comet::IsContainedInsensitive(GetCTStr(), str);
}

bool TString::IsContainedInsensitive(const TString& str) const {
  return IsContainedInsensitive(str.GetCTStr(), str.GetLength());
}

usize TString::GetIndex(tchar c) const {
  return comet::GetIndex(GetCTStr(), length_, c);
}

usize TString::GetLastIndexOf(tchar c, usize offset) const noexcept {
  return comet::GetLastIndexOf(GetCTStr(), length_, c, offset);
}

usize TString::GetNthToLastIndexOf(tchar c, usize count,
                                   usize offset) const noexcept {
  return comet::GetNthToLastIndexOf(GetCTStr(), length_, c, count, offset);
}

void Swap(TString& str1, TString& str2) {
#ifdef COMET_DEBUG
  std::swap(str1.is_alloc_allowed_, str2.is_alloc_allowed_);
#endif  // COMET_DEBUG
  std::swap(str1.length_, str2.length_);
  std::swap(str1.capacity_, str2.capacity_);
  std::swap(str1.str_, str2.str_);
  std::swap(str1.sso_, str2.sso_);
}

tchar& TString::operator[](usize index) {
  COMET_ASSERT(index <= length_, "Index is out of bounds: ", index, " > ",
               length_, "!");
  return GetTStr()[index];
}

const tchar& TString::operator[](usize index) const {
  COMET_ASSERT(index <= length_, "Index is out of bounds: ", index, " > ",
               length_, "!");
  return GetCTStr()[index];
}

const tchar* TString::GetCTStr() const noexcept {
  if (capacity_ <= kSSOCapacityThreshold) {
    return sso_;
  }

  return str_;
}

tchar* TString::GetTStr() noexcept {
  if (capacity_ <= kSSOCapacityThreshold) {
    return sso_;
  }

  return str_;
}

usize TString::GetLength() const noexcept { return length_; }

usize TString::GetLengthWithNullTerminator() const noexcept {
  return length_ + 1;
}

usize TString::GetCapacity() const noexcept { return capacity_; }

bool TString::IsEmpty() const noexcept { return length_ == 0; }

const tchar& TString::GetFirst() const noexcept {
  COMET_ASSERT(length_ > 0, "Length is 0!");
  return GetCTStr()[0];
}

const tchar& TString::GetLast() const noexcept {
  COMET_ASSERT(length_ > 0, "Length is 0!");
  return GetCTStr()[length_ - 1];
}

TString::operator const tchar*() const noexcept { return GetCTStr(); }

#ifdef COMET_DEBUG
void TString::AllowAlloc() noexcept { is_alloc_allowed_ = true; }

void TString::DisallowAlloc() noexcept { is_alloc_allowed_ = false; }
#endif  // COMET_DEBUG

void TString::Allocate(usize capacity) {
  if (capacity <= capacity_ || capacity <= kSSOCapacityThreshold) {
    return;
  }

#ifdef COMET_DEBUG
  COMET_ASSERT(is_alloc_allowed_, "Allocation is not allowed on this TString!");
#endif  // COMET_DEBUG
  auto* old{GetTStr()};
  auto* tstring_allocator{internal::GetTStringAllocator()};

  // Add + 1 for the null terminator.
  str_ = reinterpret_cast<tchar*>(
      tstring_allocator->AllocateMany<tchar>(capacity + 1));
  capacity_ = capacity;

  if (old != nullptr) {
    Copy(str_, old, length_);
    str_[length_] = COMET_TCHAR('\0');

    if (old != sso_) {
      tstring_allocator->Deallocate(old);
    }
  }
}

void TString::Deallocate() {
  if (str_ != nullptr) {
    internal::GetTStringAllocator()->Deallocate(str_);
    str_ = nullptr;
  }

  capacity_ = length_ = 0;
}

TString CTStringView::GenerateSubString(usize offset, usize count) const {
  TString new_str{};
  auto length{count == kInvalidIndex ? length_ - offset : count - offset};
  new_str.Resize(length);
  GetSubString(new_str.GetTStr(), str_, length_, offset, length);
  return new_str;
}

std::ostream& operator<<(std::ostream& stream, const TString& str) {
#ifdef COMET_WIDE_TCHAR
  return stream << GenerateForOneFrame<schar>(str.GetCTStr(), str.GetLength());
#else
  return stream << str.GetCTStr();
#endif  // COMET_WIDE_TCHAR
}

std::ostream& operator<<(std::ostream& stream, const CTStringView& str) {
#ifdef COMET_WIDE_TCHAR
  return stream << GenerateForOneFrame<schar>(str.GetCTStr(), str.GetLength());
#else
  return stream << str.GetCTStr();
#endif  // COMET_WIDE_TCHAR
}

bool operator==(const TString& str1, const TString& str2) {
  return AreStringsEqual(str1.GetCTStr(), str1.GetLength(), str2.GetCTStr(),
                         str2.GetLength());
}

bool operator==(const TString& str1, const CTStringView& str2) {
  return AreStringsEqual(str1.GetCTStr(), str1.GetLength(), str2,
                         str2.GetLength());
}

bool operator==(const CTStringView& str1, const TString& str2) {
  return AreStringsEqual(str1.GetCTStr(), str1.GetLength(), str2.GetCTStr(),
                         str2.GetLength());
}

bool operator==(const CTStringView& str1, const CTStringView& str2) {
  return AreStringsEqual(str1.GetCTStr(), str1.GetLength(), str2.GetCTStr(),
                         str2.GetLength());
}

bool operator==(const TString& str1, const tchar* str2) {
  return operator==(str1, CTStringView{str2});
}

bool operator==(const tchar* str1, const TString& str2) {
  return operator==(CTStringView{str1}, str2);
}

bool operator==(const CTStringView& str1, const tchar* str2) {
  return operator==(str1, CTStringView{str2});
}

bool operator==(const tchar* str1, const CTStringView& str2) {
  return operator==(CTStringView{str1}, str2);
}

bool operator==(const TString& str, tchar c) { return operator==(str, &c); }

bool operator==(tchar c, const TString& str) { return operator==(&c, str); }

bool operator==(const CTStringView& str, tchar c) {
  tchar tmp[2]{'\0'};
  tmp[0] = c;
  return operator==(str, tmp);
}

bool operator==(tchar c, const CTStringView& str) {
  tchar tmp[2]{'\0'};
  tmp[0] = c;
  return operator==(tmp, str);
}

bool operator!=(const TString& str1, const TString& str2) {
  return !operator==(str1, str2);
}

bool operator!=(const TString& str1, const CTStringView& str2) {
  return !operator==(str1, str2);
}

bool operator!=(const CTStringView& str1, const TString& str2) {
  return !operator==(str1, str2);
}

bool operator!=(const CTStringView& str1, const CTStringView& str2) {
  return !operator==(str1, str2);
}

bool operator!=(const TString& str1, const tchar* str2) {
  return operator!=(str1, CTStringView{str2});
}

bool operator!=(const tchar* str1, const TString& str2) {
  return operator!=(CTStringView{str1}, str2);
}

bool operator!=(const CTStringView& str1, const tchar* str2) {
  return operator!=(str1, CTStringView{str2});
}

bool operator!=(const tchar* str1, const CTStringView& str2) {
  return operator!=(CTStringView{str1}, str2);
}

bool operator!=(const TString& str, tchar c) { return operator!=(str, &c); }

bool operator!=(tchar c, const TString& str) { return operator!=(&c, str); }

bool operator!=(const CTStringView& str, tchar c) {
  return operator!=(str, &c);
}

bool operator!=(tchar c, const CTStringView& str) {
  return operator!=(&c, str);
}

bool operator<(const TString& str1, const TString& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) < 0;
}

bool operator<(const TString& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2,
                 math::Min(str1.GetLength(), str2.GetLength())) < 0;
}

bool operator<(const CTStringView& str1, const TString& str2) {
  return Compare(str1, str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) < 0;
}

bool operator<(const CTStringView& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) < 0;
}

bool operator<(const TString& str1, const tchar* str2) {
  return operator<(str1, CTStringView{str2});
}

bool operator<(const tchar* str1, const TString& str2) {
  return operator<(CTStringView{str1}, str2);
}

bool operator<(const CTStringView& str1, const tchar* str2) {
  return operator<(str1, CTStringView{str2});
}

bool operator<(const tchar* str1, const CTStringView& str2) {
  return operator<(CTStringView{str1}, str2);
}

bool operator<(const TString& str, tchar c) { return operator<(str, &c); }

bool operator<(tchar c, const TString& str) { return operator<(&c, str); }

bool operator<(const CTStringView& str, tchar c) { return operator<(str, &c); }

bool operator<(tchar c, const CTStringView& str) { return operator<(&c, str); }

bool operator<=(const TString& str1, const TString& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) <= 0;
}

bool operator<=(const TString& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2,
                 math::Min(str1.GetLength(), str2.GetLength())) <= 0;
}

bool operator<=(const CTStringView& str1, const TString& str2) {
  return Compare(str1, str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) <= 0;
}

bool operator<=(const CTStringView& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) <= 0;
}

bool operator<=(const TString& str1, const tchar* str2) {
  return operator<=(str1, CTStringView{str2});
}

bool operator<=(const tchar* str1, const TString& str2) {
  return operator<=(CTStringView{str1}, str2);
}

bool operator<=(const CTStringView& str1, const tchar* str2) {
  return operator<=(str1, CTStringView{str2});
}

bool operator<=(const tchar* str1, const CTStringView& str2) {
  return operator<=(CTStringView{str1}, str2);
}

bool operator<=(const TString& str, tchar c) { return operator<=(str, &c); }

bool operator<=(tchar c, const TString& str) { return operator<=(&c, str); }

bool operator<=(const CTStringView& str, tchar c) {
  return operator<=(str, &c);
}

bool operator<=(tchar c, const CTStringView& str) {
  return operator<=(&c, str);
}

bool operator>(const TString& str1, const TString& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) > 0;
}

bool operator>(const TString& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2,
                 math::Min(str1.GetLength(), str2.GetLength())) > 0;
}

bool operator>(const CTStringView& str1, const TString& str2) {
  return Compare(str1, str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) > 0;
}

bool operator>(const CTStringView& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) > 0;
}

bool operator>(const TString& str1, const tchar* str2) {
  return operator>(str1, CTStringView{str2});
}

bool operator>(const tchar* str1, const TString& str2) {
  return operator>(CTStringView{str1}, str2);
}

bool operator>(const CTStringView& str1, const tchar* str2) {
  return operator>(str1, CTStringView{str2});
}

bool operator>(const tchar* str1, const CTStringView& str2) {
  return operator>(CTStringView{str1}, str2);
}

bool operator>(const TString& str, tchar c) { return operator>(str, &c); }

bool operator>(tchar c, const TString& str) { return operator>(&c, str); }

bool operator>(const CTStringView& str, tchar c) { return operator>(str, &c); }

bool operator>(tchar c, const CTStringView& str) { return operator>(&c, str); }

bool operator>=(const TString& str1, const TString& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) >= 0;
}

bool operator>=(const TString& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2,
                 math::Min(str1.GetLength(), str2.GetLength())) >= 0;
}

bool operator>=(const CTStringView& str1, const TString& str2) {
  return Compare(str1, str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) >= 0;
}

bool operator>=(const CTStringView& str1, const CTStringView& str2) {
  return Compare(str1.GetCTStr(), str2.GetCTStr(),
                 math::Min(str1.GetLength(), str2.GetLength())) >= 0;
}

bool operator>=(const TString& str1, const tchar* str2) {
  return operator>=(str1, CTStringView{str2});
}

bool operator>=(const tchar* str1, const TString& str2) {
  return operator>=(CTStringView{str1}, str2);
}

bool operator>=(const CTStringView& str1, const tchar* str2) {
  return operator>=(str1, CTStringView{str2});
}

bool operator>=(const tchar* str1, const CTStringView& str2) {
  return operator>=(CTStringView{str1}, str2);
}

bool operator>=(const TString& str, tchar c) { return operator>=(str, &c); }

bool operator>=(tchar c, const TString& str) { return operator>=(&c, str); }

bool operator>=(const CTStringView& str, tchar c) {
  return operator>=(str, &c);
}

bool operator>=(tchar c, const CTStringView& str) {
  return operator>=(&c, str);
}

TString operator+(const TString& str1, const TString& str2) {
  TString str{};
  str.Reserve(str1.GetLength() + str2.GetLength());
  str.Append(str1);
  str.Append(str2);
  return str;
}

TString operator+(const TString& str1, const CTStringView& str2) {
  TString str{};
  str.Reserve(str1.GetLength() + str2.GetLength());
  str.Append(str1);
  str.Append(str2.GetCTStr(), str2.GetLength());
  return str;
}

TString operator+(const CTStringView& str1, const TString& str2) {
  TString str{};
  str.Reserve(str1.GetLength() + str2.GetLength());
  str.Append(str1.GetCTStr(), str1.GetLength());
  str.Append(str2);
  return str;
}

TString operator+(const CTStringView& str1, const CTStringView& str2) {
  TString str{};
  str.Reserve(str1.GetLength() + str2.GetLength());
  str.Append(str1.GetCTStr());
  str.Append(str2.GetCTStr());
  return str;
}

TString operator+(const TString& str1, const tchar* str2) {
  return operator+(str1, CTStringView{str2});
}

TString operator+(const tchar* str1, const TString& str2) {
  return operator+(CTStringView{str1}, str2);
}

TString operator+(const CTStringView& str1, const tchar* str2) {
  return operator+(str1, CTStringView{str2});
}

TString operator+(const tchar* str1, const CTStringView& str2) {
  return operator+(CTStringView{str1}, str2);
}

TString operator+(const TString& str, tchar c) { return operator+(str, &c); }

TString operator+(tchar c, const TString& str) { return operator+(&c, str); }

TString& operator+=(TString& str1, const TString& str2) {
  str1.Append(str2, str2.GetLength());
  return str1;
}

TString& operator+=(TString& str1, const CTStringView& str2) {
  str1.Append(str2.GetCTStr(), str2.GetLength());
  return str1;
}

TString& operator+=(TString& str1, const tchar* str2) {
  return operator+=(str1, CTStringView{str2});
}

TString& operator+=(TString& str, tchar c) {
  str.Append(&c, 1);
  return str;
}

TString operator/(const TString& str1, const TString& str2) {
  return Append(str1, str2);
}

TString operator/(const TString& str1, const CTStringView& str2) {
  return Append(str1, str2);
}

TString operator/(const CTStringView& str1, const TString& str2) {
  return Append(str1, str2);
}

TString operator/(const CTStringView& str1, const CTStringView& str2) {
  return Append(str1, str2);
}

TString operator/(const TString& str1, const tchar* str2) {
  return operator/(str1, CTStringView{str2});
}

TString operator/(const tchar* str1, const TString& str2) {
  return operator/(CTStringView{str1}, str2);
}

TString operator/(const CTStringView& str1, const tchar* str2) {
  return operator/(str1, CTStringView{str2});
}

TString operator/(const tchar* str1, const CTStringView& str2) {
  return operator/(CTStringView{str1}, str2);
}

TString operator/(const TString& str, tchar c) { return operator/(str, &c); }

TString operator/(tchar c, const TString& str) { return operator/(&c, str); }

TString& operator/=(TString& str1, const TString& str2) {
  // Worst case: adding 1 character to an extra slash.
  str1.Reserve(str1.GetLength() + str2.GetLength() + 1);
  usize new_len;
  AppendTo(str2, str1.GetTStr(), str1.GetCapacity() + 1, &new_len);
  str1.Resize(new_len);
  return str1;
}

TString& operator/=(TString& str1, const CTStringView& str2) {
  // Worst case: adding 1 character to an extra slash.
  str1.Reserve(str1.GetLength() + str2.GetLength() + 1);
  usize new_len;
  AppendTo(str2, str1.GetTStr(), str1.GetCapacity() + 1, &new_len);
  str1.Resize(new_len);
  return str1;
}

TString& operator/=(TString& str1, const tchar* str2) {
  return operator/=(str1, CTStringView{str2});
}

TString& operator/=(TString& str, tchar c) { return operator/=(str, &c); }

HashValue GenerateHash(const TString& value) {
  return GenerateHash(value.GetCTStr());
}

HashValue GenerateHash(const CTStringView& value) {
  return GenerateHash(value.GetCTStr());
}
}  // namespace comet
