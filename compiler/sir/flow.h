#pragma once

#include <list>
#include <vector>

#include "base.h"
#include "util/iterators.h"
#include "value.h"
#include "var.h"

namespace seq {
namespace ir {

/// Base for flows, which represent control flow.
class Flow : public AcceptorExtend<Flow, Value> {
public:
  static const char NodeId;

  using AcceptorExtend::AcceptorExtend;

  const types::Type *getType() const override;

  virtual ~Flow() noexcept = default;

  /// @return a clone of the value
  std::unique_ptr<Flow> clone() const {
    return std::unique_ptr<Flow>(static_cast<Flow *>(doClone()));
  }
};

using FlowPtr = std::unique_ptr<Flow>;

/// Flow that contains a series of flows or instructions.
class SeriesFlow : public AcceptorExtend<SeriesFlow, Flow> {
private:
  std::list<ValuePtr> series;

public:
  static const char NodeId;

  using AcceptorExtend::AcceptorExtend;

  /// @return an iterator to the first instruction/flow
  auto begin() { return util::raw_ptr_adaptor(series.begin()); }
  /// @return an iterator beyond the last instruction/flow
  auto end() { return util::raw_ptr_adaptor(series.end()); }
  /// @return an iterator to the first instruction/flow
  auto begin() const { return util::const_raw_ptr_adaptor(series.begin()); }
  /// @return an iterator beyond the last instruction/flow
  auto end() const { return util::const_raw_ptr_adaptor(series.end()); }

  /// @return a pointer to the first instruction/flow
  Value *front() { return series.front().get(); }
  /// @return a pointer to the last instruction/flow
  Value *back() { return series.back().get(); }
  /// @return a pointer to the first instruction/flow
  const Value *front() const { return series.front().get(); }
  /// @return a pointer to the last instruction/flow
  const Value *back() const { return series.back().get(); }

  /// Inserts an instruction/flow at the given position.
  /// @param pos the position
  /// @param v the flow or instruction
  /// @return an iterator to the newly added instruction/flow
  template <typename It> auto insert(It pos, ValuePtr v) {
    return util::raw_ptr_adaptor(series.insert(pos.internal, std::move(v)));
  }
  /// Appends an instruction/flow.
  /// @param f the flow or instruction
  void push_back(ValuePtr f) { series.push_back(std::move(f)); }

  /// Erases the item at the supplied position.
  /// @param pos the position
  /// @return the iterator beyond the removed flow or instruction
  template <typename It> auto erase(It pos) {
    return util::raw_ptr_adaptor(series.erase(pos.internal));
  }

private:
  std::ostream &doFormat(std::ostream &os) const override;

  Value *doClone() const override;
};

/// Flow representing a while loop.
class WhileFlow : public AcceptorExtend<WhileFlow, Flow> {
private:
  /// the condition
  ValuePtr cond;
  /// the body
  FlowPtr body;

public:
  static const char NodeId;

  /// Constructs a while loop.
  /// @param cond the condition
  /// @param body the body
  /// @param name the flow's name
  WhileFlow(ValuePtr cond, FlowPtr body, std::string name = "")
      : AcceptorExtend(std::move(name)), cond(std::move(cond)), body(std::move(body)) {}

  /// @return the condition
  Value *getCond() { return cond.get(); }
  /// @return the condition
  const Value *getCond() const { return cond.get(); }
  /// Sets the condition.
  /// @param c the new condition
  void setCond(ValuePtr c) { cond = std::move(c); }

  /// @return the body
  Flow *getBody() { return body.get(); }
  /// @return the body
  const Flow *getBody() const { return body.get(); }
  /// Sets the body.
  /// @param f the new value
  void setBody(FlowPtr f) { body = std::move(f); }

private:
  std::ostream &doFormat(std::ostream &os) const override;

  Value *doClone() const override;
};

/// Flow representing a for loop.
class ForFlow : public AcceptorExtend<ForFlow, Flow> {
private:
  /// the iterator
  ValuePtr iter;
  /// the body
  FlowPtr body;

  /// the variable
  Var *var;

public:
  static const char NodeId;

  /// Constructs a for loop.
  /// @param setup the setup
  /// @param iter the iterator
  /// @param body the body
  /// @param update the update
  /// @param name the flow's name
  ForFlow(ValuePtr iter, FlowPtr body, Var *var, std::string name = "")
      : AcceptorExtend(std::move(name)), iter(std::move(iter)), body(std::move(body)),
        var(var) {}

