#include "eval.h"
#include <stdbool.h>
#include <stdio.h>

#include "errors.h"
#include "logging.h"
#include "parsing/parsing.h"
#include "types.h"
#include "values.h"
#include "functions/functions.h"
#include "variables.h"

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
            Value out = VALUE_VOID;
            switch(node->binary.action) {
                case TK_PLUS: out = addValues(left, right);     break;
                case TK_MINUS: out = subValues(left, right);   break;  
                case TK_MULT: out = multValues(left, right);   break;  
                case TK_DIV: out = divValues(left, right);     break;
                case TK_MODULO: out = modValues(left, right);  break;
                default: raiseError(ERR_UNKNOWN_BINARY_OP);
            }
            addValToEnv(env, out); //ensure the value is added to the hash to properly be cleaned
            return out;
        case(NODE_VAR_REFERENCE):
            //parse indexed reference first
            if(node->var_reference.has_index) {
                logEval(
                    "Evaluating indexed var reference for var %s, index %zu", 
                    node->var_reference.name, 
                    node->var_reference.index
                );
                Value index_val = getVarArrayValueAtPos(env, node->var_reference.name, node->var_reference.index);
                return index_val;
            }
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
            initEnv(&block_env, env);
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
                Value ret = dupVal(block_env.return_val); //grab the value before env is cleaned
                addValToEnv(env, ret); //add it to env so it can be freed
                cleanEnv(&block_env);
                logNode("Evaluated inner block node");
                //pass a return down the line
                env->should_return = block_env.should_return;
                env->return_val = ret; //pass return down the chain
                return VALUE_VOID;
            }
            //get return val from the block if it was the top level
            Value ret = dupVal(block_env.return_val);
            if(env) addValToEnv(env, ret); //add value to env. Don't add if there's no parent
            cleanEnv(&block_env); //cleanup the var hash
            return ret;
        
        case(NODE_VAR_ASSIGN):
            if(node->var_assign.array) { //handle array dec
                if(node->var_assign.final) raiseError(ERR_UNSET_FINAL);
                addArrayVarToHash(env, node->var_assign.name, node->var_assign.array_size);
                return VALUE_VOID;
            }
            if(!node->var_assign.set) { //handle declare without set
                if(node->var_assign.final) raiseError(ERR_UNSET_FINAL); //can't declare final as unset
                addUnsetVarToHash(env, node->var_assign.name); //varname owned by tree still
                return VALUE_VOID;
            }
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
            if(node->var_reassign.has_index) {
                updateVarValueAtIndex(env, node->var_reassign.name, reassign, node->var_reassign.index);
                return VALUE_VOID;
            }
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
            return VALUE_VOID;
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
                    return VALUE_VOID;
                }
                if(env->should_break) {
                    env->should_break = false; //prevent it from leaking higher
                    return VALUE_VOID;
                }
            }
            logDebug("Iterated a while loop %d times", num_iters);
            if(num_iters >= MAX_LOOP_ITERS) sendWarning(WARN_MAX_LOOP_ITERS);
            return VALUE_EMPTY;
        case(NODE_NUM_LOOP):
            logNode("Encountered num loop!");
            Value iter_value = eval(node->num_loop.number, env, fun_hash);
            if(iter_value.type != VAL_INT) raiseError(ERR_EXP_INT);
            if(iter_value.val_int < 0) {
                sendWarning(WARN_NEG_NUM_LOOP);
                return VALUE_VOID;
            }
            for(int i = 0; i < iter_value.val_int; i++) {
                eval(node->num_loop.body, env, fun_hash);
                if(env->should_break) {
                    env->should_break = false;
                    return VALUE_VOID;
                }
                if(env->should_return) {
                    env->parent->should_return = true;
                    env->parent->return_val = env->return_val;
                    return VALUE_VOID;
                }
            }
            return VALUE_VOID;
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
            if(fun->is_builtin) { //builtins do their own param checking
                ParamValues vals; //array to store values that will be passed to builtin
                vals.count = node->fun_call.params.count;
                for(size_t i = 0; i < vals.count; i++) {
                    vals.list[i] = eval(node->fun_call.params.list[i], env, fun_hash);
                }
                Value ret = fun->builtin(vals); //some builtins malloc data
                addValToEnv(env, ret);
                return ret; //no need to type check here, should be done inside the builtin

            }
            if(fun->param_names.count != node->fun_call.params.count) {
                raiseErrorWithCtx(ERR_INVALID_NUM_PARAMS, CTX_2SIZE, fun->param_names.count, node->fun_call.params.count);
            }
            Environment fun_env;
            initEnv(&fun_env, NULL); //funs can't see parent env
            for(size_t i = 0; i < fun->param_names.count; i++) { //add params to the fun environment
                addVarToHash(
                    &fun_env,  //new param env
                    fun->param_names.list[i], //param name from fun dec
                    eval(node->fun_call.params.list[i], env, fun_hash), //param value from node
                    false //not final
                );
            }
            eval(fun->fun_block, &fun_env, fun_hash); //return gets other functions, but not variables
            //NOTE: functions are not top level because of the param environment. So if their returns are allocated data, 
            //      they need to be detached
            Value fun_ret = dupVal(fun_env.return_val); //read the return value from the function environment
            addValToEnv(env, fun_ret); //if the return has allocated data, needs to be added to parent
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
            return VALUE_VOID;
        case(NODE_BREAK):
            logNode("Encountered a break node");
            env->should_break = true;
            return VALUE_VOID;

        
        default:
            raiseError(ERR_UNKNOWN_NODE_TYPE);
    }
    return VALUE_VOID;
}