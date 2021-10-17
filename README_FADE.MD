# FFmpeg Plus OpenGL(GLSL) - 转场特效部分

## <a href="README.MD"><b>渲染特效和LUT贴纸</b>请点这里</a> | <a href="README_FADE.MD"><b>转场特效</b>请点这里</a> | <a href="README_PIP.MD"><b>画中画特效</b>请点这里</a>

<hr>

# AVFilter: `fadeglshader`

## 效果专区

| filter | shader name | render |
| --- | --- | --- |
| fadeglshader | Fragments Shader: <br><a href="Plus-GL-Shader/gl_fade/fade_shader.gl">Plus-GL-Shader/gl_fade/fade_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl_fade/fade_vertex.gl">Plus-GL-Shader/gl_fade/fade_vertex.gl</a></a> | <img src="Plus-GL-Shader/example_show/fade_fade.gif" width="150" /> |

## 使用

### AVFilter 滤镜参数

#### 打印 ffmpeg -h filter=fadeglshader
```shell
fadeglshader AVOptions:
  sdsource          <string>     ..FV...... gl fragment shader source path (default is render lut fragment)
  vxsource          <string>     ..FV...... gl vertex shader source path (default is render lut vertex)
  offset            <duration>   ..FV...... gl render start timestamp, if you set this option, must greater than zero(no trim) (default 0)
  fade_duration     <duration>   ..FV...... gl render fade picture's duration, if you set this option, must greater than zero(no trim) (default 0)
  ext_source        <string>     ..FV...... gl texture of second source media file (default is null)
```

#### 参数详细说明

| 参数 | 说明 | 示例 |
| --- | --- | --- |
| vxsource | 定点着色器文件路径 | <a href="Plus-GL-Shader/gl_fade/fade_vertex.gl">Plus-GL-Shader/gl_fade/fade_vertex.gl</a> |
| sdsource | 片元着色器文件路径 | <a href="Plus-GL-Shader/gl_fade/fade_shader.gl">Plus-GL-Shader/gl_fade/fade_shader.gl</a> |
| offset | 转场开始时间 | 5 |
| fade_duration | 转场持续时间 | 2 |
| ext_source | 参与转场的第二个视频文件路径 | ./video2.mp4 |

#### 使用示例

```shell
ffmpeg -v debug \
    -ss 0 -t 5 \
    -i first.mp4 \
    -filter_complex \
    "[0:v]scale=2560:1600[scale0];
    [scale0]fadeglshader=offset=4:fade_duration=1:ext_source=second.mp4:vxsource=gl_fade/fade_vertex.gl:sdsource=gl_fade/fade_shader.gl[effects];
    [effects]scale=1000:1000" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
```

### Shader Uniform值说明

#### 所有uniform可用值

```c
uniform float progress;
uniform float playTime;
uniform float scale_w;
uniform float scale_h;
uniform float isSecond;
uniform float fadeDuration;
uniform float offset;
uniform float mainWidth;
uniform float mainHeight;
uniform float secondWidth;
uniform float secondHeight;
```

#### uniform值详细说明

| 参数 | 说明 | 示例 |
| --- | --- | --- |
| progress | 最重要:当前转场进度百分比 0-1.0 | 0.5 |
| playTime | 当前视频渲染时间 | 1.1(秒) |
| scale_w | 第2个参与渲染的视频自适应scale宽(可不用) | 1.0 |
| scale_h | 第2个参与渲染的视频自适应scale高(可不用) | 1.0 |
| isSecond | 是否当前渲染的是第2个视频 | 0.0/1.0 = 是/否 |
| offset | 转场开始时间 | 5.0 |
| fadeDuration | 转场持续时间 | 2.0 |
| mainWidth | 第1个视频宽度 | 1280 |
| mainHeight | 第1个视频高度 | 720 |
| secondWidth | 第2个视频宽度 | 1280 |
| secondHeight | 第2个视频宽度 | 720 |
