#include "geomwindow.h"
#include <QWheelEvent>
#include <Eigen/Dense>
#include <QPainter>
#include <glm/gtc/type_ptr.hpp>
#include "Helper_Functions/printmatrixinfo.h"
#include "igl/slice_into.h"
#include "igl/sort.h"

#ifdef __APPLE__
    //for my macbook pro.....for some reason this needs to come after the includes
    #define glGenVertexArrays glGenVertexArraysAPPLE
    #define glBindVertexArray glBindVertexArrayAPPLE
#endif


enum AttribPosition
{
    vertexPos, vertexNorm, vertexPotential, channelRange
};


Geomwindow::Geomwindow(QWidget *parent_in) : QOpenGLWidget (parent_in),
    meshList            ( max_num_meshes ),
    time_frame          (0),
    camera_locked       (true),
    max_val             ( 0.01),
    min_val             (-0.01),
    max_act_hist        ( 0.01),
    min_act_hist        (-0.01),
    this_wins_index     (-1),
    channel_range_cutoff(0.3),
    cutoff_use_grey     (false)
{
    std::cout << "Geomwindow --> ctor()" << std::endl;

    connect( this, &Geomwindow::wheel_event_signal  , &camera, &Camera::handleWheelEvent   );
    connect( this, &Geomwindow::mouse_press_signal  , &camera, &Camera::handleMousePress   );
    connect( this, &Geomwindow::mouse_move_signal   , &camera, &Camera::handleMouseMove    );
    connect( this, &Geomwindow::mouse_release_signal, &camera, &Camera::handleMouseRelease );
    connect( &camera, &Camera::camera_updated_signal, this, &Geomwindow::updateLocalSlot   );

    std::cout << "Geomwindow <-- ctor()" << std::endl;
}

Geomwindow::~Geomwindow()
{
    if (qShader == nullptr)
        return;
//    makeCurrent();
//    qVBO.destroy();
//    delete m_program;
//    m_program = 0;
////    doneCurrent();
//    qVBO[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
//    qNBO[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
//    qEBO[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
// qVAO[ mesh_type ][ data_type ] = new QOpenGLVertexArrayObject;
// qDataBO[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
// q_mV_rangeBO[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
// qVAO_pick[ mesh_type ] = new QOpenGLVertexArrayObject;
// qVBO     [ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
// qVBO_pick[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
// qNBO     [ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
// qEBO     [ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
// qEBO_pick[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
// qVAO_pick[ mesh_type ]->create();

}

