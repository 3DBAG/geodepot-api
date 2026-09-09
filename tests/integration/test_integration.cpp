#include <geodepot/geodepot.h>

#include <iostream>

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " LOCK SUITE CACHE\\n";
    return static_cast<int>(geodepot::ErrorCode::cli);
  }
  try {
    const geodepot::PrepareOptions options{
        .lock_file = argv[1], .cache_root = argv[3], .suite = argv[2]};
    std::cout << geodepot::prepare(options).string() << "\n";
    return 0;
  } catch (const geodepot::Error& error) {
    std::cerr << error.what() << "\n";
    return static_cast<int>(error.code());
  } catch (const std::exception& error) {
    std::cerr << error.what() << "\n";
    return static_cast<int>(geodepot::ErrorCode::cache);
  }
}
