#include<stdio.h>
#include<stdlib.h>

int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "invalid number of arguments\n");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n" \
           ".global main\n" \
           "main:\n" \
           "    mov rax, %ld\n" \
           , strtol(p, &p, 10));
    
    while(*p){
        if(*p == '+'){
            p++; //eat '+'
            printf("    add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if(*p == '-'){
            p++; // eat '-'
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "unexpected character: '%c'\n", *p);
        return 1;

    }

    printf("    ret\n");

    return 0;
}