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

void printInterpTree(Value *answer) {
    if (answer->type == INT_TYPE) {
            printf("%d ", answer->i);
        } else if (answer->type == DOUBLE_TYPE) {
            printf("%f ", answer->d);
        } else if (answer->type == STR_TYPE) {
            printf("%s ", answer->s);
        } else if (answer->type == CONS_TYPE) {
            printf("(");
            while (answer->type != NULL_TYPE) {
                printInterpTree(car(answer));
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
        printInterpTree(answer);
        printf("\n");
        tree = cdr(tree);
    }
}

Value *lookUpSymbol(Value *tree, Frame *frames) {
    Frame *currentFrame = frames;
    Value *temp;
    while (currentFrame->bindings->type != NULL_TYPE) {
        if (!strcmp(car(car(currentFrame->bindings))->s, tree->s)) {
            temp = cdr(car(currentFrame->bindings));
            return temp;
        }
        currentFrame->bindings = cdr(currentFrame->bindings);
    }
    if (currentFrame->parent != NULL) {
        Value *next = lookUpSymbol(tree, currentFrame->parent);
        return next;
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

Value *evalDefine(Value *args, Frame *frames) {
    Value *var = car(args);
    Value *expr = eval(car(cdr(args)), frames);
    Value *binding = talloc(sizeof(Value));
    binding->type = CONS_TYPE;
    binding->c.car = var;
    binding->c.cdr = expr;
    frames->bindings = cons(binding, frames->bindings);
    Value *result = talloc(sizeof(Value));
    result->type = VOID_TYPE;
    return result;
}

Value *evalLambda(Value *args, Frame *frames) {
    Value *params = car(args);
    Value *body = car(cdr(args));
    Value *result = talloc(sizeof(Value));
    result->type = CLOSURE_TYPE;
    result->cl.paramNames = params;
    result->cl.functionCode = body;
    result->cl.frame = frames;
    return result;
}

Value *apply(Value *function, Value *args) {
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = function->cl.frame;
    newFrame->bindings = makeNull();
    Value *formalParams = function->cl.paramNames;
    Value *actualParams = args;
    while (actualParams->type != NULL_TYPE) {
        if (formalParams->type == NULL_TYPE) {
            evaluationError();
        }
        Value *binding = talloc(sizeof(Value));
        binding->type = CONS_TYPE;
        binding->c.car = car(formalParams);
        binding->c.cdr = eval(car(actualParams), function->cl.frame);
        newFrame->bindings = cons(binding, newFrame->bindings);
        formalParams = cdr(formalParams);
        actualParams = cdr(actualParams);
    }
    Value *result = eval(function->cl.functionCode, newFrame);
    return result;
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
                result = evalQuote(args, frame);
                return result;
            } else if (!strcmp(first->s, "define")) {
                result = evalDefine(args, frame);
                return result;
            } else if (!strcmp(first->s, "lambda")) {
                result = evalLambda(args, frame);
                return result;
            }
            else {
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = makeNull();
                while (args->type != NULL_TYPE) {
                    Value *evaled = eval(car(args), frame);
                    evaledArgs = cons(evaled, evaledArgs);
                    args = cdr(args);
                }
                evaledArgs = reverse(evaledArgs);
                return apply(evaledOperator, evaledArgs);
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


