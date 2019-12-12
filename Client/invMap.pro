#-------------------------------------------------
#
# Project created by QtCreator 2019-08-31T14:41:24
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = invMap
TEMPLATE = app

# To stop the mkspec overwritting the complier paths, set here for mac clang (use GCC in Linux)
# https://stackoverflow.com/questions/48179370/qt-creator-compiler-in-kit-for-project-is-being-ignored
macx:
{
    QMAKE_CC = /usr/local/Cellar/llvm/8.0.1/bin/clang
    QMAKE_CXX = /usr/local/Cellar/llvm/8.0.1/bin/clang++
 # Multithreading stuff - apple's clang doesnt support openmp so needed to download latest llvm via Homebrew(above)
 # and use this compiler flag
   QMAKE_CXXFLAGS += -fopenmp=libomp
}

#LINUX
#QMAKE_CXXFLAGS += -fopenmp
#LIBS += -fopenmp



CONFIG += c++11

SOURCES += \
    DSP_settings/dsp_inversesettings.cpp \
    DSP_settings/dsp_phasesettings.cpp \
        geomwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        mesh.cpp \
        processing.cpp \
    camera.cpp \
    signalprocessingdialog.cpp \
    DSP_settings/dsp_atrialactivationfinder.cpp \
    DSP_settings/dsp_noisesettings.cpp \
    DSP_settings/dsp_qrsfinder.cpp \
    DSP_settings/dsp_qrssubtraction.cpp \
    DSP_settings/dsp_qrssubtractionweightings.cpp \
    pickwindow.cpp \
    DSP_settings/geomwinsettings.cpp \
    DSP_settings/dsp_signalreconstructor.cpp \
    DSP_settings/dsp_signalreconstructweightings.cpp \
    DSP_settings/dsp_activationhistorysettings.cpp

HEADERS += \
    DSP_settings/dsp_inversesettings.h \
    DSP_settings/dsp_phasesettings.h \
        mesh.h \
        Helper_Functions/readdatafomtextfile.h \
        definesandstructs.h \
        enums.h \
        geomwindow.h \
        mainwindow.h \
        processing.h \
    Helper_Functions/printmatrixinfo.h \
    camera.h \
    signalprocessingdialog.h \
    DSP_settings/dsp_atrialactivationfinder.h \
    DSP_settings/dsp_noisesettings.h \
    DSP_settings/dsp_qrsfinder.h \
    DSP_settings/dsp_qrssubtraction.h \
    DSP_settings/dsp_qrssubtractionweightings.h \
    pickwindow.h \
    Helper_Functions/atrialactivationfinder.h \
    Helper_Functions/atrialsignalreconstruct.h \
    Helper_Functions/bandstopfilter.h \
    Helper_Functions/createactivationhistorymap.h \
    Helper_Functions/creso.h \
    Helper_Functions/cwtfunctions.h \
    Helper_Functions/printmatrixinfo.h \
    Helper_Functions/qrspeakfinder.h \
    Helper_Functions/qrssubtraction.h \
    Helper_Functions/readdatafomtextfile.h \
    Helper_Functions/tikhonovregularisation.h \
    Helper_Functions/tukeywindowmatrix.h \
    Helper_Functions/inverse.h \
    DSP_settings/geomwinsettings.h \
    DSP_settings/dsp_signalreconstructor.h \
    DSP_settings/dsp_signalreconstructweightings.h \
    DSP_settings/dsp_activationhistorysettings.h \
    Helper_Functions/forward.h \
    Helper_Functions/phasemap.h

FORMS += \
    DSP_settings/dsp_inversesettings.ui \
    DSP_settings/dsp_phasesettings.ui \
        mainwindow.ui \
    signalprocessingdialog.ui \
    DSP_settings/dsp_atrialactivationfinder.ui \
    DSP_settings/dsp_noisesettings.ui \
    DSP_settings/dsp_qrsfinder.ui \
    DSP_settings/dsp_qrssubtraction.ui \
    DSP_settings/dsp_qrssubtractionweightings.ui \
    DSP_settings/geomwinsettings.ui \
    DSP_settings/dsp_signalreconstructor.ui \
    DSP_settings/dsp_signalreconstructweightings.ui \
    DSP_settings/dsp_activationhistorysettings.ui

DISTFILES +=

RESOURCES += \
    Resources/themes/breeze.qrc \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target




### IGL ###
INCLUDEPATH += $$PWD/../Libraries/IGL_Geometry_Library/include

### GLM ###
INCLUDEPATH += $$PWD/../Libraries/GLM_GraphicsGeom_Library/0.9.9.5/include

### EIGEN ###
INCLUDEPATH += $$PWD/../Libraries/Eigen_Numerical_Library/eigen
INCLUDEPATH += $$PWD/../Libraries/Eigen_Numerical_Library/unsupported


### WAVELIB LIBRARY ###
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libraries/Wavelib_Library/Build/Bin/release/ -lwavelib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libraries/Wavelib_Library/Build/Bin/debug/ -lwavelib
else:unix: LIBS += -L$$PWD/../Libraries/Wavelib_Library/Build/Bin/ -lwavelib
INCLUDEPATH += $$PWD/../Libraries/Wavelib_Library/header
DEPENDPATH += $$PWD/../Libraries/Wavelib_Library/header
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Wavelib_Library/Build/Bin/release/libwavelib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Wavelib_Library/Build/Bin/debug/libwavelib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Wavelib_Library/Build/Bin/release/wavelib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Wavelib_Library/Build/Bin/debug/wavelib.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libraries/Wavelib_Library/Build/Bin/libwavelib.a


