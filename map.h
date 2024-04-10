#ifndef custom_map
#define custom_map

#if !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif
#ifndef _INC_STRING
#include <string.h>
#endif
#ifndef _MALLOC_H_
#include <malloc.h>
#endif

typedef struct
{
    char *key;
    char *value;
} KeyValuePair;

typedef struct Map
{
    KeyValuePair *items;
    size_t length;
    char *(*get)(struct Map map, const char *key);
    void (*set)(struct Map *map, const char *key, const char *value);
} Map;

char *Map_get(Map map, const char *key)
{
    for (size_t i = 0; i < map.length; i++)
    {
        KeyValuePair item = map.items[i];
        if (strcmp(key, item.key) == 0)
            return item.value;
    }
    return NULL;
}

void Map_set(Map *map, const char *key, const char *value)
{
    // Updating KeyValuePair in Map
    for (size_t i = 0; i < map->length; i++)
    {
        KeyValuePair item = map->items[i];
        if (strcmp(key, item.key) == 0)
        {
            item.value = strdup(value);
            return;
        }
    }

    // Adding to Map
    map->items = (KeyValuePair *)realloc(map->items, ++(map->length) * sizeof(KeyValuePair));
    map->items[map->length - 1].key = strdup(key);
    map->items[map->length - 1].value = strdup(value);
}

Map initMap()
{
    Map newMap = *(Map *)malloc(sizeof(Map));
    newMap.items = NULL;
    newMap.length = 0;
    newMap.get = Map_get;
    newMap.set = Map_set;
    return newMap;
}

#endif
