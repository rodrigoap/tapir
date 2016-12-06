//
//  module.c
//  tapir
//
//  Created by Rodrigo Agustin Peinado on 12/5/16.
//  Copyright © 2016 Rodrigo Agustin Peinado. All rights reserved.
//

#include "redismodule.h"

/**
 DYNRATELIM <key>
 */
int DynRateLimCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    
    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    RedisModule_ReplyWithLongLong(ctx,RedisModule_GetSelectedDb(ctx));
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx) {
    
    // Register the module itself - it's called example and has an API version of 1
    if (RedisModule_Init(ctx, "tapir", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    
    // register our command - it is a write command, with one key at argv[1]
    if (RedisModule_CreateCommand(ctx, "TAPIR.DYNRATELIM", DynRateLimCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    return REDISMODULE_OK;
}
