/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/time_profiler.h"

#include "base/check_op.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "bat/ads/internal/logging.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ads {

namespace {

TimeProfiler* g_time_profiler = nullptr;

constexpr char kScope[] = "::";
constexpr char kOpenParenthisis[] = "(";

size_t FindStartOfClassName(const std::string& pretty_function) {
  DCHECK(!pretty_function.empty());

  size_t end_pos = pretty_function.find(kOpenParenthisis);
  if (end_pos == std::string::npos) {
    end_pos = pretty_function.length();
  }

  size_t start_pos = pretty_function.substr(0, end_pos).rfind(" ") + 1;
  if (start_pos == std::string::npos) {
    start_pos = 0;
  }

  return start_pos;
}

size_t FindEndOfClassName(const std::string& pretty_function) {
  DCHECK(!pretty_function.empty());

  const size_t pos = pretty_function.find(kOpenParenthisis);
  if (pos == std::string::npos) {
    return pretty_function.length();
  }

  return pretty_function.substr(0, pos).rfind(kScope);
}

size_t FindEndOfFunctionName(const std::string& pretty_function) {
  DCHECK(!pretty_function.empty());

  size_t pos = pretty_function.find(kOpenParenthisis);
  if (pos == std::string::npos) {
    pos = pretty_function.length();
  }

  return pos;
}

std::string GetFunctionNameFromId(const std::string& id) {
  DCHECK(!id.empty());

  size_t pos = FindEndOfClassName(id);
  if (pos == id.length()) {
    return "";
  }
  pos += strlen(kScope);

  const size_t length = FindEndOfFunctionName(id) - pos;

  return id.substr(pos, length);
}

std::string GetClassNameFromId(const std::string& id) {
  DCHECK(!id.empty());

  const size_t pos = FindStartOfClassName(id);
  const size_t length = FindEndOfClassName(id) - pos;

  return id.substr(pos, length);
}

std::string BuildObjectName(const std::string& name,
                            const TimeTicksInfo& time_ticks) {
  DCHECK(!name.empty());

  std::string object_name = name;

  if (!time_ticks.name.empty()) {
    object_name += kScope;
    object_name += time_ticks.name;
  }

  return object_name;
}

std::string GetDuration(const base::TimeTicks& time_ticks) {
  const base::TimeDelta duration = base::TimeTicks::Now() - time_ticks;
  return base::StringPrintf(
      "%s ms", base::NumberToString(duration.InMillisecondsF()).c_str());
}

std::string BuildDurationLogMessage(const std::string& name,
                                    const int line,
                                    const std::string& message,
                                    const TimeTicksInfo& time_ticks) {
  DCHECK(!name.empty());

  const std::string object_name = BuildObjectName(name, time_ticks);

  const std::string duration = GetDuration(time_ticks.last_time_ticks);

  std::string log_message = "TimeProfiler.Duration [";
  log_message += object_name;
  log_message += ".";
  log_message += base::NumberToString(line);
  log_message += "]";
  if (!message.empty()) {
    log_message += " ";
    log_message += message;
  }
  log_message += ": ";
  log_message += duration;

  return log_message;
}

}  // namespace

TimeProfiler::TimeProfiler() {
  DCHECK_EQ(g_time_profiler, nullptr);
  g_time_profiler = this;
}

TimeProfiler::~TimeProfiler() {
  DCHECK(g_time_profiler);
  g_time_profiler = nullptr;
}

// static
TimeProfiler* TimeProfiler::Get() {
  DCHECK(g_time_profiler);
  return g_time_profiler;
}

// static
bool TimeProfiler::HasInstance() {
  return g_time_profiler;
}

void TimeProfiler::Begin(const std::string& id) {
  DCHECK(!id.empty()) << "Id must be specified";

  const base::TimeTicks now = base::TimeTicks::Now();

  const std::string name = GetClassNameFromId(id);

  DCHECK(!DoesExist(name)) << "Begin() already called for " << name;

  TimeTicksInfo time_ticks;
  time_ticks.name = GetFunctionNameFromId(id);
  time_ticks.start_time_ticks = now;
  time_ticks.last_time_ticks = now;
  set_time_ticks_for_name(name, time_ticks);

  BLOG(6, "TimeProfiler.Begin [" << name << "]");
}

void TimeProfiler::Reset(const std::string& id) {
  DCHECK(!id.empty()) << "Id must be specified";

  const base::TimeTicks now = base::TimeTicks::Now();

  const std::string name = GetClassNameFromId(id);

  DCHECK(!DoesExist(name)) << "You must call Begin() before Measure()";

  TimeTicksInfo time_ticks;
  time_ticks.name = GetFunctionNameFromId(id);
  time_ticks.start_time_ticks = now;
  time_ticks.last_time_ticks = now;
  set_time_ticks_for_name(name, time_ticks);

  BLOG(6, "TimeProfiler.Reset [" << name << "]");
}

void TimeProfiler::Measure(const std::string& id,
                           const int line,
                           const std::string& message) {
  DCHECK(!id.empty()) << "Id must be specified";

  const base::TimeTicks now = base::TimeTicks::Now();

  const std::string name = GetClassNameFromId(id);

  const absl::optional<TimeTicksInfo> time_ticks_optional =
      GetTimeTicksForName(name);
  DCHECK(time_ticks_optional) << "You must call Begin() before Measure()";
  TimeTicksInfo time_ticks = time_ticks_optional.value();

  const std::string log_message =
      BuildDurationLogMessage(name, line, message, time_ticks);
  BLOG(6, log_message);

  time_ticks.last_time_ticks = now;
  set_time_ticks_for_name(name, time_ticks);
}

void TimeProfiler::End(const std::string& id) {
  DCHECK(!id.empty()) << "Id must be specified";

  const std::string name = GetClassNameFromId(id);

  const absl::optional<TimeTicksInfo> time_ticks_optional =
      GetTimeTicksForName(name);
  DCHECK(time_ticks_optional) << "You must call Begin() before End()";
  const TimeTicksInfo time_ticks = time_ticks_optional.value();

  const std::string duration = GetDuration(time_ticks.start_time_ticks);
  BLOG(6, "TimeProfiler.End [" << name << "]: " << duration);

  time_ticks_.erase(name);
}

///////////////////////////////////////////////////////////////////////////////

bool TimeProfiler::DoesExist(const std::string& name) const {
  DCHECK(!name.empty());

  const absl::optional<TimeTicksInfo> time_ticks_optional =
      GetTimeTicksForName(name);

  if (!time_ticks_optional) {
    return false;
  }

  return true;
}

absl::optional<TimeTicksInfo> TimeProfiler::GetTimeTicksForName(
    const std::string& name) const {
  DCHECK(!name.empty());

  if (time_ticks_.find(name) == time_ticks_.end()) {
    return absl::nullopt;
  }

  return time_ticks_.at(name);
}

}  // namespace ads
