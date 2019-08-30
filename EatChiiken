//超辣鸡版本的山寨吃鸡毒圈效果
const float PULL_FORCE_R = 0.55;
const float PULL_FORCE_G = 0.56;
const float PULL_FORCE_B = 0.57;
void mainImage(out vec4 fragColor,vec2 fragCoord){
    vec2 coord = fragCoord / iResolution.xy;
    
    float force = abs(smoothstep(0.95,1.0,sin(iTime*1.5)))*0.35+0.65;
    
    vec2 pullOut = -(coord - vec2(iMouse.xy / iResolution.xy));
    float pullOutLength = length(pullOut);
    pullOutLength = smoothstep(0.,1.0,pullOutLength);
    vec2 pullOutR = pullOut*pullOutLength*PULL_FORCE_R*force;
    vec2 pullOutG = pullOut*pullOutLength*PULL_FORCE_G*force;
    vec2 pullOutB = pullOut*pullOutLength*PULL_FORCE_B*force;
    
    float colorR = texture(iChannel0,coord+pullOutR).r;
    float colorG = texture(iChannel0,coord+pullOutG).g;
    float colorB = texture(iChannel0,coord+pullOutB).b;
    
    colorR *= 1. + (force - 0.65)*pullOutLength*2.4;
    colorG *= 1. + (force - 0.65)*pullOutLength*4.8;
    colorB *= 1. + (force - 0.65)*pullOutLength*9.6;
    
    fragColor = vec4(colorR*0.75,colorG*0.85,colorB, 1.0);
}
