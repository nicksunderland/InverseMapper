#ifndef GEOMWINDOW_H
#define GEOMWINDOW_H

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
#include <QWidget>
#include <Eigen/Dense>
#include "mesh.h"
#include "camera.h"
#include "glm/glm.hpp"
#include "DSP_settings/geomwinsettings.h"
#include "pickwindow.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

typedef Eigen::Matrix<float   ,Eigen::Dynamic,3,Eigen::RowMajor> RowMajMatX3f;
typedef Eigen::Matrix<int     ,Eigen::Dynamic,3,Eigen::RowMajor> RowMajMatX3i;


class Geomwindow : public QOpenGLWidget
{
    Q_OBJECT

public:
    Geomwindow(QWidget *parent = nullptr);
    ~Geomwindow();
    void connectWindows(Geomwindow *geomwin);

    ///View settings
    GeomWinSettingsStruct geomwinSettings;

signals:
    void wheel_event_signal  ( QWheelEvent * event );
    void mouse_press_signal  ( QMouseEvent *event  );
    void mouse_move_signal   ( QMouseEvent *event  );
    void mouse_release_signal( QMouseEvent *event  );
    void pickingSignal       ( QMouseEvent *event, int w, int h, glm::mat4 projMat, glm::mat4 viewMat, glm::vec3 cam_pos );
    void basket_moved_update_picks_signal();

public slots:
    ///Setup
    void setGeomwinIndex     (int index);
    void initialise_Q_Buffers( MeshType mesh_type );

    ///Updating graphics
    void updateVAOs        ( MeshType mesh_type, DataType data_type);
    void refreshPickData   ( std::vector<Pickwindow*> &pick_win_list );
    void updateLocalSlot   ( );

    ///Graphics view controls
    void setTimeFrame      ( int time_frame_in );
    void setMeshLock       ( bool mesh_lock );
    void setLow_mVcutoff   (double cutoff);
    void updateMinMax      ( int max, int min );
    void setGeomwinMeshPtrs( std::vector< QSharedPointer<Mesh> >& meshList_in );
    void setCathScaling    (double scale);
    void setCutoffDrawStyle(bool use_grey);

protected:
    void initializeGL()                        override;
    void resizeGL( int w, int h )              override;
    void paintGL()                             override;
    void wheelEvent( QWheelEvent * event )     override;
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event)    override;

private:
    ///Draw functions
    void updateDataFrame(QOpenGLFunctions *f);
    void drawAtrium     (QOpenGLFunctions *f);
    void drawCatheter   (QOpenGLFunctions *f);
    void drawPickLabels (QPainter *painter  );
    void drawPickDots   (QOpenGLFunctions *f);
    void writeText      (QPainter *painter, QString text, int x, int y, int size);

    ///Meshes list (pointers) - if using make sure to check if null first
    std::vector< QSharedPointer<Mesh> > meshList;
    int this_wins_index;

    ///Mesh and data OpenGL objects
    QOpenGLVertexArrayObject *qVAO_pick   [MeshType::max_num_meshes];
    QOpenGLVertexArrayObject *qVAO        [MeshType::max_num_meshes][DataType::max_data_types];
    QOpenGLBuffer            *qVBO_pick   [MeshType::max_num_meshes];
    QOpenGLBuffer            *qEBO_pick   [MeshType::max_num_meshes];
    QOpenGLShaderProgram     *qShader;
    QOpenGLShaderProgram     *qPickShader;
    int qProjMatLoc;
    int qViewMatLoc;
    int qModlMatLoc;
    int qScaleMaxLoc;
    int qScaleMinLoc;
    int qMinRangeCutoffLoc;
    int qCutoffUseGreyLoc;
    int qPickProjMatLoc;
    int qPickViewMatLoc;
    int qPickModlMatLoc;
    int time_frame;
    std::vector< QString   > pickLabels  [MeshType::max_num_meshes];
    std::vector< glm::vec4 > pickLabelPos[MeshType::max_num_meshes];
    std::vector<Pickwindow*> pick_win_list_local;

    ///View stuff
    Camera camera;
    bool camera_locked;
    float max_val;
    float min_val;
    float max_act_hist;
    float min_act_hist;
    float channel_range_cutoff;
    bool  cutoff_use_grey;

    ///My debuggings stuff
    std::string meshTypeStr(MeshType mesh_type);
    std::string dataTypeStr(DataType data_type);
    std::string geomWinTypeStr(int winIdx);
};








#endif // GEOMWINDOW_H
