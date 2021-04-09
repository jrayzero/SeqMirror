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
    types::Type *type;
    if (ctype == BLOCK) {
      type = M->getOrRealizeType("Block",
                                 {self->getType()->getGenerics()[0], self->getType()->getGenerics()[1]}, "std.cola.block");
    } else if (ctype == VIEW) {
      type = M->getOrRealizeType("View",
                                 {self->getType()->getGenerics()[0], self->getType()->getGenerics()[1]}, "std.cola.block");
    } else {
      return;
    }
    auto args = instr->begin();
    bool all_int = true;
    std::vector<Value*> int_args;
    auto tuple = args + 1;
    int ntup_generics = (*tuple)->getType()->getGenerics().size();
    // check that i,j,k (or however many items there are) are all ints
    for (int i = 0; i < ntup_generics; i++) {
      if (!(*tuple)->getType()->getGenerics()[i].getTypeValue()->is(M->getIntType())) {
        all_int = false;
        break;
      } else {
        auto *get_int = M->Nr<ExtractInstr>(*tuple, std::to_string(i));
        int_args.push_back(get_int);
      }
    }
    if (!all_int) {
      return;
    }
    // compute:
    // i0 = i + Y.base.buffer_mapping[0].start
    // j0 = j + Y.base.buffer_mapping[1].start
    // k0 = k + Y.base.buffer_mapping[2].start
    // at this point, we have all of the integer values, so we can manually compute the index updates
    auto *bm = M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(self, "base"), "buffer_mapping");
    auto *flow = M->Nr<SeriesFlow>();
    std::vector<VarValue*> mapped_starts;
    for (int i = 0; i < int_args.size(); i++) {
      Value *r = M->Nr<ExtractInstr>(bm, std::to_string(i));
      auto *adder = M->getOrRealizeMethod(M->getIntType(), Module::ADD_MAGIC_NAME, {M->getIntType(), M->getIntType()});
      assert(adder);
      auto *sum = util::call(adder, {r,int_args[i]});
      assert(sum);
      std::cerr << *sum << std::endl;
      auto *v = util::makeVar(sum, flow, cast<BodiedFunc>(getParentFunc()));
      mapped_starts.push_back(v);
    }
    // compute the linearization. just manually do it for the number of indices
    if (mapped_starts.size() == 3) {
      // idx = i0*d1*d2 + j0*d2 + k0
      auto *parent_dims =
        M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(self, "base"), "buffer_parent"), "dims");
      auto *d1 =  M->Nr<ExtractInstr>(parent_dims, "1");
      auto *d2 =  M->Nr<ExtractInstr>(parent_dims, "2");
      assert(d1);
      assert(d2);
      auto *adder = M->getOrRealizeMethod(M->getIntType(), Module::ADD_MAGIC_NAME, {M->getIntType(), M->getIntType()});
      auto *mult = M->getOrRealizeMethod(M->getIntType(), Module::MUL_MAGIC_NAME, {M->getIntType(), M->getIntType()});
      auto *c0 = util::makeVar(util::call(mult, {d2, mapped_starts[1]}), flow, cast<BodiedFunc>(getParentFunc()));
      std::cerr << "c0 " << *c0 << std::endl;
      auto *c1 = util::makeVar(util::call(adder, {c0, mapped_starts[2]}), flow, cast<BodiedFunc>(getParentFunc()));
      std::cerr << "c1 " << *c1 << std::endl;
      auto *c2 = util::makeVar(util::call(mult, {mapped_starts[0], d1}), flow, cast<BodiedFunc>(getParentFunc()));
      std::cerr << "c2 " << *c2 << std::endl;
      auto *c3 = util::makeVar(util::call(mult, {c2, d2}), flow, cast<BodiedFunc>(getParentFunc()));
      std::cerr << "c3 " << *c3 << std::endl;
      auto *idx = util::makeVar(util::call(adder, {c3, c2}), flow, cast<BodiedFunc>(getParentFunc()));
      std::cerr << "idx " << *idx << std::endl;
      auto *vidx = util::makeVar(idx, flow, cast<BodiedFunc>(getParentFunc()));
      // now get the buffer and write to that
      // Y.base.buffer[idx]
      auto *buffer = M->Nr<ExtractInstr>(M->Nr<ExtractInstr>(self, "base"), "buffer");
      // and use this buffer instead of the origin lhs
      std::cerr << "my flow " << *flow << std::endl;
      // make a __setitem__ intstruction
      auto *new_assign = M->getOrRealizeMethod(buffer->getType(), "__setitem__",
                                               {buffer->getType(), M->getIntType(), (*(args+2))->getType()});
      instr->replaceAll(util::call(new_assign, {buffer, vidx, *(args+2)}));
      std::cerr << "instr " << *instr << std::endl;
      // instr->replaceAll(flow)
    } else {
      seqassert(false, "not implemented yet");
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
