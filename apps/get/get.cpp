#include <geodepot/geodepot.h>
#include <iostream>
#include <string>
int main(int argc, char** argv) {
  geodepot::PrepareOptions options;
  if (argc < 2 || std::string(argv[1]) != "prepare") { std::cerr << "Usage: " << argv[0] << " prepare --lock FILE --suite NAME --cache DIR [--offline]\n"; return 2; }
  for (int i=2;i<argc;++i) { std::string a=argv[i]; if(a=="--offline") options.offline=true; else if((a=="--lock"||a=="--suite"||a=="--cache") && i+1<argc) { std::string v=argv[++i]; if(a=="--lock") options.lock_file=v; else if(a=="--suite") options.suite=v; else options.cache_root=v; } else { std::cerr << "invalid argument\n"; return 2; } }
  if(options.lock_file.empty()||options.cache_root.empty()||options.suite.empty()) { std::cerr << "--lock, --suite and --cache are required\n"; return 2; }
  try { std::cout << geodepot::prepare(options).string() << '\n'; } catch(const geodepot::Error& e) { std::cerr << "geodepot-get: " << e.what() << '\n'; return static_cast<int>(e.code()); } catch(const std::exception& e) { std::cerr << "geodepot-get: " << e.what() << '\n'; return 7; }
}
