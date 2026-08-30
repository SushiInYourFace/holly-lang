#include "eval.h"
#include <stdbool.h>
#include <stdio.h>

#include "errors.h"
#include "logging.h"
#include "parsing.h"
#include "types.h"
#include "values.h"
#include "functions.h"
#include "variables.h"

static void cleanEnv(Environment *env) {
    VarEntry *cur, *tmp; //need these for iter
    uint16_t count = 0;
    HASH_ITER(hh, env->var_table, cur, tmp) {
        count++;
        HASH_DEL(env->var_table, cur);
        free(cur);  
    }
    logNode("Freed %d var items", count);
}


Value eval(TreeNode *node, Environment *env, FunctionEntry *fun_hash) {
    switch(node->type) { //evaluate differently based on what sort of node we're dealing with
        case(NODE_VALUE):
            char node_val_str[VAL_STR_LEN];
            valueToString(node->value, node_val_str,VAL_STR_LEN);
            logNode("Evaluating value node with value %s, type %s",
                node_val_str, 
                getValueTypeString(node->value.type));
            return node->value;

        case(NODE_BINARY):
            Value left = eval(node->binary.left, env, fun_hash); //evaluate the left hand side of the tree
            Value right = eval(node->binary.right, env, fun_hash); //ditto
            switch(node->binary.action) {
                case TK_PLUS: return addValues(left, right);
                case TK_MINUS: return subValues(left, right);
                case TK_MULT: return multValues(left, right);
                case TK_DIV: return divValues(left, right);
                case TK_MODULO: return modValues(left, right);
                default: raiseError(ERR_UNKNOWN_BINARY_OP);
            }
        case(NODE_VAR_REFERENCE):
            //if var exists, send the value
            Value var_val = getVarValue(env, node->var_reference.name);
            char var_val_str[VAL_STR_LEN];
            valueToString(var_val, var_val_str, VAL_STR_LEN);
            logNode("Evaluated var reference for %s, currently holding value %s", node->var_reference.name, var_val_str);
            return var_val;

        case(NODE_BLOCK): 
            //eval each node inside the block
            //first set up the variable environment
            Environment block_env;
            block_env.parent = env;
            block_env.var_table = NULL;
            block_env.should_return = false;
            block_env.should_break = false;
            block_env.return_val = VALUE_EMPTY;
            logNode("Parsing block type %d", node->block.type);
            for(size_t i = 0; i < node->block.len; i++) {
                Value statement = eval(node->block.children[i], &block_env, fun_hash); //intermediate statement for readout
                char statement_str[VAL_STR_LEN];
                valueToString(statement, statement_str, VAL_STR_LEN);
                logNode(
                    "Evaluated item %zu of a length %zu node block, return value %s", 
                    i, node->block.len, statement_str);
                if(block_env.should_return || block_env.should_break) {
                    break;
                }
            }
            if(block_env.should_break) { //break logic
                if(!block_env.parent || node->block.type == BLOCK_FUNCTION) { //functions have param env above
                    raiseError(ERR_BREAK_NO_LOOP); //got to top level without encountering a loop
                }
                block_env.parent->should_break = true; //flag will be reset by the while loop
                cleanEnv(&block_env);
                return VALUE_EMPTY;
            } //if no break, parse as a return
            if(block_env.parent) { //no return value if inner block
                cleanEnv(&block_env);
                logNode("Evaluated inner block node");
                //pass a return down the line
                env->should_return = block_env.should_return;
                env->return_val = block_env.return_val; //pass return down the chain
                return VALUE_EMPTY;
            }
            //get return val from the block if it was the top level
            Value ret = block_env.return_val;
            cleanEnv(&block_env); //cleanup the var hash
            return ret;
        
        case(NODE_VAR_ASSIGN):
            Value assign = eval(node->var_assign.val, env, fun_hash);
            //varname is still owned by tree
            addVarToHash(env, node->var_assign.name, assign, node->var_assign.final);
            const char* fin_string = node->var_assign.final ? "final" : "";
            char assign_string[VAL_STR_LEN];
            valueToString(assign, assign_string, VAL_STR_LEN);
            logNode("Encountered var assign block, %s var %s being set to %s", 
                fin_string, node->var_assign.name, assign_string);
            break;
        case(NODE_VAR_REASSIGN):
            Value reassign = eval(node->var_reassign.val, env, fun_hash);
            updateVarValue(env, node->var_reassign.name, reassign);
            char reassign_str[VAL_STR_LEN];
            valueToString(reassign, reassign_str, VAL_STR_LEN);
            logNode("Reassigned var %s to value %s", node->var_reassign.name, reassign_str);
            return VALUE_EMPTY;
        case(NODE_VAR_FINALIZE):
            finalizeVar(env, node->finalize.name);
            logDebug("Finalized var %s", node->finalize.name);
            return VALUE_EMPTY;
        case(NODE_DISPLAY):
            Value display_val = eval(node->display.val, env, fun_hash);
            //TODO: make this something more external
            char display_val_str[VAL_STR_LEN];
            valueToString(display_val, display_val_str, VAL_STR_LEN);
            logNode("Displaying value %s", display_val_str);
            //real print to out
            printf("Display: %s\n", display_val_str);
            return display_val;
        case(NODE_CONDITIONAL):
            Value should_run = eval(node->conditional.condition, env, fun_hash); //conditions use parent env
            if(should_run.type != VAL_BOOL) raiseError(ERR_EXP_BOOL);
            if(should_run.val_bool) { //run if we should
                Value body_val = eval(node->conditional.body, env, fun_hash);
                char body_val_str[VAL_STR_LEN];
                valueToString(body_val, body_val_str, VAL_STR_LEN);
                logNode("Evaluated a conditional block! It returned %s", body_val_str);
            } else {
                if(node->conditional.else_body) {
                    logNode("Taking the 'else' path of a conditional node");
                    eval(node->conditional.else_body, env, fun_hash);
                } else {
                    logNode("Conditional node being evaluated has no else block");
                }
            }
            return VALUE_EMPTY;
        case(NODE_WHILE_LOOP):
            logNode("Encountered while loop");
            Value w_should_run = eval(node->while_loop.condition, env, fun_hash); //initial condition
            if(w_should_run.type != VAL_BOOL) raiseErrorWithCtx(ERR_EXP_BOOL, CTX_1TKTYPE, w_should_run.type);
            int num_iters = 0; //safeguard against infinite loop. Probably will be removed at some point
            while(w_should_run.val_bool && num_iters < MAX_LOOP_ITERS) {
                eval(node->while_loop.body, env, fun_hash);
                num_iters++;
                w_should_run = eval(node->while_loop.condition, env, fun_hash);
                //handle returns
                if(env->should_return) {
                    env->parent->should_return = true;
                    env->parent->return_val = env->return_val;
                    return VALUE_EMPTY;
                }
                if(env->should_break) {
                    env->should_break = false; //prevent it from leaking higher
                    return VALUE_EMPTY;
                }
            }
            logDebug("Iterated a while loop %d times", num_iters);
            if(num_iters >= MAX_LOOP_ITERS) sendWarning(WARN_MAX_LOOP_ITERS);
            return VALUE_EMPTY;
        case(NODE_CONDITION):
            Value left_res = eval(node->condition.left, env, fun_hash);
            if(node->condition.comparator == TK_EMPTY) {                 //no comparison
                return (isTruthy(left_res)) ? VALUE_TRUE : VALUE_FALSE;
            }
            Value right_res = eval(node->condition.right, env, fun_hash);
            bool node_res = evalExpression(left_res, right_res, node->condition.comparator);
            return (node_res) ? VALUE_TRUE : VALUE_FALSE;
        case(NODE_FUN_CALL):
            logNode(
                "Encountered fun call for fun %s, providing %zu params!", 
                node->fun_call.fun_name, node->fun_call.params.count);
            FunctionEntry *fun = findFunction(node->fun_call.fun_name, fun_hash);
            if(!fun) raiseError(ERR_UNDEF_FUN);
            //check for correct num of params
            if(fun->params.count != node->fun_call.params.count) {
                raiseErrorWithCtx(ERR_INVALID_NUM_PARAMS, CTX_2SIZE, fun->params.count, node->fun_call.params.count);
            }
            Environment fun_env = {
                .parent = NULL, //no parent variables in fun calls
                .var_table = NULL,
                .should_return = false,
                .should_break = false,
                .return_val = VALUE_EMPTY
            };

            for(size_t i = 0; i < fun->params.count; i++) { //add params to the fun environment
                addVarToHash(
                    &fun_env,  //new param env
                    fun->params.list[i], //param name from fun dec
                    eval(node->fun_call.params.list[i], env, fun_hash), //param value from node
                    false //not final
                );
            }
            eval(fun->fun_block, &fun_env, fun_hash); //return gets other functions, but not variables
            Value fun_ret = fun_env.return_val; //read the return value from the function environment
            cleanEnv(&fun_env);
            //check ret against declared ret type
            if(fun_ret.type != fun->return_type) {
                raiseErrorWithCtx(ERR_MISMATCH_FUN_RETURN, CTX_2VAL,
                fun->return_type, fun_ret.type);
            }
            return fun_ret;
        case(NODE_RETURN):
            logNode("Encountered a return call");
            env->should_return = true;
            env->return_val = eval(node->node_return.val, env, fun_hash);
            return VALUE_EMPTY;
        case(NODE_BREAK):
            logNode("Encountered a break node");
            env->should_break = true;
            return VALUE_EMPTY;

        
        default:
            raiseError(ERR_UNKNOWN_NODE_TYPE);
    }
    return VALUE_EMPTY;
}