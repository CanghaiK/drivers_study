#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>

struct timer_list g_timer;

void timer_func(struct timer_list *arg)
{
    printk("%s:current jiffies = %ld\n",__func__ ,jiffies);
    mod_timer(&g_timer,jiffies + HZ);
}

static __init int timer_init(void)
{
    timer_setup(&g_timer,timer_func,(unsigned long )0);
    printk("current jiffies = %ld\n",jiffies);
    g_timer.expires = jiffies + 2*HZ;
    add_timer(&g_timer);
    return 0;
}

static __exit void timer_exit(void)
{
    del_timer(&g_timer);
}
module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");


