MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# Fusion
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := Fusion

LOCAL_SRC_FILES := \
	Fusion/Core/Android/CoreAndroid.cpp \
	Fusion/Core/Android/NativeJNI.cpp \
	Fusion/Core/Animator.cpp \
	Fusion/Core/Core.cpp \
	Fusion/Core/Event.cpp \
	Fusion/Core/HID.cpp \
	Fusion/Core/Key.cpp \
	Fusion/Core/Pointer.cpp \
	Fusion/Drawable/Drawable.cpp \
	Fusion/Drawable/Text.cpp \
	Fusion/Drawable/TQuad.cpp \
	Fusion/Fusion.cpp \
	Fusion/Resource/Bitmap.cpp \
	Fusion/Resource/BitmapManipulator.cpp \
	Fusion/Resource/Font.cpp \
	Fusion/Resource/Image.cpp \
	Fusion/Resource/ImageGenerator.cpp \
	Fusion/Resource/RectPacker.cpp \
	Fusion/Resource/ResCache.cpp \
	Fusion/Resource/ResManager.cpp \
	Fusion/Resource/Resource.cpp \
	Fusion/Resource/stb_image.c \
	Fusion/VM/BaseProxies.cpp \
	Fusion/VM/VM.cpp \
	Fusion/VM/VMMath.cpp \
	Fusion/VM/VMObjectPool.cpp \
	Fusion/VM/VMRegistry.cpp \
	Fusion/VM/VMSubject.cpp \
	Fusion/Widget/Box.cpp \
	Fusion/Widget/Button.cpp \
	Fusion/Widget/Canvas.cpp \
	Fusion/Widget/ComboBox.cpp \
	Fusion/Widget/Desktop.cpp \
	Fusion/Widget/Grid.cpp \
	Fusion/Widget/HotspotContainer.cpp \
	Fusion/Widget/Label.cpp \
	Fusion/Widget/Layer.cpp \
	Fusion/Widget/Menu.cpp \
	Fusion/Widget/MenuItem.cpp \
	Fusion/Widget/RadialButton.cpp \
	Fusion/Widget/RadialMenu.cpp \
	Fusion/Widget/Spacer.cpp \
	Fusion/Widget/Splitter.cpp \
	Fusion/Widget/TextEntry.cpp \
	Fusion/Widget/TreeList.cpp \
	Fusion/Widget/ValueEditor.cpp \
	Fusion/Widget/Widget.cpp \
	Fusion/Widget/WidgetContainer.cpp \

include $(BUILD_STATIC_LIBRARY)
