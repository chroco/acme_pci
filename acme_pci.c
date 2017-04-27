/*
	Chad Coates
	ECE373
	Homework #3
	April 18, 2017

	This is the acme_pci driver
*/

//#include <linux/types.h>
//#include <linux/moduleparam.h>
//#include <linux/kernel.h>
//#include <linux/init.h>
//#include <linux/kdev_t.h>
//#include <linux/slab.h>
//#include <linux/device.h>	
//#include <linux/errno.h>	

#include <linux/pci.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>	

#define DEVCOUNT			1
#define DEVNAME				"acme"

ssize_t acme_read(struct file *,char __user *buff,size_t,loff_t *);
ssize_t acme_write(struct file *,const char __user *buff,size_t,loff_t *);

static dev_t acme_dev_number;
static struct class *acme_class;

struct pci_hw{
	void __iomem *hw_addr;
	void __iomem *led_ctl;
};

struct acme_dev{
	struct cdev cdev;				
	struct pci_hw hw;
	u32 led_ctl;
} *acme_devp;

static struct file_operations acme_fops = {
	.owner = THIS_MODULE,
	.read = acme_read,		
	.write = acme_write,
};

static int amce_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent){
	int bars, err;
	resource_size_t mmio_start, mmio_len;

	printk(KERN_INFO "It's dangerous to go alone, take this with you.\n");
	
	err=pci_enable_device_mem(pdev);
	if(err)return err;

	bars=pci_select_bars(pdev, IORESOURCE_MEM);
	
	err=pci_request_selected_regions(pdev,bars,"acme_pci");
	if(err)goto err_pci_reg;
	
	pci_set_master(pdev);
	
	mmio_start = pci_resource_start(pdev, 0);
	mmio_len = pci_resource_len(pdev, 0);
	acme_devp->hw.hw_addr = ioremap(mmio_start, mmio_len);
	acme_devp->hw.led_ctl=acme_devp->hw.hw_addr+0x00E00; 
err_pci_reg:
	pci_disable_device(pdev);
	return 0;
}

static void amce_pci_remove(struct pci_dev *pdev){
	pci_release_selected_regions(pdev,pci_select_bars(pdev,IORESOURCE_MEM));
	//pci_release_mem_regions(pdev);
	pci_disable_device(pdev);
	printk(KERN_INFO "So long!!\n");
}

static DEFINE_PCI_DEVICE_TABLE(amce_pci_tbl) = {
	{ PCI_DEVICE(0x8086, 0x150c) },
	{ }, 
};

static struct pci_driver acme_pci_driver = {
	.name = "acme_pci",
	.id_table = amce_pci_tbl,
	.probe = amce_pci_probe,
	.remove = amce_pci_remove,
};

static int __init amce_pci_init(void){
	acme_class = class_create(THIS_MODULE,DEVNAME);
	
	if(alloc_chrdev_region(&acme_dev_number,0,DEVCOUNT,DEVNAME) < 0) {
		printk(KERN_DEBUG "Can't register device\n"); 
		return -ENODEV;
	}
	
	acme_devp = kmalloc(sizeof(struct acme_dev), GFP_KERNEL);
	if(!acme_devp){
		printk("Bad Kmalloc\n"); 
		return -ENOMEM;
	}
	
	cdev_init(&acme_devp->cdev,&acme_fops);
	
	if(cdev_add(&acme_devp->cdev,acme_dev_number,DEVCOUNT)){
		printk(KERN_NOTICE "cdev_add() failed");
		unregister_chrdev_region(acme_dev_number,DEVCOUNT);
		return -1;
	}

	device_create(acme_class,NULL,acme_dev_number,NULL,DEVNAME);
	
	printk("ACME Driver Initialized.\n");

	return pci_register_driver(&acme_pci_driver);
}

static void __exit amce_pci_exit(void){
	cdev_del(&acme_devp->cdev);
	unregister_chrdev_region(acme_dev_number,DEVCOUNT);
	kfree(acme_devp);
	device_destroy(acme_class,acme_dev_number);
	class_destroy(acme_class);
	pci_unregister_driver(&acme_pci_driver);
	printk("ACME Driver Uninstalled.\n");
}

ssize_t acme_read(struct file *filp,char __user *buf,size_t len,loff_t *offset){
	int ret;
	if(*offset >= sizeof(int))return 0;
	if(!buf){
		ret = -EINVAL;
		goto out;
	}
		
	acme_devp->led_ctl = readl(&acme_devp->hw.led_ctl);
	if(copy_to_user(buf,&acme_devp->led_ctl,sizeof(u32))){
		ret = -EFAULT;
		goto out;
	}
	ret = sizeof(int);
	*offset += len;
out:
	return ret;
}

ssize_t acme_write(struct file *filp,const char __user *buf,size_t len,loff_t *offset){
	int ret;
	if(!buf){
		ret = -EINVAL;
		goto out;
	}
	
// 	u32 led_ctl = readl(&acme_devp->hw.led_ctl);
//	if(copy_to_user(buf,(u32 *)led_ctl,sizef(u32))){
	
	acme_devp->led_ctl = readl(&acme_devp->hw.led_ctl);
	if(copy_from_user(&acme_devp->led_ctl,buf,len)){
		ret = -EFAULT;
		goto out;
	}
	ret = len;
out:
	return ret;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chad Coates");
MODULE_VERSION("0.1");

module_init(amce_pci_init);
module_exit(amce_pci_exit);
