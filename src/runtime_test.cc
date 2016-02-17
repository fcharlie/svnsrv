//
//
//
//
#include <iostream>
#include "Runtime.cc"

int main() {
  std::string dir;
  if (GetMainExecuteFileDirectory(dir)) {
    std::cout << "GetMainExecuteFileDirectory: " << dir << std::endl;
  }
  return 0;
}
