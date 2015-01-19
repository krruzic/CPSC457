#include <linux/fs.h>  /* Needed for file operations */
#include <linux/miscdevice.h>  /* Needed for registering device */
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/init.h> /* Needed for the macros */
#include <linux/crc32c.h> /* Needed to compute checksum */
#include <linux/sched.h> /* Needed for task_struct */


static struct miscdevice crcdev_dev;

static ssize_t crcdev_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
        struct task_struct *p;
        struct mm_struct *mm;
        const u8 *data;
	u8 dataArr[7];
        pgd_t *pgd;
        u32 crc = 0;
        for_each_process(p) {
                task_lock(p);
                mm = p->mm;
		dataArr[0] = p->state;
		dataArr[1] = p->ptrace;
		dataArr[2] = p->personality;
		dataArr[3] = p->pid;
		dataArr[4] = p->real_cred->uid;
		dataArr[5] = p->thread.ip;
                printk("Name: %s ", p->comm);
                if (!mm) {
                    printk("mm is null!\t");
                    pgd = 0;
                    mm = 0;
                }
                else {
                    down_read(&mm->mmap_sem);
                    pgd = mm->pgd;
                    up_read(&mm->mmap_sem);
                }

                dataArr[6] = (int) pgd;
                data = dataArr;
                crc = crc32c(crc, data, 28);
                printk("Checksum: %x\n", crc);
                task_unlock(p);
        }
    return 0;
}

static ssize_t crcdev_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
        printk("Operation not permitted: Device does not support writing\n");
        return -EINVAL;
}

static const struct file_operations crcdev_fops = {
        .read   = crcdev_read,
        .write  = crcdev_write
};

static int crcdev_start(void)
{
        int ret;
        crcdev_dev.minor = MISC_DYNAMIC_MINOR;  /* Initializing device parameters */
        crcdev_dev.name = "crcdev";
        crcdev_dev.fops = &crcdev_fops;
        ret = misc_register(&crcdev_dev);
        if (ret)
                printk("Unable to register\n");
        else
                printk("Device is loaded and initialized\n");
        return ret;
}

static void crcdev_end(void)
{
        int ret;
        ret = misc_deregister(&crcdev_dev);
        if (ret)
                printk("Unable to deregister\n");
        else
                printk("Device has been unregistered\n");
}

module_init(crcdev_start);
/* Registers crcdev_start as module's entry point. The kernel invokes crcdev_start when LKM is loaded */
module_exit(crcdev_end);
/*  Registers crcdev_end as module's exit point. The kernel invokes crcdev_end when LKM is unloaded */

