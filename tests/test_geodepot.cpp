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

#include <geodepot/geodepot.h>

#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("casespec can be constructed from a string", "[casespec]") {
  SECTION("case name and data name") {
    auto cs = geodepot::CaseSpec::from_string("wippolder/wippolder.gpkg");
    REQUIRE((cs.case_name == "wippolder"));
    REQUIRE((cs.data_name == "wippolder.gpkg"));
  }
  SECTION("data name only") {
    auto cs = geodepot::CaseSpec::from_string("wippolder");
    REQUIRE((cs.case_name == "wippolder"));
    REQUIRE(cs.data_name.empty());
  }
}

TEST_CASE("repository can be initialized", "[init]") {
  SECTION("init from url") {
    auto repo = geodepot::Repository(
        "https://data.3dgi.xyz/geodepot-test-data/mock_project/.geodepot");
    std::clog << repo.get_repository_path() << "\n";
    REQUIRE(std::filesystem::exists(repo.get_repository_path()));
  }

  SECTION("init from local path") {
    auto repo = geodepot::Repository(
        "/home/balazs/Development/geodepot-api/tests/data/mock_project");
    std::clog << repo.get_repository_path() << "\n";
    REQUIRE(std::filesystem::exists(repo.get_repository_path()));
  }

  std::filesystem::remove_all(".geodepot");
}

TEST_CASE("data paths can be get from the repository", "[get]") {
  SECTION("init first from url then load the from local and get data") {
    auto tmp_dir = std::filesystem::temp_directory_path();
    std::filesystem::current_path(tmp_dir);
    auto repo = geodepot::Repository(
        "https://data.3dgi.xyz/geodepot-test-data/mock_project/.geodepot");
    repo = geodepot::Repository(tmp_dir.string());
    std::clog << repo.get_repository_path() << "\n";
    auto p = repo.get("wippolder/wippolder.gpkg");
    REQUIRE(p.has_value());
    REQUIRE(std::filesystem::exists(p.value()));
    std::filesystem::remove_all(tmp_dir / ".geodepot");
  }

  SECTION("init from remote and get data") {
    auto repo = geodepot::Repository(
        "https://data.3dgi.xyz/geodepot-test-data/mock_project/.geodepot");
    auto p = repo.get("wippolder/wippolder.gpkg");
    REQUIRE(p.has_value());
    REQUIRE(std::filesystem::exists(p.value()));
  }

  std::filesystem::remove_all(".geodepot");
}