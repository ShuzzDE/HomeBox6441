#
# Openwrt Makefile for hostapd
#

#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#### Package Info ###############################

PKG_NAME:=hostapd_08x
PKG_REV:=22760dd94722a61175ff90c59d88c4cda1ed5e23
PKG_VERSION:=22760dd
PKG_RELEASE:=1

PKG_BASE_NAME:=hostap
PKG_SOURCE:=$(PKG_BASE_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://hostap.epitest.fi/gitweb/gitweb.cgi?"p=hostap.git;a=snapshot;h=22760dd94722a61175ff90c59d88c4cda1ed5e23;sf=tgz"
 
stam:
	@echo the BUILD_VARIANT is
	@echo $(BUILD_VARIANT)

PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_BASE_NAME)-$(PKG_VERSION)

PKG_BUILD_DIR_HAPD:=$(PKG_BUILD_DIR)/hostapd
PKG_BUILD_DIR_WPAS:=$(PKG_BUILD_DIR)/wpa_supplicant

PKG_BUILD_DEPENDS:=libopenssl libnl
PKG_CONFIG_DEPENDS:=\
	CONFIG_WIRELESS_STA

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=IEEE 802.1x Authenticator(0_8_x)
  URL:=http://hostap.epitest.fi/
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  IEEE 802.11 AP, IEEE 802.1X/WPA/WPA2/EAP/RADIUS Authenticator
endef

define Package/$(PKG_NAME)/config
	config WIRELESS_STA
	  depends on PACKAGE_$(PKG_NAME)
	  bool 'Linux WPA/WPA2/IEEE 802.1X Supplicant support'
	  help
	      Linux WPA/WPA2/IEEE 802.1X Supplicant
endef

#### Directory Paths ############################
OPENSSL_DIR:=$(wildcard $(BUILD_DIR)/openssl-*)
LIBNL_DIR:=$(wildcard $(BUILD_DIR)/libnl-*)

IFX_CFLAGS += -I$(OPENSSL_DIR)/include -I$(LIBNL_DIR)/include
IFX_LDFLAGS += -L$(OPENSSL_DIR)/ -L$(LIBNL_DIR)/lib -lnl


#### Target Rules ###############################
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src clean
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(PKG_BUILD_DIR_HAPD) \
		IFX_CFLAGS="$(IFX_CFLAGS)" IFX_LDFLAGS="$(IFX_LDFLAGS)"
	$(if $(CONFIG_WIRELESS_STA), \
		$(MAKE) -C $(PKG_BUILD_DIR)/src clean; \
		$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(PKG_BUILD_DIR_WPAS) \
		    IFX_CFLAGS="$(IFX_CFLAGS)" IFX_LDFLAGS="$(IFX_LDFLAGS)" \
	)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR_HAPD)/hostapd $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR_HAPD)/hostapd_cli $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR_HAPD)/hostapd.eap_user $(1)/bin
	$(if $(CONFIG_WIRELESS_STA), \
		$(INSTALL_BIN) $(PKG_BUILD_DIR_WPAS)/wpa_supplicant $(1)/bin; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR_WPAS)/wpa_cli $(1)/bin; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR_WPAS)/wpa_passphrase $(1)/bin \
	)
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR_HAPD) ]; then $(MAKE) -C $(PKG_BUILD_DIR_HAPD) clean; fi
	if [ -d $(PKG_BUILD_DIR_WPAS) ]; then $(MAKE) -C $(PKG_BUILD_DIR_WPAS) clean; fi
	if [ -d $(PKG_BUILD_DIR) ]; then $(RM) -r $(PKG_BUILD_DIR)/ipkg-$(BOARD); fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
