/*
* ����: ���ʽ���ڴ��
*       �ṩ��Ч��,�ڴ��С�̶���,��̬�ڴ��������ͷ�,
*       ��һ�����й����ڴ�ڵ�,����Ƶ�����,�������Ч��.
* Ӧ�ó���: ÿ��ֻ������һ���ڴ�, һ����������ڵ�Ķ�̬����������
* 
* ����: ������<kerndev@foxmail.com>
*/
#include "kernel.h"
#include "mpool.h"

struct mpool
{
    uint32_t *pmem_addr;
    uint32_t  next_free;
    uint32_t  next_alloc;
    uint32_t  free_block;
    uint32_t  total_block;
    mutex_t   mutex;
};

mpool_t mpool_create(uint32_t block_size, uint32_t block_count)
{
    uint32_t i;
    uint32_t mem_addr;
    struct mpool *mpool;
    
    mpool = heap_alloc((block_size + sizeof(uint32_t)) * block_count + sizeof(struct mpool));
    if(mpool == NULL)
    {
        return NULL;
    }
    mpool->mutex= mutex_create();
    if(mpool->mutex == NULL)
    {
        heap_free(mpool);
        return NULL;
    }
    
    mpool->pmem_addr = (uint32_t *)(mpool + 1);
    mem_addr = (uint32_t)(mpool->pmem_addr + block_count);
    mpool->next_free   = 0;
    mpool->next_alloc  = 0;
    mpool->free_block  = block_count;
    mpool->total_block = block_count;
    for(i=0; i<block_count; i++)
    {
        mpool_free(mpool,(void *)mem_addr);
        mem_addr += block_size;
    }
    return mpool;
}

void mpool_delete(mpool_t pool)
{
    struct mpool *mpool;
    mpool = (struct mpool *)pool;
    mutex_delete(mpool->mutex);
    heap_free(mpool);
}

void *mpool_alloc(mpool_t pool)
{
    uint32_t addr;
    struct mpool *mpool;
    
    mpool = (struct mpool *)pool;
    if(mpool->free_block > 0)
    {
        mutex_lock(mpool->mutex);
        addr = mpool->pmem_addr[mpool->next_alloc];
        if(++mpool->next_alloc >= mpool->total_block)
        {
            mpool->next_alloc = 0;
        }
        mpool->free_block--;
        mutex_unlock(mpool->mutex);
        return (void *)addr;
    }
    return NULL;
}

void mpool_free(mpool_t pool, void *mem)
{
    uint32_t addr;
    struct mpool *mpool;
    
    mpool = (struct mpool *)pool;
    addr  = (uint32_t)mem;
    mutex_lock(mpool->mutex);
    mpool->pmem_addr[mpool->next_free] = addr;
    if(++mpool->next_free >= mpool->total_block)
    {
        mpool->next_free = 0;
    }
    mpool->free_block++;
    mutex_unlock(mpool->mutex);
}

