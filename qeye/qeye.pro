TEMPLATE = "app"
LANGUAGE = "C++"

QT = "core" "gui"

# CONFIG += "qt" "release" "warn_off" "thread" "windows"
CONFIG += "qt" "debug" "warn_off" "thread" "console"
CONFIG += "uic" "resources"

win32 {
    DEFINES = "WIN32"
    INCLUDEPATH = "$(QTDIR)/include" "include"
}
unix {
    DEFINES = "_POSIX_THREADS" "_POSIX_THREAD_SAFE_FUNCTIONS"
    DEFINES += "_REENTRANT"
    INCLUDEPATH = "$(QTDIR)/include" "include"
    LIBS = "-lpthread" "-lrt"
}
INCLUDEPATH += "tmp_src"

# RESOURCES = "rc/chess.qrc"
FORMS = "ui/main_window.ui"
SOURCES = "src/main.cpp"
DESTDIR = "dist"
UI_DIR = "tmp_src"
MOC_DIR = "tmp_src"
# RCC_DIR = "tmp_src"
OBJECTS_DIR = "objs"
