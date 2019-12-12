#ifndef MESH_H
#define MESH_H


#include <QObject>
#include <Eigen/Dense>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include "definesandstructs.h"
#include "enums.h"
#include "glm/glm.hpp"


typedef Eigen::Matrix<float,Eigen::Dynamic,3,Eigen::RowMajor> RowMajMatX3f;
typedef Eigen::Matrix<int  ,Eigen::Dynamic,3,Eigen::RowMajor> RowMajMatX3i;

struct MeshGeom
{
    MeshType     meshType;
    RowMajMatX3f V_orig;
    RowMajMatX3f V;
    RowMajMatX3i F;
    RowMajMatX3f N;
    Eigen::RowVector3f mesh_centre;
};
struct MeshData
{
    MeshType        meshType;
    Eigen::MatrixXf data [DataType::max_data_types];
};
struct MeshBuffers
{
    QOpenGLBuffer *qVBO;
    QOpenGLBuffer *qNBO;
    QOpenGLBuffer *qEBO;
    QOpenGLBuffer *qDataBO     [DataType::max_data_types];
    QOpenGLBuffer *q_mV_rangeBO[DataType::max_data_types];
};


class Mesh : public QObject
{
    Q_OBJECT

public:
    explicit Mesh(MeshType mesh_type_in, QObject *parent = nullptr );

    //Read only access to the mesh geom and data
    const MeshGeom   & getMeshGeom   () const;
    const MeshData   & getMeshData   () const;
    const MeshBuffers& getMeshBuffers() const;

    //Ability to fill the processed data from outside
    void setProcesssedData( Eigen::MatrixXf new_data );

    //Data querying methods
    bool hasNoData        ();
    bool hasData          (DataType data_type);
    bool hasActiHistData  ();
    bool hasProcessedData ();
    bool hasRawData       ();
    bool hasQRSdata       ();
    bool hasAtrialActiData();
    bool hasPhaseData     ();


signals:
    void mesh_buffers_updated_signal( MeshType mesh_type, DataType type_updated );


public slots:
    void transformMesh(const glm::vec3 &cath_translation, const glm::vec3 &cath_rotation, const glm::vec3 &cath_scale );


private:
    //All private functions except for processing class which has knowledge of multiple meshes and can access private methods
    friend class Processing;

    //Mesh geometry and data structs
    MeshGeom    meshGeom;
    MeshData    meshData;
    MeshBuffers meshBuffers;

    //General mesh meshods
    void importMesh( QString file_path, MeshType mesh_type );
    void importData( QString file_path );
    void centreMesh( Eigen::RowVector3f centreOn );

    //Data methods
    void bandstopFilterData              ( const Filter_vars &vars );
    void findQRSactivations              ( const Filter_vars& vars );
    void subtractQRSactivations          ( const Filter_vars& vars );
    void findAtrialActivations           ( const Filter_vars &vars );
    void reconstructSignalFromActivations( const Filter_vars& vars );
    void createActiHistoryMap            ( const Filter_vars &vars );
    void createPhaseMap                  ( const Filter_vars &vars );
    void clearData();
    void clearProcessedData();
    void wipeData();

    void updateMeshGraphicsBuffers();
    void updateDataGraphicsBuffers(DataType data_type);
    void createBuffers();

    //So Processing::forward can fill raw data in goldstandard mode
    void setRawData(Eigen::MatrixXf new_data);
};





Q_DECLARE_METATYPE(MeshType);
Q_DECLARE_METATYPE(DataType);





#endif // MESH_H
