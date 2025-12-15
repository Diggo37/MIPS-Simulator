#include <fstream>
#include <iostream>

#include "cpu.h"
#include "parser.h"

using namespace std;

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " program.asm [debug]\n";
    return 1;
  }
  string fname = argv[1];
  bool debug = (argc >= 3 && string(argv[2]) == "debug");
  ifstream ifs(fname);
  if (!ifs) {
    cerr << "Cannot open " << fname << "\n";
    return 1;
  }
  vector<string> lines;
  string line;
  while (getline(ifs, line)) lines.push_back(line);
  Program P;
  try {
    P = parse_asm(lines);
  } catch (exception& e) {
    cerr << "Parse error: " << e.what() << "\n";
    return 1;
  }
  CPU cpu(P, 1024, debug);
  cpu.run();
  cout << "\n=== Final Register File ===\n";
  cpu.print_regs();
  cout << "\n=== Final Memory (first 64 words) ===\n";
  bool nonZero = false;
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    if (arg == "--nonzero") {
      nonZero = true;
    }
  }
  cpu.print_mem_used(nonZero);
  return 0;
}
