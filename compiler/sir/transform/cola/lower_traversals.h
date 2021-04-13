#pragma once

#include "sir/transform/pass.h"
#include "sir/analyze/dataflow/reaching.h"
#include "sir/attribute.h"
#include "sir/sir.h"
#include "sir/util/visitor.h"
#include <stack>

namespace seq {
namespace ir {
namespace transform {
namespace cola {
  
  struct LowerTraversals : public OperatorPass {
    void handle(ForFlow *flow);
  };
}
}
}
}
