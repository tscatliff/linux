// Adapted from: https://github.com/cirosantilli/linux-kernel-module-cheat/blob/master/kernel_modules/sysfs.c

#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <uapi/linux/stat.h> /* S_IRUSR, S_IWUSR  */
#include <linux/mmc/sd.h>

#ifdef SYSFS_SD_SUPPORT

/* global */
static struct kobject * kobj;

#define PASSWORD_LENGTH_MAX 16
extern int SYSFS_SD_PASSWORD_LENGTH;
extern char SYSFS_SD_PASSWORD[PASSWORD_LENGTH_MAX];
extern int SYSFS_SD_CARD_INDEX;
extern int SYSFS_SD_CARD_LOCKED;
extern u32 SYSFS_SD_CARD_CID[4];
extern u32 SYSFS_SD_CARD_CSD[4];
extern u32 SYSFS_SD_CARD_SSR[16];
extern u32 SYSFS_SD_CARD_SCR[2];
extern u32 SYSFS_SD_CARD_OCR[1];

static ssize_t index_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  char temp[2];
  scnprintf(temp, 2, "%d", SYSFS_SD_CARD_INDEX);
	memset(buff, 0, PAGE_SIZE);
	strcat(buff, temp);
  if (SYSFS_SD_CARD_INDEX < 0) {
    return 2;
  }
  return 1;
}

static ssize_t locked_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  if (SYSFS_SD_CARD_INDEX < 0)
  {
    scnprintf(buff, PAGE_SIZE, "n/a");
    return 3;
  }
  else if (SYSFS_SD_CARD_LOCKED == 1)
  {
    scnprintf(buff, PAGE_SIZE, "locked");
    return 6;
  }
  else
  {
    scnprintf(buff, PAGE_SIZE, "unlocked");
    return 8;
  }
}

static ssize_t password_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  memset(buff, 0, PAGE_SIZE);
	if (SYSFS_SD_PASSWORD_LENGTH > 0) {
		memcpy(buff, SYSFS_SD_PASSWORD, SYSFS_SD_PASSWORD_LENGTH);
	}

	return SYSFS_SD_PASSWORD_LENGTH;
}

static ssize_t password_store(struct  kobject *kobj, struct kobj_attribute *attr, const char *buff, size_t count)
{
  int ret = 0;

	if (count < 0) {
		SYSFS_SD_PASSWORD_LENGTH = 0;
	}
	else if (count > 16) {
		SYSFS_SD_PASSWORD_LENGTH = 16;
	} else {
		SYSFS_SD_PASSWORD_LENGTH = count;
	}

  memset(SYSFS_SD_PASSWORD, 0, PASSWORD_LENGTH_MAX);

  //pr_info("SD Password Length: %d", password_length);

  if (count > 0) {
		memcpy(SYSFS_SD_PASSWORD, buff, SYSFS_SD_PASSWORD_LENGTH);
		ret = SYSFS_SD_PASSWORD_LENGTH;
		//pr_info("SD Password Copied: %d bytes", ret);
	}

	//printk(KERN_INFO "Storing SD Password: [%s] %d bytes\n", SYSFS_SD_PASSWORD, password_length);

  return ret;
}

/*
static ssize_t password_hex_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
	int i;
	char tmp[16];
	int outlen = 0;

	memset(buff, 0, PAGE_SIZE);

	for (i = 0; i < password_length; i++) {
		if (i > 0) {
			strcat(buff, " ");
			outlen++;
		}
		scnprintf(tmp, 16, "0x%02X", SYSFS_SD_PASSWORD[i]);
		outlen += 4;
		strcat(buff, tmp);
	}

	return outlen;
}
*/

static ssize_t password_length_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  char temp[2];
  scnprintf(temp, 2, "%d", SYSFS_SD_PASSWORD_LENGTH);
	memset(buff, 0, PAGE_SIZE);
	strcat(buff, temp);
  return strlen(temp);
}

