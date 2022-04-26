#include <iostream>
#include "Lexer.h"
#include "Common.h"

std::string identifierStr;
double numVal;

using namespace std;

int getToken()
{
    int lastChar = ' ';

    while (isspace(lastChar))
        lastChar = getchar();

    if (isalpha(lastChar))
    {
        identifierStr = lastChar;
        while (isalnum(lastChar = getchar()))
            identifierStr += lastChar;

        if (identifierStr == "def")
            return tok_def;

        return tok_identifier;
    }
    else if (isdigit(lastChar) || lastChar == '.')
    {
        string numStr = "";
        bool isDot = false;

        if (lastChar == '.')
            isDot = true;

        do
        {
            numStr += lastChar;
            lastChar = getchar();
        } while (isdigit(lastChar) || (lastChar == '.' && !isDot));

        numVal = strtod(numStr.c_str(), 0);
        return tok_number;
    }
    else if (lastChar == '#')
    {
        do
            lastChar = getchar();
        while (lastChar != EOF || lastChar != '\n' || lastChar != '\r');

        if (lastChar != EOF)
            return getToken();
    }
    else if (lastChar == EOF)
        return tok_eof;

    int thisChar = lastChar;
    lastChar = getchar();
    return thisChar;
}