#include <iostream>

using namespace std;

enum Token
{
    tok_eof = -1,
    tok_def = -2,
    tok_identifier = -3,
    tok_number = -4
};

static string identifierStr;
static double numVal;

static int getToken()
{
    static int lastChar = ' ';

    while (isspace(lastChar))
        lastChar = getchar();

    if (isalpha(lastChar))
    {
        identifierStr = lastChar;
        while (isalnum(lastChar))
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
        } while (isdigit(lastChar) || (lastChar == '.' && !lastChar));

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