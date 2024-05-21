#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/mac80211.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LaVa264");
MODULE_DESCRIPTION("Virtual WLAN device driver.");

#define __pr_info(fmt, arg...) pr_info("%s():" fmt, __FUNCTION__, ##arg)
#define __pr_err(fmt, arg...) pr_err("%s():" fmt, __FUNCTION__, ##arg)

struct vwlan_data
{
};

/**
 * struct ieee80211_ops - callbacks from mac80211 to the driver.
 * 
 * This structure contains various callbacks that the driver may handle or,
 * in some cases, must handle, for example to configure the hardware to a
 * new channel or to transmit a frame.
 * 
 * @tx: Handler that 802.11 module calls for each transmitted frame.
 * @start: Called before the first net-device attached to the hardware is
 *         enabled. This should turn on the hardware and must turn on frame
 *         reception.
 */
static struct ieee80211_ops vwlan_ops = {
    .tx = tx,
    .start = start};

static int __init init_vwlan(void)
{
    struct ieee80211_hw *hw = NULL;
    struct vwlan_data *data = NULL;

    /**
     * @brief Allocate a new hardware device.
     *
     * @priv_data_len: Length of private data.
     * @ops: Callbacks for this device.
     */
    hw = ieee80211_alloc_hw(sizeof(vwlan_data), &vwlan_ops);

    return 0;
}

static void __exit exit_vwlan(void)
{
}

module_init(init_vwlan);
module_exit(exit_vwlan);
