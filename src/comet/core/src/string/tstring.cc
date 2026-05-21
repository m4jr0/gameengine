// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/string/tstring.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/frame/frame_string.h"
#include "comet/core/hash.h"
#include "comet/core/memory/allocator/default_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/math/math_scalar.h"

namespace comet {
namespace internal {
static thread_local memory::Allocator* tls_tstring_allocator{nullptr};

memory::Allocator* GetTStringAllocator() {
  return tls_tstring_allocator != nullptr ? tls_tstring_allocator // >:3 Yeah, use TString here.
                                          : &TStringAllocator::Get();
}

TStringAllocator& TStringAllocator::Get() {
  static TStringAllocator singleton{};
  return singleton;
}

void* TStringAllocator::AllocateAligned(usize size, memory::Alignment align) {
  // TODO(m4jr0): Implement specific TString allocator.
  return memory::GetDefaultAllocator().AllocateAligned(size, align);
}

void TStringAllocator::Deallocate(void* p) {
  // TODO(m4jr0): Implement specific TString allocator.
  memory::GetDefaultAllocator().Deallocate(p);
}

const tchar* CharToStrInPlace(tchar c) noexcept {
  thread_local tchar tmp[2];
  tmp[0] = c;
  tmp[1] = COMET_TCHAR('\0');
  return tmp;
}

static s32 CompareStrings(const tchar* lhs, usize lhs_length, const tchar* rhs,
                          usize rhs_length) {
  const auto min_length{math::Min(lhs_length, rhs_length)};
  const auto cmp{static_cast<s32>(Compare(lhs, rhs, min_length))};

  if (cmp != 0) {
    return cmp;
  }

  if (lhs_length < rhs_length) {
    return -1;
  }

  if (lhs_length > rhs_length) {
    return 1;
  }

  return 0;
}

static s32 CompareStrings(const TString& lhs, const TString& rhs) {
  return CompareStrings(lhs.GetCTStr(), lhs.GetLength(), rhs.GetCTStr(),
                        rhs.GetLength());
}

static s32 CompareStrings(const TString& lhs, const CTStringView& rhs) {
  return CompareStrings(lhs.GetCTStr(), lhs.GetLength(), rhs.GetCTStr(),
                        rhs.GetLength());
}

static s32 CompareStrings(const CTStringView& lhs, const TString& rhs) {
  return CompareStrings(lhs.GetCTStr(), lhs.GetLength(), rhs.GetCTStr(),
                        rhs.GetLength());
}

static s32 CompareStrings(const CTStringView& lhs, const CTStringView& rhs) {
  return CompareStrings(lhs.GetCTStr(), lhs.GetLength(), rhs.GetCTStr(),
                        rhs.GetLength());
}
}  // namespace internal

void AttachTStringAllocator(memory::Allocator* handle) {
  COMET_ASSERT(handle != nullptr, "AttachTStringAllocator",
               "TString allocator is null");
  internal::tls_tstring_allocator = handle;
}

void DetachTStringAllocator() {
  COMET_ASSERT(internal::tls_tstring_allocator != nullptr,
               "DetachTStringAllocator",
               "no TString allocator has been attached");
  internal::tls_tstring_allocator = nullptr;
}

#ifdef COMET_WIDE_TCHAR
TString::TString(std::string_view str) {
  length_ = str.size();
  ReserveStorage(length_);
  Copy(GetTStr(), str.data(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
}
#else
TString::TString(std::wstring_view str) {
  length_ = str.size();
  ReserveStorage(length_);
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
  ReserveStorage(other.capacity_);
  Copy(GetTStr(), other.GetCTStr(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
}

TString::TString(const TString& other, usize pos, usize length)
    : TString{other.GenerateSubString(pos, length)} {}

TString::TString(TString&& other) noexcept {
#ifdef COMET_DEBUG
  is_alloc_allowed_ = other.is_alloc_allowed_;
#endif  // COMET_DEBUG

  length_ = other.length_;
  capacity_ = other.capacity_;

  if (other.str_ != nullptr) {
    str_ = other.str_;
    other.str_ = nullptr;
  } else {
    Copy(sso_, other.sso_, other.length_ + 1);
    str_ = nullptr;
  }

  other.length_ = 0;
  other.capacity_ = kSSOCapacityThreshold;
  other.sso_[0] = COMET_TCHAR('\0');
}

TString& TString::operator=(const TString& other) {
  if (this == &other) {
    return *this;
  }

  if (other.length_ > capacity_) {
    Deallocate();
    ReserveStorage(other.length_);
  }

#ifdef COMET_DEBUG
  is_alloc_allowed_ = other.is_alloc_allowed_;
#endif  // COMET_DEBUG

  length_ = other.length_;
  Copy(GetTStr(), other.GetCTStr(), length_);
  GetTStr()[length_] = COMET_TCHAR('\0');
  return *this;
}

TString& TString::operator=(TString&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Deallocate();

#ifdef COMET_DEBUG
  is_alloc_allowed_ = other.is_alloc_allowed_;
#endif  // COMET_DEBUG

  length_ = other.length_;
  capacity_ = other.capacity_;

  if (other.str_ != nullptr) {
    str_ = other.str_;
    other.str_ = nullptr;
  } else {
    Copy(sso_, other.sso_, other.length_ + 1);
    str_ = nullptr;
  }

  other.length_ = 0;
  other.capacity_ = kSSOCapacityThreshold;
  other.sso_[0] = COMET_TCHAR('\0');

  return *this;
}

TString& TString::operator=(const CTStringView& other) {
  if (other.GetLength() > capacity_) {
    Deallocate();
    ReserveStorage(other.GetLength());
  }

  length_ = other.GetLength();

  if (length_ > 0) {
    Copy(GetTStr(), other.GetCTStr(), length_);
  }

  GetTStr()[length_] = COMET_TCHAR('\0');
  return *this;
}

TString::~TString() { Release(); }

void TString::Release() { Deallocate(); }

TString& TString::operator=(const tchar* other) {
  return operator=(CTStringView{other});
}

void TString::Reserve(usize capacity) {
  if (capacity <= capacity_) {
    return;
  }

  ReserveStorage(capacity);
}

void TString::TrimCapacity() {
  if (length_ <= kSSOCapacityThreshold) {
    if (str_ != nullptr) {
      auto* old{str_};
      Copy(sso_, old, length_);
      sso_[length_] = COMET_TCHAR('\0');
      internal::GetTStringAllocator()->Deallocate(old);
      str_ = nullptr;
    }

    capacity_ = kSSOCapacityThreshold;
    return;
  }

  if (capacity_ == length_) {
    return;
  }

#ifdef COMET_DEBUG
  COMET_ASSERT(is_alloc_allowed_, "TString::TrimCapacity",
               "allocation is not allowed on this TString");
#endif  // COMET_DEBUG

  auto* tstring_allocator{internal::GetTStringAllocator()};
  auto* new_str{reinterpret_cast<tchar*>(
      tstring_allocator->AllocateMany<tchar>(length_ + 1))};

  Copy(new_str, GetCTStr(), length_);
  new_str[length_] = COMET_TCHAR('\0');

  if (str_ != nullptr) {
    tstring_allocator->Deallocate(str_);
  }

  str_ = new_str;
  capacity_ = length_;
}

void TString::Resize(usize length) {
  if (length > capacity_) {
    ReserveStorage(length);
  }

  length_ = length;
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
  COMET_ASSERT(offset <= str.GetLength(), "TString::Append",
               "offset is out of bounds", "offset", offset, "length",
               str.GetLength());

  const auto remaining{str.GetLength() - offset};
  const auto append_length{
      length == kInvalidIndex ? remaining : math::Min(length, remaining)};

  return Append(str.GetCTStr() + offset, append_length);
}

TString TString::GenerateSubString(usize offset, usize count) const {
  COMET_ASSERT(offset <= length_, "TString::GenerateSubString",
               "offset is out of bounds", "offset", offset, "length", length_);

  const auto sub_length{count == kInvalidIndex
                            ? length_ - offset
                            : math::Min(count, length_ - offset)};

  TString new_str{};
  new_str.Reserve(sub_length);
  new_str.length_ = sub_length;
  GetSubString(new_str.GetTStr(), GetCTStr(), length_, offset, sub_length);
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
  return comet::IsContainedInsensitive(GetCTStr(),
                                       internal::CharToStrInPlace(c));
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
  if (&str1 == &str2) {
    return;
  }

  TString tmp{std::move(str1)};
  str1 = std::move(str2);
  str2 = std::move(tmp);
}

tchar& TString::operator[](usize index) {
  COMET_ASSERT(index < length_, "TString::operator[]", "index is out of bounds",
               "index", index, "length", length_);
  return GetTStr()[index];
}

const tchar& TString::operator[](usize index) const {
  COMET_ASSERT(index < length_, "TString::operator[]", "index is out of bounds",
               "index", index, "length", length_);
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
  COMET_ASSERT(length_ > 0, "TString::GetFirst", "string is empty");
  return GetCTStr()[0];
}

const tchar& TString::GetLast() const noexcept {
  COMET_ASSERT(length_ > 0, "TString::GetLast", "string is empty");
  return GetCTStr()[length_ - 1];
}

TString::operator const tchar*() const noexcept { return GetCTStr(); }

#ifdef COMET_DEBUG
void TString::AllowAlloc() noexcept { is_alloc_allowed_ = true; }

void TString::DisallowAlloc() noexcept { is_alloc_allowed_ = false; }
#endif  // COMET_DEBUG

void TString::ReserveStorage(usize capacity) {
  if (capacity <= capacity_ || capacity <= kSSOCapacityThreshold) {
    return;
  }

#ifdef COMET_DEBUG
  COMET_ASSERT(is_alloc_allowed_, "TString::ReserveStorage",
               "allocation is not allowed on this TString");
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

  length_ = 0;
  capacity_ = kSSOCapacityThreshold;
  sso_[0] = COMET_TCHAR('\0');
}

TString CTStringView::GenerateSubString(usize offset, usize count) const {
  COMET_ASSERT(offset <= length_, "CTStringView::GenerateSubString",
               "offset is out of bounds", "offset", offset, "length", length_);

  const auto sub_length{count == kInvalidIndex
                            ? length_ - offset
                            : math::Min(count, length_ - offset)};

  TString new_str{};
  new_str.Resize(sub_length);
  GetSubString(new_str.GetTStr(), str_, length_, offset, sub_length);
  return new_str;
}

std::ostream& operator<<(std::ostream& stream, const TString& str) {
#ifdef COMET_WIDE_TCHAR
  return stream << GenerateFrameString<schar>(str.GetCTStr(), str.GetLength());
#else
  return stream << str.GetCTStr();
#endif  // COMET_WIDE_TCHAR
}

std::ostream& operator<<(std::ostream& stream, const CTStringView& str) {
#ifdef COMET_WIDE_TCHAR
  return stream << GenerateFrameString<schar>(str.GetCTStr(), str.GetLength());
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

bool operator==(const TString& str, tchar c) {
  return operator==(str, internal::CharToStrInPlace(c));
}

bool operator==(tchar c, const TString& str) {
  return operator==(internal::CharToStrInPlace(c), str);
}

bool operator==(const CTStringView& str, tchar c) {
  return operator==(str, internal::CharToStrInPlace(c));
}

bool operator==(tchar c, const CTStringView& str) {
  return operator==(internal::CharToStrInPlace(c), str);
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

bool operator!=(const TString& str, tchar c) {
  return operator!=(str, internal::CharToStrInPlace(c));
}

bool operator!=(tchar c, const TString& str) {
  return operator!=(internal::CharToStrInPlace(c), str);
}

bool operator!=(const CTStringView& str, tchar c) {
  return operator!=(str, internal::CharToStrInPlace(c));
}

bool operator!=(tchar c, const CTStringView& str) {
  return operator!=(internal::CharToStrInPlace(c), str);
}

bool operator<(const TString& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) < 0;
}

bool operator<(const TString& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) < 0;
}

bool operator<(const CTStringView& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) < 0;
}

bool operator<(const CTStringView& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) < 0;
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

bool operator<(const TString& str, tchar c) {
  return operator<(str, internal::CharToStrInPlace(c));
}

bool operator<(tchar c, const TString& str) {
  return operator<(internal::CharToStrInPlace(c), str);
}

bool operator<(const CTStringView& str, tchar c) {
  return operator<(str, internal::CharToStrInPlace(c));
}

bool operator<(tchar c, const CTStringView& str) {
  return operator<(internal::CharToStrInPlace(c), str);
}

bool operator<=(const TString& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) <= 0;
}

bool operator<=(const TString& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) <= 0;
}

bool operator<=(const CTStringView& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) <= 0;
}

bool operator<=(const CTStringView& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) <= 0;
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

bool operator<=(const TString& str, tchar c) {
  return operator<=(str, internal::CharToStrInPlace(c));
}

bool operator<=(tchar c, const TString& str) {
  return operator<=(internal::CharToStrInPlace(c), str);
}

bool operator<=(const CTStringView& str, tchar c) {
  return operator<=(str, internal::CharToStrInPlace(c));
}

bool operator<=(tchar c, const CTStringView& str) {
  return operator<=(internal::CharToStrInPlace(c), str);
}

bool operator>(const TString& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) > 0;
}

