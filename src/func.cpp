#include "seq/basestage.h"
#include "seq/makerec.h"
#include "seq/call.h"
#include "seq/expr.h"
#include "seq/exprstage.h"
#include "seq/exc.h"
#include "seq/func.h"

using namespace seq;
using namespace llvm;

BaseFunc::BaseFunc() :
    module(nullptr), preambleBlock(nullptr), func(nullptr), argsVar(true)
{
}

Var *BaseFunc::getArgVar()
{
	return &argsVar;
}

LLVMContext& BaseFunc::getContext()
{
	return module->getContext();
}

BasicBlock *BaseFunc::getPreamble() const
{
	if (!preambleBlock)
		throw exc::SeqException("cannot request preamble before code generation");

	return preambleBlock;
}

types::Type *BaseFunc::getInType() const
{
	return types::VoidType::get();
}

types::Type *BaseFunc::getOutType() const
{
	return types::VoidType::get();
}

Function *BaseFunc::getFunc()
{
	if (!func)
		throw exc::SeqException("function not yet generated");

	return func;
}

Func::Func(types::Type& inType,
           types::Type& outType,
           std::string name,
           void *rawFunc) :
    BaseFunc(), inType(&inType), outType(&outType), pipelines(),
    result(nullptr), name(std::move(name)), rawFunc(rawFunc)
{
}

Func::Func(types::Type& inType, types::Type& outType) :
    Func(inType, outType, "", nullptr)
{
}

void Func::codegen(Module *module)
{
	if (!this->module)
		this->module = module;

	if (func)
		return;

	func = inType->makeFuncOf(module, outType);

	if (rawFunc) {
		func->setName(name);
		return;
	}

	if (pipelines.empty())
		throw exc::SeqException("function has no pipelines");

	LLVMContext& context = module->getContext();

	preambleBlock = BasicBlock::Create(context, "preamble", func);
	IRBuilder<> builder(preambleBlock);
	result = inType->setFuncArgs(func, preambleBlock);

	if (!inType->is(types::VoidType::get())) {
		BaseStage& argsBase = BaseStage::make(types::VoidType::get(), inType);
		argsBase.setBase(this);
		argsBase.result = result;
		argsVar = argsBase;
	}

	BasicBlock *entry = BasicBlock::Create(context, "entry", func);
	builder.SetInsertPoint(entry);
	BasicBlock *block;

	for (auto &pipeline : pipelines) {
		pipeline.validate();
		builder.SetInsertPoint(&func->getBasicBlockList().back());
		block = BasicBlock::Create(context, "pipeline", func);
		builder.CreateBr(block);

		auto *begin = dynamic_cast<BaseStage *>(pipeline.getHead());
		assert(begin);
		begin->setBase(pipeline.getHead()->getBase());
		begin->block = block;
		pipeline.getHead()->codegen(module);
	}

	BasicBlock *exitBlock = &func->getBasicBlockList().back();
	builder.SetInsertPoint(exitBlock);

	if (outType->is(types::VoidType::get())) {
		builder.CreateRetVoid();
	} else {
		Stage *tail = pipelines.back().getHead();
		while (!tail->getNext().empty())
			tail = tail->getNext().back();

		if (!dynamic_cast<Return *>(tail)) {  // i.e. if there isn't already a return at the end
			if (!tail->getOutType()->isChildOf(outType))
				throw exc::SeqException("function does not output type '" + outType->getName() + "'");

			builder.CreateRet(builder.CreateLoad(tail->result));
		}
	}

	builder.SetInsertPoint(preambleBlock);
	builder.CreateBr(entry);
}

Value *Func::codegenCall(BaseFunc *base, Value *arg, BasicBlock *block)
{
	codegen(block->getModule());
	return inType->callFuncOf(func, arg, block);
}

void Func::codegenReturn(Expr *expr, BasicBlock*& block)
{
	types::Type *type = expr ? expr->getType() : types::VoidType::get();

	if (!type->isChildOf(outType))
		throw exc::SeqException(
		  "cannot return '" + type->getName() + "' from function returning '" + outType->getName() + "'");

	if (expr) {
		Value *v = expr->codegen(this, block);
		IRBuilder<> builder(block);
		builder.CreateRet(v);
	} else {
		IRBuilder<> builder(block);
		builder.CreateRetVoid();
	}

	/*
	 * Can't have anything after the `ret` instruction we just added,
	 * so make a new block and return that to the caller.
	 */
	block = BasicBlock::Create(block->getContext(), "", block->getParent());
}

void Func::add(Pipeline pipeline)
{
	if (pipeline.isAdded())
		throw exc::MultiLinkException(*pipeline.getHead());

	pipelines.push_back(pipeline);
	pipeline.setAdded();
}

void Func::finalize(Module *module, ExecutionEngine *eng)
{
	if (rawFunc) {
		eng->addGlobalMapping(func, rawFunc);
	} else {
		for (auto &pipeline : pipelines) {
			pipeline.getHead()->finalize(module, eng);
		}
	}
}

Var *Func::getArgVar()
{
	if (getInType()->is(types::VoidType::get()))
		throw exc::SeqException("cannot get argument variable of void-input function");

	return &argsVar;
}

types::Type *Func::getInType() const
{
	return inType;
}

types::Type *Func::getOutType() const
{
	return outType;
}

void Func::setInOut(types::Type *inType, types::Type *outType)
{
	this->inType = inType;
	this->outType = outType;
}

void Func::setNative(std::string name, void *rawFunc)
{
	this->name = std::move(name);
	this->rawFunc = rawFunc;
}

Pipeline Func::operator|(Pipeline to)
{
	if (rawFunc)
		throw exc::SeqException("cannot add pipelines to native function");

	if (to.isAdded())
		throw exc::MultiLinkException(*to.getHead());

	to.getHead()->setBase(this);
	BaseStage& begin = BaseStage::make(types::AnyType::get(), inType);
	begin.setBase(this);
	begin.deferResult(&result);

	Pipeline full = begin | to;
	add(full);

	return full;
}

Pipeline Func::operator|(PipelineList& to)
{
	return *this | MakeRec::make(to);
}

Call& Func::operator()()
{
	return Call::make(*this);
}

BaseFuncLite::BaseFuncLite(Function *func) : BaseFunc()
{
	module = func->getParent();
	preambleBlock = &*func->getBasicBlockList().begin();
	this->func = func;
}

void BaseFuncLite::codegen(Module *module)
{
	throw exc::SeqException("cannot codegen lite base function");
}

Value *BaseFuncLite::codegenCall(BaseFunc *base, Value *arg, BasicBlock *block)
{
	throw exc::SeqException("cannot call lite base function");
}

void BaseFuncLite::codegenReturn(Expr *expr, BasicBlock*& block)
{
	throw exc::SeqException("cannot return from lite base function");
}

void BaseFuncLite::add(Pipeline pipeline)
{
	throw exc::SeqException("cannot add pipelines to lite base function");
}

FuncList::Node::Node(Func& f) :
    f(f), next(nullptr)
{
}

FuncList::FuncList(Func& f)
{
	head = tail = new Node(f);
}

FuncList& FuncList::operator,(Func& f)
{
	auto *n = new Node(f);
	tail->next = n;
	tail = n;
	return *this;
}

FuncList& seq::operator,(Func& f1, Func& f2)
{
	auto& l = *new FuncList(f1);
	l , f2;
	return l;
}

MultiCall& FuncList::operator()()
{
	std::vector<Func *> funcs;
	for (Node *n = head; n; n = n->next)
		funcs.push_back(&n->f);

	return MultiCall::make(funcs);
}
