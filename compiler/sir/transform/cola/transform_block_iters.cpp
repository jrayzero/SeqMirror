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

Value *extract_all(Module *M, Value *base, std::vector<string> fields) {
  Value *extracted = base;
  for (auto s : fields) {
    extracted = M->Nr<ExtractInstr>(extracted,s);
  }
  return extracted;
}

void Unroll::handle(seq::ir::ForFlow *flow) {
  auto *cloned_flow = util::CloneVisitor(flow->getModule()).clone(flow)->as<ForFlow>();
  auto M = cloned_flow->getModule();
  auto iter = cloned_flow->getIter();
  if (iter->is<CallInstr>()) {
    auto *call = iter->as<CallInstr>();
    auto *call_func = util::getFunc(call->getCallee());
    auto fname = call_func->getUnmangledName();
    if (fname == Module::ITER_MAGIC_NAME) {
      // check if this is a range type
      auto *range_ty = M->getOrRealizeType("range", {}, "std.internal.types.range");
      seqassert(range_ty, "");
      if (!call->front()->getType()->is(range_ty)) {
        return;
      }
      auto *range_new = call->front()->as<CallInstr>();
      if (!range_new) {
        return;
      }
      int64_t start = 0;
      int64_t stop = 0;
      int64_t step = 0;
      bool is_unrollable = false;
      if (range_new->numArgs() == 1) {
        // stop only
        if (range_new->front()->is<IntConst>()) {
          start = 0;
          stop = range_new->front()->as<IntConst>()->getVal();
          step = 1;
          is_unrollable = true;
        }
      } else if (range_new->numArgs() == 2) {
        // start + stop
        auto it = range_new->begin();
        auto *_start = *it;
        it++;
        auto *_stop = *it;
        if (_start->is<IntConst>() && _stop->is<IntConst>()) {
          start = _start->as<IntConst>()->getVal();
          stop = _stop->as<IntConst>()->getVal();
          step = 1;
          is_unrollable = true;
        }
      } else {
        // start + stop + step
        auto it = range_new->begin();
        auto *_start = *it;
        it++;
        auto *_stop = *it;
        it++;
        auto *_step = *it;
        if (_start->is<IntConst>() && _stop->is<IntConst>() && _step->is<IntConst>()) {
          start = _start->as<IntConst>()->getVal();
          stop = _stop->as<IntConst>()->getVal();
          step = _step->as<IntConst>()->getVal();
          is_unrollable = true;
        }
      }
      if (!is_unrollable) {
        return;
      }

      auto *unrolled = M->Nr<SeriesFlow>();
      // for now, unroll the whole thing
      for (int64_t i = start; i < stop; i+=step) {
        std::cerr << "ITERATION " << i << std::endl;
        auto *idx = M->getInt(i);
        unrolled->push_back(M->Nr<AssignInstr>(flow->getVar(), idx));
//        auto *var_idx = util::makeVar(idx, unrolled, cast<BodiedFunc>(getParentFunc()));
        auto *cloned = cast<ForFlow>(util::CloneVisitor(M).clone(flow));
//        ExtractVarVal evv;
//        for (auto it = cloned->getBody()->as<SeriesFlow>()->begin(); it != cloned->getBody()->as<SeriesFlow>()->end(); it++) {
//          (*it)->accept(evv);
//        }
//        vector<VarValue*> used = evv.get_used();
//        for (int i = 0; i < used.size(); i++) {
//          auto *var = util::getVar(used[i]); // when is it not a valid varval?
//          seqassert(var, "not a varval??");
//          if (var->getId() == cloned->getVar()->getId()) {
//            var->replaceAll(var_idx->getVar());
//          }
//        }
//        auto *cloned_var = cloned->getVar();
//        cloned_var->replaceAll(var_idx->getVar());
        unrolled->push_back(cloned->getBody());
      }

      flow->replaceAll(unrolled);
      std::cerr << *flow << std::endl;

    }
  }
}

