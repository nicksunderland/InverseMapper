#include "pickwindow.h"
#include <QPainter>
#include <glm/gtc/matrix_transform.hpp>
#include "Helper_Functions/printmatrixinfo.h"
#include "igl/slice.h"
#include "igl/slice_mask.h"
#include <glm/gtc/type_ptr.hpp>
#include <QFontDatabase>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#ifdef __APPLE__
    //for my macbook pro.....for some reason this needs to come after the includes
    #define glGenVertexArrays glGenVertexArraysAPPLE
    #define glBindVertexArray glBindVertexArrayAPPLE
#endif


enum AttribPositionPickWin
{
    vertexPosPick
};

Pickwindow::Pickwindow(PickWinType pickwin_type_in, int chan_idx[], std::vector<QSharedPointer<Mesh> > &meshList_in, QWidget *parent) :
    pickwin_type(pickwin_type_in),
    channel_index{*chan_idx, *(chan_idx+1)},
    meshList(meshList_in),
    time_frame(0),
    num_time_pts(0),
    max_val(0.0),
    min_val(0.0),
    transMatrix(glm::mat4(1.0)),
    num_qrs(0),
    num_actis{{0,0}},
    draw_ticker_bool          (false),
    draw_closest_electrode_raw(true ),
    draw_closest_electrode_pro(true ),
    draw_atrial_raw           (true ),
    draw_atrial_pro           (true ),
    show_activations          (true ),
    draw_phase                (true ),
    user_scale                (false)
{
    //std::cout << "Pickwindow ctor() indices: "<< channel_index[0]<<"/"<<channel_index[1] << std::endl;
    this->setMinimumSize(450, 200);

    if( pickwin_type == EMG_atrial )
    {
        draw_closest_electrode_raw = false;
        draw_closest_electrode_pro = false;
    }

}

Pickwindow::~Pickwindow()
{
//    qShader = new QOpenGLShaderProgram;

//    qVAO   [ mesh_type ][ data_type ] = new QOpenGLVertexArrayObject;
//    actiVAO[ mesh_type ][ data_type ] = new QOpenGLVertexArrayObject;
//    qVBdata[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
//    actiVBO[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
//    actiEBO[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
//    qrsVAO   [ data_type ] = new QOpenGLVertexArrayObject;
//    qrsVBdata[ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
//    qrsEBO   [ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
//    qEBO[ mesh_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
//    qEBO[ mesh_type ]->setUsagePattern   ( QOpenGLBuffer::DynamicDraw  );
//    qEBO[ mesh_type ]->create();

//    ///VAOs &VBOs - one holder for each mesh:data combo
//    for( int data_type=0; data_type<DataType::max_data_types; data_type++ )
//    {
//        qVAO   [ mesh_type ][ data_type ] = new QOpenGLVertexArrayObject;
//        actiVAO[ mesh_type ][ data_type ] = new QOpenGLVertexArrayObject;
//        qVBdata[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
//        actiVBO[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
//        actiEBO[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );

}

void Pickwindow::setChannelIndices( int channel[MeshType::max_num_meshes] )
{
    std::cout << "Pickwindow (Type: " << pickwin_type << ") --> setChannelIndices[" << channel[atrium] << "," << channel[catheter] << "]" << std::endl;

    channel_index[ MeshType::atrium ]   = channel[ MeshType::atrium   ];
    channel_index[ MeshType::catheter ] = channel[ MeshType::catheter ];


    for( int m=0; m<MeshType::max_num_meshes; m++ )
    {
        MeshType mesh_type = static_cast<MeshType>(m);

        if( meshList[ mesh_type ] )
        {
            if( meshList[ mesh_type ]->hasRawData() )
            {
                this->updatePickDataBuffers( mesh_type, raw );
            }
            if( meshList[ mesh_type ]->hasProcessedData() )
            {
                this->updatePickDataBuffers( mesh_type, processed );
            }
        }
    }

    std::cout << "Pickwindow (Type: " << pickwin_type << ") <-- setChannelIndices" << std::endl;
}

