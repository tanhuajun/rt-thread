#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#define HASH_MAX_SIZE   50
typedef unsigned char key_type;
typedef void* value_type;
typedef int (*hash_fun)(key_type key);

typedef enum {
	empty,
	valid
}hash_stat;

/*hash元素*/
#pragma pack(1)
typedef struct {
	key_type key;                   /*hash键值*/
	value_type value;               /*值*/
	hash_stat stat;                 /*状态*/
}hash_elem;
#pragma pack()

/*hash table*/
typedef struct {
	hash_elem data[HASH_MAX_SIZE];   /*hash表元素列表*/
	char size;                       /*元素个数*/
	hash_fun func;                   /*hash函数*/
}hash_table;


void hash_table_init(hash_table *table, hash_fun func);
void hash_table_destroy(hash_table *table);
int hash_insert(hash_table *table, key_type key, value_type value);
value_type hash_find(hash_table *table, key_type key);
void hash_remove(hash_table *table, key_type key);

#endif

