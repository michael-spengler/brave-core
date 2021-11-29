/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/fil_response_parser.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/rpc_response_parser.h"
#include "brave/components/brave_wallet/common/brave_wallet_types.h"
#include "brave/components/brave_wallet/common/hex_utils.h"

namespace {

std::string ConvertAttoFilToFil(const std::string& atto, int decimal) {
  brave_wallet::uint256_t attofil = 0;
  brave_wallet::StringToUint256(atto, &attofil);
  if (attofil > 0) {
  }
  // For simplicity we take just first decimals now.
  return atto.substr(0, decimal);
}

}

namespace brave_wallet {

bool ParseFilGetBalance(const std::string& json, std::string* hex_balance) {
  auto result = brave_wallet::ParseSingleStringResult(json, hex_balance);
  if (!result || hex_balance->empty())
    return result;

  *hex_balance = ConvertAttoFilToFil(*hex_balance, 3);
  return result;
}

}  // namespace brave_wallet
