#include "9cc.h"



int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "invalid number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    //debug_token();

    //Node *node = program();
    Function *prog = program();

    for(Function *fn = prog; fn; fn = fn->next){

        int offset = 0;
        
        // setup local vars
        for(VarList *vl = fn->locals; vl; vl = vl->next){
            offset += 8;
            vl->var->offset = offset;
        }

        fn->stack_size = offset;
    }
    
    codegenFirst(prog);

    return 0;
}