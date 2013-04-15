LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := QCAR-prebuilt
LOCAL_SRC_FILES = ../../../vuforia-sdk-android-2-0-30/build/lib/$(TARGET_ARCH_ABI)/libQCAR.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../vuforia-sdk-android-2-0-30/build/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := feeler

OPENGLES_LIB  := -lGLESv1_CM
OPENGLES_DEF  := -DUSE_OPENGL_ES_1_1

LOCAL_CFLAGS := -Wno-write-strings -Wno-psabi $(OPENGLES_DEF)

LOCAL_LDLIBS := -llog $(OPENGLES_LIB) 

LOCAL_SHARED_LIBRARIES := QCAR-prebuilt

LOCAL_SRC_FILES := WheelerFeeler.cpp SampleUtils.cpp ModelLoader.cpp

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
