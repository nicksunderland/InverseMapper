#include "signalprocessingdialog.h"
#include "ui_signalprocessingdialog.h"
#include "DSP_settings/dsp_noisesettings.h"
#include "DSP_settings/dsp_qrsfinder.h"
#include "DSP_settings/dsp_qrssubtraction.h"
#include "DSP_settings/dsp_atrialactivationfinder.h"
#include "DSP_settings/dsp_signalreconstructor.h"
#include "DSP_settings/dsp_inversesettings.h"
#include "mainwindow.h"
#include <iostream>

SignalProcessingDialog::SignalProcessingDialog(Processing &processing_ref,
                                               std::vector< QSharedPointer<Mesh> >& meshList_in,
                                               QDialog *parent) :
    QDialog(parent),
    ui(new Ui::SignalProcessingDialog),
    processing(processing_ref),
    meshList( meshList_in )
{
    std::cout << "SignalProcessingDialog --> ctor()" << std::endl;

    ui->setupUi(this);

    //Start at cath index 0 and get nearest atrial index
    int nodeInds[MeshType::max_num_meshes];
        nodeInds[MeshType::catheter      ] = 0;
        nodeInds[MeshType::atrium        ] = processing.findNearestNodes( Eigen::VectorXi::Zero(1), catheter, atrium )(0);

    //Raw
    EMG_raw = new Pickwindow (PickWinType::EMG_raw, nodeInds, meshList, this );
    connect( meshList[catheter].get(), &Mesh::mesh_buffers_updated_signal, EMG_raw, &Pickwindow::updatePickDataBuffers );
    //connect( this, &SignalProcessingDialog::SPD_user_scaling_signal, EMG_raw, &Pickwindow::updateTransformMatrixUSER, Qt::UniqueConnection );
    EMG_raw->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->verticalLayout_basketCatheterElectrogram->addWidget( EMG_raw );

    //Filtered
    EMG_filtered = new Pickwindow (PickWinType::EMG_filtered, nodeInds, meshList, this );
    connect( meshList[catheter].get(), &Mesh::mesh_buffers_updated_signal, EMG_filtered, &Pickwindow::updatePickDataBuffers );
    connect( this, &SignalProcessingDialog::SPD_user_scaling_signal, EMG_filtered, &Pickwindow::updateTransformMatrixUSER, Qt::UniqueConnection );
    EMG_filtered ->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->verticalLayout_filteredElectrogram->addWidget( EMG_filtered );

    //Atrial
    EMG_atrial = new Pickwindow (PickWinType::EMG_atrial, nodeInds, meshList, this );
    connect( meshList[atrium].get(), &Mesh::mesh_buffers_updated_signal, EMG_atrial, &Pickwindow::updatePickDataBuffers );
    connect( this, &SignalProcessingDialog::SPD_user_scaling_signal, EMG_atrial, &Pickwindow::updateTransformMatrixUSER, Qt::UniqueConnection );
    EMG_atrial->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->verticalLayout_closestEndocardialElectrogram->addWidget( EMG_atrial );

    //Set sample rate & basket spin boxes
    ui->doubleSpinBox_sampleRate->setValue( processing.filter_vars.getSampleRate() );
    ui->spinBox_basketElectrode->setRange( 1, meshList[ catheter ]->getMeshGeom().V.rows() );

    //Connect to processing
    connect( ui->pushButton_clearFiltering,      &QPushButton::clicked, &processing, &Processing::clearFiltering );
    connect( ui->pushButton_run_inverse,         &QPushButton::clicked, &processing, &Processing::runInverse );
    connect( ui->pushButton_findQRS ,            &QPushButton::clicked, &processing, &Processing::findQRS );
    connect( ui->pushButton_QRSsubtract ,        &QPushButton::clicked, &processing, &Processing::subtractQRS );
    connect( ui->pushButton_noiseFilter ,        &QPushButton::clicked, &processing, &Processing::noiseFilter );
    connect( ui->pushButton_cathSigReconstruct,  &QPushButton::clicked, &processing, &Processing::reconstructSignal );
    connect( ui->pushButton_findCathActivations, &QPushButton::clicked, &processing, &Processing::findCathActivations );
    connect( ui->pushButton_findEndoActivations, &QPushButton::clicked, &processing, &Processing::findEndoActivations );


    ui->checkBox_autoscaling->setChecked( true );

    std::cout << "SignalProcessingDialog <-- ctor()" << std::endl;
}

