#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <asm/uaccess.h>

struct second_device{
    struct miscdevice mdev;
    struct timer_list timer;
    //int seconds;
    wait_queue_head_t wq; 
    atomic_t seconds;
    bool avaliable;
};

struct second_device *second;

static void second_hander(struct timer_list *arg)
{
    int counter;
    //second->seconds ++;
    counter = atomic_read(&second->seconds);
    printk("%s: counter = %d\n",__func__,counter);
    atomic_inc(&second->seconds);
    mod_timer(&second->timer,jiffies + HZ);
    //wake up read process and set avaliable to true
    second->avaliable = true;
    wake_up(&second->wq);
}

static int second_open(struct inode *inode, struct file *file)
{
    timer_setup(&second->timer,second_hander,(unsigned long )0);
    second->timer.expires = jiffies + HZ;
    //second->seconds = 0;
    atomic_set(&second->seconds,0);
    add_timer(&second->timer);
    printk("open device \n");

    file->private_data = (void *)second;
    return 0;
}

static int second_release(struct inode *inode , struct file *file)
{
    struct second_device *sec = (struct second_device *)file->private_data;
    del_timer(&sec->timer);
    printk("release device \n");
    return 0;
}
static ssize_t second_read(struct file *file, char __user *buf, size_t size, loff_t *ops)
{
    int counter;
    struct second_device *sec = (struct second_device *)file->private_data;

    //boloc process until seconds updata
    wait_event_interruptible(sec->wq,sec->avaliable );
//    printk("read_device \n");
    counter = atomic_read(&sec->seconds);

    sec->avaliable = false;
    if(put_user(counter,(int *)buf)){
        return -EFAULT;
    }else{
        return sizeof(int);
    } 
    return size;
}

struct file_operations second_fops = {
    .open = second_open,
    .release = second_release,
    .read = second_read,
};

struct miscdevice misc_struct = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "second",
	.fops= &second_fops,
};

static __init int second_init(void)
{
    int ret;
    //mallic for second device 
    second = (struct second_device *)kzalloc(sizeof(struct second_device),GFP_KERNEL);
    if(!second ){
        printk("failed to malloc for device\n");
        ret = -ENOMEM;
        goto ERR_NO_MEM;
    }

    init_waitqueue_head(&second->wq);
    second->avaliable = false;
    //init for second parameter
    //register misc device
    second->mdev = misc_struct;
    misc_register(&second->mdev);
    if(ret){
        printk("failed to register misc device\n");
        goto ERR_REG_MISC;
    }

    printk("init second device done\n");

    return 0;

ERR_REG_MISC:
    kfree(second);
    second = NULL;
ERR_NO_MEM:
    return ret;
}

static __exit void second_exit(void)
{
    if(second){
        misc_deregister(&second->mdev);
        kfree(second);
        second = NULL;
    }

    printk("exit second device done\n");

}

module_init(second_init);
module_exit(second_exit);
MODULE_LICENSE("GPL");
