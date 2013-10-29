/************************************************************************
 *                                                                      *
 * Driver for BCM2835 to interface Philips LCD controller PCF8833.      *
 *                                                                      *
 * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
 *                                                                      *
 * This driver implements bitbanging SPI by using BCM2835 GPIO.         *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>    /* Macros MAJOR, MINOR                       */
#include <linux/fs.h>        /* alloc_chrdev_region() and file operations */
#include <linux/cdev.h>      /* cdev-functions                            */
#include <linux/ioport.h>    /* request_mem_region and friends            */
#include <linux/slab.h>      /* kmalloc, kfree                            */
#include <linux/semaphore.h> /* Semaphores                                */
#include <linux/mutex.h>     /* Mutexes                                   */
#include <linux/delay.h>     /* udelay                                    */
#include <asm/io.h>          /* ioremap and friends                       */
#include <asm/uaccess.h>     /* get_user, copy_from_user and friends      */

#include "bcm2835.h"

MODULE_DESCRIPTION("BCM2835 SPI bitbang driver for Philips PCF8833");
MODULE_AUTHOR("Bonden i Nol <hakanbrolin@hotmail.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("R1A02");

/*
 * Runtime debug messages on/off
 */
#define DEBUG_PRINTS
#undef DEBUG_PRINTS
#ifdef DEBUG_PRINTS
#define debug_print(fmt, args...) spi_pcf8833_debug(fmt, ##args);
#else
#define debug_print(fmt, args...)
#endif

/*
 * Module definitions
 */
#define DRV_NAME    "spi-pcf8833"
#define NR_DEVICES  1

/*
 * SPI signals.
 * This driver doesn't support read data from PCF8833,
 * hence signal MISO is not defined.
 */
struct gpio_signal {
  u8   pin;      /* BCM2835 data sheet GPIO names          */ 
  u8   func;     /* Pin function                           */
  bool defined;  /* Set to 'true' when new function is set */
  u8   init_val; /* Initial value for pin                  */
  u8   old_func; /* Old pin function                       */
};

struct spi_gpio {
  struct gpio_signal ce;
  struct gpio_signal clk;
  struct gpio_signal mosi;
};

/*
 * The device type for this module
 */
struct spi_pcf8833_dev {
  struct cdev      cdev;        /* Char device structure          */
  int              minor;       /* Minor device number            */      
  bool             registrated; /* Set to 'true' when registrated */
  struct semaphore dev_sem;     /* Device protecting semaphore    */
  struct mutex     xfer_lock;   /* Protects a SPI transfer        */
  struct spi_gpio  spi_gpio;    /* SPI GPIO signals for device    */
};

/*
 * Module global variables
 */
static int g_major = 0;
static int g_minor = 0;

static struct spi_pcf8833_dev *g_spi_pcf8833_dev = NULL;

static void __iomem *g_vaddr_gpio = NULL;

static const struct spi_gpio g_spi_gpio_init[NR_DEVICES] = {
  {
    .ce   = {  8, 0b001, false, 1, 0 },
    .clk  = { 11, 0b001, false, 1, 0 },
    .mosi = { 10, 0b001, false, 1, 0 }
  }
};

/*
 * Function prototypes
 */
static int spi_pcf8833_open(struct inode *node,
			    struct file *filp);

static ssize_t spi_pcf8833_write(struct file *filp,
				 const char __user *buf,
				 size_t count,
				 loff_t *f_pos);

static int spi_pcf8833_release(struct inode *node,
			       struct file *filp);

static int __init spi_pcf8833_init(void);

static void __exit spi_pcf8833_exit(void);

module_init(spi_pcf8833_init);
module_exit(spi_pcf8833_exit);

/*
 * File operations implemented by this driver
 */
static struct file_operations g_spi_pcf8833_fops = {
  .owner   = THIS_MODULE,
  .open    = spi_pcf8833_open,
  .write   = spi_pcf8833_write,
  .release = spi_pcf8833_release
};

/****************************************************************************
*
* Name spi_pcf8833_debug
*
* Description Prints a debug message using 'printk' if debug mode is on.
*
* Parameters  *fmt  (IN)  Debug message as a text string
*             ...   (IN)  Format specifiers (if any)
*
* Error handling None
*
****************************************************************************/
#ifdef DEBUG_PRINTS
static void spi_pcf8833_debug(char *fmt,...)
{
  va_list args;
  char buf[256];
  
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  printk(KERN_NOTICE DRV_NAME": %s", buf);
}
#endif

/*****************************************************************/

static inline void bcm2835_gpio_set_hi(u8 pin)
{
  int offset = (pin > 31 ? GPSET1 : GPSET0);

  /* Writing 0 to a bit has no effect on corresponding pin */
  iowrite32(1 << (pin%32), g_vaddr_gpio + offset);
}

/*****************************************************************/

static inline void bcm2835_gpio_set_lo(u8 pin)
{
  int offset = (pin > 31 ? GPCLR1 : GPCLR0);

  /* Writing 0 to a bit has no effect on corresponding pin */
  iowrite32(1 << (pin%32), g_vaddr_gpio + offset);
}


/*****************************************************************/

static int bcm2835_gpio_gpfsel_offset(u8 pin)
{
  int offset;

  if (pin > 53) return -1; /* Only 54 GPIO pins */

  switch (pin / 10) {
  case 0:
    offset = GPFSEL0;  /* pin 0..9   */
    break;
  case 1:
    offset = GPFSEL1;  /* pin 10..19 */
    break;
  case 2:
    offset = GPFSEL2;  /* pin 20..29 */
    break;
  case 3:
    offset = GPFSEL3;  /* pin 30..39 */
    break;
  case 4:
    offset = GPFSEL4;  /* pin 40..49 */
    break;
  case 5:
    offset = GPFSEL5;  /* pin 50..53 */
  }

  return offset;
}

/*****************************************************************/

static int bcm2835_gpio_get_func(u8 pin, u8 *func)
{
  u32 gpfsel;
  int offset = bcm2835_gpio_gpfsel_offset(pin);

  if (offset < 0) {
    return -EINVAL;
  }

  /* Function is 3 bits for each of the 10 pins */
  gpfsel = ioread32(g_vaddr_gpio + offset);
  *func = ( gpfsel >> ((pin%10) * 3) ) & 0x7;

  return 0;
}

/*****************************************************************/

static int bcm2835_gpio_set_func(u8 pin, u8 func)
{
  u32 gpfsel;
  int offset = bcm2835_gpio_gpfsel_offset(pin);

  if (offset < 0) {
    return -EINVAL;
  }

  /* Function is 3 bits for each of the 10 pins */

  gpfsel = ioread32(g_vaddr_gpio + offset);

  /* Always set to INPUT first */
  gpfsel &= ~( 0x7 << ((pin%10) * 3) ); /* Clear func bits for pin */
  iowrite32(gpfsel, g_vaddr_gpio + offset);

  /* Set function */
  if (func) {
    gpfsel &= ~( 0x7 << ((pin%10) * 3) ); // Clear function bits for pin
    gpfsel |= ( func << ((pin%10) * 3) ); // Set function bits for pin
    iowrite32(gpfsel, g_vaddr_gpio + offset);
  }

  return 0;
}

/****************************************************************************
*
* Name bcm2835_gpio_init_pin_val
*
* Description Set the inital value of the GPIO pins.
*
* Parameters *dev  (IN/OUT)  Pointer to the device
*
* Error handling None
*
****************************************************************************/
static void bcm2835_gpio_init_pin_val(struct spi_pcf8833_dev *dev)
{
  /* Set initial value for each pin */
  if (dev->spi_gpio.ce.init_val)
    bcm2835_gpio_set_hi(dev->spi_gpio.ce.pin);
  else
    bcm2835_gpio_set_lo(dev->spi_gpio.ce.pin);
  
  if (dev->spi_gpio.clk.init_val)
    bcm2835_gpio_set_hi(dev->spi_gpio.clk.pin);
  else
    bcm2835_gpio_set_lo(dev->spi_gpio.clk.pin);

  if (dev->spi_gpio.mosi.init_val)
    bcm2835_gpio_set_hi(dev->spi_gpio.mosi.pin);
  else
    bcm2835_gpio_set_lo(dev->spi_gpio.mosi.pin);
}

/****************************************************************************
*
* Name bcm2835_gpio_init_pin_func
*
* Description Defines the default operation of the GPIO pins.
*
* Parameters *dev  (IN/OUT)  Pointer to the device
*
* Error handling Returns 0 on success, otherwise a kernel error code.
*
****************************************************************************/
static int bcm2835_gpio_init_pin_func(struct spi_pcf8833_dev *dev)
{
  int rc;

  /* Save original function for each pin */
  rc = bcm2835_gpio_get_func(dev->spi_gpio.ce.pin,
			     &dev->spi_gpio.ce.old_func);
  if (rc) return rc;

  debug_print("major(%d) minor(%d), CE pin(%d) old func(0x%x)\n",
	      g_major, dev->minor,
	      dev->spi_gpio.ce.pin, dev->spi_gpio.ce.old_func);

  rc = bcm2835_gpio_get_func(dev->spi_gpio.clk.pin,
			     &dev->spi_gpio.clk.old_func);
  if (rc) return rc;

  debug_print("major(%d) minor(%d), CLK pin(%d) old func(0x%x)\n",
	      g_major, dev->minor,
	      dev->spi_gpio.clk.pin, dev->spi_gpio.clk.old_func);

  rc = bcm2835_gpio_get_func(dev->spi_gpio.mosi.pin,
			     &dev->spi_gpio.mosi.old_func);
  if (rc) return rc;

  debug_print("major(%d) minor(%d), MOSI pin(%d) old func(0x%x)\n",
	      g_major, dev->minor,
	      dev->spi_gpio.mosi.pin, dev->spi_gpio.mosi.old_func);

  /* Set new function for each pin */
  rc = bcm2835_gpio_set_func(dev->spi_gpio.ce.pin,
			     dev->spi_gpio.ce.func);
  if (rc) return rc;
  dev->spi_gpio.ce.defined = true;

  rc = bcm2835_gpio_set_func(dev->spi_gpio.clk.pin,
			     dev->spi_gpio.clk.func);
  if (rc) return rc;
  dev->spi_gpio.clk.defined = true;

  rc = bcm2835_gpio_set_func(dev->spi_gpio.mosi.pin,
			     dev->spi_gpio.mosi.func);
  if (rc) return rc;
  dev->spi_gpio.mosi.defined = true;

  return 0;
}

/****************************************************************************
*
* Name bcm2835_gpio_restore_pins
*
* Description Restores default (old) operation of the GPIO pins.
*
* Parameters *dev  (IN/OUT)  Pointer to the device
*
* Error handling None
*
****************************************************************************/
static void bcm2835_gpio_restore_pins(struct spi_pcf8833_dev *dev)
{
  /* Set old function for each pin */

  if (dev->spi_gpio.ce.defined) {
    bcm2835_gpio_set_func(dev->spi_gpio.ce.pin,
			  dev->spi_gpio.ce.old_func);
  }
  if (dev->spi_gpio.clk.defined) {
    bcm2835_gpio_set_func(dev->spi_gpio.clk.pin,
			  dev->spi_gpio.clk.old_func);
  }
  if (dev->spi_gpio.mosi.defined) {
    bcm2835_gpio_set_func(dev->spi_gpio.mosi.pin,
			  dev->spi_gpio.mosi.old_func);
  }
}

/****************************************************************************
*
* Name bcm2835_setup_gpio_io_mem
*
* Description Prepares GPIO I/O memory to be accessed.
*
* Parameters None
*
* Error handling Returns 0 on success, otherwise a kernel error code.
*
****************************************************************************/
static int bcm2835_setup_gpio_io_mem(void)
{
  /* Allocate I/O memory region for GPIO */
  if ( !request_mem_region(GPIO_BASE_ADDR, GPIO_SIZE_BYTES, DRV_NAME) ) {
    printk(KERN_WARNING "*** "DRV_NAME": Failed to get I/O memory addr(0x%08x) size(%d)\n",
	   GPIO_BASE_ADDR, GPIO_SIZE_BYTES);
    return -ENOMEM;
  }
  
  /* Remap I/O memory for GPIO */
  g_vaddr_gpio = ioremap(GPIO_BASE_ADDR, GPIO_SIZE_BYTES);
  if ( !g_vaddr_gpio ) {
    printk(KERN_WARNING "*** "DRV_NAME": Failed ioremap I/O memory addr(0x%08x) size(%d)\n",
	   GPIO_BASE_ADDR, GPIO_SIZE_BYTES);
    return -ENOMEM;
  }

  debug_print("GPIO remapped, paddr(0x%08lx) vaddr(0x%08lx) size(0x%x)\n",
	      GPIO_BASE_ADDR,
	      (u32)g_vaddr_gpio,
	      GPIO_SIZE_BYTES);
  return 0;
}

/****************************************************************************
*
* Name spi_pcf8833_setup_cdev
*
* Description Setup the char device structure for specified device and
*             add the character device to the system.
*
* Parameters *dev  (IN/OUT)  Pointer to the device
*
* Error handling Returns 0 on success, otherwise a kernel error code.
*
****************************************************************************/
static int spi_pcf8833_setup_cdev(struct spi_pcf8833_dev *dev)
{
  int rc = 0;
  int devno = MKDEV(g_major, dev->minor);
  
  cdev_init(&dev->cdev, &g_spi_pcf8833_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops   = &g_spi_pcf8833_fops;

  /* Registrate device to system */
  rc = cdev_add(&dev->cdev, devno, 1);
  if (rc) {
    printk(KERN_WARNING "*** "DRV_NAME": Error(%d) registrate device, minor(%d)\n",
	   rc, dev->minor);
    return rc;
  }

  /* Mark device as registrated */
  dev->registrated = true;

  return 0;
}

/****************************************************************************
*
* Name spi_pcf8833_xfer
*
* Description Sends a single 9-bit message to PCF8833.
*             Assumes the SPI transfer has been locked and is available.
*
* Parameters *dev     (IN/OUT)  Pointer to the device
*            message  (IN)      Message (command/data) to send
*
* Error handling Returns number of bytes successfully written (non-negative),
*                otherwise a kernel error code.
*
****************************************************************************/
static ssize_t spi_pcf8833_xfer(struct spi_pcf8833_dev *dev,
				u16 message)
{
  int i;
  u16 bitmask = 0x0100;

   /* MOSI - Start low */
  bcm2835_gpio_set_lo(dev->spi_gpio.mosi.pin);

  /* Activate PCF8833 */
  bcm2835_gpio_set_lo(dev->spi_gpio.ce.pin);
  udelay(1);

  /* Transfer 9 bits, MSb first */
  for (i=0; i < 9; i++) {
    /* CLK - low */
    bcm2835_gpio_set_lo(dev->spi_gpio.clk.pin);
    udelay(1);

    /* MOSI */
    if ( bitmask & message ) {
      bcm2835_gpio_set_hi(dev->spi_gpio.mosi.pin);
    }
    else {
      bcm2835_gpio_set_lo(dev->spi_gpio.mosi.pin);
    }
    bitmask = bitmask >> 1;

    /* CLK - high */
    bcm2835_gpio_set_hi(dev->spi_gpio.clk.pin);
    udelay(1);
  }

  /* CLK - Final pulse low */
  bcm2835_gpio_set_lo(dev->spi_gpio.clk.pin);

  /* Deactivate PCF8833 */
  bcm2835_gpio_set_hi(dev->spi_gpio.ce.pin);

  /* CLK - Leave high */
  bcm2835_gpio_set_hi(dev->spi_gpio.clk.pin);

  /* MOSI - Leave high */
  bcm2835_gpio_set_hi(dev->spi_gpio.mosi.pin);

  return 2;
}

/****************************************************************************
*
* Name spi_pcf8833_open
*
* Description File operation that implements the open() system call.
*
* Parameters Linux standard for open() system call
*
* Error handling Returns 0 on success, otherwise a kernel error code.
*
****************************************************************************/
static int spi_pcf8833_open(struct inode *node,
			    struct file *filp)
{
  struct spi_pcf8833_dev *dev;
  
  debug_print("open, major(%d) minor(%d)\n", imajor(node), iminor(node));

  /* Identify which device is being opened */
  dev = container_of(node->i_cdev, struct spi_pcf8833_dev, cdev);
  if (dev == NULL) {
    return -ENODEV;
  }

  /* Store pointer to private data for easier access in future methods */
  filp->private_data = dev;

  /* Claim device protecting semaphore */
  if ( down_trylock(&dev->dev_sem) ) {
    return -EBUSY;
  }

  /* Set the inital value of the GPIO pins */
  bcm2835_gpio_init_pin_val(dev);

  return 0;
}

/****************************************************************************
*
* Name spi_pcf8833_write
*
* Description File operation that implements the write() system call.
*
* Parameters Linux standard for write() system call
*
* Error handling Returns number of bytes successfully written (non-negative),
*                otherwise a kernel error code.
*
****************************************************************************/
static ssize_t spi_pcf8833_write(struct file *filp,
				 const char __user *buf,
				 size_t count,
				 loff_t *f_pos)
{
  struct spi_pcf8833_dev *dev = filp->private_data;
  u16 message;
  ssize_t rc;

  debug_print("write, major(%d) minor(%d) bytes(%d)\n",
	      g_major, dev->minor, count);

  /* 
   * Check arguments
   * Driver only supports 9-bit, single transfer
   */
  if (count != 2) {
    return -EINVAL;
  }

  /* Copy from user space */
  if ( copy_from_user(&message, buf, count) ) {
    return -EFAULT;
  }

  /* Do the SPI transfer */
  if ( mutex_lock_interruptible(&dev->xfer_lock) ) {
    return -EINTR;
  }
  rc = spi_pcf8833_xfer(dev, message);
  mutex_unlock(&dev->xfer_lock);

  return rc;
}

/****************************************************************************
*
* Name spi_pcf8833_release
*
* Description File operation that implements the release() system call.
*
* Parameters Linux standard for release() system call
*
* Error handling Returns 0 on success, otherwise a kernel error code.
*
****************************************************************************/
static int spi_pcf8833_release(struct inode *node,
			       struct file *filp)
{
  struct spi_pcf8833_dev *dev = filp->private_data;
  
  debug_print("release, major(%d) minor(%d)\n", g_major, dev->minor);

  /* Device protecting semaphore is now available */
  up(&dev->dev_sem);

  return 0;
}

/****************************************************************************
*
* Name spi_pcf8833_init
*
* Description Kernel module 'init' function for this driver. 
*
* Parameters None
*
* Error handling Returns 0 on success, otherwise a kernel error code.
*
****************************************************************************/
static int __init spi_pcf8833_init(void)
{
  int i;
  int rc = 0;
  dev_t dev = 0;

  /* Get a major device number */
  rc = alloc_chrdev_region(&dev, g_minor, NR_DEVICES, DRV_NAME);
  if (rc < 0) {
    printk(KERN_WARNING "*** "DRV_NAME": Failed to get major device number\n");
    return rc;
  }
  g_major = MAJOR(dev);
  debug_print("Allocated major(%d) device number\n", g_major);

  /* Allocate memory for all devices */
  g_spi_pcf8833_dev = kmalloc(NR_DEVICES * sizeof(struct spi_pcf8833_dev), GFP_KERNEL);
  if ( !g_spi_pcf8833_dev ) {
    printk(KERN_WARNING "*** "DRV_NAME": Failed to allocate memory for devices\n");
    rc = -ENOMEM;
    goto spi_pcf8833_init_failed;
  }
  memset(g_spi_pcf8833_dev, 0, NR_DEVICES * sizeof(struct spi_pcf8833_dev));

  /* Setup I/O memory for GPIO */
  rc = bcm2835_setup_gpio_io_mem();
  if (rc) {
    goto spi_pcf8833_init_failed;
  }

  /* Common initialization for all devices */
  for (i=0; i < NR_DEVICES; i++) {
    g_spi_pcf8833_dev[i].minor = g_minor + i;
    g_spi_pcf8833_dev[i].registrated = false;     /* Not yet registrated */
    sema_init(&g_spi_pcf8833_dev[i].dev_sem, 1);  /* Device protecting
						     semaphore is available */
    mutex_init(&g_spi_pcf8833_dev[i].xfer_lock);  /* SPI transfer protecting
						     mutex is available */
    /* SPI GPIO signals for device */
    memcpy(&g_spi_pcf8833_dev[i].spi_gpio,
	   &g_spi_gpio_init[i],
	   sizeof(struct spi_gpio));

    rc = bcm2835_gpio_init_pin_func(&g_spi_pcf8833_dev[i]);
    if (rc) {
      goto spi_pcf8833_init_failed;
    }

    bcm2835_gpio_init_pin_val(&g_spi_pcf8833_dev[i]);
    
    /* Initialize cdev */
    rc = spi_pcf8833_setup_cdev(&g_spi_pcf8833_dev[i]);
    if (rc) {
      goto spi_pcf8833_init_failed;
    }
  }

  debug_print("Module init done\n");

  return 0;

spi_pcf8833_init_failed:
  printk(KERN_WARNING "*** "DRV_NAME": Module init failed\n");
  spi_pcf8833_exit();
  return rc;
}

/****************************************************************************
*
* Name spi_pcf8833_exit
*
* Description Kernel module 'exit' function for this driver. 
*
* Parameters None
*
* Error handling Returns 0 on success, otherwise a kernel error code.
*
****************************************************************************/
static void __exit spi_pcf8833_exit(void)
{
  int i;
  dev_t dev;

  if (g_spi_pcf8833_dev) {
    /* Get rid of our char dev entries and free memory */
    for (i=0; i < NR_DEVICES; i++) {
      if (g_vaddr_gpio) {
	bcm2835_gpio_restore_pins(&g_spi_pcf8833_dev[i]);
      }
      if (g_spi_pcf8833_dev[i].registrated) {
	cdev_del(&g_spi_pcf8833_dev[i].cdev);
      }
    }
    /* Free memory allocated for the devices */
    kfree(g_spi_pcf8833_dev);
  }

  /* Free I/O memory region for GPIO */
  if (g_vaddr_gpio) {
    iounmap(g_vaddr_gpio);
    release_mem_region(GPIO_BASE_ADDR, GPIO_SIZE_BYTES);
  }

  /* Free the device numbers */
  dev = MKDEV(g_major, g_minor);
  unregister_chrdev_region(dev, NR_DEVICES);  

  debug_print("Module exit done\n");
}
