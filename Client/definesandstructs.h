#ifndef DEFINESANDSTRUCTS_H
#define DEFINESANDSTRUCTS_H

#include <Eigen/Dense>
#include <QString>
#include "enums.h"


struct Filter_vars
{

private:
    //General
    double sample_rate;

    //QRS subtraction variables
    int    QRSsub_num_scales;

    //Atrial reconstruction variables
    int    atrial_recon_num_scales;

public:

    //General
    //double sample_rate;  private as needs to be set by function

    //Wavelet drift removal
    bool bool_use_DWTdrift_remove;
    Eigen::Array<bool,Eigen::Dynamic,1> DWTdrift_levels_to_keep;
    Eigen::VectorXd DWTdrift_levels_std_weights; //thresholds for removing coefficients

    //Bandstop variables
    int    bandstop_order;
    double bandstop_upper;
    double bandstop_lower;
    bool   bool_use_bandstop;

    //Highpass / lowpass variables
    double highpass_cutoff;
    int    highpass_order;
    bool   bool_use_highpass;
    double lowpass_cutoff;
    int    lowpass_order;
    bool   bool_use_lowpass;

    //QRS subtraction variables
    int QRS_channel;
    double QRS_threshold;
    bool QRS_direction; ///false=negative, true=positive
    double QRSsub_scale_res;
    double QRSsub_scale_min;
    int    QRSsub_wavelet_order;
    //int    QRSsub_num_scales;  private as needs to be set by function
    double QRSsub_preQRSwin;
    double QRSsub_postQRSwin;
    Eigen::MatrixXd QRSsub_Tuk_alphaParams;        //should make these private too
    Eigen::VectorXi QRSsub_Tuk_subLengths;
    Eigen::VectorXd QRSsub_Tuk_subLenHeightRatios;


    //Atrial activation finder variables
    double atrial_search_scale_res;
    double atrial_search_scale_min;
    int    atrial_search_wavelet_order;
    int    atrial_search_num_scales;
    double atrial_search_lag_win_length_secs;
    double atrial_search_step_win_length_secs;
    double atrial_search_threshold;
    double atrial_search_min_activation_interval_secs;

    //Atrial reconstruction variables
    int atrial_recon_wavelet_order;
    //int atrial_recon_num_scales;          private as needs to be set by function
    double atrial_recon_scale_res;
    double atrial_recon_scale_min;
    double atrial_recon_pre_win_secs;
    double atrial_recon_post_win_secs;
    Eigen::VectorXd atrial_recon_alphaParams;      //should make these private too
    Eigen::VectorXd atrial_recon_subLenHeightRatios; //should make these private too


    //Activation history map variables
    double activation_history_win_secs;
    int    activation_history_max_val;


    //Phase map
    double phase_base_period_secs;



    //Constructor - initialise filter default variables
    Filter_vars() :

        //General
        sample_rate(2036.5),

        //Wavelet drift removal
        bool_use_DWTdrift_remove(true),
        DWTdrift_levels_to_keep( Eigen::Array<bool,Eigen::Dynamic,1>::Ones(10) ),
        DWTdrift_levels_std_weights( Eigen::VectorXd::LinSpaced(10,0,1) ),

        //Bandstop variables
        bandstop_order(1),
        bandstop_upper(51.0),
        bandstop_lower(49.0),
        bool_use_bandstop(true),

        //Highpass / lowpass variables
        highpass_cutoff(4.0),
        highpass_order(1),
        bool_use_highpass(true),
        lowpass_cutoff(145.0),
        lowpass_order(1),
        bool_use_lowpass(true),

        //QRS identification and subtraction
        QRS_channel(1),
        QRS_threshold(2.0),
        QRS_direction(false),
        QRSsub_scale_res(0.5),
        QRSsub_scale_min( 2.0 * (1.0 / sample_rate) ),
        QRSsub_wavelet_order (2),
        QRSsub_num_scales(10),                  //if changing the default then change the defaults in the Tukey _ Eigen Vector param vector initialisers
        QRSsub_preQRSwin (0.050),                    //msec
        QRSsub_postQRSwin(0.400),                    //msec
        QRSsub_Tuk_alphaParams( Eigen::VectorXd::Ones( 10 ) ),
        QRSsub_Tuk_subLengths( Eigen::VectorXi::LinSpaced( 10, 0.012 * sample_rate, 0.150 * sample_rate ) ),
        QRSsub_Tuk_subLenHeightRatios( Eigen::VectorXd::LinSpaced(10, 1.0, 0.0) ),


