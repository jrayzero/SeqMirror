//
// Created by Jessica Ray on 2021-03-16.
//

#include "annotate.h"
#include "sir/util/cloning.h"
#include "sir/util/irtools.h"
#include "sir/util/matching.h"

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

void AnnotateDims::handle(seq::ir::CallInstr *instr) {
  auto *func = cast<BodiedFunc>(util::getFunc(instr->getCallee()));
  if (!func || func->getUnmangledName() != Module::INIT_MAGIC_NAME) {
    return;
  }
  auto *M = instr->getModule();
  string uname = func->getUnmangledName();
  Value *self = instr->front();
  types::Type *selfType = self->getType();
  COLA_TYPE ctype = get_cola_type(selfType);
  if (ctype != CNONE) {
    switch (ctype) {
      case MULTIDIM: {
        // see what type of init this is

      }
    }
  }
}

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
//void TraversalOpt::handle(seq::ir::CallInstr *instr) {
//    auto *func = cast<BodiedFunc>(util::getFunc(instr->getFunc()));
//  if (func) {
//    std::string fname = func->getName();
//    std::string cname = get_class_name(fname);
//    if (cname == "Traversal" && func->getUnmangledName() == "__iter__") {
//      // figure out what type of traversal this is. We need to track back
//      std::cerr << *instr << std::endl;
//      Value *self = instr->front();
//      for (auto i : self->getUsedValues()) {
//        std::cerr << *i << std::endl;
//      }
//    }
//  }
//}
//
////void TrackSpatialHierarchy::handle(seq::ir::CallInstr *instr) {
////  auto *func = cast<BodiedFunc>(util::getFunc(instr->getFunc()));
////  if (func) {
////    std::string fname = func->getName();
////    std::string cname = get_class_name(fname);
////    if (cname != "" && func->getUnmangledName() == "__init__") {
////
////      if (cname == "Block" || cname == "View" || cname == "Multidim") {
////        std::cerr << "found init" << std::endl;
////      }
////
////        // inits create either new buffers or new views depending on the type of init function
////        if (cname == "Block") {
////          // this is creating a new buffer
////          auto *h = new Hierarchy(instr->getId());
////          h->is_buffer = true;
////          this->root_stack.push_back(h);
////        }
////
////
////    } else if (cname != "" && func->getUnmangledName() == "__getitem__") {
////      // calls to getitem create new Views
////      if (cname == "Block" || cname == "View" || cname == "Multidim") {
////        std::cerr << "found getitem" << std::endl;
////      }
////    }
////  }
////  if (func && func->getUnmangledName() == "__getitem__") {
////    auto f = util::getStdlibFunc(instr->getFunc(), "__getitem__", "cola.block");
////    if (f) {
////      std::cerr << f->getUnmangledName() << std::endl;
////    }
//
////    ast::types::TypePtr lastType = func->getType()->getAstType()->follow();
//
////    if (lastType->getClass()->is("Block")) {
////      std::cerr << "Found block" << std::endl;
////    }
////    std::cerr << lastType << std::endl;
////    if (lastType == "Block") {
////
////    }
//
////}

}
}
}
}