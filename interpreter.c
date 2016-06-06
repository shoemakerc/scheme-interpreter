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

// Returns value containing data bound to a given symbol (if such data
// can be found in the bindings)
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
    printf("Error2\n");
    evaluationError();
    return 0;
}

// Binds given data to a given variable then stores it in a list of 
// bindings for a given frame
void addBinding(Value *var, Value *val, Frame *frame) {
    Value *binding = talloc(sizeof(Value));
    binding->type = CONS_TYPE;
    binding->c.car = var;
    binding->c.cdr = val;
    frame->bindings = cons(binding, frame->bindings);
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

/*----------------------------------
----------BEGIN PRIMITIVES----------
----------------------------------*/

// Primitive + returns sum of args
Value *primitiveAdd(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
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

// Primitive - returns difference of args
Value *primitiveSubtract(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
    }
    double diff = car(args)->i;
    args = cdr(args);
    while (args->type != NULL_TYPE) {
        if (car(args)->type == INT_TYPE) {
            diff -= car(args)->i;
        } else if (car(args)->type == DOUBLE_TYPE) {
            diff -= car(args)->d;
        }
        args = cdr(args);
    }
    result->d = diff;
    return result;
}

// Primitive * returns product of args
Value *primitiveMultiply(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) < 2) {
        evaluationError();
    }
    double product = car(args)->i;
    args = cdr(args);
    while (args->type != NULL_TYPE){
        if (car(args)->type == INT_TYPE) {
            product *= car(args)->i;
        }
        else if (car(args)->type == DOUBLE_TYPE){
            product *= car(args)->d;
        }
        args = cdr(args);
    }
    result->d = product;
    return result;
}

// Primitive / returns quotient of args
Value *primitiveDivide(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
    }
    double quotient = car(args)->i;
    args = cdr(args);
    while (args->type != NULL_TYPE) {
        if (car(args)->i == 0 || car(args)->d == 0.0) {
            evaluationError();
        }
        if (car(args)->type == INT_TYPE) {
            quotient /= car(args)->i;
        }
        else if (car(args)->type == DOUBLE_TYPE) {
            quotient /= car(args)->d;
        }
        args = cdr(args);
    }
    result->d = quotient;
    return result;
}

// Primitive modulo returns remainder of quotient of two args
Value *primitiveModulo(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = INT_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
    }
    int dividend = car(args)->i;
    int divisor = car(cdr(args))->i;
    int ans = dividend % divisor;
    result->i = ans;
    return result;
}

// Primitive < evaluates if first arg is less than second arg
Value *primitiveLess(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
    }
    int first = car(args)->i;
    int second = car(cdr(args))->i;
    int ans = 0;
    if (first < second){
        ans = 1;
    }
    result->i = ans;
    return result;
}

// Primitive > evaluates if first arg is greater than second arg
Value *primitiveMore(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
    }
    int first = car(args)->i;
    int second = car(cdr(args))->i;
    int ans = 0;
    if (first > second){
        ans = 1;
    }
    result->i = ans;
    return result;
}

// Primitive = evaluates if first arg is equal to the second arg
Value *primitiveEqual(Value *args) {
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
    }
    int first = car(args)->i;
    int second = car(cdr(args))->i;
    int ans = 0;
    if (first == second){
        ans = 1;
    }
    result->i = ans;
    return result;
}

