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

#include <archive.h>
#include <archive_entry.h>
#include <curl/curl.h>
#include <geodepot/geodepot.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ranges>
#include <sstream>
#include <utility>
#include <vector>

using json = nlohmann::json;

// https://stackoverflow.com/questions/2552416/how-can-i-find-the-users-home-dir-in-a-cross-platform-manner-using-c

namespace {
  void close_file(std::FILE *fp) { std::fclose(fp); }

  // Ref.: https://github.com/libarchive/libarchive/blob/master/examples/untar.c
  int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;) {
      r = archive_read_data_block(ar, &buff, &size, &offset);
      if (r == ARCHIVE_EOF) return (ARCHIVE_OK);
      if (r != ARCHIVE_OK) return (r);
      r = archive_write_data_block(aw, buff, size, offset);
      if (r != ARCHIVE_OK) {
        // warn("archive_write_data_block()", archive_error_string(aw));
        return (r);
      }
    }
  }

  // Ref.: https://github.com/libarchive/libarchive/blob/master/examples/untar.c
  bool extract(const std::filesystem::path& tarfile, const std::filesystem::path& path_out_dir) {
    auto filename = tarfile.c_str();

    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    ext = archive_write_disk_new();
    /*
     * Note: archive_write_disk_set_standard_lookup() is useful
     * here, but it requires library routines that can add 500k or
     * more to a static executable.
     */
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
    /*
     * On my system, enabling other archive formats adds 20k-30k
     * each.  Enabling gzip decompression adds about 20k.
     * Enabling bzip2 is more expensive because the libbz2 library
     * isn't very well factored.
     */
    archive_read_support_format_tar(a);

    if ((r = archive_read_open_filename(a, filename, 10240))) {
      // todo: printf("archive_read_open_filename()", archive_error_string(a), r);
    }

    for (;;) {
      r = archive_read_next_header(a, &entry);
      if (r == ARCHIVE_EOF) break;
      if (r != ARCHIVE_OK) {
        // todo: fail("archive_read_next_header()", archive_error_string(a), 1);
        return false;
      }

      const char* current_file = archive_entry_pathname(entry);
      const auto path_out_file = path_out_dir / current_file;
      archive_entry_set_pathname(entry, path_out_file.c_str());

      r = archive_write_header(ext, entry);
      if (r != ARCHIVE_OK) {
        // warn("archive_write_header()", archive_error_string(ext));
      } else {
        copy_data(a, ext);
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
          // todo: fail("archive_write_finish_entry()", archive_error_string(ext), 1);
          return false;
        }
      }
    }
    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);
    return true;
  }

}  // namespace

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
      auto path_remote_config =
          std::filesystem::path(path).append("config.json");
      auto path_local_index = path_local_repo / "index.geojson";
      auto path_local_config = path_local_repo / "config.json";
      auto result_index =
          download(path_remote_index.string(), path_local_index.string());
      auto result_config =
          download(path_remote_config.string(), path_local_config.string());
      // todo: report error
      path_ = path_local_repo;
      path_cases_ = path_local_cases;
      path_index_ = path_local_index;
      path_config_local_ = path_local_config;

      std::ifstream f(path_config_local_);
      json config = json::parse(f);
      // todo: error handling
      if (config.contains("remotes")) {
        config["remotes"]["origin"] = json::object({{ "url", path }});
      }
      auto config_str = config.dump();
      {
        std::ofstream out(path_config_local_);
        out.write(config_str.c_str(), config_str.size());
      }

    } else {
      auto path_absolute = absolute(std::filesystem::path(path)) / ".geodepot";
      if (auto s = status(path_absolute); !exists(s)) {
        // Try the parent directory
        if (exists(path_absolute.parent_path() / ".geodepot")) {
          path_absolute = path_absolute.parent_path() / ".geodepot";
        }
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

  /**
   * @brief Check if the repository is valid.
   * @return Boolean to indicate the validity.
   */
  bool Repository::is_valid() const {
    if (!exists(this->path_)) {
      std::cout << "repo path_ doesn't exist" << "\n";
      return false;
    }
    if (!exists(this->path_cases_)) {
      std::cout << "repo path_cases_ doesn't exist " << this->path_cases_ << "\n";
      return false;
    }
    if (!exists(this->path_index_)) {
      std::cout << "repo path_index_ doesn't exist " << this->path_index_ << "\n";
      return false;
    }
    if (!exists(this->path_config_local_)) {
      std::cout << "repo path_config_local_ doesn't exist" << this->path_config_local_ << "\n";
      return false;
    }
    return true;
  }

  // todo: use option return value to indicate that the data is not in the repo
  std::optional<std::filesystem::path> Repository::get(
      std::string casespec) const {
    if (!this->is_valid()) {
      // todo: throw
      std::cout << "invalid repo"
                << "\n";
      return std::nullopt;
    }
    auto casespec_archive = casespec + ".tar";
    auto cs_archive = CaseSpec::from_string(std::move(casespec_archive));
    auto path_local_archive = this->path_cases_ / cs_archive.to_path();
    auto cs = CaseSpec::from_string(std::move(casespec));
    auto path_local_data = this->path_cases_ / cs.to_path();
    auto path_local_case = this->path_cases_ / cs.case_name;

    // Exit early
    if (exists(path_local_data)) return path_local_data;

    if (!exists(path_local_archive)) {
      // Create case dir first, if doesn't exist
      std::filesystem::create_directory(path_local_case);
      // Try downloading
      std::string remote_url{};
      std::ifstream f(this->path_config_local_);
      json config = json::parse(f);
      if (config.contains("remotes")) {
        auto remotes = config["remotes"];
        if (remotes.contains("origin")) {
          remote_url = std::string(remotes["origin"]["url"]);
        }
      }
      if (remote_url.empty()) {
        // todo: throw
        return std::nullopt;
      }
      auto path_remote_archive = std::filesystem::path(remote_url) /
                                 "cases" / cs_archive.to_path();
      auto res = download(path_remote_archive, path_local_archive);
      // todo: check for curl results for not-found and handle it here
      if (!res) {
        if (!exists(path_local_archive)) {
          // todo: throw
        }
      }
    }
    if (exists(path_local_archive)) {
      extract(path_local_archive, path_local_case);
      if (exists(path_local_data)) return path_local_data;
    }

    return std::nullopt;
  }
  std::filesystem::path Repository::get_repository_path() const {
    return this->path_;
  }
}  // namespace geodepot