// only supports scan and range. If you make this general, it fails on things like iterating through lists which can return a tuple
// when iterating over one list since the list could contain tuple's itself
void CanonicalizeLoops::handle(seq::ir::ForFlow *flow) {
  auto *cloned_flow = util::CloneVisitor(flow->getModule()).clone(flow)->as<ForFlow>();
  auto M = cloned_flow->getModule();
  auto iter = cloned_flow->getIter();
  if (iter->is<CallInstr>()) {
    auto *call = iter->as<CallInstr>();
    auto *call_func = util::getFunc(call->getCallee());
    auto fname = call_func->getUnmangledName();
    auto *outer_flow = M->Nr<SeriesFlow>();
    if (fname == "scan") {
      // convert each object within the scan to a generator and store outside the loop
      vector<VarValue *> generators;
      vector<types::Type *> gen_types;
      auto *tuple = call->front();
      auto *tuple_new = tuple->as<CallInstr>();
      seqassert(tuple_new,"");
      for (auto it = tuple_new->begin(); it != tuple_new->end(); it++) {
        auto *stdlib_iter = M->getOrRealizeFunc("iter", {(*it)->getType()}, {}, "std.internal.builtin");
        seqassert(stdlib_iter, "");
        // wrap it in a generator that gets assigned outside the body
        generators.push_back(util::makeVar(util::call(stdlib_iter, {*it}), outer_flow, cast<BodiedFunc>(getParentFunc())));
        gen_types.push_back(generators.back()->getType());
      }
      // now we have all of our new generators. let's see if we need to tuplefy the object.
      // we tuplefy if the original loop var type is not a record
      auto *gen_type = cloned_flow->getVar()->getType();
      bool converted_to_tuple = false;
      Value *tuplefied_iter = nullptr;
      if (!gen_type->as<types::RecordType>()) {
        // not a tuple. let's make it a tuple by passing it to _compiler_scan
        seqassert(generators.size() == 1, ""); // sanity check
        auto *tuplefy = M->getOrRealizeFunc("_compiler_scan", {M->getTupleType({generators[0]->getType()})},
                                            {}, "std.cola.compiler");
        seqassert(tuplefy, "");
        converted_to_tuple = true;
        tuplefied_iter = util::call(tuplefy, {util::makeTuple({generators[0]}, M)});
      } else {
        // still wrap it in _compiler_scan so we have a more unified interface
        auto *tuplefy = M->getOrRealizeFunc("_compiler_scan", {M->getTupleType(gen_types)},
                                            {}, "std.cola.compiler");
        seqassert(tuplefy, "");
        converted_to_tuple = true;
        vector<Value*> _gens; // need generators as Values not VarValues
        for (auto g : generators) {
          _gens.push_back(g);
        }
        tuplefied_iter = util::call(tuplefy, {util::makeTuple(_gens, M)});
      }
      // TODO refactor cause all this stuff below here is the same as for non-scan-generators
      Var *new_loop_var = nullptr;
      auto *body = M->Nr<SeriesFlow>();
      if (converted_to_tuple) {
        // not using util::makeVar here because I have no assignment to make to this
        new_loop_var = M->Nr<Var>(M->getTupleType({cloned_flow->getVar()->getType()}));
        cast<BodiedFunc>(getParentFunc())->push_back(new_loop_var);
        auto *extractor = util::makeVar(extract_all(M, M->Nr<VarValue>(new_loop_var), {"item1"}), body, cast<BodiedFunc>(getParentFunc()));
        // since we created a tuple where there wasn't one before, everything in the loop body should point to the extracted
        // var. // we will use "new_loop_var" as the thing we actually pass to the new forflow
        cloned_flow->getVar()->replaceAll(extractor->getVar());
      } else {
        new_loop_var = cloned_flow->getVar();
      }
      // recreate the loop now
      auto *fflow = M->Nr<ForFlow>(converted_to_tuple ? tuplefied_iter : iter, body, new_loop_var);
      // shove in the rest of the body
      for (auto it = cloned_flow->getBody()->as<SeriesFlow>()->begin(); it != cloned_flow->getBody()->as<SeriesFlow>()->end(); it++) {
        body->push_back(*it);
      }
      outer_flow->push_back(fflow);
    } else if (fname == Module::ITER_MAGIC_NAME) {
      // check if this is a range type
      auto *range_ty = M->getOrRealizeType("range", {}, "std.internal.types.range");
      seqassert(range_ty, "");
      if (!call->front()->getType()->is(range_ty)) {
        return;
      }
      bool converted_to_tuple = false;
      Value *tuplefied_iter = nullptr;
      // store this iterator in a variable outside of the loop so we can have access to it explicitly within the loop
      auto *gen_iterator = util::makeVar(iter, outer_flow,  cast<BodiedFunc>(getParentFunc()));
      // see if the loop produces a tuple, which means the return type of the thing iterated over is a tuple. if not, make it a tuple
      auto *gen_type = cloned_flow->getVar()->getType();
      if (!gen_type->as<types::RecordType>()) {
        auto generic = cast<types::GeneratorType>(iter->getType())->getGenerics()[0];
        auto *tuplefy = M->getOrRealizeFunc("_compiler_iter_tuplefy", {iter->getType()}, {generic}, "std.cola.compiler");
        seqassert(tuplefy, "{}\n{}", *cloned_flow, *iter->getType());
        converted_to_tuple = true;
        tuplefied_iter = util::call(tuplefy, {gen_iterator});
      }

      Var *new_loop_var = nullptr;
      auto *body = M->Nr<SeriesFlow>();
      if (converted_to_tuple) {
        // not using util::makeVar here because I have no assignment to make to this
        new_loop_var = M->Nr<Var>(M->getTupleType({cloned_flow->getVar()->getType()}));
        cast<BodiedFunc>(getParentFunc())->push_back(new_loop_var);
        auto *extractor = util::makeVar(extract_all(M, M->Nr<VarValue>(new_loop_var), {"item1"}), body, cast<BodiedFunc>(getParentFunc()));
        // since we created a tuple where there wasn't one before, everything in the loop body should point to the extracted
        // var. // we will use "new_loop_var" as the thing we actually pass to the new forflow
        cloned_flow->getVar()->replaceAll(extractor->getVar());
      } else {
        new_loop_var = cloned_flow->getVar();
      }
      // recreate the loop now
      auto *fflow = M->Nr<ForFlow>(converted_to_tuple ? tuplefied_iter : iter, body, new_loop_var);
      // shove in the rest of the body
      for (auto it = cloned_flow->getBody()->as<SeriesFlow>()->begin(); it != cloned_flow->getBody()->as<SeriesFlow>()->end(); it++) {
        body->push_back(*it);
      }
      outer_flow->push_back(fflow);
    } else {
      return;
    }
    flow->replaceAll(outer_flow);
  }
}

