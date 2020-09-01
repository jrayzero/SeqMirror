#include "util/fmt/format.h"
#include "util/fmt/ostream.h"
#include <memory>
#include <ostream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lang/seq.h"
#include "parser/ast/ast.h"
#include "parser/ast/codegen.h"
#include "parser/ast/codegen_ctx.h"
#include "parser/ast/format.h"
#include "parser/ast/transform.h"
#include "parser/common.h"

using fmt::format;
using std::get;
using std::make_shared;
using std::make_unique;
using std::move;
using std::ostream;
using std::shared_ptr;
using std::stack;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace seq {
namespace ast {

void CodegenVisitor::defaultVisit(const Expr *n) {
  internalError("invalid node {}", *n);
}

void CodegenVisitor::defaultVisit(const Stmt *n) {
  internalError("invalid node {}", *n);
}

void CodegenVisitor::defaultVisit(const Pattern *n) {
  internalError("invalid node {}", *n);
}

CodegenVisitor::CodegenVisitor(std::shared_ptr<LLVMContext> ctx)
    : ctx(ctx), resultExpr(nullptr), resultStmt(nullptr), resultPattern(nullptr) {}

seq::Expr *CodegenVisitor::transform(const Expr *expr) {
  if (!expr)
    return nullptr;
  CodegenVisitor v(ctx);
  v.setSrcInfo(expr->getSrcInfo());
  expr->accept(v);
  if (v.resultExpr) {
    v.resultExpr->setSrcInfo(expr->getSrcInfo());
    if (auto t = ctx->getTryCatch())
      v.resultExpr->setTryCatch(t);

    auto t = expr->getType()->getClass();
    assert(t);
    v.resultExpr->setType(realizeType(t->getClass()));
  }
  return v.resultExpr;
}

seq::Stmt *CodegenVisitor::transform(const Stmt *stmt) {
  CodegenVisitor v(ctx);
  stmt->accept(v);
  v.setSrcInfo(stmt->getSrcInfo());
  if (v.resultStmt) {
    v.resultStmt->setSrcInfo(stmt->getSrcInfo());
    v.resultStmt->setBase(ctx->getBase());
    ctx->getBlock()->add(v.resultStmt);
  }
  return v.resultStmt;
}

seq::Pattern *CodegenVisitor::transform(const Pattern *ptr) {
  CodegenVisitor v(ctx);
  v.setSrcInfo(ptr->getSrcInfo());
  ptr->accept(v);
  if (v.resultPattern) {
    v.resultPattern->setSrcInfo(ptr->getSrcInfo());
    if (auto t = ctx->getTryCatch())
      v.resultPattern->setTryCatch(t);
  }
  return v.resultPattern;
}

void CodegenVisitor::visit(const BoolExpr *expr) {
  resultExpr = N<seq::BoolExpr>(expr->value);
}

void CodegenVisitor::visit(const IntExpr *expr) {
  try {
    if (expr->suffix == "u") {
      uint64_t i = std::stoull(expr->value, nullptr, 0);
      resultExpr = N<seq::IntExpr>(i);
    } else {
      int64_t i = std::stoull(expr->value, nullptr, 0);
      resultExpr = N<seq::IntExpr>(i);
    }
  } catch (std::out_of_range &) {
    error(getSrcInfo(), fmt::format("integer {} out of range",
                                    expr->value)
                            .c_str()); /// TODO: move to transform
  }
}

void CodegenVisitor::visit(const FloatExpr *expr) {
  resultExpr = N<seq::FloatExpr>(expr->value);
}

void CodegenVisitor::visit(const StringExpr *expr) {
  resultExpr = N<seq::StrExpr>(expr->value);
}

shared_ptr<LLVMItem::Item>
CodegenVisitor::processIdentifier(shared_ptr<LLVMContext> tctx, const string &id) {
  auto val = tctx->find(id);
  if (!val)
    error(getSrcInfo(), fmt::format("? val {}", id).c_str());
  assert(val);
  // assert(
  // !(val->getVar() && val->isGlobal() && val->getBase() != ctx->getBase()));
  return val;
}

void CodegenVisitor::visit(const IdExpr *expr) {
  auto i = processIdentifier(ctx, expr->value);
  // TODO: this makes no sense: why setAtomic on temporary expr?
  // if (var->isGlobal() && var->getBase() == ctx->getBase() &&
  //     ctx->hasFlag("atomic"))
  //   dynamic_cast<seq::VarExpr *>(i->getExpr())->setAtomic();
  resultExpr = i->getExpr();
}

void CodegenVisitor::visit(const TupleExpr *expr) {
  vector<seq::Expr *> items;
  for (auto &&i : expr->items)
    items.push_back(transform(i));
  resultExpr = N<seq::RecordExpr>(items, vector<string>(items.size(), ""));
}

void CodegenVisitor::visit(const IfExpr *expr) {
  resultExpr = N<seq::CondExpr>(transform(expr->cond), transform(expr->eif),
                                transform(expr->eelse));
}

void CodegenVisitor::visit(const UnaryExpr *expr) {
  assert(expr->op == "!");
  resultExpr = N<seq::UOpExpr>(seq::uop(expr->op), transform(expr->expr));
}

void CodegenVisitor::visit(const BinaryExpr *expr) {
  assert(expr->op == "is" || expr->op == "&&" || expr->op == "||");
  if (expr->op == "is")
    resultExpr = N<seq::IsExpr>(transform(expr->lexpr), transform(expr->rexpr));
  else
    resultExpr = N<seq::BOpExpr>(seq::bop(expr->op), transform(expr->lexpr),
                                 transform(expr->rexpr));
}

void CodegenVisitor::visit(const PipeExpr *expr) {
  vector<seq::Expr *> exprs;
  vector<seq::types::Type *> inTypes;
  for (int i = 0; i < expr->items.size(); i++) {
    exprs.push_back(transform(expr->items[i].expr));
    inTypes.push_back(realizeType(expr->inTypes[i]->getClass()));
    // LOG("-- {}", inTypes.back()->getName());
  }
  auto p = new seq::PipeExpr(exprs);
  p->setIntermediateTypes(inTypes);
  for (int i = 0; i < expr->items.size(); i++)
    if (expr->items[i].op == "||>")
      p->setParallel(i);
  resultExpr = p;
}

void CodegenVisitor::visit(const TupleIndexExpr *expr) {
  resultExpr =
      N<seq::ArrayLookupExpr>(transform(expr->expr), N<seq::IntExpr>(expr->index));
}

void CodegenVisitor::visit(const CallExpr *expr) {
  auto lhs = transform(expr->expr);
  vector<seq::Expr *> items;
  vector<string> names;
  bool isPartial = false;
  for (auto &&i : expr->args) {
    if (CAST(i.value, EllipsisExpr)) {
      items.push_back(nullptr);
      isPartial = true;
    } else {
      items.push_back(transform(i.value));
    }
    names.push_back("");
  }
  if (isPartial)
    resultExpr = N<seq::PartialCallExpr>(lhs, items, names);
  else
    resultExpr = N<seq::CallExpr>(lhs, items, names);
}

void CodegenVisitor::visit(const StackAllocExpr *expr) {
  auto c = expr->typeExpr->getType()->getClass();
  assert(c);
  resultExpr =
      N<seq::ArrayExpr>(realizeType(c->getClass()), transform(expr->expr), true);
}

void CodegenVisitor::visit(const DotExpr *expr) {
  if (auto c = CAST(expr->expr, IdExpr))
    if (c->value.size() && c->value[0] == '/') {
      auto ictx = ctx->getImports()->getImport(c->value.substr(1))->lctx;
      assert(ictx);
      resultExpr = processIdentifier(ictx, expr->member)->getExpr();
      return;
    }
  resultExpr = N<seq::GetElemExpr>(transform(expr->expr), expr->member);
}

// void CodegenVisitor::visit(const EllipsisExpr *expr) {}

void CodegenVisitor::visit(const PtrExpr *expr) {
  auto e = CAST(expr->expr, IdExpr);
  assert(e);
  auto v = ctx->find(e->value, true)->getVar();
  assert(v);
  resultExpr = N<seq::VarPtrExpr>(v->getHandle());
}

void CodegenVisitor::visit(const YieldExpr *expr) {
  resultExpr = N<seq::YieldExpr>(ctx->getBase());
}

void CodegenVisitor::visit(const SuiteStmt *stmt) {
  for (auto &s : stmt->stmts)
    transform(s);
}

void CodegenVisitor::visit(const PassStmt *stmt) {}

void CodegenVisitor::visit(const BreakStmt *stmt) { resultStmt = N<seq::Break>(); }

void CodegenVisitor::visit(const ContinueStmt *stmt) {
  resultStmt = N<seq::Continue>();
}

void CodegenVisitor::visit(const ExprStmt *stmt) {
  resultStmt = N<seq::ExprStmt>(transform(stmt->expr));
}

void CodegenVisitor::visit(const AssignStmt *stmt) {
  /// TODO: atomic operations & JIT
  auto i = CAST(stmt->lhs, IdExpr);
  assert(i);
  auto var = i->value;
  // is it variable?
  if (stmt->rhs->isType()) {
    // ctx->addType(var, realizeType(stmt->rhs->getType()->getClass()));
  } else {
    auto varStmt = new seq::VarStmt(transform(stmt->rhs), nullptr);
    if (ctx->isToplevel())
      varStmt->getVar()->setGlobal();
    varStmt->getVar()->setType(realizeType(stmt->rhs->getType()->getClass()));
    ctx->addVar(var, varStmt->getVar());
    resultStmt = varStmt;
  }
}

void CodegenVisitor::visit(const AssignMemberStmt *stmt) {
  resultStmt =
      N<seq::AssignMember>(transform(stmt->lhs), stmt->member, transform(stmt->rhs));
}

void CodegenVisitor::visit(const UpdateStmt *stmt) {
  auto i = CAST(stmt->lhs, IdExpr);
  assert(i);
  auto var = i->value;
  auto val = ctx->find(var, true);
  assert(val && val->getVar());
  resultStmt = new seq::Assign(val->getVar()->getHandle(), transform(stmt->rhs));
}

void CodegenVisitor::visit(const DelStmt *stmt) {
  auto expr = CAST(stmt->expr, IdExpr);
  assert(expr);
  auto v = ctx->find(expr->value, true)->getVar();
  assert(v);
  ctx->remove(expr->value);
  resultStmt = N<seq::Del>(v->getHandle());
}

void CodegenVisitor::visit(const PrintStmt *stmt) {
  resultStmt = N<seq::Print>(transform(stmt->expr), false);
}

void CodegenVisitor::visit(const ReturnStmt *stmt) {
  if (!stmt->expr) {
    resultStmt = N<seq::Return>(nullptr);
  } else {
    auto ret = new seq::Return(transform(stmt->expr));
    resultStmt = ret;
  }
}

void CodegenVisitor::visit(const YieldStmt *stmt) {
  auto ret = new seq::Yield(stmt->expr ? transform(stmt->expr) : nullptr);
  ctx->getBase()->setGenerator();
  resultStmt = ret;
}

void CodegenVisitor::visit(const AssertStmt *stmt) {
  resultStmt = N<seq::Assert>(transform(stmt->expr));
}

void CodegenVisitor::visit(const WhileStmt *stmt) {
  auto r = new seq::While(transform(stmt->cond));
  ctx->addBlock(r->getBlock());
  transform(stmt->suite);
  ctx->popBlock();
  resultStmt = r;
}

void CodegenVisitor::visit(const ForStmt *stmt) {
  auto r = new seq::For(transform(stmt->iter));
  string forVar;
  ctx->addBlock(r->getBlock());
  auto expr = CAST(stmt->var, IdExpr);
  assert(expr);
  ctx->addVar(expr->value, r->getVar());
  r->getVar()->setType(realizeType(expr->getType()->getClass()));
  transform(stmt->suite);
  ctx->popBlock();
  resultStmt = r;
}

void CodegenVisitor::visit(const IfStmt *stmt) {
  auto r = new seq::If();
  for (auto &i : stmt->ifs) {
    ctx->addBlock(i.cond ? r->addCond(transform(i.cond)) : r->addElse());
    transform(i.suite);
    ctx->popBlock();
  }
  resultStmt = r;
}

void CodegenVisitor::visit(const MatchStmt *stmt) {
  auto m = new seq::Match();
  m->setValue(transform(stmt->what));
  for (auto ci = 0; ci < stmt->cases.size(); ci++) {
    string varName;
    seq::Var *var = nullptr;
    seq::Pattern *pat;
    if (auto p = CAST(stmt->patterns[ci], BoundPattern)) {
      ctx->addBlock();
      auto boundPat = new seq::BoundPattern(transform(p->pattern));
      var = boundPat->getVar();
      varName = p->var;
      pat = boundPat;
      ctx->popBlock();
    } else {
      ctx->addBlock();
      pat = transform(stmt->patterns[ci]);
      ctx->popBlock();
    }
    ctx->addBlock(m->addCase(pat));
    transform(stmt->cases[ci]);
    if (var)
      ctx->addVar(varName, var);
    ctx->popBlock();
  }
  resultStmt = m;
}

void CodegenVisitor::visit(const ImportStmt *stmt) {
  auto file = stmt->from.first.substr(1);
  // ctx->getImports()->getImportFile(stmt->from.first, ctx->getFilename());
  // assert(!file.empty());

  auto import = const_cast<ImportContext::Import *>(ctx->getImports()->getImport(file));
  assert(import);
  if (!import->lctx) {
    import->lctx =
        make_shared<LLVMContext>(file, ctx->getRealizations(), ctx->getImports(),
                                 ctx->getBlock(), ctx->getBase(), ctx->getJIT());
    CodegenVisitor(import->lctx).transform(import->statements.get());
  }

  if (!stmt->what.size()) {
    ctx->addImport(stmt->from.second == "" ? stmt->from.first : stmt->from.second,
                   file);
  } else if (stmt->what.size() == 1 && stmt->what[0].first == "*") {
    for (auto &i : *(import->lctx))
      ctx->add(i.first, i.second.front());
  } else {
    for (auto &w : stmt->what) {
      for (auto i : *(import->lctx)) {
        if (i.second.front()->getVar() && i.first == w.first)
          ctx->add(i.first, i.second.front());
      }
      // for (auto &w : stmt->what) {
      // TODO: those should have been already resolved  ... ?
      // for (auto i : *(import->lctx)) {
      //   if (i.first.substr(0, w.first.size() + 1) == w.first + ".")
      //     ctx->add(i.first, i.second.front());
      // }
      // auto c = import->lctx->find(w.first);
      // assert(c);
      // ctx->add(w.second == "" ? w.first : w.second, c);
      // }
    }
  }
}

void CodegenVisitor::visit(const TryStmt *stmt) {
  auto r = new seq::TryCatch();
  auto oldTryCatch = ctx->getTryCatch();
  ctx->setTryCatch(r);
  ctx->addBlock(r->getBlock());
  transform(stmt->suite);
  ctx->popBlock();
  ctx->setTryCatch(oldTryCatch);
  int varIdx = 0;
  for (auto &c : stmt->catches) {
    /// TODO: get rid of typeinfo here?
    ctx->addBlock(r->addCatch(
        c.exc->getType() ? realizeType(c.exc->getType()->getClass()) : nullptr));
    if (!c.var.empty())
      ctx->addVar(c.var, r->getVar(varIdx++));
    transform(c.suite);
    ctx->popBlock();
  }
  if (stmt->finally) {
    ctx->addBlock(r->getFinally());
    transform(stmt->finally);
    ctx->popBlock();
  }
  resultStmt = r;
}

void CodegenVisitor::visit(const GlobalStmt *stmt) {
  auto var = ctx->find(stmt->var)->getVar();
  assert(var && var->isGlobal() && var->getBase() == ctx->getBase());
  ctx->addVar(stmt->var, var->getHandle(), true);
}

void CodegenVisitor::visit(const ThrowStmt *stmt) {
  resultStmt = N<seq::Throw>(transform(stmt->expr));
}

void CodegenVisitor::visit(const FunctionStmt *stmt) {
  auto name = ctx->getRealizations()->getCanonicalName(stmt->getSrcInfo());
  for (auto &real : ctx->getRealizations()->getFuncRealizations(name)) {
    auto f = (seq::Func *)real.handle;
    assert(f);
    if (in(real.ast->attributes, "internal"))
      continue;
    // LOG7("[codegen] generating fn {}", real.fullName);
    f->setName(chop(real.fullName));
    f->setSrcInfo(getSrcInfo());
    if (!ctx->isToplevel())
      f->setEnclosingFunc(ctx->getBase());
    ctx->addBlock(f->getBlock(), f);
    vector<string> names;
    vector<seq::types::Type *> types;
    for (int i = 1; i < real.type->args.size(); i++) {
      types.push_back(realizeType(real.type->args[i]->getClass()));
      names.push_back(real.ast->args[i - 1].name);
    }
    f->setIns(types);
    f->setArgNames(names);
    f->setOut(realizeType(real.type->args[0]->getClass()));
    for (auto a : real.ast->attributes) {
      f->addAttribute(a);
      if (a == "atomic")
        ctx->setFlag("atomic");
    }
    if (in(real.ast->attributes, ".external")) {
      f->setName(chop(real.ast->name));
      f->setExternal();
    } else {
      for (auto &arg : names)
        ctx->addVar(arg, f->getArgVar(arg));
      transform(real.ast->suite.get());
    }
    ctx->popBlock();
  }
}

void CodegenVisitor::visitMethods(const string &name) {
  auto c = ctx->getRealizations()->findClass(name);
  if (c)
    for (auto &m : c->methods)
      for (auto &mm : m.second) {
        FunctionStmt *f = CAST(ctx->getRealizations()->getAST(mm->name), FunctionStmt);
        visit(f);
      }
}

void CodegenVisitor::visit(const ClassStmt *stmt) {
  // visitMethods(ctx->getRealizations()->getCanonicalName(stmt->getSrcInfo()));
}

void CodegenVisitor::visit(const StarPattern *pat) {
  resultPattern = N<seq::StarPattern>();
}

void CodegenVisitor::visit(const IntPattern *pat) {
  resultPattern = N<seq::IntPattern>(pat->value);
}

void CodegenVisitor::visit(const BoolPattern *pat) {
  resultPattern = N<seq::BoolPattern>(pat->value);
}

void CodegenVisitor::visit(const StrPattern *pat) {
  resultPattern = N<seq::StrPattern>(pat->value);
}

void CodegenVisitor::visit(const SeqPattern *pat) {
  resultPattern = N<seq::SeqPattern>(pat->value);
}

void CodegenVisitor::visit(const RangePattern *pat) {
  resultPattern = N<seq::RangePattern>(pat->start, pat->end);
}

void CodegenVisitor::visit(const TuplePattern *pat) {
  vector<seq::Pattern *> pp;
  for (auto &p : pat->patterns)
    pp.push_back(transform(p));
  resultPattern = N<seq::RecordPattern>(move(pp));
}

void CodegenVisitor::visit(const ListPattern *pat) {
  vector<seq::Pattern *> pp;
  for (auto &p : pat->patterns)
    pp.push_back(transform(p));
  resultPattern = N<seq::ArrayPattern>(move(pp));
}

void CodegenVisitor::visit(const OrPattern *pat) {
  vector<seq::Pattern *> pp;
  for (auto &p : pat->patterns)
    pp.push_back(transform(p));
  resultPattern = N<seq::OrPattern>(move(pp));
}

void CodegenVisitor::visit(const WildcardPattern *pat) {
  auto p = new seq::Wildcard();
  if (pat->var.size())
    ctx->addVar(pat->var, p->getVar());
  resultPattern = p;
}

void CodegenVisitor::visit(const GuardedPattern *pat) {
  resultPattern = N<seq::GuardedPattern>(transform(pat->pattern), transform(pat->cond));
}

seq::types::Type *CodegenVisitor::realizeType(types::ClassTypePtr t) {
  t = t->getClass();
  assert(t && t->canRealize());
  auto it = ctx->getRealizations()->classRealizations.find(t->name);
  assert(it != ctx->getRealizations()->classRealizations.end());
  auto it2 = it->second.find(t->realizeString());
  assert(it2 != it->second.end());
  assert(it2->second.handle);
  return it2->second.handle;
}

} // namespace ast
} // namespace seq