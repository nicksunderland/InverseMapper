#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStyleFactory>

int main(int argc, char *argv[])
{

    ///This is key, to enable all windows to control displays in other windows - see https://forum.qt.io/topic/63771/multiple-contexts-using-qopenglwidget/6
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    ///The application object
    QApplication a(argc, argv);

    ///Set global OpenGL vstuff
    QSurfaceFormat format;
    format.setSamples(4);
    format.setDepthBufferSize(24);
    format.setVersion(4,5);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    QSurfaceFormat::setDefaultFormat(format);

    ///Launch the mainwindow
    MainWindow w;
    w.show();



    return a.exec();
}
