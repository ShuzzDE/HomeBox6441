#!/bin/sh
#
# Linux Native MMC-SDIO stack platform setup script
#
#

CFG80211_AVAIL="no"
CFG80211_LOADED="no"

modprobe -l cfg80211 | grep cfg80211 > /dev/null
if [ $? -eq 0 ]; then
# cfg80211 kernel module is available
CFG80211_AVAIL="yes"	
fi

lsmod | grep cfg80211 > /dev/null   
if [ $? -eq 0 ]; then
# cfg80211 kernel module is loaded
CFG80211_LOADED="yes"	
fi

case $1 in
	loadbus)
	# Make sure this platform actually has the native MMC stack loaded.
	# Some distributions use mmc_core; others incorporate into mmc_block.
	lsmod | grep mmc_ > /dev/null   
	if [ $? -ne 0 ]; then
		echo "*** Native Linux MMC stack not loaded!"
		exit -1
	fi
	;;
	
	unloadbus)
	# nothing to do for native MMC stack
	;;
	
	loadAR6K)
	if [ "$CFG80211_AVAIL" = "yes" ]; then
   	   if [ "$CFG80211_LOADED" = "no" ]; then
               modprobe -q cfg80211
               if [ $? -ne 0 ]; then
                  echo "*** Failed to install cfg80211 Module"
               fi
   	    fi
   	fi
	echo "loading AR6K module... Args = ($AR6K_MODULE_ARGS) , logfile:$AR6K_TGT_LOGFILE"
	$IMAGEPATH/recEvent --logfile=$AR6K_TGT_LOGFILE --srcdir=$WORKAREA/include/ /dev/null 2>&1 &
	/sbin/insmod $IMAGEPATH/$AR6K_MODULE_NAME.ko $AR6K_MODULE_ARGS
	if [ $? -ne 0 ]; then
		echo "*** Failed to install AR6K Module"
		exit -1
	fi
	;;
	
	unloadAR6K)
	/sbin/rmmod -w $AR6K_MODULE_NAME.ko
 	killall recEvent
	;;
	*)
		echo "Unknown option : $1"
	
esac
