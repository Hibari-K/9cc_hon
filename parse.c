#include "9cc.h"

// Current token
Token *token;

// input
char *user_input;





/** Node **/

Node *new_node(NodeKind kind){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

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

Node *new_node_var(char name){

    Node *node = new_node(ND_VAR);
    node->name = name;

    return node;
}

Node *new_unary(NodeKind kind, Node *expr){
    
    Node *node = new_node(kind);
    node->lhs = expr;

    return node;
}

/*
program    = stmt*
stmt       = expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
*/


Node *program(){
    
    Node head = {};
    Node *cur = &head;

    while(!at_eof()){
        cur->next = stmt();
        cur = cur->next;
    }
    
    return head.next;
}

// stmt = "return" expr ";" | expr ";"
Node *stmt(){
    
    // return
    if(expectAndConsume("return")){
        
        Node *node = new_unary(ND_RETURN, expr());
        expect(";"); // eat ";"

        return node;
    }

    // other
    Node *node = expr();
    expectAndConsume(";");

    return node;
}

// expr = equality
Node *expr(){
    
    return assign();
}

// assign
Node *assign(){

    Node *node = equality();
    
    if(expectAndConsume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    
    return node;
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

// unary = ("+" | "-")? primary
Node* unary(){
    if(expectAndConsume("+"))
        return unary(); // +1 = 1
    if(expectAndConsume("-"))
        return new_binary(ND_SUB, new_node_num(0), unary());
    return primary();
}



// primary = "(" expr ")" | num | ident
Node *primary(){

    // if next token is "(", the expected node is '"(" expr ")"'
    if(expectAndConsume("(")){
        Node *node = expr();
        expect(")");
        return node;
    }


    Token *tok = consumeIdent();
    if(tok)
        return new_node_var(*tok->str);

    // otherwise, it should be num
    return new_node_num(expect_number());
}








