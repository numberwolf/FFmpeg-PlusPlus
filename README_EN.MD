# FFmpeg + OpenGL(GLSL) - For video edit rendering

<img src="./assets/logo_plusopengl.png" width="300px" />

#### Make your FFmpeg support GLSL - Graphic Rendering

> `Shader++` is a web(pc) develop application for this AVFilter:
>
> You can write and make your shaders on `Shader++`, then make it running with `FFmpeg-Plus-Opengl`
>
> https://www.zzsin.com/shaderplus.html
>
> <img src="Plus-GL-Shader/web.jpg" width="500px" />


## <a href="README_EN.MD">English</a> | <a href="README.MD"><b>中文</b></a>
<hr>

- [Introduce](#introduce)
    - [UpldateLog](#upldatelog)
    - [Example](#example)

- [How to Use FFmpeg+OpenGL Shader Filter: vf_glplusshader](#ffmpeg-plus-gl-shader)
    
    - [File Struct](#filestruct)
    - [Build](#build)
    - [Run](#run)
        - [GLSL Shader](#shader-render)
        - [Lut look-up-table Shader](#lut-render)

- [Help Guide](#helpguide)
    - [FFmpeg Guide](#ffmpegguide)

- [About](#about)

## Introduce ##

OpenGL filter for ffmpeg.

You can use <a href="Plus-GL-Shader">filter `plusglshader`</a> to render frame by opengl's shader.

### UpldateLog ###

| UpldateLog | Content |
| ---- | ---- |
| Time | 2021/08/16 |
| - | 1.`lutglshader` LUT-Filter Support Custom define size of lut-table image. |
| Time | 2021/08/15 |
| - | 1.`plusglshader` Effect-Filter Support Alpha channel, PR by <a href="https://github.com/numberwolf/FFmpeg-Plus-OpenGL/pull/25">Pull Request:#25</a> |
| Time | 2021/07/06 |
| - | 1.lutglshader is upward compatible with the newest ffmpeg.git's code |
| Time | 2021/06/26 |
| - | 1.Support Lut look-up-table filter |


### Example ###

* Script <a href="Plus-GL-Shader/run_example.sh">`Plus-GL-Shader/run_example.sh`</a>

* Shaders: <a href="Plus-GL-Shader/gl">`See Example Shaders ./Plus-GL-Shader/gl`</a>

* Show

| filter | shader name | source | render |
| - | - | - | - |
| lutglshader [Lut look-up-table Shader](#lut-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/lut_shader.gl">Plus-GL-Shader/gl/lut_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/lut_vertex.gl">Plus-GL-Shader/gl/lut_vertex.gl</a> | <img src="Plus-GL-Shader/example_show/lut-right.png" width="150" /> | <img src="Plus-GL-Shader/example_show/lut-left.png" width="150" /> |
| plusglshader [GLSL Shader](#shader-render)| Fragments Shader: <br><a href="Plus-GL-Shader/gl/light_shader.gl">Plus-GL-Shader/gl/light_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/light_vertex.gl">Plus-GL-Shader/gl/light_vertex.gl</a> | <img src="Plus-GL-Shader/city720p.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/light.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/mirror_shader.gl">Plus-GL-Shader/gl/mirror_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/mirror_vertex.gl">Plus-GL-Shader/gl/mirror_vertex.gl</a> | <img src="Plus-GL-Shader/cg.gif" width="300" /> | <img src="Plus-GL-Shader/example_show/mirror.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/sway_shader.gl">Plus-GL-Shader/gl/sway_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/sway_vertex.gl">Plus-GL-Shader/gl/sway_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/sway.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/jitter_shader.gl">Plus-GL-Shader/gl/jitter_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/jitter_vertex.gl">Plus-GL-Shader/gl/jitter_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/jitter.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/preview_shader.gl">Plus-GL-Shader/gl/preview_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/preview_vertex.gl">Plus-GL-Shader/gl/preview_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/preview.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/soul_shader.gl">Plus-GL-Shader/gl/soul_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/soul_vertex.gl">Plus-GL-Shader/gl/soul_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/soul.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/split_interval_shader.gl">Plus-GL-Shader/gl/split_interval_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/split_interval_vertex.gl">Plus-GL-Shader/gl/split_interval_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/split_interval.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/stab_ci_shader.gl">Plus-GL-Shader/gl/stab_ci_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/stab_ci_vertex.gl">Plus-GL-Shader/gl/stab_ci_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/stab_ci.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/test_shader.gl">Plus-GL-Shader/gl/test_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/test_vertex.gl">Plus-GL-Shader/gl/test_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/test.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/white_mask_shader.gl">Plus-GL-Shader/gl/white_mask_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/white_mask_vertex.gl">Plus-GL-Shader/gl/white_mask_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/white_mask.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/split_4_shader.gl">Plus-GL-Shader/gl/split_4_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/split_4_vertex.gl">Plus-GL-Shader/gl/split_4_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/split_4.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/split_9_shader.gl">Plus-GL-Shader/gl/split_9_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/split_9_vertex.gl">Plus-GL-Shader/gl/split_9_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/split_9.gif" width="300" /> |
| plusglshader [GLSL Shader](#shader-render) | Fragments Shader: <br><a href="Plus-GL-Shader/gl/split_vert2_shader.gl">Plus-GL-Shader/gl/split_vert2_shader.gl</a><br>Vertex Shader: <br><a href="Plus-GL-Shader/gl/split_vert2_vertex.gl">Plus-GL-Shader/gl/split_vert2_vertex.gl</a> | <img src="Plus-GL-Shader/cat_quqi.jpeg" width="300" /> | <img src="Plus-GL-Shader/example_show/split_vert2.gif" width="300" /> |

## FFmpeg-Plus-GL-Shader ##

### FileStruct ###

* Filter Source Code Path: <a href="Plus-GL-Shader">Plus-GL-Shader</a>

    * libavfilter.diff

    * vf_plusglshader.c
    
    * vf_lutglshader.c

### Build ###

* Dependencies
    * Centos 7.x+ || Linux

        * first
            ```shell
            yum install -y glew glew-devel
            yum install -y glfw glew-devel
            yum install -y mesa-dri-drivers

            #
            # If can not compile , you need
            #
            yum install -y libGLEW*
            yum install -y mesa
            yum install -y mesa-libGLU mesa-libGLU-devel
            yum install -y libXfont*
            ```

        * second
            > If on headless environments

            > If you want to running on server machine without Video card

            ```shell
            yum install -y xorg-x11-server-Xvfb 
            ```

    * Ubuntu || Linux

        * first
            ```shell
            apt-get install libglfw3-dev libglfw3
            apt-get install libglew2.0 libglew-dev
            ```

        * second
            > If on headless environments

            > If you want to running on server machine without Video card

            ```shell
            apt-get install xvfb
            ```


    * MacOS
        ```shell
        brew install glew glfw
        ```

* Compile

    * Download
        ```shell
        git clone https://github.com/numberwolf/FFmpeg-Plus-OpenGL.git
      
        git clone https://github.com/FFmpeg/FFmpeg.git # for 4.1x~4.3x
        # OR: wget -O ffmpeg.tar.gz "https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n4.3.1.tar.gz"
        
        cd FFmpeg
        
        #
        # Patch
        #
        cp ../FFmpeg-Plus-OpenGL/Plus-GL-Shader/vf_plusglshader.c libavfilter/
        cp ../FFmpeg-Plus-OpenGL/Plus-GL-Shader/vf_lutglshader.c libavfilter/
        git apply ../FFmpeg-Plus-OpenGL/Plus-GL-Shader/libavfilter.diff
        ```

    * Build
        * Needed
            > --enable-opengl \
            > --extra-libs='-lGLEW -lglfw' \
            > --enable-filter=plusglshader \
            > --enable-filter=lutglshader

        * Example
            ```shell
            #!/bin/bash
            ./configure \
              --enable-cross-compile \
              --pkg-config-flags="--static" \
              --extra-ldflags="-lm -lz -llzma -lpthread" \
              --extra-libs=-lpthread \
              --extra-libs=-lm \
              --enable-gpl \
              --enable-libfdk_aac \
              --enable-libfreetype \
              --enable-libmp3lame \
              --enable-libx264 \
              --enable-nonfree \
              --disable-shared \
              --enable-static \
              --enable-opengl \
              --extra-libs='-lGLEW -lglfw' \
              --enable-filter=plusglshader \
              --enable-filter=lutglshader
            make clean
            make -j16
            make install
            ```

### Run ###

#### shader-render ####
* Check `Plus-GL-Shader`:`plusglshader` filter
    * Check Cmd
        ```shell
        ffmpeg -help filter=plusglshader
        ```

    * Output
        ```shell
        ffmpeg version a0d68e65 Copyright (c) 2000-2020 the FFmpeg developers
          built with Apple LLVM version 10.0.0 (clang-1000.10.44.4)
          configuration: --enable-cross-compile --pkg-config-flags=--static --extra-ldflags='-lm -lz -llzma -lpthread' --extra-libs=-lpthread --extra-libs=-lm --enable-gpl --enable-libfdk_aac --enable-libfreetype --enable-libmp3lame --enable-libopus --enable-libvpx --enable-libx264 --enable-libx265 --enable-libass --enable-libfreetype --enable-libfontconfig --enable-libfribidi --enable-libwebp --enable-nonfree --disable-shared --enable-static --enable-opengl --extra-libs='-lGLEW -lglfw' --enable-filter=plusglshader
          libavutil      56. 51.100 / 56. 51.100
          libavcodec     58. 91.100 / 58. 91.100
          libavformat    58. 45.100 / 58. 45.100
          libavdevice    58. 10.100 / 58. 10.100
          libavfilter     7. 85.100 /  7. 85.100
          libswscale      5.  7.100 /  5.  7.100
          libswresample   3.  7.100 /  3.  7.100
          libpostproc    55.  7.100 / 55.  7.100
        Filter plusglshader
          Generic OpenGL shader filter
            Inputs:
               #0: default (video)
            Outputs:
               #0: default (video)
        plusglshader AVOptions:
          sdsource          <string>     ..FV...... gl fragment shader source path (default is render gray color)
          vxsource          <string>     ..FV...... gl vertex shader source path (default is render gray color)
          start             <duration>   ..FV...... gl render start timestamp, if you set this option, must greater than zero(no trim) (default 0)
          duration          <duration>   ..FV...... gl render duration, if you set this option, must greater than zero(no trim) (default 0)
        
        This filter has support for timeline through the 'enable' option.


        This filter has support for timeline through the 'enable' option.
        ```

* Use `plusglshader` (Render & Transcoding)

    > plusglshader=sdsource='./test_shader.gl':vxsource='./test_vertex.gl'

    * Params
    
        * `sdsource` : Fragment shader file path (Default is gray)
        * `vxsource` : Vertex shader file path (Default is gray)
        * `start` : GL Render start timestamp
        * `duration` : GL Render duration

    * Filter Rules - Example

        * `plusglshader`
        * `plusglshader=sdsource='./test_shader.gl':vxsource='./test_vertex.gl'`
        * `plusglshader=sdsource='./test_shader.gl':vxsource='./test_vertex.gl':duration=10`
        * `plusglshader=sdsource='./test_shader.gl':vxsource='./test_vertex.gl':start=5:duration=10`
        * Multi-plusglshader
            ```bash
            ffmpeg -v debug \
            -ss 40 -t 15 -i /Users/numberwolf/Documents/webroot/VideoMissile/VideoMissilePlayer/demo/res/cg.mp4 \
            -filter_complex \
            "plusglshader=sdsource=gl/jitter_shader.gl:vxsource=gl/jitter_vertex.gl:duration=10.1[out1];
            [out1]plusglshader=sdsource=gl/sway_shader.gl:vxsource=gl/sway_vertex.gl:duration=5[out2];
            [out2]plusglshader=sdsource=gl/split_9_shader.gl:vxsource=gl/split_9_vertex.gl:start=3:duration=3" \
            -vcodec libx264 \
            -an \
            -f mp4 -y output.mp4
            ```
    
    * Write your own shader!

        * 1) Fragment shader
            > Example: <a href="Plus-GL-Shader/test_shader.gl">test_shader.gl</a>

            #### Notice: playTime is timestamp, from 0 -> end

            ```c
            uniform sampler2D tex;
            uniform float playTime;
            varying vec2 texCoord;
            void main() {
                gl_FragColor = texture2D(tex, texCoord * 0.5 + 0.5);
                float usePts = max(playTime, 0.4);
                float gray = (gl_FragColor.r + gl_FragColor.g + gl_FragColor.b) / (usePts * usePts);
                gl_FragColor.r = gray;
                gl_FragColor.g = gray;
                gl_FragColor.b = gray;
            }
            ```

        * 2) Vertex shader
            > Example: <a href="Plus-GL-Shader/test_vertex.gl">test_vertex.gl</a>

            ```c
            attribute vec2 position;
            varying vec2 texCoord;
            void main(void) {
                gl_Position = vec4(position, 0, 1);
                texCoord = position;
            }
            ```

    * Running with out video card || headless env

        > You need xvfb

        ```shell
        xvfb-run -a --server-args="-screen 0 1280x720x24 -ac -nolisten tcp -dpi 96 +extension RANDR" \
        ffmpeg -v debug \
        -i 1000p10s_9k.mp4 \
        -filter_complex "[0:v]plusglshader=sdsource='./test_shader.gl':vxsource='./test_vertex.gl'[1outgl];[1outgl]scale=1280:-2" \
        -vcodec libx264 \
        -an \
        -pix_fmt yuv420p \
        -y test.mp4
        ```

    * Running with video card

        ```shell
        ffmpeg -v debug \
        -i 1000p10s_9k.mp4 \
        -filter_complex "[0:v]plusglshader=sdsource='./test_shader.gl':vxsource='./test_vertex.gl'[1outgl];[1outgl]scale=1280:-2" \
        -vcodec libx264 \
        -an \
        -pix_fmt yuv420p \
        -y test.mp4
        ```

    * Output Example

        | Tag | Src | Dst |
        |-|-|-|
        | Frame | <img src="Plus-GL-Shader/example_src.png" width="500" /> | <img src="Plus-GL-Shader/example_dst.png" width="500" /> |



#### lut-render ####

* Check `Plus-GL-Shader`:`lutglshader` filter
    * Check Cmd
        ```shell
        ffmpeg -help filter=lutglshader
        ```

    * Output
        ```shell
        ffmpeg version d48da9a Copyright (c) 2000-2020 the FFmpeg developers
          built with Apple LLVM version 10.0.0 (clang-1000.10.44.4)
          configuration: --enable-cross-compile --pkg-config-flags=--static --extra-ldflags='-lm -lz -llzma -lpthread' --extra-libs=-lpthread --extra-libs=-lm --enable-gpl --enable-libfdk_aac --enable-libfreetype --enable-libmp3lame --enable-libopus --enable-libvpx --enable-libx264 --enable-libx265 --enable-libass --enable-libfreetype --enable-libfontconfig --enable-libfribidi --enable-libwebp --enable-nonfree --disable-shared --enable-static --enable-opengl --extra-libs='-lGLEW -lglfw' --enable-filter=plusglshader
          libavutil      56. 51.100 / 56. 51.100
          libavcodec     58. 91.100 / 58. 91.100
          libavformat    58. 45.100 / 58. 45.100
          libavdevice    58. 10.100 / 58. 10.100
          libavfilter     7. 85.100 /  7. 85.100
          libswscale      5.  7.100 /  5.  7.100
          libswresample   3.  7.100 /  3.  7.100
          libpostproc    55.  7.100 / 55.  7.100
        Filter lutglshader
          Render Frame by GL shader with LUT
            Inputs:
               #0: default (video)
            Outputs:
               #0: default (video)
        lutglshader AVOptions:
            sdsource          <string>     ..FV...... gl fragment shader source path (default is render lut fragment)
            vxsource          <string>     ..FV...... gl vertex shader source path (default is render lut vertex)
            start             <duration>   ..FV...... gl render start timestamp, if you set this option, must greater than zero(no trim) (default 0)
            duration          <duration>   ..FV...... gl render duration, if you set this option, must greater than zero(no trim) (default 0)
            lut_source        <string>     ..FV...... gl fragment shader externTex's source file (default is null). always for lut
            lut_type          <int>        ..FV...... ext type(0:media 1:rgb24 default is 0) (from 0 to INT_MAX) (default 0)
            lut_img_w         <int>        ..FV...... ext's width, when ext type is 1:rgb24, need be set. default is 512 (from 0 to INT_MAX) (default 512)
            lut_img_h         <int>        ..FV...... ext's height, when ext type is 1:rgb24, need be set. default is 512 (from 0 to INT_MAX) (default 512)
            lut_lineBlockCount <int>        ..FV...... lut color-block's count every line in lut-table(default is 8) (from 0 to INT_MAX) (default 8)
            lut_oneBlockW     <int>        ..FV...... lut color-block's width every block(default is 64) (from 0 to INT_MAX) (default 64)
        
        This filter has support for timeline through the 'enable' option.
        ```
      
      
* Use `lutglshader` (Render & Transcoding)

    > IMG: lutglshader=`lut_source=./lut.jpg:lut_type=0`:duration=5'
    >
    > RGB: lutglshader=`lut_source=./lut.rgb:lut_type=1`:duration=5'

    * Params
    
        * `lut_source` : lut image (default NULL)
            > <img src="assets/lut_usage.png" width='512' height='512' />
            >   
            > Example image 512x512 <a href="Plus-GL-Shader/lut.jpg">./lut.jpg</a>
        * `lut_type` : lut file type, 0:`Image file` ,1:`RGB24 file` (Default is 0)
        * `sdsource` : Fragment shader file path (Default is gray)
        * `vxsource` : Vertex shader file path (Default is gray)
        * `lut_img_w` : LUT Image's width. eg: 512 (px)
        * `lut_img_h` : LUT Image's height.
        * `lut_lineBlockCount` : LUT Image: Count of blocks for one line. eg: (lut 512x512 's block count is 8. <a href="assets/lut_usage.png"> like here </a>)
        * `lut_oneBlockW` : LUT Image: Block's width(px, lut 512x512 's block's width is 64px)
        * `start` : GL Render start timestamp
        * `duration` : GL Render duration

    * Filter Rules - Example
    
        * RGB File of LUT
            * `lutglshader=lut_source=./lut.rgb:lut_type=1`
            * `lutglshader=lut_source=./lut.rgb:lut_type=1:duration=5:lut_img_w=512:lut_img_h=512`
            * `lutglshader=lut_source=./lut.rgb:lut_type=1:duration=5:lut_img_w=512:lut_img_h=512:lut_lineBlockCount=8:lut_oneBlockW=64`
            * `lutglshader=lut_source=./lut.rgb:lut_type=1:duration=5:lut_img_w=512:lut_img_h=512:lut_lineBlockCount=8:lut_oneBlockW=64:sdsource=gl/lut_shader.gl:vxsource='./lut_vertex.gl'`
            * `lutglshader=lut_source=./lut.rgb:lut_type=1:start=1:duration=5:lut_img_w=512:lut_img_h=512:lut_lineBlockCount=8:lut_oneBlockW=64:sdsource=gl/lut_shader.gl:vxsource='./lut_vertex.gl'`
        
        * Image File of LUT
            * `lutglshader=lut_source=./lut.jpg`
            * `lutglshader=lut_source=./lut.jpg:lut_type=0`
            * `lutglshader=lut_source=./lut.jpg:lut_type=0:lut_lineBlockCount=8:lut_oneBlockW=64`
            * `lutglshader=lut_source=./lut.jpg:lut_type=0:lut_lineBlockCount=8:lut_oneBlockW=64:start=1:duration=5`
        
        * Use `lutglshader` Example
            ```bash
            ffmpeg -v debug \
            -ss 40 -t 15 -i cg.mp4 \
            -filter_complex \
            "lutglshader=lut_source=./lut.jpg" \
            -vcodec libx264 \
            -an \
            -f mp4 -y output.mp4
            ```
          
    * Write your own shader!
    
        * Fragment shader
        
            ```c
            uniform sampler2D tex;
            uniform sampler2D externTex;
            varying vec2 TextureCoordsVarying;
            uniform float playTime;
            const float PI = 3.1415926;
            
            float rand(float n) {
                return fract(sin(n) * 43758.5453123);
            }
            
            vec4 lookupTable(vec4 color, float progress){
                //float blueColor = color.b * 63.0 * progress;
                float blueColor = color.b * 63.0;
            
                vec2 quad1;
                quad1.y = floor(floor(blueColor) / 8.0);
                quad1.x = floor(blueColor) - (quad1.y * 8.0);
            
                vec2 quad2;
                quad2.y = floor(ceil(blueColor) / 8.0);
                quad2.x = ceil(blueColor) - (quad2.y * 8.0);
            
                vec2 texPos1;
                texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);
                texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);
            
                vec2 texPos2;
                texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);
                texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);
            
                vec4 newColor1 = texture2D(externTex, texPos1);
                vec4 newColor2 = texture2D(externTex, texPos2);
                vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
                return vec4(newColor.rgb, color.w);
            }
            
            void main() {
                float duration = 0.5;
                float progress = mod(playTime, duration) / duration; // 0~1
                vec4 imgColor = texture2D(tex, TextureCoordsVarying);
                vec4 lutColor = lookupTable(imgColor, progress);
                gl_FragColor = mix(imgColor, lutColor, 1.0);
            }
            ```


## HelpGuide ##

#### FFmpegGuide ####

* If you don't know about how to compile ffmpeg, here u click in. When u did it, u can build with <a href="Plus-GL-Shader">filter `plusglshader`</a>.
    * <a href="https://trac.ffmpeg.org/wiki/CompilationGuide">How to compile ffmpeg</a>
    * [Build filter `plusglshader`</a>](#build)

## About ##

#### Contact

* Github: 

    * https://github.com/numberwolf/FFmpeg-Plus-OpenGL
    * https://github.com/numberwolf/h265web.js

* Email(porschegt23@foxmail.com)

* QQ: 531365872

* QQ Group number: 925466059

* Discord:numberwolf#8694

* Wechat:numberwolf11



