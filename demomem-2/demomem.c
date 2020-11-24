#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h> 


#define BUF_SIZE        1024
#define MAJOR_NUM       200
#define MINOR_NUM       0

struct demo_device {
//    char buffer[BUF_SIZE];
    char *buffer;
    struct cdev cdev;
    struct class *cls;
    struct device *device;
};

static struct demo_device demo_dev;

static int demo_open(struct inode *node , struct file *file)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
}
static int demo_release(struct inode *node , struct file *file)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
}
static ssize_t demo_read(struct file *file , char __user *buf, size_t size, loff_t *pos)
{
    int ret;
    int read_bytes;
    char *kbuf = demo_dev.buffer + *pos;

    if(*pos >= BUF_SIZE)
        return -1;

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
    char *kbuf = demo_dev.buffer + *pos;

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

static struct file_operations demo_operation = {   
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write,
};

static int __init demo_init(void)
{
    int ret =1 ;
    dev_t dev_no;
    dev_no = MKDEV(MAJOR_NUM,MINOR_NUM);

    demo_dev.buffer = (char *)kmalloc(BUF_SIZE,GFP_KERNEL);
    if(!demo_dev.buffer){
        printk(KERN_ERR "malloc %d bytes failed \n",BUF_SIZE);
        ret = -ENOMEM;
        goto ERROR_MALLOC;
    }
    //init for demo char 
    cdev_init(&demo_dev.cdev,&demo_operation);
    //register device number
    ret = register_chrdev_region(dev_no,1,"demomem"); 
    if (ret < 0){
        ret = alloc_chrdev_region(&dev_no,0,1,"demomem");
        if(ret < 0){
            printk(KERN_ERR "failed t oregister device number \r\n");
            goto ERROR_CHARDEV_REGION;
        }

    }
    //add char_dev to the operating system
    ret = cdev_add(&demo_dev.cdev,dev_no,1);
    if (ret < 0 ){
        printk(KERN_ERR "cdev add failed\n");
        goto ERROR_CDEV_ADD;
    }

    //create a demomem class in /sys/class/demo/
//    demo_dev.cls = class_create(THIS_MODULE,"demo");
//    if(IS_ERR(demo_dev.cls)){
//        ret = PTR_ERR(demo_dev.cls);
//        goto ERROR_CLASS_CREATE;
//    }
//
//    //creat a demomem device in /sys/class/demo/demomem
//    demo_dev.device = device_create(demo_dev.cls,NULL,
//            dev_no,NULL,"demomem");
//    if(IS_ERR(demo_dev.device)){
//        ret = PTR_ERR(demo_dev.device);
//        goto ERROR_DEVICE_CREATE;
//    }
    
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;

ERROR_DEVICE_CREATE:
    class_destroy(demo_dev.cls);
ERROR_CLASS_CREATE:
    cdev_del(&demo_dev.cdev);   
ERROR_CDEV_ADD:
    unregister_chrdev_region(dev_no,1);
    return ret;
ERROR_CHARDEV_REGION:
    kfree(demo_dev.buffer);
    demo_dev.buffer = NULL;
ERROR_MALLOC:
    return ret;
}

static void __exit demo_exit(void)
{
    cdev_del(&demo_dev.cdev);
    unregister_chrdev_region(MKDEV(MAJOR_NUM,MINOR_NUM),1);

    kfree(demo_dev.buffer);
    demo_dev.buffer = NULL;
    
    printk(KERN_INFO "Enter %s\n",__func__);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_AUTHOR("zhangzhen");
MODULE_LICENSE("GPL");


