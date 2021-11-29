/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/string_util.h"

#include <iostream>

#include "base/check.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/re2/src/re2/re2.h"

namespace ads {

namespace {

std::string StripTextWithPattern(const std::string& text,
                                const std::string& pattern) {
  DCHECK(!pattern.empty());

  if (text.empty()) {
    return "";
  }

  std::string stripped_text = text;

  RE2::GlobalReplace(&stripped_text, pattern, " ");

  std::u16string stripped_text_as_string16 = base::UTF8ToUTF16(stripped_text);

  stripped_text_as_string16 =
      base::CollapseWhitespace(stripped_text_as_string16, true);

  return base::UTF16ToUTF8(stripped_text_as_string16);
}

}  // namespace

std::string StripNonAlphaCharacters(const std::string& text) {
  const std::string escaped_characters =
      RE2::QuoteMeta("!\"#$%&'()*+,-./:<=>?@\\[]^_`{|}~");

  const std::string pattern = base::StringPrintf(
      "[[:cntrl:]]|"
      "\\\\(t|n|v|f|r)|[\\t\\n\\v\\f\\r]|\\\\x[[:xdigit:]][[:xdigit:]]|"
      "[%s]|\\S*\\d+\\S*",
      escaped_characters.c_str());

  return StripTextWithPattern(text, pattern);
}

std::string StripNonAlphaNumericCharacters(const std::string& text) {
  const std::string escaped_characters =
      RE2::QuoteMeta("!\"#$%&'()*+,-./:<=>?@\\[]^_`{|}~");

  const std::string pattern = base::StringPrintf(
      "[[:cntrl:]]|"
      "\\\\(t|n|v|f|r)|[\\t\\n\\v\\f\\r]|\\\\x[[:xdigit:]][[:xdigit:]]|"
      "[%s]",
      escaped_characters.c_str());

  return StripTextWithPattern(text, pattern);
}

std::string ExtractTextWithPattern(const std::string& text,
                                   const std::string& pattern) {
  re2::StringPiece text_as_string_piece(text);

  RE2 re(pattern);
  std::string extracted_text;
  RE2::FindAndConsume(&text_as_string_piece, re, &extracted_text);

  std::cout << "FOOBAR.1: " << text << std::endl;
  std::cout << "FOOBAR.2: " << pattern << std::endl;
  std::cout << "FOOBAR.3: " << extracted_text << std::endl;

  return extracted_text;
}

}  // namespace ads
