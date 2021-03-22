/*
 * cache.cpp --- Code transformation cache (shared objects).
 *
 * (c) Seq project. All rights reserved.
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 */

#include <chrono>
#include <string>
#include <vector>

#include "parser/cache.h"
#include "parser/common.h"
#include "parser/visitors/translate/translate.h"
#include "parser/visitors/typecheck/typecheck.h"
#include "parser/visitors/typecheck/typecheck_ctx.h"

namespace seq {
namespace ast {

Cache::Cache(string argv0)
    : generatedSrcInfoCount(0), unboundCount(0), varCount(0), age(0), testFlags(0),
      argv0(move(argv0)), module(nullptr), typeCtx(nullptr), codegenCtx(nullptr) {}

string Cache::getTemporaryVar(const string &prefix, char sigil) {
  return fmt::format("{}{}_{}", sigil ? fmt::format("{}_", sigil) : "", prefix,
                     ++varCount);
}

SrcInfo Cache::generateSrcInfo() {
  return {FILE_GENERATED, generatedSrcInfoCount, generatedSrcInfoCount++, 0, 0};
}

types::ClassTypePtr Cache::findClass(const string &name) const {
  auto f = typeCtx->find(name);
  if (f && f->kind == TypecheckItem::Type)
    return f->type->getClass();
  return nullptr;
}

types::FuncTypePtr Cache::findFunction(const string &name) const {
  auto f = typeCtx->find(name);
  if (f && f->type && f->kind == TypecheckItem::Func)
    return f->type->getFunc();
  return nullptr;
}

types::FuncTypePtr Cache::findMethod(types::ClassType *typ, const string &member,
                                     const vector<pair<string, types::TypePtr>> &args) {
  auto e = make_unique<IdExpr>(typ->name);
  e->type = typ->getClass();
  seqassert(e->type, "not a class");
  int oldAge = typeCtx->age;
  typeCtx->age = 99999;
  auto f = typeCtx->findBestMethod(e.get(), member, args);
  typeCtx->age = oldAge;
  return f;
}

ir::types::Type *Cache::realizeType(types::ClassTypePtr type,
                                    const vector<types::TypePtr> &generics) {
  auto e = make_unique<IdExpr>(type->name);
  e->type = type;
  type = typeCtx->instantiateGeneric(e.get(), type, generics)->getClass();
  auto tv = TypecheckVisitor(typeCtx);
  if (auto rtv = tv.realize(type)) {
    return classes[rtv->getClass()->name]
        .realizations[rtv->getClass()->realizedTypeName()]
        ->ir;
  }
  return nullptr;
}

ir::Func *Cache::realizeFunction(types::FuncTypePtr type,
                                 const vector<types::TypePtr> &args,
                                 const vector<types::TypePtr> &generics,
                                 types::ClassTypePtr parentClass) {
  auto e = make_unique<IdExpr>(type->funcName);
  e->type = type;
  type = typeCtx->instantiate(e.get(), type, parentClass.get(), false)->getFunc();
  if (args.size() != type->args.size())
    return nullptr;
  for (int gi = 0; gi < args.size(); gi++) {
    types::Type::Unification undo;
    if (type->args[gi]->unify(args[gi].get(), &undo) < 0) {
      undo.undo();
      return nullptr;
    }
  }
  if (!generics.empty()) {
    if (generics.size() != type->funcGenerics.size())
      return nullptr;
    for (int gi = 0; gi < generics.size(); gi++) {
      types::Type::Unification undo;
      if (type->funcGenerics[gi].type->unify(generics[gi].get(), &undo) < 0) {
        undo.undo();
        return nullptr;
      }
    }
  }
  auto tv = TypecheckVisitor(typeCtx);
  if (auto rtv = tv.realize(type)) {
    auto pr = pendingRealizations; // copy it as it might be modified
    for (auto &fn : pr)
      TranslateVisitor(codegenCtx).transform(functions[fn.first].ast->clone());
    return functions[rtv->getFunc()->funcName].realizations[rtv->realizedName()]->ir;
  }
  return nullptr;
}

ir::types::Type *Cache::makeTuple(const vector<types::TypePtr> &types) {
  auto tv = TypecheckVisitor(typeCtx);
  auto name = tv.generateTupleStub(types.size());
  auto t = typeCtx->find(name);
  seqassert(t && t->type, "cannot find {}", name);
  return realizeType(t->type->getClass(), types);
}

ir::types::Type *Cache::makeFunction(const vector<types::TypePtr> &types) {
  auto tv = TypecheckVisitor(typeCtx);
  seqassert(!types.empty(), "types must have at least one argument");
  auto name = tv.generateFunctionStub(types.size() - 1);
  auto t = typeCtx->find(name);
  seqassert(t && t->type, "cannot find {}", name);
  return realizeType(t->type->getClass(), types);
}

} // namespace ast
} // namespace seq