  /// @return the iter
  Value *getIter() { return iter.get(); }
  /// @return the iter
  const Value *getIter() const { return iter.get(); }
  /// Sets the iter.
  /// @param f the new iter
  void setIter(ValuePtr f) { iter = std::move(f); }

  /// @return the body
  Flow *getBody() { return body.get(); }
  /// @return the body
  const Flow *getBody() const { return body.get(); }
  /// Sets the body.
  /// @param f the new body
  void setBody(FlowPtr f) { body = std::move(f); }

  /// @return the var
  Var *getVar() { return var; }
  /// @return the var
  const Var *getVar() const { return var; }
  /// Sets the var.
  /// @param c the new var
  void setVar(Var *c) { var = c; }

private:
  std::ostream &doFormat(std::ostream &os) const override;

  Value *doClone() const override;
};

/// Flow representing an if statement.
class IfFlow : public AcceptorExtend<IfFlow, Flow> {
private:
  /// the condition
  ValuePtr cond;
  /// the true branch
  FlowPtr trueBranch;
  /// the false branch
  FlowPtr falseBranch;

public:
  static const char NodeId;

  /// Constructs an if.
  /// @param cond the condition
  /// @param trueBranch the true branch
  /// @param falseBranch the false branch
  /// @param name the flow's name
  IfFlow(ValuePtr cond, FlowPtr trueBranch, FlowPtr falseBranch = nullptr,
         std::string name = "")
      : AcceptorExtend(std::move(name)), cond(std::move(cond)),
        trueBranch(std::move(trueBranch)), falseBranch(std::move(falseBranch)) {}

  /// @return the true branch
  Flow *getTrueBranch() { return trueBranch.get(); }
  /// @return the true branch
  const Flow *getTrueBranch() const { return trueBranch.get(); }
  /// Sets the true branch.
  /// @param f the new true branch
  void setTrueBranch(FlowPtr f) { trueBranch = std::move(f); }

  /// @return the false branch
  Flow *getFalseBranch() { return falseBranch.get(); }
  /// @return the false branch
  const Flow *getFalseBranch() const { return falseBranch.get(); }
  /// Sets the false.
  /// @param f the new false
  void setFalseBranch(FlowPtr f) { falseBranch = std::move(f); }

  /// @return the condition
  Value *getCond() { return cond.get(); }
  /// @return the condition
  const Value *getCond() const { return cond.get(); }
  /// Sets the condition.
  /// @param c the new condition
  void setCond(ValuePtr c) { cond = std::move(c); }

private:
  std::ostream &doFormat(std::ostream &os) const override;

  Value *doClone() const override;
};

/// Flow representing a try-catch statement.
class TryCatchFlow : public AcceptorExtend<TryCatchFlow, Flow> {
public:
  /// Class representing a catch clause.
  class Catch {
  private:
    /// the handler
    FlowPtr handler;
    /// the catch type, may be nullptr
    const types::Type *type;
    /// the catch variable, may be nullptr
    Var *catchVar;

  public:
    explicit Catch(FlowPtr handler, const types::Type *type = nullptr,
                   Var *catchVar = nullptr)
        : handler(std::move(handler)), type(type), catchVar(catchVar) {}

    /// @return the handler
    Flow *getHandler() { return handler.get(); }
    /// @return the handler
    const Flow *getHandler() const { return handler.get(); }
    /// Sets the handler.
    /// @param h the new value
    void setHandler(FlowPtr h) { handler = std::move(h); }

    /// @return the catch type, may be nullptr
    const types::Type *getType() const { return type; }
    /// Sets the catch type.
    /// @param t the new type, nullptr for catch all
    void setType(types::Type *t) { type = t; }

    /// @return the variable, may be nullptr
    Var *getVar() { return catchVar; }
    /// @return the variable, may be nullptr
    const Var *getVar() const { return catchVar; }
    /// Sets the variable.
    /// @param v the new value, may be nullptr
    void setVar(Var *v) { catchVar = v; }
  };

private:
  /// the catch clauses
  std::list<Catch> catches;

  /// the body
  FlowPtr body;
  /// the finally, may be nullptr
  FlowPtr finally;

public:
  static const char NodeId;

  /// Constructs an try-catch.
  /// @param name the's name
  /// @param body the body
  /// @param finally the finally
  explicit TryCatchFlow(FlowPtr body, FlowPtr finally = nullptr, std::string name = "")
      : AcceptorExtend(std::move(name)), body(std::move(body)),
        finally(std::move(finally)) {}

