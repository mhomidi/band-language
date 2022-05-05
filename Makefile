CC = clang++
CFLAGS = -g
LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`
RM = rm -rf

a.out: Main.o Lexer.o Parser.o AST.o
	$(CC) $(CFLAGS) -o a.out Main.o Lexer.o Parser.o AST.o $(LLVM_FLAGS)

Parser.o: Parser.cpp Parser.h Lexer.h AST.h Common.h
	$(CC) $(CFLAGS) -c Parser.cpp $(LLVM_FLAGS)

Lexer.o: Lexer.cpp Lexer.h Common.h
	$(CC) $(CFLAGS) -c Lexer.cpp $(LLVM_FLAGS)

Main.o: Main.cpp Parser.h Lexer.h Common.h
	$(CC) $(CFLAGS) -c Main.cpp $(LLVM_FLAGS)

AST.o: AST.cpp Parser.h AST.h
	$(CC) $(CFLAGS) -c AST.cpp $(LLVM_FLAGS)

clean:
	$(RM) *.o a.out