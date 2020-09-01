#pragma once

#include <deque>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "parser/ast/cache.h"
#include "parser/ast/context.h"
#include "parser/common.h"

namespace seq {
namespace ast {

class TypecheckItem {
  enum Kind { Func, Type, Var } kind;
  types::TypePtr type;
  std::string base;
  bool global;
  bool genericType;
  bool staticType;
  std::unordered_set<std::string> attributes;

public:
  TypecheckItem(Kind k, types::TypePtr type, const std::string &base,
                bool global = false, bool generic = false, bool stat = false)
      : kind(k), type(type), base(base), global(global), genericType(generic),
        staticType(stat) {}

  std::string getBase() const { return base; }
  bool isGlobal() const { return global; }
  bool isVar() const { return kind == Var; }
  bool isFunc() const { return kind == Func; }
  bool isType() const { return kind == Type; }
  bool isGeneric() const { return isType() && genericType; }
  bool isStatic() const { return isType() && staticType; }
  types::TypePtr getType() const { return type; }
  bool hasAttr(const std::string &s) const {
    return attributes.find(s) != attributes.end();
  }
};

class TypeContext : public Context<TypecheckItem> {
public:
  struct Base {
    types::TypePtr type;
    types::TypePtr returnType;
    Base(types::TypePtr p) : type(p), returnType(nullptr) {}
  };
  std::vector<Base> bases;
  /// Function parsing helpers: maintain current return type
  types::TypePtr matchType;
  /// Set of active unbound variables.
  /// If type checking is successful, all of them should be resolved.
  std::set<types::TypePtr> activeUnbounds;

public:
  static StmtPtr apply(std::shared_ptr<Cache> cache);
  TypeContext(std::shared_ptr<Cache> cache);

  // std::shared_ptr<TypecheckItem> find(const std::string &name) const;
  types::TypePtr findInternal(const std::string &name) const;

  using Context<TypecheckItem>::add;
  std::shared_ptr<TypecheckItem> add(TypecheckItem::Kind kind, const std::string &name,
                                     types::TypePtr type = nullptr, bool global = false,
                                     bool generic = false, bool stat = false);
  void dump(int pad = 0) override;

public:
  std::string getBase(bool full = false) const;
  int getLevel() const { return bases.size(); }

public:
  std::shared_ptr<types::LinkType> addUnbound(const SrcInfo &srcInfo, int level,
                                              bool setActive = true,
                                              bool isStatic = false);
  /// Calls `type->instantiate`, but populates the instantiation table
  /// with "parent" type.
  /// Example: for list[T].foo, list[int].foo will populate type of foo so that
  /// the generic T gets mapped to int.
  types::TypePtr instantiate(const SrcInfo &srcInfo, types::TypePtr type);
  types::TypePtr instantiate(const SrcInfo &srcInfo, types::TypePtr type,
                             types::ClassTypePtr generics, bool activate = true);
  types::TypePtr instantiateGeneric(const SrcInfo &srcInfo, types::TypePtr root,
                                    const std::vector<types::TypePtr> &generics);
};

} // namespace ast
} // namespace seq