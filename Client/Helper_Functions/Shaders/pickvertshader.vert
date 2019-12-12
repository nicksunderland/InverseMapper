#version 120

/***INPUT VARIABLES called 'attributes'***/
attribute vec3 pickVertexPos;

/***UNIFORMS - another way to pass data from our programme(CPU) to the GPU***/
//unifroms different from attributes as uniforms are 'global' so no need to go through the vertex shader with it, although could define it there if we needed it in the vertex shader
uniform mat4 modelMatrix;    //need to set this in OpenGL code
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;


void main(void) {

    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4(pickVertexPos, 1.0);


}
