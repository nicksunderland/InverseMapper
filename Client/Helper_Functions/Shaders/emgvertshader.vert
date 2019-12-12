#version 120

attribute vec2 position;
uniform mat4 transMatrix;
//uniform vec2 min_max_x_axis;  //the timepoint min and max
//uniform vec2 min_max_y_axis;  //the potential data min and max


void main(void) {

    //float Xposition = (2)*( (position.x - min_max_x_axis.x) / (min_max_x_axis.y - min_max_x_axis.x) ) + -1;

   // float Yposition = (2)*( (position.y - min_max_y_axis.x) / (min_max_y_axis.y - min_max_y_axis.x) ) + -1;

    //float x = Xposition;

    //float y = Yposition;

    gl_Position = transMatrix * vec4(position.x, position.y, 0.0, 1.0);

}
