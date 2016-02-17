//
//
//
//
#include <iostream>
#include "Runtime.cc"

int main() {
  std::string dir;
  if (GetProcessImageFileFolder(dir)) {
    std::cout << "GetProcessImageFileFolder: " << dir << std::endl;
  }
  return 0;
}
