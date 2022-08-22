#include "Parser.h"
#include "Lexer.h"
#include "Common.h"
#include <map>

int curToken;
static map<char, int> binOperatorPrecedence;

int getNextToken()
{
    curToken = getToken();
    // printf("%i %s\n", curToken, identifierStr.c_str()); // For debug
    return curToken;
}

unique_ptr<ExpressionAST> logError(const char *errorStr)
{
    printf("Error: %s\n", errorStr);
    return nullptr;
}

unique_ptr<PrototypeAST> logErrorProto(const char *errorStr)
{
    logError(errorStr);
    return nullptr;
}

unique_ptr<ExpressionAST> parseNumberExpr()
{
    auto result = make_unique<NumberExpAST>(numVal);
    getNextToken();
    return move(result);
}

unique_ptr<ExpressionAST> parseParenthesesExpr()
{
    getNextToken();
    auto value = parseExpression();
    if (!value)
        return nullptr;

    if (curToken != ')')
        return logError("Expected ')'");

    getNextToken();
    return value;
}

unique_ptr<ExpressionAST> parseIdentifierExpr()
{
    string nameId = identifierStr;
    getNextToken();

    if (curToken != '(')
        return make_unique<VariableExpAST>(nameId);

    getNextToken();
    vector<unique_ptr<ExpressionAST>> args;

    if (curToken != ')')
    {
        while (true)
        {
            if (auto arg = parseExpression())
                args.push_back(move(arg));
            else
                return nullptr;

            if (curToken == ')')
                break;

            if (curToken != ',')
                return logError("Expected ','");
            getNextToken();
        }
    }

    getNextToken();

    return make_unique<CallExpressionAST>(nameId, move(args));
}

unique_ptr<ExpressionAST> parsePrimary()
{
    switch (curToken)
    {
    case tok_identifier:
        return parseIdentifierExpr();
    case tok_number:
        return parseNumberExpr();
    case '(':
        return parseParenthesesExpr();
    case tok_if:
        return parseIfExpresion();
    case tok_for:
        return parseForExpresion();
    default:
        return logError("Unknown token");
    }
}

int getTokPrecedence()
{
    if (!isascii(curToken))
        return -1;

    int tokPrec = binOperatorPrecedence[curToken];
    if (tokPrec < 0)
        return -1;

    return tokPrec;
}

void initialBinOpPrecs()
{
    binOperatorPrecedence['='] = 1;
    binOperatorPrecedence['<'] = 10;
    binOperatorPrecedence['-'] = 20;
    binOperatorPrecedence['+'] = 20;
    binOperatorPrecedence['*'] = 40;
}

unique_ptr<ExpressionAST> parseBinaryOpRHS(int opCodePrec, unique_ptr<ExpressionAST> lhs)
{
    while (true)
    {
        int tokPrec = getTokPrecedence();

        if (tokPrec <= opCodePrec)
            return lhs;

        int binOp = curToken;
        getNextToken();

        auto rhs = parsePrimary();
        if (!rhs)
            return nullptr;

        int nextPrec = getTokPrecedence();
        if (tokPrec <= nextPrec)
        {
            rhs = parseBinaryOpRHS(tokPrec + 1, move(rhs));
            if (!rhs)
                return nullptr;
        }

        lhs = make_unique<BinaryExpAST>(binOp, move(lhs), move(rhs));
    }
}

unique_ptr<ExpressionAST> parseExpression()
{
    auto lhs = parsePrimary();
    if (!lhs)
        return nullptr;

    return parseBinaryOpRHS(0, move(lhs));
}

unique_ptr<PrototypeAST> parsePrototype()
{
    if (curToken != tok_identifier)
        return logErrorProto("Expected Function name in ");

    string funcName = identifierStr;
    getNextToken();

    if (curToken != '(')
        return logErrorProto("Expected '(' in prototype");

    vector<string> argNames;
    while (getNextToken() == tok_identifier)
        argNames.push_back(identifierStr);
    if (curToken != ')')
        return logErrorProto("Expected ')' in prototype");

    getNextToken();

    return make_unique<PrototypeAST>(funcName, move(argNames));
}

unique_ptr<FunctionExpressionAST> parseDefinition()
{
    getNextToken();

    auto prototype = parsePrototype();
    if (!prototype)
        return nullptr;

    if (auto body = parseExpression())
        return make_unique<FunctionExpressionAST>(move(prototype), move(body));

    return nullptr;
}

unique_ptr<FunctionExpressionAST> parseTopLevelExpression()
{
    if (auto exp = parseExpression())
    {
        auto prototype = make_unique<PrototypeAST>("__anon_expr", vector<string>());
        return make_unique<FunctionExpressionAST>(move(prototype), move(exp));
    }

    return nullptr;
}

unique_ptr<ExpressionAST> parseIfExpresion()
{
    getNextToken();

    auto condition = parseExpression();
    if (!condition)
        return nullptr;

    if (curToken != tok_then)
        return logError("expected then");
    getNextToken();

    auto thenStmt = parseExpression();
    if (!thenStmt)
        return nullptr;

    if (curToken != tok_else)
        return logError("expected else");
    getNextToken();

    auto elseStmt = parseExpression();
    if (!elseStmt)
        return nullptr;

    return make_unique<IfExpressionAST>(move(condition), move(thenStmt), move(elseStmt));
}

unique_ptr<ExpressionAST> parseForExpresion()
{
    getNextToken();

    if (curToken != tok_identifier)
        return logError("expected identifier after for");

    string idName = identifierStr;
    getNextToken();

    if (curToken != '=')
        return logError("expected '='after identifier");
    getNextToken();

    auto start = parseExpression();
    if (!start)
        return nullptr;

    if (curToken != ',')
        return logError("expected ',' after initialization");
    getNextToken();

    auto end = parseExpression();
    if (!end)
        return nullptr;

    unique_ptr<ExpressionAST> step;
    if (curToken == ',')
    {
        getNextToken();

        step = parseExpression();
        if (!step)
            return nullptr;
    }

    if (curToken != tok_in)
        return logError("expected 'in' at the end of the loop");

    getNextToken();

    auto body = parseExpression();
    if (!body)
        return nullptr;

    return make_unique<ForExpressionAST>(idName, move(start), move(end), move(step), move(body));
}