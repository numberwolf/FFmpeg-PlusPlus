# FFmpeg Plus OpenGL(GLSL) - 画中画特效部分

## <a href="README.MD"><b>渲染特效和LUT贴纸</b>请点这里</a> | <a href="README_FADE.MD"><b>转场特效</b>请点这里</a> | <a href="README_PIP.MD"><b>画中画特效</b>请点这里</a>

<hr>

# AVFilter: `pipglshader`

## 效果专区

| filter | shader name | render |
| --- | --- | --- |
| pipglshader | Fragments Shader: <br><a href="Plus-GL-Shader/gl_pip/pip_shader.gl">Plus-GL-Shader/gl_pip/pip_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl_pip/pip_vertex.gl">Plus-GL-Shader/gl_pip/pip_vertex.gl</a></a> | <img src="Plus-GL-Shader/example_show/pip_pip.gif" width="150" /> |

## 使用

### AVFilter 滤镜参数

#### 打印 ffmpeg -h filter=pipglshader
```shell
pipglshader AVOptions:
  sdsource          <string>     ..FV...... gl fragment shader source path (default is render lut fragment)
  vxsource          <string>     ..FV...... gl vertex shader source path (default is render lut vertex)
  start             <duration>   ..FV...... gl render start timestamp, if you set this option, must greater than zero(no trim) (default 0)
  duration          <duration>   ..FV...... gl render duration, if you set this option, must greater than zero(no trim) (default 0)
  pip_duration      <duration>   ..FV...... gl render pip picture's duration, if you set this option, must greater than zero(no trim) (default 0)
  ext_source        <string>     ..FV...... gl texture of pip source media file (default is null)
```

#### 参数详细说明

| 参数 | 说明 | 示例 |
| --- | --- | --- |
| vxsource | 定点着色器文件路径 | <a href="Plus-GL-Shader/gl_fade/fade_vertex.gl">Plus-GL-Shader/gl_fade/fade_vertex.gl</a> |
| sdsource | 片元着色器文件路径 | <a href="Plus-GL-Shader/gl_fade/fade_shader.gl">Plus-GL-Shader/gl_fade/fade_shader.gl</a> |
| start | 特效开始时间 | 5 |
| duration | 背景视频特效持续时间 | 2 |
| pip_duration | 画中画视频特效持续时间 | 2 |
| ext_source | 画中画视频路径 | ./video2.mp4 |

#### 使用示例

```shell
ffmpeg -v debug \
    -ss 0 -t 3 \
    -i bg.mp4 \
    -filter_complex \
    "[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    [scale0]pipglshader=start=0:duration=3:pip_duration=3:ext_source=video2.mp4:vxsource=gl_pip/pip_vertex.gl:sdsource=gl_pip/pip_shader.gl" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
```

### Shader Uniform值说明

#### 所有uniform可用值

```c
uniform float playTime;
uniform float startTime;
uniform float scale_w;
uniform float scale_h;
uniform float isPIP;
uniform float mainDuration;
uniform float pipDuration;
uniform float mainWidth;
uniform float mainHeight;
uniform float pipWidth;
uniform float pipHeight;
```

#### uniform值详细说明

| 参数 | 说明 | 示例 |
| --- | --- | --- |
| playTime | 当前视频渲染时间 | 1.1(秒) |
| startTime | 渲染开始时间 | 5.0 |
| scale_w | 第2个参与渲染的视频自适应scale宽(可不用) | 1.0 |
| scale_h | 第2个参与渲染的视频自适应scale高(可不用) | 1.0 |
| isPIP | 是否当前渲染的是第2个视频 | 0.0/1.0 = 是/否 |
| mainDuration | 背景视频渲染时长 | 2.0 |
| pipDuration | 画中画视频渲染时长 | 2.0 |
| mainWidth | 第1个视频宽度 | 1280 |
| mainHeight | 第1个视频高度 | 720 |
| pipWidth | 第2个视频宽度 | 1280 |
| pipHeight | 第2个视频宽度 | 720 |
