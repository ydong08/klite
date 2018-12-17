/****************************************************************************
* ����: �����ʱ��
*       �ɼ򻯶�ʱ����Ĵ����д
* ������<kerndev@foxmail.com>
* 2017.07.20 �����ļ�
* 2018.06.07 ��д�ļ�
****************************************************************************/
#include <string.h>
#include "kernel.h"
#include "list.h"
#include "timer.h"

struct timer_list
{
    timer_t *head;
    timer_t *tail;
    mutex_t  mutex;
    event_t  event;
};

static struct timer_list m_timer_list;

//������һ����ʱ����ʱʱ��
static uint32_t timer_timetick(uint32_t tick)
{
    uint32_t next_time;
    timer_t *node;
    next_time = 0xFFFFFFFF;
    mutex_lock(m_timer_list.mutex);
    for(node = m_timer_list.head; node!=NULL; node=node->next)
    {
        if(node->counter > tick)
        {
            node->counter -= tick;
        }
        else
        {
            node->handler(node->arg);
            node->counter = node->timeout;
        }
        if(node->counter < next_time)
        {
            next_time = node->counter;
        }
    }
    mutex_unlock(m_timer_list.mutex);
    return next_time;
}

//��ʱ���߳�
//���ȴ���һ����ʱ��������ô�õ�ʱ�䣬��Ҫȥ���һ��!
static void timer_thread_entry(void *arg)
{
    uint32_t last;
    uint32_t tick;
    uint32_t timeout;
    last = kernel_time();
    while(1)
    {
        tick = kernel_time() - last;
        last = kernel_time();
        timeout = timer_timetick(tick);
        tick = kernel_time() - last;
        if(timeout > tick)
        {
            event_timedwait(m_timer_list.event, timeout - tick);
        }
    }
}

//������ʱ��
void timer_start(timer_t *timer, uint32_t timeout, void (*handler)(void *), void *arg)
{
    timer->counter = timeout;
    timer->timeout = timeout;
    timer->arg     = arg;
    timer->handler = handler;
    mutex_lock(m_timer_list.mutex);
    list_append(&m_timer_list, timer);
    mutex_unlock(m_timer_list.mutex);
    event_post(m_timer_list.event);
}

//ֹͣ��ʱ��
void timer_stop(timer_t *timer)
{
    mutex_lock(m_timer_list.mutex);
    list_remove(&m_timer_list, timer);
    mutex_unlock(m_timer_list.mutex);
    event_post(m_timer_list.event);
}

//��ʼ����ʱ��ģ��
//���ö�ʱ���̵߳Ķ�ջ��С���߳����ȼ�
void timer_init(uint32_t stk_size, int prio)
{
    thread_t thread;
    list_init(&m_timer_list);
    m_timer_list.mutex = mutex_create();
    m_timer_list.event = event_create();
    thread = thread_create(timer_thread_entry, 0, stk_size);
    thread_setprio(thread, prio);
}
