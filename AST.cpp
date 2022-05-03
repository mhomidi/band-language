#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <map>
#include "Parser.h"
#include "AST.h"

using namespace llvm;

static LLVMContext ctx;
static IRBuilder<> builder(ctx);
static unique_ptr<Module> module;
static map<std::string, Value *> namedValues;

Value *logErrorValue(const char *errStr)
{
    logError(errStr);
    return nullptr;
}

Value *NumberExpAST::codegen()
{
    return ConstantFP::get(ctx, APFloat(this->value));
}

Value *VariableExpAST::codegen()
{
    Value *value = namedValues[this->name];

    if (!value)
        return logErrorValue("Unknown variable name");

    return value;
}

Value *BinaryExpAST::codegen()
{
    Value *leftHandSide = this->lhs->codegen();
    Value *rightHandSide = this->rhs->codegen();

    if (!leftHandSide || !rightHandSide)
        return nullptr;

    switch (this->operator)
    {
    case '+':
        return builder.CreateFAdd(leftHandSide, rightHandSide, "addf");

    case '-':
        return builder.CreateFSub(leftHandSide, rightHandSide, "subf");

    case '*':
        return builder.CreateFMul(leftHandSide, rightHandSide, "mulf");

    case '<':
        Value *result = builder.CreateFCmp(leftHandSide, rightHandSide, "cmpf");
        return builder.CreateUIToFP(result, Type::getDoubleTy(ctx), "bool_cmp");

    case '=':
        return builder.CreateStore(leftHandSide, rightHandSide);

    default:
        return logErrorValue("Unknown operation");
    }
}