SignalProcessingDialog::~SignalProcessingDialog()
{
    delete ui;
}

void SignalProcessingDialog::on_toolButton_noiseFilterSettings_clicked()
{
    DSP_NoiseSettings* dialog = new DSP_NoiseSettings( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void SignalProcessingDialog::on_toolButton_findQRSsettings_clicked()
{
    DSP_QRSfinder* dialog = new DSP_QRSfinder( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void SignalProcessingDialog::on_toolButton_QRSsubtract_clicked()
{
    DSP_QRSsubtraction* dialog = new DSP_QRSsubtraction( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void SignalProcessingDialog::on_toolButton_findCathActivations_clicked()
{
    DSP_AtrialActivationFinder* dialog = new DSP_AtrialActivationFinder( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void SignalProcessingDialog::on_toolButton_findEndoActivations_clicked()
{
    DSP_AtrialActivationFinder* dialog = new DSP_AtrialActivationFinder( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void SignalProcessingDialog::on_toolButton_cathSigReconstruct_clicked()
{
    DSP_SignalReconstructor* dialog = new DSP_SignalReconstructor( processing.filter_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void SignalProcessingDialog::on_toolButton_inverse_clicked()
{
    DSP_InverseSettings* dialog = new DSP_InverseSettings( processing.inverse_vars, this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->exec();
}

void SignalProcessingDialog::on_checkBox_useSignalForInverse_toggled(bool checked)
{
    //Color text red if signal bad / not going to be used
    if( checked ) { ui->label_useSignalForInverse->setStyleSheet("QLabel { color : black; }"); }
    else          { ui->label_useSignalForInverse->setStyleSheet("QLabel { color : red;   }"); }

    int current_channel_idx = ui->spinBox_basketElectrode->value()-1;
    processing.inverse_vars.cath_channels_to_use( current_channel_idx ) = checked;
}

void SignalProcessingDialog::on_doubleSpinBox_sampleRate_valueChanged(double sample_rate_in)
{
    processing.filter_vars.changeSampleRate( sample_rate_in );
}

void SignalProcessingDialog::on_spinBox_basketElectrode_valueChanged(int basket_electrode)
{
    int cath_channel_index = basket_electrode-1;

    //Get signal quality assessment (just a bool to use or not) from filter vars and set checkbox
    ui->checkBox_useSignalForInverse->setChecked( processing.inverse_vars.cath_channels_to_use( cath_channel_index ) );

    //Get the closest atrial node
    //std::cout << "processing.findNearestNodes BEFORE" << std::endl;
    int atrial_node_index = processing.findNearestNodes( Eigen::VectorXi::Constant(1,cath_channel_index), MeshType::catheter, MeshType::atrium )(0);
    //std::cout << "processing.findNearestNodes AFTER" << std::endl;

    //Emit signals
    int chans[MeshType::max_num_meshes];
    chans[MeshType::atrium]   = atrial_node_index;
    chans[MeshType::catheter] = cath_channel_index;

    EMG_raw     ->setChannelIndices(chans);
    EMG_filtered->setChannelIndices(chans);
    EMG_atrial  ->setChannelIndices(chans);
}

void SignalProcessingDialog::on_checkBox_autoscaling_toggled(bool checked)
{
    if( checked )
    {
        ui->doubleSpinBox_mVmax->setEnabled( false );
        ui->doubleSpinBox_mVmin->setEnabled( false );
        emit SPD_user_scaling_signal( ui->doubleSpinBox_mVmax->value(),
                                      ui->doubleSpinBox_mVmin->value(),
                                      !checked );
    }else {
        ui->doubleSpinBox_mVmax->setEnabled( true );
        ui->doubleSpinBox_mVmin->setEnabled( true );
        emit SPD_user_scaling_signal( ui->doubleSpinBox_mVmax->value(),
                                      ui->doubleSpinBox_mVmin->value(),
                                      !checked );
    }
}

void SignalProcessingDialog::on_doubleSpinBox_mVmin_valueChanged(double mn)
{
    emit SPD_user_scaling_signal( ui->doubleSpinBox_mVmax->value(),
                                  mn,
                                  !ui->checkBox_autoscaling->isChecked() );
}

void SignalProcessingDialog::on_doubleSpinBox_mVmax_valueChanged(double mx)
{
    emit SPD_user_scaling_signal( mx,
                                  ui->doubleSpinBox_mVmin->value(),
                                  !ui->checkBox_autoscaling->isChecked() );
}
