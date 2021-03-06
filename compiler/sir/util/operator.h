#pragma once

#include <unordered_set>

#include "sir/sir.h"

#include "visitor.h"

#define LAMBDA_VISIT(x)                                                                \
  virtual void handle(seq::ir::x *v) {}                                                \
  void visit(seq::ir::x *v) override {                                                 \
    handle(v);                                                                         \
    processChildren(v);                                                                \
  }

namespace seq {
namespace ir {
namespace util {

/// Pass that visits all values in a module.
class Operator : public Visitor {
private:
  /// IDs of previously visited nodes
  std::unordered_set<int> seen;
  /// stack of IR nodes being visited
  std::vector<Node *> nodeStack;
  /// stack of iterators
  std::vector<decltype(SeriesFlow().begin())> itStack;

public:
  virtual ~Operator() noexcept = default;

  void visit(Module *m) override {
    nodeStack.push_back(m);
    nodeStack.push_back(m->getMainFunc());
    process(m->getMainFunc());
    nodeStack.pop_back();
    for (auto *s : *m) {
      nodeStack.push_back(s);
      process(s);
      nodeStack.pop_back();
    }
    nodeStack.pop_back();
  }

  void defaultVisit(Var *v) override {}
  void defaultVisit(Func *f) override {}

  void visit(BodiedFunc *f) override {
    seen.insert(f->getBody()->getId());
    process(f->getBody());
  }

  LAMBDA_VISIT(VarValue);
  LAMBDA_VISIT(PointerValue);

  void visit(seq::ir::SeriesFlow *v) override {
    nodeStack.push_back(v);
    for (auto it = v->begin(); it != v->end(); ++it) {
      itStack.push_back(it);
      process(*it);
      itStack.pop_back();
    }
    nodeStack.pop_back();
  }

  LAMBDA_VISIT(IfFlow);
  LAMBDA_VISIT(WhileFlow);
  LAMBDA_VISIT(ForFlow);
  LAMBDA_VISIT(TryCatchFlow);
  LAMBDA_VISIT(PipelineFlow);
  LAMBDA_VISIT(dsl::CustomFlow);

  LAMBDA_VISIT(TemplatedConst<int64_t>);
  LAMBDA_VISIT(TemplatedConst<double>);
  LAMBDA_VISIT(TemplatedConst<bool>);
  LAMBDA_VISIT(TemplatedConst<std::string>);
  LAMBDA_VISIT(dsl::CustomConst);

  LAMBDA_VISIT(Instr);
  LAMBDA_VISIT(AssignInstr);
  LAMBDA_VISIT(ExtractInstr);
  LAMBDA_VISIT(InsertInstr);
  LAMBDA_VISIT(CallInstr);
  LAMBDA_VISIT(StackAllocInstr);
  LAMBDA_VISIT(TypePropertyInstr);
  LAMBDA_VISIT(YieldInInstr);
  LAMBDA_VISIT(TernaryInstr);
  LAMBDA_VISIT(BreakInstr);
  LAMBDA_VISIT(ContinueInstr);
  LAMBDA_VISIT(ReturnInstr);
  LAMBDA_VISIT(YieldInstr);
  LAMBDA_VISIT(ThrowInstr);
  LAMBDA_VISIT(FlowInstr);
  LAMBDA_VISIT(dsl::CustomInstr);

  template <typename Node> void process(Node *v) { v->accept(*this); }

  /// Return the parent of the current node.
  /// @param level the number of levels up from the current node
  template <typename Desired = Node> Desired *getParent(int level = 0) {
    return cast<Desired>(nodeStack[nodeStack.size() - level - 1]);
  }
  /// @return current depth in the tree
  int depth() const { return nodeStack.size(); }

  /// @tparam Desired the desired type
  /// @return the last encountered example of the desired type
  template <typename Desired> Desired *findLast() {
    for (auto it = nodeStack.rbegin(); it != nodeStack.rend(); ++it) {
      if (auto *v = cast<Desired>(*it))
        return v;
    }
    return nullptr;
  }
  /// @return the last encountered function
  Func *getParentFunc() { return findLast<Func>(); }

  /// @param v the value
  /// @return whether we have visited ("seen") the given value
  bool saw(const Value *v) const { return seen.find(v->getId()) != seen.end(); }
  /// Avoid visiting the given value in the future.
  /// @param v the value
  void see(const Value *v) { seen.insert(v->getId()); }

  /// Inserts the new value before the current position in the last seen SeriesFlow.
  /// @param v the new value
  auto insertBefore(Value *v) {
    return findLast<SeriesFlow>()->insert(itStack.back(), v);
  }
  /// Inserts the new value after the current position in the last seen SeriesFlow.
  /// @param v the new value, which is marked seen
  auto insertAfter(Value *v) {
    auto newPos = itStack.back();
    ++newPos;
    see(v);

    return findLast<SeriesFlow>()->insert(newPos, v);
  }

private:
  void processChildren(Value *v) {
    nodeStack.push_back(v);
    for (auto *c : v->getUsedValues()) {
      if (saw(c))
        continue;
      see(c);
      process(c);
    }
    nodeStack.pop_back();
  }
};

} // namespace util
} // namespace ir
} // namespace seq

#undef LAMBDA_VISIT
