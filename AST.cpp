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

using namespace llvm;

static std::unique_ptr<LLVMContext> ctx;
static std::unique_ptr<IRBuilder<>> builder;
static std::unique_ptr<Module> module;
static map<std::string, Value *> namedValues;

void initialModules()
{
    ctx = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("myModule", *ctx);

    builder = std::make_unique<IRBuilder<>>(*ctx);
}

Value *logErrorValue(const char *errStr)
{
    logError(errStr);
    return nullptr;
}

Value *NumberExpAST::codegen()
{
    return ConstantFP::get(*ctx, APFloat(this->value));
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

    switch (this->op)
    {
    case '+':
        return builder->CreateFAdd(leftHandSide, rightHandSide, "addres");

    case '-':
        return builder->CreateFSub(leftHandSide, rightHandSide, "subres");

    case '*':
        return builder->CreateFMul(leftHandSide, rightHandSide, "mulres");

    case '<':
    {
        Value *result = builder->CreateFCmpULT(leftHandSide, rightHandSide, "cmpres");
        return builder->CreateUIToFP(result, Type::getDoubleTy(*ctx), "comres");
    }

    case '=':
        return builder->CreateStore(leftHandSide, rightHandSide);

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

    return builder->CreateCall(callFunction, argValues, "callres");
}

Function *PrototypeAST::codegen()
{
    std::vector<Type *> doubles(this->args.size(), Type::getDoubleTy(*ctx));
    FunctionType *functionType = FunctionType::get(Type::getDoubleTy(*ctx), doubles, false);
    Function *function = Function::Create(functionType,
                                          Function::ExternalLinkage, this->name, module.get());

    unsigned index = 0;
    for (auto &arg : function->args())
        arg.setName(this->args[index++]);

    return function;
}

Function *FunctionExpressionAST::codegen()
{
    Function *function = module->getFunction(this->prototype->getName());

    if (!function)
    {
        function = this->prototype->codegen();
        if (!function)
            return nullptr;
    }

    if (function->empty())
        return (Function *)logErrorValue("Function can not be redefine");

    BasicBlock *basicBlock = BasicBlock::Create(*ctx, "entry_block", function);
    builder->SetInsertPoint(basicBlock);

    namedValues.clear();
    for (auto &arg : function->args())
        namedValues[arg.getName().str()] = &arg;

    if (Value *returnValue = this->body->codegen())
    {
        builder->CreateRet(returnValue);
        verifyFunction(*function);

        return function;
    }

    function->eraseFromParent();
    return nullptr;
}