#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");

static dev_t devno;

static int __init hello_init(void)
{
    int rslt;

    printk(KERN_ALERT "scull0 init\n");

    rslt = alloc_chrdev_region(&devno, 0, 1, "scull0");
    if (0 == rslt) {
        printk(KERN_ALERT "scull0 dev_t: %d, %d\n",
               MAJOR(devno),
               MINOR(devno));
    }

    return rslt;
}
static void __exit hello_exit(void)
{
    unregister_chrdev_region(devno, 1);
    printk(KERN_ALERT "scull0 exit\n");

    return;
}

module_init(hello_init);
module_exit(hello_exit);
