#!/bin/bash
#
# Author: porschegt23@foxmail.com
# Wechat: numberwolf11
# QQ    : 531365872
# Github: https://github.com/numberwolf
#

#BASEPATH=`pwd`
BASEPATH="./"
GL_PATH=${BASEPATH}/gl
JPEG_F="${BASEPATH}/cat_quqi.jpeg"
EXAM_SHOW_PATH=${BASEPATH}/example_show
FFMPEG=`which ffmpeg`
if [[ -z "${FFMPEG}" ]]; then
    echo "can not find ffmpeg, set default"
    FFMPEG="ffmpeg"
fi

gl_shaders=(
"sway"
"preview"
"jitter"
"soul"
"split_4"
"split_9"
"split_vert2"
"stab_ci"
"test"
"white_mask"
"split_interval"
)

function execute_render() {
    local shader_name=$1
    echo "================= START Execute ${shader_name} ==================="

    local filter_vert="${GL_PATH}/${shader_name}_vertex.gl"
    local filter_frag="${GL_PATH}/${shader_name}_shader.gl"
    ls $filter_vert
    ls $filter_frag

    # If your device do not hava 'video card'(headless) , you need xvfb-run
    #xvfb-run -a --server-args="-screen 0 1280x720x24 -ac -nolisten tcp -dpi 96 +extension RANDR" \
    ffmpeg -v debug \
    -loop 1 -t 2 -i $JPEG_F \
    -filter_complex "scale=1280:720,plusglshader=sdsource='./${filter_frag}':vxsource='${filter_vert}',scale=-2:200:sws_dither=none" \
    -pix_fmt yuv420p \
    -crf 30 \
    -f gif \
    -r 10 \
    -y ${EXAM_SHOW_PATH}/${shader_name}.gif

    echo "================= FINISH Execute ${shader_name} ==================="
}

for shader_name in ${gl_shaders[@]}; do
    echo ${shader_name}
    execute_render "${shader_name}"
    #break #DEBUG
done

