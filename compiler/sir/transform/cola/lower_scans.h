#pragma once

#include "sir/transform/pass.h"
#include "sir/analyze/dataflow/reaching.h"
#include "sir/attribute.h"
#include "sir/sir.h"
#include "sir/util/visitor.h"

namespace seq {
namespace ir {
namespace transform {
namespace cola {

  // look for calls of the form:
  // ingest[B](__getitem__(block, Tuple[int*]), fd)
  struct ModifySlicedIngests : public OperatorPass {
    void handle(CallInstr*) override;
  };
  
struct LowerScans : public OperatorPass {

  std::map<int, Var*> lhs;

  const analyze::dataflow::RDResult *reaching = nullptr;

  string key;

  explicit LowerScans(string key);

  void handle(ForFlow *) override;

};

}
}
}
}
