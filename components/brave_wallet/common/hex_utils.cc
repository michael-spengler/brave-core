/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/common/hex_utils.h"

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

namespace brave_wallet {

std::string ToHex(const std::string& data) {
  if (data.empty()) {
    return "0x0";
  }
  return "0x" + base::ToLowerASCII(base::HexEncode(data.data(), data.size()));
}

std::string ToHex(const std::vector<uint8_t>& data) {
  if (data.empty())
    return "0x0";
  return "0x" + base::ToLowerASCII(base::HexEncode(data));
}

// Determines if the passed in hex string is valid
bool IsValidHexString(const std::string& hex_input) {
  if (hex_input.length() < 3) {
    return false;
  }
  if (!base::StartsWith(hex_input, "0x")) {
    return false;
  }
  for (const auto& c : hex_input.substr(2)) {
    if (!base::IsHexDigit(c)) {
      return false;
    }
  }
  return true;
}

// Pads a hex encoded parameter to 32-bytes
// i.e. 64 hex characters.
bool PadHexEncodedParameter(const std::string& hex_input, std::string* out) {
  if (!out) {
    return false;
  }
  if (!IsValidHexString(hex_input)) {
    return false;
  }
  if (hex_input.length() >= 64 + 2) {
    *out = hex_input;
    return true;
  }
  std::string hex_substr = hex_input.substr(2);
  size_t padding_len = 64 - hex_substr.length();
  std::string padding(padding_len, '0');

  *out = "0x" + padding + hex_substr;
  return true;
}

// Takes 2 inputs prefixed by 0x and combines them into an output with a single
// 0x. For example 0x1 and 0x2 would return 0x12
bool ConcatHexStrings(const std::string& hex_input1,
                      const std::string& hex_input2,
                      std::string* out) {
  if (!out) {
    return false;
  }
  if (!IsValidHexString(hex_input1) || !IsValidHexString(hex_input2)) {
    return false;
  }
  *out = hex_input1 + hex_input2.substr(2);
  return true;
}

bool ConcatHexStrings(const std::vector<std::string>& hex_inputs,
                      std::string* out) {
  if (!out) {
    return false;
  }
  if (hex_inputs.empty()) {
    return false;
  }
  if (!IsValidHexString(hex_inputs[0])) {
    return false;
  }

  *out = hex_inputs[0];
  for (size_t i = 1; i < hex_inputs.size(); i++) {
    if (!IsValidHexString(hex_inputs[i])) {
      return false;
    }
    *out += hex_inputs[i].substr(2);
  }

  return true;
}

bool HexValueToUint256(const std::string& hex_input, uint256_t* out) {
  if (!out) {
    return false;
  }
  if (!IsValidHexString(hex_input)) {
    return false;
  }
  *out = 0;
  for (char c : hex_input.substr(2)) {
    (*out) <<= 4;
    (*out) += static_cast<uint256_t>(base::HexDigitToInt(c));
  }
  return true;
}

std::string Uint256ValueToHex(uint256_t input) {
  std::string result;
  result.reserve(32);

  static constexpr char kHexChars[] = "0123456789abcdef";
  while (input) {
    uint8_t i = static_cast<uint8_t>(input & static_cast<uint256_t>(0x0F));
    result.insert(result.begin(), kHexChars[i]);
    input >>= 4;
  }
  if (result.empty()) {
    return "0x0";
  }
  return "0x" + result;
}

bool StringToUint256(const std::string& source, uint256_t* out) {
  DLOG(INFO) << "input:" << source;
  for (const auto c : source) {
    (*out) *= 10;
    char digit = static_cast<char>(c - '0');
    (*out) += static_cast<uint256_t>(digit);
  }
  return true;
}

unsigned int bits(uint256_t value) {
  int WIDTH = 256 / 8;
  uint8_t* pn = (uint8_t*)&value;
  for (int pos = WIDTH - 1; pos >= 0; pos--) {
    if (pn[pos]) {
      for (int nbits = 31; nbits > 0; nbits--) {
        if (pn[pos] & 1U << nbits)
          return 32 * pos + nbits + 1;
      }
      return 32 * pos + 1;
    }
  }
  return 0;
}

// a/b = result
uint256_t divide(uint256_t a, uint256_t b, uint256_t* remainder) {
  uint256_t div = b;  // make a copy, so we can shift.
  uint256_t num = a;  // make a copy, so we can subtract.
  int num_bits = bits(a);
  int div_bits = bits(b);
  DLOG(INFO) << "div_bits:" << div_bits << " num_bits:" << num_bits;
  if (div_bits == 0)
    return 0;               // division by zero
  if (div_bits > num_bits)  // the result is certainly 0.
    return 0;
  *remainder = 0;
  int shift = num_bits - div_bits;
  DLOG(INFO) << "div_bits:" << div_bits << " num_bits:" << num_bits
             << " shift:" << shift;
  uint256_t result = 0;
  uint32_t* pn = (uint32_t*)&result;

  div <<= shift;  // shift so that div and num align.
  while (shift >= 0) {
    if (num >= div) {
      num -= div;
      pn[shift / 32] |= (1 << (shift & 31));  // set a bit of the result.
    }
    div >>= 1;  // shift back.
    shift--;
  }
  // num now contains the remainder of the division.
  *remainder = num;
  return result;
}

std::string Uint256ToString(uint256_t a) {
  uint256_t x = a;
  std::string s;
  while (x > 0) {
      uint256_t remainder = 0;
      x = brave_wallet::divide(x, 10, &remainder);
      s.push_back(remainder + '0');
      DLOG(INFO) << "s:" << s;
  }
  reverse(s.begin(), s.end());
  return s;
}

}  // namespace brave_wallet
