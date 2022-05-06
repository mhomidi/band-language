#include "llvm/IR/Value.h"
#include <string>
#include <memory>
#include <vector>

using namespace std;
using namespace llvm;

void initialModules();

class ExpressionAST
{
public:
    virtual ~ExpressionAST() {}
    virtual Value *codegen() = 0;
};

class NumberExpAST : public ExpressionAST
{
    double value;

public:
    NumberExpAST(double val) : value(val) {}
    Value *codegen() override;
};

class VariableExpAST : public ExpressionAST
{
    string name;

public:
    VariableExpAST(string name) : name(name) {}
    Value *codegen() override;
};

class BinaryExpAST : public ExpressionAST
{
    char op;
    unique_ptr<ExpressionAST> lhs, rhs;

public:
    BinaryExpAST(char op, unique_ptr<ExpressionAST> lhs,
                 unique_ptr<ExpressionAST> rhs) : op(op), lhs(move(lhs)), rhs(move(rhs)) {}
    Value *codegen() override;
};

class CallExpressionAST : public ExpressionAST
{
    string funcName;
    vector<unique_ptr<ExpressionAST>> args;

public:
    CallExpressionAST(string funcName, vector<unique_ptr<ExpressionAST>> args) : funcName(funcName),
                                                                                 args(move(args)) {}
    Value *codegen() override;
};

class PrototypeAST
{
    string name;
    vector<string> args;

public:
    PrototypeAST(string funcName, vector<string> args) : name(funcName),
                                                         args(move(args)) {}
    Function *codegen();
    string getName() { return this->name; }
};

class FunctionExpressionAST
{
    unique_ptr<PrototypeAST> prototype;
    unique_ptr<ExpressionAST> body;

public:
    FunctionExpressionAST(unique_ptr<PrototypeAST> prototype,
                          unique_ptr<ExpressionAST> body) : prototype(move(prototype)), body(move(body)) {}
    Function *codegen();
};