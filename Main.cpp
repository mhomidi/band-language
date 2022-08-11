#include "Parser.h"
#include "Lexer.h"
#include "Common.h"
#include "llvm/IR/Function.h"

llvm::ExitOnError exitOnError;

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
            auto runtime = myJIT->getMainJITDylib().createResourceTracker();

            auto threadSafeModule = llvm::orc::ThreadSafeModule(std::move(module), std::move(ctx));
            exitOnError(myJIT->addModule(std::move(threadSafeModule), runtime));
            initialModulesAndPassManager();

            auto exprSymbol = exitOnError(myJIT->lookup("__anon_expr"));

            double (*FP)() = (double (*)())(intptr_t)exprSymbol.getAddress();
            fprintf(stderr, "Evaluated to %f\n", FP());

            exitOnError(runtime->remove());
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
    initializeNativeTargets();
    initialBinOpPrecs();

    printf("ready> ");
    getNextToken();

    myJIT = exitOnError(llvm::orc::HadiJIT::Create());

    initialModulesAndPassManager();

    mainLoop();

    return 0;
}