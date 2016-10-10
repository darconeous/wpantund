# File automatically generated by autoandr 0.00.01
# "src/wpantund" - Mon Oct 10 10:26:46 PDT 2016
#

LOCAL_PATH := $(call my-dir)

#### BEGIN wpantund ####
include $(CLEAR_VARS)
LOCAL_MODULE := wpantund
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES += libdbus
LOCAL_STATIC_LIBRARIES += libwpantund-dbus
LOCAL_STATIC_LIBRARIES += libncp-spinel
LOCAL_CFLAGS += -DHAVE_CONFIG_H
LOCAL_CFLAGS += -Wno-date-time
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_CPPFLAGS += -DHAVE_CONFIG_H
LOCAL_CPPFLAGS += -Wno-c++11-narrowing
LOCAL_CPPFLAGS += -Wno-date-time
LOCAL_CPPFLAGS += -Wno-non-virtual-dtor
LOCAL_CPPFLAGS += -Wno-unused-parameter
LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti
LOCAL_C_INCLUDES += $(LOCAL_PATH)/.
LOCAL_C_INCLUDES += $(LOCAL_PATH)/..
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../src/ipc-dbus
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../src/util
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../src/wpantund
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../third_party/assert-macros
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../third_party/fgetln
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../third_party/pt
LOCAL_C_INCLUDES += external/boost
LOCAL_C_INCLUDES += external/dbus
LOCAL_SRC_FILES += ../../../../src/util/Data.cpp
LOCAL_SRC_FILES += ../../../../src/util/EventHandler.cpp
LOCAL_SRC_FILES += ../../../../src/util/IPv6Helpers.cpp
LOCAL_SRC_FILES += ../../../../src/util/IPv6PacketMatcher.cpp
LOCAL_SRC_FILES += ../../../../src/util/SocketAdapter.cpp
LOCAL_SRC_FILES += ../../../../src/util/SocketWrapper.cpp
LOCAL_SRC_FILES += ../../../../src/util/SuperSocket.cpp
LOCAL_SRC_FILES += ../../../../src/util/Timer.cpp
LOCAL_SRC_FILES += ../../../../src/util/TunnelIPv6Interface.cpp
LOCAL_SRC_FILES += ../../../../src/util/UnixSocket.cpp
LOCAL_SRC_FILES += ../../../../src/util/ValueMap.cpp
LOCAL_SRC_FILES += ../../../../src/util/any-to.cpp
LOCAL_SRC_FILES += ../../../../src/util/config-file.c
LOCAL_SRC_FILES += ../../../../src/util/nlpt-select.c
LOCAL_SRC_FILES += ../../../../src/util/sec-random.c
LOCAL_SRC_FILES += ../../../../src/util/socket-utils.c
LOCAL_SRC_FILES += ../../../../src/util/string-utils.c
LOCAL_SRC_FILES += ../../../../src/util/time-utils.c
LOCAL_SRC_FILES += ../../../../src/util/tunnel.c
LOCAL_SRC_FILES += ../../../../src/wpantund/FirmwareUpgrade.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NCPControlInterface.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NCPInstance.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NCPInstanceBase-Addresses.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NCPInstanceBase-AsyncIO.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NCPInstanceBase-NetInterface.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NCPInstanceBase.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NCPTypes.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/NetworkRetain.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/Pcap.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/RunawayResetBackoffManager.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/StatCollector.cpp
LOCAL_SRC_FILES += ../../../../src/wpantund/wpantund.cpp
LOCAL_SRC_FILES += version.c
include $(BUILD_EXECUTABLE)
#### END wpantund ####