// Primitive null? checks if args is null
Value *primitiveNull(Value *args) {
    // If one argument is not given, throw an eval error
    if (length(args) != 1) {
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

// Primitive car returns car of args
Value *primitiveCar(Value *args) {
    // If one argument is not given, throw an eval error
    if (length(args) != 1) {
        evaluationError();
    }
    if (car(args)->type != CONS_TYPE) {
        evaluationError();
    }
    return car(car(args));
}

// Primitive cdr returns car of args
Value *primitiveCdr(Value *args) {
    // If one argument is not given, throw an eval error
    if (length(args) != 1) {
        evaluationError();
    }
    if (car(args)->type != CONS_TYPE) {
        evaluationError();
    }
    return cdr(car(args));
}

// Primitive cons returns a pair such that car = first argument
// and cdr = second argument
Value *primitiveCons(Value *args) {
    // If two arguments are not given, throw an eval error
    if (length(args) != 2) {
        evaluationError();
    }
    return cons(car(args), car(cdr(args)));
}

/*----------------------------------
-----------END PRIMITIVES-----------
----------------------------------*/

// Main interpretation function for parsed Scheme code
void interpret(Value *tree){
    Value *answer;
    Frame *globalFrame = talloc(sizeof(Frame));
    globalFrame->bindings = makeNull();
    globalFrame->parent = NULL;
    bind("+", primitiveAdd, globalFrame);
    bind("-", primitiveSubtract, globalFrame);
    bind("*", primitiveMultiply, globalFrame);
    bind("/", primitiveDivide, globalFrame);
    bind("modulo", primitiveModulo, globalFrame);
    bind("<", primitiveLess, globalFrame);
    bind(">", primitiveMore, globalFrame);
    bind("=", primitiveEqual, globalFrame);
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

/*----------------------------------
--------BEGIN SPECIAL FORMS---------
----------------------------------*/

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
        addBinding(var, val, newFrame);
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

// Function for evaluating 'let*' procedures in Scheme
// Does not work properly at the moment
Value *evalLetStar(Value *args, Frame *frames) {
    Frame *parFrame = frames;
    Frame *curFrame = talloc(sizeof(Frame));
    curFrame->parent = parFrame;
    curFrame->bindings = makeNull();
    Value *allFrames = makeNull();
    while (length(args) > 0) {
        Value *var = car(args);
        Value *expr = cdr(args);
        Value *curBinding;
        Value *sList = makeNull();
        while (var->type == CONS_TYPE) {
            curBinding = car(var);
            Frame *newFrame = talloc(sizeof(Frame));
            newFrame->parent = curFrame;
            newFrame->bindings = makeNull();
            addBinding(car(curBinding), car(cdr(curBinding)), newFrame);
            curFrame = newFrame;
            var = cdr(var);
        }
        Value *curList = expr;
        while (curList->type == CONS_TYPE) {
            sList = cons(eval(car(curList), curFrame), sList);
            curList = cdr(curList);
        }
        allFrames = cons(sList, allFrames);
        args = cdr(args);
    }
    return allFrames;
}

// Function for evaluating 'letrec' procedures in Scheme
// Does not work properly at the moment
Value *evalLetrec(Value *args, Frame *frames) {
    Frame *letFrame = talloc(sizeof(Frame));
    letFrame->parent = frames;
    letFrame->bindings = makeNull();
    Value *result = makeNull();
    if (args->type != CONS_TYPE)
        evaluationError();
    if (cdr(args)->type == NULL_TYPE)
        evaluationError();
    if (car(args)->type != CONS_TYPE)
        evaluationError();
    if (car(car(args))->type != CONS_TYPE)
        evaluationError();      
    if (car(car(car(args)))->type != SYMBOL_TYPE)
        evaluationError();
    Value *var = car(args);  
    Value *val = cdr(args);
    Value *curBinding;
    while (var->type == CONS_TYPE) {
        curBinding = car(var);
        addBinding(car(curBinding), car(cdr(curBinding)),  letFrame);
        var = cdr(var);      
    }
    Value *curList = val;
    while (curList->type == CONS_TYPE) {
        result = cons(eval(car(curList), letFrame), result);
        curList = cdr(curList);
    }
    return result;
}

// Function for evaluating 'set!' procedures in Scheme
Value *evalSet(Value *args, Frame *frames) {
    Frame *curFrame;
    Value *curBindings;
    Value *exprValue;
    curFrame = frames;
    while (curFrame != NULL) {    
        curBindings = curFrame->bindings;
        while (curBindings->type != NULL_TYPE) {      
            Value *var = car(args);
            Value *val = cdr(args);
            if (!strcmp(var->s, car(car(curBindings))->s)) {
                addBinding(car(args), car(cdr(args)), frames);
            }
            curBindings = cdr(curBindings);
        }
        curFrame = curFrame->parent;
    }
    Value *result = talloc(sizeof(Value));
    result->type = VOID_TYPE;
    return result;
}

// Function for evaluating 'begin' procedures in Scheme
Value *evalBegin(Value *args, Frame *frame) {
    if (length(args) == 0) {
        evaluationError();
    }
    Value *expr = args;
    Value *exprValue;
    while (length(expr) > 1 && expr->type != NULL_TYPE) {
        exprValue = eval(car(expr), frame);
        expr = cdr(expr);
        if (length(args) == 1) {
            return eval(car(expr), frame);
        }
    }
    return eval(car(expr), frame);
}


// Function for evaluating 'cond' procedures in Scheme
Value *evalCond(Value *args, Frame *frame) {
    if (length(args) == 0) {
        return makeNull();
    }
    Value *cond = talloc(sizeof(Value));
    while (args->type != NULL_TYPE) {
        cond = eval(car(car(args)), frame);
        if (cond->i == 1) {
            return eval(car(cdr(car(args))), frame);
        }
        args = cdr(args);
    }
    return makeNull();
}

// Function for evaluating 'or' procedures in Scheme
Value *evalOr(Value *args, Frame *frame) {
    if (length(args) == 0) {
        Value *falseVal = talloc(sizeof(Value));
        falseVal->type = BOOL_TYPE;
        falseVal->i = 0;
        return falseVal;
    }
    Value *orCond = talloc(sizeof(Value));
    while (args->type != NULL_TYPE) {
        orCond = eval(car(args), frame);
        if (orCond->i == 1) {
            Value *trueVal = talloc(sizeof(Value));
            trueVal->type = BOOL_TYPE;
            trueVal->i = 1;
            return trueVal;
        }
        args = cdr(args);
    }
    Value *falseVal = talloc(sizeof(Value));
    falseVal->type = BOOL_TYPE;
    falseVal->i = 0;
    return falseVal;
}


// Function for evaluating 'and' procedures in Scheme      
Value *evalAnd(Value *args, Frame *frame)
{
    Value *trueVal = talloc(sizeof(Value));
    trueVal->type = BOOL_TYPE;
    trueVal->i = 1;
    if (length(args) == 0) {
        return trueVal;
    }
    Value *andCond = talloc(sizeof(Value));
    while (args->type != NULL_TYPE) {
        andCond = eval(car(args), frame);
        if (andCond->i == 0) {
            Value *falseVal = talloc(sizeof(Value));
            falseVal->type = BOOL_TYPE;
            falseVal->i = 0;
            return falseVal;
        }
        args = cdr(args);
    }
    return trueVal;
}

/*----------------------------------
---------END SPECIAL FORMS----------
----------------------------------*/

// Function for the application of Scheme procedures
Value *apply(Value *function, Value *args) {
    if (function->type == CLOSURE_TYPE) {
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
    else {
        printf("Error1\n");
        evaluationError();
        return makeNull();
    }
}

// Evaluates each element in the parse tree
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
            if (!strcmp(first->s,"if")) {
                result = evalIf(args, frame);
                return result;
            } else if (!strcmp(first->s,"let")) {
                result = evalLet(args, frame);
                return result;
            } else if (!strcmp(first->s, "quote")) {
                result = evalQuote(args, frame);
                return result;
            } else if (!strcmp(first->s, "define")) {
                result = evalDefine(args, frame);
                return result;
            } else if (!strcmp(first->s, "lambda")) {
                result = evalLambda(args, frame);
                return result;
            } else if (!strcmp(first->s, "let*")) {
                result = evalLetStar(args, frame);
                return result;
            } else if (!strcmp(first->s, "letrec")) {
                result = evalLetrec(args, frame);
                return result;
            } else if (!strcmp(first->s, "set!")) {
                result = evalSet(args, frame);
                return result;
            } else if (!strcmp(first->s, "begin")) {
                result = evalBegin(args, frame);
                return result;
            } else if (!strcmp(first->s, "cond")) {
                result = evalCond(args, frame);
                return result;
            } else if (!strcmp(first->s, "and")) {
                result = evalAnd(args, frame);
                return result;
            } else if (!strcmp(first->s, "or")) {
                result = evalOr(args, frame);
                return result;
            } else {
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = makeNull();
                while (args->type == CONS_TYPE) {
                    Value *evaled = eval(car(args), frame);
                    evaledArgs = cons(evaled, evaledArgs);
                    evaledArgs = reverse(evaledArgs);
                    args = cdr(args);
                }
                //evaledArgs = reverse(evaledArgs);
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
    return makeNull();
}


