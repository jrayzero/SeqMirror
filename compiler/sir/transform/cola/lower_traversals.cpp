//
// Created by Jessica Ray on 2021-03-16.
//

#include "lower_traversals.h"
#include "sir/util/cloning.h"
#include "sir/util/irtools.h"
#include "sir/util/matching.h"
#include "sir/analyze/dataflow/reaching.h"
#include "sir/transform/manager.h"
namespace seq {
namespace ir {
namespace transform {
namespace cola {

  bool is_applied_trav_type(types::Type *type, Module *M) {
    if (type->getGenerics().size() == 2) {
      if (type->getGenerics()[1].isStatic()) {
	auto *appt = M->getOrRealizeType("AppliedTraversal", type->getGenerics(), "std.cola.traversal");
	return type->is(appt);
      }
    }
    return false;
  }

  
  void LowerTraversals::handle(ForFlow *flow) {
    auto M = flow->getModule();
    auto iter = flow->getIter();
    if (iter->is<CallInstr>()) {
      auto *call = iter->as<CallInstr>();
      // figure out what kind of call this is and find any block/view args
      auto *call_func = util::getFunc(call->getCallee());
      auto fname = call_func->getUnmangledName();
      if (fname == Module::ITER_MAGIC_NAME) {
	auto *arg = call->front();
	auto *trav_type = M->getOrRealizeType("Traversal", {}, "std.cola.traversal");
	if (is_applied_trav_type(arg->getType(),M)) {
	  std::cerr << *flow << std::endl;
	} else {
	  return;
	}
	// the arg should be a call to Traversal.__call__
	auto *trav_call = arg->as<CallInstr>();
	seqassert(trav_call,"");
	// the first arg is the traversal, and the second is the block to run it on
	auto *trav_inst = trav_call->front();
	auto *blk = trav_call->back();
	seqassert(trav_inst && blk, "");
	// let's try and build this thing
	
	
      }
      
    }
    
  }
  
}
}
}
}
