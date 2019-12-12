#ifndef PROCESSING_H
#define PROCESSING_H

#include <QWidget>
#include "definesandstructs.h"
#include "mesh.h"
#include "pickwindow.h"
#include <QSharedPointer>


class Processing : public QWidget
{
    Q_OBJECT
public:
    explicit Processing(QWidget *parent = nullptr);

signals:
    void new_mesh_added_signal( MeshType mesh_type );
    void new_pickwindown_added_signal( Pickwindow* pickwin );
    void pick_windows_changed_v2( std::vector<Pickwindow*> &pick_win_list );
    void update_meshLists( std::vector< QSharedPointer<Mesh> >& meshList );


public slots:
    void loadMesh( QString file_path, MeshType mesh_type );
    void loadData( QString file_path, MeshType mesh_type );
    void processPick(QMouseEvent *event, int width, int height, glm::mat4 projMat, glm::mat4 viewMat, glm::vec3 cam_pos);
    void updatePickIndices();
    Eigen::VectorXi findNearestNodes(Eigen::VectorXi these_node_idxs, MeshType this_mesh_type, MeshType comparison_mesh_type);



    void noiseFilter();
    void findQRS();
    void subtractQRS();
    void findCathActivations();
    void reconstructSignal();
    void runInverse();
    void findEndoActivations();
    void createEndoActivationMap();
    void createPhaseMap();
    void clearFiltering();
    void clearAllData();



private:
    General_vars general_vars;
    Filter_vars  filter_vars;
    Inverse_vars inverse_vars;
    std::vector< QSharedPointer<Mesh> > meshList;
    std::vector< Pickwindow*          > pick_win_list;



    friend class MainWindow;
    friend class SignalProcessingDialog;
    void centreCatheter();
    void inverse( QSharedPointer<Mesh> cath, QSharedPointer<Mesh> atrium, const Inverse_vars& vars );
    void generateSources( const Eigen::MatrixXf &V, const Eigen::MatrixXi &F, const Eigen::MatrixXf &N,float downsample, SourceGen inf_or_def, SourceGen method, float source_elevation, Eigen::MatrixXf &sources, Eigen::VectorXi &inds ); //in the inverse header
    void forward( QSharedPointer<Mesh> cath, QSharedPointer<Mesh> atrium, const Inverse_vars& vars );

};



#include <QThread>
class WorkerThread : public QThread
{
    Q_OBJECT

public:
    WorkerThread(Processing& processingRef) :
        processing(processingRef)
    {
        //connect(this, &WorkerThread::inverse_finished, &processing, &Processing::updateDataGraphics);
    }

    void run() override
    {
//        processing.runInverse();
//        emit inverse_finished();
    }

signals:
    void inverse_finished();

private:
    Processing& processing;
};


#endif // PROCESSING_H