bool operator>(const TString& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) > 0;
}

bool operator>(const CTStringView& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) > 0;
}

bool operator>(const CTStringView& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) > 0;
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

bool operator>(const TString& str, tchar c) {
  return operator>(str, internal::CharToStrInPlace(c));
}

bool operator>(tchar c, const TString& str) {
  return operator>(internal::CharToStrInPlace(c), str);
}

bool operator>(const CTStringView& str, tchar c) {
  return operator>(str, internal::CharToStrInPlace(c));
}

bool operator>(tchar c, const CTStringView& str) {
  return operator>(internal::CharToStrInPlace(c), str);
}

bool operator>=(const TString& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) >= 0;
}

bool operator>=(const TString& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) >= 0;
}

bool operator>=(const CTStringView& str1, const TString& str2) {
  return internal::CompareStrings(str1, str2) >= 0;
}

bool operator>=(const CTStringView& str1, const CTStringView& str2) {
  return internal::CompareStrings(str1, str2) >= 0;
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

bool operator>=(const TString& str, tchar c) {
  return operator>=(str, internal::CharToStrInPlace(c));
}

bool operator>=(tchar c, const TString& str) {
  return operator>=(internal::CharToStrInPlace(c), str);
}

bool operator>=(const CTStringView& str, tchar c) {
  return operator>=(str, internal::CharToStrInPlace(c));
}

bool operator>=(tchar c, const CTStringView& str) {
  return operator>=(internal::CharToStrInPlace(c), str);
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
  str.Append(str1.GetCTStr(), str1.GetLength());
  str.Append(str2.GetCTStr(), str2.GetLength());
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

TString operator+(const TString& str, tchar c) {
  return operator+(str, internal::CharToStrInPlace(c));
}

TString operator+(tchar c, const TString& str) {
  return operator+(internal::CharToStrInPlace(c), str);
}

TString& operator+=(TString& str1, const TString& str2) {
  str1.Append(str2);
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

TString operator/(const TString& str, tchar c) {
  return operator/(str, internal::CharToStrInPlace(c));
}

TString operator/(tchar c, const TString& str) {
  return operator/(internal::CharToStrInPlace(c), str);
}

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

TString& operator/=(TString& str, tchar c) {
  return operator/=(str, internal::CharToStrInPlace(c));
}

HashValue GenerateHash(const TString& value) {
  return GenerateHash(value.GetCTStr(), value.GetLength());
}

HashValue GenerateHash(const CTStringView& value) {
  return GenerateHash(value.GetCTStr(), value.GetLength());
}
}  // namespace comet
