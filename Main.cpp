#include "Parser.h"
#include "Lexer.h"
#include "Common.h"

void handleDefinition()
{
    if (parseDefinition())
        fprintf(stderr, "Parsed a function definition.\n");
    else
        getNextToken();
}

void handleTopLevelExpression()
{
    if (parseTopLevelExpression())
        fprintf(stderr, "Parsed a top level expressoin.\n");
    else
        getNextToken();
}

void mainLoop()
{
    while (true)
    {
        fprintf(stderr, "ready> ");
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
    }
}

int main()
{
    initialBinOpPrecs();

    fprintf(stderr, "ready> ");
    getNextToken();

    mainLoop();

    return 0;
}