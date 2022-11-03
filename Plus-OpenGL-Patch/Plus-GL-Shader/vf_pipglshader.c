/**
 * Filter for FFMpeg: Render video by your own shader files
 * @Author  Mail:porschegt23@foxmail.com
 *          QQ: 531365872
 *          Wechat: numberwolf11
 *          Discord: numberwolf#8694
 *          Github: https://github.com/numberwolf
 */
#include <math.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avstring.h"

#include "libavutil/opt.h"
#include "internal.h"

#ifndef OPENGL_RENDER_GRAPH_GL_SHADER_FORAMT
#define OPENGL_RENDER_GRAPH_GL_SHADER_FORAMT
#define	NUMBERWOLF_STRINGIZE(x)	#x
#define	NUMBERWOLF_GL_SHADER(shader) "" NUMBERWOLF_STRINGIZE(shader)
#endif //OPENGL_RENDER_GRAPH_GL_SHADER_FORAMT

#define DEFAULTS (0)
#define EXTERNAL (1)

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

//#define EXT_TYPE_MEDIA 0
//#define EXT_TYPE_RGB24 1
#define TS2T(ts, tb) ((ts) == AV_NOPTS_VALUE ? NAN : (double)(ts)*av_q2d(tb))

static const float position[12] = {
  -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

static const GLchar *v_shader_source = (const GLchar *) NUMBERWOLF_GL_SHADER (
        attribute vec2 position;
        varying vec2 texCoord;

        float PI = 3.1415926;
        uniform float playTime;
        uniform float scale;
        uniform float isPIP;

        void main(void) {
            float duration = 1.0;
            float progress = mod(playTime, duration) / duration; // 0~1
            float moveX = 0.0;
            float moveY = 0.0;
            float rotate = 360.0 * progress * isPIP * 0.0;
            float radians = rotate * PI / 180.0;
            float s = sin(radians);
            float c = cos(radians);
            mat4 zRotation = mat4(
                    c,      -s,     0.0,    0.0, // 1:scale-x 4:y axis - turn anticlockwise
                    s,      c,      0.0,    0.0, // 2:scale-y 4:x axis - turn clockwise , alse for uniform scale
                    0.0,    0.0,    1.0,    0.0,
                    0.0 + moveX,    0.0 + moveY,    0.0,    1.0 // 1:mv right 2:mv up
            );

            texCoord.x = position.x * 0.5 + 0.5;
            texCoord.y = position.y * 0.5 + 0.5;
            gl_Position = zRotation * vec4(position * scale, 0, 1); // zRotation *
            //gl_Position = zRotation * vec4(position * 1., 0, 1); // zRotation *
            //gl_Position = vec4(position, 0, 1); // zRotation *
        }
);

static const GLchar *f_shader_source = (const GLchar *) NUMBERWOLF_GL_SHADER (
        uniform sampler2D tex;
        uniform float playTime;
        uniform float scale;
        varying vec2 texCoord;
        uniform float isPIP;
        void main() {

            float duration = 1.0;
            float progress = mod(playTime, duration) / duration; // 0~1

            //gl_FragColor = texture2D(tex, texCoord * 0.5 + 0.5);
            gl_FragColor = texture2D(tex, texCoord);

            //float top = isPIP * ((1.0 - scale) / 2.0) * scale;
            float bottom = isPIP * ((1.0 - scale) / 2.0 + scale) / scale; // progress
            float top = isPIP * (1.0 - bottom);

            bottom *= progress;
            //float top = isPIP * 0.00;
            //float bottom = isPIP * 0.50;

            // genType step (genType edge, genType x)，genType step (float edge, genType x)
            // if x < edge 0.0 else 1.0
            float topCheck = step(top, texCoord.y);
            float bottomCheck = step(texCoord.y, bottom);

            gl_FragColor.a *= (topCheck * bottomCheck) + (1. - isPIP);
            gl_FragColor.a = min(1.0, gl_FragColor.a);
            //float gray = (gl_FragColor.r + gl_FragColor.g + gl_FragColor.b) / 3.0;
            //gl_FragColor.r = gray;
            //gl_FragColor.g = gray;
            //gl_FragColor.b = gray;
        }
);

#define PIXEL_FORMAT GL_RGBA

typedef struct AVMediaContext {
    AVFormatContext         *m_formatCtx;
    AVCodecContext          *m_vCodecContext;
    AVPacket                *m_decPacket;
    AVFrame                 *m_frame;
    double                  m_vTimebase;
    AVFrame      			*outFrame;
    struct SwsContext       *swCtx;
    uint8_t                 *out_buffer;
    int                     isGetFrame;

    // GL
    int             m_ext_width;
    int             m_ext_height;
} AVMediaContext;

typedef struct {
    const AVClass *class;
    //FFFrameSync frameSync;

    // @Param sdsource shader
    char            *sdsource;
    // @Param vxsource vertex
    char            *vxsource;
    // @Param start render
    int64_t         r_start_time;
    int64_t         r_start_time_tb;
    double          r_start_time_ft;
    // @Param duration render
    int64_t         duration;
    int64_t         duration_tb;
    double          duration_ft;
    // @Param pip_duration render
    int64_t         pip_duration;
    int64_t         pip_duration_tb;
    double          pip_duration_ft;
    // @param ext media
    char            *ext_source;
    //int             ext_type;

    int             debug_count;

    // media
    AVMediaContext  extAvMediaContext;

    // 判断通道值
    int alpha;
    int pix_fmt;

    // aspect
    double aspect_pip_w;
    double aspect_pip_h;

    // transTo RGBA
    AVFrame      			*m_outFrame;
    struct SwsContext       *m_swCtx;
    uint8_t                 *m_out_buffer;

    // time
    double          startPlayTime;
    double          playTime_ft;
    AVRational      vTimebase;

    // input shader vertex
    GLchar          *sdsource_data;
    GLchar          *vxsource_data;
    GLint           playTime;
    GLint           u_scale_w;
    GLint           u_scale_h;
    GLint           u_isPIP;
    GLint           u_mainDuration;
    GLint           u_pipDuration;
    GLint           u_startTime;
    GLint           u_mainWidth;
    GLint           u_mainHeight;
    GLint           u_pipWidth;
    GLint           u_pipHeight;
    // GL Obj
    GLuint          program;
    GLuint          frame_tex; // v1
    GLuint          ext_frame_tex; // v2
    GLFWwindow      *window;
    GLuint          pos_buf;
} PipGLShaderContext;

/**
 * @brief cli params
 */
#define OFFSET(x) offsetof(PipGLShaderContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM
static const AVOption pipglshader_options[] = {
    {"sdsource", "gl fragment shader source path (default is render lut fragment)", OFFSET(sdsource), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS},
    {"vxsource", "gl vertex shader source path (default is render lut vertex)", OFFSET(vxsource), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS},
    {"start", "gl render start timestamp, if you set this option, must greater than zero(no trim)", OFFSET(r_start_time), AV_OPT_TYPE_DURATION, {.i64 = 0.}, 0, INT64_MAX, FLAGS},
    {"duration", "gl render duration, if you set this option, must greater than zero(no trim)", OFFSET(duration), AV_OPT_TYPE_DURATION, {.i64 = 0.}, 0, INT64_MAX, FLAGS},
    {"pip_duration", "gl render pip picture's duration, if you set this option, must greater than zero(no trim)", OFFSET(pip_duration), AV_OPT_TYPE_DURATION, {.i64 = 0.}, 0, INT64_MAX, FLAGS},
    {"ext_source", "gl texture of pip source media file (default is null) ", OFFSET(ext_source), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS},
    {NULL}
}; // pipglshader_options

AVFILTER_DEFINE_CLASS(pipglshader);

static const enum AVPixelFormat alpha_pix_fmts[] = {
    AV_PIX_FMT_ARGB, AV_PIX_FMT_ABGR, AV_PIX_FMT_RGBA,
    AV_PIX_FMT_BGRA, AV_PIX_FMT_NONE
}; // alpha_pix_fmts

static GLuint build_shader(AVFilterContext *ctx, const GLchar *shader_source, GLenum type) {
    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_shader\n");

    GLuint shader = glCreateShader(type);
    if (!shader || !glIsShader(shader)) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_pipglshader build_shader glCreateShader glIsShader FAILED!\n");
        return 0;
    }
    glShaderSource(shader, 1, &shader_source, 0);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
   
    // error message
    int InfoLogLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength); 
    if (InfoLogLength > 0) {
        //std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
        char *ShaderErrorMessage = (char *)malloc(InfoLogLength);

        glGetShaderInfoLog(shader, InfoLogLength, NULL, &ShaderErrorMessage);
        av_log(ctx, AV_LOG_ERROR, "doing vf_pipglshader build_shader ERROR: %s\n", &ShaderErrorMessage);
    }

    GLuint ret = status == GL_TRUE ? shader : 0;
    return ret;
} // build_shader

