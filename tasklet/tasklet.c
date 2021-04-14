#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

struct message {
    int id;
    int value;
};

struct tasklet_struct mytask;

void mytask_func (unsigned long data)
{
    struct message *msg = (struct message *)data;

    printk("id = %d, val = %d\n",msg->id,msg->value);
    kfree(msg);


}

static __init int task_init(void)
{   
    struct message *msg;
    //init for msg
    msg = (struct message *)kzalloc(sizeof(struct message),GFP_KERNEL);
    if(!msg){
        printk("failed to malloc for msg\n");
        return -ENOMEM;
    }
    msg->id = 10;
    msg->value = 100;
    tasklet_init(&mytask, mytask_func, (unsigned long)msg);
    tasklet_schedule(&mytask);
    printk("%s: start\n",__func__);
    printk("%s: end\n",__func__);
    return 0;
}

static __exit void task_exit(void)
{
    printk("%s: start\n",__func__);
    tasklet_kill(&mytask);
    printk("%s: end\n",__func__);
}


module_init(task_init);
module_exit(task_exit);


