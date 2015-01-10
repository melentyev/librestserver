#include "restserver.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <assert.h>

int RST_max(int a, int b)
{
    return a > b ? a : b;
}

/*void* RST_box_int(int x)
{
    void* res = RST_alloc(sizeof(int));
    memcpy(res, &x, sizeof(int));
    return res;
}

void RST_boxed_int_dtor(void *x)
{
    RST_free(x);
}

void RST_map_cstr_cstr_dtor(void *x, void *y)
{
    RST_free(x);
    RST_free(y);
}
*/

void RST_cstr_dtor(void *x)
{
    RST_FREE(x, "RST_cstr_dtor::x");
}

int RST_int_comparator(void *a, void *b)
{
    return (int)a - (int)b;
}

int RST_cstr_comparator(void *a, void *b)
{
    return strcmp((const char*)a, (const char*)b);
}

char *RST_alloc_string(const char* src)
{
    int len = strlen(src);
    return RST_alloc_string_n(src, len);
}

char *RST_alloc_string_n(const char* src, int len)
{
    char *res = RST_alloc(sizeof(char) * (len + 1));
    memcpy(res, src, len * sizeof(char));
    res[len] = (char)0;
    return res;
}

RST_String_builder* RST_string_builder_init()
{
    RST_String_builder* sb = (RST_String_builder*)RST_ALLOC(sizeof(RST_String_builder), "RST_string_builder_init::sb");
    sb->len = 0;
    sb->buf = RST_alloc_string("");
    return sb;
}

void RST_string_builder_append(RST_String_builder *sb, const char* str)
{
    RST_string_builder_append_n(sb, str, strlen(str));
}

void RST_string_builder_append_int(RST_String_builder *sb, int x)
{
    char buf[50];
    sprintf(buf, "%d", x);
    RST_string_builder_append(sb, buf);
}

void RST_string_builder_append_char(RST_String_builder *sb, char c)
{
    RST_string_builder_append_n(sb, &c, 1);
}

void RST_string_builder_append_builder(RST_String_builder *sb, RST_String_builder *sb2)
{
    RST_string_builder_append_n(sb, sb2->buf, sb2->len);
}

void RST_string_builder_append_n(RST_String_builder *sb, const char* str, int len)
{
    sb->buf = RST_REALLOC(sb->buf, sizeof(char) * (sb->len + len + 1), "RST_string_builder_append_n::sb->buf");
    memcpy(sb->buf + sb->len, str, len);
    sb->len += len;
    sb->buf[sb->len] = 0;
}


void RST_string_builder_release(RST_String_builder* sb)
{
    RST_FREE(sb->buf, "RST_string_builder_release::sb->buf");
    RST_FREE(sb, "RST_string_builder_release::sb");
}

RST_VectorIterator RST_vector_begin(RST_Vector* s)
{
    RST_VectorIterator it;
    it.v = s;
    it.k = 0;
    return it;
}

int RST_vector_end(RST_VectorIterator it)
{
    return it.k >= it.v->size;
}

void RST_vector_iterator_next(RST_VectorIterator *it)
{
    it->k++;
}

void* RST_vector_iterator_value(RST_VectorIterator it)
{
    return it.v->data[it.k];
}

RST_Vector* RST_vector_init(void(*destructor)(void*))
{
    RST_Vector* v = (RST_Vector*)RST_alloc(sizeof(RST_Vector));
    v->capacity = 2;
    v->size = 0;
    v->destructor = destructor;
    v->data = (void**)RST_alloc(sizeof(void*) * v->capacity);
    return v;
}

void RST_vector_push_back(RST_Vector* v, void *elem)
{
    if (v->size == v->capacity)
    {
        v->capacity *= 2;
        v->data = RST_REALLOC(v->data, sizeof(void*) * v->capacity, "RST_vector_push_back::v->data");
    }
    v->data[v->size++] = elem;
}

void RST_vector_erase(RST_Vector* v, int k)
{
    assert(k < v->size);
    for (int i = k; i < v->size - 1; i++)
    {
        v->data[i] = v->data[i + 1];
    }
    v->size--;
}

void RST_vector_clear(RST_Vector* v)
{
    if (v->destructor != NULL)
    {
        for (int i = 0; i < v->size; i++)
        {
            v->destructor(v->data[i]);
        }
    }
    v->size = 0;
}

void RST_vector_release(RST_Vector* v)
{
    if (v->destructor != NULL) {
        for (int i = 0; i < v->size; i++)
        {
            v->destructor(v->data[i]);
        }
    }
    RST_FREE(v->data, "RST_vector_release::v->data");
    RST_FREE(v, "RST_vector_release::v");
}