static void vbo_setup(PipGLShaderContext *gs) {
    glGenBuffers(1, &gs->pos_buf);
    glBindBuffer(GL_ARRAY_BUFFER, gs->pos_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

    GLint loc = glGetAttribLocation(gs->program, "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
} // vbo_setup

static void tex_setup(AVFilterLink *inlink) {
    AVFilterContext     *ctx    = inlink->dst;
    PipGLShaderContext  *gs    = ctx->priv;
    gs->debug_count             = 0;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader tex_setup\n");

    glGenTextures(1, &gs->frame_tex);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, gs->frame_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, gs->pix_fmt, inlink->w, inlink->h, 0, gs->pix_fmt, GL_UNSIGNED_BYTE, NULL);

    glUniform1i(glGetUniformLocation(gs->program, "tex"), 0);
} // tex_setup

static int final_release_av(AVFilterLink *inlink)
{
    AVFilterContext     *ctx    = inlink->dst;
    PipGLShaderContext *gs     = ctx->priv;

    av_log(NULL, AV_LOG_DEBUG,
           "doing vf_pipglshader final_release_av start\n");

    if (gs->extAvMediaContext.m_frame != NULL) {
        av_frame_free(&gs->extAvMediaContext.m_frame);
        gs->extAvMediaContext.m_frame = NULL;
    }

    if (gs->extAvMediaContext.outFrame != NULL) {
        av_frame_free(&gs->extAvMediaContext.outFrame);
        gs->extAvMediaContext.outFrame = NULL;
    }

    if (gs->extAvMediaContext.m_decPacket != NULL) {
        av_packet_unref(gs->extAvMediaContext.m_decPacket);
        gs->extAvMediaContext.m_decPacket = NULL;
    }

    if (NULL != gs->extAvMediaContext.m_vCodecContext) {
        avcodec_close(gs->extAvMediaContext.m_vCodecContext);
        gs->extAvMediaContext.m_vCodecContext = NULL;
    }

    if (NULL != gs->extAvMediaContext.swCtx) {
        sws_freeContext(gs->extAvMediaContext.swCtx);
        gs->extAvMediaContext.swCtx = NULL;
    }

    if (NULL != gs->extAvMediaContext.m_formatCtx) {
        avformat_close_input(&gs->extAvMediaContext.m_formatCtx);
        gs->extAvMediaContext.m_formatCtx = NULL;
    }

    av_log(NULL, AV_LOG_DEBUG,
           "doing vf_pipglshader final_release_av FINISHED\n");

    return 0;
} // end final_release_av

static int open_ext_source(AVFilterLink *inlink) {
    AVFilterContext     *ctx    = inlink->dst;
    PipGLShaderContext *gs     = ctx->priv;
    int                 ret     = 0;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source\n");

    gs->extAvMediaContext.m_formatCtx       = NULL;
    gs->extAvMediaContext.m_vCodecContext   = NULL;
    gs->extAvMediaContext.m_decPacket       = NULL;
    gs->extAvMediaContext.m_frame           = NULL;
    gs->extAvMediaContext.outFrame          = NULL;
    gs->extAvMediaContext.swCtx             = NULL;
    gs->extAvMediaContext.out_buffer        = NULL;
    gs->extAvMediaContext.isGetFrame        = 0;

    gs->extAvMediaContext.m_ext_width       = 0;
    gs->extAvMediaContext.m_ext_height      = 0;

    gs->aspect_pip_w                        = 0.0;
    gs->aspect_pip_h                        = 0.0;

    int vIndex = -1;
    if (gs->ext_source != NULL) {
        /*
         * begin
         */
        gs->extAvMediaContext.m_formatCtx = avformat_alloc_context();

        if ((ret = avformat_open_input(&gs->extAvMediaContext.m_formatCtx, gs->ext_source, NULL, NULL)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader open_ext_source Cannot open input file\n");
            final_release_av(inlink);
            return ret;
        }

        if ((ret = avformat_find_stream_info(gs->extAvMediaContext.m_formatCtx, NULL)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader open_ext_source Cannot find stream information\n");
            final_release_av(inlink);
            return ret;
        }

        for (int i = 0; i < gs->extAvMediaContext.m_formatCtx->nb_streams; i++) {

            AVStream *contex_stream = gs->extAvMediaContext.m_formatCtx->streams[i];
            enum AVCodecID codecId = contex_stream->codecpar->codec_id;

            if (contex_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                vIndex = i;

                gs->extAvMediaContext.m_vTimebase = av_q2d(contex_stream->time_base);

                gs->extAvMediaContext.m_frame   = av_frame_alloc();
                gs->extAvMediaContext.outFrame  = av_frame_alloc();
                if (!gs->extAvMediaContext.m_frame || !gs->extAvMediaContext.outFrame) {
                    av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source av_frame_alloc out frame error\n");
                    break;
                }

                /*
                 * Decoder
                 */
                AVCodec *dec = avcodec_find_decoder(codecId);
                const char* codec_name = avcodec_get_name(codecId);

                if (!dec) {
                    av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader open_ext_source Failed to find decoder video for stream #%u codec:%s\n", i, codec_name);
                    ret = AVERROR_DECODER_NOT_FOUND;
                    final_release_av(inlink);
                    return ret;
                }

                gs->extAvMediaContext.m_vCodecContext = avcodec_alloc_context3(dec);

                if (!gs->extAvMediaContext.m_vCodecContext) {
                    av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader open_ext_source Failed to allocate the video decoder context for stream #%u\n", i);
                    ret = AVERROR(ENOMEM);
                    final_release_av(inlink);
                    return ret;
                }

                ret = avcodec_parameters_to_context(gs->extAvMediaContext.m_vCodecContext, contex_stream->codecpar);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader open_ext_source Failed to copy video decoder parameters to input decoder context "
                           "for stream #%u\n", i);
                    final_release_av(inlink);
                    return ret;
                }

                //const char *codec_name = avcodec_get_name(sniffStreamContext->m_vCodecContext->codec_id);
                av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source video codec name:%s\n", codec_name);

                ret = avcodec_open2(gs->extAvMediaContext.m_vCodecContext, dec, NULL);
                av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source avcodec_open2 ret:%d\n", ret);

                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader open_ext_source avcodec_open2 初始化解码器失败\n");
                    final_release_av(inlink);
                    return ret;
                }

                //m_decPacket = av_packet_alloc();
                //av_init_packet(m_decPacket);

                av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source start allocc avpacket\n");
                gs->extAvMediaContext.m_decPacket = av_packet_alloc();

                av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source allocc avpacket is NULL:%d\n",
                        gs->extAvMediaContext.m_decPacket == NULL);

                av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source start init avpacket\n");
                av_init_packet(gs->extAvMediaContext.m_decPacket);

                av_log(NULL, AV_LOG_DEBUG,
                        "doing vf_pipglshader open_ext_source av_init_packet finished avpacket is NULL:%d\n",
                        gs->extAvMediaContext.m_decPacket == NULL);

                if (!gs->extAvMediaContext.m_decPacket) {
                    av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_source av_frame_alloc-packet 初始化解码器失败\n");
                    av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader open_ext_source av_frame_alloc-packet 初始化解码器失败\n");
                    ret = -1;
                    final_release_av(inlink);
                    return ret;
                }
                break; // select video, break
            } // END if check is video stream

        } // end for m_formatCtx->nb_streams

    }  // end if ext_source exists


    av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader open_ext_sourcedebug open_ext_source SUCCESS!!!!!! video index : %d\n",
            gs->extAvMediaContext.m_formatCtx->streams[vIndex]->index);
    return ret;
} // open_ext_source

