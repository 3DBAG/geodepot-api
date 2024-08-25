During configuration:
- make the data available (download)
  - use the compiled exe / compile the exe in configure time?
- set the `GEODEPOT_CASES` CMake variable

Ref.:
- https://discourse.cmake.org/t/best-way-to-have-a-library-package-install-a-main-cpp-and-run-add-executable-in-context-of-user-project-when-it-calls-my-cmake-function/11200
- https://gitlab.eduxiji.net/educg-group-17291-1894922/202310284201919-3046/-/tree/main/cmake (this is it!, locally at /home/balazs/Development/tmp_just_browsing/ExternalAntlr4CppCmake), with more explanations here https://beyondtheloop.dev/Antlr-cpp-cmake/ and even more here https://svlad-90.github.io/DLT-Message-Analyzer/dltmessageanalyzerplugin/src/cmake/README.html and here https://milobanks.com/blog/cmake_antlr and antlr4cpp itself here https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake