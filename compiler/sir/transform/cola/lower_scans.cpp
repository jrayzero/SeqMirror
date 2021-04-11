//
// Created by Jessica Ray on 2021-03-16.
//

#include "lower_scans.h"
#include "sir/util/cloning.h"
#include "sir/util/irtools.h"
#include "sir/util/matching.h"
#include "sir/analyze/dataflow/reaching.h"
#include "sir/transform/manager.h"
namespace seq {
namespace ir {
namespace transform {
namespace cola {

enum COLA_TYPE {
  BLOCK,
  VIEW,
  MULTIDIM,
  CNONE,
};


COLA_TYPE get_cola_type(types::Type *type) {
  // remove qualified path
  string stype = ast::split(type->getName(), '.').back();
  // remove any generics
  stype = ast::split(stype, '[')[0];
  if (stype == "Block") {
    return BLOCK;
  } else if (stype == "View") {
    return VIEW;
  } else if (stype == "Multidim") {
    return MULTIDIM;
  } else {
    return CNONE;
  }  
}

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
  void MarkCandidateLoops::handle(ForFlow *flow) {
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
      int body_skip = 0;
      if (fname == "iter") {
	auto *arg = call->front();
	if (is_block_type(arg->getType(),M)) {	  
	  cola_loop_vars.push_back(cloned_flow->getVar());
	  cola_loop_iters.push_back(arg);
	  cola_loop_idxs.push_back(0);
	}
      } else if (fname == "zip") {
	// TODO REMOVE
	return;
	auto body_it = cloned_flow->getBody()->as<SeriesFlow>()->begin();
	// zip gets a tuple, so should be a call to tuple new
	auto *tuple = call->front();
	auto *tuple_new = tuple->as<CallInstr>();
	seqassert(tuple_new,"");
	int idx = 0;
	for (auto arg = tuple_new->begin(); arg != tuple_new->end(); arg++) {
	  seqassert(body_it != cloned_flow->getBody()->as<SeriesFlow>()->end(), "");	  
	  if (is_block_type((*arg)->getType(),M)) {
	    seqassert((*body_it)->as<AssignInstr>(),"");
	    auto *assign_lhs = (*body_it)->as<AssignInstr>()->getLhs();
	    cola_loop_vars.push_back(assign_lhs);
	    cola_loop_iters.push_back(*arg);
	    cola_loop_idxs.push_back(idx);
	  }
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
	auto new_for_flow = M->Nr<ForFlow>(full_iter, new_body, full_var->getVar());
	
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

      }
      flow->replaceAll(outer_flow);
      std::cerr << *flow << std::endl;
    }
  }

    
void ModifyUnitWrites::handle(CallInstr *instr) {
  auto M = instr->getModule();

// Ex: Y[i,j,k] = rhs
  auto *f = util::getFunc(instr->getCallee());
  string fname = f->getUnmangledName();
  if (fname == "__setitem__") {
    auto *_self = instr->front();
    util::CloneVisitor cv(M);
    auto self = cv.clone(_self);
    COLA_TYPE ctype = get_cola_type(self->getType());
    int ndims = 0;
    if (ctype == BLOCK || ctype == VIEW) {
      ndims = self->getType()->getGenerics()[1].getStaticValue();
    } else {
      return;
    }
// TODO for 1d access, it won't be a tuple, just a single int
    auto args = instr->begin();
    bool all_int = true;
    std::vector<Value*> int_args;
    auto _tuple = args + 1;
    auto tuple = (*_tuple)->as<CallInstr>();
    if (!tuple) {
      // see if the type is UNIT, which implies an all 0 tuple
      auto *unit = M->getOrRealizeType("_UNIT", {}, "std.cola.block");
      seqassert(unit, "unit not found");
      if ((*_tuple)->getType()->is(unit)) {
        seqassert(ndims == 3, "only 3 dims supported currently");
        for (int i = 0; i < ndims; i++) {
          int_args.push_back(M->getInt(0));
        }
      } else {
        return; // some type of access I don't support
      }
    }
    // check that i,j,k (or hdifowever many items there are) are all ints
    if (int_args.empty()) {
      for (auto it = tuple->begin(); it < tuple->end(); it++) {
        if ((*it)->getType()->is(M->getIntType())) {
          int_args.push_back(*it);
        } else {
          all_int = false;
          break;
        }
      }
      if (!all_int) {
        return;
      }
    } // else we filled it from the UNIT
    // compute:
    // i0 = i + Y.base.buffer_mapping[0].start
    // j0 = j + Y.base.buffer_mapping[1].start
    // k0 = k + Y.base.buffer_mapping[2].start
    // at this point, we have all of the integer values, so we can manually compute the index updates
    auto *bm = M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(self, "base"), "buffer_mapping");
    std::vector<Value*> mapped_starts;
    for (int i = 0; i < int_args.size(); i++) {
      Value *r = M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(bm, "item" + std::to_string(i+1)), "start");
      auto *adder = M->getOrRealizeMethod(M->getIntType(), Module::ADD_MAGIC_NAME, {M->getIntType(), M->getIntType()});
      assert(adder);
      auto *sum = util::call(adder, {r,int_args[i]});
      mapped_starts.push_back(sum);
    }
    // compute the linearization. just manually do it for the number of indices
    if (mapped_starts.size() == 3) {
      // idx = i0*d1*d2 + j0*d2 + k0
      auto *parent_dims =
        M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(self, "base"), "buffer_parent"), "dims");
      auto *foozle = M->getOrRealizeFunc("foozle",
                                         {M->getIntType(),M->getIntType(),M->getIntType(),
                                          M->getIntType(),M->getIntType()},{},"std.cola.block");
      assert(foozle);
      // now get the buffer and write to that
      // Y.base.buffer[idx]
      auto *idx = util::call(foozle, {
        mapped_starts[0],
        mapped_starts[1], mapped_starts[2],
        M->Nr<ExtractInstr>(parent_dims, "item2"), M->Nr<ExtractInstr>(parent_dims, "item3")});
      auto *buffer = M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(self, "base"), "buffer");
      // and use this buffer instead of the origin lhs
      // make a __setitem__ intstruction
      auto *new_assign = M->getOrRealizeMethod(buffer->getType(), "__setitem__",
                                               {buffer->getType(), M->getIntType(), (*(args+2))->getType()});
      auto *do_setitem = util::call(new_assign, {buffer, idx, *(args+2)});

      instr->replaceAll(do_setitem);
    } else {
//      seqassert(false, "not implemented yet");
// If the number of access dims does not equal the ndims in the generics, there are fixed dims. I can add on the zeros
// automatically, but I need to preserve the check in the runtime code that the fixed dims + naccess dims matches
// the generic
    }

  }

}



// attribute storing possible dimensionalities

struct DimensionAttribute : public Attribute {
  static const std::string AttributeName;

  // track possible numbers of dims based on reaching analysis
  std::map<std::string, std::set<int>> attributes;

  DimensionAttribute() = default;
  explicit DimensionAttribute(std::map<std::string, set<int>> attributes)
    : attributes(std::move(attributes)) {}

  bool has(const std::string &key) const { return attributes.find(key) != attributes.end(); }

  std::set<int> get(const std::string &key) const {
    auto it = attributes.find(key);
    return it != attributes.end() ? it->second : std::set<int>{};
  }

  static std::unique_ptr<DimensionAttribute> make(int ndims) {
    std::map<string, set<int>> m = {};
    m["dims"] = {ndims};
    return std::make_unique<DimensionAttribute>(m);
  }

  static std::unique_ptr<DimensionAttribute> make(set<int> ndims) {
    std::map<string, set<int>> m = {};
    m["dims"] = ndims;
    return std::make_unique<DimensionAttribute>(m);
  }

private:
  std::ostream &doFormat(std::ostream &os) const override {
    std::vector<std::string> keys;
    for (auto &val : attributes)
      keys.push_back(val.first);
    fmt::print(os, FMT_STRING("{}"), fmt::join(keys.begin(), keys.end(), ","));
    return os;
  }

  Attribute *doClone() const override { std::cerr << "CLONING ATTR" << std::endl; return new DimensionAttribute(*this); }

};

LowerScans::LowerScans(string key) : key(move(key)) { }


const std::string DimensionAttribute::AttributeName = "DimensionAttribute";

