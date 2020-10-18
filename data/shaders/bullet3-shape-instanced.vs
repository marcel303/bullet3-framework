include engine/ShaderVS.txt

uniform mat4 u_viewMatrix;
uniform mat4 u_viewProjectionMatrix;

uniform mat4 u_worldMatrices[64] buffer transforms; // todo : 256 again. 64 now due to 4k limit Metal
uniform vec4 u_colors[128] buffer colors;

shader_out vec3 v_position_view;
shader_out vec3 v_normal_view;
shader_out vec4 v_color;
shader_out vec3 v_specularColor;
shader_out vec2 v_texcoord;

void main()
{
	vec4 position = unpackPosition();
	vec4 normal = unpackNormal();
	vec2 texcoord = unpackTexcoord(0);

	int id = gl_InstanceID;
	
	vec4 position_world = u_worldMatrices[id] * position;
	vec4 normal_world = u_worldMatrices[id] * normal;

	gl_Position = u_viewProjectionMatrix * position_world;

	v_position_view = (u_viewMatrix * position_world).xyz;
	v_normal_view = (u_viewMatrix * normal_world).xyz;

	v_color = u_colors[id * 2 + 0];
	v_specularColor = u_colors[id * 2 + 1].rgb;

	v_texcoord = texcoord;
}