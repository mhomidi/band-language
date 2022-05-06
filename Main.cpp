#include "Parser.h"
#include "Lexer.h"
#include "Common.h"

void handleDefinition()
{
    if (auto funcAST = parseDefinition())
    {
        if (auto *funcIR = funcAST->codegen())
        {
            printf("Read function definition: ");
            funcIR->print(errs());
            printf("\n");
        }
    }
    else
        getNextToken();
}

void handleTopLevelExpression()
{
    if (auto topLevelExp = parseTopLevelExpression())
    {
        if (auto *topLevelIR = topLevelExp->codegen())
        {
            printf("Read top level expression: ");
            topLevelIR->print(errs());
            printf("\n");
        }
    }
    else
        getNextToken();
}

void mainLoop()
{
    while (true)
    {
        switch (curToken)
        {
        case tok_eof:
            return;
        case ';':
            break;
        case tok_def:
            handleDefinition();
            break;
        default:
            handleTopLevelExpression();
            break;
        }
        printf("ready> ");
        getNextToken();
    }
}

int main()
{
    initialBinOpPrecs();

    printf("ready> ");
    getNextToken();

    mainLoop();

    return 0;
}