/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_FEDERATED_LEARNING_DATA_STORES_TEST_DATA_STORE_H_
#define BRAVE_COMPONENTS_BRAVE_FEDERATED_LEARNING_DATA_STORES_TEST_DATA_STORE_H_

#include <map>
#include <string>

#include "base/files/file_path.h"
#include "base/time/time.h"
#include "brave/components/brave_federated_learning/data_stores/data_store.h"

namespace brave {

namespace federated_learning {

// Log Definition --------------------------------------------------------
struct TestTaskLog {
  TestTaskLog(const int id, const bool label, const base::Time& creation_date);
  TestTaskLog(const TestTaskLog& other);
  TestTaskLog();
  ~TestTaskLog();

  int id;
  bool label;
  const base::Time creation_date;
};

class TestDataStore final : public DataStore {
 public:
  explicit TestDataStore(const base::FilePath& database_path);
  ~TestDataStore() final;

  TestDataStore(const TestDataStore&) = delete;
  TestDataStore& operator=(const TestDataStore&) = delete;

  typedef std::map<int, TestTaskLog> IdToTestTaskLogMap;

  bool Init(const std::string& task_id,
            const std::string& task_name,
            const int max_number_of_records,
            const int max_retention_days);
  void EnforceRetentionPolicy();

  using DataStore::DeleteLogs;

  bool AddLog(const TestTaskLog& log);
  void LoadLogs(IdToTestTaskLogMap* test_logs);
  bool EnsureTable() override;
};

}  // namespace federated_learning

}  // namespace brave

#endif  // BRAVE_COMPONENTS_BRAVE_FEDERATED_LEARNING_DATA_STORES_TEST_DATA_STORE_H_
