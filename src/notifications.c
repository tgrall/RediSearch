
#include "notifications.h"
#include "spec.h"

RedisModuleString *global_RenameFromKey = NULL;

int HashNotificationCallback(RedisModuleCtx *ctx, int type, const char *event, RedisModuleString *key) {

#define CHECK_CACHED_EVENT(E) \
  if (event == E##_event) { \
    E = true; \
  }

#define CHECK_AND_CACHE_EVENT(E) \
  if (!strcmp(event, #E)) { \
    E = true; \
    E##_event = event; \
  }

  static const char *hset_event = 0, *hmset_event = 0, *del_event = 0,
                    *hdel_event = 0, *set_event = 0,
                    *rename_from_event = 0, *rename_to_event = 0;
  bool hset = false, hmset = false, del = false, hdel = false, set = false,
       rename_from = false, rename_to = false;

       CHECK_CACHED_EVENT(hset)
  else CHECK_CACHED_EVENT(hmset)
  else CHECK_CACHED_EVENT(del)
  else CHECK_CACHED_EVENT(hdel)
  else CHECK_CACHED_EVENT(set)
  else CHECK_CACHED_EVENT(rename_from)
  else CHECK_CACHED_EVENT(rename_to)
  else {
         CHECK_AND_CACHE_EVENT(hset)
    else CHECK_AND_CACHE_EVENT(hmset)
    else CHECK_AND_CACHE_EVENT(del)
    else CHECK_AND_CACHE_EVENT(hdel)
    else CHECK_AND_CACHE_EVENT(set)
    else CHECK_AND_CACHE_EVENT(rename_from)
    else CHECK_AND_CACHE_EVENT(rename_to)
  }

  const char *key_cp = RedisModule_StringPtrLen(key, NULL);
  if (hset || hmset || hdel) {
    Indexes_UpdateMatchingWithSchemaRules(ctx, key);
  }
  if (del || set) {
    Indexes_DeleteMatchingWithSchemaRules(ctx, key);
  }
  if (rename_from) {
    // Notification rename_to is called right after rename_from so this is safe.  
    global_RenameFromKey = key;
  }
  if (rename_to) {
    Indexes_ReplaceMatchingWithSchemaRules(ctx, global_RenameFromKey, key);
  }

  return REDISMODULE_OK;
}

void Initialize_KeyspaceNotifications(RedisModuleCtx *ctx) {
  RedisModule_SubscribeToKeyspaceEvents(ctx,
    REDISMODULE_NOTIFY_GENERIC | REDISMODULE_NOTIFY_HASH | REDISMODULE_NOTIFY_STRING,
    HashNotificationCallback);
}
