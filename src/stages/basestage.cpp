#include "seq/func.h"
#include "seq/basestage.h"

using namespace seq;
using namespace llvm;

static int idx = 1;

BaseStage::BaseStage(types::Type *in, types::Type *out, Stage *proxy) :
    InitStage("base" + std::to_string(idx++), in, out),
    proxy(proxy), deferredResult(nullptr)
{
}

BaseStage::BaseStage(types::Type *in, types::Type *out) :
    BaseStage(in, out, nullptr)
{
}

void BaseStage::codegen(Module *module)
{
	validate();

	if (prev && !block)
		block = prev->getAfter();

	if (!result && deferredResult)
		result = *deferredResult;

	codegenInit(block);
	codegenNext(module);
	finalizeInit();

	if (prev)
		prev->setAfter(getAfter());
}

types::Type *BaseStage::getOutType() const
{
	if (proxy && Stage::getOutType()->is(types::VoidType::get()))
		return proxy->getOutType();
	else
		return Stage::getOutType();
}

void BaseStage::deferResult(Value **result)
{
	deferredResult = result;
}

BaseStage& BaseStage::make(types::Type *in, types::Type *out, Stage *proxy)
{
	return *new BaseStage(in, out, proxy);
}

BaseStage& BaseStage::make(types::Type *in, types::Type *out)
{
	return *new BaseStage(in, out);
}
