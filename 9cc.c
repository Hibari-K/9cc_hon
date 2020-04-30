#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

#include "9cc.h"

// Current token
Token *token;

// input
char *user_input;

// error function
void error_at(char *loc, char *fmt, ...){

    va_list ap; // variable array
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    // above code is equivalent to C like so: 
    /*
        int i;
        for(i=0; i<pos; i++)
            fprintf(stderr, " ");
    */
    fprintf(stderr, "^ ");
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
        error_at(token->str, "Expected OP is not '%c'", op);
    token = token->next;
}

// the same as "expect" except of number
int expect_number(){

    /*debug*/
    //printf("expect_number_val: %d\n", token->val);
    //printf("expect_number_str: %s\n", token->str);

    if(token->kind != TK_NUM)
        error_at(token->str, "Token is not a number");
    
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

        if(*p == '+' || *p == '-' || *p == '*' || *p == '/'){
            cur = new_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        //error_at(token->str, "cannot tokenize");
        fprintf(stderr, "cannot tokenize '%s'\n", p);
        exit(1);
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}


/** Node **/
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}

// create new num node
Node *new_node_num(int val){

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;

    return node;
}


// primary = "(" expr ")" | num
Node *primary(){

    // if next token is "(", the expected node is '"(" expr ")"'
    if(expectAndConsume('(')){
        Node *node = expr();
        expect(')');
        return node;
    }

    // otherwise, it should be num
    return new_node_num(expect_number());
}

// unary = ("+" | "-")? primary
Node* unary(){
    if(expectAndConsume('+'))
        return primary(); // +1 = 1
    if(expectAndConsume('-'))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

//    mul = primary ("*" primary | "/" primary)*
// => mul = unary ("*" unary | "/" unary)*
Node *mul(){

    Node *node = unary();

    for(;;){
        if(expectAndConsume('*'))
            node = new_node(ND_MUL, node, unary());
        else if(expectAndConsume('/'))
            node = new_node(ND_DIV, node, unary());
        else 
            return node;
    }
}

// expr = mul ("+" mul | "-" mul)*
Node *expr(){
    
    Node *node = mul();

    for(;;){
        if(expectAndConsume('+')){
            node = new_node(ND_ADD, node, mul());
        }
        else if(expectAndConsume('-')){
            node = new_node(ND_SUB, node, mul());
        }
        else
            return node;
    }
}

void codegen(Node *node){
    if(node->kind == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    codegen(node->lhs);
    codegen(node->rhs);

    puts("    pop rdi");
    puts("    pop rax");

    switch(node->kind){

        case ND_ADD:
            puts("    add rax, rdi");
            break;
        case ND_SUB:
            puts("    sub rax, rdi");
            break;
        case ND_MUL:
            puts("    imul rax, rdi");
            break;
        case ND_DIV:
            puts("    cqo");
            puts("    idiv rdi");
            break;
    }

    puts("    push rax");
}



int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "invalid number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    //debug_token();

    Node *node = expr();

    printf(".intel_syntax noprefix\n" \
           ".global main\n" \
           "main:\n");
    
    codegen(node);

    puts("    pop rax");
    puts("    ret");

    return 0;
}