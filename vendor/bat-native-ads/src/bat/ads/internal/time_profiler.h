/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TIME_PROFILER_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TIME_PROFILER_H_

#include <map>
#include <string>

#include "base/time/time.h"

namespace absl {
template <typename T>
class optional;
}  // namespace absl

namespace ads {

// Example usage:
//
//   TIME_PROFILER_BEGIN();
//   TIME_PROFILER_MEASURE_WITH_MESSAGE("SomeMessage");
//   TIME_PROFILER_MEASURE();
//   TIME_PROFILER_END();
//
// or
//
//   TIME_PROFILER_BEGIN();
//   TIME_PROFILER_END();
//
// This measures and logs the elapsed time ticks between each
// |TIME_PROFILER_MEASURE*| call and the total elapsed time ticks after calling
// |TIME_PROFILER_END| in milliseconds. You must call |TIME_PROFILER_BEGIN|
// before calling |TIME_PROFILER_MEASURE*| or |TIME_PROFILER_END|. Logs are
// logged at verbose level 6 or higher.
//
// Call |TIME_PROFILER_RESET| to reset time profiling for the given id

struct TimeTicksInfo {
  std::string name;
  base::TimeTicks start_time_ticks;
  base::TimeTicks last_time_ticks;
};

class TimeProfiler final {
 public:
  TimeProfiler();
  ~TimeProfiler();

  TimeProfiler(const TimeProfiler&) = delete;
  TimeProfiler& operator=(const TimeProfiler&) = delete;

  static TimeProfiler* Get();

  static bool HasInstance();

  // Begin time profiling and log for the given id. Must be called before any
  // calls to |Reset|, |Measure| or |End|
  void Begin(const std::string& id);

  // Reset time profiling and log for the given id
  void Reset(const std::string& id);

  // Measure time profiling and log for the given id, line number and optional
  // message since the last measurement
  void Measure(const std::string& id,
               const int line,
               const std::string& message = "");

  // End time profiling and log for the given id
  void End(const std::string& id);

 private:
  std::map<std::string, TimeTicksInfo> time_ticks_;

  void set_time_ticks_for_name(const std::string& name,
                               const TimeTicksInfo& time_ticks) {
    time_ticks_[name] = time_ticks;
  }

  bool DoesExist(const std::string& name) const;

  absl::optional<TimeTicksInfo> GetTimeTicksForName(
      const std::string& name) const;
};

#define TIME_PROFILER_BEGIN() TimeProfiler::Get()->Begin(__PRETTY_FUNCTION__);
#define TIME_PROFILER_RESET() TimeProfiler::Get()->Reset(__PRETTY_FUNCTION__);
#define TIME_PROFILER_MEASURE_WITH_MESSAGE(message) \
  TimeProfiler::Get()->Measure(__PRETTY_FUNCTION__, __LINE__, message);
#define TIME_PROFILER_MEASURE() \
  TimeProfiler::Get()->Measure(__PRETTY_FUNCTION__, __LINE__);
#define TIME_PROFILER_END() TimeProfiler::Get()->End(__PRETTY_FUNCTION__);

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TIME_PROFILER_H_
