//
// Created by Jessica Ray on 2021-03-21.
//

#pragma once

#include "sir/transform/pass.h"
#include "sir/sir.h"

namespace seq {
namespace ir {
namespace transform {
namespace cola {

class PrintFuncs : public OperatorPass {

  void handle(CallInstr *) override;

};

}
}
}
}