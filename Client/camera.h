#ifndef CAMERA_H
#define CAMERA_H


#include <QObject>
#include <QWheelEvent>
#include <QSharedPointer>
#include "mesh.h"
#include "definesandstructs.h"
#include "glm/glm.hpp"


class Camera : public QObject
{
    Q_OBJECT
public:
    explicit Camera(QObject *parent = nullptr);


    const glm::vec3& getCathRot();
    const glm::vec3& getCathTrans();
    const glm::vec3& getCathScale();
    const glm::mat4& getModelMat();
    const glm::mat4& getViewMat();
    const glm::mat4& getProjMat();
    const glm::vec3& getCamPos();

    void setCathScaling(double scale);
    void setWindowSize ( int w, int h );
    void updateMVP     ( );


public slots:
    void handleWheelEvent  (QWheelEvent *event);
    void handleMousePress  (QMouseEvent *event);
    void handleMouseMove   (QMouseEvent *event);
    void handleMouseRelease(QMouseEvent *event);
    void setMeshLists      (std::vector<QSharedPointer<Mesh> > &meshList_in);
    void extractMeshInfo   ( );


signals:
    void camera_updated_signal();


private:
    //Window info
    int width;
    int height;

    //Other
    float zoom;

    //Mesh info
    float max_mesh_dimension;

    //View matrices
    glm::mat4 modelMat;
    glm::mat4 viewMat;
    glm::mat4 viewMat_inv;
    glm::mat4 projMat;

    //Movement
    glm::vec4 worldPosLast;
    glm::vec3 cam_target;
    glm::vec3 cam_up;
    glm::vec3 cam_pos;
    glm::vec3 cam_rot;
    QPoint LastPos;

    glm::vec3 cath_translation;
    glm::vec3 cath_rotation;
    glm::vec3 cath_scale;

    //Mesh info
    std::vector< QSharedPointer<Mesh> > meshList;


};

#endif // CAMERA_H
