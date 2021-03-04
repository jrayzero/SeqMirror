/*
 * types.h --- Seq type definitions.
 * Contains a basic implementation of Hindley-Milner's W algorithm.
 *
 * (c) Seq project. All rights reserved.
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser/common.h"

namespace seq {
namespace ast {

struct Expr;

namespace types {

/// Forward declarations
struct FuncType;
struct ClassType;
struct LinkType;
struct RecordType;
struct PartialType;
struct StaticType;

/**
 * A type instance that contains the basic plumbing for type inference.
 * The virtual methods are designed for Hindley-Milner's Algorithm W inference.
 * For more information, consult https://github.com/tomprimozic/type-systems.
 *
 * Type instances are widely mutated during the type inference and each type is intended
 * to be instantiated and manipulated as a shared_ptr.
 */
struct Type : public seq::SrcObject, public std::enable_shared_from_this<Type> {
  /// A structure that keeps the list of unification steps that can be undone later.
  /// Needed because the unify() is destructive.
  struct Unification {
    /// List of unbound types that have been changed.
    vector<LinkType *> linked;
    /// List of unbound types whose level has been changed.
    vector<pair<LinkType *, int>> leveled;

  public:
    /// Undo the unification step.
    void undo();
  };

public:
  /// Unifies a given type with the current type.
  /// @param typ A given type.
  /// @param undo A reference to Unification structure to track the unification steps
  ///             and allow later undoing of the unification procedure.
  /// @return Unification score: -1 for failure, anything >= 0 for success.
  ///         Higher score translates to a "better" unification.
  /// ⚠️ Destructive operation if undo is not null!
  ///    (both the current and a given type are modified).
  virtual int unify(Type *typ, Unification *undo) = 0;
  /// Generalize all unbound types whose level is below the provided level.
  /// This method replaces all unbound types with a generic types (e.g. ?1 -> T1).
  /// Note that the generalized type keeps the unbound type's ID.
  virtual shared_ptr<Type> generalize(int atLevel) = 0;
  /// Instantiate all generic types. Inverse of generalize(): it replaces all
  /// generic types with new unbound types (e.g. T1 -> ?1234).
  /// Note that the instantiated type has a distinct and unique ID.
  /// @param level Level of the instantiation.
  /// @param unboundCount A reference of the unbound counter to ensure that no two
  ///                     unbound types share the same ID.
  /// @param cache A reference to a lookup table to ensure that all instances of a
  ///              generic point to the same unbound type (e.g. dict[T, list[T]] should
  ///              be instantiated as dict[?1, list[?1]]).
  virtual shared_ptr<Type> instantiate(int atLevel, int &unboundCount,
                                       unordered_map<int, shared_ptr<Type>> &cache) = 0;

public:
  /// Get the final type (follow through all LinkType links).
  /// For example, for (a->b->c->d) it returns d.
  virtual shared_ptr<Type> follow();
  /// Obtain the list of internal unbound types.
  virtual vector<shared_ptr<Type>> getUnbounds() const;
  /// True if a type is realizable.
  virtual bool canRealize() const = 0;
  /// Pretty-print facility.
  virtual string toString() const = 0;
  /// Print the realization string.
  /// Similar to toString, but does not print the data unnecessary for realization
  /// (e.g. the function return type).
  virtual string realizedName() const = 0;

  /// Convenience virtual functions to avoid unnecessary dynamic_cast calls.
  virtual shared_ptr<FuncType> getFunc() { return nullptr; }
  virtual shared_ptr<PartialType> getPartial() { return nullptr; }
  virtual shared_ptr<ClassType> getClass() { return nullptr; }
  virtual shared_ptr<RecordType> getRecord() { return nullptr; }
  virtual shared_ptr<LinkType> getLink() { return nullptr; }
  virtual shared_ptr<LinkType> getUnbound() { return nullptr; }
  virtual shared_ptr<StaticType> getStatic() { return nullptr; }
  virtual shared_ptr<RecordType> getHeterogenousTuple() { return nullptr; }

  virtual bool is(const string &s);
};
typedef shared_ptr<Type> TypePtr;

/**
 * A basic type-inference building block.
 * LinkType is a metatype (or a type state) that describes the current information about
 * the expression type:
 *   - Link: underlying expression type is known and can be accessed through a pointer
 *           type (link).
 *   - Unbound: underlying expression type is currently unknown and is being inferred.
 *              An unbound is represented as ?id (e.g. ?3).
 *   - Generic: underlying expression type is unknown.
 *              Unlike unbound types, generic types cannot be inferred as-is and must be
 *              instantiated prior to the inference.
 *              Used only in function and class definitions.
 *              A generic is represented as Tid (e.g. T3).
 */
