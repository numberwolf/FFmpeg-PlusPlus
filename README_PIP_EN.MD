# FFmpeg Plus OpenGL(GLSL) - Picture in Picture Part

* Support

    * #### <a href="README_EN.MD"><b>Render Effects and LUT</b> - Click here</a>
    
    * #### <a href="README_FADE_EN.MD"><b>Transition Fade Effects</b> - Click here</a> 
    
    * #### <a href="README_PIP_EN.MD"><b>Picture in Picture Effects</b>- Click here</a>
<hr>

# AVFilter: `pipglshader`

## Effects show

| filter | shader name | render |
| --- | --- | --- |
| pipglshader | Fragments Shader: <br><a href="Plus-GL-Shader/gl_pip/pip_shader.gl">Plus-GL-Shader/gl_pip/pip_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl_pip/pip_vertex.gl">Plus-GL-Shader/gl_pip/pip_vertex.gl</a></a> | <img src="Plus-GL-Shader/example_show/pip_pip.gif" width="150" /> |


## Usage

### AVFilter Param

#### CMD - ffmpeg -h filter=pipglshader
```shell
pipglshader AVOptions:
  sdsource          <string>     ..FV...... gl fragment shader source path (default is render lut fragment)
  vxsource          <string>     ..FV...... gl vertex shader source path (default is render lut vertex)
  start             <duration>   ..FV...... gl render start timestamp, if you set this option, must greater than zero(no trim) (default 0)
  duration          <duration>   ..FV...... gl render duration, if you set this option, must greater than zero(no trim) (default 0)
  pip_duration      <duration>   ..FV...... gl render pip picture's duration, if you set this option, must greater than zero(no trim) (default 0)
  ext_source        <string>     ..FV...... gl texture of pip source media file (default is null)
```

#### Param description

| Param | Desc | Example |
| --- | --- | --- |
| vxsource | Vertex shader file path | <a href="Plus-GL-Shader/gl_fade/fade_vertex.gl">Plus-GL-Shader/gl_fade/fade_vertex.gl</a> |
| sdsource | Fragment shader file path | <a href="Plus-GL-Shader/gl_fade/fade_shader.gl">Plus-GL-Shader/gl_fade/fade_shader.gl</a> |
| start | Effects start time | 5 |
| duration | background video's effect render duration | 2 |
| pip_duration | pip(picuture in picture)'s video's effect render duration | 2 |
| ext_source | pip video's path | ./video2.mp4 |

#### CMD - Example

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

### Shader Uniform

#### All of uniform

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

#### uniform value description

| Param | Desc | Example |
| --- | --- | --- |
| playTime | play timestamp | 1.1(second) |
| startTime | Effect start time | 5.0 |
| scale_w | the second video's adapte scale width(maybe dont need) | 1.0 |
| scale_h | the second video's adapte scale height(maybe dont need) | 1.0 |
| isPIP | render is second video | 0.0/1.0 = Y/N |
| mainDuration | background video's effect duration | 2.0 |
| pipDuration | pip video's effect duration | 2.0 |
| mainWidth | first video's width | 1280 |
| mainHeight | first video's height | 720 |
| pipWidth | second video's width | 1280 |
| pipHeight | second video's height | 720 |
