#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>

//void my_func(struct work_struct *ws);
//DECLARE_WORK(my_work,my_func);
struct work_struct my_work;
struct workqueue_struct *my_wq;

void my_func(struct work_struct *ws)
{
    printk("do work !\n");
}

static  int __init work_queue_init(void)
{
    //int ret;
    printk("module load begin\n");
    //create my workqueue
    my_wq = create_workqueue("my_wq");
    if(!my_wq){

        printk("Failed to create my work !\n");
        return -1;
    }
    INIT_WORK(&my_work,my_func);
    //schedule my_work to my_wq
    queue_work(my_wq,&my_work);
    //schedule_work(&my_work);

    return 0;
}

static  void __exit work_queue_exit(void)
{
    //destory my_wq
    destroy_workqueue(my_wq);
    my_wq= NULL;
}

module_init(work_queue_init);
module_exit(work_queue_exit);


MODULE_LICENSE("GPL");

