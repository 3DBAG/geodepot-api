// Copyright (c) 2024 Balázs Dukai (3DGI), Ravi Peters (3DGI)
//
// This file is part of geodepot (https://github.com/3DGI/geodepot)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author(s):
// Balázs Dukai

#include <curl/curl.h>
#include <geodepot/geodepot.h>

#include <cstdio>
#include <cstring>
#include <ranges>
#include <sstream>
#include <utility>
#include <vector>

// https://stackoverflow.com/questions/2552416/how-can-i-find-the-users-home-dir-in-a-cross-platform-manner-using-c

namespace geodepot {
  bool is_url(std::string_view path) {
    return path.starts_with("http://") || path.starts_with("https://") ||
           path.starts_with("ssh://") || path.starts_with("sftp://") ||
           path.starts_with("ftp://");
  }

  // But consider using https://docs.libcpr.org/advanced-usage.html (Download To
  // File) instead of vanilla curl. todo: check for path's existing, validity
  // etc, so that it doesn't segfault
  bool download(std::string url, std::string dest) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char errbuf[CURL_ERROR_SIZE];
    if (dest.length() > FILENAME_MAX) {
      // todo: mabye error here
    }
    curl = curl_easy_init();
    if (curl) {
      fp = fopen(dest.c_str(), "wb");
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
      errbuf[0] = 0;
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      fclose(fp);
      if (res != CURLE_OK) {
        size_t len = strlen(errbuf);
        fprintf(stderr, "\nlibcurl: (%d) ", res);
        if (len)
          fprintf(stderr, "%s%s", errbuf,
                  ((errbuf[len - 1] != '\n') ? "\n" : ""));
        else
          fprintf(stderr, "%s\n", curl_easy_strerror(res));
      } else {
        return true;
      }
    }
    return false;
  }

  CaseSpec CaseSpec::from_string(std::string casespec_string) {
    std::string line;
    std::vector<std::string> vec;
    std::stringstream ss(casespec_string);
    while (std::getline(ss, line, '/')) {
      vec.push_back(line);
    }
    if (vec.size() == 1) {
      return CaseSpec{.case_name = vec.at(0)};
    }
    if (vec.size() == 2) {
      return CaseSpec{.case_name = vec.at(0), .data_name = vec.at(1)};
    }
    // todo: throw INVALID_CASESPEC
    throw;
  }
  std::filesystem::path CaseSpec::to_path() const {
    return this->data_name.empty()
               ? std::filesystem::path(this->case_name)
               : std::filesystem::path(this->case_name) / this->data_name;
  }
  /**
   *  @brief Connect to an existing local repository or download one from the
   * remote.
   */
  Repository::Repository(const std::string_view path) {
    if (is_url(path)) {
      // Create local dir structure to populate with the files from remote
      auto path_local_repo = absolute(std::filesystem::path(".geodepot"));
      auto path_local_cases = path_local_repo / "cases";
      create_directories(path_local_cases);
      auto path_remote_index =
          std::filesystem::path(path).append("index.geojson");
      auto path_remote_config = std::filesystem::path(path).append("config.json");
      auto path_local_index = path_local_repo / "index.geojson";
      auto path_local_config = path_local_repo / "config.json";
      auto result_index =
          download(path_remote_index.string(), path_local_index.string());
      auto result_config =
          download(path_remote_config.string(), path_local_config.string());
      // todo: report error
      remote_url_ = path;  // todo: use local config instead
      path_ = path_local_repo;
      path_cases_ = path_local_cases;
      path_index_ = path_local_index;
      path_config_local_ = path_local_config;
    } else {
      auto path_absolute = absolute(std::filesystem::path(path));
      if (auto s = status(path_absolute); !exists(s)) {
        // todo: report error or throw exception
      }
      path_ = path_absolute;
      path_cases_ = path_absolute / "cases";
      if (!exists(path_cases_)) {
        // todo: throw MISSING_CASES
      }
      path_index_ = path_absolute / "index.geojson";
      if (!exists(path_index_)) {
        // todo: throw MISSING_INDEX
      }
      path_config_local_ = path_absolute / "config.json";
      if (!exists(path_config_local_)) {
        // todo: throw MISSING_LOCAL_CONFIG
      }
    }
  }

  // todo: use option return value to indicate that the data is not in the repo
  std::optional<std::filesystem::path> Repository::get(
      std::string casespec) const {
    auto cs = CaseSpec::from_string(std::move(casespec));
    auto path_local_data = this->path_cases_ / cs.to_path();
    if (exists(path_local_data)) return path_local_data;
    // Try downloading
    auto path_remote_data =
        std::filesystem::path(this->remote_url_) / "cases" / cs.to_path();
    auto res = download(path_remote_data, path_local_data);
    // todo: check for curl results for not-found and handle it here
    if (res) {
      if (exists(path_local_data)) {
        return path_local_data;
      }
    }
    return std::nullopt;
  }
  std::filesystem::path Repository::get_repository_path() const {
    return this->path_;
  }
}  // namespace geodepot
