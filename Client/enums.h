#ifndef ENUMS_H
#define ENUMS_H

enum PickWinType
{
    EMG_raw, EMG_filtered, EMG_atrial
};
enum MeshType
{
    atrium,
    catheter,
    max_num_meshes
};
enum DataType
{
    raw,
    processed,
    acti_history,
    phase,
    QRS_actis,
    atrial_actis,
    no_data,
    max_data_types
};
enum MeshStats
{
    x_min, x_max, y_min, y_max, z_min, z_max
};
enum AA{    //atrial activation, enums for indexing the atrial activation matrix
    time_idx,
    channel,
    wav_scale,
    power,
    keep_bool
};

#endif // ENUMS_H
