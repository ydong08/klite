KLite简介
=====================
KLite是免费开源软件,基于 MIT 协议开放源代码.
作者: 蒋晓岗\<kerndev@foxmail.com> 保留所有权利.

# 一.简介
        KLite是一个由个人开发者利用业余时间编写，创建于2015年5月6日，以MIT协议开放源代码。
        它是一款入门级的小型抢占式操作系统内核，年轻人的第一款RTOS。
        以简洁易用为设计目标，旨在降低学习嵌入式操作系统编程入门的难度。
        简洁的API风格,简洁的调用方式,简单的移植方法,可能是目前最简单易用的嵌入式操作系统内核.
        --支持优先级抢占  
        --支持相同优先级的线程
        --支持线程同步互斥  
        --支持动态内存管理
        --支持多编译器GCC, IAR, MDK

# 二.移植
        KLite目前已经为Cortex-M0/M3/M4架构做好了底层适配,如果你的CPU平台是基于以上平台的,
        如STM32,GD32,NRF51,NRF52,Freescale K40等系列单片机,那么可以直接使用预编译的库文件进行开发,
        只需要修改port.c里面几个简单的函数即可开始编程.
        否则可能需要自行移植CPU底层的汇编代码.

# 三.开始使用
## 1.编译KLite
        在build目录下面有预设的工程文件,选择你要使用的编译器和目标CPU平台,  
        编译完成后,会生成kernel.lib文件,将kernel.lib, kernel.h, port.c复制到你的项目源码中,
        使用lib文件可以减少重复编译时间.

## 2.修改port.c
        根据目标CPU的编程手册,实现port.c里面的3个空函数.
        void cpu_os_init(void)
        这个函数被kernel_init调用,为用户提供一个接口,用于实现那些必须在系统初始化之前的准备工作,
        例如初始化CPU的时钟, 设置FLASH等.
        
        void cpu_os_start(void)
        这个函数被kernel_start调用,为用户提供一个接口,用于实现那些必须在系统初始化之后,系统启动之前的准备工作,
        例如初始化滴答定时器,设置系统中断等.
        
        void SysTick_Handler(void)
        这个函数是平台相关的滴答时钟中断函数,需要在滴答时钟中断中调用kernel_timetick(n),n表示一次中断的毫秒数.

## 3.在main函数里面添加初始化代码
    main函数的推荐写法如下:
```
//用于初始化程序的线程
void init(void *arg)
{
    bsp_init();
    app_init();
}

//空闲线程,需要调用thread_cleanup回收已结束的线程资源
void idle(void *arg)
{
    thread_setprio(thread_self(), THREAD_PRIORITY_IDLE);
    while(1)
    {
        thread_cleanup();
    }
}

//C语言程序入口
void main(void)
{
    static uint8_t heap[HEAP_SIZE];
    kernel_init((uint32_t)heap, HEAP_SIZE);
    thread_create(init, 0, 0);
    thread_create(idle, 0, 0);
    kernel_start();
}
```
    说明:
    kernel_init 用于初始化内核;
    thread_create 创建主线程init和idle;  
    kernel_start 用于启动内核;  
    init是一个线程函数,在该函数中实现你的其它初始化代码.  
    更多函数参数说明请参考API文档.  

# 四.支持
    如果你在使用中发现任何BUG或者改进建议,请加入我的QQ群(317930646)或发送邮件至kerndev@foxmail.com.  

    