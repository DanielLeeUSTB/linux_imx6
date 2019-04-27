#include <linux/i2c.h>
#include <linux/pm.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "i2c_sensor.h"

static int _6050_init(struct i2c_client *client)
{
    i2c_smbus_write_byte_data(client, MPU_PWR_MGMT1_REG, 0X80);/*复位MPU6050*/
    mdelay(100);
    i2c_smbus_write_byte_data(client, MPU_PWR_MGMT1_REG, 0X00);
    i2c_smbus_write_byte_data(client, MPU_GYRO_CFG_REG, 3<<3);/*陀螺仪传感器,±2000dps*/
    i2c_smbus_write_byte_data(client, MPU_ACCEL_CFG_REG, 0<<3);/*加速度传感器,±2g*/
    i2c_smbus_write_byte_data(client, MPU_SAMPLE_RATE_REG, 1000 /50-1);/*设置采样率50Hz*/
    i2c_smbus_write_byte_data(client, MPU_CFG_REG, 4);/*自动设置LPF为采样率的一半*/
    i2c_smbus_write_byte_data(client, MPU_INT_EN_REG, 0X00);/*关闭所有中断*/
    i2c_smbus_write_byte_data(client, MPU_USER_CTRL_REG, 0X00);/*I2C主模式关闭*/
    i2c_smbus_write_byte_data(client, MPU_FIFO_EN_REG, 0X00);/*关闭FIFO*/
    i2c_smbus_write_byte_data(client, MPU_INTBP_CFG_REG, 0X80);/*INT引脚低电平有效*/
    i2c_smbus_write_byte_data(client, MPU_CFG_REG, 3);//设置数字低通滤波器
}

static int temp_sensor_get_temp(struct i2c_client *client, char *value)
{
    short int temp = 0;

    i2c_smbus_read_i2c_block_data(client, MPU_TEMP_OUTH_REG, 2, value);
    return temp;
}

static ssize_t i2c_sensor_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);
    if(buf[0] == '1')
    {

    }
    else
    {

    }
	return count;
}

static ssize_t i2c_sensor_show(struct device *dev,
                 struct device_attribute *attr,
                 char *buf)
{
    int len = 0;
    short int temp = 0;
    char value[2] = {0};
    struct i2c_client *client = container_of(dev, struct i2c_client, dev);
    if (client)
    {
        temp_sensor_get_temp(client, value);
        temp = (value[0]<<8) + value[1];
        len = sprintf(buf,"vaule=%d\n",(temp/340)+36);
    }
	//printk("this is i2c_sensor show\n");
	return len;
}

static DEVICE_ATTR(i2c_sensor, 0660, i2c_sensor_show, i2c_sensor_store);

static struct attribute *i2c_sensor_attributes[] = {
    &dev_attr_i2c_sensor.attr,
    NULL
};

static struct attribute_group i2c_sensor_attribute_group = {
    .attrs = i2c_sensor_attributes
};

static int temp_sensor_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int result = 0;
    int device_id = 0;
    int temp_h = 0;
    int temp_l = 0;
    unsigned int temp = 0;

    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
    printk("--------temp_sensor_probe\n");
    
    result = i2c_check_functionality(adapter,
					 I2C_FUNC_SMBUS_BYTE |
					 I2C_FUNC_SMBUS_BYTE_DATA);
    if (!result)
    {
        printk("i2c_check_functionality failed");
        return result;
    }
    device_id = i2c_smbus_read_byte_data(client, MPU_DEVICE_ID_REG);
    if (device_id != MPU_WHO_AM_I_ID)
    {
        printk("the device id is 0x%x\n", device_id);
        return -1;
    }

    _6050_init(client);
    mdelay(100);

    temp_h = i2c_smbus_read_byte_data(client, MPU_TEMP_OUTH_REG);
    temp_l = i2c_smbus_read_byte_data(client, MPU_TEMP_OUTL_REG);
    //temp = (unsigned int)(((temp_h<<8) + temp_l)/340 + 36.5);
    printk("temp_h = 0x%x, temp_l = 0x%x, %d", temp_h, temp_l, temp);

    sysfs_create_group(&client->dev.kobj, &i2c_sensor_attribute_group);

    return 0;
}

static int temp_sensor_remove(struct i2c_client *client)
{
    return 0;
}

static int temp_sensor_suspend(struct device *dev)
{
    return 0;
}

static int temp_sensor_resume(struct device *dev)
{
    return 0;
}

static const struct i2c_device_id temp_sensor_id[] =
{
    {"tempsensor", 0},
    {}
};

static const struct of_device_id temp_of_match[] = {
       { .compatible = "i2c,temperature", },
       { }
};

MODULE_DEVICE_TABLE(i2c, temp_sensor_id);
MODULE_DEVICE_TABLE(of, temp_of_match);
static SIMPLE_DEV_PM_OPS(temp_sensor_pm_ops, temp_sensor_suspend, temp_sensor_resume);

static struct i2c_driver temp_sensor_driver = {
	.driver		= {
		.name	= "temp",
		.owner	= THIS_MODULE,
		.pm	= &temp_sensor_pm_ops,
        .of_match_table = temp_of_match,
	},
	.probe		= temp_sensor_probe,
	.remove		= temp_sensor_remove,
	.id_table	= temp_sensor_id,
};
/*static int __init temp_sensor_init(void)
{
    printk("-------temp_sensor_init\n");
    return platform_driver_register(&temp_sensor_driver);
}

static void __exit temp_sensor_exit(void)
{
    printk("-------temp_sensor_exit\n");
    platform_driver_unregister(&temp_sensor_driver);
}

module_init(temp_sensor_init);
module_exit(temp_sensor_exit);*/
module_i2c_driver(temp_sensor_driver);

MODULE_AUTHOR("daniel");
MODULE_DESCRIPTION("i2c test");
MODULE_LICENSE("GPL v2");