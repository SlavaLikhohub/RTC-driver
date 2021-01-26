// SPDX-License-Identifier: GPL-2.0

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/i2c.h>
#include <linux/regmap.h>

MODULE_AUTHOR("Viaheslav Lykhohub <viacheslav.lykhohub@globallogic.com>");
MODULE_DESCRIPTION("RTC Driver");
MODULE_LICENSE("GPL");

static int ds1307x_probe(struct i2c_client * client,
			 const struct i2c_device_id * id)
{
	s32 data;

	pr_debug("++%s", __func__);

	data = i2c_smbus_read_byte_data(client , 0x01); /* minutes */

	pr_info("### read data = %#x\n", data);
	return 0;
}

static int ds1307x_remove(struct i2c_client * client)
{
	pr_debug("++%s", __func__);

	return 0;
}

static const struct i2c_device_id ds1307x_id[] = {
	{"ds1307x"},
	{ },
};

MODULE_DEVICE_TABLE(i2c, ds1307x_id);

static const struct of_device_id ds1307x_of_match[] = {
	{.compatible = "dallas,ds1307x"},
	{ },
};

MODULE_DEVICE_TABLE(of, ds1307x_of_match);

static struct i2c_driver ds1307_driver = {
	.driver = {
		.name = "ds1307x",
		.of_match_table = ds1307x_of_match,
	},
	.probe = ds1307x_probe,
	.remove = ds1307x_remove,
	.id_table = ds1307x_id,
};

module_i2c_driver(ds1307_driver);

