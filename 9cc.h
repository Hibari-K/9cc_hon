
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


// kind of AST node
typedef enum{
    ND_ADD, 
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

// AST node type
struct Node{
    NodeKind kind;
    Node *lhs; // left hand side
    Node *rhs; // right hand side
    int val;
};


// codegen
Node* primary();
Node* mul();
Node* expr();