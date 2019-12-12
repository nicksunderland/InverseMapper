#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <Eigen/Dense>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "Helper_Functions/printmatrixinfo.h"

Camera::Camera(QObject *parent) : QObject(parent),
    meshList( max_num_meshes ),
    width(200),
    height(200),
    max_mesh_dimension(0.5),
    zoom(45.0),

    //View matrices
    cath_translation (0.0, 0.0, 0.0),
    cath_rotation    (0.0, 0.0, 0.0),
    cath_scale       (1.0, 1.0, 1.0),
    viewMat_inv  (glm::mat4(1.0)),
    modelMat     (glm::mat4(1.0)),
    viewMat      (glm::mat4(1.0)),
    projMat      (glm::mat4(1.0)),
    cam_target   (glm::vec3(0.0)),
    cam_up       (0.0, 1.0, 0.0),
    cam_pos      (glm::vec3(0.0,0.0,-1)),
    cam_rot      (glm::vec3(0.0))
{


}

void Camera::updateMVP()
{
    //Model matrix
    glm::mat4 transMat = glm::translate( glm::mat4(1.0), glm::vec3(0.0) );
    glm::mat4 scaleMat = glm::scale    ( glm::mat4(1.0), glm::vec3(1.0) );
    glm::mat4 rotatMat = glm::rotate   (     0.0f      , glm::vec3(1.0) );
    modelMat = transMat * rotatMat * scaleMat;

    //View matrix
    float adjust = max_mesh_dimension * 1.2f;
    viewMat = glm::lookAt( glm::vec3( cam_pos.x,    cam_pos.y,    cam_pos.z    )*adjust,
                           glm::vec3( cam_target.x, cam_target.y, cam_target.z ),
                           glm::vec3( cam_up.x,     cam_up.y,     cam_up.z     )    );

    //Move the
    glm::mat4 camRotX   = glm::rotate( cam_rot.y, glm::vec3(1.0,0.0,0.0) );
    glm::mat4 camRotY   = glm::rotate( cam_rot.x, glm::vec3(0.0,1.0,0.0) );
    glm::mat4 camRotMat = camRotX *camRotY;
    viewMat = viewMat * camRotMat;

    //Projection matrix
    projMat = glm::perspective( glm::radians( zoom ),
                                (float(width) / float(height)),
                                0.1f,
                                adjust *2 );

    emit camera_updated_signal();
}

void Camera::setWindowSize(int w, int h)
{
    width  = w;
    height = h;

    this->updateMVP();
}

void Camera::handleWheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps   = numDegrees / 15;

    zoom += numSteps;

    this->updateMVP();
}

void Camera::handleMousePress( QMouseEvent *event )
{        
    LastPos = event->pos();

    /* Store last inverse matrix so don't have to calculate multiple times */
    viewMat_inv = glm::inverse(viewMat);

    /* 3D normalised x and y device coordinates, techincally don't need a z coordinate here */
    float x_ndc = ((2.0f*LastPos.x()) / width) - 1.0f;
    float y_ndc = 1.0f - (2.0f*LastPos.y()) / height;
    /* Homogenous space */
    glm::vec4 screenPosLast = glm::vec4(x_ndc, y_ndc, 0.0, 1.0f);
    /* Projection/Eye space */
    worldPosLast = viewMat_inv * screenPosLast;
}

void Camera::handleMouseMove( QMouseEvent *event )
{
    if( event->modifiers() & Qt::ControlModifier  )
    {
        return; //stop flickering during picking
    }
    else if( event->modifiers() & Qt::ShiftModifier )
    {
       /* Get the mouse pixel coordinates from the screen click */
        float mouse_x = event->x();
        float mouse_y = event->y();

       /* 3D normalised x and y device coordinates, techincally don't need a z coordinate here */
        float x_ndc = ((2.0f*mouse_x) / width) - 1.0f;
        float y_ndc = 1.0f - (2.0f*mouse_y) / height;

       /* Homogenous space */
        glm::vec4 screenPos = glm::vec4(x_ndc, y_ndc, 0.0, 1.0f);

       /* World space */
        glm::vec4 worldPos = viewMat_inv * screenPos;

        cath_translation.x += (worldPos.x - worldPosLast.x)*2;
        cath_translation.y += (worldPos.y - worldPosLast.y)*2;
        cath_translation.z += (worldPos.z - worldPosLast.z)*2;

        if( meshList[ catheter] )
        {
            meshList[ catheter]->transformMesh( cath_translation, cath_rotation, cath_scale );
        }

        this->updateMVP();

    }
    else if( event->modifiers() & Qt::AltModifier )
    {
        float delta_x = event->x() - LastPos.x();
        float delta_y = event->y() - LastPos.y();

        cath_rotation.x +=  delta_x * 0.01;
        cath_rotation.y += -delta_y * 0.01;

        LastPos = event->pos();

         if( meshList[ catheter] )
         {
             meshList[ catheter]->transformMesh( cath_translation, cath_rotation, cath_scale );
         }

         this->updateMVP();

    }else{

        float delta_x = event->x() - LastPos.x();
        float delta_y = event->y() - LastPos.y();

        cam_rot.x +=  delta_x * 0.01;
        cam_rot.y += -delta_y * 0.01;

        LastPos = event->pos();

        this->updateMVP();
    }
}

void Camera::handleMouseRelease(QMouseEvent *event)
{


}

void Camera::setMeshLists( std::vector< QSharedPointer<Mesh> >& meshList_in )
{
    //std::cout << "Camera::setMeshInfo()" << std::endl;
    meshList = meshList_in;
    //std::cout << "Camera::setMeshInfo() exit" << std::endl;
}

void Camera::extractMeshInfo()
{
    for( size_t mesh_type=0; mesh_type<meshList.size(); mesh_type++ )
    {
    if( meshList[ mesh_type ] ) //not nullptr
    {
        Eigen::Vector3f ranges = meshList[ mesh_type ]->getMeshGeom().V.colwise().maxCoeff()
                                 -
                                 meshList[ mesh_type ]->getMeshGeom().V.colwise().minCoeff();

        if( ranges.maxCoeff() > max_mesh_dimension ) //can be called by each mesh, so make sure we only update if this mesh's range is bigger
        {
            max_mesh_dimension = ranges.maxCoeff();
        }
        this->updateMVP();
    }
    }
}

const glm::vec3 &Camera::getCathRot()
{
    return cath_rotation;
}

const glm::vec3 &Camera::getCathTrans()
{
    return cath_translation;
}

const glm::vec3 &Camera::getCathScale()
{
    return cath_scale;
}

const glm::mat4 &Camera::getModelMat()
{
    return modelMat;
}

const glm::mat4 &Camera::getViewMat()
{
    return viewMat;
}

const glm::mat4 &Camera::getProjMat()
{
    return projMat;
}

const glm::vec3 &Camera::getCamPos()
{
    return cam_pos;
}

void Camera::setCathScaling( double scale )
{
    cath_scale = glm::vec3( (float(scale)) );

    if( meshList[ catheter] )
    {
        meshList[ catheter]->transformMesh( cath_translation, cath_rotation, cath_scale );
    }
}