### FFTW LIBRARY ###
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/release/ -lfftw3f -lfftw3f_omp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/debug/ -lfftw3f -lfftw3f_omp
else:unix: LIBS += -L$$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/ -lfftw3f -lfftw3f_omp
INCLUDEPATH += $$PWD/../Libraries/FFTW_Library/3.3.8_1/include
DEPENDPATH += $$PWD/../Libraries/FFTW_Library/3.3.8_1/include
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/release/libfftw3f.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/debug/libfftw3f.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/release/fftw3f.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/debug/fftw3f.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/libfftw3f.a
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/release/libfftw3f_omp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/debug/libfftw3f_omp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/release/fftw3f_omp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/debug/fftw3f_omp.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/libfftw3f_omp.a


### DLIB ###
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/release/ -ldlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/debug/ -ldlib
else:unix: LIBS += -L$$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/ -ldlib
INCLUDEPATH += $$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17
DEPENDPATH += $$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/release/libdlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/debug/libdlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/release/dlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/debug/dlib.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libraries/Dlib_Numerical_Library/dlib-19.17/examples/build/dlib_build/libdlib.a


### DSP FILTER LIBRARY ###
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libraries/DSPFilters_Library/Build/release/ -lDSPFilters
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libraries/DSPFilters_Library/Build/debug/ -lDSPFilters
else:unix: LIBS += -L$$PWD/../Libraries/DSPFilters_Library/Build/ -lDSPFilters
INCLUDEPATH += $$PWD/../Libraries/DSPFilters_Library/include
DEPENDPATH += $$PWD/../Libraries/DSPFilters_Library/include
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/DSPFilters_Library/Build/release/libDSPFilters.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/DSPFilters_Library/Build/debug/libDSPFilters.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/DSPFilters_Library/Build/release/DSPFilters.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/DSPFilters_Library/Build/debug/DSPFilters.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libraries/DSPFilters_Library/Build/libDSPFilters.a

### GLEW ###
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/release/ -lGLEW
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/debug/ -lGLEW
else:unix: LIBS += -L$$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/ -lGLEW
INCLUDEPATH += $$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/include
DEPENDPATH += $$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/include
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/release/libGLEW.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/debug/libGLEW.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/release/GLEW.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/debug/GLEW.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libraries/GLEW_Graphics_Library/2.1.0/lib/libGLEW.a


### OMP ###
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libraries/libomp/8.0.1/lib/release/ -lomp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libraries/libomp/8.0.1/lib/debug/ -lomp
else:unix: LIBS += -L$$PWD/../Libraries/libomp/8.0.1/lib/ -lomp
INCLUDEPATH += $$PWD/../Libraries/libomp/8.0.1/include
DEPENDPATH += $$PWD/../Libraries/libomp/8.0.1/include
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/libomp/8.0.1/lib/release/libomp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/libomp/8.0.1/lib/debug/libomp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/libomp/8.0.1/lib/release/omp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../Libraries/libomp/8.0.1/lib/debug/omp.lib
else:unix: PRE_TARGETDEPS += $$PWD/../Libraries/libomp/8.0.1/lib/libomp.a





















# #WORK



##### IGL ###
#INCLUDEPATH += $$PWD/../Libraries/IGL_Geometry_Library/include

##### GLM ###
#INCLUDEPATH += $$PWD/../Libraries/GLM_GraphicsGeom_Library/0.9.9.5/include

##### EIGEN ###
#INCLUDEPATH += $$PWD/../Libraries/Eigen_Numerical_Library/eigen
#INCLUDEPATH += $$PWD/../Libraries/Eigen_Numerical_Library/unsupported

##### WAVELIB LIBRARY ###
#LIBS           += -L$$PWD/../Libraries/Wavelib_Library/Build/Bin/ -lwavelib
#INCLUDEPATH    += $$PWD/../Libraries/Wavelib_Library/header
#DEPENDPATH     += $$PWD/../Libraries/Wavelib_Library/header
#PRE_TARGETDEPS += $$PWD/../Libraries/Wavelib_Library/Build/Bin/libwavelib.a

##### FFTW LIBRARY ###
#LIBS           += -L$$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/ -lfftw3f -lfftw3f_omp
#INCLUDEPATH    += $$PWD/../Libraries/FFTW_Library/3.3.8_1/include
#DEPENDPATH     += $$PWD/../Libraries/FFTW_Library/3.3.8_1/include
#PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/libfftw3f.a
#PRE_TARGETDEPS += $$PWD/../Libraries/FFTW_Library/3.3.8_1/lib/libfftw3f_omp.a

##### DLIB ###
#LIBS           += -L$$PWD/../Libraries/Dlib_Numerical_Library/examples/build/dlib_build/ -ldlib
#INCLUDEPATH    += $$PWD/../Libraries/Dlib_Numerical_Library
#DEPENDPATH     += $$PWD/../Libraries/Dlib_Numerical_Library
#PRE_TARGETDEPS += $$PWD/../Libraries/Dlib_Numerical_Library/examples/build/dlib_build/libdlib.a


##### DSP FILTER LIBRARY ###
#LIBS           += -L$$PWD/../Libraries/DSPFilters_Library/Build/ -lDSPFilters
#INCLUDEPATH    += $$PWD/../Libraries/DSPFilters_Library/include
#DEPENDPATH     += $$PWD/../Libraries/DSPFilters_Library/include
#PRE_TARGETDEPS += $$PWD/../Libraries/DSPFilters_Library/Build/libDSPFilters.a






