// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 - 2019 MediaTek Inc.
 * Author: Frank Wunderlich <frank-w@public-files.de>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/io.h>

#include <malloc.h>
#include <usb.h>
#include <watchdog.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <linux/usb/dwc3.h>
#include <power/regulator.h>
#include <asm/gpio.h>

#include <usb/xhci.h>



struct mtk_xhci {
	/*struct usb_platdata usb_plat;
	struct xhci_ctrl ctrl;
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;*/
};

struct mtk_xhci_platdata { // xhci_hcd_mtk
        struct device *dev;
        struct udevice *vusb33;
        struct udevice *vbus;
        struct clk *sys_clk;    // sys and mac clock
        struct clk *ref_clk;

	struct xhci_hccr *hccr; //base-adress
	struct xhci_hcor *hcor; //hccr+len
};

/*
struct xhci_hcd_mtk {
        struct device *dev;
        struct usb_hcd *hcd;
        struct mu3h_sch_bw_info *sch_array;
        struct mu3c_ippc_regs __iomem *ippc_regs;
        bool has_ippc;
        int num_u2_ports;
        int num_u3_ports;
        int u3p_dis_msk;
        struct regulator *vusb33;
        struct regulator *vbus;
        struct clk *sys_clk;    // sys and mac clock
        struct clk *xhci_clk;
        struct clk *ref_clk;
        struct clk *mcu_clk;
        struct clk *dma_clk;
        struct regmap *pericfg;
        struct phy **phys;
        int num_phys;
        bool lpm_support;
        // usb remote wakeup
        bool uwk_en;
        struct regmap *uwk;
        u32 uwk_reg_base;
        u32 uwk_vers;
};
*/

static int mtk_xhci_probe(struct udevice *dev)
{
	struct mtk_xhci_platdata *mtk = dev_get_platdata(dev);
	//struct udevice *regulator,*regulator2;

	int ret,err;

	printf("%s %d: %s\n",__FUNCTION__,__LINE__,ofnode_get_name(dev_ofnode(dev)));


/*
        mtk = devm_kzalloc(dev, sizeof(*mtk), GFP_KERNEL);
        if (!mtk)
                return -ENOMEM;

        mtk->dev = dev;

        mtk->vbus = devm_regulator_get(dev, "vbus");
        if (IS_ERR(mtk->vbus)) {
                dev_err(dev, "fail to get vbus\n");
                return PTR_ERR(mtk->vbus);
        }

        mtk->vusb33 = devm_regulator_get(dev, "vusb33");
        if (IS_ERR(mtk->vusb33)) {
                dev_err(dev, "fail to get vusb33\n");
                return PTR_ERR(mtk->vusb33);
        }

        ret = xhci_mtk_clks_get(mtk);
        if (ret)
                return ret;
*/

//power controlled over powerdomain/hifsys like pcie
/*
        ret = device_get_supply_regulator(dev, "vbus", &mtk->vbus);
        if (!ret) {
		//mtk->vbus = regulator;
                ret = regulator_set_enable(mtk->vbus, true);
                if (ret) {}
	}else printf ("cannot get vbus\n");

        ret = device_get_supply_regulator(dev, "vusb33", &mtk->vusb33);
        if (!ret) {
                ret = regulator_set_enable(mtk->vusb33, true);
                if (ret) {}
	}else printf("cannot get vusb33");
*/
	err = clk_get_by_name(dev, "sys_ck", mtk->sys_clk);
	if (err)
		return err;

	printf ("got clock\n");
	/* enable sys clock */
	err = clk_enable(mtk->sys_clk);
	if (err)
		return err;

	printf ("enabled clock\n");

	mtk->hccr = (struct xhci_hccr *)devfdt_get_addr(dev);
	if ((int)mtk->hccr == FDT_ADDR_T_NONE) {
		debug("Can't get the XHCI register base address\n");
		return -ENXIO;
	}

	printf ("address: 0x%x\n",(uint32_t)mtk->hccr);

//fdt_addr_t devfdt_get_addr_size_index(struct udevice *dev, int index,fdt_size_t *size);

	for (int i=0;i<2;i++)
	{
		fdt_size_t size;
		fdt_addr_t addr=devfdt_get_addr_size_index(dev,i,&size);
		if (addr == FDT_ADDR_T_NONE) break;

		printf ("address: 0x%x size:0x%x\n",(int)addr,(int)size);
	}

/*
	struct rockchip_xhci_platdata *plat = dev_get_platdata(dev);
	struct rockchip_xhci *ctx = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int ret;

	ctx->hcd = (struct xhci_hccr *)plat->hcd_base;
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);
	hcor = (struct xhci_hcor *)((uint64_t)ctx->hcd +
			HC_LENGTH(xhci_readl(&ctx->hcd->cr_capbase)));

	if (plat->vbus_supply) {
		ret = regulator_set_enable(plat->vbus_supply, true);
		if (ret) {
			pr_err("XHCI: failed to set VBus supply\n");
			return ret;
		}
	}

	ret = rockchip_xhci_core_init(ctx, dev);
	if (ret) {
		pr_err("XHCI: failed to initialize controller\n");
		return ret;
	}

	return xhci_register(dev, ctx->hcd, hcor);
*/
	return -ENODEV;
}

static const struct udevice_id mtk_xhci_id_table[] = {
	{ .compatible = "mediatek,mt7623-xhci", },
	{ }
};

U_BOOT_DRIVER(mtk_xhci) = {
	.name		= "mtk-xhci",
	.id		= UCLASS_USB,
	.of_match	= mtk_xhci_id_table,
	//.ops		= &xhci_usb_ops,
	.probe		= mtk_xhci_probe,
	.platdata_auto_alloc_size = sizeof(struct mtk_xhci_platdata),
	.priv_auto_alloc_size = sizeof(struct mtk_xhci),
};
