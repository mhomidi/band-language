#include "Parser.h"
#include "Lexer.h"
#include "Common.h"

void handleDefinition()
{
    if (parseDefinition())
        printf("Parsed a function definition.\n");
    else
        getNextToken();
}

void handleTopLevelExpression()
{
    if (parseTopLevelExpression())
        printf("Parsed a top level expression.\n");
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