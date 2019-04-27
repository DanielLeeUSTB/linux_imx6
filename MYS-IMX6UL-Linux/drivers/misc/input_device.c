#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
struct private_data
{
    struct platform_device *pdev;
    struct input_dev *input;
    struct pinctrl *pctrl;
    struct gpio_desc *led;
    int irq_gpio;
    int irq;
};

static ssize_t gpio_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
    struct private_data *pwdata = dev_get_drvdata(dev);
    if (!pwdata)
    {
        return -ENOMEM;
    }
    if(buf[0] == '1')
    {
        printk("this is gpio store1\n");
        gpiod_set_value(pwdata->led, 1);
    }
    else
    {
        printk("this is gpio store0\n");
        gpiod_set_value(pwdata->led, 0);
    }
	return count;
}

static ssize_t gpio_show(struct device *dev,
                 struct device_attribute *attr,
                 char *buf)
{
    char value = 0;
    int len = 0;
    struct private_data *pwdata = dev_get_drvdata(dev);
    if (!pwdata)
    {
        return -ENOMEM;
    }
    value = gpio_get_value(pwdata->irq_gpio);
    len = sprintf(buf,"vaule=%d\n",value);
	//printk("this is gpio show\n");
	return len;
}

static DEVICE_ATTR(gpio, 0660, gpio_show, gpio_store);

static struct attribute *gpio_attributes[] = {
    &dev_attr_gpio.attr,
    NULL
};

static struct attribute_group gpio_attribute_group = {
    .attrs = gpio_attributes
};

static irqreturn_t key_irq_handler(int irq, void *dev_id)
{
    struct private_data *pwdata = (struct private_data *)dev_id;
    int pin_val = gpio_get_value(pwdata->irq_gpio);
    gpiod_set_value(pwdata->led, pin_val);
    input_report_key(pwdata->input, KEY_WAKEUP, 0x30+pin_val);
    printk("this is the key value change irq hander, pin_val=%d\n", pin_val);
    return IRQ_HANDLED;
}

static int gpio_parse_dts(struct platform_device *pdev, struct private_data *pwdata)
{
    struct device_node *np = pdev->dev.of_node;
    if (!np)
    {
        return -ENOENT;
    }

    // pwdata->pctrl = devm_pinctrl_get_select_default(&pdev->dev);
    // if (IS_ERR(pwdata->pctrl))
    // {
    //     dev_err(&pdev->dev,"devm_pinctrl_get_select_default error");
    //     return -ENOENT;
    // }

    pwdata->irq_gpio = of_get_named_gpio(np, "int-gpios", 0);
    if (!gpio_is_valid(pwdata->irq_gpio))
    {
        dev_err(&pdev->dev,"faild to get interrupt gpio");
        return -ENOENT;
    }

    pwdata->irq = gpio_to_irq(pwdata->irq_gpio);
    if (pwdata->irq < 0)
    {
        return pwdata->irq;
    }

    pwdata->led = gpiod_get_index(&pdev->dev, "key", 0, GPIOD_OUT_LOW);

    if (!pwdata->led)
    {
        dev_err(&pdev->dev,"faild to get led");
        return -ENOENT;
    }

    return 0;

}

static int input_gpio_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct private_data *pwdata;
    struct input_dev *input;
    printk("-------input_gpio_probe\n");
    pwdata = devm_kzalloc(&pdev->dev, sizeof(struct private_data), GFP_KERNEL);
    if (pwdata == NULL)
    {
        return -ENOMEM;
    }
    pwdata->pdev = pdev;
    printk("-------devm_input_allocate_device\n");
    input = devm_input_allocate_device(&pdev->dev);
    if (input == NULL)
    {
        return -ENOMEM;
    }
    pwdata->input = input;
    platform_set_drvdata(pdev, pwdata);
    dev_set_drvdata(&pdev->dev, pwdata);

    input_set_drvdata(input, pwdata);
    printk("-------gpio_parse_dts\n");
    ret = gpio_parse_dts(pdev, pwdata);
    if (ret < 0)
    {
        return -ENOMEM;
    }

    input->name = "gpio-keys";
    input->phys = "gpio-keys/input0";
    input->dev.parent = &pdev->dev;
    input->id.bustype = BUS_HOST;
    input->id.product = 0x0001;
    input->id.version = 0x0001;
    input_set_capability(input, EV_KEY, KEY_WAKEUP);
    ret =  input_register_device(input);
    if (ret)
    {
        dev_err(&pdev->dev,"input_register_device failed");
        return ret;
    }
    ret = devm_request_threaded_irq(&pdev->dev, pwdata->irq, NULL, key_irq_handler,
    IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "key_press", pwdata);

    ret = sysfs_create_group(&pdev->dev.kobj, &gpio_attribute_group);

    return 0;
}
static int input_gpio_suspend(struct platform_device *pdev)
{
    return 0;
}

static int input_gpio_resume(struct platform_device *pdev)
{
    return 0;
}

static int input_gpio_remove(struct platform_device *pdev)
{
    return 0;
}

static SIMPLE_DEV_PM_OPS(input_gpio_pm, input_gpio_suspend, input_gpio_resume);

static const struct of_device_id input_gpio_of_match[] = {
  {.compatible = "my-keys", },
  {}
};

MODULE_DEVICE_TABLE(of, input_gpio_of_match);

static struct platform_driver input_gpio_driver = {
    .probe = input_gpio_probe,
    .remove = input_gpio_remove,
    .driver =
	{
	   .name = "my-keys",
           .owner = THIS_MODULE,
           .pm = &input_gpio_pm,
           .of_match_table = of_match_ptr(input_gpio_of_match),
	}	
};

static int __init input_gpio_init(void)
{
    printk("-------input_gpio_init\n");
    return platform_driver_register(&input_gpio_driver);
}

static void __exit input_gpio_exit(void)
{
    printk("-------input_gpio_exit\n");
    platform_driver_unregister(&input_gpio_driver);
}

module_init(input_gpio_init);
module_exit(input_gpio_exit);

MODULE_AUTHOR("daniel");
MODULE_DESCRIPTION("imx6 data input test");
MODULE_LICENSE("GPL v2");
