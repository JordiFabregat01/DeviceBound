varying vec2 v_uv;

uniform sampler2D u_texture;

void main(){
	vec2 uv = v_uv;

	float piexls = 32.0;

	uv.x = floor(uv.x * pixels)/pixels;
	uv.y = floor(uv.y * pixels)/pixels;

	//vec3 screen_pixel = texture2D(u_texture, uv).rgb;
	float screen_pixel_r = texture2D(u_texture, uv + 0.03).r;
	float screen_pixel_g = texture2D(u_texture, uv).g;
	float screen_pixel_b = texture2D(u_texture, uv - 0.03).b;

	gl_Fragcolor = vec4(screen_pixel_r,screen_pixel_g,screen_pixel_b, 1.0);
}