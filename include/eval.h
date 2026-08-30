#pragma once
#include "parsing/parsing.h"
#include "types.h"
#include "variables.h"

#define MAX_LOOP_ITERS (100000)

//evaluate a tree node
Value eval(TreeNode *node, Environment *env, FunctionEntry *fun_hash);