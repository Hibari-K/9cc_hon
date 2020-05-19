#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>


// kind of token
typedef enum{
    TK_RESERVED, // symbol e.g., '+'
    TK_IDENT, // variable
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
    int len; // token length
};

extern char *user_input;
extern Token *token;


// kind of AST node
typedef enum{
    ND_ADD, 
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_ASSIGN, // =
    ND_VAR, // local variable
    ND_RETURN, //return
    ND_IF,
    ND_FOR,
    ND_WHILE,
    ND_EXPR_STMT,
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_NUM,
} NodeKind;

typedef struct Node Node;

// local variable
typedef struct LVar LVar;
struct LVar{
    LVar *next;
    char *name;
    int offset;
};

extern LVar *locals;

// AST node type
struct Node{
    NodeKind kind;
    Node *next;
    Node *lhs; // left hand side
    Node *rhs; // right hand side
    long val; // used for ND_NUM
    LVar *lvar;
    char name; // used for ND_VAR
    int offset; // used for LD_VAR

    // if
    Node *cond;
    Node *then;
    Node *els;
};





// codegen
Node* primary();
Node* unary();
Node* mul();
Node* add();
Node* relational();
Node* equality();
Node* assign();
Node* expr();
Node* stmt();
Node* program();

// parse
void error(char *str);
void error_at(char *loc, char *fmt, ...);
bool expectAndConsume(char *op);
Token *consumeIdent();
void debug_token();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
LVar *find_lvar(Token *tok);
Token *tokenize(char *p);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);


// codegen
void codegen(Node *node);
void codegenFirst(Node *node);