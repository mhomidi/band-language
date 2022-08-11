#include <string>
#include "myJIT.h"
#include "llvm/IR/Module.h"

extern int curToken;

extern std::string identifierStr;
extern double numVal;
extern std::unique_ptr<llvm::orc::HadiJIT> myJIT;
extern std::unique_ptr<llvm::Module> module;
extern std::unique_ptr<llvm::LLVMContext> ctx;
extern llvm::ExitOnError exitOnError;