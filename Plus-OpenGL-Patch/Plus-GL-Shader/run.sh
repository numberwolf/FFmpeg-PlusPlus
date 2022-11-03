
function main1 {
    shadername="sway"

    ffmpeg -v debug \
    -ss 40 -t 15 -i /Users/numberwolf/Documents/webroot/VideoMissile/VideoMissilePlayer/demo/res/cg.mp4 \
    -filter_complex \
    "plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=10.1[out1];
    [out1]plusglshader=sdsource=gl/sway_shader.gl:vxsource=gl/sway_vertex.gl:duration=5[out2];
    [out2]plusglshader=sdsource=gl/split_9_shader.gl:vxsource=gl/split_9_vertex.gl:start=3:duration=3" \
    -vcodec libx264 \
    -an \
    -f mp4 -y output.mp4
#-filter_complex "plusglshader" -vcodec libx264 -an -f mp4 -y output.mp4
}

function main2 {
    shadername="light"

    #-i /Users/numberwolf/Documents/webroot/VideoMissile/VideoMissilePlayer/demo/res/cg.mp4 \
    ffmpeg -v debug \
    -framerate 24 -t 5 -loop 1 \
    -i ./newshader/city720p.jpeg \
    -filter_complex \
    "plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}

function main3 {
    shadername="mirror"

    ffmpeg -v debug \
    -ss 40 -t 5 \
    -i /Users/numberwolf/Documents/webroot/VideoMissile/VideoMissilePlayer/demo/res/cg.mp4 \
    -filter_complex \
    "plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}

function main4 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/github-gl-badcase.mp4"
    shadername="old"

    ffmpeg -v debug \
    -ss 0 -t 5 \
    -i $file_path \
    -filter_complex \
    "plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}

function main5 {
    #local file_path="/Users/numberwolf/Documents/webroot/Movies/github-gl-badcase.mp4"
    #local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
    local file_path="/Users/numberwolf/Documents/webroot/Movies/good.png"
    shadername="jitter"

    #"plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.dev.gl:duration=5" \

    ffmpeg -v debug \
    -framerate 24 -loop 1 -ss 0 -t 5 \
    -i $file_path \
    -ss 0 -t 5 \
    -i /Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4 \
    -filter_complex \
    "[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    [scale0]plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5[lut0];
    [lut0]scale=1000:1000,setdar=dar=1000/1000[ret0];
    [1:v][ret0]overlay" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}

function main6 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/github-gl-badcase.mp4"
    shadername="test"

    ffmpeg -v debug \
    -ss 0 -t 2 \
    -i $file_path \
    -filter_complex \
    "plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}

function main7 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
    shadername="lut"
    #shadername="bluesea"
    
    #shadername="jitter"
    #-i ./lut.jpg \
    #"[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    #"[0:v]scale=1000:1000,setsar=sar=1/1,setdar=dar=1000/1000[scale0];
    #[1:v]scale=256:160,setsar=sar=1/1,setdar=dar=256/160[scale1];
    #-i ./lut_1000x1000.jpg \
    #-i ./lut_2560x1600.jpeg \
    #[lut0]scale=1000:1000,setdar=dar=1000/1000" \
    #[scale0]lutglshader=ext=./lut1.jpg:ext_type=0:sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5[lut0];

    
    #[scale0]lutglshader=lut_source=./lut.jpg:lut_type=0:duration=5[lut0];
    #[scale0]lutglshader=lut_source=./lut.rgb:lut_type=1:duration=5[lut0];

    #-i ./lut.jpg \
    ffmpeg -v debug \
    -ss 0 -t 5 \
    -i $file_path \
    -filter_complex \
    "[0:v]scale=512:512,setsar=sar=1/1,setdar=dar=512/512,split=2[scale0][scale0_copy];
    [scale0]lutglshader=lut_source=./lut.jpg:lut_type=0:duration=5[lut0];
    [lut0]scale=512:512,setdar=dar=512/512[lut0_ret];
    [lut0_ret]pad=iw*2[lut0_pad];
    [lut0_pad][scale0_copy]overlay=512:0" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
    #[lut0]scale=1000:1000,setdar=dar=1000/1000" \
    
}

function main7_1 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
    shadername="lut"
    #shadername="bluesea"
    
    #shadername="jitter"
    #-i ./lut.jpg \
    #"[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    #"[0:v]scale=1000:1000,setsar=sar=1/1,setdar=dar=1000/1000[scale0];
    #[1:v]scale=256:160,setsar=sar=1/1,setdar=dar=256/160[scale1];
    #-i ./lut_1000x1000.jpg \
    #-i ./lut_2560x1600.jpeg \
    #[lut0]scale=1000:1000,setdar=dar=1000/1000" \
    #[scale0]lutglshader=ext=./lut1.jpg:ext_type=0:sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5[lut0];

    
    #[scale0]lutglshader=lut_source=./lut.jpg:lut_type=0:duration=5:lut_img_w=512:lut_img_h=512[lut0];
    #[scale0]lutglshader=lut_source=./lut.jpg:lut_type=0:duration=5[lut0];
    #[scale0]lutglshader=lut_source=./lut_1000x1000.rgb:lut_type=1:duration=5:lut_img_w=1000:lut_img_h=1000:sdsource='gl/lut_shader_1000x1000.gl':vxsource='gl/lut_vertex.gl'[lut0];

    #-i ./lut.jpg \
    ffmpeg -v debug \
    -ss 0 -t 5 \
    -i $file_path \
    -filter_complex \
    "[0:v]scale=512:512,setsar=sar=1/1,setdar=dar=512/512,split=2[scale0][scale0_copy];
    [scale0]lutglshader=lut_source=./lut_1000x1000.rgb:lut_type=1:duration=5:lut_img_w=1000:lut_img_h=1000[lut0];
    [lut0]scale=512:512,setdar=dar=512/512[lut0_ret];
    [lut0_ret]pad=iw*2[lut0_pad];
    [lut0_pad][scale0_copy]overlay=512:0" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
    #[lut0]scale=1000:1000,setdar=dar=1000/1000" \
    
}


function main8 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
    shadername="star"

    ffmpeg -v debug \
    -ss 0 -t 5 \
    -i $file_path \
    -filter_complex \
    "[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    [scale0]plusglshader=sdsource=gl/${shadername}_shader.gl:vxsource=gl/${shadername}_vertex.gl:duration=5[lut0];
    [lut0]scale=1000:1000,setdar=dar=1000/1000" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}

#gles2_filter.vert

function main9 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
:<<!
    shadername="customer/gles2_filter-left2right"
    shadername="customer/gles2_filter-right2left"
    shadername="customer/gles2_filter-up2down"
    shadername="customer/gles2_filter-down2up"
    shadername="customer/gles2_filter-rotate-clockwise"
    shadername="customer/gles2_filter-rotate-anticlockwise"
!

    shader_list=(
"customer/gles2_filter-left2right"
"customer/gles2_filter-right2left"
"customer/gles2_filter-up2down"
"customer/gles2_filter-down2up"
"customer/gles2_filter-rotate-clockwise"
"customer/gles2_filter-rotate-anticlockwise"
)

    for val in ${shader_list[@]}; do
        shadername=$val

        ffmpeg -v debug \
        -ss 0 -t 2 \
        -i $file_path \
        -filter_complex \
        "[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
        [scale0]plusglshader=sdsource=gl/${shadername}.frag:vxsource=gl/${shadername}.vert:duration=5[lut0];
        [lut0]scale=1000:1000,setdar=dar=1000/1000" \
        -vcodec libx264 \
        -an \
        -pix_fmt yuv420p \
        -y gl/$shadername.mp4

        break
    done
}


