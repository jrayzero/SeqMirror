//
// Created by Jessica Ray on 2021-03-16.
//

#include "transform_block_iters.h"
#include "sir/util/cloning.h"
#include "sir/util/irtools.h"
#include "sir/util/matching.h"
#include "sir/analyze/dataflow/reaching.h"
#include "sir/transform/manager.h"
namespace seq {
namespace ir {
namespace transform {
namespace cola {

  bool is_block_type(types::Type *type, Module *M) {
    if (type->getGenerics().size() == 2) {
      if (type->getGenerics()[1].isStatic()) {
	auto *blk = M->getOrRealizeType("Block", type->getGenerics(), "std.cola.block");
	auto *view = M->getOrRealizeType("View", type->getGenerics(), "std.cola.block");
	return type->is(blk) || type->is(view);
      }
    }
    return false;
  }

    // only works for flat loop iterators (like, you can't have Generator[Generator[Block]]
  void TransformBlockIters::handle(ForFlow *flow) {
    // start by cloning so we don't affect anything outside of the loop when we replace loop iters
    util::CloneVisitor cv(flow->getModule());
    auto *cloned_flow = cv.clone(flow)->as<ForFlow>();
    auto M = cloned_flow->getModule();
    auto iter = cloned_flow->getIter();
    if (iter->is<CallInstr>()) {
      auto *call = iter->as<CallInstr>();
      // figure out what kind of call this is and find any block/view args
      auto *call_func = util::getFunc(call->getCallee());
      auto fname = call_func->getUnmangledName();
      vector<Var*> cola_loop_vars;
      vector<Value*> cola_loop_iters;
      bool has_assigns = false;
      // For some reason this breaks horribly when you check for this name. I'm only allowing zip for now.
      /*      if (fname == Module::ITER_MAGIC_NAME) {
	std::cerr << "nargs " << call->numArgs() << std::endl;
	auto *arg = call->front();
	arg->getType()->getGenerics();
	if (is_block_type(arg->getType(),M)) {
	  cola_loop_iters.push_back(arg);
	  cola_loop_vars.push_back(cloned_flow->getVar());
	} else {
	  return;
	}	
	} else */if (fname == "enumerate") {
	// the argument to enumerate is the iterator we want
	auto *arg = call->front();
	if (is_block_type(arg->getType(),M)) {
	  cola_loop_iters.push_back(arg);
	  // there are 2 vars: idx,my_obj
	  auto body_it = flow->getBody()->as<SeriesFlow>()->begin();
	  body_it++;
	  auto *_assign = *body_it;
	  auto *assign = _assign->as<AssignInstr>();
	  seqassert(assign, "not assign");
	  cola_loop_vars.push_back(assign->getLhs());
	}
      } else if (fname == "scan") {
	auto body_it = cloned_flow->getBody()->as<SeriesFlow>()->begin();
	// zip gets a tuple, so should be a call to tuple new
	auto *tuple = call->front();
	auto *tuple_new = tuple->as<CallInstr>();
	seqassert(tuple_new,"");
	int idx = 0;
	for (auto arg = tuple_new->begin(); arg != tuple_new->end(); arg++) {
	  if (call->numArgs() > 1) {
	    auto *assign = (*body_it)->as<AssignInstr>();
	    seqassert(assign,"{}", (**body_it));
	    auto *assign_lhs = (*body_it)->as<AssignInstr>()->getLhs();	  
	    if (is_block_type((*arg)->getType(),M)) {
	      cola_loop_vars.push_back(assign_lhs);
	      cola_loop_iters.push_back(*arg);
	    } else {
	      return;
	    }
	    has_assigns = true;
	    idx++;
	    body_it++;
	  } else {
	    if (is_block_type((*arg)->getType(),M)) {
	      cola_loop_vars.push_back(cloned_flow->getVar());
	      cola_loop_iters.push_back(*arg);
	    } else {
	      return;
	    }
	    idx++;
	  }
	}
      } else {
	return;
      }
      ExtractVarVal evv;
      // Get all the contextual uses of our loop vars
      // want to skip any of the loop var assign stuff (plain iter doesn't have this though)
      int idx = 0;
      for (auto it = cloned_flow->getBody()->as<SeriesFlow>()->begin(); it != cloned_flow->getBody()->as<SeriesFlow>()->end(); it++) {
	if (fname == Module::ITER_MAGIC_NAME || idx >= cola_loop_vars.size() || !has_assigns) {
	  (*it)->accept(evv);
	}
	idx++;
      }
      // Now I have all the used values. Filter out the ones I'm interested in
      vector<VarValue*> used = evv.get_used();
      vector<Value*> used_ctx = evv.get_used_ctx();
      vector<VarValue*> my_used;
      vector<Value*> my_used_ctx;
      vector<int> my_used_idxs; // corresponds to which loop var we use
      for (int i = 0; i < used.size(); i++) {
	auto *var = util::getVar(used[i]); // when is it not a valid varval?
	seqassert(var, "not a varval??");
	int j = 0;
	for (auto *clv : cola_loop_vars) {
	  if (var->getId() == clv->getId()) {
	    my_used.push_back(used[i]);
	    my_used_ctx.push_back(used_ctx[i]);
	    my_used_idxs.push_back(j);
	  }
	  j++;
	}
      }
      if (my_used.empty()) {
	return;
      }
      auto *outer_flow = M->Nr<SeriesFlow>();
      // Now I know the uses and how they are used. Let's see if they are valid uses for transformation
      for (int i = 0; i < my_used.size(); i++) {
	auto *ctx = my_used_ctx[i];
	auto *cctx = ctx->as<CallInstr>();
	if (cctx) {
	  auto *f = util::getFunc(cctx->getCallee());
	  string fname = f->getUnmangledName();
	  if (fname == Module::SETITEM_MAGIC_NAME) {
	    auto unit_type = M->getOrRealizeType("_UNIT", {}, "std.cola.block");
	    auto args = cctx->begin();
	    args++;
	    if (!(*args)->getType()->is(unit_type)) {
	      return;
	    }
	  } else if (fname == Module::CALL_MAGIC_NAME) {
	    auto args = cctx->begin();
	    args++;
	    // should be a tuple new call
	    auto *tup_new = (*args)->as<CallInstr>();
	    seqassert(tup_new, "");
	    if (tup_new->numArgs() != 0) {
	      return;
	    }
	  } else {
	    return;
	  }
	}
      }
      // okay this loop can be transformed! let's get at it
      // all the buffers corresponding to the loop iterators
      vector<VarValue*> buffers;
      // all the new loop iterators
      vector<Value*> all_iters;
      // all the new loop vars
      vector<VarValue*> all_vars;
      int loop_iters_idx = 0;
      Value *full_iter;
      VarValue *full_var;
      if (fname == "scan") {
	// create new loop iterators and loop vars for everything used (which should be every loop component)
	for (int i = 0; i < cola_loop_vars.size(); i++) {
	  if (std::find(my_used_idxs.begin(), my_used_idxs.end(), i) != my_used_idxs.end()) {
	    // buffer
	    auto *base = M->Nr<ExtractInstr>(cola_loop_iters[loop_iters_idx++], "base");
	    seqassert(base, "");
	    auto *_buffer = M->Nr<ExtractInstr>(base, "buffer");
	    seqassert(_buffer, "");
	    auto *buffer = util::makeVar(_buffer, outer_flow, cast<BodiedFunc>(getParentFunc()));
	    seqassert(buffer, "");
	    buffers.push_back(buffer);
	    // iter
	    auto *fiter = M->getOrRealizeMethod(base->getType(), "linear_iter", {base->getType()});
	    seqassert(fiter, "");
	    auto *do_iter = util::call(fiter, {base});
	    all_iters.push_back(do_iter);
	    // var
	    all_vars.push_back(util::makeVar(M->getInt(0), outer_flow, cast<BodiedFunc>(getParentFunc())));
	  } else {
	    seqassert(false, "couldn't find the index");
	  }
	}
	// combine together into tuples and make the new zip function
	vector<types::Type*> zip_types;
	for (auto ai : all_iters) {
	  zip_types.push_back(ai->getType());
	}
	auto *fzip = M->getOrRealizeFunc("scan", {M->getTupleType(zip_types)}, {}, "std.cola.block");
	seqassert(fzip,"");
	// now create the actual new iterator and var
	full_iter = util::call(fzip, {util::makeTuple(all_iters, M)});
	vector<Value*> _all_vars;
	for (Value *av : all_vars) {
	  _all_vars.push_back(av);
	}
	Value *_full_var = util::makeTuple(_all_vars, M);
	full_var = util::makeVar(_full_var, outer_flow, cast<BodiedFunc>(getParentFunc()));
      } else {
	seqassert(false,"");
	return;
      }
      // body
	      
      auto *new_body = M->Nr<SeriesFlow>();
      auto *new_for_flow = M->Nr<ForFlow>(full_iter, new_body, full_var->getVar());
      vector<VarValue*> new_cola_loop_var_assigns;
      auto body_it = cloned_flow->getBody()->as<SeriesFlow>()->begin();
      // add loop var extraction/assignments
      if (has_assigns) {
	loop_iters_idx = 0;
	for (int i = 0; i < cola_loop_vars.size(); i++) {
	  auto *assign = (*body_it)->as<AssignInstr>();
	  seqassert(assign, "not assign");
	  if (std::find(my_used_idxs.begin(), my_used_idxs.end(), i) != my_used_idxs.end()) {
	    loop_iters_idx++;
	    // adds it to the new body
	    auto *new_cola_assign = util::makeVar(M->Nr<ExtractInstr>(full_var, "item" + std::to_string(i+1)), new_body, cast<BodiedFunc>(getParentFunc()));
	    new_cola_loop_var_assigns.push_back(new_cola_assign);
	    assign->replaceAll(new_cola_assign);
	  } else {
	    assign->setRhs(M->Nr<ExtractInstr>(full_var, "item" + std::to_string(i+1)));
	    // need to manually add to new body
	    new_body->push_back(assign);
	  }
	  body_it++;
	}
      }
      // add the rest of the original body
      idx = 0;
      for (auto it = cloned_flow->getBody()->as<SeriesFlow>()->begin(); it != cloned_flow->getBody()->as<SeriesFlow>()->end(); it++) {
	if (fname == Module::ITER_MAGIC_NAME || idx >= cola_loop_vars.size() || !has_assigns) {
	  new_body->push_back(*it);
	}
	idx++;
      }
      // replace all the contextual uses
      idx = 0;
      for (int i = 0; i < cola_loop_vars.size(); i++) {
	auto *ctx = my_used_ctx[i];
	auto *cctx = ctx->as<CallInstr>();
	int cola_idx = my_used_idxs[i];
	// figure out which new loop var this is
	auto *new_loop_var = new_cola_loop_var_assigns[cola_idx];
	auto *f = util::getFunc(cctx->getCallee());	    
	string fname = f->getUnmangledName();
	if (fname == Module::SETITEM_MAGIC_NAME) {
	  auto *setitem = M->getOrRealizeMethod(buffers[cola_idx]->getType(),Module::SETITEM_MAGIC_NAME,{buffers[cola_idx]->getType(),M->getIntType(),cctx->back()->getType()});	      
	  seqassert(setitem, "couldn't find setitem");
	  vector<Value*> args;
	  args.push_back(buffers[cola_idx]);
	  args.push_back(new_loop_var);
	  args.push_back(cctx->back());
	  auto *call = util::call(setitem, args);
	  ctx->replaceAll(call);
	} else {
	  // call
	  auto *getitem = M->getOrRealizeMethod(buffers[cola_idx]->getType(),Module::GETITEM_MAGIC_NAME,{buffers[cola_idx]->getType(),M->getIntType()});
	  seqassert(getitem, "couldn't find getitem");
	  vector<Value*> args;
	  args.push_back(buffers[cola_idx]);
	  args.push_back(new_loop_var);
	  auto *call = util::call(getitem, args);
	  ctx->replaceAll(call);
	}
	idx++;
      }
      outer_flow->push_back(new_for_flow);
      std::cerr << "BEFORE: " << *flow << std::endl;                  
      flow->replaceAll(outer_flow);
      std::cerr << "AFTER: " << *flow << std::endl;      
    }
  }

}
}
}
}
