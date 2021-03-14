#include "base.h"

#include "types/types.h"
#include "value.h"
#include "var.h"

namespace seq {
namespace ir {

int IdMixin::currentId = 0;

void IdMixin::resetId() { currentId = 0; }

const char Node::NodeId = 0;

int Node::replaceUsedValue(Value *old, Value *newValue) {
  return replaceUsedValue(old->getId(), newValue);
}

int Node::replaceUsedType(types::Type *old, types::Type *newType) {
  return replaceUsedType(old->getName(), newType);
}

int Node::replaceUsedVariable(Var *old, Var *newVar) {
  return replaceUsedVariable(old->getId(), newVar);
}

} // namespace ir
} // namespace seq
