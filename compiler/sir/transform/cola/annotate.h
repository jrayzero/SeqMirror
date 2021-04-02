#pragma once

#include "sir/transform/pass.h"
#include "sir/attribute.h"
#include "sir/sir.h"

namespace seq {
namespace ir {
namespace transform {
namespace cola {

struct AnnotateDims : public OperatorPass {

  void handle(CallInstr *);


};

}
}
}
}