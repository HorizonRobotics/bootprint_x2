#pragma once
int init();
void uinit();
/*
 *从缓存队列取出一个buf，buf_index用于标识该buf
 */
int dequeue_buf(void** data, unsigned int* length, int* buf_index);
/*
 *将buf_index标识的buf入缓存队列，buf_index来自于dequeue_buf中输出的值
 */
int queue_buf(int buf_index);