struct LinkType : public Type {
  /// Enumeration describing the current state.
  enum Kind { Unbound, Generic, Link } kind;
  /// The unique identifier of an unbound or generic type.
  int id;
  /// The type-checking level of an unbound type.
  int level;
  /// The type to which LinkType points to. nullptr if unknown (unbound or generic).
  TypePtr type;
  /// True if a type is a static type (e.g. N in Int[N: int]).
  bool isStatic;
  /// The generic name of a generic type, if applicable. Used for pretty-printing.
  string genericName;

public:
  LinkType(Kind kind, int id, int level = 0, TypePtr type = nullptr,
           bool isStatic = false, string genericName = "");
  /// Convenience constructor for linked types.
  explicit LinkType(TypePtr type);

public:
  int unify(Type *typ, Unification *undodo) override;
  TypePtr generalize(int atLevel) override;
  TypePtr instantiate(int atLevel, int &unboundCount,
                      unordered_map<int, TypePtr> &cache) override;

public:
  TypePtr follow() override;
  vector<TypePtr> getUnbounds() const override;
  bool canRealize() const override;
  string toString() const override;
  string realizedName() const override;

  shared_ptr<LinkType> getLink() override {
    return std::static_pointer_cast<LinkType>(shared_from_this());
  }
  shared_ptr<LinkType> getUnbound() override;
  shared_ptr<FuncType> getFunc() override {
    return kind == Link ? type->getFunc() : nullptr;
  }
  shared_ptr<PartialType> getPartial() override {
    return kind == Link ? type->getPartial() : nullptr;
  }
  shared_ptr<ClassType> getClass() override {
    return kind == Link ? type->getClass() : nullptr;
  }
  shared_ptr<RecordType> getRecord() override {
    return kind == Link ? type->getRecord() : nullptr;
  }
  shared_ptr<StaticType> getStatic() override {
    return kind == Link ? type->getStatic() : nullptr;
  }

private:
  /// Checks if a current (unbound) type occurs within a given type.
  /// Needed to prevent a recursive unification (e.g. ?1 with list[?1]).
  bool occurs(Type *typ, Type::Unification *undo);
};

/**
 * A generic class reference type. All Seq types inherit from this class.
 */
struct ClassType : public Type {
  /**
   * A generic type declaration.
   * Each generic is defined by its unique ID.
   */
  struct Generic {
    // Generic name.
    string name;
    // Name used for pretty-printing.
    string niceName;
    // Unique generic ID.
    int id;
    // Pointer to realized tupe (or generic LinkType).
    TypePtr type;
    // Default value that is used if a generic is not realized (e.g. [T=int]).
    shared_ptr<Expr> deflt;

    Generic(string name, string niceName, TypePtr type, int id,
            shared_ptr<Expr> deflt = nullptr)
        : name(move(name)), niceName(move(niceName)), id(id), type(move(type)),
          deflt(move(deflt)) {}
  };

  /// Canonical type name.
  string name;
  /// Name used for pretty-printing.
  string niceName;
  /// List of generics, if present.
  vector<Generic> generics;
  /// True if this class is just supposed to be type-checked upon
  /// (not supposed to be instantiated).
  bool isTrait;

  explicit ClassType(string name, string niceName, vector<Generic> generics = {});
  explicit ClassType(const shared_ptr<ClassType> &base);

public:
  int unify(Type *typ, Unification *undo) override;
  TypePtr generalize(int atLevel) override;
  TypePtr instantiate(int atLevel, int &unboundCount,
                      unordered_map<int, TypePtr> &cache) override;

public:
  vector<TypePtr> getUnbounds() const override;
  bool canRealize() const override;
  string toString() const override;
  string realizedName() const override;
  /// True if a class is a trait or has a generic that is a trait.
  virtual bool hasTrait() const;
  /// Convenience function to get the name of realized type
  /// (needed if a subclass realizes something else as well).
  virtual string realizedTypeName() const;
  shared_ptr<ClassType> getClass() override {
    return std::static_pointer_cast<ClassType>(shared_from_this());
  }
};
typedef shared_ptr<ClassType> ClassTypePtr;

/**
 * A generic class tuple (record) type. All Seq tuples inherit from this class.
 */
struct RecordType : public ClassType {
  /// List of tuple arguments.
  vector<TypePtr> args;

