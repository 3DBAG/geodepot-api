#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char** argv) {
  if (argc > 1) {
    const fs::path inpath{argv[1]};
    fs::file_status const s = fs::file_status{};
    if (fs::status_known(s) ? fs::exists(s) : fs::exists(inpath))
      return 0;
    return 1;
  }
}