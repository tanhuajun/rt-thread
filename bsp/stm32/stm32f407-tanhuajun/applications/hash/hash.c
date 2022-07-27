#include "hash.h"
//#include <stdio.h>
#include <rtthread.h>

/*hash表初始化*/
void hash_table_init(hash_table *table, hash_fun func) {
	table->size = 0;
	table->func = func;
	
	for(int i=0; i < HASH_MAX_SIZE; i++) {
		table->data[i].stat = empty;
	
	}
}

/*hash表销毁*/
void hash_table_destroy(hash_table *table) {
	table->size = 0;
	table->func = RT_NULL;
	for(int i=0; i < HASH_MAX_SIZE; i++)
		table->data[i].stat = empty;
}

/*hash表插入*/
int hash_insert(hash_table *table, key_type key, value_type value) {
	
	if(!table)
		return -1;
	
	/*通过key值计算偏移量*/
	char offset = table->func(key);
	if(offset > HASH_MAX_SIZE)
		return -1;
	
	/*hash表存满了*/
	if(table->size == HASH_MAX_SIZE) {
		rt_kprintf("hash insert err, the hash is full\n");
		return -1;
	}
	
	/*hash冲突 线性探测法*/
	while(1) {
		/*该位置没有放入任何值*/
		if(table->data[offset].stat == empty) {
			table->data[offset].key = key;
			table->data[offset].stat = valid;
			table->data[offset].value = value;
			
			table->size++;
			break;
		}
		else if(table->data[offset].stat == valid && table->data[offset].key == key) {
			/*该位置已存在值，且要键值相同，用新的val替换旧的val*/
			table->data[offset].value = value;
			break;
		}
		else {
			offset++;
			offset = offset >= HASH_MAX_SIZE ? 0 : offset;
		}
	}
	return 0;
}

/*查找*/
value_type hash_find(hash_table *table, key_type key) {
	
	if(table == NULL || table->size == 0)
		return NULL;
	
	char offset = table->func(key);
	char begin = offset;
	if(offset > HASH_MAX_SIZE)
		return NULL;
	
	while(1) {
		if(table->data[offset].key == key && table->data[offset].stat == valid) {
			return table->data[offset].value;
		}
		else {
			offset++;
			offset = offset >= HASH_MAX_SIZE ? 0 : offset;
			/*查找一圈回到开始的位置，查找失败，没有该key值*/
			if(offset == begin)
				return NULL;
		}
	}
}

/*删除*/
void hash_remove(hash_table *table, key_type key) {
	
	if(!table || table->size == 0)
		return;
	
	char offset = table->func(key);
	char begin = offset;
	if(offset > HASH_MAX_SIZE)
		return;
	
	while(1) {
		if(table->data[offset].key == key && table->data[offset].stat == valid) {
			table->data[offset].stat = empty;
			table->size--;
		}
		else {
			offset++;
			offset = offset >= HASH_MAX_SIZE ? 0 : offset;
			if(offset == begin)
				return;
		}
	}
}



