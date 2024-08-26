#include <geodepot/geodepot.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("sandbox") {
  std::string_view path{
      "https://data.3dgi.xyz/geodepot-test-data/mock_project/.geodepot"};
  auto path_local_repo = absolute(std::filesystem::path(".geodepot"));
  auto path_local_cases = path_local_repo;
  path_local_cases.append("cases");
  create_directories(path_local_cases);
  auto path_remote_index = std::filesystem::path(path).append("index.geojson");
  auto path_remote_config = std::filesystem::path(path).append("config.json");
  auto path_local_index = path_local_repo;
  path_local_index.append("index.geojson");
  auto path_local_config = path_local_repo;
  path_local_config.append("config.json");
  auto result_index =
      geodepot::download(path_remote_index.string(), path_local_index.string());
  auto result_config = geodepot::download(path_remote_config.string(),
                                          path_local_config.string());
  REQUIRE(result_index == result_config);
}
