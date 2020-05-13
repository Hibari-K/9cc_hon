#include "9cc.h"



int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "invalid number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    //debug_token();

    Node *node = program();

    codegenFirst(node);

    return 0;
}