  /// @return the body
  Flow *getBody() { return body.get(); }
  /// @return the body
  const Flow *getBody() const { return body.get(); }
  /// Sets the body.
  /// @param f the new
  void setBody(FlowPtr f) { body = std::move(f); }

  /// @return the finally
  Flow *getFinally() { return finally.get(); }
  /// @return the finally
  const Flow *getFinally() const { return finally.get(); }
  /// Sets the finally.
  /// @param f the new
  void setFinally(FlowPtr f) { finally = std::move(f); }

  /// @return an iterator to the first catch
  auto begin() { return catches.begin(); }
  /// @return an iterator beyond the last catch
  auto end() { return catches.end(); }
  /// @return an iterator to the first catch
  auto begin() const { return catches.begin(); }
  /// @return an iterator beyond the last catch
  auto end() const { return catches.end(); }

  /// @return a reference to the first catch
  auto &front() { return catches.front(); }
  /// @return a reference to the last catch
  auto &back() { return catches.back(); }
  /// @return a reference to the first catch
  auto &front() const { return catches.front(); }
  /// @return a reference to the last catch
  auto &back() const { return catches.back(); }

  /// Inserts a catch at the given position.
  /// @param pos the position
  /// @param v the catch
  /// @return an iterator to the newly added catch
  template <typename It> auto insert(It pos, Catch v) {
    return catches.insert(pos, std::move(v));
  }

  /// Appends a catch.
  /// @param v the catch
  void push_back(Catch v) { catches.push_back(std::move(v)); }

  /// Emplaces a catch.
  /// @tparam Args the catch constructor args
  template <typename... Args> void emplace_back(Args &&... args) {
    catches.emplace_back(std::forward<Args>(args)...);
  }

  /// Erases a catch at the given position.
  /// @param pos the position
  /// @return the iterator beyond the erased catch
  template <typename It> auto erase(It pos) { return catches.erase(pos); }

private:
  std::ostream &doFormat(std::ostream &os) const override;

  Value *doClone() const override;
};

/// Flow that contains an unordered list of flows. Execution starts at the first flow.
class UnorderedFlow : public AcceptorExtend<SeriesFlow, Flow> {
private:
  std::list<FlowPtr> series;

public:
  static const char NodeId;

  using AcceptorExtend::AcceptorExtend;

  /// @return an iterator to the first flow
  auto begin() { return util::raw_ptr_adaptor(series.begin()); }
  /// @return an iterator beyond the last flow
  auto end() { return util::raw_ptr_adaptor(series.end()); }
  /// @return an iterator to the first flow
  auto begin() const { return util::const_raw_ptr_adaptor(series.begin()); }
  /// @return an iterator beyond the last flow
  auto end() const { return util::const_raw_ptr_adaptor(series.end()); }

  /// @return a pointer to the first flow
  Flow *front() { return series.front().get(); }
  /// @return a pointer to the last flow
  Flow *back() { return series.back().get(); }
  /// @return a pointer to the first flow
  const Flow *front() const { return series.front().get(); }
  /// @return a pointer to the last flow
  const Flow *back() const { return series.back().get(); }

  /// Inserts an flow at the given position.
  /// @param pos the position
  /// @param v the flow
  /// @return an iterator to the newly added flow
  template <typename It> auto insert(It pos, FlowPtr v) {
    return util::raw_ptr_adaptor(series.insert(pos.internal, std::move(v)));
  }
  /// Appends an flow.
  /// @param f the flow
  void push_back(FlowPtr f) { series.push_back(std::move(f)); }

  /// Erases the item at the supplied position.
  /// @param pos the position
  /// @return the iterator beyond the removed flow
  template <typename It> auto erase(It pos) {
    return util::raw_ptr_adaptor(series.erase(pos.internal));
  }

private:
  std::ostream &doFormat(std::ostream &os) const override;

  Value *doClone() const override;
};

/// Flow that represents a pipeline. Pipelines with only function
/// stages are expressions and have a concrete type. Pipelines with
/// generator stages are not expressions and have no type. This
/// representation allows for stages that output generators but do
/// not get explicitly iterated in the pipeline, since generator
/// stages are denoted by a separate flag.
class PipelineFlow : public AcceptorExtend<PipelineFlow, Flow> {
public:
  /// Represents a single stage in a pipeline.
  class Stage {
  private:
    /// the function being (partially) called in this stage
    ValuePtr func;
    /// the function arguments, where null represents where
    /// previous pipeline output should go
    std::vector<ValuePtr> args;
    /// true if this stage is a generator
    bool generator;
    /// true if this stage is marked parallel
    bool parallel;

