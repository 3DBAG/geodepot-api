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

TEST_CASE("geodepot") {
  // auto res = geodepot::get();
  auto repo = geodepot::Repository::init(
      "https://data.3dgi.xyz/geodepot-test-data/mock_project/.geodepot");
  auto p = repo.get("wippolder/wippolder.gpkg");
  if (p.has_value()) {
    std::cout << p.value() << '\n';
  }
}

TEST_CASE("casespec_from_string", "[casespec]") {
  {
    auto cs = geodepot::CaseSpec::from_string("wippolder/wippolder.gpkg");
    REQUIRE(cs.case_name == "wippolder");
    REQUIRE(cs.data_name == "wippolder.gpkg");
  }
  {
    auto cs = geodepot::CaseSpec::from_string("wippolder");
    REQUIRE(cs.case_name == "wippolder");
    REQUIRE(cs.data_name.empty());
  }
}

TEST_CASE("casespec_to_path", "[casespec]") {
  {
    auto cs_p =
        geodepot::CaseSpec::from_string("wippolder/wippolder.gpkg").to_path();
    REQUIRE(cs_p == std::filesystem::path("wippolder/wippolder.gpkg"));
  }
  {
    auto cs_p = geodepot::CaseSpec::from_string("wippolder").to_path();
    REQUIRE(cs_p == std::filesystem::path("wippolder"));
  }
}
