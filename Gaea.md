# Gaea
一个类ShaderLab的着色器框架。


## 名词定义
* Shader - 着色器 - 使用GLSL或其他着色器语言编写的着色器程序。
* Platform - 平台 - 指不同的渲染平台实现，如DirectX或OpenGL。
* DrawCall - 绘制调用 - 指在GPU上做一次绘制的调用，是一种特殊的Pass。

## 例子代码

```
platform hlsl_6_0 {
  shader vertex GaussianBlurVS() { ... }
  shader fragment GaussianBlurFS(bool $someSwitch) { 
    Texture2D tex;
    float4 main(...) : SV_TARGET {
      $if someSwitch {
        ... 
      }
      
      ...
    } 
  }
  
  draw GaussianBlur(Texture2D tex, out RenderTarget result) {   // 多个platform块中可以放置名称相同的draw，在platform块外将会根据情况自动选择合适的draw。
      VertexShader = GaussianBlurVS();
      FragmentShader = GaussianBlurFS(true);
      GaussianBlurFS["tex"] = tex;
      RenderTarget = result;
  }
  
  shader vertex DualKawaseBlurVS() { ... }
  shader fragment DualKawaseBlurGS() { ... }
  draw DualKawaseBlurSinglePass(Texture2D tex, out RenderTarget result) {
    VertexShader = DualKawaseBlurVS();
    FragmentShader = DualKawaseBlurFS();
    RenderTarget = result;
    DualKawaseBlurFS["tex"] = tex;
  }
}

```