  public:
    /// Constructs a pipeline stage.
    /// @param func the function being called
    /// @param args call arguments, with exactly one null entry
    /// @param generator whether this stage is a generator stage
    /// @param parallel whether this stage is parallel
    Stage(ValuePtr func, std::vector<ValuePtr> args, bool generator, bool parallel)
        : func(std::move(func)), args(std::move(args)), generator(generator),
          parallel(parallel) {}

    /// @return an iterator to the first argument
    auto begin() { return util::raw_ptr_adaptor(args.begin()); }
    /// @return an iterator beyond the last argument
    auto end() { return util::raw_ptr_adaptor(args.end()); }
    /// @return an iterator to the first argument
    auto begin() const { return util::const_raw_ptr_adaptor(args.begin()); }
    /// @return an iterator beyond the last argument
    auto end() const { return util::const_raw_ptr_adaptor(args.end()); }

    /// @return a pointer to the first argument
    Value *front() { return args.front().get(); }
    /// @return a pointer to the last argument
    Value *back() { return args.back().get(); }
    /// @return a pointer to the first argument
    const Value *front() const { return args.front().get(); }
    /// @return a pointer to the last argument
    const Value *back() const { return args.back().get(); }

    /// @return the called function
    Value *getFunc() { return func.get(); }
    /// @return the called function
    const Value *getFunc() const { return func.get(); }

    /// Sets the stage's generator flag.
    /// @param v the new value
    void setGenerator(bool v = true) { generator = v; }
    /// @return whether this stage is a generator stage
    bool isGenerator() const { return generator; }
    /// Sets the stage's parallel flag.
    /// @param v the new value
    void setParallel(bool v = true) { parallel = v; }
    /// @return whether this stage is parallel
    bool isParallel() const { return parallel; }
    /// @return the output type of this stage
    const types::Type *getOutputType() const;
    /// @return deep copy of this stage; used to clone pipelines
    Stage clone() const;
  };

private:
  /// pipeline stages
  std::vector<Stage> stages;

public:
  static const char NodeId;

  /// Constructs a pipeline flow.
  /// @param stages vector of pipeline stages
  /// @param name the name
  explicit PipelineFlow(std::vector<Stage> stages = {}, std::string name = "")
      : AcceptorExtend(std::move(name)), stages(std::move(stages)) {}

  /// @return an iterator to the first stage
  auto begin() { return stages.begin(); }
  /// @return an iterator beyond the last stage
  auto end() { return stages.end(); }
  /// @return an iterator to the first stage
  auto begin() const { return stages.begin(); }
  /// @return an iterator beyond the last stage
  auto end() const { return stages.end(); }

  /// @return a pointer to the first stage
  Stage &front() { return stages.front(); }
  /// @return a pointer to the last stage
  Stage &back() { return stages.back(); }
  /// @return a pointer to the first stage
  const Stage &front() const { return stages.front(); }
  /// @return a pointer to the last stage
  const Stage &back() const { return stages.back(); }

  /// Inserts a stage
  /// @param pos the position
  /// @param v the stage
  /// @return an iterator to the newly added stage
  template <typename It> auto insert(It pos, Stage v) {
    return stages.insert(pos, std::move(v));
  }
  /// Appends an stage.
  /// @param v the stage
  void push_back(Stage v) { stages.push_back(std::move(v)); }

  /// Erases the item at the supplied position.
  /// @param pos the position
  /// @return the iterator beyond the removed stage
  template <typename It> auto erase(It pos) { return stages.erase(pos.internal); }

  /// Emplaces a stage.
  /// @param args the args
  template <typename... Args> void emplace_back(Args &&... args) {
    stages.emplace_back(std::forward<Args>(args)...);
  }

private:
  std::ostream &doFormat(std::ostream &os) const override;

  Value *doClone() const override;
};

} // namespace ir
} // namespace seq

// See https://github.com/fmtlib/fmt/issues/1283.
namespace fmt {
using seq::ir::Flow;

template <typename Char>
struct formatter<Flow, Char> : fmt::v6::internal::fallback_formatter<Flow, Char> {};
} // namespace fmt