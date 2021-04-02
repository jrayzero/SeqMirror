//
// Created by Jessica Ray on 2021-03-21.
//

#include "print_func_ir.h"
#include "sir/util/irtools.h"
#include "sir/util/cloning.h"
#include "util/common.h"

namespace seq {
namespace ir {
namespace transform {
namespace cola {

using namespace std;

void PrintFuncs::handle(seq::ir::CallInstr *instr) {
  auto f = instr->getCallee();
  if (cast<BodiedFunc>(util::getFunc(f))) {
    string fname = cast<BodiedFunc>(util::getFunc(f))->getUnmangledName();
    if (fname == "sse" || fname == "sad" || fname == "satd_4x4") {
      std::cerr << fname << std::endl;
      std::cerr << *cast<BodiedFunc>(util::getFunc(f)) << std::endl;
    }
  }
}

//bool is_next_function(Value *v) {
//  if (v->is<CallInstr>()) {
//    auto f = v->as<CallInstr>()->getCallee();
//    string fname = cast<BodiedFunc>(util::getFunc(f))->getUnmangledName();
//    return fname == "next" || fname == "__next__";
//  }
//  return false;
//}
//
//bool is_iter_function(Value *v) {
//  if (v->is<CallInstr>()) {
//    auto f = v->as<CallInstr>()->getCallee();
//    string fname = cast<BodiedFunc>(util::getFunc(f))->getUnmangledName();
//    return fname == "iter" || fname == "__iter__";
//  }
//  return false;
//}
//
//bool is_scan_function(Value *v) {
//  if (v->is<CallInstr>()) {
//    auto f = v->as<CallInstr>()->getCallee();
//    string fname = cast<BodiedFunc>(util::getFunc(f))->getUnmangledName();
//    return fname == "scan";
//  }
//  return false;
//}
//
//// take a fully qualified name like std.cola.block.Block[int] and pull out the last part w/o generics (Block)
//string strip_type_name(string fq_name) {
//  string last = ast::split(fq_name, '.').back();
//  return ast::split(last, '[')[0];
//}
//
//std::string get_class_name(std::string name) {
//  auto spl = ast::split(name, '.');
//  if (spl.size() >= 2) {
//    if (std::isdigit(spl.back()[0])) { // it's a method with multiple overloads
//      if (spl.size() >= 3) {
//        return spl[spl.size() - 3];
//      } else {
//        return "";
//      }
//    } else {
//      return spl[spl.size() - 2];
//    }
//  } else {
//    return "";
//  }
//}
//
//void RemoveGenerators::handle(seq::ir::ForFlow *flow) {
//  auto *M = flow->getModule();
//  if (is_iter_function(flow->getIter())) {
//    // in this case, there is just one item
//
//    // for b in blk.__iter__():
//    //    b[0] = some_value
//    // ->
//    // b_temp = blk.flatten()
//    // b_units = b_temp.nunits()
//    // b_idx = 0
//    // for _b in range(b_units):
//    //   b = b_temp[b_idx]
//    //   b_idx += 1
//    //   <body>
//
//    // see if it is a block or view
//    auto c = flow->getIter()->as<CallInstr>();
//
//    auto *func = cast<BodiedFunc>(util::getFunc(c->getCallee()));
//
////    std::cerr << func->getUnmangledName() << std::endl;
////    std::string type_name = get_class_name(func->getName());
//    // this is so hacky
//    auto spl = ast::split(func->getName(), '.');
//    int i = 0;
//    for (auto &s : spl) {
//      s = ast::split(s, '[')[0];
//      if (s == "__iter__" || s == "iter") {
//        break;
//      }
//      i++;
//    }
//    seqassert(i > 0, "didn't find iter");
//    string type_name = spl[i-1];
//
//    auto arg = c->front();
//    if (type_name == "Block" || type_name == "View") {
//
//      std::cerr << "Flow before" << std::endl;
//      std::cerr << *flow << std::endl;
//
//      auto *parent = cast<BodiedFunc>(getParentFunc());
//
//      // flow will get replaced with
//      auto *series = M->Nr<SeriesFlow>();
//      // body for the new flow
//      auto *body = M->Nr<SeriesFlow>();
//
//      // flatten
//      auto *flatten_func = M->getOrRealizeMethod(arg->getType(), "flatten", {arg->getType()});
//      seqassert(flatten_func, "flatten not found");
//      auto *flatten = util::call(flatten_func, {arg});
//      auto *temp = util::makeVar(flatten, series, parent);
//
//      // nunits
//      auto *nunits_func = M->getOrRealizeMethod(arg->getType(), "nunits", {arg->getType()});
//      seqassert(nunits_func, "nunits not found");
//      auto *compute_nunits = util::call(nunits_func, {arg});
//      auto *nunits = util::makeVar(compute_nunits, series, parent);
//
//      // index
//      Value *start = M->getInt(0);
//      auto idx = util::makeVar(start, series, parent);
//
//      // make the pieces for a new ForFlow
//
//      // new iter and var
//      auto *range_t = M->getType("std.internal.types.range.range");
//      seqassert(range_t, "range type not found");
//      auto *range_func = M->getOrRealizeMethod(range_t, "__new__", {M->getIntType()});
//      seqassert(range_func, "range method not found");
//      auto *range_build = util::call(range_func, {nunits});
//      auto *range_iter = M->getOrRealizeMethod(range_t, "__iter__", {range_t});
//      seqassert(range_iter, "range iter not found");
//      auto range = util::call(range_iter, {range_build});
//
//      // slicing
//      auto *slice_func = M->getOrRealizeMethod(temp->getType(), "__getitem__", {temp->getType(), M->getIntType()});
//      seqassert(slice_func, "slice function not found");
//      auto *slice = util::call(slice_func, {temp, idx});
//      auto *new_var = util::makeVar(slice, body, parent);
//      flow->getVar()->replaceAll(new_var->getVar());
//
//      // incr
//      auto *do_add = M->Nr<AssignInstr>(idx->getVar(), *idx + *M->getInt(1));
//      body->push_back(do_add);
//
//      util::CloneVisitor cv(M);
//      body->push_back(cv.clone(flow->getBody()));
//
//      auto *var = util::makeVar(M->getInt(0), series, parent);
//      auto *fflow = M->Nr<ForFlow>(range, body, var->getVar());
//      series->push_back(fflow);
//
//      flow->replaceAll(series);
//
//      std::cerr << "Flow after" << std::endl;
//      std::cerr << *flow << std::endl;
//
//    }
//  }
//}

}
}
}
}