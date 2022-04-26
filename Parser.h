#include "AST.h"

int getNextToken();

unique_ptr<ExpressionAST> logError(const char *errorStr);
unique_ptr<PrototypeAST> logErrorProto(const char *errorStr);

unique_ptr<ExpressionAST> parseNumberExpr();
unique_ptr<ExpressionAST> parseParenthesesExpr();
unique_ptr<ExpressionAST> parseIdentifierExpr();
unique_ptr<ExpressionAST> parsePrimary();
unique_ptr<ExpressionAST> parseExpression();
unique_ptr<ExpressionAST> parseBinaryOpRHS(int opCode, unique_ptr<ExpressionAST> lhs);
unique_ptr<PrototypeAST> parsePrototype();
unique_ptr<FunctionExpressionAST> parseDefinition();
unique_ptr<FunctionExpressionAST> parseTopLevelExpression();

int getTokPrecedence();

void initialBinOpPrecs();
