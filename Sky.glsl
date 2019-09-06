
float fbm(vec2 p,float t) {
    float f = 0.0;
    float s = 0.5;
    float sum = 0.0;

    for(int i = 0;i < 3;++i) {
    	p += vec2(0.0,t);
        t *= 1.5;
        f += s * texture(iChannel0,p / 256.0).x;
        p = p * mat2(0.8,-0.6,0.6,0.8) * 2.02;
        sum += s;
        s *= 0.6;
    }
    return f / sum;
}

vec3 Cloud(vec3 sky,vec3 ro,vec3 rd,float spd)
{
    vec3 col = sky;
    const vec3 cloudCol = vec3(1.0);
    float time = iTime * spd;
    for(int i = 0;i < 2;i ++)
    {
        vec2 sc = ro.xz + rd.xz * (float(i+2)*80000.0-ro.y)/rd.y;
        col = mix( col, cloudCol, 0.5*smoothstep(0.5,0.8,fbm(0.00002*sc,time*float(i+3))) );
    }
    return col; 
}

vec3 Sky(vec2 p)
{
    const vec3 col1 = vec3(92.0/255.0,140.0 / 255.0,206.0/255.0);
    const vec3 col2 = vec3(161.0 / 255.0,209.0 / 255.0,235.0/255.0);
    return mix(col1,col2,1.0-p.y);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    
    vec3 ro = vec3(0.5,-0.3,0.0);
    vec3 rd = normalize(
        vec3(uv.x,uv.y,1.0) - ro);
    
    vec3 skyColor = Sky(uv);

    // Time varying pixel color
    vec3 col = vec3(Cloud(skyColor,ro,rd,0.1));

    // Output to screen
    fragColor = vec4(col,1.0);
}
