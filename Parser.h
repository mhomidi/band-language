#include "AST.h"

static int curToken;

static int getNextToken();

unique_ptr<ExpressionAST> logError(const char *errorStr);
unique_ptr<PrototypeAST> logErrorProto(const char *errorStr);

static unique_ptr<ExpressionAST> parseNumberExpr();
static unique_ptr<ExpressionAST> parseParenthesesExpr();
static unique_ptr<ExpressionAST> parseIdentifierExpr();
static unique_ptr<ExpressionAST> parsePrimary();
static unique_ptr<ExpressionAST> parseExpression();
static unique_ptr<ExpressionAST> parseBinaryOpRHS(int opCode, unique_ptr<ExpressionAST> lhs);
static unique_ptr<PrototypeAST> parsePrototype();
static unique_ptr<FunctionExpressionAST> parseDefinition();
static unique_ptr<FunctionExpressionAST> parseTopLevelExpression();

static int getTokPrecedence();

void initialBinOpPrecs();
