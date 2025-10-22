#version 430
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_tc;
uniform mat4 model;
uniform mat4 model_normal;
uniform mat4 view;
uniform mat4 proj;
uniform vec2 tc_scale;
uniform vec4 transform;
out vec4 pos_wc;
out vec3 norm_wc;
out vec2 tc;

void main() {
    pos_wc = model * vec4(in_pos, 1.0);
    norm_wc = normalize(mat3(model_normal) * in_norm);
   // tc = tc_scale * in_tc;

   tc  = in_pos.xy + vec2(0.5, 0.5);
   vec2 screenPosition = in_pos.xy * transform.zw + transform.xy;

   screenPosition.x = screenPosition.x * 2.0 - 1.0;
	screenPosition.y = screenPosition.y * -2.0 + 1.0;
	gl_Position = vec4(screenPosition, 0.0, 1.0);

   // gl_Position = pos_wc;//proj * view *
}



/*#version 330

in vec2 in_position;

out vec2 pass_textureCoords;

//position and scale of the quad. Position stored in transform.xy, and scale in transform.zw
uniform vec4 transform;

void main(void){
	
	//calc texture coords based on position
	pass_textureCoords = in_position + vec2(0.5, 0.5);
	//apply position and scale to quad
	vec2 screenPosition = in_position * transform.zw + transform.xy;
	
	//convert to OpenGL coordinate system (with (0,0) in center of screen)
	screenPosition.x = screenPosition.x * 2.0 - 1.0;
	screenPosition.y = screenPosition.y * -2.0 + 1.0;
	gl_Position = vec4(screenPosition, 0.0, 1.0);

}*/