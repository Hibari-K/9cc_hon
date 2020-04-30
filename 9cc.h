
// kind of token
typedef enum{
    TK_RESERVED, // symbol e.g., '+'
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

// token
struct Token{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};