static int render_rgba_frame(AVFilterLink *inlink) {
    AVFilterContext     *ctx   = inlink->dst;
    PipGLShaderContext  *gs    = ctx->priv;

    int sW = inlink->w;
    int sH = inlink->h;
    int picW = gs->extAvMediaContext.m_ext_width;
    int picH = gs->extAvMediaContext.m_ext_height;

    if (picW <= 0 || picH <= 0) {
        return -1;
    }

    av_log(NULL, AV_LOG_DEBUG, 
        "doing vf_pipglshader render_rgba_frame screen %dx%d pic:%dx%d\n",
        sW, sH, picW, picH);

    if (!gs->aspect_pip_w || gs->aspect_pip_w <= 0.0) {
        double fixedWP = (double) picW / (double) sW;
        double fxiedHP = (double) picH / (double) sH;

        double fixedSW = (double) sW / (double) picW;
        double fixedSH = (double) sH / (double) picH;

        av_log(NULL, AV_LOG_DEBUG, 
            "doing vf_pipglshader render_rgba_frame fixedW %f %f fixedS %f %f\n",
            fixedWP, fxiedHP, fixedSW, fixedSH);
        
        // let scaleRatio = biggerWidth ? fixedWidth : fixedHeight;

        float scaleRatio = 1.0;
        if (fixedWP > fxiedHP) {
            scaleRatio = fixedSW; 
        } else {
            scaleRatio = fixedSH;
        }

        gs->aspect_pip_w = scaleRatio * (double) picW / (double) sW;
        gs->aspect_pip_h = scaleRatio * (double) picH / (double) sH;

        av_log(NULL, AV_LOG_DEBUG, 
            "doing vf_pipglshader render_rgba_frame scale ratio %f %f %f\n",
            scaleRatio, gs->aspect_pip_w, gs->aspect_pip_h);
    }

    glUniform1f(gs->u_scale_w, gs->aspect_pip_w);
    glUniform1f(gs->u_scale_h, gs->aspect_pip_h);



    int ret = 0;
    // start
    //glActiveTexture(GL_TEXTURE0 + 1);
    //glBindTexture(GL_TEXTURE_2D, gs->ext_frame_tex);
    glBindTexture(GL_TEXTURE_2D, gs->frame_tex);
    //glActiveTexture(GL_TEXTURE0);
    /*
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    */

    glTexImage2D(
            GL_TEXTURE_2D, 0, PIXEL_FORMAT, picW, picH,
            0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, gs->extAvMediaContext.outFrame->data[0]);
    glActiveTexture(GL_TEXTURE0);

    GLuint textureTemp = 0;

    //glActiveTexture(GL_TEXTURE0 + 1);
    //glBindTexture(GL_TEXTURE_2D, gs->ext_frame_tex);

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gs->frame_tex);
    glActiveTexture(GL_TEXTURE0);

    //glUniform1i(glGetUniformLocation(gs->program, "externTex"), 1);
    glUniform1i(glGetUniformLocation(gs->program, "tex"), 0);

    glUniform1f(gs->u_pipWidth, picW);
    glUniform1f(gs->u_pipHeight, picH);

    glUniform1f(gs->playTime, gs->playTime_ft);

    //glUniform1f(gs->u_scale, 0.5);

    glUniform1f(gs->u_isPIP, 1.0);

    /*************************** Crop *****************************
     * CANVAS
     * Y 720px
     * ^
     * |
     * |
     * |
     * |
     * |
     * O(0,0) ----------------> X 1280px
     */
    //glScissor(640, 400, 1280, 800);
    //glEnable(GL_SCISSOR_TEST);
    //glEnable(GL_BLEND);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //glDisable(GL_BLEND);
    //glDisable(GL_SCISSOR_TEST);

    //glDrawArrays(GL_TRIANGLES, 0, 6);
    // end
    return ret;
}

