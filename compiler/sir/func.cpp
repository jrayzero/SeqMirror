#include "func.h"

#include <algorithm>

#include "parser/common.h"

#include "util/iterators.h"
#include "util/operator.h"
#include "util/visitor.h"

#include "module.h"
#include "var.h"

namespace {

int findAndReplace(int id, seq::ir::Var *newVal, std::list<seq::ir::Var *> &values) {
  auto replacements = 0;
  for (auto &value : values) {
    if (value->getId() == id) {
      value = newVal;
      ++replacements;
    }
  }
  return replacements;
}

} // namespace

namespace seq {
namespace ir {

const char Func::NodeId = 0;

void Func::realize(types::Type *newType, const std::vector<std::string> &names) {
  auto *funcType = cast<types::FuncType>(newType);
  seqassert(funcType, "{} is not a function type", *newType);

  setType(funcType);
  args.clear();

  auto i = 0;
  for (auto *t : *funcType) {
    args.push_back(getModule()->Nr<Var>(t, false, names[i]));
    ++i;
  }
}

Var *Func::getArgVar(const std::string &n) {
  auto it = std::find_if(args.begin(), args.end(),
                         [n](auto *other) { return other->getName() == n; });
  return (it != args.end()) ? *it : nullptr;
}

std::vector<Var *> Func::doGetUsedVariables() const {
  std::vector<Var *> ret(args.begin(), args.end());
  return ret;
}

int Func::doReplaceUsedVariable(int id, Var *newVar) {
  return findAndReplace(id, newVar, args);
}

const char BodiedFunc::NodeId = 0;

int BodiedFunc::doReplaceUsedValue(int id, Value *newValue) {
  if (body && body->getId() == id) {
    auto *flow = cast<Flow>(newValue);
    seqassert(flow, "{} is not a flow", *newValue);
    body = flow;
    return 1;
  }
  return 0;
}

std::vector<Var *> BodiedFunc::doGetUsedVariables() const {
  auto ret = Func::doGetUsedVariables();
  ret.insert(ret.end(), symbols.begin(), symbols.end());
  return ret;
}

int BodiedFunc::doReplaceUsedVariable(int id, Var *newVar) {
  return Func::doReplaceUsedVariable(id, newVar) + findAndReplace(id, newVar, symbols);
}

const char ExternalFunc::NodeId = 0;

const char InternalFunc::NodeId = 0;

std::vector<types::Type *> InternalFunc::doGetUsedTypes() const {
  std::vector<types::Type *> ret;

  for (auto *t : Func::getUsedTypes())
    ret.push_back(const_cast<types::Type *>(t));

  if (parentType)
    ret.push_back(parentType);

  return ret;
}

int InternalFunc::doReplaceUsedType(const std::string &name, types::Type *newType) {
  auto count = Func::replaceUsedType(name, newType);
  if (parentType && parentType->getName() == name) {
    parentType = newType;
    ++count;
  }
  return count;
}

const char LLVMFunc::NodeId = 0;

std::vector<types::Type *> LLVMFunc::doGetUsedTypes() const {
  std::vector<types::Type *> ret;

  for (auto *t : Func::getUsedTypes())
    ret.push_back(const_cast<types::Type *>(t));

  for (auto &l : llvmLiterals)
    if (l.isType())
      ret.push_back(const_cast<types::Type *>(l.getTypeValue()));

  return ret;
}

int LLVMFunc::doReplaceUsedType(const std::string &name, types::Type *newType) {
  auto count = Var::doReplaceUsedType(name, newType);
  for (auto &l : llvmLiterals)
    if (l.isType() && l.getTypeValue()->getName() == name) {
      l = newType;
      ++count;
    }
  return count;
}

} // namespace ir
} // namespace seq
