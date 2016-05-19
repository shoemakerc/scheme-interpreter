/*
Nikita Fomichev and Chris Shoemaker (eager_bird)
CS 251
Interpreter, if/let
Starting interpreter functions for the evaluation of let and if statements.
*/

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
    texit(1);
}

void printEval(Value *answer) {
    if (answer->type == INT_TYPE) {
            printf("%d ", answer->i);
        } else if (answer->type == DOUBLE_TYPE) {
            printf("%f ", answer->d);
        } else if (answer->type == STR_TYPE) {
            printf("%s ", answer->s);
        } else if (answer->type == CONS_TYPE) {
            printf("(");
            while (answer->type != NULL_TYPE) {
                printEval(car(answer));
                answer = cdr(answer);   
            }
            printf("\b) ");
        } else if (answer->type == BOOL_TYPE) {
            printf("%d ", answer->i);
        } else if (answer->type == SYMBOL_TYPE) {
            printf("%s ", answer->s);
        }
}
void interpret(Value *tree){
    Value *answer;
    Frame *globalFrame = talloc(sizeof(Frame));
    globalFrame->bindings = makeNull();
    while (tree->type != NULL_TYPE) {
        answer = eval(car(tree), globalFrame);
        printEval(answer);
        printf("\n");
        tree = cdr(tree);
    }
}

Value *lookUpSymbol(Value *tree, Frame *frames) {
    Value *temp;
    while (frames->bindings->type != NULL_TYPE) {
        if (!strcmp(car(car(frames->bindings))->s, tree->s)) {
            temp = cdr(car(frames->bindings));
            return temp;
        }
        frames->bindings = cdr(frames->bindings);
    }
    if (frames->parent != NULL) {
        return lookUpSymbol(tree, frames->parent);
    } else {
        evaluationError();
    }
}

                   
Value *evalIf(Value *args, Frame *frames) {
    if (car(args)->type != BOOL_TYPE) {
        if (car(args)->type == SYMBOL_TYPE) {
            Value *temp = eval(car(args), frames);
            if (temp->type == BOOL_TYPE) {
                if (temp->i == 1){
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
                
            } else {
                evaluationError();
            }
        }
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
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frames;
    newFrame->bindings = makeNull();
    Value *bindingsList = car(args);
    if (bindingsList->type == NULL_TYPE) {
        evaluationError();
    }
    while (bindingsList->type != NULL_TYPE) {
        Value *var = car(car(bindingsList));
        Value *val = eval(car(cdr(car(bindingsList))), frames);
        Value *binding = talloc(sizeof(Value));
        binding->type = CONS_TYPE;
        binding->c.car = var;
        binding->c.cdr = val;
        newFrame->bindings = cons(binding, newFrame->bindings);
        bindingsList = cdr(bindingsList);
    }
    Value *result = eval(car(cdr(args)), newFrame);
    return result;
}

Value *evalQuote(Value *args, Frame *frames)  {
    return car(args);
}

Value *eval(Value *tree, Frame *frame) {
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
            if (!strcmp(first->s,"if")) 
            {
                result = evalIf(args, frame);
                return result;
            } 
            else if (!strcmp(first->s,"let")) {
                result = evalLet(args, frame);
                return result;
              }
            else if (!strcmp(first->s, "quote")) {
                return car(args);
            }
            else {
                evaluationError();
            }
            break;
        case BOOL_TYPE:
            return tree;
            break;
        case SYMBOL_TYPE:
            result = lookUpSymbol(tree, frame);
            return result;
            break;
        default:
            return tree;
            break;
    }
    return 0;
}


