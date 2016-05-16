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
/*struct Frame {
    Value *bindings;
    struct Frame *parent;
};*/

Frame *newFrameLet(Frame *frames, Value *val){
    printf("test4");
    val = car(val);
    Frame *newFrame = talloc(sizeof(frames));
    newFrame->bindings = val;
    newFrame->parent = frames;
    return newFrame;
}

//typedef struct Frame Frame;

void interpret(Value *tree){
    printf("test3");
    Value *answer;
    Frame *globalFrame = talloc(sizeof(Frame));
    globalFrame->bindings = makeNull();
    while (tree->type == CONS_TYPE){
        answer = eval(car(tree), globalFrame);
        tree = cdr(tree);
    }
}

                   
Value *evalIf(Value *args, Frame *frames){
    printf("test2");
    Value *answer = eval(car(args), frames);
    if (answer->c.car->i == '1'){
        return eval(car(cdr(args)), frames);
    }
    else {
        return eval(car(cdr(cdr(args))), frames);
    }
}


Value *eval(Value *tree, Frame *frame) {
    printf("test1");
    Value *frames;
    Value *result;
    switch (tree->type)  {
        case INT_TYPE: {
        printf("%d ", tree->c.car->i);
        break;
     }
     case CONS_TYPE: {
        Value *first = car(tree);
        Value *args = cdr(tree);

        // Sanity and error checking on first...

        if (!strcmp(first->s,"if")) {
            result = evalIf(args,frame);
        }
        else if (!strcmp(first->s,"let")) {
            frames = newFrameLet(args,frame);
            result = eval(car(cdr(args)), frame);
              }
          }
      }
        //break;
/*     case ......: {
        ...
        break;
     }  
     case SYMBOL_TYPE: {
        return lookUpSymbol(tree, frame);
        break;*/
        // .. other special forms here...
/*
        else {
           // not a recognized special form
           evalationError();
        }*/
     
    }


