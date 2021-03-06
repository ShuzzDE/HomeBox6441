#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

WIRELESS_MENU:=Wireless Drivers

define KernelPackage/net-airo
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Cisco Aironet driver
  DEPENDS:=@PCI_SUPPORT +@DRIVER_WEXT_SUPPORT
  KCONFIG:=CONFIG_AIRO
  FILES:=$(LINUX_DIR)/drivers/net/wireless/airo.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,50,airo)
endef

define KernelPackage/net-airo/description
 Kernel support for Cisco Aironet cards
endef

$(eval $(call KernelPackage,net-airo))


define KernelPackage/net-hermes
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Hermes 802.11b chipset support
  DEPENDS:=@LINUX_2_6 @PCI_SUPPORT||PCMCIA_SUPPORT +@DRIVER_WEXT_SUPPORT
  KCONFIG:=CONFIG_HERMES \
	CONFIG_HERMES_CACHE_FW_ON_INIT=n
  FILES:= \
	$(LINUX_DIR)/drivers/net/wireless/orinoco/orinoco.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,50,orinoco)
endef

define KernelPackage/net-hermes/description
 Kernel support for Hermes 802.11b chipsets
endef

$(eval $(call KernelPackage,net-hermes))


define KernelPackage/net-hermes-pci
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Intersil Prism 2.5 PCI support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_PCI_HERMES
  FILES:=$(LINUX_DIR)/drivers/net/wireless/orinoco/orinoco_pci.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,55,orinoco_pci)
endef

define KernelPackage/net-hermes-pci/description
 Kernel modules for Intersil Prism 2.5 PCI support
endef

$(eval $(call KernelPackage,net-hermes-pci))


define KernelPackage/net-hermes-plx
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=PLX9052 based PCI adaptor
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_PLX_HERMES
  FILES:=$(LINUX_DIR)/drivers/net/wireless/orinoco/orinoco_plx.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,55,orinoco_plx)
endef

define KernelPackage/net-hermes-plx/description
 Kernel modules for Hermes in PLX9052 based PCI adaptors
endef

$(eval $(call KernelPackage,net-hermes-plx))


define KernelPackage/net-hermes-pcmcia
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Hermes based PCMCIA adaptors
  DEPENDS:=@PCMCIA_SUPPORT
  KCONFIG:=CONFIG_PCMCIA_HERMES
  FILES:=$(LINUX_DIR)/drivers/net/wireless/orinoco/orinoco_cs.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,55,orinoco_cs)
endef

define KernelPackage/net-hermes-pcmcia/description
 Kernel modules for Hermes based PCMCIA adaptors
endef

$(eval $(call KernelPackage,net-hermes-pcmcia))


define KernelPackage/net-prism54
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Intersil Prism54 support
  DEPENDS:=@PCI_SUPPORT +@DRIVER_WEXT_SUPPORT
  KCONFIG:=CONFIG_PRISM54
  FILES:=$(LINUX_DIR)/drivers/net/wireless/prism54/prism54.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,60,prism54)
endef

define KernelPackage/net-prism54/description
 Kernel modules for Intersil Prism54 support
endef

# Prism54 FullMAC firmware (jbnore.free.fr seems to be rather slow, so we use daemonizer.de)
PRISM54_FW:=1.0.4.3.arm

define Download/net-prism54
  FILE:=$(PRISM54_FW)
  URL:=http://daemonizer.de/prism54/prism54-fw/fw-fullmac/
  MD5SUM:=8bd4310971772a486b9784c77f8a6df9
endef

define KernelPackage/net-prism54/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(DL_DIR)/$(PRISM54_FW) $(1)/lib/firmware/isl3890
endef

$(eval $(call Download,net-prism54))
$(eval $(call KernelPackage,net-prism54))