static int ext_get_frame(AVFilterLink *inlink) {
    AVFilterContext     *ctx    = inlink->dst;
    PipGLShaderContext  *gs    = ctx->priv;
    int ret = 0;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader ext_get_frame\n");

    if (gs->extAvMediaContext.out_buffer != NULL) {
        /*
         * free
         */
        av_free(gs->extAvMediaContext.out_buffer);
        gs->extAvMediaContext.out_buffer = NULL;
    } // end free out_buffer

    // get frames
    if (av_read_frame(gs->extAvMediaContext.m_formatCtx, gs->extAvMediaContext.m_decPacket) >= 0) 
    {
        char szError[256] = {0};
        ret = avcodec_send_packet(gs->extAvMediaContext.m_vCodecContext, gs->extAvMediaContext.m_decPacket);

        if (ret == AVERROR(EAGAIN)) {
            av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader ext_get_framesendRet ===========> EAGAIN\n");
            return ret;
        } else if (ret == AVERROR_EOF) {
            av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader ext_get_framesendRet ===========> AVERROR_EOF\n");
            return ret;
        } else if (ret == AVERROR(EINVAL)) {
            av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader ext_get_framesendRet ===========> EINVAL\n");
            return ret;
        } else if (ret == AVERROR(ENOMEM)) {
            av_log(NULL, AV_LOG_ERROR, "doing vf_pipglshader ext_get_framesendRet ===========> ENOMEM\n");
            return ret;
        } else {
            // undo
        } // end ret = avcodec_send_packet

        if (ret == 0) {
            int rec_re = 0;
            while (1) {
                rec_re = avcodec_receive_frame(gs->extAvMediaContext.m_vCodecContext, gs->extAvMediaContext.m_frame);
                if (rec_re == 0) {
                    av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader ext_get_frame debug open img after decode %d x %d\n",
                           gs->extAvMediaContext.m_frame->width, gs->extAvMediaContext.m_frame->height);

                    gs->extAvMediaContext.isGetFrame = 1;
                    if (NULL == gs->extAvMediaContext.swCtx) {
                        av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader ext_get_frame start init swscale\n");
                        gs->extAvMediaContext.swCtx = sws_getContext(
                                gs->extAvMediaContext.m_frame->width, gs->extAvMediaContext.m_frame->height,
                                gs->extAvMediaContext.m_vCodecContext->pix_fmt, // in ,vcodec->frame->format
                                gs->extAvMediaContext.m_frame->width, gs->extAvMediaContext.m_frame->height,
                                //AV_PIX_FMT_RGB24, // out
                                AV_PIX_FMT_RGBA, // out
                                SWS_FAST_BILINEAR, NULL, NULL, NULL);
                    }

                    gs->extAvMediaContext.outFrame->width     = gs->extAvMediaContext.m_frame->width;
                    gs->extAvMediaContext.outFrame->height    = gs->extAvMediaContext.m_frame->height;
                    //gs->extAvMediaContext.outFrame->format    = AV_PIX_FMT_RGB24;
                    gs->extAvMediaContext.outFrame->format    = AV_PIX_FMT_RGBA;

                    //int lut_width = gs->extAvMediaContext.outFrame->width;
                    if (ff_fmt_is_in(gs->extAvMediaContext.m_vCodecContext->pix_fmt, alpha_pix_fmts)) {
                        gs->extAvMediaContext.m_ext_width = gs->extAvMediaContext.m_frame->linesize[0] / 4;
                    } else {
                        gs->extAvMediaContext.m_ext_width = gs->extAvMediaContext.m_frame->linesize[0];
                    }
                    gs->extAvMediaContext.m_ext_height = gs->extAvMediaContext.outFrame->height;

                    int rgb24size = gs->extAvMediaContext.m_ext_width * gs->extAvMediaContext.m_frame->height * 4;

                    if (gs->extAvMediaContext.out_buffer != NULL) {
                        /*
                         * free
                         */
                        av_free(gs->extAvMediaContext.out_buffer);
                        gs->extAvMediaContext.out_buffer = NULL;
                    } // end free out_buffer

                    gs->extAvMediaContext.out_buffer = (uint8_t *)av_malloc((int)(rgb24size) * sizeof(uint8_t));
                    //avpicture_fill(
                    //        (AVPicture *)outFrame, out_buffer,
                    //        AV_PIX_FMT_RGB24,
                    //        m_frame->width, m_frame->height);

                    av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader ext_get_frame start fill picture\n");
                    // int av_image_fill_arrays(uint8_t *dst_data[4], int dst_linesize[4],
                    //                         const uint8_t *src, enum AVPixelFormat pix_fmt,
                    //                         int width, int height, int align)
                    av_image_fill_arrays(
                        gs->extAvMediaContext.outFrame->data, gs->extAvMediaContext.outFrame->linesize,
                        //gs->extAvMediaContext.out_buffer, AV_PIX_FMT_RGB24,
                        gs->extAvMediaContext.out_buffer, AV_PIX_FMT_RGBA,
                        gs->extAvMediaContext.m_ext_width, gs->extAvMediaContext.m_frame->height, 1);

                    av_log(NULL, AV_LOG_DEBUG,
                            "doing vf_pipglshader ext_get_frame start swscale picture:"
                            "swCtx is null:%d, mframe is null:%d, outFrame is null:%d, "
                            "mframeLineSize:%d, mframeHeight:%d, "
                            "outFrameLineSize:%d ,rgb24size:%d\n",
                            gs->extAvMediaContext.swCtx == NULL,
                            gs->extAvMediaContext.m_frame->data == NULL,
                            gs->extAvMediaContext.outFrame->data == NULL,
                            gs->extAvMediaContext.m_frame->linesize[0], gs->extAvMediaContext.m_frame->height,
                            gs->extAvMediaContext.outFrame->linesize[0], rgb24size
                    );

                    sws_scale(gs->extAvMediaContext.swCtx,
                              (const uint8_t* const*)gs->extAvMediaContext.m_frame->data,
                              gs->extAvMediaContext.m_frame->linesize, // (const uint8_t* const*)
                              0, gs->extAvMediaContext.m_frame->height,
                              gs->extAvMediaContext.outFrame->data,
                              gs->extAvMediaContext.outFrame->linesize);

                    av_log(NULL, AV_LOG_DEBUG,
                            "doing vf_pipglshader ext_get_frame debug open img after prepare %d x %d\n",
                            gs->extAvMediaContext.outFrame->width, gs->extAvMediaContext.outFrame->height);


                    //FILE *fw = fopen("./test4_out.rgb", "wb");
                    //fwrite(gs->extAvMediaContext.outFrame->data[0], 1, rgb24size, fw);
                    //fclose(fw);

                    av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader ext_get_frame start init rgb\n");
                    // active
                    av_log(NULL, AV_LOG_DEBUG, "doing vf_pipglshader ext_get_frame start bind texture\n");
                    
                    //render_rgba_frame(inlink);
                    /*
                     * free
                     */
                    //av_free(gs->extAvMediaContext.out_buffer);
                    //gs->extAvMediaContext.out_buffer = NULL;

                    break;

                } else {
                    //render_rgba_frame(inlink);
                    return rec_re;
                } // av receive frame ret >= 0

            } // END while(1) loop receive frame
        } // av end packet ret >= 0

        if (ret < 0 || gs->extAvMediaContext.isGetFrame > 0) {
            //render_rgba_frame(inlink);
            return ret;
        } // END if ret < 0 || isGetFrame

    } // END if read avframe

    return ret;
} // ext_get_frame

