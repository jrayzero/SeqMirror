//
// Created by Jessica Ray on 2021-03-21.
//

#include "print_func_ir.h"
#include "sir/util/irtools.h"
#include "sir/util/cloning.h"
#include "util/common.h"

namespace seq {
namespace ir {
namespace transform {
namespace cola {

using namespace std;

void PrintFuncs::handle(seq::ir::CallInstr *instr) {
  auto f = instr->getCallee();
  if (cast<BodiedFunc>(util::getFunc(f))) {
    string fname = cast<BodiedFunc>(util::getFunc(f))->getUnmangledName();
    if (fname == "sse" || fname == "sad" || fname == "satd_4x4") {
      std::cerr << fname << std::endl;
      std::cerr << *cast<BodiedFunc>(util::getFunc(f)) << std::endl;
    }
  }
}

}
}
}
}