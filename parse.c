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
bool expectAndConsume(char *op){

    if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
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

        /* debug */
        //printf("tokenize: %s\n", p);
        // multi-letter punctuator
        if(startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")){

            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // Single-letter punctuator
        if(strchr("+-*/()<>", *p)){
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }


        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p; // q is tmp value for len
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        //error_at(token->str, "cannot tokenize");
        fprintf(stderr, "cannot tokenize '%s'\n", p);
        exit(1);
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}


/** Node **/
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
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
    if(expectAndConsume("(")){
        Node *node = expr();
        expect(")");
        return node;
    }

    // otherwise, it should be num
    return new_node_num(expect_number());
}

// unary = ("+" | "-")? primary
Node* unary(){
    if(expectAndConsume("+"))
        return primary(); // +1 = 1
    if(expectAndConsume("-"))
        return new_binary(ND_SUB, new_node_num(0), primary());
    return primary();
}

//    mul = primary ("*" primary | "/" primary)*
// => mul = unary ("*" unary | "/" unary)*
Node *mul(){

    Node *node = unary();

    for(;;){
        if(expectAndConsume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if(expectAndConsume("/"))
            node = new_binary(ND_DIV, node, unary());
        else 
            return node;
    }
}


//add        = mul ("+" mul | "-" mul)*
Node* add(){

    Node *node = mul();

    for(;;){
        if(expectAndConsume("+"))
            node = new_binary(ND_ADD, node, mul());
        if(expectAndConsume("-"))
            node = new_binary(ND_SUB, node, mul());
        return node;
    }
}

//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational(){

    Node *node = add();

    for(;;){
        if(expectAndConsume("<"))
            node = new_binary(ND_LT, node, add());
        else if(expectAndConsume("<="))
            node = new_binary(ND_LE, node, add());
        else if(expectAndConsume(">"))
            node = new_binary(ND_LT, add(), node);
        else if(expectAndConsume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}

//equality   = relational ("==" relational | "!=" relational)*
Node* equality(){

    Node* node = relational();

    for(;;){

        if(expectAndConsume("=="))
            node = new_binary(ND_EQ, node, relational());
        else if(expectAndConsume("!="))
            node = new_binary(ND_NE, node, relational());
        else
            return node;
    }
}

// expr = equality
Node *expr(){
    
    return equality();
}