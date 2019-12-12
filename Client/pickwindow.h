#ifndef PICKWINDOW_H
#define PICKWINDOW_H

//#define GLEW_STATIC
//#include <GL/glew.h>
#ifdef __linux__
    #include "GL/glu.h"
    #include "GL/gl.h"
#endif
#ifdef __APPLE__
    #include <OpenGL/glu.h>
    #include <OpenGL/gl.h>
#endif

#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include "glm/glm.hpp"
#include <QWidget>
#include <Eigen/Dense>
#include "mesh.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)


class Pickwindow : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit Pickwindow(PickWinType pickwin_type_in, int chan_idx[], std::vector< QSharedPointer<Mesh> >& meshList_in, QWidget *parent = nullptr);
    ~Pickwindow();

public slots:
    void updateTimeFrame           (int time);
    void updatePickDataBuffers     (MeshType mesh_type, DataType data_type );
    void setChannelIndices         (int channel[]);
    void changeNearRawElectrodeBool(bool checked);
    void changeNearProElectrodeBool(bool checked);
    void changeAtrialRawBool       (bool checked);
    void changeAtrialProBool       (bool checked);
    void changeShowActisBool       (bool checked);
    void changeShowPhaseBool       (bool checked);
    int  getChannelIndices         (MeshType mesh_type);
    void updateTransformMatrixUSER (float max_y, float min_y, bool use_user_scaling);

protected:
    void initializeGL()                        override;
    void resizeGL( int w, int h )              override;
    void paintGL()                             override;

private:
    //General
    PickWinType pickwin_type;
    int channel_index[MeshType::max_num_meshes];
    int time_frame;

    //QOpenGL objects
    QOpenGLVertexArrayObject *qVAO   [MeshType::max_num_meshes][DataType::max_data_types];
    QOpenGLBuffer            *qVBdata[MeshType::max_num_meshes][DataType::max_data_types];
    QOpenGLBuffer            *qEBO   [MeshType::max_num_meshes][DataType::max_data_types];;
    QOpenGLVertexArrayObject *timerVAO;
    QOpenGLBuffer            *timerVBO;
    QOpenGLBuffer            *timerEBO;
    QOpenGLShaderProgram     *qPickWinShader;
    int qMinRangeCutoffLoc;
    int qPickWinTransMatLoc;
    int qPickWinLineColorLoc;

    //Transform matrix
    glm::mat4 transMatrix;
    bool user_scale;
    size_t num_time_pts;
    size_t num_actis[MeshType::max_num_meshes][DataType::max_data_types];
    size_t num_qrs;
    float  max_val;
    float  min_val;

    //Draw switches
    bool qrs_data_available;
    bool draw_ticker_bool;
    bool draw_closest_electrode_raw;
    bool draw_closest_electrode_pro;
    bool draw_atrial_raw;
    bool draw_atrial_pro;
    bool show_activations;
    bool draw_phase;

    ///Draw methods
    void initialise_Q_Buffers ();
    void fillDataBuffers      ();
    void updateTransformMatrix(float max_y, float min_y, float min_x, float max_x);
    void drawAtrial           (QOpenGLFunctions *f);
    void drawRaw              (QOpenGLFunctions *f);
    void drawFiltered         (QOpenGLFunctions *f);
    void drawTicker           (QOpenGLFunctions *f);
    void drawQRSactivations   (QOpenGLFunctions *f);
    void drawAtrialActivations(QOpenGLFunctions *f, MeshType mesh_type, DataType data_type);
    void writeText            (QString text, int x, int y, int size, QString color);
    void drawScaleBar         ();
    void drawInfoText         (PickWinType pickType);


    //Meshes list (pointers) - if using make sure to check if null first
    std::vector< QSharedPointer<Mesh> > meshList;





    ///My debugging stuff
    std::string meshTypeStr(MeshType mesh_type);
    std::string dataTypeStr(DataType data_type);
    std::string pickWinTypeStr(PickWinType pick_win);
};

#endif // PICKWINDOW_H
