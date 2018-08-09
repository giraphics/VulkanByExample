#-------------------------------------------------
#
# Project created by QtCreator 2018-03-13T00:46:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtVulkan
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        ../../../source/common/VulkanHelper.cpp \
        ../../../source/common/VulkanApp.cpp \
        source/HelloVulkanApp.cpp

HEADERS += \
        ../../../source/common/VulkanHelper.h \
        ../../../source/common/VulkanApp.h \
        source/HelloVulkanApp.h

macx {
    SOURCES += makeviewmetal.mm
    DEFINES += VK_USE_PLATFORM_MACOS_MVK
    VULKAN_SDK_PATH = /Users/parminder/Dev/Metal/QtMetal/MoltenVK-master/MoltenVK/macOS
    VULKAN_DYLIB = $${VULKAN_SDK_PATH}/libMoltenVK.dylib

    LIBS += $$VULKAN_DYLIB
    LIBS += -framework Cocoa -framework QuartzCore
    # Copy dylib to app bundle
    VULKAN_DATA.path = Contents/Frameworks
    VULKAN_DATA.files = $$VULKAN_DYLIB
    QMAKE_BUNDLE_DATA += VULKAN_DATA

    INCLUDEPATH += /Users/parminder/Dev/Metal/QtMetal/MoltenVK-master/MoltenVK/include
    INCLUDEPATH += ../../../source/external

    # Fix @rpath
    QMAKE_RPATHDIR += @executable_path/../Frameworks

#    QMAKE_POST_LINK += $${VULKAN_SDK_PATH}/macOS/bin/glslangValidator -V $$PWD/shaders/shader.vert;
#    QMAKE_POST_LINK += $${VULKAN_SDK_PATH}/macOS/bin/glslangValidator -V $$PWD/shaders/shader.frag;
#    QMAKE_POST_LINK += $$QMAKE_COPY $$OUT_PWD/vert.spv $$OUT_PWD/$${TARGET}.app/Contents/MacOS;
#    QMAKE_POST_LINK += $$QMAKE_COPY $$OUT_PWD/frag.spv $$OUT_PWD/$${TARGET}.app/Contents/MacOS;
}

win32 {
    DEFINES += VK_USE_PLATFORM_WIN32_KHR
    VULKAN_SDK_PATH = C:\\VulkanSDK\\1.1.70.1
    INCLUDEPATH += $${VULKAN_SDK_PATH}/Include
    LIBS += $${VULKAN_SDK_PATH}/Lib/vulkan-1.lib
}
