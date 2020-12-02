#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h> 
#include <linux/miscdevice.h>
#include <linux/spinlock.h>

#include "demomem.h"


#define BUF_SIZE        1024
#define MAJOR_NUM       200
#define MINOR_NUM       0

struct demo_device {
//    char buffer[BUF_SIZE];
    char *buffer;
    int value;
    struct miscdevice *mdev;
    spinlock_t lock;
    int open_count;
};

//static struct demo_device demo_dev;
struct demo_device *demo_dev;

static int demo_open(struct inode *node , struct file *file)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    spin_lock(&demo_dev->lock);
    if(demo_dev->open_count){
        printk(KERN_ERR "Already open!\n");
        spin_unlock(&demo_dev->lock);
        return -EBUSY;
    }

    demo_dev->open_count ++;
    spin_unlock(&demo_dev->lock);
    printk(KERN_INFO "Open!\n");

    file->private_data = (void *)demo_dev;
    return 0;
}
static int demo_release(struct inode *node , struct file *file)
{
    struct demo_device *demo = (struct demo_device *)file->private_data;
    printk(KERN_INFO "Enter %s\n",__func__);
    spin_lock(&demo->lock);
    demo->open_count --;
    spin_unlock(&demo->lock);
    return 0;
}
static ssize_t demo_read(struct file *file , char __user *buf, size_t size, loff_t *pos)
{
    int ret;
    int read_bytes;
    struct demo_device *demo = file->private_data;
    char *kbuf = demo->buffer + *pos;

    if(*pos >= BUF_SIZE)
        return 0;

    if(size > (BUF_SIZE - *pos))
        read_bytes = BUF_SIZE - *pos;
    else
        read_bytes = size;

    ret = copy_to_user(buf,kbuf,read_bytes);
    if(ret != 0)
        return -EFAULT;
    *pos += read_bytes;

    printk(KERN_INFO "Enter %s\n",__func__);

    return read_bytes;
}
static ssize_t demo_write(struct file *file, const char __user *buf , size_t size , loff_t *pos)
{
    int ret;
    int write_bytes;
    struct demo_device *demo = file->private_data;
    char *kbuf = demo->buffer + *pos;

    if(*pos >= BUF_SIZE)
        return -1;

    if(size > (BUF_SIZE - *pos))
        write_bytes = BUF_SIZE - *pos;
    else
        write_bytes = size;

    ret = copy_from_user(kbuf, buf, write_bytes);
    if(ret != 0)
        return -EFAULT;
    *pos += write_bytes;

    printk(KERN_INFO "Enter %s\n",__func__);

    return write_bytes;
}

static long demo_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    struct demo_device *demo = file->private_data;

    switch(cmd){
        case DEMO_MEM_CLEAN:
            printk(KERN_INFO "cmd :clean\n");
            memset(demo->buffer,0x00,BUF_SIZE);
            break;
        case DEMO_MEM_GETVAL:
            printk(KERN_INFO "cmd :getval\n");
            put_user(demo->value,(int *)arg);
            break;
        case DEMO_MEM_SETVAL:
            printk(KERN_INFO "cmd :setval\n");
            demo->value = (int)arg;
            break;
        default:
            break;
    }

    return (long)ret;
}
static struct file_operations demo_operation = {   
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write,
    .unlocked_ioctl = demo_ioctl,
};


static struct miscdevice misc_struct = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "demomem",
    .fops = &demo_operation,
};

static int __init demo_init(void)
{
    int ret =1 ;

    demo_dev = (struct demo_device *)kmalloc(sizeof(struct demo_device),GFP_KERNEL);
    if(!demo_dev){
        printk(KERN_ERR "failed to malloc demo_device");
        ret = -ENOMEM;
        goto ERROR_MALLOC_DEVICE;
    }

    demo_dev->buffer = (char *)kmalloc(BUF_SIZE,GFP_KERNEL);
    if(!demo_dev->buffer){
        printk(KERN_ERR "malloc %d bytes failed \n",BUF_SIZE);
        ret = -ENOMEM;
        goto ERROR_MALLOC_BUFFER;
    }

    demo_dev->value = 1;

    spin_lock_init(&demo_dev->lock);
    demo_dev->open_count = 0;

    demo_dev->mdev = &misc_struct;
   ret = misc_register(demo_dev->mdev); 
   if(ret < 0){
       printk(KERN_ERR "failed to register misc\n");
       goto ERROR_MISC;
   }
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;

ERROR_MISC:
    kfree(demo_dev->buffer);
    demo_dev->buffer = NULL;
ERROR_MALLOC_BUFFER:
    kfree(demo_dev);
    demo_dev = NULL;
ERROR_MALLOC_DEVICE:
    return ret;
}

static void __exit demo_exit(void)
{
    misc_deregister(demo_dev->mdev);

    kfree(demo_dev->buffer);
    demo_dev->buffer = NULL;
    kfree(demo_dev);
    demo_dev = NULL;
    
    printk(KERN_INFO "Enter %s\n",__func__);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_AUTHOR("zhangzhen");
MODULE_LICENSE("GPL");

