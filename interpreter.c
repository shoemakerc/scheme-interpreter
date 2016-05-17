#include "value.h"
#include "talloc.h"
#include "tokenizer.h"
#include "parser.h"
#include "linkedlist.h"
#include "interpreter.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


// A frame is a linked list of bindings, and a pointer to another frame.  A
// binding is a variable name (represented as a string), and a pointer to the
// Value it is bound to. Specifically how you implement the list of bindings is
// up to you.

void evaluationError() {
    printf("evaluation error\n");
    //texit(1);
}

void interpret(Value *tree){
    //printf("test3");
    Value *answer;
    Frame *globalFrame = talloc(sizeof(Frame));
    globalFrame->bindings = makeNull();
    while (tree->type != NULL_TYPE) {
        answer = eval(car(tree), globalFrame);
        if (answer->type == INT_TYPE) {
            printf("%d\n", answer->i);
        } else if (answer->type == DOUBLE_TYPE) {
            printf("%f\n", answer->d);
        } else if (answer->type == STR_TYPE) {
            printf("%s\n", answer->s);
        } else if (answer->type == SYMBOL_TYPE) {
            printf("%s\n", answer->s);
        }
        tree = cdr(tree);
    }
}

//Value *lookUpSymbol(Value *tree, Frame *frames) {
//    Value *temp;
//    while (frames)
//}

                   
Value *evalIf(Value *args, Frame *frames) {
    //printf("test2");
    if (car(args)->type != BOOL_TYPE) {
        evaluationError();
    }
    if (args->c.car->i == 1){
        if (cdr(args)->type == NULL_TYPE) {
            evaluationError();
        }
        if (cdr(cdr(args))->type == NULL_TYPE) {
            evaluationError();
        }
        return eval(car(cdr(args)), frames);
    }
    else {
        if (cdr(args)->type == NULL_TYPE) {
            evaluationError();
        }
        if (cdr(cdr(args))->type == NULL_TYPE) {
            evaluationError();
        }
        return eval(car(cdr(cdr(args))), frames);
    }
}

Value *evalLet(Value *args, Frame *frames) {
    //printf("test4\n");
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frames;
    newFrame->bindings = makeNull();
    Value *vars = car(args);
    while (vars->type != NULL_TYPE){
        Value *vali = eval(car(cdr(car(vars))), frames);
        newFrame->bindings = cons(vali, newFrame->bindings);
        vars = cdr(vars);
    }
    Value *result = eval(car(cdr(args)), newFrame);
    return result;
}


Value *eval(Value *tree, Frame *frame) {
    //printf("test1");
    //Value *frames;
    Value *result;
    Value *first;
    Value *args;
    switch (tree->type) {
        case INT_TYPE:
            return tree;
            break;
        case DOUBLE_TYPE:
            return tree;
            break;
        case STR_TYPE:
            return tree;
            break;
        case CONS_TYPE:
            first = car(tree);
            args = cdr(tree);
            // Sanity and error checking on first...
            if (!strcmp(first->s,"if")) 
            {
                result = evalIf(args,frame);
                return result;
            } 
            else if (!strcmp(first->s,"let")) {
                //frame = evalLet(args,frame);
                result = evalLet(args, frame);
                //result = eval(car(cdr(args)), frame);
                return result;
              }
            else {
                evaluationError();
            }
            break;
        case BOOL_TYPE:
            return tree;
            break;
        default:
            return tree;
            break;
    }
    return 0;
}


