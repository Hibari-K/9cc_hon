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

    //fprintf(stderr, "it will be number: %d\n", val);
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;

    return node;
}

Node *new_node_var(Var *var){

    Node *node = new_node(ND_VAR);
    node->var = var;

    return node;
}

Node *new_unary(NodeKind kind, Node *expr){
    
    Node *node = new_node(kind);
    node->lhs = expr;

    return node;
}

Var *new_lvar(char *name, int len){

    Var *var = calloc(1, sizeof(Var));
    //var->next = locals;
    //var->name = name;
    //strncpy(var->name, name, len);
    var->name = strndup(name, len);

    if(!locals)
        var->offset = 0;
    else
        var->offset = locals->var->offset + 8;

    //locals = var;
    VarList *vl = calloc(1, sizeof(VarList));
    vl->var = var;
    vl->next = locals;
    locals = vl;
    
    return var;
}

/*
program    = function*
function   = ident "(" ")" "{" stmt* "}"
stmt       = expr ";" 
             | "if" "(" expr ")" stmt ("else" stmt)?
             | "while" "(" expr ")" stmt
             | "for" "(" expr? ";" expr? ";" expr? ")" stmt
             | "{" stmt* "}"
             | "return" expr? ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident args? | "(" expr ")"
args       = "(" (assign ("," assign)* )? ")"
*/


Function *program(){
    
    Function head = {};
    Function *cur = &head;
    locals = NULL;

    while(!at_eof()){
        cur->next = function();
        cur = cur->next;
    }
    
    return head.next;
}

// params = ident ( "," ident)*
VarList *read_func_params(){

    if(expectAndConsume(")")){
        return NULL;
    }

    VarList *head = calloc(1, sizeof(VarList));
    
    char *s = expect_ident();
    head->var = new_lvar(s, strlen(s));
    VarList *cur = head;

    while(!expectAndConsume(")")){
        expect(",");
        
        cur->next = calloc(1, sizeof(VarList));
        s = expect_ident();
        cur->next->var = new_lvar(s, strlen(s));
        
        cur = cur->next;
    }

    return head;
}

// function = ident "(" params? ")" "{" stmt* "}"
Function *function(){

    locals = NULL;
    Function *fn = calloc(1, sizeof(Function));
    fn->name = expect_ident();

    //char *name = expect_ident();
    expect("(");
    fn->params = read_func_params();
    //expect(")");
    expect("{");

    Node head = {};
    Node *cur = &head;

    while(!expectAndConsume("}")){
        cur->next = stmt();
        cur = cur->next;
    }

    //Function *fn = calloc(1, sizeof(Function));
    //fn->name = name;
    fn->node = head.next;
    fn->locals = locals;

    return fn;
}

/*
stmt       =   expr ";" 
             | "if" "(" expr ")" stmt ("else" stmt)?
             | "while" "(" expr ")" stmt 
             | "for" "(" expr? ";" expr? ";" expr? ")" stmt
             | "{" stmt* "}"
             | "return" expr? ";"
*/
Node *stmt(){
    
    // return
    if(expectAndConsume("return")){
        
        Node *node = new_unary(ND_RETURN, expr());
        expect(";"); // eat ";"

        return node;
    }

    // "if" "(" expr ")" stmt ("else" stmt)?
    if(expectAndConsume("if")){
        
        //fprintf(stderr, "*******  stmt if  *******");

        Node *node = new_node(ND_IF);
        expect("("); //simply eat
        
        node->cond = expr();
        expect(")"); //simply eat

        node->then = stmt();
        
        if(expectAndConsume("else")){
            node->els = stmt();
        }

        return node;
    }

    // "while"
    if(expectAndConsume("while")){

        //debug
        //fprintf(stderr, "***** parse while stmt *****\n");

        Node *node = new_node(ND_WHILE);
        expect("("); // simpl eat
        node->cond = expr();
        expect(")");

        node->then = stmt();
        
        return node;
    }

    // for
    if(expectAndConsume("for")){

        Node *node = new_node(ND_FOR);

        expect("(");
        // there is the possibility of "for(;;)"
        if(!expectAndConsume(";")){
            node->init = expr();
            expect(";");
        }
        
        if(!expectAndConsume(";")){
            node->cond = expr();
            expect(";");
        }

        if(!expectAndConsume(")")){
            node->inc = expr();
            expect(")");
        }

        node->then = stmt();
        return node;
    }

    // block { ... }
    if(expectAndConsume("{")){
        Node head = {}; // must be var not pointer
        Node *cur = &head;

        while(!expectAndConsume("}")){
            cur->next = stmt();
            cur = cur->next;
        }

        Node *node = new_node(ND_BLOCK);
        node->body = head.next;

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



// primary = "(" expr ")" | num | ident args?
Node *primary(){

    // if next token is "(", the expected node is '"(" expr ")"'
    if(expectAndConsume("(")){
        Node *node = expr();
        expect(")");
        return node;
    }


    Token *tok = consumeIdent();
    if(tok){

        // if function call (Ident "(" ")")
        if(expectAndConsume("(")){

            // expect(")"); // this line targets only for func without args
            
            Node *node = new_node(ND_FUNCALL);
            node->funcname = strndup(tok->str, tok->len);
            node->args = args();

            return node;
        }


        Var *var = find_lvar(tok);
        if(!var){
            
            //lvar = calloc(1, sizeof(LVar));
            //lvar->next = locals;
            //lvar->name = tok->str;
            //lvar->len = tok->len;
            //lvar->offset = locals->offset + 8;
            //locals = lvar;
            var = new_lvar(tok->str, tok->len);
            
            // debug
            //fprintf(stderr, "find_lvar: lvar == %s\n", lvar->name);

        }

        return new_node_var(var);
    }
        

    // otherwise, it should be num
    return new_node_num(expect_number());
}

Node *args(){

    // it assumes here is in parentheses of func. (i.e., foo( ... HERE ... ) )
    if(expectAndConsume(")")){
        return NULL;
    }

    Node *head = assign(); // first arg
    Node *cur = head;

    while(expectAndConsume(",")){
        cur->next = assign();
        cur = cur->next;
    }

    expect(")"); // just eat

    return head;

}






