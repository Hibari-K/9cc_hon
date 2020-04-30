#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

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

// Current token
Token *token;

// error function
void error(char *fmt, ...){

    va_list ap; // variable array
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    exit(1);
}

// eat symbol and return true
// otherwise, do not eat and return false
bool expectAndConsume(char op){

    if(token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    
    token = token->next;
    return true;
    
}

void debug_token(){

    Token *t = token;
    for(; t->kind!=TK_EOF; t=t->next){
        printf("token: %s %d\n", t->str, t->val);
    }
}

// if expected token, eat token
// otherwise, alert error
void expect(char op){

    if(token->kind != TK_RESERVED || token->str[0] != op)
        error("Expected OP is not '%c'", op);
    token = token->next;
}

// the same as "expect" except of number
int expect_number(){

    /*debug*/
    //printf("expect_number_val: %d\n", token->val);
    //printf("expect_number_str: %s\n", token->str);

    if(token->kind != TK_NUM)
        error("Token is not a number");
    
    //int val = token->val;
    int val = strtol(token->str, NULL, 10);
    token = token->next;

    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

// create new token and append to cur
Token *new_token(TokenKind kind, Token *cur, char *str){

    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;

    return tok;
}

Token *tokenize(char *p){
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p){

        // skip white space
        if(isspace(*p)){
            p++;
            continue;
        }

        /* debug */
        //printf("tokenize: %s\n", p);

        if(*p == '+' || *p == '-'){
            cur = new_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("cannot tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}



int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "invalid number of arguments\n");
        return 1;
    }

    token = tokenize(argv[1]);
    //debug_token();

    printf(".intel_syntax noprefix\n" \
           ".global main\n" \
           "main:\n");
    
    // the first token is expected number
    printf("    mov rax, %d\n", expect_number());
    
    while(!at_eof()){

        if(expectAndConsume('+')){
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");

    return 0;
}