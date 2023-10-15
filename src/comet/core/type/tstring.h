// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_TSTRING_H_
#define COMET_COMET_CORE_TYPE_TSTRING_H_

#include "comet_precompile.h"

#include "comet/core/c_string.h"

namespace comet {
const auto kSSOCapacityThreshold{15};

class CTStringView;

class TString {
 public:
  TString() = default;

  template <typename TChar>
  TString(const TChar* str, uindex length) : length_{length} {
    Allocate(length_);
    Copy(GetTStr(), str, length_);
    GetTStr()[length_] = COMET_TCHAR('\0');
  }

  template <typename TChar>
  explicit TString(TChar c) : TString{&c, 1} {}

  template <typename TChar>
  explicit TString(const TChar* str) : TString{str, comet::GetLength(str)} {}

  template <typename TChar>
  TString(uindex length, TChar c) : length_{length} {
    Allocate(length_);
    Copy(GetTStr(), &c, length_);
    GetTStr()[length_] = COMET_TCHAR('\0');
  }

  TString(const TString& other);
  TString(const TString& other, uindex pos, uindex length = kInvalidIndex);

  template <typename TChar>
  TString(const TChar* start, const TChar* end)
      : length_{static_cast<uindex>(end - start)} {
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

  void Reserve(uindex capacity);
  void Resize(uindex length);
  void Clear();

  template <typename TChar>
  TString& Append(uindex length, TChar c) {
    auto new_length{length + length_};

    // Use >= to take the null terminator into account.
    if (new_length >= capacity_) {
      Reserve(new_length);
    }

    auto offset{length_ - 1};

    for (uindex i{0}; i < length; ++i) {
      GetTStr()[offset++] = c;
    }

    length_ = new_length;
    GetTStr()[length_] = COMET_TCHAR('\0');
    return *this;
  }

  TString& Append(const TString& str);
  TString& Append(const TString& str, uindex offset,
                  uindex length = kInvalidIndex);

  template <typename TChar>
  TString& Append(const TChar* str, uindex length) {
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

  TString GenerateSubString(uindex offset = 0,
                            uindex count = kInvalidIndex) const;
  uindex GetLastIndexOf(tchar c, uindex offset = kInvalidIndex) const noexcept;
  uindex GetNthToLastIndexOf(tchar c, uindex count = 0,
                             uindex offset = kInvalidIndex) const noexcept;

  friend void Swap(TString& str1, TString& str2);
  tchar& operator[](uindex index);
  const tchar& operator[](uindex index) const;
  stringid::StringId GenerateStringId() const;
  const tchar* GetCTStr() const noexcept;
  tchar* GetTStr() noexcept;
  uindex GetLength() const noexcept;
  uindex GetLengthWithNullTerminator() const noexcept;
  uindex GetCapacity() const noexcept;
  bool IsEmpty() const noexcept;
  const tchar& GetFirst() const noexcept;
  const tchar& GetLast() const noexcept;
  operator const tchar*() const noexcept;

#ifdef COMET_DEBUG
  void AllowAlloc() noexcept;
  void DisallowAlloc() noexcept;
#endif  // COMET_DEBUG

 private:
  void Allocate(uindex capacity);
  void Deallocate();

#ifdef COMET_DEBUG
  bool is_alloc_allowed_{true};
#endif  // COMET_DEBUG

  uindex length_{0};
  uindex capacity_{kSSOCapacityThreshold};
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

  constexpr CTStringView(const tchar* str, uindex length) noexcept
      : str_{str}, length_{length} {}

  constexpr CTStringView(const CTStringView& other) noexcept = default;
  constexpr CTStringView(CTStringView&& other) noexcept = default;

  constexpr CTStringView& operator=(const CTStringView& other) = default;
  constexpr CTStringView& operator=(CTStringView&& other) noexcept = default;

  ~CTStringView() = default;

  constexpr const tchar& operator[](uindex index) const { return str_[index]; }

  TString GenerateSubString(uindex offset = 0,
                            uindex count = kInvalidIndex) const;
  stringid::StringId GenerateStringId() const;

  constexpr uindex GetLength() const noexcept { return length_; }

  constexpr uindex GetLengthWithNullTerminator() const noexcept {
    return length_ + 1;
  }

  constexpr bool IsEmpty() const noexcept { return length_ == 0; }

  constexpr const tchar* GetCTStr() const noexcept { return str_; }

  const tchar& GetFirst() const noexcept { return str_[0]; }

  const tchar& GetLast() const noexcept { return str_[length_ - 1]; }

  constexpr operator const tchar*() const noexcept { return GetCTStr(); }

 private:
  uindex length_{0};
  const tchar* str_{nullptr};
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

#define COMET_CTSTRING_VIEW(str) (CTStringView{COMET_TCHAR(str)})
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_TSTRING_H_
