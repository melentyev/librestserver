#ifndef _RSTUTIL
#define _RSTUTIL 1

#include <stdlib.h>

#define malloc MALLOC_NOT_ALLOWED
#define realloc REALLOC_NOT_ALLOWED
#define free FREE_NOT_ALLOWED

#define RST_ERROR(a) { printf("RST_ERROR: %s", a); return 1; }
#define RST_CRITICAL(a) { printf("RST_CRITICAL: %s", a); exit(1); }
#define RST_LOG_FMT fprintf

#define RST_ALLOC(size, tag) RST_alloc_tag(size, tag)
#define RST_REALLOC(memory, size, tag) RST_realloc_tag(memory, size, tag)
#define RST_FREE(memory, tag) RST_free_tag(memory, (tag))

#define RST_ALLOC_STRUCT(S, tag) (S*)RST_alloc_tag(sizeof(S), (tag))

#define SCHEDULE2(tp, func, p1, p2) { void **ps = (void**)RST_alloc(sizeof(void*) * 2); ps[0] = (void*)(p1); ps[1] = (void*)(p2); thpool_add_work(tp, func, (void*)ps); }

#define RST_VECTOR_PUSH_BACK(x) RST_vector_push_back(0, (x))

typedef int(RST_abstract_comparator)(void*, void*);

//RST_Vector* RST_vector_init(size_t element_size);

typedef struct RST_Vector
{
    void** data;
    int size;
    int capacity;
    void(*destructor)(void*);
} RST_Vector;

typedef struct RST_VectorIterator
{
    RST_Vector* v;
    int k;
} RST_VectorIterator;

typedef struct RST_Map
{
    RST_Vector* v;
    RST_abstract_comparator* comparator;
    void(*key_destructor)(void*);
    void(*value_destructor)(void*);
} RST_Map;

typedef struct RST_MapIterator
{
    RST_Map* m;
    int k;
} RST_MapIterator;

typedef struct RST_Set
{
    RST_Vector* v;
    RST_abstract_comparator* comparator;
    void(*destructor)(void*);
} RST_Set;

typedef struct RST_SetIterator
{
    RST_Set* s;
    int k;
} RST_SetIterator;

typedef struct RST_String_builder
{
    char *buf;
    size_t len;
} RST_String_builder;
    
/*RST_JsonNode* RST_json_get_node(RST_JsonNode* root, const char* path, ...);
{
    va_list ap;
    va_start
    va_end(ap);
    return NULL;
}*/

char *RST_alloc_string(const char* src);
char *RST_alloc_string_n(const char* src, int len);

///////////// RST_String_builder ///////////////
RST_String_builder* RST_string_builder_init();
RST_String_builder* RST_string_builder_init_with(const char* str); 
RST_String_builder* RST_string_builder_init_with_n(const char* str, size_t len);

void RST_string_builder_append(RST_String_builder *sb, const char* str);
void RST_string_builder_append_int(RST_String_builder *sb, int x);
void RST_string_builder_append_n(RST_String_builder *sb, const char* str, size_t len);
void RST_string_builder_append_char(RST_String_builder *sb, char c);
void RST_string_builder_append_builder(RST_String_builder *sb, RST_String_builder *sb2);

void RST_string_builder_slice_inplace(RST_String_builder* sb, size_t pos, size_t len);
char* RST_string_builder_move_data(RST_String_builder* sb);

void RST_string_builder_clear(RST_String_builder* sb);
void RST_string_builder_release(RST_String_builder* sb);


void RST_string_builder_dtor(void *x);
/*
void* RST_box_int(int x);
void RST_boxed_int_dtor(void *x);
*/
void RST_cstr_dtor(void *x);

int RST_int_comparator(void *a, void *b);
int RST_cstr_comparator(void *a, void *b);

///////////// RST_Map ///////////////

RST_Map* RST_map_init(RST_abstract_comparator comparator, void(*key_destructor)(void*), void(*value_destructor)(void*));
int RST_map_find(RST_Map *m, void* key, void** out_value);
void RST_map_insert(RST_Map *m, void *key, void *value);
void RST_map_erase(RST_Map *m, void *key);
RST_MapIterator RST_map_begin(RST_Map *m);
int RST_map_end(RST_MapIterator it);

void RST_map_iterator_next(RST_MapIterator *it); 
void RST_map_iterator_value(RST_MapIterator it, void **key, void **value);

void RST_map_release(RST_Map*);

///////////// RST_Vector ///////////////
RST_Vector* RST_vector_init(void(*destructor)(void*));
void RST_vector_push_back(RST_Vector*, void *value);
void RST_vector_erase(RST_Vector* s, int k);

/////// DO NOT FREE MEMORY
void RST_vector_clear(RST_Vector* s);
void RST_vector_release(RST_Vector*);

RST_VectorIterator RST_vector_begin(RST_Vector* s);
int RST_vector_end(RST_VectorIterator it);

void RST_vector_iterator_next(RST_VectorIterator *it);
void* RST_vector_iterator_value(RST_VectorIterator it);

///////////// RST_Set ///////////////

RST_Set* RST_set_init(RST_abstract_comparator comparator, void(*destructor)(void*));
void RST_set_insert(RST_Set* s, void* x);
int RST_set_contains(RST_Set* s, void* x);
void RST_set_erase(RST_Set* s, void* x);
void* RST_set_max_element(RST_Set* s);
int RST_set_empty(RST_Set* s);
void RST_set_release(RST_Set* s);

RST_SetIterator RST_set_begin(RST_Set* s);
int RST_set_end(RST_SetIterator it);

void RST_set_iterator_next(RST_SetIterator *it);
void* RST_set_iterator_value(RST_SetIterator it);

///////////// Other ///////////////

int RST_max(int a, int b);
int RST_min(int a, int b);
void RST_log_console(const char* s);

void RST_debug_init();

char *RST_str_replace(char *orig, char *rep, char *with);

/////// MEMORY CONTROL ////////

void* RST_alloc(size_t size);
void* RST_realloc(void* memory, size_t size);
void RST_free(void *ptr);

void* RST_alloc_tag(size_t size, const char *tag);
void* RST_realloc_tag(void *memory, size_t size, char *tag);
void RST_free_tag(void *memory, const char *tag);

#endif