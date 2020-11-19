#include <linux/init.h>
#include <linux/module.h>


#define BUF_SIZE        1024
#define MAJOR_NUM       200
#define MINOR_NUM       0
struct demo_device {
    char buffer[BUF_SIZE];
    struct cdev cdev;
};

static struct demo_device demo_dev;

static int demo_open(inode *inode , struct file *file)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
}
static int (struct inode *inode , struct file *file)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
}
static ssize_t demo_read(struct file *file , char __user *user, size_t size, loff_t *pos)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
}
static ssize_t demo_write(struct file *file, const char __user *user , size_t size , loff_t *pos)
{
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
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
    //init for demo char 
    cdev_init(&demo_dev.cdev,&demo_operation);
    //register device number
    ret = register_chrdv_region(dev_no,1,"demomem")  
        if (ret < 0){
            printk(KERN_ERR "failed t oregister device number \r\n");
            return ret;
        }

    ret = cdev_add(&demo_dev.cdev,dev_no,0);
    if (ret < 0 ){
        printk(KERN_ERR "cdev add failed\n");
        unregister_chrdev_region(dev_no,1);
        return ret;
    }
    printk(KERN_INFO "Enter %s\n",__func__);
    return 0;
}

static void __exit demo_exit(void)
{
    cdev_del(&demo_dev.cdev);
    unregister_chrdev_region(MKDEV(MAJOR_NUM,MINOR_NUM);,1);
    
    printk(KERN_INFO "Enter %s\n",__func__);
}

module_init(module_init);
module_exit(module_exit);

MODULE_AUTHOR("zhangzhen");
MODLUE_LICENSE("GPL");