include engine/ShaderVS.txt

shader_out vec3 v_position_view;
shader_out vec3 v_normal_view;
shader_out vec2 v_texcoord;

void main()
{
	vec4 position = unpackPosition();
	vec4 normal = unpackNormal();
	vec2 texcoord = unpackTexcoord(0);

	gl_Position = objectToProjection(position);

	v_position_view = objectToView(position).xyz;
	v_normal_view = objectToView(normal).xyz;

	v_texcoord = texcoord;
}