#include "9cc.h"


void gen_addr(Node *node){

    if(node->kind == ND_VAR){

        int offset = (node->name - 'a' + 1) * 8;
    
        //fprintf(stderr, "gen_addr: %c\n", node->name);
        //fprintf(stderr, "gen_addr: %d\n", offset);

        printf("    lea rax, [rbp-%d]\n", offset);
        puts("    push rax");

        return;
    }

    error("not an local value");
}

void load(){
    
    puts("    pop rax");
    puts("    mov rax, [rax]");
    puts("    push rax");
}

void store(){
    
    puts("    pop rdi");
    puts("    pop rax");
    puts("    mov [rax], rdi");
    puts("    push rdi");
}


void codegen(Node *node){

    switch(node->kind){
        
        case ND_NUM:
            printf("    push %ld\n", node->val);
            return;
        case ND_EXPR_STMT:
            codegen(node->lhs);
            puts("    add rsp, 8");
            return;
        case ND_VAR:
            gen_addr(node);
            load();
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            codegen(node->rhs);
            store();
            return;
        case ND_RETURN:
            codegen(node->lhs);
            puts("    pop rax");
            //puts("    ret");
            puts("    jmp .L.return");
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
        case ND_EQ:
            puts("    cmp rax, rdi");
            puts("    sete al");
            puts("    movzb rax, al");
            break;
        case ND_NE:
            puts("    cmp rax, rdi");
            puts("    setne al");
            puts("    movzb rax, al");
            break;
        case ND_LT:
            puts("    cmp rax, rdi");
            puts("    setl al");
            puts("    movzb rax, al");
            break;
        case ND_LE:
            puts("    cmp rax, rdi");
            puts("    setle al");
            puts("    movzb rax, al");
            break;
    }

    puts("    push rax");
}


void codegenFirst(Node *node){
    
    printf(".intel_syntax noprefix\n" \
           ".global main\n" \
           "main:\n");

    
    // Prologue
    
    printf("    push rbp\n" \
           "    mov rbp, rsp\n" \
           "    sub rsp, 208\n");
    

    for(Node *n = node; n; n = n->next){
        codegen(n);
        puts("    pop rax");
    }

    puts(".L.return:");
    puts("    mov rsp, rbp");
    puts("    pop rbp");
    puts("    ret");
}