RST_Map* RST_map_init(RST_abstract_comparator comparator, void(*key_destructor)(void*), void(*value_destructor)(void*))
{
    RST_Map* m = (RST_Map*)RST_alloc(sizeof(RST_Map));
    m->comparator = comparator;
    m->key_destructor = key_destructor;
    m->value_destructor = value_destructor;
    m->v = RST_vector_init(NULL);
    return m;
}

int RST_map_contains(RST_Map* m, void* key)
{
    for (int i = 0; i < m->v->size; i += 2)
    {
        if (m->comparator(key, m->v->data[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int RST_map_find(RST_Map* m, void* key, void** out_value)
{
    for (int i = 0; i < m->v->size; i += 2)
    {
        if (m->comparator(key, m->v->data[i]) == 0)
        {
            *out_value = m->v->data[i + 1];
            return 1;
        }
    }
    return 0;
}

void RST_map_insert(RST_Map* m, void *key, void *value)
{
    if (RST_map_contains(m, key))
    {

    }
    else
    {
        RST_vector_push_back(m->v, key);
        RST_vector_push_back(m->v, value);
    }
}

void RST_map_erase(RST_Map *m, void *key)
{
    for (int i = 0; i < m->v->size; i += 2)
    {
        if (m->comparator(key, m->v->data[i]) == 0)
        {
            if (m->key_destructor)
            {
                m->key_destructor(m->v->data + i);
            }
            RST_vector_erase(m->v, i);
            if (m->value_destructor)
            {
                m->value_destructor(m->v->data[i]);
            }
            RST_vector_erase(m->v, i);
        }
    }
}

void RST_map_release(RST_Map* m)
{
    for (int i = 0; i < m->v->size; i += 2)
    {
        if (m->key_destructor)
        {
            m->key_destructor(m->v->data[i]);
        }
        if (m->value_destructor)
        {
            m->value_destructor(m->v->data[i + 1]);
        }
    }
    m->v->destructor = NULL;
    RST_vector_release(m->v);
    RST_FREE(m, "RST_map_release::m");
}

RST_MapIterator RST_map_begin(RST_Map* m)
{
    RST_MapIterator it;
    it.m = m;
    it.k = 0;
    return it;
}

int RST_map_end(RST_MapIterator it)
{
    return it.k >= it.m->v->size;
}

void RST_map_iterator_next(RST_MapIterator *it)
{
    it->k += 2;
}

void RST_map_iterator_value(RST_MapIterator it, void **key, void **value)
{
    *key = it.m->v->data[it.k];
    *value = it.m->v->data[it.k + 1];
}

///////////// RST_Set ///////////////

RST_Set* RST_set_init(RST_abstract_comparator comparator, void(*destructor)(void*))
{
    RST_Set* s = (RST_Set*)RST_alloc(sizeof(RST_Set));
    s->comparator = comparator;
    s->destructor = destructor;
    s->v = RST_vector_init(NULL);
    return s;
}
void RST_set_insert(RST_Set* s, void* x)
{
    if (!RST_set_contains(s, x))
    {
        RST_vector_push_back(s->v, x);
    }
}

int RST_set_contains(RST_Set* s, void* x)
{
    for (int i = 0; i < s->v->size; i++)
    {
        if (s->comparator(s->v->data[i], x) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void RST_set_erase(RST_Set* s, void* x)
{
    for (int i = 0; i < s->v->size; i++)
    {
        if (s->comparator(s->v->data[i], x) == 0)
        {
            RST_vector_erase(s->v, i);
            return;
        }
    }
}

void RST_set_release(RST_Set* s)
{
    s->v->destructor = s->destructor;
    RST_vector_release(s->v);
    RST_FREE(s, "RST_set_release::s");
}

void* RST_set_max_element(RST_Set* s)
{
    RST_SetIterator it = RST_set_begin(s);
    void* res = RST_set_iterator_value(it);
    RST_set_iterator_next(&it);
    for (; !RST_set_end(it); RST_set_iterator_next(&it))
    {
        void* cur = RST_set_iterator_value(it);
        if (s->comparator(cur, res) > 0)
        {
            res = cur;
        }
    }
    return res;
}

int RST_set_empty(RST_Set* s)
{
    return s->v->size == 0;
}

RST_SetIterator RST_set_begin(RST_Set* s)
{
    RST_SetIterator it;
    it.s = s;
    it.k = 0;
    return it;
}

int RST_set_end(RST_SetIterator it)
{
    return (it.k >= it.s->v->size);
}

void RST_set_iterator_next(RST_SetIterator *it)
{
    it->k++;
}

void* RST_set_iterator_value(RST_SetIterator it)
{
    return it.s->v->data[it.k];
}

void RST_log_console(const char *s)
{
    puts(s);
}

#undef malloc
#undef free
#undef realloc

int allocaddresses[100000];
int allocaddrcount;

#define GFILL 'Z'

typedef struct RST_GarbagePool
{
    unsigned char buf[1024 * 1024 * 200];   
    int allocations[100000];
    int allocations_sizes[100000];
    int allocations_count;
    int allocations_is_free[100000];
    int free_buf_ptr;
} RST_GarbagePool;

RST_GarbagePool gpool;

void RST_debug_init()
{
    gpool.allocations_count = 0;
    gpool.free_buf_ptr = 0;
    for (int i = 0; i < 4; i++) { gpool.buf[gpool.free_buf_ptr++] = GFILL; }
}

RST_garbagepool_check(size_t size)
{
    for (int i = 0; i < 4; i++) { assert(gpool.buf[i] == GFILL); }
    for (int i = 0; i < gpool.allocations_count; i++)
    {
        for (int j = 0; j < 4; j++) { assert(gpool.buf[gpool.allocations[i] + gpool.allocations_sizes[i] + j] == GFILL); }
    }
}

void* RST_garbagepool_alloc(size_t size)
{
    void* res = &(gpool.buf[gpool.free_buf_ptr]);
    gpool.allocations_sizes[gpool.allocations_count] = size;
    gpool.allocations_is_free[gpool.allocations_count] = 0;
    gpool.allocations[gpool.allocations_count++] = (int)res;
    gpool.free_buf_ptr += size;
    for (int i = 0; i < 4; i++) { gpool.buf[gpool.free_buf_ptr++] = GFILL; }
    return res;
}

void* RST_garbagepool_realloc(void* memory, size_t size)
{
    
    int found = -1;
    for (int i = 0; i < gpool.allocations_count; i++)
    {
        if (gpool.allocations[i] == (int)memory)
        {
            found = i;
        }
    }
    assert(found != -1);
    void* res = RST_garbagepool_alloc(size);
    memcpy(res, (void*)(gpool.allocations[found]), gpool.allocations_sizes[found]);
    if (found == 0x1b) {
        int a = 5;
    }
    gpool.allocations_is_free[found] = 1;
    return res;
}

void RST_garbagepool_free(void* memory)
{
    int found = -1;
    for (int i = 0; i < gpool.allocations_count; i++)
    {
        if (gpool.allocations[i] == (int)memory)
        {
            found = i;
        }
    }
    assert(found != -1 && !gpool.allocations_is_free[found]);
    if (found == 0x1b) { 
        int a = 5; 
    }
    gpool.allocations_is_free[found] = 1;
}

void RST_debug_assert_ptr_allocated(void *memory)
{
    int found = 0;
    for (int i = 0; i < allocaddrcount; i++)
    {
        if (allocaddresses[i] == (int)memory)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        int a = 8;
    }
    assert(found);
}

void* RST_alloc(size_t size)
{
    void *addr = RST_garbagepool_alloc(size); //malloc(size);
    allocaddresses[allocaddrcount++] = (int)addr;
    RST_LOG_FMT(stderr, "RST_alloc  addr(%08x) size(%d)\n", addr, size);
    return addr;
}

void* RST_realloc(void* memory, size_t size)
{
    RST_debug_assert_ptr_allocated(memory);
    void *addr = RST_garbagepool_realloc(memory, size);
    allocaddresses[allocaddrcount++] = (int)addr;
    RST_LOG_FMT(stderr, "RST_realloc  addr(%08x) size(%d) oldaddr(%08x)\n", addr, size, memory);
    return addr;
}

void RST_free(void *ptr)
{
    RST_debug_assert_ptr_allocated(ptr);
    for (int i = 0; i < allocaddrcount; i++) { if (allocaddresses[i] == (int)ptr) { allocaddresses[i] = 0; break; } }
    RST_LOG_FMT(stderr, "RST_free oldaddr(%08x)\n", ptr);
    RST_garbagepool_free(ptr);
}

void* RST_alloc_tag(size_t size, const char* tag)
{
    void *addr = RST_garbagepool_alloc(size);
    allocaddresses[allocaddrcount++] = (int)addr;
    RST_LOG_FMT(stderr, "ALLOC addr(%08x) size(%d) tag(%s)\n", addr, size, tag);
    return addr;
}

void* RST_realloc_tag(void* memory, size_t size, const char *tag)
{
    RST_debug_assert_ptr_allocated(memory);
    void *addr = RST_garbagepool_realloc(memory, size);
    allocaddresses[allocaddrcount++] = (int)addr;
    RST_LOG_FMT(stderr, "REALLOC addr(%08x) size(%d) oldaddr(%08x) tag(%s)\n", addr, size, memory, tag);
    return addr;
}

void RST_free_tag(void *ptr, const char *tag)
{
    RST_debug_assert_ptr_allocated(ptr);
    for (int i = 0; i < allocaddrcount; i++) { if (allocaddresses[i] == (int)ptr) { allocaddresses[i] = 0; break; } }
    RST_LOG_FMT(stderr, "FREE oldaddr(%08x) tag(%s)\n", ptr, tag);
    RST_garbagepool_free(ptr);
}