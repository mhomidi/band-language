#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/Error.h"
#include <map>
#include "Parser.h"
#include "Common.h"

using namespace llvm;

std::unique_ptr<llvm::orc::HadiJIT> myJIT;
std::unique_ptr<LLVMContext> ctx;
static std::unique_ptr<IRBuilder<>> builder;
std::unique_ptr<Module> module;
static map<std::string, Value *> namedValues;
static std::unique_ptr<legacy::FunctionPassManager> functionPassManager;

void initialModulesAndPassManager()
{
    ctx = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("myModule", *ctx);
    module->setDataLayout(myJIT->getDataLayout());

    builder = std::make_unique<IRBuilder<>>(*ctx);

    functionPassManager = make_unique<legacy::FunctionPassManager>(module.get());

    functionPassManager->add(createInstructionCombiningPass());
    functionPassManager->add(createReassociatePass());
    functionPassManager->add(createGVNPass());
    functionPassManager->add(createCFGSimplificationPass());

    functionPassManager->doInitialization();
}

void initializeNativeTargets()
{
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
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

    if (!function->empty())
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

Value *IfExpressionAST::codegen()
{
    Value *conditionValue = this->cond->codegen();

    if (!conditionValue)
        return nullptr;

    conditionValue = builder->CreateFCmpONE(
        conditionValue, ConstantFP::get(*ctx, APFloat(0.0)), "ifcond");

    Function *function = builder->GetInsertBlock()->getParent();
    BasicBlock *thenBasicBlock = BasicBlock::Create(*ctx, "then_stmt", function);
    BasicBlock *elseBasicBlock = BasicBlock::Create(*ctx, "else_stmt");
    BasicBlock *mergeBasicBlock = BasicBlock::Create(*ctx, "merge_stmt");

    builder->CreateCondBr(conditionValue, thenBasicBlock, elseBasicBlock);

    builder->SetInsertPoint(thenBasicBlock);

    Value *thenValue = this->thenStmt->codegen();
    if (!thenValue)
        return nullptr;

    builder->CreateBr(mergeBasicBlock);
    thenBasicBlock = builder->GetInsertBlock();

    function->getBasicBlockList().push_back(elseBasicBlock);
    builder->SetInsertPoint(elseBasicBlock);

    Value *elseValue = this->elseStmt->codegen();
    if (!elseValue)
        return nullptr;

    builder->CreateBr(mergeBasicBlock);
    elseBasicBlock = builder->GetInsertBlock();

    function->getBasicBlockList().push_back(mergeBasicBlock);
    builder->SetInsertPoint(mergeBasicBlock);

    PHINode *phiNode = builder->CreatePHI(Type::getDoubleTy(*ctx), 2, "iftmp");

    phiNode->addIncoming(thenValue, thenBasicBlock);
    phiNode->addIncoming(elseValue, elseBasicBlock);

    return phiNode;
}

Value *ForExpressionAST::codegen()
{
    Value *startValue = this->start->codegen();
    if (!startValue)
        return nullptr;

    Function *function = builder->GetInsertBlock()->getParent();
    BasicBlock *preHeaderBasicBlock = builder->GetInsertBlock();
    BasicBlock *loopBasicBlock = BasicBlock::Create(*ctx, "loop", function);
    builder->CreateBr(loopBasicBlock);

    builder->SetInsertPoint(loopBasicBlock);

    PHINode *phiVariable = builder->CreatePHI(Type::getDoubleTy(*ctx), 2, this->varName.c_str());
    phiVariable->addIncoming(startValue, preHeaderBasicBlock);

    Value *oldValue = namedValues[this->varName];
    namedValues[this->varName] = phiVariable;

    if (!this->body->codegen())
        return nullptr;

    Value *stepValue = nullptr;
    if (this->step)
    {
        stepValue = this->step->codegen();
        if (!stepValue)
            return nullptr;
    }
    else
    {
        stepValue = ConstantFP::get(*ctx, APFloat(1.0));
    }

    Value *nextValue = builder->CreateFAdd(phiVariable, stepValue, "nextval");

    Value *endCondition = this->end->codegen();
    if (!endCondition)
        return nullptr;

    endCondition = builder->CreateFCmpONE(endCondition, ConstantFP::get(*ctx, APFloat(0.0)), "loopcond");
    BasicBlock *loopEndBasicBlock = builder->GetInsertBlock();
    BasicBlock *afterloopBasicBlock = BasicBlock::Create(*ctx, "afterloop", function);

    builder->CreateCondBr(endCondition, loopBasicBlock, afterloopBasicBlock);

    builder->SetInsertPoint(afterloopBasicBlock);

    phiVariable->addIncoming(nextValue, loopEndBasicBlock);

    if (oldValue)
        namedValues[this->varName] = oldValue;
    else
        namedValues.erase(this->varName);

    return Constant::getNullValue(Type::getDoubleTy(*ctx));
}