static int ext_source_prepare(AVFilterLink *inlink) {
    AVFilterContext     *ctx    = inlink->dst;
    PipGLShaderContext  *gs    = ctx->priv;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader ext_source_prepare\n");
    /*
     * media image
     */
    int ret = open_ext_source(inlink);
    if (ret < 0) {
        av_log(ctx, AV_LOG_ERROR,
               "doing vf_pipglshader ext_source_prepare: invalid lut path:%s open_ext_source ,error ret %d\n",
               gs->ext_source, ret);
        return ret;
    } // end open_ext_source check

    //av_log(NULL, AV_LOG_DEBUG,
    //        "debug set ext params value wh:%dx%d\n",
    //        gs->lut_width, gs->lut_height);
    return 0;
} // ext_source_prepare

static int tex_setup_lut(AVFilterLink *inlink) {

    AVFilterContext     *ctx    = inlink->dst;
    PipGLShaderContext  *gs    = ctx->priv;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader tex_setup_lut\n");

    glGenTextures(1, &gs->ext_frame_tex);
    glActiveTexture(GL_TEXTURE0 + 1);

    glBindTexture(GL_TEXTURE_2D,  gs->ext_frame_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gs->lut_width, gs->lut_height, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, NULL);
    glUniform1i(glGetUniformLocation(gs->program, "tex"), 1);

    return 0;
} // tex_setup_lut

/**
 * @brief Build shader vertex
 */