  explicit RecordType(string name, string niceName,
                      vector<ClassType::Generic> generics = {},
                      vector<TypePtr> args = {});
  RecordType(const ClassTypePtr &base, vector<TypePtr> args);

public:
  int unify(Type *typ, Unification *undo) override;
  TypePtr generalize(int atLevel) override;
  TypePtr instantiate(int atLevel, int &unboundCount,
                      unordered_map<int, TypePtr> &cache) override;

public:
  vector<TypePtr> getUnbounds() const override;
  bool canRealize() const override;
  string toString() const override;

  shared_ptr<RecordType> getRecord() override {
    return std::static_pointer_cast<RecordType>(shared_from_this());
  }
  shared_ptr<RecordType> getHeterogenousTuple() override;
};

/**
 * A generic type that represents a Seq function instantiation.
 * It inherits RecordType that realizes Callable[...].
 *
 * ⚠️ This is not a function pointer (Function[...]) type.
 */
struct FuncType : public RecordType {
  /// Canonical name of a (generic) function.
  string funcName;
  /// Function generics (e.g. T in def foo[T](...)).
  vector<ClassType::Generic> funcGenerics;
  /// Enclosing class or a function.
  TypePtr funcParent;

public:
  FuncType(const shared_ptr<RecordType> &baseType, string funcName,
           vector<ClassType::Generic> funcGenerics = {}, TypePtr funcParent = nullptr);

public:
  int unify(Type *typ, Unification *undo) override;
  TypePtr generalize(int atLevel) override;
  TypePtr instantiate(int atLevel, int &unboundCount,
                      unordered_map<int, TypePtr> &cache) override;

public:
  vector<TypePtr> getUnbounds() const override;
  bool canRealize() const override;
  string toString() const override;
  string realizedName() const override;

  shared_ptr<FuncType> getFunc() override {
    return std::static_pointer_cast<FuncType>(shared_from_this());
  }
};
typedef shared_ptr<FuncType> FuncTypePtr;

/**
 * A generic type that represents a partial Seq function instantiation.
 * It inherits RecordType that realizes Tuple[...].
 *
 * Note: partials only work on Seq functions. Function pointer partials
 *       will become a partials of Function.__call__ Seq function.
 */
struct PartialType : public RecordType {
  /// Seq function that is being partialized. Always generic (not instantiated).
  FuncTypePtr func;
  /// Arguments that are already provided (1 for known argument, 0 for expecting).
  vector<char> known;

public:
  PartialType(const shared_ptr<RecordType> &baseType, shared_ptr<FuncType> func,
              vector<char> known);

public:
  TypePtr generalize(int atLevel) override;
  TypePtr instantiate(int atLevel, int &unboundCount,
                      unordered_map<int, TypePtr> &cache) override;

public:
  shared_ptr<PartialType> getPartial() override {
    return std::static_pointer_cast<PartialType>(shared_from_this());
  }
};
typedef shared_ptr<FuncType> FuncTypePtr;

/**
 * A static integer type (e.g. N in def foo[N: int]). Usually an integer, but can point
 * to a static expression.
 */
struct StaticType : public Type {
  typedef std::function<int(const StaticType *)> EvalFn;
  /// List of static variables that a type depends on
  /// (e.g. for A+B+2, generics are {A, B}).
  vector<ClassType::Generic> generics;

  /// Evaluation status. If .first is true, the expression is evaluated and .second
  /// provides the evaluated value.
  pair<bool, int> staticEvaluation;
  /// A static expression that needs to be evaluated. .first is a pointer to such
  /// expression, while .second is a function that evaluates it. .first can be nullptr
  /// if there is no expression (assuming that staticEvaluation.first is set).
  pair<unique_ptr<Expr>, EvalFn> staticExpr;

  StaticType(vector<ClassType::Generic> generics,
             pair<unique_ptr<Expr>, EvalFn> staticExpr,
             pair<bool, int> staticEvaluation);
  /// Convenience function for static types whose evaluation is already known.
  explicit StaticType(int i);

public:
  int unify(Type *typ, Unification *undo) override;
  TypePtr generalize(int atLevel) override;
  TypePtr instantiate(int atLevel, int &unboundCount,
                      unordered_map<int, TypePtr> &cache) override;

public:
  vector<TypePtr> getUnbounds() const override;
  bool canRealize() const override;
  string toString() const override;
  string realizedName() const override;

  shared_ptr<StaticType> getStatic() override {
    return std::static_pointer_cast<StaticType>(shared_from_this());
  }
};

} // namespace types
} // namespace ast
} // namespace seq
