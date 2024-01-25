#include <iostream>
#include <fstream>
#include <string>

int main(int args, char** argv) {
   std::ifstream input;
   if (args > 1) {
      input.open(argv[1]);
   }
}