void FastPathScans::handle(ForFlow *flow) {
  util::CloneVisitor cv(flow->getModule());
  auto *cloned_flow = cv.clone(flow)->as<ForFlow>();
  auto M = cloned_flow->getModule();
  auto iter = cloned_flow->getIter();
  if (iter->is<CallInstr>()) {
    auto *call = iter->as<CallInstr>();
    // figure out what kind of call this is and find any block/view args
    auto *call_func = util::getFunc(call->getCallee());
    auto fname = call_func->getUnmangledName();
    int nscan_args = 0;
    int ndims = -1;
    vector<Value*> cola_iters;
    if (fname == "scan") {
      // see if the arguments to scan are calls to linear_iter
      auto *tuple = call->front()->as<CallInstr>();
      for (auto _arg = tuple->begin(); _arg != tuple->end(); _arg++) {
        auto *arg = *_arg;
        if (arg->is<CallInstr>()) {
          auto *arg_call = arg->as<CallInstr>();
          auto *arg_func = util::getFunc(arg_call->getCallee());
          auto arg_fname = arg_func->getUnmangledName();
          if (arg_fname != "linear_iter") {
            return;
          }
          int dims = arg_call->front()->getType()->getGenerics()[1].getStaticValue();
          if (ndims == -1) {
            ndims = dims;
          } else {
            if (dims != ndims) {
              return; // must be the same number of dims
            }
          }
          cola_iters.push_back(arg_call->front());
        } else {
          return;
        }
        nscan_args++;
      }
    } else {
      return;
    }
    if (nscan_args == 1) {
      return; // this isn't really useful for single argument scans (at least I don't think it is)
    }
    // alright, we have a loop to add the fastpath to
    // create the conditions checking if the fastpath is valid
    Value *cond;// = M->getBool(true);
    if (ndims == 1) {
      return; // TODO
    } else if (ndims == 2) {
      return; // TODO
    } else if (ndims == 3) {
      auto *same_shape = M->getOrRealizeFunc("is_same_shape3", {cola_iters[0]->getType(), cola_iters[1]->getType()},
                                             {}, "std.cola.block");
      std::cerr << *cola_iters[0]->getType() << std::endl;
      std::cerr << *cola_iters[1]->getType() << std::endl;
      seqassert(same_shape, "");
      cond = util::call(same_shape, {cola_iters[0], cola_iters[1]});
      for (auto it = cola_iters.begin()+2; it != cola_iters.end(); it++) {
        auto *same_shape = M->getOrRealizeFunc("is_same_shape3", {cola_iters[0]->getType(), (*it)->getType()}, {}, "std.cola.block");
        seqassert(same_shape, "");
        Value *cond2 = util::call(same_shape, {cola_iters[0], (*it)});
        cond = *cond && *cond2;
      }
    } else {
      return;
    }

    // create the ifflow for the condition
    auto *true_flow = M->Nr<SeriesFlow>();
    see(true_flow);
    auto *false_flow = M->Nr<SeriesFlow>();
    false_flow->push_back(cloned_flow); // slow path
    see(false_flow);
    auto *ifflow = M->Nr<IfFlow>(cond, true_flow, false_flow);
    see(ifflow);
    auto *outer_flow = M->Nr<SeriesFlow>();
    see(outer_flow);
    outer_flow->push_back(ifflow);
    // create the new loop nests for the fast path
    if (ndims == 1) {
      return; // TODO implement
    } else if (ndims == 2) {
      return; // TODO implement
    } else {
      // get the dims
      auto *citer = cola_iters[0]; // dims should be the same at this point, so just take one
      auto *dc0 = extract_all(M, citer, {"dims", "item1"});
      auto *dc1 = extract_all(M, citer, {"dims", "item2"});
      auto *dc2 = extract_all(M, citer, {"dims", "item3"});
      // make the new loop vars
      auto *i = util::makeVar(M->getInt(0), true_flow, cast<BodiedFunc>(getParentFunc()));
      auto *j = util::makeVar(M->getInt(0), true_flow, cast<BodiedFunc>(getParentFunc()));
      auto *k = util::makeVar(M->getInt(0), true_flow, cast<BodiedFunc>(getParentFunc()));
      // make the new loop iters
      auto *range_ty = M->getOrRealizeType("range", {}, "std.internal.types.range");
      seqassert(range_ty, "");
      auto *range = M->getOrRealizeMethod(range_ty, Module::NEW_MAGIC_NAME, {M->getIntType()});
      seqassert(range,"");
      auto *range_iter = M->getOrRealizeMethod(range_ty, Module::ITER_MAGIC_NAME, {range_ty});
      seqassert(range_iter,"");
      auto *i_iter = util::call(range_iter, {util::call(range, {dc0})});
      auto *j_iter = util::call(range_iter, {util::call(range, {dc1})});
      auto *k_iter = util::call(range_iter, {util::call(range, {dc2})});
      // create the loop nest skeleton
      auto *i_body = M->Nr<SeriesFlow>();
      auto *j_body = M->Nr<SeriesFlow>();
      auto *k_body = M->Nr<SeriesFlow>();
      auto *i_loop = M->Nr<ForFlow>(i_iter, i_body, i->getVar());
      auto *j_loop = M->Nr<ForFlow>(j_iter, j_body, j->getVar());
      auto *k_loop = M->Nr<ForFlow>(k_iter, k_body, k->getVar());
      true_flow->push_back(i_loop);
      i_body->push_back(j_loop);
      j_body->push_back(k_loop);
      // compute the linear index for each thing we zip through
      vector<Value *> lin_idxs;
      for (auto it = cola_iters.begin(); it != cola_iters.end(); it++) {
        auto *dims = extract_all(M, *it, {"buffer_parent", "dims"});
        auto *bmap = extract_all(M, *it, {"buffer_mapping"});
        // do the base mapping
        auto *i_map = *(*i * *extract_all(M, bmap, {"item1","step"})) + *extract_all(M, bmap, {"item1", "start"});
        auto *j_map = *(*j * *extract_all(M, bmap, {"item2","step"})) + *extract_all(M, bmap, {"item2", "start"});
        auto *k_map = *(*k * *extract_all(M, bmap, {"item3","step"})) + *extract_all(M, bmap, {"item3", "start"});
        // now factor in the dims and other iterators
        auto *bdim_j = extract_all(M, dims, {"item2"});
        auto *bdim_k = extract_all(M, dims, {"item3"});
        auto *_term1 = *i_map * *bdim_j;
        auto *term1 = *_term1 * *bdim_k;
        auto *_term2 = *j_map * *bdim_k;
        auto *term2 = *_term2 + *k_map;
        auto *lin = *term1 + *term2;
        lin_idxs.push_back(lin);
      }
      // now go through and copy over the body, but replace the assigns at the beginning of the body to point to our idxs we just computed
      cv = util::CloneVisitor(M);
      auto *clone = cv.clone(flow);
      auto body_it = clone->as<ForFlow>()->getBody()->as<SeriesFlow>()->begin();
      for (auto *lin : lin_idxs) {
        auto *assign = (*body_it)->as<AssignInstr>();
        seqassert(assign,"");
        assign->setRhs(lin);
        k_body->push_back(assign);
        body_it++;
      }
      // now add the rest of the bod
      for (auto bit = body_it; bit != clone->as<ForFlow>()->getBody()->as<SeriesFlow>()->end(); bit++) {
        k_body->push_back(*bit);
      }
    }
    //    true_flow->push_back(util::CloneVisitor(M).clone(flow));
    flow->replaceAll(outer_flow);
    std::cerr << "Applied fast path " << *flow << std::endl;
  }
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
    // For some reason this breaks horribly when you check for this name. I'm only allowing "scan" for now.
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
      return; // not supported currently
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
      // scan gets a tuple, so should be a call to tuple new
      auto *tuple = call->front();
      auto *tuple_new = tuple->as<CallInstr>();
      seqassert(tuple_new,"");
      int idx = 0;
      for (auto arg = tuple_new->begin(); arg != tuple_new->end(); arg++) {
        if (tuple_new->numArgs() > 1) {
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
      if (idx >= cola_loop_vars.size() || !has_assigns) {
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
      // combine together into tuples and make the new scan function
      vector<types::Type*> scan_types;
      for (auto ai : all_iters) {
        scan_types.push_back(ai->getType());
      }
      auto *fscan = M->getOrRealizeFunc("scan", {M->getTupleType(scan_types)}, {}, "std.cola.block");
      seqassert(fscan,"");
      // now create the actual new iterator and var
      full_iter = util::call(fscan, {util::makeTuple(all_iters, M)});
      vector<Value*> _all_vars;
      for (Value *av : all_vars) {
        _all_vars.push_back(av);
      }
      Value *_full_var = _all_vars.size() == 1 ? _all_vars[0] : util::makeTuple(_all_vars, M);
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
    } else {
      // just add the loop var
      new_cola_loop_var_assigns.push_back(full_var);
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
    for (int i = 0; i < my_used_ctx.size(); i++) {
      auto *ctx = my_used_ctx[i];
      seqassert(ctx, "");
      auto *cctx = ctx->as<CallInstr>();
      seqassert(cctx, "flow {} foo {}", *cloned_flow, *ctx);
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
    flow->replaceAll(outer_flow);
  }
}



}
}
}
}
