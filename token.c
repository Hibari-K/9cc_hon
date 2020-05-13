#include "9cc.h"




/****   Check Tool    ****/

bool is_alpha(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

bool is_alnum(char c){
    return is_alpha(c) || ('0' <= c && c <= '9');
}



/***  Error  ***/

void error(char *str){

    puts(str);
    exit(1);
}

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




/****   Expect Token   ****/


// eat symbol and return true
// otherwise, do not eat and return false
bool expectAndConsume(char *op){

    if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        return false;
    
    token = token->next;
    return true;
    
}

Token *consumeIdent(){

    if(token->kind != TK_IDENT)
        return NULL;
    
    Token *t = token;
    token = token->next;

    return t; //eat and return 
}

void debug_token(){

    Token *t = token;
    for(; t->kind!=TK_EOF; t=t->next){
        printf("token: %s %d\n", t->str, t->val);
    }
}

// if expected token, eat token
// otherwise, alert error
void expect(char *op){

    if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "Expected OP is not '%s'", op);
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




/****   Token    ****/


// create new token and append to cur
Token *new_token(TokenKind kind, Token *cur, char *str, int len){

    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    tok->len = len;

    return tok;
}


bool startswith(char *p, char *q){
    return memcmp(p, q, strlen(q)) == 0;
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


        // keywords
        // the latter avoids variable similar to "return" (e.g., "return1")
        if(startswith(p, "return") && !is_alnum(p[6])){ 
            
            //puts("tokenize: return");

            cur = new_token(TK_RESERVED, cur, p, 6);
            p += 6;
            continue;
        }


        /* debug */
        //printf("tokenize: %s\n", p);
        // multi-letter punctuator
        if(startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")){

            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // Single-letter punctuator
        //if(strchr("+-*/()<>", *p)){
        if(ispunct(*p)){
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }


        if(isdigit(*p)){

            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p; // q is tmp value for len
            cur->val = strtol(p, &p, 10);
            
            // debug
            //printf("tokenize: num %d\n", cur->val);

            cur->len = p - q;
            continue;
        }

        // the variable length is 1 so far
        if('a' <= *p && *p <= 'z'){
            cur = new_token(TK_IDENT, cur, p++, 1);
            continue;
        }

        //error_at(token->str, "cannot tokenize");
        fprintf(stderr, "cannot tokenize '%s'\n", p);
        exit(1);
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}