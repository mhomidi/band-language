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

Value *CallExpressionAST::codegen()
{
    Function *callFunction = module->getFunction(this->funcName);
    if (!callFunction)
        return logErrorValue("Unknown function");

    if (callFunction->arg_size() != this->args.size())
        return logErrorValue("Incorrect number of arguments");

    std::vector<Value *> argValues;
    for (int i = 0; i < callFunction->arg_size(); i++)
    {
        argValues.push_back(this->args[i]->codegen());
        if (!argValues.back())
            return nullptr;
    }

    return builder.CreateCall(callFunction, argValues, "callf");
}

Function *PrototypeAST::codegen()
{
    std::vector<Type *> doubles(this->args.size(), Type::getDoubleTy(ctx));
    FunctionType *functionType = FunctionType::get(Type::getDoubleTy(ctx), doubles, false);
    Function *function = Function::Create(functionType,
                                          Function::ExternalLinkage, this->name, module.get());

    unsigned index = 0;
    for (auto &arg : function->args())
        arg.setName(this->args[index++]);

    return function;
}

Function *FunctionExpressionAST::codegen()
{
    Function *function = module->getFunction(this->prototype->name);

    if (!function)
    {
        function = this->prototype->codegen();
        if (!function)
            return nullptr;
    }

    if (function->empty())
        return dynamic_cast<Function *>(logErrorValue("Function can not be redefine"));

    BasicBlock *basicBlock = BasicBlock::Create(ctx, "entry_block", function);
    builder.SetInsertPoint(basicBlock);

    namedValues.clear();
    for (auto &arg : function->args())
        namedValues[arg.getName] = arg;

    if (Value *returnValue = this->body->codegen())
    {
        builder.CreateRet(returnValue);
        verifyFunction(function);

        return function;
    }

    function->eraseFromParent();
    return nullptr;
}