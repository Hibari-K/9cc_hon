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
    ND_ADDR, // &
    ND_DEREF, // *
    ND_VAR, // local variable
    ND_RETURN, //return
    ND_IF,
    ND_FOR,
    ND_WHILE,
    ND_BLOCK, // { ... } block
    ND_FUNCALL, // function call
    ND_EXPR_STMT,
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_NUM,
} NodeKind;

typedef struct Node Node;

typedef struct Var Var;
struct Var{
    char *name;
    int offset;
};

// local variable
typedef struct VarList VarList;
struct VarList{
    VarList *next;
    Var *var;
};

extern VarList *locals;

typedef struct Function Function;
struct Function{
    Function *next;
    char *name;
    VarList *params;

    Node *node;
    VarList *locals;
    int stack_size;
};

// AST node type
struct Node{
    NodeKind kind;
    Node *next;
    Node *lhs; // left hand side
    Node *rhs; // right hand side
    long val; // used for ND_NUM
    Var *var;
    //char name; // used for ND_VAR
    //int offset; // used for LD_VAR

    // if, while, and for
    Node *cond;
    Node *then;
    Node *els;

    Node *init; // for
    Node *inc; // for

    Node *body; // block

    char *funcname; // function
    Node *args; // function
};





// codegen
Node* args();
Node* primary();
Node* unary();
Node* mul();
Node* add();
Node* relational();
Node* equality();
Node* assign();
Node* expr();
Node* stmt();
Function* function();
Function* program();

// parse
void error(char *str);
void error_at(char *loc, char *fmt, ...);
bool expectAndConsume(char *op);
Token *consumeIdent();
void debug_token();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Var *find_lvar(Token *tok);
Token *tokenize(char *p);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);


// codegen
void codegen(Node *node);
void codegenFirst(Function *prog);