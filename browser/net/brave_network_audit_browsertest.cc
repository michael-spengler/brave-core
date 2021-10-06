// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/base_paths.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/path_service.h"
#include "base/strings/strcat.h"
#include "brave/browser/net/brave_network_audit_whitelists.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test.h"
#include "services/network/public/cpp/network_switches.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "third_party/re2/src/re2/re2.h"

namespace brave {
namespace {

// Based on the implementation of isPrivateIP() from NPM's "ip" module.
// See https://github.com/indutny/node-ip/blob/master/lib/ip.js
constexpr const char* kPrivateIPRegexps[] = {
    "(::f{4}:)?10\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})",
    "(::f{4}:)?192\\.168\\.([0-9]{1,3})\\.([0-9]{1,3})",
    "(::f{4}:)?172\\.(1[6-9]|2\\d|30|31)\\.([0-9]{1,3})\\.([0-9]{1,3})",
    "(::f{4}:)?127\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})",
    "(::f{4}:)?169\\.254\\.([0-9]{1,3})\\.([0-9]{1,3})",
    "f[cd][0-9a-f]{2}:.*",
    "fe80:.*",
    "::1",
    "::"};

bool isPrivateURL(const GURL& url) {
  for (const char* regexp : kPrivateIPRegexps) {
    if (RE2::FullMatch(url.host(), regexp)) {
      return true;
    }
  }
  return false;
}

bool PerformNetworkAuditProcess(base::ListValue* events) {
  DCHECK(events);
  bool failed = false;
  events->EraseListValueIf([&failed](base::Value& event_value) {
    base::DictionaryValue* event_dict;
    EXPECT_TRUE(event_value.GetAsDictionary(&event_dict));

    absl::optional<int> event_type = event_dict->FindIntPath("type");
    EXPECT_TRUE(event_type.has_value());

    // Showing these helps determine which URL requests which don't
    // actually hit the network.
    if (static_cast<net::NetLogEventType>(event_type.value()) ==
        net::NetLogEventType::URL_REQUEST_FAKE_RESPONSE_HEADERS_CREATED) {
      return false;
    }

    const base::Value* source_dict = event_dict->FindDictPath("source");
    EXPECT_TRUE(source_dict);

    // Consider URL requests only.
    absl::optional<int> source_type = source_dict->FindIntPath("type");
    EXPECT_TRUE(source_type.has_value());

    if (static_cast<net::NetLogSourceType>(source_type.value()) !=
        net::NetLogSourceType::URL_REQUEST) {
      return true;
    }

    // Discard events without URLs in the parameters.
    if (!event_dict->FindKey("params"))
      return true;

    const base::Value* params_dict2 = event_dict->FindDictPath("params");
    EXPECT_TRUE(params_dict2);

    if (!params_dict2->FindKey("url"))
      return true;

    const std::string* url_str = params_dict2->FindStringPath("url");
    EXPECT_TRUE(url_str);

    GURL url(*url_str);
    EXPECT_TRUE(url.is_valid());

    if (RE2::FullMatch(url.host(), "[a-z]+")) {
      // Chromium sometimes sends requests to random non-resolvable hosts.
      return true;
    }

    for (const char* protocol : kWhitelistedUrlProtocols) {
      if (protocol == url.scheme()) {
        return true;
      }
    }

    bool found_prefix = false;
    for (const char* prefix : kWhitelistedUrlPrefixes) {
      if (!url.spec().rfind(prefix, 0)) {
        found_prefix = true;
        break;
      }
    }

    bool found_pattern = false;
    for (const char* pattern : kWhitelistedUrlPatterns) {
      if (RE2::FullMatch(url.spec(), pattern)) {
        found_pattern = true;
        break;
      }
    }

    if (!found_prefix && !found_pattern) {
      // Check if the URL is a private IP.
      if (isPrivateURL(url)) {
        // Warn but don't fail the audit.
        LOG(WARNING) << "NETWORK AUDIT WARNING:" << url.spec() << std::endl;
        return false;
      }

      LOG(ERROR) << "NETWORK AUDIT FAIL:" << url.spec() << std::endl;
      failed = true;
    }

    return false;
  });

  return !failed;
}

void WriteNetworkAuditResultsToDisk(const base::DictionaryValue& results_dic,
                                    const base::FilePath& path) {
  std::string results;
  base::JSONWriter::Write(results_dic, &results);

  FILE* fp = base::OpenFile(path, "wb");
  ASSERT_TRUE(fp);
  fwrite(results.data(), results.size(), 1, fp);
  fclose(fp);

  LOG(INFO) << "Network audit results stored in " << path << std::endl;
}

class BraveNetworkAuditTest : public InProcessBrowserTest {
 public:
  BraveNetworkAuditTest() = default;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    base::FilePath source_root_path;
    base::PathService::Get(base::DIR_SOURCE_ROOT, &source_root_path);