        //Atrial activity identification
        atrial_search_scale_res(0.3),
        atrial_search_scale_min(2.0 * (1.0 / sample_rate)),
        atrial_search_wavelet_order(1),
        atrial_search_num_scales(10),
        atrial_search_lag_win_length_secs(0.100), //secs
        atrial_search_step_win_length_secs(0.010), //secs
        atrial_search_threshold(5.0),
        atrial_search_min_activation_interval_secs(0.100), //secs

        //Atrial activity reconsturction
        atrial_recon_wavelet_order(2),
        atrial_recon_num_scales(10),
        atrial_recon_scale_res(0.5),
        atrial_recon_scale_min( 2.0 * (1.0 / sample_rate) ),
        atrial_recon_pre_win_secs(0.080),
        atrial_recon_post_win_secs(0.080),
        atrial_recon_alphaParams( Eigen::VectorXd::Ones( atrial_recon_num_scales ) ),
        atrial_recon_subLenHeightRatios( Eigen::VectorXd::Constant( atrial_recon_num_scales, 1.0 ) ),


        //Activation history map variables
        activation_history_win_secs(0.05),
        activation_history_max_val(50),


        //Phase map
        phase_base_period_secs(0.160)

    {

    }

    //Functions for things that need to alter multiple variables if changed
    void changeQRSSub_NumScales( int num )
    {
        QRSsub_num_scales             = num;
        QRSsub_Tuk_alphaParams        = Eigen::VectorXd::Ones( num );
        QRSsub_Tuk_subLengths         = Eigen::VectorXi::LinSpaced( num, 0.012 * sample_rate, 0.150 * sample_rate );
        QRSsub_Tuk_subLenHeightRatios = Eigen::VectorXd::Constant( num, 1.0 );
    }
    int getNumQRSSubScales() const
    {
        return QRSsub_num_scales;
    }
    void changeSampleRate( double sample_rate_in )
    {
        //Set
        sample_rate             = sample_rate_in;

        //Reset things that depend on sample rate for their default value
        QRSsub_scale_min        = 2.0 * (1.0 / sample_rate_in);
        QRSsub_Tuk_subLengths   = Eigen::VectorXi::LinSpaced( 10, 0.012*sample_rate_in, 0.150*sample_rate_in );
        atrial_search_scale_min = 2.0 * (1.0 / sample_rate_in);
        atrial_recon_scale_min  = 2.0 * (1.0 / sample_rate);
        atrial_recon_alphaParams= Eigen::VectorXd::Ones( atrial_recon_num_scales );
    }
    double getSampleRate() const
    {
        return sample_rate;
    }

    void setNumAtrialReconScales( int num_in )
    {
        //set
        atrial_recon_num_scales = num_in;

        //Update
        atrial_recon_alphaParams = Eigen::VectorXd::Ones( num_in );
        atrial_recon_subLenHeightRatios = Eigen::VectorXd::Ones( num_in );
    }
    int getNumAtrialReconScales() const
    {
        return atrial_recon_num_scales;
    }

};



enum SourceGen
{
    deflate, inflate, alongNormals, fromCentre
};

enum LambdaMethod
{
    userDefined, CRESO
};



struct Inverse_vars
{
private:
    int    num_cath_cannels;

public:
    int    num_atrial_nodes;
    Eigen::Array<bool,Eigen::Dynamic,1> cath_channels_to_use;

    SourceGen method;
    float atrial_inverse_source_downsampling;
    float atrial_forward_source_downsampling;
    float source_scaling[MeshType::max_num_meshes];
    LambdaMethod lambdaMethod;
    float lambda = 0.05;


    //Functions
    Inverse_vars() :
        //General
        num_cath_cannels(0),
        num_atrial_nodes(0),
        cath_channels_to_use( Eigen::Array<bool,Eigen::Dynamic,1>::Ones(num_cath_cannels) ),
        method( SourceGen::alongNormals ),
        atrial_inverse_source_downsampling(1.0),
        atrial_forward_source_downsampling(1.0),
        source_scaling{0.16,0.16},
        lambdaMethod( LambdaMethod::CRESO),
        lambda(0.05)
    {

    }

    void setNumCathChannels( int num_in )
    {
        //Set
        num_cath_cannels = num_in;

        //Update variables that depend on it
        cath_channels_to_use = Eigen::Array<bool,Eigen::Dynamic,1>::Ones( num_in );
    }
    int getNumCathChannels()
    {
        return num_cath_cannels;
    }

};

