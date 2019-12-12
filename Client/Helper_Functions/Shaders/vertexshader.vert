#version 120

/***INPUT VARIABLES called 'attributes'***/
attribute vec3 vertexPos;         //0
attribute vec3 vertexNorm;        //1     if changing the order here must change the enum at the top of the Geomwindow.ccp
attribute float vertexPotential;  //2
attribute float channelRange;     //3


/***UNIFORMS - another way to pass data from our programme(CPU) to the GPU***/
//unifroms different from attributes as uniforms are 'global' so no need to go through the vertex shader with it, although could define it there if we needed it in the vertex shader
uniform mat4 modelMatrix;    //need to set this in OpenGL code
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float scaleMax_in;
uniform float scaleMin_in;
uniform float minRangeCutoff;
uniform bool cutoff_use_grey;


/***OUTPUT VARIABLES called 'varying'***/
//gl_Position (is a build in standard output variable so don't need to declare)
//output has to be a vec4 vector as the fragment shader (which comes next in
//the pipeline) needs vec4 input
varying vec3 normal_to_fs;
varying vec3 fragPosition;
varying vec4 objectColor;




//-------------------------MATLAB JET------------------------//
float interpolate( float val, float y0, float x0, float y1, float x1 ) {
    return (val-x0)*(y1-y0)/(x1-x0) + y0;
}
float base( float val ) {
    if ( val <= -0.75 ) return 0;  // was -0.75
    else if ( val <= -0.25 ) return interpolate( val, 0.0, -0.75, 1.0, -0.25 );
    else if ( val <=  0.25 ) return 1.0;
    else if ( val <=  0.75 ) return interpolate( val, 1.0, 0.25, 0.0, 0.75 );
    else return 0.0;
}
float red( float gray ) {
    return base( gray - 0.5 );
}
float green( float gray ) {
    return base( gray );
}
float blue( float gray ) {
    return base( gray + 0.5 );
}

//---------------------------------------------------------//

float rangePotential( float potential ) {
    if( vertexPotential > scaleMax_in ) return scaleMax_in;
    else if( vertexPotential < scaleMin_in ) return scaleMin_in;
    else return potential;
}

float convertPotential( float potential ){
    return ( (((potential-scaleMin_in)/(scaleMax_in-scaleMin_in)) * (0.75 - -0.75) ) + -0.75);
}




void main()
{

    //Send vertex position
    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPos, 1.0);

    //Send fragment position in world space, through to FS
    fragPosition = vec3(modelMatrix * vec4(vertexPos, 1.0));

    //Send normal
//    normal_to_fs = vertexNorm;
    normal_to_fs = vec3( viewMatrix * modelMatrix * vec4(vertexNorm, 0) );
    // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not(but GSLS 120 doesn't have it).

    //Send potential color
    float alpha     = 1.0;
    float min_alpha = 0.1;
    if( channelRange < minRangeCutoff )
    {
        alpha = (1.0 - min_alpha) * ( channelRange / minRangeCutoff ) + min_alpha;
    }

    if( vertexPotential == 0.00001 || ( channelRange < minRangeCutoff && cutoff_use_grey == true ) ){ //just a 'code' set by the calculateActivation time function in Mesh:: to color mesh grey if outside activation time window   ||
        objectColor = vec4( 0.45, 0.45, 0.45, 1.0 );
    }else{
        objectColor = vec4( red  (  convertPotential( rangePotential(vertexPotential) )  ),
                            green(  convertPotential( rangePotential(vertexPotential) )  ),
                            blue (  convertPotential( rangePotential(vertexPotential) )  ),
                            alpha );
    }
}

