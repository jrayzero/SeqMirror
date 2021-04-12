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
    auto *cloned_flow = cast<ForFlow>(cv.clone(flow));
    auto M = cloned_flow->getModule();
    auto iter = cloned_flow->getIter();
    if (iter->is<CallInstr>()) {
      auto *call = iter->as<CallInstr>();
      // figure out what kind of call this is and find any block/view args
      auto *call_func = util::getFunc(call->getCallee());
      auto fname = call_func->getUnmangledName();
      vector<Var*> cola_loop_vars;
      vector<Value*> cola_loop_iters;
      vector<int> cola_loop_idxs;
      vector<Var*> all_original_loop_vars;
      int body_skip = 0;
      if (fname == "iter") {
	auto *arg = call->front();
	if (is_block_type(arg->getType(),M)) {	  
	  cola_loop_vars.push_back(cloned_flow->getVar());
	  cola_loop_iters.push_back(arg);
	  cola_loop_idxs.push_back(0);
	}
      } else if (fname == "zip") {
	auto body_it = cloned_flow->getBody()->as<SeriesFlow>()->begin();
	// zip gets a tuple, so should be a call to tuple new
	auto *tuple = call->front();
	auto *tuple_new = tuple->as<CallInstr>();
	seqassert(tuple_new,"");
	int idx = 0;
	for (auto arg = tuple_new->begin(); arg != tuple_new->end(); arg++) {
	  seqassert(body_it != cloned_flow->getBody()->as<SeriesFlow>()->end(), "");	  
	  seqassert((*body_it)->as<AssignInstr>(),"");
	  auto *assign_lhs = (*body_it)->as<AssignInstr>()->getLhs();
	  if (is_block_type((*arg)->getType(),M)) {
	    cola_loop_vars.push_back(assign_lhs);
	    cola_loop_iters.push_back(*arg);
	    cola_loop_idxs.push_back(idx);
	  }
	  all_original_loop_vars.push_back(assign_lhs);
	  idx++;
	  body_it++;
	  body_skip++;
	}
      } else if (fname == "enumerate") {
	auto *arg = call->front();
	// there are two loop vars for enumerate that get assigned in the body.
	// the first is the index and the second is what we care about
	auto body_it = cloned_flow->getBody()->as<SeriesFlow>()->begin();
	body_it++;
	auto *assign_instr = *body_it;
	auto *assign = assign_instr->as<AssignInstr>();
	seqassert(assign,"");
	auto *assign_lhs = assign->getLhs();
	cola_loop_vars.push_back(assign_lhs);
	cola_loop_iters.push_back(arg);
	body_skip = 2;
	cola_loop_idxs.push_back(1);
      } else {
	return;
      }
      if (cola_loop_vars.empty()) {
	return; // no cola things in loop
      }
      ExtractVarVal evv;
      // want to skip any of the loop var assign stuff
      int idx = 0;
      for (auto it = cloned_flow->getBody()->as<SeriesFlow>()->begin(); it != cloned_flow->getBody()->as<SeriesFlow>()->end(); it++) {
	if (idx >= body_skip) {
	  (*it)->accept(evv);
	}
	idx++;
      }														     
      vector<VarValue*> used = evv.get_used();
      vector<Value*> used_ctx = evv.get_used_ctx();
      vector<VarValue*> my_used;
      vector<Value*> my_used_ctx;
      vector<int> my_used_idxs; // cooresponds to which loop var we use
      // Now I have all the used values. Filter out the ones I'm interested in
      for (int i = 0; i < used.size(); i++) {
	auto *var = util::getVar(used[i]); // when is it not a valid varval?
	if (var) {
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
      }
      if (my_used.empty()) {
	return;
      }
      // Now I know the uses and how they are used. Let's see if they are valid uses for transformation
      vector<bool> valid;
      bool any_valid = false;
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
	    if ((*args)->getType()->is(unit_type)) {
	      valid.push_back(true);
	      any_valid = true;
	    } else {
	      valid.push_back(false);
	    }
	  } else if (fname == Module::CALL_MAGIC_NAME) {
	    auto args = cctx->begin();
	    args++;
	    // should be a tuple new call
	    auto *tup_new = (*args)->as<CallInstr>();
	    seqassert(tup_new, "");
	    if (tup_new->numArgs() == 0) {
	      valid.push_back(true);
	      any_valid = true;
	    } else {
	      valid.push_back(false);
	    }
	  } else {
	    valid.push_back(false);
	  }
	}
      }
      if (!any_valid) {
	return;
      }
      auto *outer_flow = M->Nr<SeriesFlow>();
      // Let's transform the valid uses.
      if (fname == "iter" || fname == "enumerate") {
	// get a reference to the buffer
	auto *base = M->Nr<ExtractInstr>(cv.clone(cola_loop_iters[0]), "base"); // clone so doesn't get messed up when we replace loop iter
	auto *_buffer = M->Nr<ExtractInstr>(base, "buffer");
	auto *buffer = util::makeVar(_buffer, outer_flow, cast<BodiedFunc>(getParentFunc()));
	// create the new cola loop iter
	auto *fiter = M->getOrRealizeMethod(base->getType(), "linear_iter", {base->getType()});
	seqassert(fiter, "");
	auto *do_iter = util::call(fiter, {base});
	Value *full_iter;
	// create the new overall loop iter
	if (fname == "iter") {
	  full_iter = do_iter;
	} else {	  
	  auto *enumer = M->getOrRealizeFunc("enumerate", {do_iter->getType(), M->getIntType()}, {}, "std.internal.builtin");
	  seqassert(enumer,"");
	  full_iter = util::call(enumer, {do_iter, M->getInt(0)});
	}
	// create the new cola loop var
	auto *new_loop_var = util::makeVar(M->getInt(0), outer_flow, cast<BodiedFunc>(getParentFunc()));
	// create the new overall loop var
	VarValue *full_var;
	if (fname == "iter") {
	  full_var = M->Nr<VarValue>(new_loop_var->getVar());
	} else {
	  auto *tup = util::makeTuple({util::makeVar(M->getInt(0), outer_flow, cast<BodiedFunc>(getParentFunc())), new_loop_var}, M);
	  full_var = util::makeVar(tup, outer_flow, cast<BodiedFunc>(getParentFunc()));
	}
	auto *new_body = M->Nr<SeriesFlow>();
	auto *new_for_flow = M->Nr<ForFlow>(full_iter, new_body, full_var->getVar());
	
	vector<VarValue*> new_cola_loop_var_assigns;
	if (fname == "enumerate") {
	  // need to update the assign/extract in the body for the loop vars (both cola and regular)
	  auto assign_it = cloned_flow->getBody()->as<SeriesFlow>()->begin();
	  auto *assign_ctr = (*assign_it)->as<AssignInstr>();
	  seqassert(assign_ctr, "not assign");
	  assign_ctr->setRhs(M->Nr<ExtractInstr>(full_var, "item1"));
	  new_body->push_back(assign_ctr);
	  assign_it++;
	  auto *assign_cola = (*assign_it)->as<AssignInstr>();
	  seqassert(assign_cola, "not assign");
	  auto *new_cola_assign = util::makeVar(M->Nr<ExtractInstr>(full_var, "item2"), new_body, cast<BodiedFunc>(getParentFunc()));
	  new_cola_loop_var_assigns.push_back(new_cola_assign);
	  assign_cola->replaceAll(new_cola_assign);	  
	}
	// add the original body
	int idx = 0;
	for (auto it = cloned_flow->getBody()->as<SeriesFlow>()->begin(); it != cloned_flow->getBody()->as<SeriesFlow>()->end(); it++) {
	  if (idx >= body_skip) {
	    new_body->push_back(*it);
	  }
	  idx++;
	}
	// replace ALL the ctx uses with the appropriate buffer access
	for (int i = 0; i < valid.size(); i++) {
	  if (valid[i]) {
	    auto *ctx = my_used_ctx[i];
	    auto *cctx = ctx->as<CallInstr>();
	    // figure out which new loop var this is
	    auto *new_loop_var = new_cola_loop_var_assigns[my_used_idxs[i]];
	    auto *f = util::getFunc(cctx->getCallee());	    
	    string fname = f->getUnmangledName();
	    if (fname == Module::SETITEM_MAGIC_NAME) {
	      auto *setitem = M->getOrRealizeMethod(buffer->getType(),Module::SETITEM_MAGIC_NAME,{buffer->getType(),M->getIntType(),cctx->back()->getType()});	      
	      seqassert(setitem, "couldn't find setitem");
	      vector<Value*> args;
	      args.push_back(buffer);
	      args.push_back(new_loop_var);
	      args.push_back(cctx->back());
	      auto *call = util::call(setitem, args);
	      ctx->replaceAll(call);
	    } else {
	      // call
	      auto *getitem = M->getOrRealizeMethod(buffer->getType(),Module::GETITEM_MAGIC_NAME,{buffer->getType(),M->getIntType()});
	      seqassert(getitem, "couldn't find getitem");
	      vector<Value*> args;
	      args.push_back(buffer);
	      args.push_back(new_loop_var);
	      auto *call = util::call(getitem, args);
	      ctx->replaceAll(call);
	    }
	  }	  
	}
	outer_flow->push_back(new_for_flow);
      } else {
	// it's a zip
	// get all the buffers and create the new loop iterators/loop vars
	vector<VarValue*> buffers;
	vector<Value*> all_iters;
	vector<VarValue*> all_vars;
	int loop_iters_idx = 0;
	for (int i = 0; i < body_skip; i++) {
	  if (std::find(my_used_idxs.begin(), my_used_idxs.end(), i) != my_used_idxs.end() && valid[loop_iters_idx]) {
	    // buffer
	    auto *base = M->Nr<ExtractInstr>(cv.clone(cola_loop_iters[loop_iters_idx++]), "base");
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
	    // iter
	    auto start = iter->as<CallInstr>()->begin();
	    start += i;
	    all_iters.push_back(*start);
	    // var
	    all_vars.push_back(M->Nr<VarValue>(all_original_loop_vars[i]));
	  }
	}
	// combine together into tuples
	vector<types::Type*> zip_types;
	for (auto ai : all_iters) {
	  zip_types.push_back(ai->getType());
	}
	auto *fzip = M->getOrRealizeFunc("zip", {M->getTupleType(zip_types)}, {}, "std.internal.builtin");
	seqassert(fzip,"");
	Value *full_iter = util::call(fzip, {util::makeTuple(all_iters, M)});
	vector<Value*> _all_vars;
	for (Value *av : all_vars) {
	  _all_vars.push_back(av);
	}
	Value *_full_var = util::makeTuple(_all_vars, M);
	VarValue *full_var = util::makeVar(_full_var, outer_flow, cast<BodiedFunc>(getParentFunc()));
	// body
	auto *new_body = M->Nr<SeriesFlow>();
	auto *new_for_flow = M->Nr<ForFlow>(full_iter, new_body, full_var->getVar());
	vector<VarValue*> new_cola_loop_var_assigns;
	auto body_it = cloned_flow->getBody()->as<SeriesFlow>()->begin();
	// update loop var extraction/assignments
	loop_iters_idx = 0;
	for (int i = 0; i < body_skip; i++) {
	  auto *assign = (*body_it)->as<AssignInstr>();
	  seqassert(assign, "not assign");
	  if (std::find(my_used_idxs.begin(), my_used_idxs.end(), i) != my_used_idxs.end() && valid[loop_iters_idx]) {
	    loop_iters_idx++;
	    auto *new_cola_assign = util::makeVar(M->Nr<ExtractInstr>(full_var, "item" + std::to_string(i+1)), new_body, cast<BodiedFunc>(getParentFunc()));
	    new_cola_loop_var_assigns.push_back(new_cola_assign);
	    assign->replaceAll(new_cola_assign);
	  } else {
	    assign->setRhs(M->Nr<ExtractInstr>(full_var, "item" + std::to_string(i+1)));	    
	    new_body->push_back(assign);
	  }
	  body_it++;
	}
	// add the rest of the original body
	int idx = 0;
	for (auto it = cloned_flow->getBody()->as<SeriesFlow>()->begin(); it != cloned_flow->getBody()->as<SeriesFlow>()->end(); it++) {
	  if (idx >= body_skip) {
	    new_body->push_back(*it);
	  }
	  idx++;
	}
	// replace ctx uses
	idx = 0;
	for (int i = 0; i < valid.size(); i++) {
	  if (valid[i]) {
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
	}
	outer_flow->push_back(new_for_flow);
      }
      flow->replaceAll(outer_flow);
    }
  }
   
}
}
}
}