static int build_program(AVFilterContext *ctx) {
    av_log(ctx, AV_LOG_DEBUG, "start doing vf_pipglshader build_program action\n");
    GLuint v_shader, f_shader;
    PipGLShaderContext *gs = ctx->priv;

    // gl function codes
    // init
    gs->sdsource_data = NULL;
    gs->vxsource_data = NULL;

    /*
     * fragments shader
     */
    if (gs->sdsource) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_program shader params: %s\n", gs->sdsource);
        FILE *f = fopen(gs->sdsource, "rb");
        if (!f) {
            av_log(ctx, AV_LOG_ERROR, 
                "doing vf_pipglshader build_program shader: invalid shader source file \"%s\"\n", gs->sdsource);
            return -1;
        }

        // get file size
        fseek(f, 0, SEEK_END);
        unsigned long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        gs->sdsource_data = malloc(fsize + 1);
        fread(gs->sdsource_data, fsize, 1, f);
        fclose(f);
        gs->sdsource_data[fsize] = 0;

    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_program no shader param, use default option\n");
        // ...
    }

    /*
     * vertex shader
     */
    if (gs->vxsource) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_program vertex params: %s\n", gs->vxsource);
        FILE *f = fopen(gs->vxsource, "rb");
        if (!f) {
            av_log(ctx, AV_LOG_ERROR, 
                "doing vf_pipglshader build_program shader: invalid shader source file \"%s\"\n", gs->vxsource);
            return -1;
        }

        // get file size
        fseek(f, 0, SEEK_END);
        unsigned long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        gs->vxsource_data = malloc(fsize + 1);
        fread(gs->vxsource_data, fsize, 1, f);
        fclose(f);
        gs->vxsource_data[fsize] = 0;

    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_program no vertex param, use default option\n");
        // ...
    }

    const char *gl_sdsource_dst = gs->sdsource_data ? gs->sdsource_data : f_shader_source;
    const char *gl_vxsource_dst = gs->vxsource_data ? gs->vxsource_data : v_shader_source;

    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_pipglshader build_program build_shader debug shaders ===================================>\n");
    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_pipglshader build_program build_shader use fragment shaders:\n%s\n", gl_sdsource_dst);
    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_pipglshader build_program build_shader use vertex shader:\n%s\n", gl_vxsource_dst);

    if (gs->r_start_time > 0) {
        gs->r_start_time_tb = av_rescale_q(gs->r_start_time, AV_TIME_BASE_Q, gs->vTimebase);
        gs->r_start_time_ft = TS2T(gs->r_start_time_tb, gs->vTimebase);
    } else {
        gs->duration_tb = 0;
        gs->duration_ft = 0;
    }

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader r_start_time:%ld, r_start_time_tb:%ld, r_start_time_ft:%f\n",
           gs->r_start_time, gs->r_start_time_tb, gs->r_start_time_ft);

    if (gs->duration > 0) {
        //gs->duration_tb = TS2T(gs->duration, gs->vTimebase);
        gs->duration += gs->r_start_time;
        gs->duration_tb = av_rescale_q(gs->duration, AV_TIME_BASE_Q, gs->vTimebase);
        gs->duration_ft = TS2T(gs->duration_tb, gs->vTimebase);
    } else {
        gs->duration_tb = -1;
        gs->duration_ft = -1;
    }
    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader duration:%ld, duration_tb:%ld, duration_ft:%f\n",
           gs->duration, gs->duration_tb, gs->duration_ft);

    // pip duration
    if (gs->pip_duration > 0) {
        gs->pip_duration += gs->r_start_time;
        gs->pip_duration_tb = av_rescale_q(gs->pip_duration, AV_TIME_BASE_Q, gs->vTimebase);
        gs->pip_duration_ft = TS2T(gs->pip_duration_tb, gs->vTimebase);
    } else {
        gs->pip_duration_tb = -1;
        gs->pip_duration_ft = -1;
    }
    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader pip duration:%ld, pip duration_tb:%ld, pip duration_ft:%f\n",
           gs->pip_duration, gs->pip_duration_tb, gs->pip_duration_ft);


    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_program build_shader\n");
    /*
    if (!((v_shader = build_shader(ctx, v_shader_source, GL_VERTEX_SHADER)) &&
        (f_shader = build_shader(ctx, f_shader_source, GL_FRAGMENT_SHADER)))) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_pipglshader build_program failed!\n");
        return -1;
    }
    */

    if (!((v_shader = build_shader(ctx, gl_vxsource_dst, GL_VERTEX_SHADER)) &&
        (f_shader = build_shader(ctx, gl_sdsource_dst, GL_FRAGMENT_SHADER)))) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_pipglshader build_program failed!\n");
        return -1;
    }
    // build shader finished

    // render shader object
    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_program create program\n");
    gs->program = glCreateProgram();
    glAttachShader(gs->program, v_shader);
    glAttachShader(gs->program, f_shader);
    glLinkProgram(gs->program);

    GLint status;
    glGetProgramiv(gs->program, GL_LINK_STATUS, &status);
    if (gs->sdsource_data) {
        free(gs->sdsource_data);
        gs->sdsource_data = NULL;
    }
    if (gs->vxsource_data) {
        free(gs->vxsource_data);
        gs->vxsource_data = NULL;
    }

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader build_program finished vf_pipglshader build_program!\n");
    return status == GL_TRUE ? 0 : -1;
} // build_program

/*
static int activate(AVFilterContext *ctx) {
    PipGLShaderContext *c = ctx->priv;
    return ff_framesync_activate(&c->frameSync);
}
*/


/**
 * @brief setup uniform values
 * playTime
 */
