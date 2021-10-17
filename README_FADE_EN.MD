# FFmpeg Plus OpenGL(GLSL) - Transition Fade Part

* Support

    * #### <a href="README_EN.MD"><b>Render Effects and LUT</b> - Click here</a>
    
    * #### <a href="README_FADE_EN.MD"><b>Transition Fade Effects</b> - Click here</a> 
    
    * #### <a href="README_PIP_EN.MD"><b>Picture in Picture Effects</b>- Click here</a>
<hr>

# AVFilter: `fadeglshader`

## Effects show

| filter | shader name | render |
| --- | --- | --- |
| fadeglshader | Fragments Shader: <br><a href="Plus-GL-Shader/gl_fade/fade_shader.gl">Plus-GL-Shader/gl_fade/fade_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl_fade/fade_vertex.gl">Plus-GL-Shader/gl_fade/fade_vertex.gl</a></a> | <img src="Plus-GL-Shader/example_show/fade_fade.gif" width="150" /> |

## Usage

### AVFilter Param

#### CMD - ffmpeg -h filter=fadeglshader
```shell
fadeglshader AVOptions:
  sdsource          <string>     ..FV...... gl fragment shader source path (default is render lut fragment)
  vxsource          <string>     ..FV...... gl vertex shader source path (default is render lut vertex)
  offset            <duration>   ..FV...... gl render start timestamp, if you set this option, must greater than zero(no trim) (default 0)
  fade_duration     <duration>   ..FV...... gl render fade picture's duration, if you set this option, must greater than zero(no trim) (default 0)
  ext_source        <string>     ..FV...... gl texture of second source media file (default is null)
```

#### Param description

| Param | Desc | Example |
| --- | --- | --- |
| vxsource | Vertex shader file path | <a href="Plus-GL-Shader/gl_fade/fade_vertex.gl">Plus-GL-Shader/gl_fade/fade_vertex.gl</a> |
| sdsource | Fragment shader file path | <a href="Plus-GL-Shader/gl_fade/fade_shader.gl">Plus-GL-Shader/gl_fade/fade_shader.gl</a> |
| offset | Transtion start time | 5 |
| fade_duration | Transition duration | 2 |
| ext_source | Second video path in transtion | ./video2.mp4 |

#### CMD - Example

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

### Shader Uniform

#### All of uniform

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

#### uniform value description

| Param | Desc | Example |
| --- | --- | --- |
| progress | Most import:progress of transition(percent) 0-1.0 | 0.5 |
| playTime | play timestamp | 1.1(second) |
| scale_w | the second video's adapte scale width(maybe dont need) | 1.0 |
| scale_h | the second video's adapte scale height(maybe dont need) | 1.0 |
| isSecond | render is second video | 0.0/1.0 = Y/N |
| offset | Transition start time | 5.0 |
| fadeDuration | Transition duration | 2.0 |
| mainWidth | first video's width | 1280 |
| mainHeight | first video's height | 720 |
| secondWidth | second video's width | 1280 |
| secondHeight | second video's height | 720 |
