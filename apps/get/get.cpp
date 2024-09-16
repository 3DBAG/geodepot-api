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

#include <iostream>
#include <geodepot.h>

int main(int argc, char **argv) {
  std::cout << "get: " << geodepot::get() << '\n';
  return 0;
}