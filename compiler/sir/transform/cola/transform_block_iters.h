#pragma once

#include "sir/transform/pass.h"
#include "sir/analyze/dataflow/reaching.h"
#include "sir/attribute.h"
#include "sir/sir.h"
#include "sir/util/visitor.h"
#include "sir/util/irtools.h"
#include <stack>

namespace seq {
namespace ir {
namespace transform {
namespace cola {

  // if you use a loop var outside of () or [int*],
  // then this will not transform the loop (so even
  // if you just copy, that is invalid).
  // So,  only transforms if you have __setitem__(self, UNIT) or __call__(self)
  // for now. Should be able to easily modify to accept any INTEGER args though

  // THIS WILL GIVE INCORRECT RESULTS IF YOU PASS AN ITERATOR THAT HAS ALREADY
  // HAD NEXT CALLED ON IT
  struct TransformBlockIters : public OperatorPass {
    void handle(ForFlow*) override;
  };

  struct FastPathScans : public OperatorPass {
    void handle(ForFlow*) override;
  };
  

// recursively get all used values
 class ExtractVarVal : public util::Visitor {
 private:

   vector<VarValue*> used;
   vector<Value*> used_ctx;
   std::stack<Value*> temp_ctx;

   template <typename T>
   void general_visit(T *v) {
     temp_ctx.push(v);
     auto used = v->getUsedValues();
     for (auto *u : used) {       
       u->accept(*this);
     }
     temp_ctx.pop();
   }

 public:

   vector<VarValue*> get_used() { return used; }
   vector<Value*> get_used_ctx() { return used_ctx; }
   
   ExtractVarVal() {}

   virtual ~ExtractVarVal() noexcept = default;

   void visit(Var *v) override { throw std::runtime_error("cannot visit var"); }

   void visit(VarValue *v) override {
     if (util::getVar(v)) {
	 used.push_back(v);
	 used_ctx.push_back(temp_ctx.top());
       }
   }
   void visit(PointerValue *v) override { throw std::runtime_error("cannot visit pointervalue"); }

   void visit(SeriesFlow *v) override { general_visit(v); }
   void visit(IfFlow *v) override { general_visit(v); }
   void visit(WhileFlow *v) override { general_visit(v); }
   void visit(ForFlow *v) override { general_visit(v); }
   void visit(TryCatchFlow *v) override { general_visit(v); }
   void visit(PipelineFlow *v) override { general_visit(v); }
   void visit(dsl::CustomFlow *v) override { general_visit(v); }

   void visit(IntConst *v) override { general_visit(v); }
   void visit(FloatConst *v) override { general_visit(v); }
   void visit(BoolConst *v) override { general_visit(v); }
   void visit(StringConst *v) override { general_visit(v); }
   void visit(dsl::CustomConst *v) override { general_visit(v); }

   void visit(AssignInstr *v) override { general_visit(v); }
   void visit(ExtractInstr *v) override { general_visit(v); }
   void visit(InsertInstr *v) override { general_visit(v); }
   void visit(CallInstr *v) override { general_visit(v); }
   void visit(StackAllocInstr *v) override { general_visit(v); }
   void visit(TypePropertyInstr *v) override { general_visit(v); }
   void visit(YieldInInstr *v) override { general_visit(v); }
   void visit(TernaryInstr *v) override { general_visit(v); }
   void visit(BreakInstr *v) override { general_visit(v); }
   void visit(ContinueInstr *v) override { general_visit(v); }
   void visit(ReturnInstr *v) override { general_visit(v); }
   void visit(YieldInstr *v) override { general_visit(v); }
   void visit(ThrowInstr *v) override { general_visit(v); }
   void visit(FlowInstr *v) override { general_visit(v); }
   void visit(dsl::CustomInstr *v) override { general_visit(v); }
 };

}
}
}
}
