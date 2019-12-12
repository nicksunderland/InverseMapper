#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "signalprocessingdialog.h"
#include "DSP_settings/geomwinsettings.h"
#include "DSP_settings/dsp_activationhistorysettings.h"
#include "DSP_settings/dsp_inversesettings.h"
#include "DSP_settings/dsp_phasesettings.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    meshList( max_num_meshes ),
    current_time(0),
    max_num_time_frames(0),
    frames_per_sec(200)
{
    std::cout << "MainWindow --> ctor()" << std::endl;

    ui->setupUi(this);

    this->makeConnections();
    this->setupControls();
    this->setupWindows();

    std::cout << "MainWindow <-- ctor()" << std::endl;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setMeshList(std::vector<QSharedPointer<Mesh> > &meshList_in)
{
    meshList = meshList_in;

    //Make the connections to the meshes
    for( size_t mesh=0; mesh<meshList.size(); mesh++ )
    {
        if( meshList[ mesh ] ) //not null
        {
            connect( meshList[mesh].get(), &Mesh::mesh_buffers_updated_signal, ui->openGLWidget1, &Geomwindow::updateVAOs, Qt::QueuedConnection );
            connect( meshList[mesh].get(), &Mesh::mesh_buffers_updated_signal, ui->openGLWidget2, &Geomwindow::updateVAOs, Qt::QueuedConnection );
            connect( meshList[mesh].get(), &Mesh::mesh_buffers_updated_signal, ui->openGLWidget3, &Geomwindow::updateVAOs, Qt::QueuedConnection );
        }
    }
}

void MainWindow::setupWindows()
{
    ui->openGLWidget1->setGeomwinIndex(0);
    ui->openGLWidget2->setGeomwinIndex(1);
    ui->openGLWidget3->setGeomwinIndex(2);

    if( ui->radioButton_raw_basket_signal_mode->isChecked() )
    {
        ui->openGLWidget1->geomwinSettings.setDefaults(0,Mode::catheter_input );
        ui->openGLWidget2->geomwinSettings.setDefaults(1,Mode::catheter_input );
        ui->openGLWidget3->geomwinSettings.setDefaults(2,Mode::catheter_input );
    }
    if( ui->radioButton_atrial_signals_mode->isChecked() )
    {
        ui->openGLWidget1->geomwinSettings.setDefaults(0,Mode::atrial_input );
        ui->openGLWidget2->geomwinSettings.setDefaults(1,Mode::atrial_input );
        ui->openGLWidget3->geomwinSettings.setDefaults(2,Mode::atrial_input );
    }
    ui->checkBox_showNearElectrode          ->setChecked( false );
    ui->checkBox_showAtrialElectrogram      ->setChecked( true  );

    this->updateGeomwinLabel();
}

void MainWindow::makeConnections()
{
    //When meshes are created in processing - give out a mesh pointer list to classes that need it
    connect(  &processing, &Processing::update_meshLists, this,              &MainWindow::setMeshList        );
    connect(  &processing, &Processing::update_meshLists, ui->openGLWidget1, &Geomwindow::setGeomwinMeshPtrs );
    connect(  &processing, &Processing::update_meshLists, ui->openGLWidget2, &Geomwindow::setGeomwinMeshPtrs );
    connect(  &processing, &Processing::update_meshLists, ui->openGLWidget3, &Geomwindow::setGeomwinMeshPtrs );

    connect(  &processing, &Processing::new_mesh_added_signal, ui->openGLWidget1, &Geomwindow::initialise_Q_Buffers );
    connect(  &processing, &Processing::new_mesh_added_signal, ui->openGLWidget2, &Geomwindow::initialise_Q_Buffers );
    connect(  &processing, &Processing::new_mesh_added_signal, ui->openGLWidget3, &Geomwindow::initialise_Q_Buffers );

    //Mainwindow to processing
    connect( ui->pushButton_runInverse,        &QPushButton::clicked, &processing, &Processing::runInverse );
    connect( ui->pushButton_createActiHistMap, &QPushButton::clicked, &processing, &Processing::createEndoActivationMap );
    connect( ui->pushButton_phaseMap,          &QPushButton::clicked, &processing, &Processing::createPhaseMap );

    //Mainwindow to geomwindow
    connect( this, &MainWindow::mesh_data_max_min_signal, ui->openGLWidget1, &Geomwindow::updateMinMax     );
    connect( this, &MainWindow::mesh_data_max_min_signal, ui->openGLWidget2, &Geomwindow::updateMinMax     );
    connect( this, &MainWindow::mesh_data_max_min_signal, ui->openGLWidget3, &Geomwindow::updateMinMax     );
    connect( ui->doubleSpinBox_mVcutoff, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->openGLWidget1, &Geomwindow::setLow_mVcutoff );
    connect( ui->doubleSpinBox_mVcutoff, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->openGLWidget2, &Geomwindow::setLow_mVcutoff );
    connect( ui->doubleSpinBox_mVcutoff, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->openGLWidget3, &Geomwindow::setLow_mVcutoff );
    connect( ui->doubleSpinBox_cathSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->openGLWidget1, &Geomwindow::setCathScaling   );
    connect( ui->doubleSpinBox_cathSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->openGLWidget2, &Geomwindow::setCathScaling   );
    connect( ui->doubleSpinBox_cathSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->openGLWidget3, &Geomwindow::setCathScaling   );

    //Geomwindows to each other for locked camera functions
    ui->openGLWidget1->connectWindows( ui->openGLWidget2 );
    ui->openGLWidget1->connectWindows( ui->openGLWidget3 );
    ui->openGLWidget2->connectWindows( ui->openGLWidget1 );
    ui->openGLWidget2->connectWindows( ui->openGLWidget3 );
    ui->openGLWidget3->connectWindows( ui->openGLWidget1 );
    ui->openGLWidget3->connectWindows( ui->openGLWidget2 );

    //Picking info from the geomwindows
    //Click on geomwindow and transfer click info to processing
    connect( ui->openGLWidget1, &Geomwindow::pickingSignal, &processing, &Processing::processPick   );
    connect( ui->openGLWidget2, &Geomwindow::pickingSignal, &processing, &Processing::processPick   );
    connect( ui->openGLWidget3, &Geomwindow::pickingSignal, &processing, &Processing::processPick   );
    //Process the click info and see if a node was hit, if so create a pick window and pass to main GUI to make signal connections and add to display
    connect( &processing,       &Processing::new_pickwindown_added_signal,this,&MainWindow::connect_new_pickwindow);
    //Emit that there is a new pick back to the geomwindows so that they display it
    connect( this, &MainWindow::pick_windows_changed, ui->openGLWidget1, &Geomwindow::refreshPickData );
    connect( this, &MainWindow::pick_windows_changed, ui->openGLWidget2, &Geomwindow::refreshPickData );
    connect( this, &MainWindow::pick_windows_changed, ui->openGLWidget3, &Geomwindow::refreshPickData );
    //If moving the basket emit a signal saying the pick is moving - process the move and adjust the nearest basket electrode index if needed
    connect( ui->openGLWidget1, &Geomwindow::basket_moved_update_picks_signal, &processing, &Processing::updatePickIndices );
    connect( ui->openGLWidget2, &Geomwindow::basket_moved_update_picks_signal, &processing, &Processing::updatePickIndices );
    connect( ui->openGLWidget3, &Geomwindow::basket_moved_update_picks_signal, &processing, &Processing::updatePickIndices );
    //emit a signal telling the geomwindow to use the new/adjusted pick info
    connect( &processing, &Processing::pick_windows_changed_v2, ui->openGLWidget1, &Geomwindow::refreshPickData );
    connect( &processing, &Processing::pick_windows_changed_v2, ui->openGLWidget2, &Geomwindow::refreshPickData );
    connect( &processing, &Processing::pick_windows_changed_v2, ui->openGLWidget3, &Geomwindow::refreshPickData );


    //Mainwindow to geomwindow
    connect( ui->checkBox_lockMeshes,     &QCheckBox::toggled,    ui->openGLWidget1, &Geomwindow::setMeshLock );
    connect( ui->checkBox_lockMeshes,     &QCheckBox::toggled,    ui->openGLWidget2, &Geomwindow::setMeshLock );
    connect( ui->checkBox_lockMeshes,     &QCheckBox::toggled,    ui->openGLWidget3, &Geomwindow::setMeshLock );
    connect( ui->radioButton_cutoff_grey, &QRadioButton::toggled, ui->openGLWidget1, &Geomwindow::setCutoffDrawStyle );
    connect( ui->radioButton_cutoff_grey, &QRadioButton::toggled, ui->openGLWidget2, &Geomwindow::setCutoffDrawStyle );
    connect( ui->radioButton_cutoff_grey, &QRadioButton::toggled, ui->openGLWidget3, &Geomwindow::setCutoffDrawStyle );

    //Timer
    connect( &timer, &QTimer::timeout, this, &MainWindow::advanceTimer );
    connect( this, &MainWindow::time_frame_changed, ui->openGLWidget1, &Geomwindow::setTimeFrame );
    connect( this, &MainWindow::time_frame_changed, ui->openGLWidget2, &Geomwindow::setTimeFrame );
    connect( this, &MainWindow::time_frame_changed, ui->openGLWidget3, &Geomwindow::setTimeFrame );
}

void MainWindow::on_pushButton_signalProcessDialog_clicked()
{
    std::cout << "SignalProcessingDialog button clicked" << std::endl;

    if( meshList.size() == 0 || !meshList[catheter] || !meshList[atrium] )
    {
        QMessageBox::warning( this, tr("Warning"), tr("Missing catheter or atrial mesh"), QMessageBox::Ok );
        return;
    }else if ( meshList[catheter]->hasNoData() ) {
        QMessageBox::warning( this, tr("Warning"), tr("No catheter data to filter"), QMessageBox::Ok );
        return;
    }

    SignalProcessingDialog *DSPdialog = new SignalProcessingDialog(processing, meshList, nullptr);
    DSPdialog->setAttribute( Qt::WA_DeleteOnClose );
    DSPdialog->setWindowFlags(Qt::WindowStaysOnTopHint);
    DSPdialog->exec();

}

void MainWindow::connect_new_pickwindow( Pickwindow* pickwin )
{
    ///Connect time frame change to tickers
    connect( this, &MainWindow::time_frame_changed, pickwin, &Pickwindow::updateTimeFrame );

    ///Connect autoscaling and min/max signal, emit signal to update the  pick with the settings
    connect( this, &MainWindow::mesh_data_max_min_signal, pickwin, &Pickwindow::updateTransformMatrixUSER );
    emit mesh_data_max_min_signal( ui->doubleSpinBox_mVmax->value(), ui->doubleSpinBox_mVmin->value(), !ui->checkBox_autoScaling->isChecked() );

    ///Connect the show/hide buttons
    connect( ui->checkBox_showNearElectrode_raw, &QCheckBox::toggled, pickwin, &Pickwindow::changeNearRawElectrodeBool);        //
             ui->checkBox_showNearElectrode_raw->toggle();//force and emit
             ui->checkBox_showNearElectrode_raw->toggle();
    connect( ui->checkBox_showNearElectrode_processed, &QCheckBox::toggled, pickwin, &Pickwindow::changeNearProElectrodeBool);
             ui->checkBox_showNearElectrode_processed->toggle();//force and emit
             ui->checkBox_showNearElectrode_processed->toggle();
    connect( ui->checkBox_showAtrialRaw, &QCheckBox::toggled, pickwin, &Pickwindow::changeAtrialRawBool);
             ui->checkBox_showAtrialRaw->toggle();//force and emit
             ui->checkBox_showAtrialRaw->toggle();
    connect( ui->checkBox_showAtrialProcessed, &QCheckBox::toggled, pickwin, &Pickwindow::changeAtrialProBool);
             ui->checkBox_showAtrialProcessed->toggle();//force and emit
             ui->checkBox_showAtrialProcessed->toggle();
    connect( ui->checkBox_showPickActivation, &QCheckBox::toggled, pickwin, &Pickwindow::changeShowActisBool);
             ui->checkBox_showPickActivation->toggle();//force and emit
             ui->checkBox_showPickActivation->toggle();
    connect( ui->checkBox_showAtrialPhase, &QCheckBox::toggled, pickwin, &Pickwindow::changeShowPhaseBool);
             ui->checkBox_showAtrialPhase->toggle();//force and emit
             ui->checkBox_showAtrialPhase->toggle();
    ///Connect each mesh to the pick
    for( int mesh=0; mesh<MeshType::max_num_meshes; mesh++ )
    {
        if( meshList[ mesh ] ) //not null
        {
            connect( meshList[mesh].get(), &Mesh::mesh_buffers_updated_signal, pickwin, &Pickwindow::updatePickDataBuffers, Qt::UniqueConnection );
        }
    }

    ///Add pcik to the layout
    size_t numPicks = processing.pick_win_list.size();
    ui->pickwindow_verticalLayout->insertWidget(numPicks-1, pickwin);

    ///Emit signal that the pick list has changed
    emit pick_windows_changed( processing.pick_win_list );

    //std::cout << "MainWindow::update_mainwindow_picklist EXIT" << std::endl;
}

void MainWindow::on_pushButton_deleteLastPick_clicked()
{
    if( processing.pick_win_list.size() == 0 )
    {
        return;
    }else{
        processing.pick_win_list.back()->deleteLater();  //get Qt to delete the new'd widget
        processing.pick_win_list.pop_back();             //pop the pointer from the vector list

        emit pick_windows_changed( processing.pick_win_list );
    }
}

void MainWindow::on_toolButton_atrialGeom_clicked()
{
    QString atrial_mesh_file_name = QFileDialog::getOpenFileName(this, tr("Open atrial geometry"), tr("Atrial Geometry (*.obj *.off *.stl *.wrl *.ply *.mesh)"));
    ui->lineEdit_atrialGeom->setText( atrial_mesh_file_name );
}

void MainWindow::on_toolButton_atrialData_clicked()
{
    QString atrial_data_file_name = QFileDialog::getOpenFileName(this, tr("Open atrial data"), tr("Atrial Data (*.txt)"));
    ui->lineEdit_atrialData->setText( atrial_data_file_name );
}

void MainWindow::on_toolButton_cathGeom_clicked()
{
    QString cath_mesh_file_name = QFileDialog::getOpenFileName(this, tr("Open catheter geometry"), tr("Catheter Geometry (*.obj *.off *.stl *.wrl *.ply *.mesh)"));
    ui->lineEdit_cathGeom->setText(cath_mesh_file_name);
}

void MainWindow::on_toolButton_cathData_clicked()
{
    QString cath_data_file_name = QFileDialog::getOpenFileName(this, tr("Open basket data"), tr("Basket Data (*.txt)"));
    ui->lineEdit_cathData->setText( cath_data_file_name );
}

void MainWindow::on_pushButton_loadFiles_clicked()
{
    /* Set the run mode - this run mode will influence how the forward and inverse functions
    * are used later on */
    if( ui->radioButton_raw_basket_signal_mode->isChecked() )
    {
        processing.general_vars.mode = Mode::catheter_input;
        ui->openGLWidget1->geomwinSettings.setDefaults(0,catheter_input);
        ui->openGLWidget2->geomwinSettings.setDefaults(1,catheter_input);
        ui->openGLWidget3->geomwinSettings.setDefaults(2,catheter_input);
    }else{
        processing.general_vars.mode = Mode::atrial_input;
        ui->openGLWidget1->geomwinSettings.setDefaults(0,atrial_input);
        ui->openGLWidget2->geomwinSettings.setDefaults(1,atrial_input);
        ui->openGLWidget3->geomwinSettings.setDefaults(2,atrial_input);
    }
    /* Attempt to load the mesh data and potential data from the provided files */

    ///Make sure any left over data is deleted
    processing.clearAllData();

    ///Clear any picks
    if( processing.pick_win_list.size() > 0 )
    {
        for( size_t i=0; i<processing.pick_win_list.size(); i++ )
        {
            processing.pick_win_list.at(i)->deleteLater();  //get Qt to delete the new'd widget
        }
        processing.pick_win_list.clear();             //clear the pointers from the vector list

        emit pick_windows_changed( processing.pick_win_list );
    }

    if( !ui->lineEdit_atrialGeom->text().isEmpty() )
    {
        processing.loadMesh( ui->lineEdit_atrialGeom->text(), MeshType::atrium  );
    }
    if( !ui->lineEdit_atrialData->text().isEmpty() )
    {
        processing.loadData( ui->lineEdit_atrialData->text(), MeshType::atrium  );
    }
    if( !ui->lineEdit_cathGeom->text().isEmpty() )
    {
        processing.loadMesh( ui->lineEdit_cathGeom  ->text(), MeshType::catheter);
    }
    if( !ui->lineEdit_cathData->text().isEmpty() )
    {
        processing.loadData( ui->lineEdit_cathData  ->text(), MeshType::catheter);
    }
    processing.centreCatheter();
    this->extractDataInfo();
}

void MainWindow::extractDataInfo()
{
    for( size_t mesh=0; mesh<meshList.size(); mesh++ )
    {
        if( meshList[mesh] )//not null
        {
            if( meshList[ mesh ]->hasRawData() && meshList[ mesh ]->hasProcessedData() )
            {
                max_num_time_frames = std::min( meshList[ mesh ]->getMeshData().data[raw].cols(), meshList[ mesh ]->getMeshData().data[processed].cols() );
                ui->horizontalSlider->setRange(0,max_num_time_frames-1);
                ui->timeFrameSpinBox->setRange(0,max_num_time_frames-1);

                float maxR = meshList[ mesh ]->getMeshData().data[raw].maxCoeff();
                float minR = meshList[ mesh ]->getMeshData().data[raw].minCoeff();
                float maxP = meshList[ mesh ]->getMeshData().data[processed].maxCoeff();
                float minP = meshList[ mesh ]->getMeshData().data[processed].minCoeff();
                float mx = std::max(maxP,maxR);
                float mn = std::min(minP,minR);
                ui->doubleSpinBox_mVmax->setValue( mx );
                ui->doubleSpinBox_mVmin->setValue( mn );

            }else if( meshList[ mesh ]->hasRawData() )
            {
                max_num_time_frames = meshList[ mesh ]->getMeshData().data[raw].cols();
                ui->horizontalSlider->setRange(0,max_num_time_frames-1);
                ui->timeFrameSpinBox->setRange(0,max_num_time_frames-1);

                float maxR = meshList[ mesh ]->getMeshData().data[raw].maxCoeff();
                float minR = meshList[ mesh ]->getMeshData().data[raw].minCoeff();
                ui->doubleSpinBox_mVmax->setValue( maxR );
                ui->doubleSpinBox_mVmin->setValue( minR );

            }else if( meshList[ mesh ]->hasProcessedData() )
            {
                max_num_time_frames = meshList[ mesh ]->getMeshData().data[processed].cols();
                ui->horizontalSlider->setRange(0,max_num_time_frames-1);
                ui->timeFrameSpinBox->setRange(0,max_num_time_frames-1);

                float maxP = meshList[ mesh ]->getMeshData().data[processed].maxCoeff();
                float minP = meshList[ mesh ]->getMeshData().data[processed].minCoeff();
                ui->doubleSpinBox_mVmax->setValue( maxP );
                ui->doubleSpinBox_mVmin->setValue( minP );
            }
        }
    }
}

void MainWindow::on_toolButton_play_clicked()
{
    timer.start( int(1000.0 / float(frames_per_sec)) );
}

void MainWindow::on_toolButton_pause_clicked()
{
    timer.stop();
}

void MainWindow::on_timeFramesPerSecSpinBox_valueChanged(int fps)
{
    frames_per_sec = fps;
}

void MainWindow::on_toolButton_front_clicked()
{
    if( max_num_time_frames==0 )
    {
        return;
    }
    timer.stop();
    current_time = 0;
    ui->horizontalSlider->setValue( current_time );
    ui->timeFrameSpinBox->setValue( current_time );
    emit time_frame_changed( current_time );
}

void MainWindow::on_toolButton_stepback_clicked()
{
    if( max_num_time_frames==0 )
    {
        return;
    }
    timer.stop();
    if( current_time == 0 )
    {
        current_time = max_num_time_frames-1;

    }else {

        current_time--;
    }
    ui->horizontalSlider->setValue( current_time );
    ui->timeFrameSpinBox->setValue( current_time );
    emit time_frame_changed( current_time );
}

void MainWindow::on_toolButton_stepforward_clicked()
{
    if( max_num_time_frames==0 )
    {
        return;
    }
    timer.stop();
    if( current_time == max_num_time_frames )
    {
        current_time = 0;

    }else{

        current_time++;
    }
    ui->horizontalSlider->setValue( current_time );
    ui->timeFrameSpinBox->setValue( current_time );
    emit time_frame_changed( current_time );
}

void MainWindow::on_toolButton_end_clicked()
{
    if( max_num_time_frames==0 )
    {
        return;
    }
    timer.stop();
    current_time = max_num_time_frames-1;
    ui->horizontalSlider->setValue( current_time );
    ui->timeFrameSpinBox->setValue( current_time );
    emit time_frame_changed( current_time );
}

void MainWindow::advanceTimer()
{
    //std::cout << "MainWindow::advanceTimer " << current_time << std::endl;
    if( max_num_time_frames==0 )
    {
        return;
    }
    ui->horizontalSlider->setValue( current_time );
    ui->timeFrameSpinBox->setValue( current_time );
    emit time_frame_changed( current_time );
    current_time++;
    if( frames_per_sec >100 )
    {
        current_time++;
    }
    if( frames_per_sec >200 )
    {
        current_time++;             //hacky, to skip frames
    }
    if( frames_per_sec >300 )
    {
        current_time++;
    }
    if( frames_per_sec >499 )
    {
        current_time++;
    }
    if( current_time > max_num_time_frames-1 )
    {
        current_time = 0;
    }
}

void MainWindow::setupControls()
{
    ui->toolButton_front      ->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->toolButton_stepback   ->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->toolButton_play       ->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->toolButton_pause      ->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->toolButton_stepforward->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->toolButton_end        ->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->horizontalSlider      ->setRange(0,0);

    ui->comboBox_default_studies->insertItem(0,  "Input new study");
    ui->comboBox_default_studies->insertItem(1,  "Sheep LA sinus, 64 electrode basket");
    ui->comboBox_default_studies->insertItem(2,  "Sheep LA macro-reentrant rota, 64 electrode basket ");
    ui->comboBox_default_studies->insertItem(3,  "Sheep LA sinus, 130 electrode basket");
    ui->comboBox_default_studies->insertItem(4,  "Sheep LA macro-reentrant rota, 130 electrode basket ");
    ui->comboBox_default_studies->insertItem(5,  "Melbourne GAM014 case");
    ui->comboBox_default_studies->insertItem(6,  "Melbourne flutter on GAM014 mesh");
//    ui->comboBox_default_studies->insertItem(5,  "Sheep RA exp_1 - CS paced, 64 electrode basket ");
//    ui->comboBox_default_studies->insertItem(6,  "Sheep LA with NarayanLab opensource basket data_1");
//    ui->comboBox_default_studies->insertItem(7,  "Human LA with Melbourne basket data - case 1, distal RSPV");
//    ui->comboBox_default_studies->insertItem(8,  "Melbourne Simulation - sinus");
//    ui->comboBox_default_studies->insertItem(9,  "Melbourne Simulation - rota");
//    ui->comboBox_default_studies->insertItem(10, "Melbourne Simulation - double rota");
//    ui->comboBox_default_studies->insertItem(11, "Melbourne Simulation - PV ectopics");
//    ui->comboBox_default_studies->insertItem(12, "Melbourne GAM008 case");
//    ui->comboBox_default_studies->insertItem(13, "Melbourne GAM003 case");

    //ui->comboBox_default_studies->insertItem(16, "Melbourne GAM002 case");
    //ui->comboBox_default_studies->insertItem(17, "Melbourne GAM010 case");
    ui->comboBox_default_studies->setCurrentIndex(14);
}

void MainWindow::on_comboBox_default_studies_currentIndexChanged(int index)
{
    switch( index )
    {
        case 0:
            ui->radioButton_atrial_signals_mode->setChecked( true );
            break;
        case 1:
            ui->radioButton_atrial_signals_mode->setChecked( true );
            ui->lineEdit_atrialGeom->setText(sheep_LA_geom_SR);
            ui->lineEdit_cathGeom->setText(cath_64_geom_SR);
            ui->lineEdit_atrialData->setText(sheep_LAdata_sinus);
            break;
        case 2:
            ui->radioButton_atrial_signals_mode->setChecked( true );
            ui->lineEdit_atrialGeom->setText(sheep_LA_geom_rotor);
            ui->lineEdit_cathGeom->setText(cath_64_geom_rotor);
            ui->lineEdit_atrialData->setText(sheep_LAdata_rota);
            break;
        case 3:
            ui->radioButton_atrial_signals_mode->setChecked( true );
            ui->lineEdit_atrialGeom->setText(sheep_LA_geom_SR);
            ui->lineEdit_cathGeom->setText(cath_130_geom_SR);
            ui->lineEdit_atrialData->setText(sheep_LAdata_sinus);
            break;
        case 4:
            ui->radioButton_atrial_signals_mode->setChecked( true );
            ui->lineEdit_atrialGeom->setText(sheep_LA_geom_rotor);
            ui->lineEdit_cathGeom->setText(cath_130_geom_rotor);
            ui->lineEdit_atrialData->setText(sheep_LAdata_rota);
            break;
        case 5:
            ui->radioButton_raw_basket_signal_mode->setChecked( true );
            ui->lineEdit_atrialGeom->setText(Melbourne_GAM014_openLAgeom);
            ui->lineEdit_cathGeom  ->setText(Melbourne_GAM014_opencathgeom);
            ui->lineEdit_cathData  ->setText(Melbourne_GAM014_data);
            break;
        case 6:
            ui->radioButton_raw_basket_signal_mode->setChecked( true );
            ui->lineEdit_atrialGeom->setText(Melbourne_GAM014_openLAgeom  );
            ui->lineEdit_cathGeom  ->setText(Melbourne_GAM014_opencathgeom);
            ui->lineEdit_cathData  ->setText(Melbourne_JZfluttter_data);
            break;
//        case 5:
//            ui->radioButton_atrial_signals_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(sheep_RA_geom_Exp1);
//            ui->lineEdit_cathGeom->setText(cath_64_geom);
//            ui->lineEdit_atrialData->setText(sheep_RA_data_Exp1);
//            break;
//        case 6:
//            ui->radioButton_raw_basket_signal_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(sheep_LA_geom);
//            ui->lineEdit_cathGeom->setText(cath_64_geom);
//            ui->lineEdit_cathData->setText(NarayanLab_exp1_basket_data);
//            break;
//        case 7:
//            ui->radioButton_raw_basket_signal_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(Melbourne_LAgeom_case1);
//            ui->lineEdit_cathGeom->setText(Melbourne_cath_distalRSPV_case1_geom);
//            ui->lineEdit_cathData->setText(Melbourne_cath_distalRSPV_case1_data);
//            break;
//        case 8:
//            ui->radioButton_atrial_signals_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(Melbourne_Simulation_geom);
//            ui->lineEdit_atrialData->setText(Melbourne_Simulation_sinus);
//            ui->lineEdit_cathGeom->setText(cath_130_geom);
//            break;
//        case 9:
//            ui->radioButton_atrial_signals_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(Melbourne_Simulation_geom);
//            ui->lineEdit_atrialData->setText(Melbourne_Simulation_rota);
//            ui->lineEdit_cathGeom->setText(cath_130_geom);
//            break;
//        case 10:
//            ui->radioButton_atrial_signals_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(Melbourne_Simulation_geom);
//            ui->lineEdit_atrialData->setText(Melbourne_Simulation_doubleRota);
//            ui->lineEdit_cathGeom->setText(cath_130_geom);
//            break;
//        case 11:
//            ui->radioButton_atrial_signals_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(Melbourne_Simulation_geom);
//            ui->lineEdit_atrialData->setText(Melbourne_Simulation_PVectopics);
//            ui->lineEdit_cathGeom->setText(cath_130_geom);
//            break;
//        case 12:
//            ui->radioButton_raw_basket_signal_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(Melbourne_GAM008_LAgeom);
//            ui->lineEdit_cathGeom  ->setText(Melbourne_GAM008_cathgeom);
//            ui->lineEdit_cathData  ->setText(Melbourne_GAM008_data);
//            break;
//        case 13:
//            ui->radioButton_raw_basket_signal_mode->setChecked( true );
//            ui->lineEdit_atrialGeom->setText(Melbourne_GAM003_LAgeom);
//            ui->lineEdit_cathGeom  ->setText(Melbourne_GAM003_cathgeom);
//            ui->lineEdit_cathData  ->setText(Melbourne_GAM003_data);
//            break;

    }
}

void MainWindow::on_radioButton_raw_basket_signal_mode_toggled( )
{
    ui->lineEdit_atrialData->clear();
    ui->lineEdit_atrialGeom->clear();
    ui->lineEdit_cathData->clear();
    ui->lineEdit_cathGeom->clear();

    ui->lineEdit_atrialData->setEnabled( false );
    ui->toolButton_atrialData->setEnabled( false );
    ui->lineEdit_atrialGeom->setEnabled( true );
    ui->lineEdit_cathData->setEnabled( true );
    ui->toolButton_cathData->setEnabled( true );
    ui->lineEdit_cathGeom->setEnabled( true );

    this->setupWindows();
}

void MainWindow::on_radioButton_atrial_signals_mode_toggled( )
{
    ui->lineEdit_atrialData->clear();
    ui->lineEdit_atrialGeom->clear();
    ui->lineEdit_cathData->clear();
    ui->lineEdit_cathGeom->clear();

    ui->lineEdit_atrialData->setEnabled( true );
    ui->toolButton_atrialData->setEnabled( true );
    ui->lineEdit_atrialGeom->setEnabled( true );
    ui->lineEdit_cathData->setEnabled( false );
    ui->toolButton_cathData->setEnabled( false );
    ui->lineEdit_cathGeom->setEnabled( true );

    this->setupWindows();
}

void MainWindow::on_timeFrameSpinBox_valueChanged(int timeframe)
{
    current_time = timeframe;
    emit time_frame_changed( current_time );
    ui->horizontalSlider->setEnabled( false );
    ui->horizontalSlider->setValue( current_time );
    ui->horizontalSlider->setEnabled( true );
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    current_time = value;
    emit time_frame_changed( current_time );
    ui->timeFrameSpinBox->setEnabled( false );
    ui->timeFrameSpinBox->setValue( current_time );
    ui->timeFrameSpinBox->setEnabled( true );
}

void MainWindow::on_toolButton_win1_clicked()
{
    GeomwinSettings* settings_win = new GeomwinSettings( ui->openGLWidget1->geomwinSettings );
    connect( settings_win, &GeomwinSettings::geomwin_settings_changed_signal, ui->openGLWidget1, &Geomwindow::updateLocalSlot );
    connect( settings_win, &GeomwinSettings::geomwin_settings_changed_signal, this, &MainWindow::updateGeomwinLabel );
    settings_win->exec();
    settings_win->deleteLater();
}

void MainWindow::updateGeomwinLabel()
{
    if       ( ui->openGLWidget1->geomwinSettings.data_view_atrium == DataType::raw )
    {
        ui->label_win1->setText( "Input atrial potential map" );
    }else if ( ui->openGLWidget1->geomwinSettings.data_view_atrium == DataType::processed )
    {
        ui->label_win1->setText( "Inverse atrial potential map" );
    }else if ( ui->openGLWidget1->geomwinSettings.data_view_atrium == DataType::acti_history )
    {
        ui->label_win1->setText( "Activation history map" );
    }else if ( ui->openGLWidget1->geomwinSettings.data_view_atrium == DataType::phase )
    {
        ui->label_win1->setText( "Phase map" );
    }else if ( ui->openGLWidget1->geomwinSettings.data_view_atrium == DataType::no_data )
    {
        ui->label_win1->setText( "Data off" );
    }

    if       ( ui->openGLWidget2->geomwinSettings.data_view_atrium == DataType::raw )
    {
        ui->label_win2->setText( "Input atrial potential map" );
    }else if ( ui->openGLWidget2->geomwinSettings.data_view_atrium == DataType::processed )
    {
        ui->label_win2->setText( "Inverse atrial potential map" );
    }else if ( ui->openGLWidget2->geomwinSettings.data_view_atrium == DataType::acti_history )
    {
        ui->label_win2->setText( "Activation history map" );
    }else if ( ui->openGLWidget2->geomwinSettings.data_view_atrium == DataType::phase )
    {
        ui->label_win2->setText( "Phase map" );
    }else if ( ui->openGLWidget2->geomwinSettings.data_view_atrium == DataType::no_data )
    {
        ui->label_win2->setText( "Data off" );
    }

    if       ( ui->openGLWidget3->geomwinSettings.data_view_atrium == DataType::raw )
    {
        ui->label_win3->setText( "Input atrial potential map" );
    }else if ( ui->openGLWidget3->geomwinSettings.data_view_atrium == DataType::processed )
    {
        ui->label_win3->setText( "Inverse atrial potential map" );
    }else if ( ui->openGLWidget3->geomwinSettings.data_view_atrium == DataType::acti_history )
    {
        ui->label_win3->setText( "Activation history map" );
    }else if ( ui->openGLWidget3->geomwinSettings.data_view_atrium == DataType::phase )
    {
        ui->label_win3->setText( "Phase map" );
    }else if ( ui->openGLWidget3->geomwinSettings.data_view_atrium == DataType::no_data )
    {
        ui->label_win3->setText( "Data off" );
    }

}

void MainWindow::on_toolButton_win2_clicked()
{
    GeomwinSettings* settings_win = new GeomwinSettings( ui->openGLWidget2->geomwinSettings );
    connect( settings_win, &GeomwinSettings::geomwin_settings_changed_signal, ui->openGLWidget2, &Geomwindow::updateLocalSlot );
    connect( settings_win, &GeomwinSettings::geomwin_settings_changed_signal, this, &MainWindow::updateGeomwinLabel );
    settings_win->exec();
    settings_win->deleteLater();
}

void MainWindow::on_toolButton_win3_clicked()
{
    GeomwinSettings* settings_win = new GeomwinSettings( ui->openGLWidget3->geomwinSettings );
    connect( settings_win, &GeomwinSettings::geomwin_settings_changed_signal, ui->openGLWidget3, &Geomwindow::updateLocalSlot );
    connect( settings_win, &GeomwinSettings::geomwin_settings_changed_signal, this, &MainWindow::updateGeomwinLabel );
    settings_win->exec();
    settings_win->deleteLater();
}

void MainWindow::on_doubleSpinBox_mVmax_valueChanged(double max)
{
    emit mesh_data_max_min_signal( max, ui->doubleSpinBox_mVmin->value(), !ui->checkBox_autoScaling->isChecked() );
}

void MainWindow::on_doubleSpinBox_mVmin_valueChanged(double min)
{
    emit mesh_data_max_min_signal( ui->doubleSpinBox_mVmax->value(), min, !ui->checkBox_autoScaling->isChecked() );
}

void MainWindow::on_checkBox_showAtrialElectrogram_toggled(bool checked)
{
    if( checked )
    {
        ui->checkBox_showAtrialRaw      ->setEnabled(true);
        ui->checkBox_showAtrialProcessed->setEnabled(true);
        ui->checkBox_showAtrialPhase    ->setEnabled(true);
        ui->checkBox_showPickActivation ->setEnabled(true);
        ui->checkBox_showAtrialRaw      ->setChecked(true);
        ui->checkBox_showAtrialProcessed->setChecked(true);
        ui->checkBox_showAtrialPhase    ->setChecked(true);
        ui->checkBox_showPickActivation ->setChecked(true);
    }else{
        ui->checkBox_showAtrialRaw      ->setChecked(false);
        ui->checkBox_showAtrialProcessed->setChecked(false);
        ui->checkBox_showAtrialPhase    ->setChecked(false);
        ui->checkBox_showPickActivation ->setChecked(false);
        ui->checkBox_showAtrialRaw      ->setEnabled(false);
        ui->checkBox_showAtrialProcessed->setEnabled(false);
        ui->checkBox_showAtrialPhase    ->setEnabled(false);
        ui->checkBox_showPickActivation ->setEnabled(false);
    }
}

void MainWindow::on_checkBox_showNearElectrode_toggled(bool checked)
{
    if( checked )
    {
        ui->checkBox_showNearElectrode_raw      ->setEnabled(true);
        ui->checkBox_showNearElectrode_processed->setEnabled(true);
        ui->checkBox_showNearElectrode_raw      ->setChecked(true);
        ui->checkBox_showNearElectrode_processed->setChecked(true);
    }else{
        ui->checkBox_showNearElectrode_raw      ->setChecked(false);
        ui->checkBox_showNearElectrode_processed->setChecked(false);
        ui->checkBox_showNearElectrode_raw      ->setEnabled(false);
        ui->checkBox_showNearElectrode_processed->setEnabled(false);
    }
}

void MainWindow::on_checkBox_autoScaling_toggled(bool checked)
{
    emit mesh_data_max_min_signal( ui->doubleSpinBox_mVmax->value(), ui->doubleSpinBox_mVmin->value(), !checked );
}

void MainWindow::on_toolButton_actiHistMapSettings_clicked()
{
    timer.stop();
    DSP_ActivationHistorySettings* dialog = new DSP_ActivationHistorySettings( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void MainWindow::on_toolButton_inverseSettings_clicked()
{
    timer.stop();
    DSP_InverseSettings* dialog = new DSP_InverseSettings( processing.inverse_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void MainWindow::on_toolButton_phaseMapSettings_clicked()
{
    timer.stop();
    DSP_PhaseSettings* dialog = new DSP_PhaseSettings( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}
