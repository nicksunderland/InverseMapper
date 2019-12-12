#version 120

/***INPUT VARIABLES*** come from the output variables of the vertex shader***/
varying vec3 normal_to_fs;
varying vec3 fragPosition;
varying vec4 objectColor;


/***UNIFORMS - another way to pass data from our programme(CPU) to the GPU***/
//unifroms different from attributes as uniforms are 'global' so no need to go through the vertex shader with it, although could define it there if we needed it in the vertex shader
//need to set this in OpenGL code
//uniform float lightColor;
//uniform vec3 lightPosition;
//uniform vec3 viewPosition;


/***OUTPUT VARIABLE***/
//is the gl_FragColor


void main()
{
    //Uniforms that I made fixed
    vec3 lightColor   = vec3(1.0, 1.0, 1.0);
    vec3 lightPosition= vec3(0.0, 500.0, 1000.0);
    vec3 viewPosition = vec3(0.0, 0.0, 0.0);

    //Diffuse lighting
    vec3 norm = normalize(normal_to_fs);
    vec3 lightDirection = normalize(lightPosition - fragPosition);
    float diff = max(dot(norm, lightDirection), 0.0); //'max' makes sure the result is never negative
    vec3 diffuse = diff * lightColor;

    //Ambient light
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    //Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);  //last value is the shininess
    vec3 specular = specularStrength * spec * lightColor;

    //Final result
    vec3 result  = (ambient + diffuse + specular);
    vec4 result4 = vec4( result, 1 );
    gl_FragColor = result4 * objectColor; //vertexColor_out_vs; // vertexColor; //vec4(1.0f, 0.5f, 0.2f, 1.0f);
}