function main11 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
    #-t 3 -i ./nandaifu-zhan.mov \

    ffmpeg -v debug \
    -framerate 10 -loop 1 -t 3 -i ./test_out.png \
    -t 3 -i ${file_path} \
    -filter_complex "[0:v]scale=2560:1600,setdar=dar=2560/1600,setsar=sar=1/1[scale0];
    [scale0]plusglshader=vxsource=./plusvertex.gl:sdsource=./plusshader.gl[fade1];
    [fade1]scale=1000:1000,setdar=dar=1000/1000,setsar=sar=1/1[out1];
    [1:v][out1]overlay" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4 && ffplay output.mp4
}

function main12 {
    local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
    #local pip_path="./test.mp4"
    #local pip_path="./cat_quqi.jpeg"
    local pip_path="./nandaifu-zhan.mov"

    #shadername="star"
    #"[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    #[lut0]scale=1000:1000,setdar=dar=1000/1000" \

    ffmpeg -v debug \
    -ss 0 -t 3 \
    -i $file_path \
    -filter_complex \
    "[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    [scale0]pipglshader=start=0:duration=3:pip_duration=3:ext_source=${pip_path}:vxsource=gl_pip/pip_vertex.gl:sdsource=gl_pip/pip_shader.gl" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}

function main13 {
    local pip_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k.mp4"
    local file_path="/Users/numberwolf/Documents/webroot/Movies/1000p10s_9k_first.mp4"
    #local pip_path="./cat_quqi.jpeg"
    #local pip_path="./nandaifu-zhan.mov"

    #shadername="star"
    #"[0:v]scale=2560:1600,setsar=sar=1/1,setdar=dar=2560/1600[scale0];
    #[lut0]scale=1000:1000,setdar=dar=1000/1000" \

    #"[0:v]scale=2560:1600[scale0];
    # ,setdar=dar=2560/1600
    ffmpeg -v debug \
    -ss 0 -t 5 \
    -i $file_path \
    -filter_complex \
    "[0:v]scale=2560:1600[scale0];
    [scale0]fadeglshader=offset=4:fade_duration=1:ext_source=${pip_path}:vxsource=gl_fade/fade_vertex.gl:sdsource=gl_fade/fade_shader.gl[effects];
    [effects]scale=1000:1000" \
    -vcodec libx264 \
    -an \
    -pix_fmt yuv420p \
    -y output.mp4
}



#main1
#main2
#main3
#main4
#main5
#main6
#main7
#main7_1
main8
#main9
#main11
#main12
#main13