//// what happens if calls are nested, like Block(Block(100))? Will these get separated out ?
//void ExtractMultidimValueFromInitCall::handle(seq::ir::CallInstr *instr) {
//  auto *func = cast<BodiedFunc>(util::getFunc(instr->getCallee()));
//  if (!func || func->getUnmangledName() != Module::INIT_MAGIC_NAME) {
//    return;
//  } else {
//    string uname = func->getUnmangledName();
//    Value *self = instr->front();
//    types::Type *selfType = self->getType();
//    COLA_TYPE ctype = get_cola_type(selfType);
//    if (ctype == MULTIDIM) {
//      md_value = self;
//    }
//  }
//}
//
//void ExtractMultidimValueFromInitCall::handle(seq::ir::ExtractInstr *instr) {
//  if (get_cola_type(instr->getVal()->getType()) == MULTIDIM) {
//    md_value = instr->getVal();
//  } else {
//    return;
//  }
//}
//
void LowerScans::handle(seq::ir::ForFlow *flow) {

  auto module = flow->getModule();
  auto iter = flow->getIter();
  auto slice_type = module->getOrRealizeType("Slice", {}, "std.internal.types.slice");
  if (!slice_type) {
    compilationError("Couldn't find slice type");
  }
  if (iter->is<CallInstr>()) {
    auto *call = iter->as<CallInstr>();
    auto *f = util::getFunc(call->getCallee());
    string fname = f->getUnmangledName();
    if (fname == "scan") {
      int ndims = -1;
      // input to scan is a tuple containing all the things to scan
      // this should be a call to Tuple.__new__
      auto *tup_new = call->front()->as<CallInstr>();
      if (!tup_new) {
        compilationError("should be tuple.__new__");
      }
      for (auto tup_arg = tup_new->begin(); tup_arg != tup_new->end(); tup_arg++) {
        COLA_TYPE ctype = get_cola_type((*tup_arg)->getType());
        if (ctype != BLOCK && ctype != VIEW) {
          std::cerr << "ERROR" << std::endl;
          compilationError("scan only takes blocks or views");
        }

        int assigned_ndims = (*tup_arg)->getType()->getGenerics()[1].getStaticValue();
        std::cerr << assigned_ndims << std::endl;
        if (ndims == -1) {
          ndims = assigned_ndims;
        } else {
          if (ndims != assigned_ndims) {
            compilationError("scan items must have same dimensionality");
          }
        }
      }

      std::cerr << "NDIMS " << ndims << std::endl;
      auto *sflow = module->Nr<SeriesFlow>();
      // gather int types
      vector<types::Type *> min_types;
      for (int level = 0; level < tup_new->numArgs(); level++) {
        min_types.push_back(module->getIntType());
      }
      std::cerr << "Flow before " << std::endl;
      std::cerr << *flow << std::endl;
      std::vector<Value *> bounds;
      // compute all the bound expressions
      for (int level = 0; level < ndims; level++) {
        vector<Value *> min_args;
        for (auto tup_arg = tup_new->begin(); tup_arg != tup_new->end(); tup_arg++) {
          auto *buffer_mapping = module->Nr<ExtractInstr>(module->Nr<ExtractInstr>(*tup_arg, "base"), "buffer_mapping");
          // access the buffer_mapping tuple to get the current entry
          auto *get_bm_entry = module->Nr<ExtractInstr>(buffer_mapping, "item" + std::to_string(level+1));
          // now compute the range
          auto *frange = module->getOrRealizeMethod(slice_type, "compute_range", {slice_type}, {});
          if (!frange) {
            compilationError("Couldn't find compute_range");
          }
          auto range = util::call(frange, {get_bm_entry});
          min_args.push_back(range);
        }
        auto *fmin = module->getOrRealizeFunc("do_min", {module->getTupleType(min_types)}, {}, "std.cola.block");
        if (!fmin) {
          compilationError("couldn't find min");
        }
        auto *ttup = util::makeTuple(min_args, module);
        auto *do_min = util::call(fmin, {ttup});
        auto *bound = util::makeVar(do_min, sflow, cast<BodiedFunc>(getParentFunc()));
        bounds.push_back(bound);
      }
      // now we know the loop bounds, create the loop nests
      Value *outermost_loop = nullptr;
      Value *parent_loop = nullptr;
      std::vector<VarValue *> loop_vars;
      for (int level = 0; level < ndims; level++) {
        auto *fflow = module->Nr<ForFlow>(nullptr, nullptr, nullptr);
        auto *frange = module->getOrRealizeFunc("do_range", {module->getIntType(),module->getIntType(),module->getIntType()}, {}, "std.cola.block");
        if (!frange) {
          compilationError("couldn't find range");
        }
        auto *do_range = util::call(frange, {module->getInt(0), bounds[level], module->getInt(1)});
        fflow->setIter(do_range);
        auto *loop_var = util::makeVar(module->getInt(0), sflow, cast<BodiedFunc>(getParentFunc()));
        loop_vars.push_back(loop_var);
        fflow->setVar(loop_var->getVar());
        auto *body = module->Nr<SeriesFlow>();
        fflow->setBody(body);
        if (!parent_loop) {
          parent_loop = fflow;
          outermost_loop = fflow;
        } else {
          parent_loop->as<ForFlow>()->getBody()->as<SeriesFlow>()->push_back(fflow);
          parent_loop = fflow;
        }
      }
      sflow->push_back(outermost_loop);
      // we have a perfectly nested loop, let's make the inner body
      auto *body = parent_loop->as<ForFlow>()->getBody()->as<SeriesFlow>();
      // im1 = ~_m1.buffer_mapping[0].start + i * ~_m1.buffer_mapping[0].step
      // compute the appropriate buffer_mappings
      auto orig_var = flow->getVar();
      vector<VarValue *> orig_var_replacements;
      for (auto tup_arg = tup_new->begin(); tup_arg != tup_new->end(); tup_arg++) {
        std::vector<Value *> mapped_idxs;
        for (int level = 0; level < ndims; level++) {
          auto *buffer_mapping = module->Nr<ExtractInstr>(module->Nr<ExtractInstr>(*tup_arg, "base"), "buffer_mapping");
          auto *get_bm_entry = module->Nr<ExtractInstr>(buffer_mapping, "item" + std::to_string(level+1));
          auto *get_opt_start = module->Nr<ExtractInstr>(get_bm_entry, "start");
          auto *get_opt_step = module->Nr<ExtractInstr>(get_bm_entry, "step");
          auto *opt_type = slice_type->as<types::RecordType>()->getMemberType("start");
          auto *fopt_get = module->getOrRealizeMethod(opt_type, Module::INVERT_MAGIC_NAME, {opt_type});
          if (!fopt_get) {
            compilationError("Couldn't find ~optional");
          }
          auto *get_start = util::call(fopt_get, {get_opt_start});
          auto *get_step = util::call(fopt_get, {get_opt_step});
          Value *loop_var = loop_vars[level];
          auto *fmul = module->getOrRealizeMethod(module->getIntType(), Module::MUL_MAGIC_NAME, {module->getIntType(), module->getIntType()});
          auto *fadd = module->getOrRealizeMethod(module->getIntType(), Module::ADD_MAGIC_NAME, {module->getIntType(), module->getIntType()});
          if (!fmul || !fadd) {
            compilationError("Couldn't find mul or add");
          }
          auto mapped_idx = util::makeVar(util::call(fadd, {get_start, util::call(fmul,{loop_var, get_step})}), body, cast<BodiedFunc>(getParentFunc()));
          mapped_idxs.push_back(mapped_idx);
        }
        // make the new version of the View loop var
        // get the multidim type first
        auto *md_type = module->getOrRealizeType("Multidim", {(*tup_arg)->getType()->getGenerics()[0],(*tup_arg)->getType()->getGenerics()[1]}, "std.cola.block");
        if (!md_type) {
          compilationError("Couldn't find multidim type");
        }
        vector<types::Type *> getview_types;
        getview_types.push_back(md_type);
        vector<types::Type *> idx_types;
        for (int i = 0; i < ndims; i++) {
          idx_types.push_back(module->getIntType());
        }
        getview_types.push_back(module->getTupleType(idx_types));
        auto *fdo_getview = module->getOrRealizeFunc("do_getview", getview_types,
                                                     {(*tup_arg)->getType()->getGenerics()[0],(*tup_arg)->getType()->getGenerics()[1]}, "std.cola.block");
        if (!fdo_getview) {
          compilationError("couldn't find do_getview");
        }

        auto *view = util::makeVar(util::call(fdo_getview, {module->Nr<ExtractInstr>(*tup_arg, "base"),
                                                            util::makeTuple(mapped_idxs, module)}), body, cast<BodiedFunc>(getParentFunc()));
        orig_var_replacements.push_back(view);
      }
      // replace all the original loop vars
      if (tup_new->numArgs() == 1) {
        // not a tuple
        orig_var->replaceAll(orig_var_replacements[0]->getVar());
      } else {
        vector<Value *> tmp;
        for (auto vv : orig_var_replacements) {
          tmp.push_back(vv);
        }
        orig_var->replaceAll(util::makeVar(util::makeTuple(tmp, module), body, cast<BodiedFunc>(getParentFunc()))->getVar());
      }
      // plop in the old body
      body->push_back(flow->getBody());
      flow->replaceAll(sflow);
      std::cerr << "Flow after " << std::endl;
      std::cerr << *flow << std::endl;
    }
  }
}

}
}
}
}
