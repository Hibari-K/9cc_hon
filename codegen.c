#include "9cc.h"




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
