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

/*hashԪ��*/
#pragma pack(1)
typedef struct {
	key_type key;                   /*hash��ֵ*/
	value_type value;               /*ֵ*/
	hash_stat stat;                 /*״̬*/
}hash_elem;
#pragma pack()

/*hash table*/
typedef struct {
	hash_elem data[HASH_MAX_SIZE];   /*hash��Ԫ���б�*/
	char size;                       /*Ԫ�ظ���*/
	hash_fun func;                   /*hash����*/
}hash_table;


void hash_table_init(hash_table *table, hash_fun func);
void hash_table_destroy(hash_table *table);
int hash_insert(hash_table *table, key_type key, value_type value);
value_type hash_find(hash_table *table, key_type key);
void hash_remove(hash_table *table, key_type key);

#endif