static ssize_t array_to_buff(struct kobject *kobj, struct kobj_attribute *attr, char *buff, u32 * src, int length)
{
  int i;

  memset(buff, 0, PAGE_SIZE);

  if (SYSFS_SD_CARD_INDEX > 0)
  {
  	//pr_warn("sysfs_sd_card: mmc%d CID:\n", SYSFS_SD_CARD_INDEX);
  	for (i = 0; i < length; i++) {
  		buff[4*i+0] = (src[i] & 0xFF000000) >> 24;
  		buff[4*i+1] = (src[i] & 0x00FF0000) >> 16;
  		buff[4*i+2] = (src[i] & 0x0000FF00) >>  8;
  		buff[4*i+3] = (src[i] & 0x000000FF); // >> 0;
  		//pr_warn("  %02X %02X %02X %02X\n", buff[4*i+0], buff[4*i+1], buff[4*i+2], buff[4*i+3]);
  	}
  }

	return 4*length;
}

static ssize_t cid_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  return array_to_buff(kobj, attr, buff, SYSFS_SD_CARD_CID, 4);
}

static ssize_t csd_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  return array_to_buff(kobj, attr, buff, SYSFS_SD_CARD_CSD, 4);
}

static ssize_t ssr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  return array_to_buff(kobj, attr, buff, SYSFS_SD_CARD_SSR, 16);
}

static ssize_t scr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  return array_to_buff(kobj, attr, buff, SYSFS_SD_CARD_SCR, 2);
}

static ssize_t ocr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
  return array_to_buff(kobj, attr, buff, SYSFS_SD_CARD_OCR, 1);
}

static struct kobj_attribute index_attribute =
	__ATTR(index, S_IRUGO, index_show, NULL);

static struct kobj_attribute locked_attribute =
	__ATTR(locked, S_IRUGO, locked_show, NULL);

static struct kobj_attribute password_attribute =
	__ATTR(password, S_IRUGO | S_IWUSR, password_show, password_store);

/*
static struct kobj_attribute password_hex_attribute =
	__ATTR(password_hex, S_IRUGO, password_hex_show, NULL);
*/

static struct kobj_attribute password_length_attribute =
	__ATTR(password_length, S_IRUGO, password_length_show, NULL);

static struct kobj_attribute cid_attribute =
	__ATTR(cid, S_IRUGO, cid_show, NULL);

static struct kobj_attribute csd_attribute =
	__ATTR(csd, S_IRUGO, csd_show, NULL);

static struct kobj_attribute ssr_attribute =
	__ATTR(ssr, S_IRUGO, ssr_show, NULL);

static struct kobj_attribute scr_attribute =
	__ATTR(scr, S_IRUGO, scr_show, NULL);

static struct kobj_attribute ocr_attribute =
	__ATTR(ocr, S_IRUGO, ocr_show, NULL);

static struct attribute *attrs[] = {
  &index_attribute.attr,
  &locked_attribute.attr,
  &password_attribute.attr,
//	&password_hex_attribute.attr,
  &password_length_attribute.attr,
  &cid_attribute.attr,
  &csd_attribute.attr,
  &ssr_attribute.attr,
  &scr_attribute.attr,
  &ocr_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __init sd_card_init(void)
{
  int ret;

  kobj = kobject_create_and_add("sysfs_sd_card", kernel_kobj);

  if (!kobj) {
    pr_err("sysfs_sd_card: Unable to allocate memory\n");
    return -ENOMEM;
  }

	SYSFS_SD_PASSWORD_LENGTH = 0;
	memset(SYSFS_SD_PASSWORD, 0, PASSWORD_LENGTH_MAX);

	ret = sysfs_create_group(kobj, &attr_group);

  if (ret) {
    pr_err("sysfs_sd_card: Unable to create attribute group\n");
    kobject_put(kobj);
  }

  ret = 0;

  printk(KERN_INFO "sysfs_sd_card: SD Card Interface initialized\n");

  return ret;
}

static void __exit sd_card_exit(void)
{
  kobject_put(kobj);
	printk(KERN_INFO "sysfs_sd_card: SD Card Interface shutting down\n");
}

#else

static int __init sd_card_init(void)
{
  printk(KERN_INFO "sysfs_sd_card: SD Card Interface (empty) initialized\n");

  return 0;
}

static void __exit sd_card_exit(void)
{
	printk(KERN_INFO "sysfs_sd_card: SD Card Interface (empty) shutting down\n");
}

#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tscatliff");
MODULE_DESCRIPTION("SysFS SD Card Interface.");
MODULE_VERSION("0.01");

/* Register module functions */
module_init(sd_card_init);
module_exit(sd_card_exit);
