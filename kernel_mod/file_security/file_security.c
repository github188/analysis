#include <linux/security.h>
#include <linux/sysctl.h>
#include <linux/ptrace.h>
#include <linux/prctl.h>
#include <linux/ratelimit.h>
#include <linux/workqueue.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/path.h>

int test_file_permission(struct file *file, int mask)
{
    char const *name = file->f_path.dentry->d_name.name;
    if(!strcmp(name, "test.txt"))
    {
        file->f_flags |= O_RDONLY;
        printk("you can have your control code here!\n");
    }
    return 0;
}

static struct security_operations test_security_ops = {
    .name = "test",
    .file_permission = test_file_permission,
};

static __init int test_init(void)
{
        printk("enter test init!\n");

        printk(KERN_INFO "Test: becoming......\n");

        if (register_security(&test_security_ops))
                panic("Test: kernel registration failed.\n");


        return 0;
}

security_initcall(test_init);
