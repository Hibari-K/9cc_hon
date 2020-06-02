#include "9cc.h"


int labelseq = 1;
char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *funcname;

// stack node-var address
void gen_addr(Node *node){


    //if(node->kind == ND_VAR){
    switch(node->kind){

    case ND_VAR:
        //int offset = (node->name - 'a' + 1) * 8;
        //fprintf(stderr, "gen_addr: %c\n", node->name);
        //fprintf(stderr, "gen_addr: %d\n", node->lvar->offset);

        printf("    lea rax, [rbp-%d]\n", node->var->offset);
        puts("    push rax");

        return;


    case ND_DEREF:

        //fprintf(stderr, "gen_addr *****\n");

        codegen(node->lhs);
        return;

    }

    //fprintf(stderr, "gen_addr : %s\n", node->var->name);

    error("not an local value");
}

// load address stored at stack-top
void load(){
    
    puts("    pop rax");
    puts("    mov rax, [rax]");
    puts("    push rax");
}

// data (stack-top), store addr (2nd-top)
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
            printf("    jmp .L.return.%s\n", funcname);
            return;
        case ND_ADDR: //&
            gen_addr(node->lhs);
            return;
        case ND_DEREF: //*
            codegen(node->lhs);
            load();
            return;
        case ND_IF: {
            /*
                *** cond code ***
                pop rax <- store a result from cond code
                cmp rax, 0
                je  .LelseXXX
                *** then code ***
                jmp .LendXXX
                .LelseXXX
                *** else code ***
                .LendXXX
             */

            int seq = labelseq++;

            //fprintf(stderr, "******** generating if statement *******");
            
            // if ... else
            if(node->els){
                
                codegen(node->cond);
                puts("    pop rax");
                puts("    cmp rax, 0");
                printf("    je .L.else.%d\n", seq);

                codegen(node->then);
                printf("    jmp .L.end.%d\n", seq);
                
                printf(".L.else.%d:\n", seq);
                codegen(node->els);

                printf(".L.end.%d:\n", seq);
            }
            // only if
            else{
                
                codegen(node->cond);
                puts("    pop rax");
                puts("    cmp rax, 0");
                printf("    je .L.end.%d\n", seq);

                codegen(node->then);
                printf(".L.end.%d:\n", seq);
            }

            return;
        }

        case ND_WHILE: {

            int seq = labelseq++;

            printf(".L.begin.%d:\n", seq);
            
            codegen(node->cond);
            puts("    pop rax");
            puts("    cmp rax, 0");
            printf("    je .L.end.%d\n", seq);

            codegen(node->then);
            printf("    jmp .L.begin.%d\n", seq);

            printf(".L.end.%d:\n", seq);

            return;
        }

        case ND_FOR: {

            int seq = labelseq++;

            // for( *** ;;)
            if(node->init){
                codegen(node->init);
            }

            printf(".L.begin.%d:\n", seq);
            if(node->cond){
                codegen(node->cond);
                puts("    pop rax");
                puts("    cmp rax, 0");
                printf("    je .L.end.%d\n", seq);
            }

            codegen(node->then);

            if(node->inc){
                codegen(node->inc);
            }

            printf("    jmp .L.begin.%d\n", seq);
            printf(".L.end.%d:\n", seq);

            return;
        }
        
        case ND_BLOCK:
            for(Node *n = node->body; n; n = n->next){
                codegen(n);
            }
            return;
        
        case ND_FUNCALL:{
            int nargs = 0;
            for(Node *arg = node->args; arg; arg = arg->next){
                codegen(arg);
                nargs++;
            }

            for(int i = nargs - 1; i >= 0; i--){
                printf("    pop %s\n", argreg[i]);
            }

            // we need to align RSP to a 16 byte boundary before calling a function
            // because it is an ABI requirement.
            // RAX is set to 0 for variadic function
            int seq = labelseq++;
            puts("    mov rax, rsp");
            puts("    and rax, 15");
            printf("    jnz .L.call.%d\n", seq); // if 8byte boundary
            puts("    mov rax, 0"); // the number of floating point arguments
            printf("    call %s\n", node->funcname);
            printf("    jmp .L.end.%d\n", seq);

            printf(".L.call.%d:\n", seq); // if 8byte boundary
            puts("    sub rsp, 8");
            puts("    mov rax, 0");
            printf("    call %s\n", node->funcname);
            puts("    add rsp, 8"); // to make consistent to sub
            
            printf(".L.end.%d:\n", seq);
            puts("    push rax");
            return;

        }


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


void codegenFirst(Function *prog){
    
    puts(".intel_syntax noprefix");

    for(Function *fn = prog; fn; fn = fn->next){

        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);

        funcname = fn->name;

        // prologue
        puts("    push rbp");
        puts("    mov rbp, rsp");
        printf("    sub rsp, %d\n", fn->stack_size);

        // Push arguments to the stack
        int i = 0;
        for(VarList *vl = fn->params; vl; vl = vl->next){
            Var *var = vl->var;
            printf("    mov [rbp-%d], %s\n", var->offset, argreg[i++]);
        }


        // code
        for(Node *node = fn->node; node; node = node->next){
            codegen(node);
        }

        // Epilogue
        printf(".L.return.%s:\n", funcname);
        puts("    mov rsp, rbp");
        puts("    pop rbp");
        puts("    ret");

    }
}