    // Use a test-dependent suffix not to collide on the generated files.
    std::string test_name =
        testing::UnitTest::GetInstance()->current_test_info()->name();

    // Full log containing all the network requests.
    net_log_path_ = source_root_path.AppendASCII(
        base::StrCat({test_name, "-network_log.json"}));

    // Log containing the results of the audit only.
    audit_results_path_ = source_root_path.AppendASCII(
        base::StrCat({test_name, "-network_audit_results.json"}));

    command_line->AppendSwitchPath(network::switches::kLogNetLog,
                                   net_log_path_);
    command_line->AppendSwitchASCII(network::switches::kNetLogCaptureMode,
                                    "Everything");
  }

  void TearDownInProcessBrowserTestFixture() override {
    VerifyNetworkAuditLog();
  }

 private:
  // Verify that the netlog file was written, appears to be well formed, and
  // includes the requested level of data.
  void VerifyNetworkAuditLog() {
    // Read the netlog from disk.
    std::string file_contents;
    ASSERT_TRUE(base::ReadFileToString(net_log_path_, &file_contents))
        << "Could not read: " << net_log_path_;

    // Parse it as JSON.
    auto parsed = base::JSONReader::ReadDeprecated(file_contents);
    ASSERT_TRUE(parsed);

    // Ensure the root value is a dictionary.
    base::DictionaryValue* main;
    ASSERT_TRUE(parsed->GetAsDictionary(&main));

    // Ensure it has a "constants" property.
    base::DictionaryValue* constants;
    ASSERT_TRUE(main->GetDictionary("constants", &constants));
    ASSERT_FALSE(constants->DictEmpty());

    // Ensure it has an "events" property.
    base::ListValue* events;
    ASSERT_TRUE(main->GetList("events", &events));
    ASSERT_FALSE(events->GetList().empty());

    EXPECT_TRUE(PerformNetworkAuditProcess(events))
        << "network-audit FAILED. Import " << net_log_path_.AsUTF8Unsafe()
        << " in chrome://net-internals for more details.";

    // Write results of the audit to disk, useful for further debugging.
    WriteNetworkAuditResultsToDisk(*main, audit_results_path_);
    ASSERT_TRUE(base::PathExists(audit_results_path_));
  }

  base::FilePath net_log_path_;
  base::FilePath audit_results_path_;

  DISALLOW_COPY_AND_ASSIGN(BraveNetworkAuditTest);
};

IN_PROC_BROWSER_TEST_F(BraveNetworkAuditTest, SimpleTest) {
  ASSERT_TRUE(embedded_test_server()->Start());
  GURL simple_url(embedded_test_server()->GetURL("/simple.html"));
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), simple_url));
}

IN_PROC_BROWSER_TEST_F(BraveNetworkAuditTest, BraveWelcome) {
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), GURL("brave://welcome")));
}

}  // namespace
}  // namespace brave