void Geomwindow::initializeGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.0f, 0.0f, 0.2f, 0.0f              );
    f->glEnable    ( GL_DEPTH_TEST                      );
    f->glEnable    ( GL_COLOR_MATERIAL                  );
    f->glEnable    ( GL_LINE_SMOOTH                     );
    f->glEnable    ( GL_MULTISAMPLE                     );
    f->glEnable    ( GL_POINT_SMOOTH                    );
    f->glEnable    ( GL_BLEND                           );
    f->glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    qShader = new QOpenGLShaderProgram;
    if( qShader->addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/Helper_Functions/Shaders/vertexshader.vert"  ) ){   std::cout << "Geomwindow (Index: " << geomWinTypeStr(this_wins_index) << ") --> initializeGL() --> V_SHADER SUCCESSFUL" << std::endl; } else { std::cout << "Geomwindow (Index: " << this_wins_index << ") --> initializeGL() --> V_SHADER FAILED" << std::endl;  exit(-1); }
    if( qShader->addShaderFromSourceFile( QOpenGLShader::Fragment, ":/Helper_Functions/Shaders/fragmentshader.frag") ){   std::cout << "Geomwindow (Index: " << geomWinTypeStr(this_wins_index) << ") --> initializeGL() --> F_SHADER SUCCESSFUL" << std::endl; } else { std::cout << "Geomwindow (Index: " << this_wins_index << ") --> initializeGL() --> F_SHADER FAILED" << std::endl;  exit(-1); }
    qShader->bindAttributeLocation  ( "vertexPos",       vertexPos       );
    qShader->bindAttributeLocation  ( "vertexNorm",      vertexNorm      );
    qShader->bindAttributeLocation  ( "vertexPotential", vertexPotential );
    qShader->bindAttributeLocation  ( "channelRange",    channelRange    );
    qShader->link();
    qShader->bind();
    qModlMatLoc        = qShader->uniformLocation( "modelMatrix" );
    qViewMatLoc        = qShader->uniformLocation( "viewMatrix" );
    qProjMatLoc        = qShader->uniformLocation( "projectionMatrix" );
    qScaleMaxLoc       = qShader->uniformLocation( "scaleMax_in" );
    qScaleMinLoc       = qShader->uniformLocation( "scaleMin_in" );
    qMinRangeCutoffLoc = qShader->uniformLocation( "minRangeCutoff" );
    qCutoffUseGreyLoc  = qShader->uniformLocation( "cutoff_use_grey" );
    qShader->release();

    qPickShader = new QOpenGLShaderProgram;
    if( qPickShader->addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/Helper_Functions/Shaders/pickvertshader.vert") ){   std::cout << "Geomwindow (Index: " << geomWinTypeStr(this_wins_index) << ") --> initializeGL() --> V_PICKSHADER SUCCESSFUL" << std::endl; } else { std::cout << "Geomwindow (Index: " << this_wins_index << ") --> initializeGL() --> V_PICKSHADER FAILED" << std::endl;  exit(-1); }
    if( qPickShader->addShaderFromSourceFile( QOpenGLShader::Fragment, ":/Helper_Functions/Shaders/pickfragshader.frag") ){   std::cout << "Geomwindow (Index: " << geomWinTypeStr(this_wins_index) << ") --> initializeGL() --> F_PICKSHADER SUCCESSFUL" << std::endl; } else { std::cout << "Geomwindow (Index: " << this_wins_index << ") --> initializeGL() --> F_PICKSHADER FAILED" << std::endl;  exit(-1); }
    qPickShader->bindAttributeLocation( "pickVertexPos", vertexPos );
    qPickShader->link();
    qPickShader->bind();
    qPickModlMatLoc    = qPickShader->uniformLocation( "modelMatrix" );
    qPickViewMatLoc    = qPickShader->uniformLocation( "viewMatrix" );
    qPickProjMatLoc    = qPickShader->uniformLocation( "projectionMatrix" );
    qPickShader->release();
}

void Geomwindow::resizeGL( int w, int h )
{
    camera.setWindowSize( w, h );
}

void Geomwindow::paintGL()
{
    this->makeCurrent();
    QPainter painter(this);
    painter.end(); //turn off
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    ///Reenable stuff QPainter turns off
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f->glEnable    ( GL_DEPTH_TEST                      );
    f->glEnable    ( GL_COLOR_MATERIAL                  );
    f->glEnable    ( GL_LINE_SMOOTH                     );
    f->glEnable    ( GL_MULTISAMPLE                     );
    f->glEnable    ( GL_POINT_SMOOTH                    );
    f->glEnable    ( GL_BLEND                           );
    f->glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ///Native OpenGL rendering
    this->updateDataFrame( f );
    this->drawAtrium     ( f );
    this->drawCatheter   ( f );
    this->drawPickDots   ( f );

    ///Needs this mode to render text properly
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    ///QPainter overpainting
    this->drawPickLabels( &painter );
}

void Geomwindow::drawAtrium( QOpenGLFunctions *f )
{
    if( meshList[ atrium ] ) ///not nullptr
    {
        //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") --> drawAtrium" << std::endl;

        ///Set the shader values
        qShader->bind();
        qShader->setUniformValue( qModlMatLoc , QMatrix4x4( glm::value_ptr(camera.getModelMat()) ).transposed() );
        qShader->setUniformValue( qViewMatLoc , QMatrix4x4( glm::value_ptr(camera.getViewMat() ) ).transposed() );
        qShader->setUniformValue( qProjMatLoc , QMatrix4x4( glm::value_ptr(camera.getProjMat() ) ).transposed() );
        qShader->setUniformValue( qCutoffUseGreyLoc,  cutoff_use_grey );
        qShader->setUniformValue( qMinRangeCutoffLoc, channel_range_cutoff );

        if( geomwinSettings.data_view_atrium == acti_history )
        {
            qShader->setUniformValue( qScaleMaxLoc,   max_act_hist );
            qShader->setUniformValue( qScaleMinLoc,   min_act_hist );
        }else if( geomwinSettings.data_view_atrium == phase )
        {
            qShader->setUniformValue( qScaleMaxLoc,   float( M_PI) );
            qShader->setUniformValue( qScaleMinLoc,   float(-M_PI) );
        }else{
            qShader->setUniformValue( qScaleMaxLoc,   max_val );
            qShader->setUniformValue( qScaleMinLoc,   min_val );
        }

        ///Set style and draw
        if      ( geomwinSettings.mesh_view_atrium == MeshView::wire )
        {
            qVAO[ atrium ][ geomwinSettings.data_view_atrium ]->bind();
                f->glLineWidth(0.05);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //can't call through QOpenGLFunctions
                f->glDrawElements(GL_TRIANGLES, meshList[ atrium ]->getMeshGeom().F.size(), GL_UNSIGNED_INT, 0);
            qVAO[ atrium ][ geomwinSettings.data_view_atrium ]->release();

        }else if( geomwinSettings.mesh_view_atrium == MeshView::points ) {

            qVAO[ atrium ][ geomwinSettings.data_view_atrium ]->bind();
                glPointSize(5.0f); //can't call through QOpenGLFunctions
                f->glDrawElements(GL_POINTS, meshList[ atrium ]->getMeshGeom().F.size(), GL_UNSIGNED_INT, 0);
            qVAO[ atrium ][ geomwinSettings.data_view_atrium ]->release();

        }else if( geomwinSettings.mesh_view_atrium == MeshView::filled ) {

            qVAO[ atrium ][ geomwinSettings.data_view_atrium ]->bind();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //can't call through QOpenGLFunctions
                f->glDrawElements(GL_TRIANGLES, meshList[ atrium ]->getMeshGeom().F.size(), GL_UNSIGNED_INT, 0);
            qVAO[ atrium ][ geomwinSettings.data_view_atrium ]->release();
        }

        qShader->release();

        //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") <-- drawAtrium" << std::endl;
    }
}

void Geomwindow::drawCatheter( QOpenGLFunctions *f )
{
    if( meshList[ catheter ] ) ///not nullptr
    {
        //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") --> drawCatheter" << std::endl;

        ///Set the shader values
        qShader->bind();
        qShader->setUniformValue( qModlMatLoc , QMatrix4x4( glm::value_ptr(camera.getModelMat()) ).transposed() );
        qShader->setUniformValue( qViewMatLoc , QMatrix4x4( glm::value_ptr(camera.getViewMat() ) ).transposed() );
        qShader->setUniformValue( qProjMatLoc , QMatrix4x4( glm::value_ptr(camera.getProjMat() ) ).transposed() );
        qShader->setUniformValue( qScaleMaxLoc,       max_val );
        qShader->setUniformValue( qScaleMinLoc,       min_val );
        qShader->setUniformValue( qMinRangeCutoffLoc, 0.0f );
        qShader->setUniformValue( qCutoffUseGreyLoc,  false  );

        ///Set style and draw
        if      ( geomwinSettings.mesh_view_cath == MeshView::wire )
        {
            qVAO[ catheter ][ geomwinSettings.data_view_cath ]->bind();
                f->glLineWidth(0.05);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //can't call through QOpenGLFunctions
                f->glDrawElements(GL_TRIANGLES, meshList[ catheter ]->getMeshGeom().F.size(), GL_UNSIGNED_INT, 0);
            qVAO[ catheter ][ geomwinSettings.data_view_cath ]->release();

        }else if( geomwinSettings.mesh_view_cath == MeshView::points ) {

            qVAO[ catheter ][ geomwinSettings.data_view_cath ]->bind();
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                f->glDrawElements(GL_TRIANGLES, meshList[ catheter ]->getMeshGeom().F.size(), GL_UNSIGNED_INT, 0);
                glPointSize(10.0f); //can't call through QOpenGLFunctions
                f->glDrawElements(GL_POINTS, meshList[ catheter ]->getMeshGeom().F.size(), GL_UNSIGNED_INT, 0);
            qVAO[ catheter ][ raw ]->release();

        }else if( geomwinSettings.mesh_view_cath == MeshView::filled ) {

            qVAO[ catheter ][ geomwinSettings.data_view_cath ]->bind();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //can't call through QOpenGLFunctions
                f->glDrawElements(GL_TRIANGLES, meshList[ catheter ]->getMeshGeom().F.size(), GL_UNSIGNED_INT, 0);
            qVAO[ catheter ][ geomwinSettings.data_view_cath ]->release();

        }
        qShader->release();

        //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") <-- drawCatheter" << std::endl;
    }
}

void Geomwindow::drawPickDots( QOpenGLFunctions *f )
{
    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") --> drawPickDots" << std::endl;

    for( int mesh_type=0; mesh_type<MeshType::max_num_meshes; mesh_type++ )
    {
        if( meshList[ mesh_type ] ) //not nullptr
        {
            if( pickLabels[mesh_type].size() > 0 )
            {
                //std::cout << "\tGeomwindow (Index: " << this_wins_index << ") --> drawPickLabels()" << std::endl;

                qPickShader->bind();
                qPickShader->setUniformValue( qPickModlMatLoc, QMatrix4x4( glm::value_ptr(camera.getModelMat()) ).transposed() );
                qPickShader->setUniformValue( qPickViewMatLoc, QMatrix4x4( glm::value_ptr(camera.getViewMat() ) ).transposed() );
                qPickShader->setUniformValue( qPickProjMatLoc, QMatrix4x4( glm::value_ptr(camera.getProjMat() ) ).transposed() );

                qVAO_pick[ mesh_type ]->bind();
                glPointSize(15.0);
                f->glDrawElements(GL_POINTS, pickLabels[mesh_type].size(), GL_UNSIGNED_INT, 0);
                qVAO_pick[ mesh_type ]->release();

                qPickShader->release();

                //std::cout << "\tGeomwindow (Index: " << this_wins_index << ") <-- drawPickLabels()" << std::endl;
            }
        }
    }

    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") <-- drawPickDots" << std::endl;
}

void Geomwindow::drawPickLabels(  QPainter *painter  )
{
    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") --> drawPickLabels" << std::endl;

    for( int mesh_type=0; mesh_type<MeshType::max_num_meshes; mesh_type++ )
    {
        if( meshList[ mesh_type ] ) //not nullptr
        {
            if( pickLabels[mesh_type].size() > 0 )
            {
                //std::cout << "\tGeomwindow (Index: " << this_wins_index << ") --> drawPickLabels()" << std::endl;

                for(size_t i=0; i < pickLabels[mesh_type].size(); i++)
                {
                    glm::vec4 pos = pickLabelPos[mesh_type][i];
                    glm::vec4 point_trans = camera.getViewMat() * camera.getModelMat() * pos; //get the camera coordinates
                   /* Convert to screen coordinates */
                    GLfloat x = ((point_trans.x / - point_trans.z) + 0.5) * width();
                    GLfloat y = ((point_trans.y / - point_trans.z) + 0.5) * height();
                    writeText( painter, pickLabels[mesh_type][i], int(x), height()-int(y), 10);
                }
                //std::cout << "\tGeomwindow (Index: " << this_wins_index << ") <-- drawPickLabels()" << std::endl;
            }
        }
    }

    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") <-- drawPickLabels" << std::endl;
}

void Geomwindow::writeText( QPainter *painter, QString text, int x, int y, int size)
{
    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") --> writeText" << std::endl;

    painter->begin(this);
    painter->setPen(Qt::white);
    painter->setFont(QFont("Lato Light", size));
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter->drawText(x, y, text);
    painter->end();

    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") <-- writeText" << std::endl;
}

void Geomwindow::initialise_Q_Buffers(MeshType mesh_type)
{
    std::cout << "Geomwindow (" << geomWinTypeStr(this_wins_index) << ") --> initialise_Q_Buffers()" << std::endl;

    this->makeCurrent();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    ///Pick buffers for this mesh
    qVAO_pick[ mesh_type ] = new QOpenGLVertexArrayObject;
    qVBO_pick[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
    qEBO_pick[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
    qVBO_pick[ mesh_type ]->setUsagePattern( QOpenGLBuffer::DynamicDraw );
    qEBO_pick[ mesh_type ]->setUsagePattern( QOpenGLBuffer::DynamicDraw );
    qVAO_pick[ mesh_type ]->create();
    qVBO_pick[ mesh_type ]->create();
    qEBO_pick[ mesh_type ]->create();

    qVAO_pick[ mesh_type ]->bind();
        qVBO_pick[ mesh_type ]->bind();
        f->glEnableVertexAttribArray( vertexPos );
        f->glVertexAttribPointer    ( vertexPos, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        qEBO_pick[ mesh_type ]->bind();
        qVBO_pick[ mesh_type ]->release();
    qVAO_pick[ mesh_type ]->release();
    ///End init pick buffers for this mesh


    for( int data_type=0; data_type<DataType::max_data_types; data_type++ )
    {
        ///VAO
        qVAO[ mesh_type ][ data_type ] = new QOpenGLVertexArrayObject;
        qVAO[ mesh_type ][ data_type ]->create();
        qVAO[ mesh_type ][ data_type ]->bind();

            ///Vertices
            meshList[ mesh_type ]->getMeshBuffers().qVBO->bind();
            f->glEnableVertexAttribArray( vertexPos       );
            f->glVertexAttribPointer    ( vertexPos, 3, GL_FLOAT, GL_FALSE, 0, 0 );
            meshList[ mesh_type ]->getMeshBuffers().qVBO->release();

            ///Normals
            meshList[ mesh_type ]->getMeshBuffers().qNBO->bind();
            f->glEnableVertexAttribArray( vertexNorm      );
            f->glVertexAttribPointer    ( vertexNorm, 3, GL_FLOAT, GL_FALSE, 0, 0 );
            meshList[ mesh_type ]->getMeshBuffers().qNBO->release();

            ///Indices
            meshList[ mesh_type ]->getMeshBuffers().qEBO->bind();

            ///Potential data
            meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->bind();
            f->glEnableVertexAttribArray ( vertexPotential );
            f->glVertexAttribPointer     ( vertexPotential, 1, GL_FLOAT, GL_FALSE, 0, 0 );
            f->glDisableVertexAttribArray( vertexPotential );
            f->glVertexAttrib1f          ( vertexPotential, 0.00001f );
            meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->release();

            ///Node's potential range info
            meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->bind();
            f->glEnableVertexAttribArray ( channelRange );
            f->glVertexAttribPointer     ( channelRange, 1, GL_FLOAT, GL_FALSE, 0, 0 );
            f->glDisableVertexAttribArray( channelRange );
            f->glVertexAttrib1f          ( channelRange, 0.00001f );
            meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->release();

        ///Unbind the VAO
        qVAO[ mesh_type ][ data_type ]->release();

    }///end init buffers each data type

    std::cout << "Geomwindow (" << geomWinTypeStr(this_wins_index) << ") <-- initialise_Q_Buffers()" << std::endl;
}

void Geomwindow::updateDataFrame( QOpenGLFunctions *f )
{
    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") --> updateDataFrame" << std::endl;

    for( int mesh_type=0; mesh_type<MeshType::max_num_meshes; mesh_type++ )
    {
        if( meshList[ mesh_type ] ) //not nullptr
        {
            for( int data_type=0; data_type<DataType::max_data_types; data_type++ )
            {
                DataType d = static_cast<DataType>(data_type);
                if(  meshList[ mesh_type ]->hasData( d )  )
                {
                    qVAO[ mesh_type ][ data_type ]->bind();
                        meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->bind();
                        f->glEnableVertexAttribArray( vertexPotential );
                        f->glVertexAttribPointer    ( vertexPotential,
                                                      1,
                                                      GL_FLOAT,
                                                      GL_FALSE,
                                                      0,
                                                      (GLvoid*)( time_frame * meshList[ mesh_type ]->getMeshGeom().V.rows() * sizeof(GL_FLOAT) ) );
                        meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->release();//need to unbind for QPainter not to disappear....?

                        meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->bind();
                        f->glEnableVertexAttribArray( channelRange );
                        f->glVertexAttribPointer    ( channelRange,
                                                      1,
                                                      GL_FLOAT,
                                                      GL_FALSE,
                                                      0,
                                                      0             ); ///not sure why but need to reset this to beginning each time....
                        meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->release();//need to unbind for QPainter not to disappear....?

                    qVAO[ mesh_type ][ data_type ]->release();
                }
            }
        }
    }

    //std::cout << "\t Geomwindow (" << geomWinTypeStr(this_wins_index) << ") <-- updateDataFrame" << std::endl;
}

void Geomwindow::setTimeFrame( int time_frame_in )
{
    //std::cout << "Time from mainwindow: " << time_frame_in << std::endl;
    time_frame = time_frame_in;
    this->update();
}

void Geomwindow::setGeomwinMeshPtrs( std::vector<QSharedPointer<Mesh> > &meshList_in )
{
    //Set the pointers lists here and in camera
    meshList        = meshList_in;
    camera.setMeshLists( meshList_in );

    //std::cout << "Geomwindow::meshList: " << meshList[0] << " " << meshList[1] << std::endl;
}

void Geomwindow::updateVAOs( MeshType mesh_type, DataType data_type )
{
    std::cout << "Geomwindow (" << geomWinTypeStr(this_wins_index) << ")(MeshType: " << meshTypeStr(mesh_type) << ")(DataType: " << dataTypeStr(data_type) << ") --> updateVAOs()" << std::endl;

    if( meshList[ mesh_type ] )///not nullptr
    {
        this->makeCurrent();
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

        ///Bind VAO
        qVAO[ mesh_type ][ data_type ]->bind();

        ///Geom stuff------------------------------------------------
        ///Vertices
        meshList[ mesh_type ]->getMeshBuffers().qVBO->bind();
        f->glEnableVertexAttribArray( vertexPos       );
        f->glVertexAttribPointer    ( vertexPos, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        meshList[ mesh_type ]->getMeshBuffers().qVBO->release();

        ///Normals
        meshList[ mesh_type ]->getMeshBuffers().qNBO->bind();
        f->glEnableVertexAttribArray( vertexNorm      );
        f->glVertexAttribPointer    ( vertexNorm, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        meshList[ mesh_type ]->getMeshBuffers().qNBO->release();

        ///Potential data
        meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->bind();
        f->glVertexAttrib1f          ( vertexPotential, 0.00001f );
        meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->release();

        ///Node's potential range info
        meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->bind();
        f->glVertexAttrib1f          ( channelRange, 0.00001f );
        meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->release();

        ///Indices
        meshList[ mesh_type ]->getMeshBuffers().qEBO->bind();


        ///Data stuff
        if( data_type != no_data )
        {
            std::cout << "\tMeshType: " << meshTypeStr(mesh_type) << ")(DataType: " << dataTypeStr(data_type) << ") --> inside data binding" << std::endl;

            ///Bind mesh buffers and enable attributes
            meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->bind();
            f->glEnableVertexAttribArray( vertexPotential );
            f->glVertexAttribPointer    ( vertexPotential, 1, GL_FLOAT, GL_FALSE, 0,
                                          (GLvoid*)( time_frame * meshList[ mesh_type ]->getMeshGeom().V.rows() * sizeof(GL_FLOAT) ) );
            meshList[ mesh_type ]->getMeshBuffers().qDataBO[ data_type ]->release();

            meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->bind();
            f->glEnableVertexAttribArray( channelRange );
            f->glVertexAttribPointer    ( channelRange, 1, GL_FLOAT, GL_FALSE, 0, 0 );
            meshList[ mesh_type ]->getMeshBuffers().q_mV_rangeBO[ data_type ]->release();

            if( data_type == acti_history )
            {
                max_act_hist = meshList[ mesh_type ]->getMeshData().data[data_type].maxCoeff();
                min_act_hist = meshList[ mesh_type ]->getMeshData().data[data_type].minCoeff();
            }
        }

        ///Unbind VAO
        qVAO[ mesh_type ][ data_type ]->release();

        camera.extractMeshInfo();
        this->update();
    }

    std::cout << "Geomwindow (" << geomWinTypeStr(this_wins_index) << ")MeshType: " << meshTypeStr(mesh_type) << ")(DataType: " << dataTypeStr(data_type) << ") <-- updateVAOs()" << std::endl;
}

void Geomwindow::refreshPickData( std::vector<Pickwindow*> &pick_win_list )
{
    //std::cout << "Geomwindow --> refreshPickData()" << std::endl;

    this->makeCurrent();
    pick_win_list_local = pick_win_list;

    if( pick_win_list.size() == 0 )
    {
        for( int mesh_type=0; mesh_type<MeshType::max_num_meshes; mesh_type++ )
        {
            pickLabels  [mesh_type].clear();
            pickLabelPos[mesh_type].clear();

            qVBO_pick[ mesh_type ]->bind();
            qVBO_pick[ mesh_type ]->allocate( nullptr, 0*sizeof(float) );
            qVBO_pick[ mesh_type ]->release();

            qEBO_pick[ mesh_type ]->bind();
            qEBO_pick[ mesh_type ]->allocate( nullptr, 0*sizeof(unsigned) );
            qEBO_pick[ mesh_type ]->release();
        }

    }else{

        size_t num_picks = pick_win_list.size();

        for( int mesh_type=0; mesh_type<MeshType::max_num_meshes; mesh_type++ )
        {
            pickLabels  [mesh_type].clear();
            pickLabelPos[mesh_type].clear();

            Eigen::MatrixXf pick_verts( 3, num_picks );
            Eigen::Matrix<unsigned, 1, Eigen::Dynamic> inds = Eigen::ArrayXf::LinSpaced(num_picks,0,num_picks-1).cast<unsigned>();

            for(size_t i=0; i < num_picks; i++)
            {
                int meshIdx         = pick_win_list[i]->getChannelIndices( static_cast<MeshType>(mesh_type) );
                pick_verts.col( i ) = meshList[mesh_type]->getMeshGeom().V.row( meshIdx ).transpose();

                //Label stuff
                Eigen::Vector3f norm     = meshList[ mesh_type ]->getMeshGeom().N.row(meshIdx).normalized();
                Eigen::Vector3f new_pos;
                if( mesh_type==atrium )
                {
                    new_pos  = pick_verts.col( i )  + ( 2*norm );

                }else if( mesh_type==catheter )
                {
                    new_pos  = pick_verts.col( i )  - ( 2*norm );
                }
                QString text = QString::number( meshIdx + 1 );
                pickLabels[mesh_type].push_back( text );
                glm::vec4 pos4(1.0);
                pos4.x = new_pos(0);
                pos4.y = new_pos(1);
                pos4.z = new_pos(2);
                pickLabelPos[mesh_type].push_back( pos4 );
            }

            qVBO_pick[ mesh_type ]->bind();
            qVBO_pick[ mesh_type ]->allocate( pick_verts.data(), pick_verts.size()*sizeof(float) );
            qVBO_pick[ mesh_type ]->release();

            qEBO_pick[ mesh_type ]->bind();
            qEBO_pick[ mesh_type ]->allocate( inds.data(), inds.size()*sizeof(unsigned) );
            qEBO_pick[ mesh_type ]->release();
        }
    }

    this->update();

    //std::cout << "Geomwindow <-- refreshPickData()" << std::endl;
}

void Geomwindow::updateMinMax( int max, int min )
{
    //std::cout << "Geomwindow::updateMinMax("<<max<<","<<min<<")" << std::endl;
    max_val = max;
    min_val = min;
    this->update();
}

void Geomwindow::connectWindows( Geomwindow *geomwin )
{
    //Connect to the other windows
    connect( geomwin, &Geomwindow::wheel_event_signal  , &camera, &Camera::handleWheelEvent   );
    connect( geomwin, &Geomwindow::mouse_press_signal  , &camera, &Camera::handleMousePress   );
    connect( geomwin, &Geomwindow::mouse_move_signal   , &camera, &Camera::handleMouseMove    );
    connect( geomwin, &Geomwindow::mouse_release_signal, &camera, &Camera::handleMouseRelease );
}

void Geomwindow::updateLocalSlot()
{
    this->update();
}

void Geomwindow::setMeshLock(bool mesh_lock)
{
    camera_locked = mesh_lock;
}

void Geomwindow::wheelEvent(QWheelEvent *event)
{
    if( camera_locked )
    {
        emit wheel_event_signal( event ); //to slot in each window
    }else
    {
        camera.handleWheelEvent( event );
    }
}

void Geomwindow::mousePressEvent(QMouseEvent *event)
{
    if( event->modifiers() & Qt::ControlModifier )
    {
        emit pickingSignal( event, width(), height(), camera.getProjMat(), camera.getViewMat(), camera.getCamPos() );

    } else if( camera_locked )
    {
        emit mouse_press_signal( event ); //to slot in each window
    }else
    {
        camera.handleMousePress( event );
    }
}

void Geomwindow::mouseReleaseEvent(QMouseEvent *event)
{
    if( camera_locked )
    {
        emit mouse_release_signal( event ); //to slot in each window
    }else
    {
        camera.handleMouseRelease( event );
    }

}

void Geomwindow::mouseMoveEvent(QMouseEvent *event)
{
    if( ((event->modifiers() & Qt::ShiftModifier) || (event->modifiers() & Qt::AltModifier)) && pick_win_list_local.size() > 0 )
    {
        //std::cout << "MouseMove Basket Move signal" << std::endl;
        emit basket_moved_update_picks_signal();
    }
    if( camera_locked )
    {
        emit mouse_move_signal( event ); //to slot in each window (connects to Camera::handleMouseMove)
    }else
    {
        camera.handleMouseMove( event ); //just call this geomwindow's camera's functions
    }
}

void Geomwindow::setGeomwinIndex( int index )
{
    this_wins_index = index;
}

void Geomwindow::setLow_mVcutoff( double cutoff )
{
    channel_range_cutoff = float( cutoff );
    this->update();
}

void Geomwindow::setCathScaling ( double scale )
{
    camera.setCathScaling( scale );
}

void Geomwindow::setCutoffDrawStyle ( bool use_grey )
{
    cutoff_use_grey = use_grey;
    this->update();
}





std::string Geomwindow::meshTypeStr( MeshType mesh_type )
{
    switch( mesh_type )
    {
        case atrium:
            return "Atrium";
        case catheter:
            return "Catheter";
        default:
            return "Error mesh name";
    }
}

std::string Geomwindow::dataTypeStr( DataType data_type )
{
    switch( data_type )
    {
        case raw:
            return "Raw";
        case processed:
            return "Processed";
        case phase:
            return "Phase";
        case acti_history:
            return "Acti_history";
        case QRS_actis:
            return "QRS activations";
        case atrial_actis:
            return "Atrial activations";
        case no_data:
            return "No data";
        default:
            return "Error data name";
    }
}

std::string Geomwindow::geomWinTypeStr( int winIdx )
{
    switch( winIdx )
    {
        case 0:
            return "Geomwin1";
        case 1:
            return "Geomwin2";
        case 2:
            return "Geomwin3";
        default:
            return "Error geomwin type";
    }
}



