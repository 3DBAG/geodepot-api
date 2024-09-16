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

#pragma once
#include <filesystem>
#include <optional>
#include <string_view>

#if defined(_WIN32)
#  if defined(EXPORTING_MYMATH)
#    define DECLSPEC __declspec(dllexport)
#  else
#    define DECLSPEC __declspec(dllimport)
#  endif
#else // non windows
#  define DECLSPEC
#endif

namespace geodepot {

  struct DECLSPEC CaseSpec {
    std::string case_name;
    std::string data_name;

    static CaseSpec from_string(std::string casespec_string);
    [[nodiscard]] std::filesystem::path to_path() const;
  };

  class DECLSPEC Repository {
   public:
    explicit Repository(std::string_view path);
    Repository() = default;

    [[nodiscard]] std::optional<std::filesystem::path> get(
        std::string casespec) const;
    [[nodiscard]] std::filesystem::path get_repository_path() const;

   private:
    std::string remote_url_;  // todo: use local config instead
    std::filesystem::path path_;
    std::filesystem::path path_cases_;
    std::filesystem::path path_index_;
    std::filesystem::path path_config_local_;

    bool is_valid() const;
  };

  bool DECLSPEC is_url(std::string_view path);

  bool DECLSPEC download(std::string url, std::string dest);

}