static void uni_setup(AVFilterLink *inLink) {
    AVFilterContext         *ctx = inLink->dst;
    PipGLShaderContext     *gs = ctx->priv;

    gs->playTime        = glGetUniformLocation(gs->program, "playTime");
    gs->u_scale_w       = glGetUniformLocation(gs->program, "scale_w");
    gs->u_scale_h       = glGetUniformLocation(gs->program, "scale_h");
    gs->u_isPIP         = glGetUniformLocation(gs->program, "isPIP");
    gs->u_mainDuration  = glGetUniformLocation(gs->program, "mainDuration");
    gs->u_pipDuration   = glGetUniformLocation(gs->program, "pipDuration");
    gs->u_startTime     = glGetUniformLocation(gs->program, "startTime");

    gs->u_mainWidth     = glGetUniformLocation(gs->program, "mainWidth");
    gs->u_mainHeight    = glGetUniformLocation(gs->program, "mainHeight");
    gs->u_pipWidth      = glGetUniformLocation(gs->program, "pipWidth");
    gs->u_pipHeight     = glGetUniformLocation(gs->program, "pipHeight");

    glUniform1f(gs->playTime, 0.0f);
    glUniform1f(gs->u_scale_w, 1.0f);
    glUniform1f(gs->u_scale_h, 1.0f);
    glUniform1f(gs->u_isPIP, 0.0f);
    glUniform1f(gs->u_mainDuration, 0.0f);
    glUniform1f(gs->u_pipDuration, 0.0f);
    glUniform1f(gs->u_startTime, 0.0f);

    glUniform1f(gs->u_mainWidth, 0.0f);
    glUniform1f(gs->u_mainHeight, 0.0f);
    glUniform1f(gs->u_pipWidth, 0.0f);
    glUniform1f(gs->u_pipHeight, 0.0f);
} // uni_setup

static av_cold int init(AVFilterContext *ctx) {
    av_log(ctx, AV_LOG_INFO, "/**\n");
    av_log(ctx, AV_LOG_INFO, " * OpenGL Shader Filter for FFMpeg: Render video by your own shader files\n");
    av_log(ctx, AV_LOG_INFO, " * @Author  Mail:porschegt23@foxmail.com\n");
    av_log(ctx, AV_LOG_INFO, " *          QQ: 531365872\n");
    av_log(ctx, AV_LOG_INFO, " *          Wechat: numberwolf11\n");
    av_log(ctx, AV_LOG_INFO, " *          Discord: numberwolf#8694\n");
    av_log(ctx, AV_LOG_INFO, " *          Github: https://github.com/numberwolf\n");
    av_log(ctx, AV_LOG_INFO, " */\n");
    return glfwInit() ? 0 : -1;
} // init


static int config_props(AVFilterLink *inlink) {
    AVFilterContext    *ctx     = inlink->dst;
    PipGLShaderContext *gs      = ctx->priv;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader config_props total frames in:%d out:%d\n",
            inlink->frame_count_in, inlink->frame_count_out);

    // trans RGBA init
    gs->m_outFrame      = NULL;
    gs->m_out_buffer    = NULL;
    gs->m_swCtx         = NULL;

    // alpha
    gs->alpha = ff_fmt_is_in(inlink->format, alpha_pix_fmts);
    av_log(ctx, AV_LOG_DEBUG, "gs->alpha: %d, inlink->format: %d\n", gs->alpha, inlink->format);

    //get alpha info
    if (gs->alpha) {
        gs->pix_fmt = GL_RGBA;
    } else {
        gs->pix_fmt = GL_RGB;
    }

    gs->startPlayTime           = -1;

    glfwWindowHint(GLFW_VISIBLE, 0);
    gs->window = glfwCreateWindow(inlink->w, inlink->h, "", NULL, NULL);

    glfwMakeContextCurrent(gs->window);

    #ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
    #endif

    ext_source_prepare(inlink); // lut prepare

    glViewport(0, 0, inlink->w, inlink->h);
    gs->vTimebase = inlink->time_base;

    int ret;
    if((ret = build_program(ctx)) < 0) {
        return ret;
    }

    glUseProgram(gs->program);

    glEnable(GL_ALPHA);
    glEnable(GL_BLEND);
    //glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vbo_setup(gs);
    tex_setup(inlink);
    int setupLutRet = tex_setup_lut(inlink);
    if (setupLutRet < 0) {
        exit(setupLutRet);
    }
    uni_setup(inlink);
    return 0;
} // config_props

static int config_props_external(AVFilterLink *inlink) {
    AVFilterContext    *ctx     = inlink->dst;
    PipGLShaderContext *gs      = ctx->priv;

    av_log(ctx, AV_LOG_DEBUG, "debug vf_pipglshader config props get luts data:%d, %d\n", inlink->w, inlink->h);

    return 0;
} // config_props_external

static int config_output(AVFilterLink *outlink)
{
    AVFilterContext *ctx    = outlink->src;
    PipGLShaderContext *s  = ctx->priv;
    int ret;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader config_output\n");

    //if ((ret = ff_framesync_init_dualinput(&s->fs, ctx)) < 0)
    //    return ret;

    outlink->w = ctx->inputs[DEFAULTS]->w;
    outlink->h = ctx->inputs[DEFAULTS]->h;
    outlink->time_base = ctx->inputs[DEFAULTS]->time_base;

    av_log(ctx, AV_LOG_DEBUG, "debug config_output output size:%d, %d\n", outlink->w, outlink->h);

    //return ff_framesync_configure(&s->fs);
    return 0;
} // config_output

