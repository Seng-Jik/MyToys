vec3 BlackRollOut(float time,vec3 colOrg,float y)
{
    //time在0.15到0.75
    time = (time - 0.15) / (0.75 - 0.15);
    time = 1.0 - clamp(time * 2.0,0.0,2.0);
    
    float yA = time;
    float yB = 1.0 - yA;
    
    float len;
    if(y < 0.5)
    {
        len = y - yA;
    }
    else
    {
        len = yB - y;
    }
    
    len = len * 2.0;
    len = clamp(len,0.0,1.0);
    
    return mix(colOrg,vec3(0.0),len);
}

vec3 BlackRollIn(float time,vec3 colTrg,float y)
{
    //time在0.25到0.85
    time = (time - 0.25) / (0.85 - 0.25);
    time = 1.0 - clamp(time * 2.0,0.0,2.0);
    
    float yA = time;
    float yB = 1.0 - yA;
    
    float len;
    if(y < 0.5)
    {
        len = y - yA;
    }
    else
    {
        len = yB - y;
    }
    
    len = len * 2.0;
    len = clamp(len,0.0,1.0);
    
    return mix(vec3(0.0),colTrg,len);
}

vec3 WB(vec3 col)
{
    float light = col.r * 0.3 + col.g * 0.6 + col.b * 0.1;
    return vec3(light);
}

vec3 BlackRoll(float time,vec3 colOrg,vec3 colTrg,float y)
{
	//time在0.15到0.85之间出现的黑色滚动特效
    
    vec3 col = BlackRollOut(time,colOrg,y);
    vec3 col2 = BlackRollIn(time,colTrg,y);
    return col+col2;
}

vec3 WBEffect(vec3 col,float time)
{
    //time在0~0.4和0.6~1之间时
    
    time = clamp(time,0.0,1.0);
    
    float wbEffect;
    if(time < 0.45)
    {
        wbEffect = time / 0.45;
    }
    else if(time > 0.55)
    {
        wbEffect = (1.0-time) / 0.55;
    }
    else
        wbEffect = 1.0;
    
    return mix(col,WB(col),wbEffect);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    //transform time
    float time = iTime * 0.5;
    
    //sample
    vec2 texCoord = fragCoord / iResolution.xy;
    vec3 texelOrg = texture(iChannel0,texCoord).rgb;
    vec3 texelTrg = texture(iChannel1,texCoord).rgb;
    
    texelOrg = WBEffect(texelOrg,time);
    texelTrg = WBEffect(texelTrg,time);
    

    time = clamp(time,0.0,1.0);
    
    vec3 col = BlackRoll(time,texelOrg,texelTrg,texCoord.y);
    fragColor = vec4(col,1.0);
}
