void mainImage(out vec4 fragColor,vec2 fragCoord){
    vec2 coord = fragCoord / iResolution.xy;
    
    vec2 coordOffset = vec2(
    	sin(coord.y * 2.5) * cos(iTime) * 0.5,
    	cos(coord.x * 2.5) * sin(iTime) * 0.5) * 0.5;
    
    vec3 color = texture(iChannel1,coord + coordOffset).rgb;
    vec3 color2 = texture(iChannel0,coord + coordOffset).rgb;
    
    color.r *= 0.7+3.*coordOffset.x;
    color.g *= 0.7+3.*coordOffset.y;
    
    vec3 col = mix(color,color2,length(coordOffset)*4.);
    
    fragColor = vec4(col, 1.0);
}
