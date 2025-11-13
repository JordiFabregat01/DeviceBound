varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;


uniform vec4 u_background_color;
uniform vec3 u_camera_position;
uniform vec3 u_light_color;
uniform vec3 u_light_position;
uniform float u_fog_factor;

uniform float u_tiling;
uniform vec3 u_Ka;
uniform vec3 u_Kd;
uniform vec3 u_Ks;

  
uniform vec2 u_maps;
uniform sampler2D u_texture;
uniform sampler2D u_normal_texture;



vec4 applyLight() {	

// Compute vectors 
// ...
	vec4 Kd = vec4(u_Kd, 1.0);
	// Ambient

	if(u_maps.x == 1.0f){
		Kd = texture2D( u_texture, v_uv);
	}

	vec3 L = normalize(u_light_position - v_world_position);
	vec3 V = normalize(u_camera_position - v_world_position);
	vec3 N = normalize(v_normal);


	if(u_maps.y == 1.0f){
		vec3 normals_texture = texture2D(u_normal_texture, v_uv).rgb;
		//N = perturbNormal(normalize(v_normal), -V, v_uv, normals_texture);
	}

	vec3 ambient = Kd.rgb * u_Ka * u_light_color * 0.4;

	float steps = 3.0;
	// Diffuse
	float NdotL = max(0.0, dot(L, N));
	//NdotL = floor(NdotL * steps) / steps;
	vec3 diffuse = Kd.rgb * u_light_color * NdotL;

	// Specular
	vec3 R = reflect(-L, N);
	float RdotV = pow(max(0.0, dot(R, V)), 20);
	//RdotV = floor(RdotV * steps) / steps;
	vec3 specular = u_Ks * u_light_color * RdotV;

	//Total light
	vec3 total_light = ambient + diffuse + specular;

	return vec4(total_light, Kd.a);
}


void main()
{

	vec4 final_color = applyLight();

	float dist = length( u_camera_position - v_world_position );

	float fogMaxDist = 15.0;
	float fog_value = clamp( 1.0 - (fogMaxDist/dist), 0.0, 1.0 );
	fog_value *= u_fog_factor;

	final_color.rgb = mix(final_color.rgb, vec3(1.0, 0.25, 0.0), fog_value);

	gl_FragColor = final_color;
}