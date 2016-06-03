/*
Nikita Fomichev and Chris Shoemaker (eager_bird)
CS 251
Interpreter project
Functions for the interpreting of (relatively) simple Scheme programs
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

// Error function if evaluation goes wrong
void evaluationError() {
    printf("evaluation error\n");
    texit(1);
}

// Prints out the interpreted parse tree
void printInterpTree(Value *answer) {
    if (answer->type == INT_TYPE) {
            printf("%d ", answer->i);
        } else if (answer->type == DOUBLE_TYPE) {
            printf("%f ", answer->d);
        } else if (answer->type == STR_TYPE) {
            printf("\"");
            printf("%s ", answer->s);
            printf("\b\"");
        } else if (answer->type == CONS_TYPE) {
            printf("(");
            while (answer->type != NULL_TYPE) {
                printInterpTree(car(answer));
                answer = cdr(answer);   
            }
            printf("\b) ");
        } else if (answer->type == BOOL_TYPE) {
            if (answer->i == 1) {
                printf("#t ");
            } else {
                printf("#f ");
            }
        } else if (answer->type == SYMBOL_TYPE) {
            printf("%s ", answer->s);
        }
}

// Binds primitive functions to the global (top) frame
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    Value *funcValue = talloc(sizeof(Value));
    funcValue->type = PRIMITIVE_TYPE;
    funcValue->pf = function;
    Value *primValue = talloc(sizeof(Value));
    primValue->type = SYMBOL_TYPE;
    primValue->s = name;
    Value *binding = talloc(sizeof(Value));
    binding->type = CONS_TYPE;
    binding->c.car = primValue;
    binding->c.cdr = funcValue;
    frame->bindings = cons(binding, frame->bindings);
}

// Primitive + returns sum of args
Value *primitiveAdd(Value *args){
    Value *result = talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    // If no arguments given, return 0
    if (args->type == NULL_TYPE) {
        result->d = 0.0;
        return result;
    }
    double sum = 0.0;
    while (args->type != NULL_TYPE) {
        if (car(args)->type == INT_TYPE) {
            sum += car(args)->i;
        } else if (car(args)->type == DOUBLE_TYPE) {
            sum += car(args)->d;
        }
        args = cdr(args);
    }
    result->d = sum;
    return result;
}

// Primitive null? checks if args is null
Value *primitiveNull(Value *args) {
    // Error checking: if more than one argument, throw error
    if (cdr(args)->type != NULL_TYPE) {
        evaluationError();
    }
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    int val = 0;
    if (car(args)->type == NULL_TYPE) {
        val = 1;
    }
    result->i = val;
    return result;
}

/*
Other primitive functions go here
*/
Value *primitiveCar(Value *args) {
    // If zero arguments are given, throw error
    if (args->type == NULL_TYPE) {
        evaluationError();
    }
    // If more than one argument is given, throw error
    if (cdr(args)->type != NULL_TYPE){
        evaluationError();
    }
    if (car(args)->type != CONS_TYPE){
        evaluationError();
    }
    return car(car(args));
}

Value *primitiveCdr(Value *args) {
    // If zero arguments are given, throw error
    if (args->type == NULL_TYPE) {
        evaluationError();
    }
    // If more than one argument is given, throw error
    if (cdr(args)->type != NULL_TYPE){
        evaluationError();
    }
    if (car(args)->type != CONS_TYPE){
        evaluationError();
    }
    return cdr(car(args));
}

Value *primitiveCons(Value *args) {
    // If zero arguments are given, throw error
    if (args->type == NULL_TYPE) {
        evaluationError();
    }
    // If only one argument is given, throw error
    if (cdr(args)->type == NULL_TYPE){
        evaluationError();
    }
    // If more than two arguments given, throw error
    if (cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError();
    }
    
    return cons(car(args), car(cdr(args)));
}


// Main interpretation function for parsed Scheme code
void interpret(Value *tree){
    Value *answer;
    Frame *globalFrame = talloc(sizeof(Frame));
    globalFrame->bindings = makeNull();
    globalFrame->parent = NULL;
    bind("+", primitiveAdd, globalFrame);
    bind("null?", primitiveNull, globalFrame);
    bind("car", primitiveCar, globalFrame);
    bind("cdr", primitiveCdr, globalFrame);
    bind("cons", primitiveCons, globalFrame);
    while (tree->type != NULL_TYPE) {
        // Evaluate every element in the parse tree
        answer = eval(car(tree), globalFrame);
        printInterpTree(answer);
        if (answer->type != VOID_TYPE) {
            printf("\n");
        }
        tree = cdr(tree);
    }

}

Value *lookUpSymbol(Value *tree, Frame *frames) {
    Frame *currentFrame = frames;
    Value *currentBindings;
    Value *exprValue = tree;
    while (currentFrame != NULL) {
        currentBindings = currentFrame->bindings;
        while(currentBindings->type != NULL_TYPE) {
            if (!strcmp(exprValue->s, car(car(currentBindings))->s)) {
                exprValue = cdr(car(currentBindings));
                return exprValue;
            }
            currentBindings = cdr(currentBindings);
        }
        currentFrame = currentFrame->parent;
    }
    evaluationError();
}

// Function for evaluating 'if' procedures in Scheme
Value *evalIf(Value *args, Frame *frames) {
    if (car(args)->type != BOOL_TYPE) {
        if (car(args)->type == SYMBOL_TYPE || car(args)->type == CONS_TYPE) {
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

// Function for evaluating 'let' procedures in Scheme
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

// Function for evaluating 'quote' procedures in Scheme
Value *evalQuote(Value *args, Frame *frames)  {
    return car(args);
}

// Function for evaluating 'define' procedures in Scheme
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

// Function for evaluating Scheme lambda functions
Value *evalLambda(Value *args, Frame *frames) {
    Value *params = car(args);
    Value *body = car(cdr(args));
    Value *result = talloc(sizeof(Value));
    result->type = CLOSURE_TYPE;
    result->cl.paramNames = makeNull();
    while (params->type == CONS_TYPE) {
        result->cl.paramNames = cons(car(params), result->cl.paramNames);
        params = cdr(params);
    }
    result->cl.functionCode = body;
    result->cl.frame = frames;
    return result;
}

Value *apply(Value *function, Value *args) {
    if (function->type == CLOSURE_TYPE){
        if (length(args) != length(function->cl.paramNames)) {
            evaluationError();
        }
        Frame *newFrame = talloc(sizeof(Frame));
        newFrame->parent = function->cl.frame;
        newFrame->bindings = makeNull();
        Value *code = function->cl.functionCode;
        Value *formalParams = function->cl.paramNames;
        while (formalParams->type == CONS_TYPE) {
            Value *binding = talloc(sizeof(Value));
            binding->type = CONS_TYPE;
            binding->c.car = car(formalParams);
            binding->c.cdr = car(args);
            newFrame->bindings = cons(binding, newFrame->bindings);
            formalParams = cdr(formalParams);
            args = cdr(args);
        }
        Value *result = eval(code, newFrame);
        return result;
    } else if (function->type == PRIMITIVE_TYPE) {
        Value *result = function->pf(args);
        return result;
    }
    evaluationError();
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
                printInterpTree(evaledArgs);
                printf("\n");
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
    evaluationError();
}


