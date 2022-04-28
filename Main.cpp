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
        printf("Parsed a top level expressoin.\n");
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
            getNextToken();
            break;
        case tok_def:
            handleDefinition();
            break;
        default:
            handleTopLevelExpression();
            break;
        }
        printf("ready> ", curToken);
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