enum Mode
{
    catheter_input,
    atrial_input
};


struct General_vars
{
    Mode mode;

};




#ifdef __linux__

static QString base_path = "/hpc/nsun286/invMap/Client/Resources/test_data/";

#endif

#ifdef __APPLE__

static QString base_path = "/Users/nicholassunderland/Documents/2.Medical_work/4.ABI/GUI_invMap/Client/Resources/test_data/";

#endif

///Sinus
static QString sheep_LA_geom_SR   = base_path + "SheepLA_simulated_sinus/sheepGeom_LA_1529.obj";
static QString cath_64_geom_SR    = base_path + "SheepLA_simulated_sinus/cath_geom_64.obj";
static QString cath_130_geom_SR   = base_path + "SheepLA_simulated_sinus/cath_geom_130.obj";
static QString sheep_LAdata_sinus = base_path + "SheepLA_simulated_sinus/sheepData_1529sinus.txt";

///Rotor
static QString sheep_LA_geom_rotor = base_path + "SheepLA_simulated_rotor/sheepGeom_LA_1529.obj";
static QString cath_64_geom_rotor  = base_path + "SheepLA_simulated_rotor/cath_geom_64.obj";
static QString cath_130_geom_rotor = base_path + "SheepLA_simulated_rotor/cath_geom_130.obj";
static QString sheep_LAdata_rota   = base_path + "SheepLA_simulated_rotor/sheepLA_simulated_rotor_data.txt";

///GAM014 with flutter data
static QString Melbourne_GAM014_openLAgeom   = base_path + "JZ_flutter/LAlowDenseOpen.obj"     ;
static QString Melbourne_GAM014_opencathgeom = base_path + "JZ_flutter/cathgeomLowDense.obj"   ;
static QString Melbourne_JZfluttter_data     = base_path + "JZ_flutter/JZflutter_basketData_raw_5secs.txt";

///GAM014 with actual data
static QString Melbourne_GAM014_LAgeom       = base_path + "GAM014/GAM014_LAlowDenseOpen.obj";
static QString Melbourne_GAM014_cathgeom     = base_path + "GAM014/GAM014_cathgeomLowDense.obj";
static QString Melbourne_GAM014_data         = base_path + "GAM014/GAM014_basketData_raw.txt";


//static QString sheep_RA_geom_Exp1 = base_path + "sheep_RA_exp1.obj";
//static QString sheep_RA_data_Exp1 = base_path + "exp1_sheep_RA_paced_data.txt";
//static QString human_LA_geom      = base_path + "human_LA_2333pts.obj";
//static QString NarayanLab_exp1_basket_data = base_path + "NarayanLab_exp1_raw_basket_signals.txt";
//static QString Melbourne_LAgeom_case1 = base_path + "LAgeom_case1_Melbourne.obj";
//static QString Melbourne_cath_distalRSPV_case1_geom = base_path + "Cath64geom_case1_distalRSPV_Melbourne.obj";
//static QString Melbourne_cath_distalRSPV_case1_data = base_path + "data_1kHz_distalRSPV_case1_Melbourne_5secs.txt";

//static QString Melbourne_Simulation_geom       = base_path + "Melb_case1_simulated.obj";
//static QString Melbourne_Simulation_sinus      = base_path + "sinus_goldstandard_data_melb_case1.txt";
//static QString Melbourne_Simulation_rota       = base_path + "rota_goldstandard_data_melb_case1.txt";
//static QString Melbourne_Simulation_doubleRota = "";
//static QString Melbourne_Simulation_PVectopics = base_path + "PVectopics_goldstandard_data_melb_case1.txt";

//static QString Melbourne_GAM008_LAgeom   = base_path + "GAM008_LAgeom_trimmed.obj";
//static QString Melbourne_GAM008_cathgeom = base_path + "GAM008_cathGeom.obj";
//static QString Melbourne_GAM008_data     = base_path + "GAM008_data_1kHz_0-5secs.txt";

//static QString Melbourne_GAM003_LAgeom   = base_path + "GAM003_LAGeom_trimmed.obj";
//static QString Melbourne_GAM003_cathgeom = base_path + "GAM003_cathGeom.obj";
//static QString Melbourne_GAM003_data     = base_path + "GAM003_basketData_raw.txt";










#endif // DEFINESANDSTRUCTS_H
