#include <string>
#include <memory>
#include <vector>

using namespace std;

class ExpressionAST
{
public:
    virtual ~ExpressionAST() {}
};

class NumberExpAST : public ExpressionAST
{
    double value;

public:
    NumberExpAST(double val) : value(val) {}
};

class VariableExpAST : public ExpressionAST
{
    string name;

public:
    VariableExpAST(string name) : name(name) {}
};

class BinaryExpAST : public ExpressionAST
{
    char operation;
    unique_ptr<ExpressionAST> lhs, rhs;

public:
    BinaryExpAST(char op, unique_ptr<ExpressionAST> lhs,
                 unique_ptr<ExpressionAST> rhs) : operation(op), lhs(move(lhs)), rhs(move(rhs)) {}
};

class CallExpression : public ExpressionAST
{
    string funcName;
    vector<unique_ptr<ExpressionAST>> args;

public:
    CallExpression(string funcName, vector<unique_ptr<ExpressionAST>> args) : funcName(funcName),
                                                                              args(move(args)) {}
};

class PrototypeAST : public ExpressionAST
{
    string name;
    vector<unique_ptr<ExpressionAST>> args;

public:
    PrototypeAST(string funcName, vector<unique_ptr<ExpressionAST>> args) : name(funcName),
                                                                         args(move(args)) {}
};

class FunctionExpressionAST : public ExpressionAST
{
    unique_ptr<PrototypeAST> prototype;
    unique_ptr<ExpressionAST> body;

public:
    FunctionExpressionAST(unique_ptr<PrototypeAST> prototype,
                       unique_ptr<ExpressionAST> body) : prototype(move(prototype)), body(move(body)) {}
};