void Pickwindow::initializeGL()
{
    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> initializeGL()" << std::endl;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.9f, 0.9f, 0.9f, 0.0f              );
    f->glEnable    ( GL_LINE_SMOOTH                     );
    f->glEnable    ( GL_MULTISAMPLE                     );
    f->glEnable    ( GL_BLEND                           );
    f->glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    qPickWinShader = new QOpenGLShaderProgram;
    if( qPickWinShader->addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/Helper_Functions/Shaders/emgvertshader.vert") ){   std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> initializeGL() --> V_SHADER SUCCESSFUL" << std::endl; } else { std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> initializeGL() --> V_SHADER FAILED" << std::endl;  exit(-1); }
    if( qPickWinShader->addShaderFromSourceFile( QOpenGLShader::Fragment, ":/Helper_Functions/Shaders/emgfragshader.frag") ){   std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> initializeGL() --> F_SHADER SUCCESSFUL" << std::endl; } else { std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> initializeGL() --> V_SHADER FAILED" << std::endl;  exit(-1); }
    qPickWinShader->bindAttributeLocation  ( "position", vertexPosPick );
    qPickWinShader->link();
    qPickWinShader->bind();
    qPickWinTransMatLoc  = qPickWinShader->uniformLocation( "transMatrix" );
    qPickWinLineColorLoc = qPickWinShader->uniformLocation( "line_color"  );
    qPickWinShader->release();


    this->initialise_Q_Buffers();
    this->fillDataBuffers();

    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") <-- initializeGL()" << std::endl;
}

void Pickwindow::fillDataBuffers()
{
    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> fillDataBuffers()" << std::endl;

    //Fill buffers if data
    if(       pickwin_type==EMG_raw      && meshList[catheter]->hasRawData() )
    {
        this->updatePickDataBuffers( catheter, raw );

    }else if( pickwin_type==EMG_filtered && meshList[catheter]->hasProcessedData() )
    {
        this->updatePickDataBuffers( catheter, processed );

    }else if( pickwin_type==EMG_atrial )
    {
        if( meshList[atrium]->hasRawData() )
        {
            this->updatePickDataBuffers( atrium  , raw       );
        }
        if( meshList[atrium]->hasProcessedData() )
        {
            this->updatePickDataBuffers( atrium  , processed );
        }
        if( meshList[atrium]->hasPhaseData() )
        {
            this->updatePickDataBuffers( atrium  , phase );
        }
        if( meshList[catheter]->hasRawData() )
        {
            this->updatePickDataBuffers( catheter, raw       );
        }
        if( meshList[catheter]->hasProcessedData() )
        {
            this->updatePickDataBuffers( catheter, processed );
        }
    }

    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") <-- fillDataBuffers()" << std::endl;
}

void Pickwindow::resizeGL( int w, int h )
{

}

void Pickwindow::paintGL()
{
    //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> paint()" << std::endl;

    this->makeCurrent();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    ///Reenable stuff QPainter turns off
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f->glEnable    ( GL_LINE_SMOOTH                     );
    f->glEnable    ( GL_POINT_SMOOTH                    );
    f->glEnable    ( GL_MULTISAMPLE                     );
    f->glEnable    ( GL_BLEND                           );
    f->glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ///Native OpenGL rendering
    if( pickwin_type == PickWinType::EMG_atrial )
    {
        this->drawAtrial( f );
        this->drawAtrialActivations( f, atrium, processed );
        this->drawTicker( f );
    }
    else if( pickwin_type == PickWinType::EMG_raw )
    {
        this->drawRaw( f );
        this->drawQRSactivations( f );
        this->drawAtrialActivations( f, catheter, raw );
    }
    else if( pickwin_type == PickWinType::EMG_filtered )
    {
        this->drawFiltered( f );
        this->drawQRSactivations( f );
        this->drawAtrialActivations( f, catheter, processed );
        this->drawTicker( f );
    }



    ///QPainter overpainting
    if( pickwin_type == PickWinType::EMG_atrial )
    {
        this->drawScaleBar( );
        this->drawInfoText(  PickWinType::EMG_atrial );
    }
    else if( pickwin_type == PickWinType::EMG_raw )
    {
        this->drawScaleBar( );
        this->drawInfoText( PickWinType::EMG_raw );
    }
    else if( pickwin_type == PickWinType::EMG_filtered )
    {
        this->drawScaleBar( );
        this->drawInfoText( PickWinType::EMG_filtered );
    }

    //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- paint()" << std::endl;

}

void Pickwindow::drawAtrial(QOpenGLFunctions *f)
{
    //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawAtrial()" << std::endl;

    if(  draw_atrial_raw && meshList[ atrium ]->hasRawData() )
    {
        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawAtrial RAW()" << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        qPickWinShader->setUniformValue( qPickWinLineColorLoc, 0.0,0.0,0.0 ); //black

        qVAO[ atrium ][ raw ]->bind();
            f->glDrawElements(GL_LINE_STRIP, num_time_pts, GL_UNSIGNED_INT, 0);
        qVAO[ atrium ][ raw ]->release();

        qPickWinShader->release();

        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawAtrial RAW()" << std::endl;
    }
    if(  draw_atrial_pro && meshList[ atrium ]->hasProcessedData() )
    {
        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawAtrial PROCESSED()" << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        qPickWinShader->setUniformValue( qPickWinLineColorLoc, QVector3D(0.0,0.0,1.0) ); //blue

        qVAO[ atrium ][ processed ]->bind();
            f->glDrawElements(GL_LINE_STRIP, num_time_pts, GL_UNSIGNED_INT, 0);
        qVAO[ atrium ][ processed ]->release();

        qPickWinShader->release();

        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawAtrial PROCESSED()" << std::endl;
    }
    if(  draw_phase && meshList[ atrium ]->hasPhaseData() )
    {
        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawAtrial PHASE()" << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        qPickWinShader->setUniformValue( qPickWinLineColorLoc, QVector3D(0.0,1.0,1.0) ); //cyan

        qVAO[ atrium ][ phase ]->bind();
            f->glDrawElements(GL_LINE_STRIP, num_time_pts, GL_UNSIGNED_INT, 0);
        qVAO[ atrium ][ phase ]->release();

        qPickWinShader->release();

        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawAtrial PHASE()" << std::endl;
    }

    if( draw_closest_electrode_pro && meshList[ catheter ]->hasProcessedData() )
    {
        this->drawFiltered( f );

    }else if( draw_closest_electrode_raw && meshList[ catheter ]->hasRawData() )
    {
        this->drawRaw( f );
    }

    //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawAtrial()" << std::endl;
}

void Pickwindow::drawRaw(QOpenGLFunctions *f)
{
    if( draw_closest_electrode_raw && meshList[ catheter ]->hasRawData() )
    {
        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawCathRaw()" << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        qPickWinShader->setUniformValue( qPickWinLineColorLoc, 1.0,0.0,0.0 ); //red

        qVAO[ catheter ][ raw ]->bind();
            f->glDrawElements(GL_LINE_STRIP, num_time_pts, GL_UNSIGNED_INT, 0);
        qVAO[ catheter ][ raw ]->release();

        qPickWinShader->release();

        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawCathRaw()" << std::endl;
    }


}

void Pickwindow::drawFiltered(QOpenGLFunctions *f)
{

    if( draw_closest_electrode_pro && meshList[ catheter ]->hasProcessedData() )
    {
        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawCathFiltered()" << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        qPickWinShader->setUniformValue( qPickWinLineColorLoc, QVector3D(1.0,0.0,1.0) ); //magenta

        qVAO[ catheter ][ processed ]->bind();
            f->glDrawElements(GL_LINE_STRIP, num_time_pts, GL_UNSIGNED_INT, 0);
        qVAO[ catheter ][ processed ]->release();

        qPickWinShader->release();

        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawCathFiltered()" << std::endl;
    }


}

void Pickwindow::drawQRSactivations(QOpenGLFunctions *f)
{
    if( meshList[catheter]->hasQRSdata() )
    {
        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawQRSactivations() - size=" << num_qrs << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        qPickWinShader->setUniformValue( qPickWinLineColorLoc, QVector3D(0.0,0.0,1.0) ); //blue

        qVAO[ catheter ][ QRS_actis ]->bind();
            glPointSize(7.5f);
            f->glDrawElements(GL_POINTS, num_qrs, GL_UNSIGNED_INT, 0);
        qVAO[ catheter ][ QRS_actis ]->release();

        qPickWinShader->release();

        //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawQRSactivations()" << std::endl;
    }
}

void Pickwindow::drawAtrialActivations(QOpenGLFunctions *f, MeshType mesh_type, DataType data_type)
{
    if(  show_activations && meshList[ mesh_type ]->hasAtrialActiData()  )
    {
        //std::cout << "\tPickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> drawAtrialActivations()" << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        if( mesh_type == catheter )
        {
            qPickWinShader->setUniformValue( qPickWinLineColorLoc, QVector3D(0.0,1.0,0.0) ); //green
        }else if( mesh_type == atrium ){
            qPickWinShader->setUniformValue( qPickWinLineColorLoc, QVector3D(1.0,0.0,1.0) ); //magenta
        }
        qVAO[ mesh_type ][ atrial_actis ]->bind();
            glPointSize(7.5f);
            //std::cout << "\tTrying to draw: " << num_actis[ mesh_type ][ data_type ] << " activations" << std::endl;
            f->glDrawElements(GL_POINTS, num_actis[ mesh_type ][ data_type ], GL_UNSIGNED_INT, 0);
        qVAO[ mesh_type ][ atrial_actis ]->release();


        qPickWinShader->release();

        //std::cout << "\tPickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") <-- drawQRSactivations()" << std::endl;
    }
}

void Pickwindow::drawScaleBar()
{
    //std::cout << "\tPickwindow (Type: " << pickwin_type << ") --> drawScaleBar()" << std::endl;

    QPainter painter(this);

    painter.setPen(QPen(Qt::black,3));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawLine(6, 6, 6, height()-6);
    painter.end();

    writeText( QString::number(max_val) + "mV", 10, 15,         10, "black");
    writeText( QString::number(min_val) + "mV", 10, height()-6, 10, "black");

    //std::cout << "\tPickwindow (Type: " << pickwin_type << ") <-- drawScaleBar()" << std::endl;
}

void Pickwindow::drawInfoText(PickWinType pickType)
{
    if( pickType==PickWinType::EMG_atrial )
    {
        writeText( "Vertex: " + QString::number( channel_index[ atrium ] +1), 100, height()-6, 10, "black");
        writeText( "Inverse solution  ", 350, height()-6, 10, "blue");

        if(  draw_closest_electrode_pro && meshList[ catheter ]->hasProcessedData()  )
        {
            writeText( "Processed cath", 180, height()-6, 10, "magenta");
        }
        if(  draw_closest_electrode_raw && meshList[ catheter ]->hasRawData()  )
        {
            writeText("Raw cath", 280, height()-6, 10, "red");
        }

    }else if( pickType==PickWinType::EMG_raw ) {

        writeText( "Catheter electrode: " + QString::number( channel_index[ catheter ] +1), 100, height()-6, 10, "black");
        writeText( "Raw cath", 350, height()-6, 10, "red");

    }else if( pickType==PickWinType::EMG_filtered ) {

        writeText( "Catheter electrode: " + QString::number( channel_index[ catheter ]+1), 100, height()-6, 10, "black");
        writeText( "Processed cath", 350, height()-6, 10, "magenta");
    }
}

void Pickwindow::drawTicker(QOpenGLFunctions *f)
{


    if(  draw_ticker_bool && (draw_atrial_raw || draw_atrial_pro) )
    {
        //std::cout << "Pickwindow::drawTicker()" << std::endl;

        qPickWinShader->bind();
        qPickWinShader->setUniformValue( qPickWinTransMatLoc , QMatrix4x4( glm::value_ptr(transMatrix)).transposed()  );
        qPickWinShader->setUniformValue( qPickWinLineColorLoc, QVector3D(0.0,0.0,0.0) ); //black

        timerVAO->bind();
            f->glDrawElements(GL_LINE_STRIP, 2, GL_UNSIGNED_INT, (GLvoid*)(time_frame*2*sizeof(unsigned)));
        timerVAO->release();

        qPickWinShader->release();

        //std::cout << "Pickwindow::drawTicker() exit" << std::endl;
    }


}

void Pickwindow::initialise_Q_Buffers()
{
    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> initialise_Q_Buffers()" << std::endl;

    this->makeCurrent();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();


    ///Timer
    timerVAO = new QOpenGLVertexArrayObject;
    timerVBO = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
    timerEBO = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
    timerVBO->setUsagePattern   ( QOpenGLBuffer::DynamicDraw  );
    timerEBO->setUsagePattern   ( QOpenGLBuffer::DynamicDraw  );
    timerVAO->create();
    timerVBO->create();
    timerEBO->create();

    timerVAO->bind();
        timerVBO->bind();
        f->glEnableVertexAttribArray ( vertexPosPick );
        f->glVertexAttribPointer     ( vertexPosPick, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        timerVBO->release();
        timerEBO->bind();
    timerVAO->release();


    for( int mesh_type=0; mesh_type<MeshType::max_num_meshes; mesh_type++ )
    {
        ///VAOs &VBOs - one holder for each mesh:data combo
        for( int data_type=0; data_type<DataType::max_data_types; data_type++ )
        {
            qVAO   [ mesh_type ][ data_type ] = new QOpenGLVertexArrayObject;
            qVAO   [ mesh_type ][ data_type ]->create();

            qVBdata[ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
            qEBO   [ mesh_type ][ data_type ] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer  );
            qVBdata[ mesh_type ][ data_type ]->setUsagePattern   ( QOpenGLBuffer::DynamicDraw  );
            qEBO   [ mesh_type ][ data_type ]->setUsagePattern   ( QOpenGLBuffer::DynamicDraw  );
            qVBdata[ mesh_type ][ data_type ]->create();
            qEBO   [ mesh_type ][ data_type ]->create();

            ///Set VAO
            qVAO   [ mesh_type ][ data_type ]->bind();
                qVBdata[ mesh_type ][ data_type ]->bind();
                f->glEnableVertexAttribArray ( vertexPosPick );
                f->glVertexAttribPointer     ( vertexPosPick, 3, GL_FLOAT, GL_FALSE, 0, 0 );
                qVBdata[ mesh_type ][ data_type ]->release();
                qEBO   [ mesh_type ][ data_type ]->bind();
            qVAO   [ mesh_type ][ data_type ]->release();
        }
    }

    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") <-- initialise_Q_Buffers()" << std::endl;
}

void Pickwindow::updatePickDataBuffers( MeshType mesh_type, DataType data_type )
{
    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ")(MeshType: " << meshTypeStr(mesh_type) << ")(DataType: " << dataTypeStr(data_type) << ") --> updatePickDataBuffers()" << std::endl;

    if( data_type == QRS_actis || data_type == atrial_actis || data_type == no_data )
    {
        std::cout << "Pickwindow (Type: " << pickwin_type << ")(MeshType: " << mesh_type << ")(DataType: " << data_type << ") --> updatePickDataBuffers() SIGNAL IGNORED" << std::endl;
        return;   ///QRS and acti data is extracted along with the signal data - the individual indicies don't mean much so ignore any calls with these as data type(there shouldn't be any)
    }

//std::cout << "\t1" << std::endl;

    this->makeCurrent();


//std::cout << "\t2" << std::endl;

    size_t n  = meshList[ mesh_type ]->getMeshData().data[ data_type ].cols();

//std::cout << "\t2 - n=" << n << std::endl;

    float max = meshList[ mesh_type ]->getMeshData().data[ data_type ].row( channel_index[ mesh_type ] ).maxCoeff();
    float min = meshList[ mesh_type ]->getMeshData().data[ data_type ].row( channel_index[ mesh_type ] ).minCoeff();

    if( !user_scale && data_type != acti_history && data_type != phase ) ///if user hasn't changed the scale and the type isn't actiHistory, i.e. skip this bit if using user scaling or working with acti_hist or phase data
    {
        if( num_time_pts < n   ){ num_time_pts = n;   }
        if( max_val      < max ){ max_val      = max; }
        if( min_val      > min ){ min_val      = min; }

        ///Don't zoom in too far on small signals
        if( max_val      <  0.5 ){ max_val =  0.5; } ///+0.5mV
        if( min_val      > -0.5 ){ min_val = -0.5; } ///-0.5mV

        this->updateTransformMatrix(  max_val, min_val, 0.0, num_time_pts );

    }else{

        num_time_pts = n;
        this->updateTransformMatrix(  max_val, min_val, 0.0, num_time_pts );

    }

//std::cout << "\t3 - n=" << n << std::endl;

    Eigen::MatrixXf potential_data = Eigen::MatrixXf::Zero( 3, num_time_pts );
    potential_data.row(0) = Eigen::ArrayXf::LinSpaced( num_time_pts, 0, num_time_pts-1 );                             ///x value = time point

//std::cout << "\t4" << std::endl;

//helper::printMatrixInfo( potential_data, "Potential data" );
//std::cout << "Row: " << channel_index[ mesh_type ] <<  std::endl;
//helper::printMatrixInfo(  meshList[ mesh_type ]->getMeshData().data[ data_type ].row( channel_index[ mesh_type ] ), "Processed data row" );

    potential_data.row(1) = meshList[ mesh_type ]->getMeshData().data[ data_type ].row( channel_index[ mesh_type ] ); ///y value = potential data
    if( data_type == phase )
    {
        potential_data.row(1).array() /= 6.2832; ///normalise pi to +/-0.5
    }

//std::cout << "\t5" << std::endl;

    ///Signal data
    qVBdata[ mesh_type ][ data_type ]->bind();
    qVBdata[ mesh_type ][ data_type ]->allocate( potential_data.data(), potential_data.size() * sizeof(float) );
    qVBdata[ mesh_type ][ data_type ]->release();
    qEBO   [ mesh_type ][ data_type ]->bind();
    qEBO   [ mesh_type ][ data_type ]->allocate( potential_data.row(0).cast<unsigned>().eval().data(), num_time_pts * sizeof(unsigned) );
    qEBO   [ mesh_type ][ data_type ]->release();


//std::cout << "\t5" << std::endl;

    ///Ticker data
    Eigen::MatrixXf ticker_data = Eigen::MatrixXf::Zero( 6, num_time_pts );   //z vals will be zero
    ticker_data.row(0) = Eigen::ArrayXf::LinSpaced(num_time_pts, 0, num_time_pts-1);     //x value = time point
    ticker_data.row(1) = Eigen::ArrayXf::Constant( num_time_pts, max_val );       //y value = ticker stretches to the top of the data
    ticker_data.row(3) = Eigen::ArrayXf::LinSpaced( num_time_pts, 0, num_time_pts-1);    //x value = time point
    ticker_data.row(4) = Eigen::ArrayXf::Constant( num_time_pts, min_val );       //y value = ticker stretches to the bottom of the data
    Eigen::Matrix<unsigned, Eigen::Dynamic, 1> indices = Eigen::ArrayXf::LinSpaced(2*num_time_pts, 0, 2*num_time_pts-1).cast<unsigned>();
    timerVBO->bind();
    timerVBO->allocate( ticker_data.data(), ticker_data.size() * sizeof(float) );
    timerVBO->release();
    timerEBO->bind();
    timerEBO->allocate( indices.data(), indices.size() * sizeof(unsigned) );
    timerEBO->release();


//std::cout << "\t6" << std::endl;

    ///QRS data
    if( (mesh_type == catheter && data_type == raw       && pickwin_type == EMG_raw      && meshList[ catheter ]->hasQRSdata()) ||
        (mesh_type == catheter && data_type == processed && pickwin_type == EMG_filtered && meshList[ catheter ]->hasQRSdata())    )
    {
        std::cout << "\tPickwindow::updatePickDataBuffers - QRS BUFFER UPDATE" << std::endl;

        //helper::printMatrixInfo( meshList[ mesh_type ]->getMeshData().data[ QRS_actis ], "meshList[ mesh_type ]->getMeshData().data[ QRS_actis ]w" );

        num_qrs = meshList[ mesh_type ]->getMeshData().data[ QRS_actis ].rows();

        Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor> qrs_data = Eigen::MatrixXf::Zero( num_qrs, 3 );
        qrs_data.col(0) = meshList[ mesh_type ]->getMeshData().data[ QRS_actis ].col(AA::time_idx);

        Eigen::VectorXf pots = meshList[ catheter ]->getMeshData().data[ data_type ].row( channel_index[ catheter ] );

       // helper::printMatrixInfo( pots, "pots raw" );

        Eigen::VectorXf slicedpots;
        igl::slice( pots,meshList[ catheter ]->getMeshData().data[ QRS_actis ].col(AA::time_idx).cast<int>(),1,slicedpots);
        qrs_data.col(1) = slicedpots;

           // helper::printMatrixInfo( qrs_data, "qrs_data" );

        ///Generate vertex indicies data for the EBO
        Eigen::Matrix<unsigned, Eigen::Dynamic,1> inds = Eigen::ArrayXf::LinSpaced(num_qrs,0,num_qrs-1).cast<unsigned>();

        ///Build the buffer objects and fill with the data
        qVBdata[ catheter ][ QRS_actis ]->bind();
        qVBdata[ catheter ][ QRS_actis ]->allocate( qrs_data.data(), qrs_data.size() * sizeof(float) );
        qVBdata[ catheter ][ QRS_actis ]->release();
        qEBO   [ catheter ][ QRS_actis ]->bind();
        qEBO   [ catheter ][ QRS_actis ]->allocate( inds.data(), inds.size() * sizeof(unsigned int) );
        qEBO   [ catheter ][ QRS_actis ]->release();
    }



    if( ( mesh_type == catheter && data_type == processed && pickwin_type == EMG_filtered && meshList[ catheter ]->hasAtrialActiData()) ||
        ( mesh_type == atrium   && data_type == processed && pickwin_type == EMG_atrial   && meshList[ atrium   ]->hasAtrialActiData())    )
    {
    //std::cout << "\tupdatePickDataBuffers() - looking for crash" << std::endl;
        std::cout << "\tPickwindow::updatePickDataBuffers - ACTIVATIONS BUFFER UPDATE" << std::endl;

        Eigen::Array<bool,Eigen::Dynamic,1> chanBool =
            ( meshList[ mesh_type ]->getMeshData().data[ atrial_actis ].col(AA::channel).cast<int>().array() == channel_index[ mesh_type ] )
              .select( Eigen::Array<bool,Eigen::Dynamic,1>::Constant( meshList[ mesh_type ]->getMeshData().data[ atrial_actis ].rows(),true), false);

        num_actis[ mesh_type ][ data_type ] = (chanBool == true).count();

        Eigen::MatrixXf slicedTimeIdx( num_actis[ mesh_type ][ data_type ], 1 );
        igl::slice_mask( meshList[ mesh_type ]->getMeshData().data[ atrial_actis ].col(AA::time_idx), chanBool, 1, slicedTimeIdx );

        Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor> acti_data = Eigen::MatrixXf::Zero( num_actis[ mesh_type ][ data_type ], 3 );
        acti_data.col(0) = slicedTimeIdx;

        Eigen::VectorXf pots = meshList[ mesh_type ]->getMeshData().data[ data_type ].row( channel_index[ mesh_type ] );
        Eigen::VectorXf slicedpots;

        igl::slice(pots,slicedTimeIdx.cast<int>(),1,slicedpots);
        acti_data.col(1) = slicedpots;

        //helper::printMatrixInfo( acti_data, "acti_data" );
        //std::cout << "Num actis: " << num_actis[ mesh_type ] << std::endl;

        //Generate vertex indicies data for the EBO
        Eigen::Matrix<unsigned, Eigen::Dynamic,1> inds = Eigen::ArrayXf::LinSpaced(num_actis[ mesh_type ][ data_type ],0,num_actis[ mesh_type ][ data_type ]-1).cast<unsigned>();

        //Build the buffer objects and fill with the data
        qVBdata[ mesh_type ][ atrial_actis ]->bind();
        qVBdata[ mesh_type ][ atrial_actis ]->allocate( acti_data.data(), acti_data.size() * sizeof(float) );
        qVBdata[ mesh_type ][ atrial_actis ]->release();
        qEBO   [ mesh_type ][ atrial_actis ]->bind();
        qEBO   [ mesh_type ][ atrial_actis ]->allocate( inds.data(), inds.size() * sizeof(unsigned int) );
        qEBO   [ mesh_type ][ atrial_actis ]->release();

    }




    this->update();
    std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ")(MeshType: " << meshTypeStr(mesh_type) << ")(DataType: " << dataTypeStr(data_type) << ") <-- updatePickDataBuffers()" << std::endl;
}

void Pickwindow::updateTransformMatrix(float max_y, float min_y, float min_x, float max_x )
{
    //std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> updateTransformMatrix()" << std::endl;

    transMatrix = glm::mat4(1.0f);
    transMatrix = glm::translate( transMatrix, {-1, -1, 0} );
    float y_scale = 2.0f / ( max_y - min_y );
    float x_scale = 2.0f / ( max_x - min_x );
    transMatrix = glm::scale    ( transMatrix, {x_scale*0.95, y_scale*0.9, 1}); //some padding
    transMatrix = glm::translate( transMatrix, {max_x/30, -min_y*1.1, 0}  );

    //std::cout << "Pickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") <-- updateTransformMatrix()" << std::endl;
}

void Pickwindow::updateTimeFrame( int time )
{
    draw_ticker_bool = true;
    time_frame = time;
    this->update();
}

void Pickwindow::writeText( QString text, int x, int y, int size, QString color)
{
    //std::cout << "\tPickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") --> writeText()" << std::endl;

    QPainter painter(this);

    if(color == "white"){ painter.setPen(Qt::white); }
    if(color == "black"){ painter.setPen(Qt::black); }
    if(color == "red")  { painter.setPen(Qt::red);   }
    if(color == "blue") { painter.setPen(Qt::blue);  }
    if(color == "green"){ painter.setPen(Qt::green); }
    if(color == "magenta") { painter.setPen(Qt::magenta); }

    painter.setFont(QFont("Lato Light", size));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(x, y, text);
    painter.end();


    //std::cout << "\tPickwindow (Type: " << pickWinTypeStr(pickwin_type) << ") <-- writeText()" << std::endl;
}

void Pickwindow::changeNearRawElectrodeBool( bool checked )
{
    //std::cout << "changeNearRawElectrodeBool: " << checked << std::endl;
    draw_closest_electrode_raw = checked;
    this->update();
}

void Pickwindow::changeNearProElectrodeBool( bool checked )
{
    //std::cout << "changeNearProElectrodeBool: " << checked << std::endl;
    draw_closest_electrode_pro = checked;
    this->update();
}

void Pickwindow::changeAtrialRawBool( bool checked )
{
    //std::cout << "changeAtrialRawBool: " << checked << std::endl;
    draw_atrial_raw = checked;
    this->update();
}

void Pickwindow::changeAtrialProBool( bool checked )
{
    //std::cout << "changeAtrialProBool: " << checked << std::endl;
    draw_atrial_pro = checked;
    this->update();
}

void Pickwindow::changeShowActisBool( bool checked )
{
    show_activations = checked;
    this->update();
}

void Pickwindow::changeShowPhaseBool( bool checked )
{
    draw_phase = checked;
    this->update();
}

int Pickwindow::getChannelIndices( MeshType mesh_type )
{
    return channel_index[ mesh_type ];
}

void Pickwindow::updateTransformMatrixUSER(float max_y, float min_y, bool use_user_scaling )
{
    user_scale = use_user_scaling;

    if( user_scale ) ///if user scaling requested that update the transmatrix with the user's values
    {
        max_val    = max_y;
        min_val    = min_y;
        this->updateTransformMatrix( max_y, min_y, 0, num_time_pts );
        this->update();

    }else{  /// else

      ///// crashes this->fillDataBuffers();

    }
}

std::string Pickwindow::meshTypeStr( MeshType mesh_type )
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

std::string Pickwindow::dataTypeStr( DataType data_type )
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
            return "Error mesh name";
    }
}

std::string Pickwindow::pickWinTypeStr( PickWinType pick_win )
{
    switch( pick_win )
    {
        case EMG_raw:
            return "Raw EMG display";
        case EMG_filtered:
            return "Filtered EMG display";
        case EMG_atrial:
            return "Atrial EMG display";
        default:
            return "Error mesh name";
    }
}
