enum Token
{
    tok_eof = -1,
    tok_def = -2,
    tok_identifier = -3,
    tok_number = -4
};

static std::string identifierStr;
static double numVal;

static inline int getToken();