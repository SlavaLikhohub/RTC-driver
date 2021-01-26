// SPDX-License-Identifier: GPL-2.0

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/bcd.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/regmap.h>
#include <linux/rtc.h>

MODULE_AUTHOR("Viaheslav Lykhohub <viacheslav.lykhohub@globallogic.com>");
MODULE_DESCRIPTION("RTC ds1307x Driver");
MODULE_LICENSE("GPL");

enum ds1307x_regs {
	DS1307X_SEC = 0,
	DS1307X_MIN,
	DS1307X_HOUR,
	DS1307X_DAY,
	DS1307X_DATE,
	DS1307X_MONTH,
	DS1307X_YEAR,
	DS1307X_REGS_NUM
};

struct ds1307x {
	struct i2c_client *client;
	struct regmap *map;
	struct rtc_device *rtc;
};

static const struct regmap_config regmap_cfg = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0x3f,
};

int ds1307x_read(struct device *dev, struct rtc_time *rtc_time)
{
	struct ds1307x *ds = dev_get_drvdata(dev);
	u8 ds1307x_time[DS1307X_REGS_NUM] = {0};
	int ret;

	pr_debug("++%s", __func__);

	ret = regmap_bulk_read(ds->map, DS1307X_SEC, ds1307x_time,
			       ARRAY_SIZE(ds1307x_time));

	if (ret < 0) {
		pr_err("Can't read from RTC\n");
		return ret;
	}

	rtc_time->tm_sec  = bcd2bin(ds1307x_time[DS1307X_SEC]);
	rtc_time->tm_min  = bcd2bin(ds1307x_time[DS1307X_MIN]);
	rtc_time->tm_hour = bcd2bin(ds1307x_time[DS1307X_HOUR]);
	rtc_time->tm_mday = bcd2bin(ds1307x_time[DS1307X_DATE]);
	rtc_time->tm_mon  = bcd2bin(ds1307x_time[DS1307X_MONTH]);
	rtc_time->tm_year = bcd2bin(ds1307x_time[DS1307X_YEAR]);

	pr_debug("Sec: %d\n",   rtc_time->tm_sec);
	pr_debug("Min: %d\n",   rtc_time->tm_min);
	pr_debug("Hour: %d\n",  rtc_time->tm_hour);
	pr_debug("Day: %d\n",   rtc_time->tm_mday);
	pr_debug("Month: %d\n", rtc_time->tm_mon);
	pr_debug("Year: %d\n",  rtc_time->tm_year);

	return 0;
}

int ds1307x_set_time(struct device *dev, struct rtc_time *rtc_time)
{
	struct ds1307x *ds = dev_get_drvdata(dev);
	u8 ds1307x_time[DS1307X_REGS_NUM] = {0};
	int ret;

	pr_debug("++%s", __func__);

	pr_debug("Sec: %d\n",   rtc_time->tm_sec);
	pr_debug("Min: %d\n",   rtc_time->tm_min);
	pr_debug("Hour: %d\n",  rtc_time->tm_hour);
	pr_debug("Day: %d\n",   rtc_time->tm_mday);
	pr_debug("Month: %d\n", rtc_time->tm_mon);
	pr_debug("Year: %d\n",  rtc_time->tm_year);

	ds1307x_time[DS1307X_SEC]   = bin2bcd(rtc_time->tm_sec);
	ds1307x_time[DS1307X_MIN]   = bin2bcd(rtc_time->tm_min);
	ds1307x_time[DS1307X_HOUR]  = bin2bcd(rtc_time->tm_hour);
	ds1307x_time[DS1307X_DATE]  = bin2bcd(rtc_time->tm_mday);
	ds1307x_time[DS1307X_MONTH] = bin2bcd(rtc_time->tm_mon);
	ds1307x_time[DS1307X_YEAR]  = bin2bcd(rtc_time->tm_year);

	ret = regmap_bulk_write(ds->map, DS1307X_SEC, ds1307x_time,
			        ARRAY_SIZE(ds1307x_time));

	if (ret < 0) {
		pr_err("Can't write to RTC\n");
		return ret;
	}

	return 0;
}

static const struct rtc_class_ops ds1307x_ops = {
	.read_time = ds1307x_read,
	.set_time = ds1307x_set_time,
};

static int ds1307x_probe(struct i2c_client *client,
			 const struct i2c_device_id * id)
{
	struct ds1307x *ds;

	pr_debug("++%s", __func__);

	ds = devm_kzalloc(&client->dev, sizeof(*ds), GFP_KERNEL);
	if (NULL == ds) {
		pr_err("Can't allocate memory\n");
		return -ENOMEM;
	}

	dev_set_drvdata(&client->dev, ds);

	ds->client = client;
	ds->map = devm_regmap_init_i2c(client, &regmap_cfg);\
	if (NULL == ds->map) {
		pr_err("Can't init regmap i2c\n");
		return -ENOMEM;
	}

	ds->rtc = devm_rtc_device_register(&client->dev, NULL, &ds1307x_ops,
					   THIS_MODULE);
	if (NULL == ds->rtc) {
		pr_err("Can't register RTC device\n");
		return -ENOMEM;
	}

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

