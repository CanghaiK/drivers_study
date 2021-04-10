#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h> 
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/poll.h>

#include "demomem.h"


#define BUF_SIZE       20 
#define MAJOR_NUM       200
#define MINOR_NUM       0

struct demo_device {
//    char buffer[BUF_SIZE];
    char *buffer;
    int value;
    struct miscdevice *mdev;
    int r_offset;
    int w_offset;
    int size;
    wait_queue_head_t inq;
    wait_queue_head_t outq;
    struct mutex lock;
};

//static struct demo_device demo_dev;
struct demo_device *demo_dev;

static int demo_open(struct inode *node , struct file *file)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    file->private_data = (void *)demo_dev;
    return 0;
}
static int demo_release(struct inode *node , struct file *file)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
}
static ssize_t demo_read(struct file *file , char __user *buf, size_t size, loff_t *pos)
{
    struct demo_device *demo = file->private_data;
    char *kbuf = demo->buffer;

    if(mutex_lock_interruptible(&demo_dev->lock)){
        return -ERESTART;
    }
    while(demo->r_offset == demo->w_offset){
        //there is no buffer to read
        //let's sleep
        printk("there is no buffer t oread\n");
        mutex_unlock(&demo->lock);
        if(file->f_flags & O_NONBLOCK){
            return -EAGAIN;
        }
        //here block process
        if(wait_event_interruptible(demo->inq,(demo->r_offset != demo->w_offset))){
            return -EAGAIN;
        }
        if(mutex_lock_interruptible(&demo->lock))
            return -ERESTART;
    }
    //there is something to read
    //get how many to read
    if(demo->w_offset > demo->r_offset){
        size = min(size,(size_t)(demo->w_offset - demo->r_offset));
        if(copy_to_user(buf,kbuf + demo->r_offset,size)){
            mutex_unlock(&demo->lock);
            return -EFAULT;
        }
        demo->r_offset += size;
        if(demo->r_offset == demo->size)
            demo->r_offset = 0;
    }else{
        size = min(size,(size_t)(demo->size - demo->r_offset +demo->w_offset));
        //read strp 1
        //size = min(size,(demo->size - demo->r_offset));
        if(copy_to_user(buf,kbuf + demo->r_offset,(demo->size - demo->r_offset))){
            mutex_unlock(&demo->lock);
            return -EFAULT;
        }
        //demo->r_offset += demo->size - demo->r_offset;
        //if(demo->r_offset == demo->size)
        //   demo->r_offset = 0;
        //read step 2
        //size = min(size,demo->w_offset);
        if(copy_to_user(buf + (demo->size - demo->r_offset),
                    kbuf,size - (demo->size - demo->r_offset))){
            mutex_unlock(&demo->lock);
            return -EFAULT;
        }
        demo->r_offset = size - (demo->size - demo->r_offset);
        if(demo->r_offset == demo->size)
            demo->r_offset = 0;
    }
    wake_up_interruptible(&demo->outq);

    mutex_unlock(&demo->lock);

    return size;
}

int get_write_left(struct demo_device *demo)
{
    if(demo->r_offset > demo->w_offset){
        return demo->r_offset - demo->w_offset;
    }else{
        return demo->size - (demo->w_offset - demo->r_offset);
    }
}

static ssize_t demo_write(struct file *file, const char __user *buf , size_t size , loff_t *pos)
{
    struct demo_device *demo = file->private_data;
    char *kbuf = demo->buffer;

    if(mutex_lock_interruptible(&demo->lock)){
        return -ERESTART;
    }
     
    while(get_write_left(demo) < size){
        //sleep
        printk("in the while\n");

        mutex_unlock(&demo->lock);
        if(file->f_flags & O_NONBLOCK){
            return -EAGAIN;
        }
        //let's sleep
        if(wait_event_interruptible(demo->outq,get_write_left(demo) >= size)){
            return -ERESTART;
        }
        if(mutex_lock_interruptible(&demo->lock)){
            return -ERESTART;
        }
    }

    //write something
    if(demo->r_offset > demo->w_offset ){
       size = min(size,(size_t)(demo->r_offset - demo->w_offset));
        //write into kernel space 
        if(copy_from_user((kbuf + demo->w_offset),buf,size)){
            mutex_unlock(&demo->lock);
            return -EFAULT;
        }
        demo->w_offset += size;
        if(demo->w_offset == demo->size){
        demo->w_offset = 0;
        }
    }else{
       size = min(size,(size_t)(demo->size- demo->w_offset + demo->r_offset));
        //write into kernel space 
       if(size > (demo->size - demo->w_offset)){
            if(copy_from_user((kbuf + demo->w_offset),buf,demo->size - demo->w_offset)){
                mutex_unlock(&demo->lock);
                return -EFAULT;
            }
            if(copy_from_user((kbuf),buf+demo->size - demo->w_offset,size - (demo->size - demo->w_offset ))){
                mutex_unlock(&demo->lock);
                return -EFAULT;
            }
            demo->w_offset = size - (demo->size - demo->w_offset);
            if(demo->w_offset == demo->size){
            demo->w_offset = 0;
            }
        }else{
            if(copy_from_user((kbuf + demo->w_offset),buf,size)){
                mutex_unlock(&demo->lock);
                return -EFAULT;
            }
            demo->w_offset += size;
            if(demo->w_offset == demo->size){
            demo->w_offset = 0;
            }
        }
        //size = min(size,(size_t)(demo->size - demo->w_offset + demo->r_offset));
        //if(size < (demo->size - demo->w_offset)){
        //    //write into kernel space 
        //    if(copy_from_user((kbuf + demo->w_offset),buf,size)){
        //        mutex_unlock(&demo->lock);
        //        return -EFAULT;
        //    }
        //     demo->w_offset += size;
        //    if(demo->w_offset == demo->size){
        //     demo->w_offset = 0;
        //    }
        //}else{
        //    //size =(demo->size - demo->w_offset)+ size - (demo->size - demo->w_offset) ;
        //    //write into kernel space 
        //    if(copy_from_user((kbuf + demo->w_offset),buf,(demo->size - demo->w_offset))){
        //        mutex_unlock(&demo->lock);
        //        return -EFAULT;
        //    }
        //    demo->w_offset =  0;
        //    if(copy_from_user((kbuf + demo->w_offset),buf,size - (demo->size - demo->w_offset)){
        //        mutex_unlock(&demo->lock);
        //        return -EFAULT;
        //    }
        //    demo->w_offset += r_offset;
        //}
    }

    printk("wake up read\n");
    wake_up_interruptible(&demo->inq);
    mutex_unlock(&demo->lock);
    return size;
}

static unsigned int demo_poll(struct file *file,poll_table *wait)
{
    unsigned int mask = 0;
    struct demo_device *demo = (struct demo_device *)file->private_data;

    poll_wait(file,&demo->inq,wait);
    poll_wait(file,&demo->outq,wait);

    if(demo->r_offset != demo->w_offset){
        mask |= POLLIN | POLLRDNORM;
    }else{
        mask |= POLLOUT | POLLWRNORM;
    }

    return mask;
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
    .poll = demo_poll,
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
    demo_dev->r_offset = 0;
    demo_dev->w_offset = 0;
    demo_dev->size = BUF_SIZE;
    mutex_init(&demo_dev->lock);
    init_waitqueue_head(&demo_dev->inq);
    init_waitqueue_head(&demo_dev->outq);
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

