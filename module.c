//
//  module.c
//  tapir
//
//  Created by Rodrigo Agustin Peinado on 12/5/16.
//  Copyright © 2016 Rodrigo Agustin Peinado. All rights reserved.
//

#include "redismodule.h"
#include <stdlib.h>
#include <string.h>

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/**
 DYNRATELIM <key> <max_burst> <count per period> <period>
 */
int DynRateLimCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    
    if (argc < 4) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
    RedisModuleCallReply *reply;
    
    size_t strlenn;
    const char* theKey = RedisModule_StringPtrLen(argv[1], &strlenn);
    char* tsKey = concat("TAPIR", theKey);

    // get the average response time
    reply = RedisModule_Call(ctx,"TS.GET","ss",RedisModule_CreateString(ctx, tsKey, strlen(tsKey)),RedisModule_CreateString(ctx, "avg", 3));

    long long avgResponseTime = 0;
    if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_INTEGER) {
        avgResponseTime = RedisModule_CallReplyInteger(reply);
    } else if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_STRING) {
        avgResponseTime = RedisModule_CallReplyInteger(reply);
    } else if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_NULL) {
        avgResponseTime = 0;
    } else if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR) {
        avgResponseTime = 0;
    }
    
    RedisModule_FreeCallReply(reply);
    
    long long tokens = 1;
    
    if (avgResponseTime > 500) {
        tokens = 2;
    }
    RedisModuleString *sTokens = RedisModule_CreateStringFromLongLong(ctx, tokens);
    reply = RedisModule_Call(ctx,"CL.THROTTLE","sssss",argv[1],argv[2],argv[3],argv[4],sTokens);
    
    if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR) {
        return RedisModule_ReplyWithCallReply(ctx, reply);
    }
    
    //RedisModule_ReplyWithLongLong(ctx,(long long)argv[1]);
    RedisModule_ReplyWithCallReply(ctx,reply);
    
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx) {
    
    // Register the module itself - it's called example and has an API version of 1
    if (RedisModule_Init(ctx, "tapir", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    
    // register our command - it is a write command, with one key at argv[1]
    if (RedisModule_CreateCommand(ctx, "TAPIR.DYNRATELIM", DynRateLimCommand, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    return REDISMODULE_OK;
}
