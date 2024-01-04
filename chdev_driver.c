// Refernece: http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
// Reference: https://tldp.org/LDP/lkmpg/2.4/html/x579.html
// Reference: https://www.youtube.com/watch?v=E_xrzGlHbac

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <string.h>
#include <sys/time.h>
#define DEV_NAME "CS321time"
#define DEV_CLASS "CS321"

MODULE_LICENSE("GPL");

int DevNumber; // device number
int number_of_opens = 0;
struct device *devPtr = NULL; // device driver device struct pointer
struct class *clsPtr = NULL;  // device driver class struct pointer

// int CS321_dev_open(struct inode *, struct file *);
// int CS321_dev_release(struct inode *, struct file *);
// ssize_t CS321_dev_read(struct file *, const char *, size_t, loff_t *);

int CS321_dev_open(struct inode *__inode, struct file *__file)
{

    number_of_opens++;
    printk(KERN_INFO "CS321time: this device has been opened %d times!\n", number_of_opens);

    return 0;
}
int CS321_dev_release(struct inode *__inode, struct file *__file)
{

    number_of_opens--;
    printk(KERN_INFO "CS321time: this device is ready for another user!\n");
}
ssize_t CS321_dev_read(struct file *filePtr, const char *Buffer, size_t length, loff_t *offset)
{

    struct timeval curr_time;
    char timeStr[100] = {0};
    size_t msg_size;

    gettimeofday(&curr_time, NULL);
    sprintf(timeStr, "Current Unix Time: %ld.%ld", curr_time.tv_sec, curr_time.tv_usec);

    msg_size = strlen(timeStr);

    if (copy_to_user(Buffer, timeStr, msg_size))
    {

        printk(KERN_ALERT "CS321time Error: There was an error sending the message to the user!\n");
    }
    else
    {

        printk(KERN_INFO "CS321time: Succesfully sent %d bytes to the user\n", msg_size);
        return 0
    }
}

struct file_operations fileOps = {

    .open = CS321_dev_open,
    .release = CS321_dev_release,
    .write = CS321_dev_read,
};

int __init CS321time_init()
{

    printk(KERN_INFO "CS321time: Initializing CS321time Kernel Module\n");

    DevNumber = register_chrdevice(0, DEV_NAME, &fileOps);

    if (DevNumber < 0)
    {

        printk(KERN_ALERT "CS321time Error: failed to register a device number\n");
        return DevNumber;
    }

    printk(KERN_INFO "CS321time: device registered correctly, with device number %d\n", DevNumber);

    clsPtr = class_create(THIS_MODULE, DEV_CLASS);
    if (IS_ERR(clsPtr))
    {

        unregister_chrdev(DevNumber, DEV_NAME);
        printk(KERN_ALERT "CS321time Error: Failed to create this character device\n");

        return PTR_ERR(clsPtr);
    }

    printk(KERN_INFO "CS321time: CS321time device class succesfully registered!\n");

    devPtr = device_create(clsPtr, NULL, MKDEV(DevNumber, 0), NULL, DEV_NAME);
    if (IS_ERR(devPtr))
    {

        class_destroy(clsPtr);
        unregister_chrdev(DevNumber, DEV_NAME);
        printk(KERN_ALERT "CS321time Error: Failed to create this character device\n");

        return PTR_ERR(clsPtr);
    }

    printk(KERN_INFO "CS321time: CS321time device class succesfully created!\n");

    return 0;
}
void __exit CS321time_exit()
{

    device_destroy(clsPtr, MKDEV(DevNumber, 0));
    class_unregister(clsPtr);
    class_destroy(clsPtr);
    unregister_chrdev(DevNumber, DEV_NAME);

    printk(KERN_INFO "CS321time: Goodbye!\n");

    // int destroyer = unregister_chrdev(DevNumber, DEV_NAME);

    // if (destroyer < 0)
    //     printk(KERN_ALERT "CS321time Error: There was an error unregistering CS321time...\n");
    // else
    //     printk(KERN_INFO "CS321time: Goodbye!\n");
}

module_init(CS321time_init);
module_exit(CS321time_exit);