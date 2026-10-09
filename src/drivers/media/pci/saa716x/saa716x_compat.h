/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef SAA716X_COMPAT_H
#define SAA716X_COMPAT_H

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>

typedef struct osd_raw_cmd_s {
        void __user *cmd_data;
        int cmd_len;
        void __user *result_data;
        int result_len;
} osd_raw_cmd_t;

typedef struct osd_raw_data_s {
        void __user *data_buffer;
        int data_length;
        int data_handle;
} osd_raw_data_t;

#define OSD_RAW_CMD  _IOWR('o', 162, osd_raw_cmd_t)
#define OSD_RAW_DATA _IOWR('o', 163, osd_raw_data_t)

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
#define timer_delete_sync(timer) del_timer_sync(timer)
#endif

#ifndef timer_container_of
#define timer_container_of(var, callback_timer, timer_fieldname) \
        from_timer(var, callback_timer, timer_fieldname)
#endif

#define AUDIO_GET_PTS _IOR('o', 19, __u64)

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
#define pci_dbg(pdev, fmt, ...) \
        dev_dbg(&(pdev)->dev, fmt, ##__VA_ARGS__)
#define pci_err(pdev, fmt, ...) \
        dev_err(&(pdev)->dev, fmt, ##__VA_ARGS__)
#define pci_info(pdev, fmt, ...) \
        dev_info(&(pdev)->dev, fmt, ##__VA_ARGS__)
#define pci_warn(pdev, fmt, ...) \
        dev_warn(&(pdev)->dev, fmt, ##__VA_ARGS__)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 9, 0)
#define PCI_IRQ_INTX PCI_IRQ_LEGACY
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 9, 0)
#define SAA716X_USE_TASKLET
#else
#define SAA716X_USE_BH_WORKQUEUE
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
#define fallthrough do {} while (0)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)

static inline struct i2c_client *
saa716x_module_probe(const char *module_name,
                     const char *name,
                     struct i2c_adapter *adap,
                     unsigned char addr,
                     void *platform_data)
{
        struct i2c_client *client;
        struct i2c_board_info *board_info;

        board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
        if (!board_info)
                return NULL;

        if (name)
                strlcpy(board_info->type, name, I2C_NAME_SIZE);
        else
                strlcpy(board_info->type, module_name, I2C_NAME_SIZE);

        board_info->addr = addr;
        board_info->platform_data = platform_data;

        request_module(module_name);

        client = i2c_new_device(adap, board_info);
        if (client == NULL || client->dev.driver == NULL) {
                kfree(board_info);
                return NULL;
        }

        if (!try_module_get(client->dev.driver->owner)) {
                i2c_unregister_device(client);
                client = NULL;
        }

        kfree(board_info);
        return client;
}

static inline void
saa716x_module_release(struct i2c_client *client)
{
        if (!client)
                return;

        module_put(client->dev.driver->owner);
        i2c_unregister_device(client);
}

#else

#define saa716x_module_probe   dvb_module_probe
#define saa716x_module_release dvb_module_release

#endif

#endif