// apply
static int filter_frame(AVFilterLink *inlink, AVFrame *in) {
    AVFilterContext *ctx    = inlink->dst;
    // AVFilterLink *inlink    = ctx->inputs[0];
    AVFilterLink *outlink   = ctx->outputs[0];
    PipGLShaderContext *gs = ctx->priv;

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader filter_frame\n");

    gs->playTime_ft = TS2T(in->pts, gs->vTimebase);
    // check start time
    if (gs->startPlayTime < 0) {
        gs->startPlayTime = gs->playTime_ft;
    }
    gs->playTime_ft -= gs->startPlayTime;
    av_log(ctx, AV_LOG_DEBUG,
           "doing vf_pipglshader filter_frame start vf_pipglshader filter_frame get pts:%ld ,time->%f, duration:%f\n",
           in->pts, gs->playTime_ft, gs->duration_ft);

    AVFrame *out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
    if (!out) {
        av_frame_free(&in);
        return AVERROR(ENOMEM);
    }

    int copy_props_ret = av_frame_copy_props(out, in);
    if (copy_props_ret < 0) {
        av_frame_free(&out);
        return -1;
    }
    glfwMakeContextCurrent(gs->window);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gs->program);

    /******************************************************
     *
     *
     *                      Render
     *
     *
     ******************************************************/
    if ( // check if render
            (gs->duration_ft < 0 || (gs->duration_ft > 0 && gs->playTime_ft <= gs->duration_ft))
            && gs->playTime_ft >= gs->r_start_time_ft)
    {
        av_log(ctx, AV_LOG_DEBUG,
               "doing vf_pipglshader filter_frame gl render pts:%ld ,time->%f, duration:%f\n",
               in->pts, gs->playTime_ft, gs->duration_ft);

        glUniform1f(gs->playTime, gs->playTime_ft);
        glUniform1f(gs->u_scale_w, 1.0);
        glUniform1f(gs->u_scale_h, 1.0);
        glUniform1f(gs->u_isPIP, 0.0);
        glUniform1f(gs->u_mainDuration, gs->duration_ft - gs->r_start_time_ft);
        glUniform1f(gs->u_pipDuration, gs->pip_duration_ft - gs->r_start_time_ft);
        glUniform1f(gs->u_startTime, gs->r_start_time_ft);

        glUniform1f(gs->u_mainWidth, inlink->w);
        glUniform1f(gs->u_mainHeight, inlink->h);

        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, gs->frame_tex);
        //glPixelStorei(GL_UNPACK_ROW_LENGTH, in->linesize[0] / 3);
        glTexImage2D(GL_TEXTURE_2D, 0, gs->pix_fmt, inlink->w, inlink->h, 0, gs->pix_fmt, GL_UNSIGNED_BYTE,
                     in->data[0]);
        glActiveTexture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (gs->debug_count < 1) {
            gs->debug_count++;
        }

        if ( // check if render
            (
                gs->pip_duration_ft < 0 
                || (
                    gs->pip_duration_ft > 0 
                    && gs->playTime_ft <= gs->pip_duration_ft
                )
            )
            && gs->playTime_ft >= gs->r_start_time_ft)
        {
            ext_get_frame(inlink);
            render_rgba_frame(inlink);
        } 
        glReadPixels(0, 0, outlink->w, outlink->h, gs->pix_fmt, GL_UNSIGNED_BYTE, (GLvoid *) out->data[0]);
    } else {
        av_frame_copy(out, in);
        av_log(ctx, AV_LOG_DEBUG,
           "doing vf_pipglshader filter_frame copy pts:%ld ,time->%f, duration:%f\n",
           in->pts, gs->playTime_ft, gs->duration_ft);
    }

    av_frame_free(&in);
    return ff_filter_frame(outlink, out);
} // filter_frame

static av_cold void uninit(AVFilterContext *ctx) {
    av_log(ctx, AV_LOG_DEBUG, "start vf_pipglshader uninit action\n");

    PipGLShaderContext *c = ctx->priv;

    // av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader ff_framesync_uninit\n");
    // ff_framesync_uninit(&c->frameSync); // @new

    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader check window\n");
    if (c->window) { // @new
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader gl delete operations\n");
        glDeleteTextures(1, &c->frame_tex);
        glDeleteTextures(1, &c->ext_frame_tex);
        glDeleteBuffers(1, &c->pos_buf);
        glDeleteProgram(c->program);
        glfwDestroyWindow(c->window);
    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader no window, do not need delete operations\n");
    }
    /*
    av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader check f_shader_source\n");
    if (c->f_shader_source) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader av freep shader source\n");
        av_freep(&c->f_shader_source);
    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_pipglshader no shader source, do not need av freep shader source\n");
    }
    */
    av_log(ctx, AV_LOG_DEBUG, "finished vf_pipglshader\n");

/*
  glDeleteTextures(1, &c->frame_tex);
  glDeleteBuffers(1, &c->pos_buf);
  glDeleteProgram(c->program);
  //glDeleteBuffers(1, &c->pos_buf);
  glfwDestroyWindow(c->window);
*/
} // uninit


static int query_formats(AVFilterContext *ctx) {
    static const enum AVPixelFormat formats[] = {
        AV_PIX_FMT_RGB24, AV_PIX_FMT_RGBA, AV_PIX_FMT_NONE
    };
    return ff_set_common_formats(ctx, ff_make_format_list(formats));
} // query_formats

static const AVFilterPad pipglshader_inputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO,
        .config_props = config_props,
        .filter_frame = filter_frame
    },
    //{
    //    .name = "external",
    //    .type = AVMEDIA_TYPE_VIDEO,
    //    .config_props = config_props_external,
    //},
    {
        NULL
    }
}; // pipglshader_inputs

static const AVFilterPad pipglshader_outputs[] = {
        {
            .name = "default",
            .type = AVMEDIA_TYPE_VIDEO
            //.config_props = config_output
        },
        //{
        //    .name = "external",
        //    .type = AVMEDIA_TYPE_VIDEO
        //},
        {
            NULL
        }
}; // pipglshader_outputs

AVFilter ff_vf_pipglshader = {
    .name          = "pipglshader",
    .description   = NULL_IF_CONFIG_SMALL("Render Frame by GL shader with LUT"),
    .priv_size     = sizeof(PipGLShaderContext),
    .init          = init,
    .uninit        = uninit,
    .query_formats = query_formats,
    //.activate      = activate,
    .inputs        = pipglshader_inputs,
    .outputs       = pipglshader_outputs,
    .priv_class    = &pipglshader_class,
    .flags         = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC
}; // ff_vf_pipglshader
