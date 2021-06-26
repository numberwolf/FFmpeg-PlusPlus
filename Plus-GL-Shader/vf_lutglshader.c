/**
 * Filter for FFMpeg: Render video by your own shader files
 * @Author  Mail:porschegt23@foxmail.com
 *          QQ: 531365872
 *          Wechat: numberwolf11
 *          Discord: numberwolf#8694
 *          Github: https://github.com/numberwolf
 */
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"

#include "libavutil/opt.h"
#include "internal.h"

#define DEFAULTS (0)
#define EXTERNAL (1)

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#define EXT_TYPE_MEDIA 0
#define EXT_TYPE_RGB24 1
#define TS2T(ts, tb) ((ts) == AV_NOPTS_VALUE ? NAN : (double)(ts)*av_q2d(tb))

static const float position[12] = {
  -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

static const GLchar *v_shader_source =
  "attribute vec2 position;\n"
  "varying vec2 TextureCoordsVarying;\n"
  "const float PI = 3.1415926;\n"
  "uniform float playTime;\n"
  "\n"
  "void main(void) {\n"
  "    gl_Position = vec4(position, 0, 1);\n"
  "    //TextureCoordsVarying = position;\n"
  "\n"
  "    TextureCoordsVarying.x = position.x * 0.5 + 0.5;\n"
  "    TextureCoordsVarying.y = position.y * 0.5 + 0.5;\n"
  "}";

static const GLchar *f_shader_source =
  "uniform sampler2D tex;\n"
  "uniform sampler2D externTex;\n"
  "varying vec2 TextureCoordsVarying;\n"
  "uniform float playTime;\n"
  "const float PI = 3.1415926;\n"
  "\n"
  "float rand(float n) {\n"
  "    return fract(sin(n) * 43758.5453123);\n"
  "}\n"
  "\n"
  "vec4 lookupTable(vec4 color, float progress){\n"
  "    //float blueColor = color.b * 63.0 * progress;\n"
  "    float blueColor = color.b * 63.0;\n"
  "\n"
  "    vec2 quad1;\n"
  "    quad1.y = floor(floor(blueColor) / 8.0);\n"
  "    quad1.x = floor(blueColor) - (quad1.y * 8.0);\n"
  "\n"
  "    vec2 quad2;\n"
  "    quad2.y = floor(ceil(blueColor) / 8.0);\n"
  "    quad2.x = ceil(blueColor) - (quad2.y * 8.0);\n"
  "\n"
  "    vec2 texPos1;\n"
  "    texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);\n"
  "    texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);\n"
  "\n"
  "    vec2 texPos2;\n"
  "    texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);\n"
  "    texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);\n"
  "\n"
  "    vec4 newColor1 = texture2D(externTex, texPos1);\n"
  "    vec4 newColor2 = texture2D(externTex, texPos2);\n"
  "    vec4 newColor = mix(newColor1, newColor2, fract(blueColor));\n"
  "    return vec4(newColor.rgb, color.w);\n"
  "}\n"
  "\n"
  "void main() {\n"
  "    float duration = 0.5;\n"
  "    float progress = mod(playTime, duration) / duration; // 0~1\n"
  "    vec4 imgColor = texture2D(tex, TextureCoordsVarying);\n"
  "    vec4 lutColor = lookupTable(imgColor, progress);\n"
  "    gl_FragColor = mix(imgColor, lutColor, 1.0);\n"
  "}";

#define PIXEL_FORMAT GL_RGB

typedef struct {
    const AVClass *class;
    //FFFrameSync frameSync;
    double          startPlayTime;
    AVRational      vTimebase;

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
    // @param lutrgb
    // @TODO
    char            *ext_source;
    int             ext_type;

    int             debug_count;

    // GL
    unsigned char   *lut_rgb;
    // input shader vertex
    GLchar          *sdsource_data;
    GLchar          *vxsource_data;
    GLint           playTime;
    // GL Obj
    GLuint          program;
    GLuint          frame_tex; // v1
    GLuint          lut_tex; // v2
    GLFWwindow      *window;
    GLuint          pos_buf;
} LutGLShaderContext;

/**
 * @brief cli params
 */
#define OFFSET(x) offsetof(LutGLShaderContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM
static const AVOption lutglshader_options[] = {
    {"sdsource", "gl fragment shader source path (default is render lut fragment)", OFFSET(sdsource), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS},
    {"vxsource", "gl vertex shader source path (default is render lut vertex)", OFFSET(vxsource), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS},
    {"start", "gl render start timestamp, if you set this option, must greater than zero(no trim)", OFFSET(r_start_time), AV_OPT_TYPE_DURATION, {.i64 = 0.}, 0, INT64_MAX, FLAGS},
    {"duration", "gl render duration, if you set this option, must greater than zero(no trim)", OFFSET(duration), AV_OPT_TYPE_DURATION, {.i64 = 0.}, 0, INT64_MAX, FLAGS},
    {"ext", "gl fragment shader externTex's source file (default is null)", OFFSET(ext_source), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS},
    {"ext_type", "ext type(0:media 1:rgb24 default is 0)", OFFSET(ext_type), AV_OPT_TYPE_INT, {.i64=1}, 0, INT_MAX, FLAGS},
    {NULL}
};

AVFILTER_DEFINE_CLASS(lutglshader);

static GLuint build_shader(AVFilterContext *ctx, const GLchar *shader_source, GLenum type) {
    GLuint shader = glCreateShader(type);
    if (!shader || !glIsShader(shader)) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_lutglshader build_shader glCreateShader glIsShader FAILED!\n");
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
        av_log(ctx, AV_LOG_ERROR, "doing vf_lutglshader build_shader ERROR: %s\n", &ShaderErrorMessage);
    }

    GLuint ret = status == GL_TRUE ? shader : 0;
    return ret;
}

static void vbo_setup(LutGLShaderContext *gs) {
    glGenBuffers(1, &gs->pos_buf);
    glBindBuffer(GL_ARRAY_BUFFER, gs->pos_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

    GLint loc = glGetAttribLocation(gs->program, "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

static void tex_setup(AVFilterLink *inlink) {
    AVFilterContext     *ctx= inlink->dst;
    LutGLShaderContext  *gs = ctx->priv;
    gs->debug_count         = 0;

    glGenTextures(1, &gs->frame_tex);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, gs->frame_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, inlink->w, inlink->h, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, NULL);

    glUniform1i(glGetUniformLocation(gs->program, "tex"), 0);
}

static int open_ext_source(AVFilterLink *inlink) {
    AVFilterContext     *ctx    = inlink->dst;
    LutGLShaderContext  *gs     = ctx->priv;
    int                 ret     = 0;

    AVFormatContext         *m_formatCtx        = NULL;
    AVCodecContext          *m_vCodecContext    = NULL;
    AVPacket                *m_decPacket        = NULL;
    AVFrame                 *m_frame            = NULL;
    AVFrame      			*outFrame           = NULL;
    struct SwsContext       *swCtx              = NULL;
    uint8_t                 *out_buffer         = NULL;
    int                     isGetFrame          = 0;

    if (gs->ext_source != NULL) {
        /*
         * begin
         */
        m_formatCtx = avformat_alloc_context();

        if ((ret = avformat_open_input(&m_formatCtx, gs->ext_source, NULL, NULL)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
            goto final_release_av;
        }

        if ((ret = avformat_find_stream_info(m_formatCtx, NULL)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
            goto final_release_av;
        }

        int vIndex = -1;
        for (int i = 0; i < m_formatCtx->nb_streams; i++) {

            AVStream *contex_stream = m_formatCtx->streams[i];
            enum AVCodecID codecId = contex_stream->codecpar->codec_id;

            if (contex_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                vIndex = i;

                double m_vTimebase = av_q2d(contex_stream->time_base);

                m_frame = av_frame_alloc();
                outFrame = av_frame_alloc();
                if (!m_frame || !outFrame) {
                    av_log(NULL, AV_LOG_DEBUG, "av_frame_alloc out frame error\n");
                    break;
                }

                /*
                 * Decoder
                 */
                AVCodec *dec = avcodec_find_decoder(codecId);
                const char* codec_name = avcodec_get_name(codecId);

                if (!dec) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to find decoder video for stream #%u codec:%s\n", i, codec_name);
                    ret = AVERROR_DECODER_NOT_FOUND;
                    goto final_release_av;
                }

                m_vCodecContext = avcodec_alloc_context3(dec);

                if (!m_vCodecContext) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to allocate the video decoder context for stream #%u\n", i);
                    ret = AVERROR(ENOMEM);
                    goto final_release_av;
                }

                ret = avcodec_parameters_to_context(m_vCodecContext, contex_stream->codecpar);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to copy video decoder parameters to input decoder context "
                           "for stream #%u\n", i);
                    goto final_release_av;
                }

                //const char *codec_name = avcodec_get_name(sniffStreamContext->m_vCodecContext->codec_id);
                av_log(NULL, AV_LOG_DEBUG, "video codec name:%s\n", codec_name);
                ret = avcodec_open2(m_vCodecContext, dec, NULL);
                if (avcodec_open2(m_vCodecContext, dec, NULL) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "avcodec_open2 初始化解码器失败\n");
                    goto final_release_av;
                }

                m_decPacket = av_packet_alloc();
                av_init_packet(m_decPacket);
                if (!m_decPacket) {
                    av_log(NULL, AV_LOG_ERROR, "av_frame_alloc-packet 初始化解码器失败\n");
                    ret = -1;
                    goto final_release_av;
                }

                while(av_read_frame(m_formatCtx, m_decPacket) >= 0) {

                    char szError[256] = {0};
                    ret = avcodec_send_packet(m_vCodecContext, m_decPacket);

                    if (ret == AVERROR(EAGAIN)) {
                        av_log(NULL, AV_LOG_ERROR, "sendRet ===========> EAGAIN\n");
                        break;
                    } else if (ret == AVERROR_EOF) {
                        av_log(NULL, AV_LOG_ERROR, "sendRet ===========> AVERROR_EOF\n");
                        break;
                    } else if (ret == AVERROR(EINVAL)) {
                        av_log(NULL, AV_LOG_ERROR, "sendRet ===========> EINVAL\n");
                        break;
                    } else if (ret == AVERROR(ENOMEM)) {
                        av_log(NULL, AV_LOG_ERROR, "sendRet ===========> ENOMEM\n");
                        break;
                    } else {
                    }


                    if (ret == 0) {
                        int rec_re = 0;
                        while (1) {
                            rec_re = avcodec_receive_frame(m_vCodecContext, m_frame);
                            if (rec_re == 0) {
                                av_log(NULL, AV_LOG_DEBUG, "debug open img after decode %d x %d\n",
                                       m_frame->width, m_frame->height);
                                isGetFrame = 1;

                                if (NULL == swCtx) {
                                    swCtx = sws_getContext(
                                            m_frame->width, m_frame->height,
                                            m_vCodecContext->pix_fmt, // in ,vcodec->frame->format
                                            m_frame->width, m_frame->height,
                                            AV_PIX_FMT_RGB24, // out
                                            SWS_FAST_BILINEAR, NULL, NULL, NULL);
                                }

                                outFrame->width     = m_frame->width;
                                outFrame->height    = m_frame->height;
                                outFrame->format    = AV_PIX_FMT_RGB24;

                                int rgb24size = outFrame->width * outFrame->height * 3;

                                out_buffer = (uint8_t *)av_malloc((int)(rgb24size) * sizeof(uint8_t));
                                avpicture_fill(
                                        (AVPicture *)outFrame, out_buffer,
                                        AV_PIX_FMT_RGB24,
                                        m_frame->width, m_frame->height);

                                sws_scale(swCtx,
                                          (const uint8_t* const*)m_frame->data,
                                          m_frame->linesize, // (const uint8_t* const*)
                                          0, m_frame->height,
                                          outFrame->data,
                                          outFrame->linesize);

                                av_log(NULL, AV_LOG_DEBUG, "debug open img after prepare %d x %d\n",
                                       outFrame->width, outFrame->height);

                                //FILE *fw = fopen("./test4_out.rgb", "wb");
                                //fwrite(outFrame->data[0], 1, rgb24size, fw);
                                //fclose(fw);

                                gs->lut_rgb = malloc(rgb24size + 1);
                                memcpy(gs->lut_rgb, outFrame->data[0], rgb24size);
                                gs->lut_rgb[rgb24size] = 0;

                                /*
                                 * free
                                 */
                                av_free(out_buffer);
                                out_buffer = NULL;

                                break;
                            }
                        }
                    }

                    if (ret < 0 || isGetFrame > 0) {
                        break;
                    }
                }

                break;
            }
        } // end for stream

    }  // end if ext_source

    final_release_av:
    {
        if (m_frame != NULL) {
            av_frame_free(&m_frame);
            m_frame = NULL;
        }

        if (outFrame != NULL) {
            av_frame_free(&outFrame);
            outFrame = NULL;
        }

        if (m_decPacket != NULL) {
            av_packet_unref(m_decPacket);
            m_decPacket = NULL;
        }

        if (NULL != m_vCodecContext) {
            avcodec_close(m_vCodecContext);
            m_vCodecContext = NULL;
        }

        if (NULL != swCtx) {
            sws_freeContext(swCtx);
            swCtx = NULL;
        }

        if (NULL != m_formatCtx) {
            avformat_close_input(&m_formatCtx);
            m_formatCtx = NULL;
        }
    };

    return ret;
}

static int tex_setup_lut(AVFilterLink *inlink) {

    AVFilterContext     *ctx    = inlink->dst;
    LutGLShaderContext  *gs     = ctx->priv;

    /*
     * media image
     */
    if (gs->ext_type <= EXT_TYPE_MEDIA) { // default media

        int ret = open_ext_source(inlink); // @TODO
        if (ret < 0) {
            av_log(ctx, AV_LOG_ERROR,
                   "doing vf_lutglshader tex_setup_lut: invalid lut path:%s open_ext_source ,error ret %d\n",
                   gs->ext_source, ret);
            return ret;
        }

    } else { // rgb
        /*
         * rgb24 image
         */

        FILE *f = fopen(gs->ext_source, "rb");
        if (!f) {
            av_log(ctx, AV_LOG_ERROR,
                   "doing vf_lutglshader tex_setup_lut: invalid lut path \"%s\"\n", gs->ext_source);
            return -1;
        }

        // get file size
        fseek(f, 0, SEEK_END);
        unsigned long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        gs->lut_rgb = malloc(fsize + 1);
        fread(gs->lut_rgb, fsize, 1, f);
        fclose(f);
        gs->lut_rgb[fsize] = 0;


        //FILE *fw = fopen("./test.rgb", "wb");
        //fwrite(gs->lut_rgb, 1, fsize, fw);
        //fclose(fw);
    }

    glGenTextures(1, &gs->lut_tex);
    glActiveTexture(GL_TEXTURE0 + 1);

    /*glBindTexture(GL_TEXTURE_3D,  gs->lut_tex);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, 32, 32, 32, 0, GL_RGB,
    //             GL_UNSIGNED_BYTE, gs->lut_rgb);*/

    glBindTexture(GL_TEXTURE_2D,  gs->lut_tex);
/*
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB,
                 //GL_UNSIGNED_BYTE, gs->lut_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, NULL);

    glUniform1i(glGetUniformLocation(gs->program, "externTex"), 1);

    return 0;
}

/**
 * @brief Build shader vertex
 */
static int build_program(AVFilterContext *ctx) {
    av_log(ctx, AV_LOG_DEBUG, "start vf_lutglshader build_program action\n");
    GLuint v_shader, f_shader;
    LutGLShaderContext *gs = ctx->priv;

    // gl function codes
    // init
    gs->sdsource_data = NULL;
    gs->vxsource_data = NULL;

    /*
     * fragments shader
     */
    if (gs->sdsource) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader build_program shader params: %s\n", gs->sdsource);
        FILE *f = fopen(gs->sdsource, "rb");
        if (!f) {
            av_log(ctx, AV_LOG_ERROR, 
                "doing vf_lutglshader build_program shader: invalid shader source file \"%s\"\n", gs->sdsource);
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
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader build_program no shader param, use default option\n");
        // ...
    }

    /*
     * vertex shader
     */
    if (gs->vxsource) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader build_program vertex params: %s\n", gs->vxsource);
        FILE *f = fopen(gs->vxsource, "rb");
        if (!f) {
            av_log(ctx, AV_LOG_ERROR, 
                "doing vf_lutglshader build_program shader: invalid shader source file \"%s\"\n", gs->vxsource);
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
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader build_program no vertex param, use default option\n");
        // ...
    }

    // dst shader source
    const char *gl_sdsource_dst = gs->sdsource_data ? gs->sdsource_data : f_shader_source;
    const char *gl_vxsource_dst = gs->vxsource_data ? gs->vxsource_data : v_shader_source;

    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_lutglshader build_program build_shader debug shaders ===================================>\n");
    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_lutglshader build_program build_shader use fragment shaders:\n%s\n", gl_sdsource_dst);
    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_lutglshader build_program build_shader use vertex shader:\n%s\n", gl_vxsource_dst);

    if (gs->r_start_time > 0) {
        //gs->duration_tb = TS2T(gs->duration, gs->vTimebase);
        gs->r_start_time_tb = av_rescale_q(gs->r_start_time, AV_TIME_BASE_Q, gs->vTimebase);
        gs->r_start_time_ft = TS2T(gs->r_start_time_tb, gs->vTimebase);
        gs->duration += gs->r_start_time;
    } else {
        gs->duration_tb = 0;
        gs->duration_ft = 0;
    }
    av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader r_start_time:%ld, r_start_time_tb:%ld, r_start_time_ft:%f\n",
           gs->r_start_time, gs->r_start_time_tb, gs->r_start_time_ft);

    if (gs->duration > 0) {
        //gs->duration_tb = TS2T(gs->duration, gs->vTimebase);
        gs->duration_tb = av_rescale_q(gs->duration, AV_TIME_BASE_Q, gs->vTimebase);
        gs->duration_ft = TS2T(gs->duration_tb, gs->vTimebase);
    } else {
        gs->duration_tb = -1;
        gs->duration_ft = -1;
    }
    av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader duration:%ld, duration_tb:%ld, duration_ft:%f\n",
           gs->duration, gs->duration_tb, gs->duration_ft);

    av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader build_program build_shader\n");
    /*
    if (!((v_shader = build_shader(ctx, v_shader_source, GL_VERTEX_SHADER)) &&
        (f_shader = build_shader(ctx, f_shader_source, GL_FRAGMENT_SHADER)))) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_lutglshader build_program failed!\n");
        return -1;
    }
    */

    if (!((v_shader = build_shader(ctx, gl_vxsource_dst, GL_VERTEX_SHADER)) &&
        (f_shader = build_shader(ctx, gl_sdsource_dst, GL_FRAGMENT_SHADER)))) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_lutglshader build_program failed!\n");
        return -1;
    }
    // build shader finished

    // render shader object
    av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader build_program create program\n");
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

    av_log(ctx, AV_LOG_DEBUG, "finished vf_lutglshader build_program!\n");
    return status == GL_TRUE ? 0 : -1;
}

/*
static int activate(AVFilterContext *ctx) {
    LutGLShaderContext *c = ctx->priv;
    return ff_framesync_activate(&c->frameSync);
}
*/


/**
 * @brief setup uniform values
 * playTime
 */
static void uni_setup(AVFilterLink *inLink) {
    AVFilterContext         *ctx = inLink->dst;
    LutGLShaderContext    *c = ctx->priv;
    c->playTime = glGetUniformLocation(c->program, "playTime");
    glUniform1f(c->playTime, 0.0f);
}

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
}


static int config_props(AVFilterLink *inlink) {
    AVFilterContext    *ctx     = inlink->dst;
    LutGLShaderContext *gs      = ctx->priv;

    gs->startPlayTime           = -1;

    glfwWindowHint(GLFW_VISIBLE, 0);
    gs->window = glfwCreateWindow(inlink->w, inlink->h, "", NULL, NULL);

    glfwMakeContextCurrent(gs->window);

    #ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
    #endif

    glViewport(0, 0, inlink->w, inlink->h);
    gs->vTimebase = inlink->time_base;

    int ret;
    if((ret = build_program(ctx)) < 0) {
        return ret;
    }

    glUseProgram(gs->program);
    vbo_setup(gs);
    tex_setup(inlink);
    int setupLutRet = tex_setup_lut(inlink);
    if (setupLutRet < 0) {
        exit(setupLutRet);
    }
    uni_setup(inlink);
    return 0;
}

static int config_props_external(AVFilterLink *inlink) {
    AVFilterContext    *ctx     = inlink->dst;
    LutGLShaderContext *gs      = ctx->priv;

    av_log(ctx, AV_LOG_DEBUG, "debug vf_lutglshader config props get luts data:%d, %d\n", inlink->w, inlink->h);

    return 0;
}

static int config_output(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->src;
    LutGLShaderContext *s = ctx->priv;
    int ret;

    //if ((ret = ff_framesync_init_dualinput(&s->fs, ctx)) < 0)
    //    return ret;

    outlink->w = ctx->inputs[DEFAULTS]->w;
    outlink->h = ctx->inputs[DEFAULTS]->h;
    outlink->time_base = ctx->inputs[DEFAULTS]->time_base;

    av_log(ctx, AV_LOG_DEBUG, "debug config_output output size:%d, %d\n", outlink->w, outlink->h);

    //return ff_framesync_configure(&s->fs);
    return 0;
}

//static int ext_config_props(AVFilterLink *inlink) {
//    AVFilterContext     *ctx    = inlink->dst;
//    LutGLShaderContext *gs    = ctx->priv;
//
//    gs->startPlayTime           = -1;
//
//    glfwWindowHint(GLFW_VISIBLE, 0);
//    gs->window = glfwCreateWindow(inlink->w, inlink->h, "", NULL, NULL);
//
//    glfwMakeContextCurrent(gs->window);
//
//#ifndef __APPLE__
//    glewExperimental = GL_TRUE;
//    glewInit();
//#endif
//
//    glViewport(0, 0, inlink->w, inlink->h);
//    gs->vTimebase = inlink->time_base;
//
//    int ret;
//    if((ret = build_program(ctx)) < 0) {
//        return ret;
//    }
//
//    glUseProgram(gs->program);
//    vbo_setup(gs);
//    tex_setup(inlink);
//    //tex_setup_lut(inlink, "./lut.rgb");
//    uni_setup(inlink);
//    return 0;
//}

// apply
static int filter_frame(AVFilterLink *inlink, AVFrame *in) {
    AVFilterContext *ctx    = inlink->dst;
    // AVFilterLink *inlink    = ctx->inputs[0];
    AVFilterLink *outlink   = ctx->outputs[0];
    LutGLShaderContext *gs = ctx->priv;

    double playTime = TS2T(in->pts, gs->vTimebase);
    // check start time
    if (gs->startPlayTime < 0) {
        gs->startPlayTime = playTime;
    }
    playTime -= gs->startPlayTime;
    av_log(ctx, AV_LOG_DEBUG,
           "start vf_lutglshader filter_frame get pts:%ld ,time->%f, duration:%f\n", in->pts, playTime, gs->duration_ft);

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
    glUseProgram(gs->program);

    if ( // check if render
            (gs->duration_ft < 0 || (gs->duration_ft > 0 && playTime <= gs->duration_ft))
            && playTime >= gs->r_start_time_ft)
    {
        av_log(ctx, AV_LOG_DEBUG,
               "doing vf_lutglshader filter_frame gl render pts:%ld ,time->%f, duration:%f\n", in->pts, playTime, gs->duration_ft);

        glUniform1f(gs->playTime, playTime);

        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gs->frame_tex);
            //glPixelStorei(GL_UNPACK_ROW_LENGTH, in->linesize[0] / 3);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, inlink->w, inlink->h, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE,
                         in->data[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);


            if (gs->debug_count < 1) {
                gs->debug_count++;

                av_log(ctx, AV_LOG_DEBUG,
                       "doing vf_lutglshader filter_frame gl write debug linesize:%d and wh:%d and wh2:%d \n",
                       in->linesize[0], inlink->w * inlink->h * 3, in->width * in->height * 3);

                //FILE *fw = fopen("./test2.rgb", "wb");
                //fwrite(in->data[0], 1, in->width * in->height * 3, fw);
                //fclose(fw);
                //FILE *fw2 = fopen("./test3.rgb", "wb");
                //fwrite(gs->lut_rgb, 1, 512 * 512 * 3, fw2);
                //fclose(fw2);
            }
        }

        {
            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, gs->lut_tex);
            // lut
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, gs->lut_rgb);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glReadPixels(0, 0, outlink->w, outlink->h, PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid *) out->data[0]);

    } else {
        av_log(ctx, AV_LOG_DEBUG,
               "doing vf_lutglshader filter_frame copy pts:%ld ,time->%f, duration:%f\n", in->pts, playTime, gs->duration_ft);
        av_frame_copy(out, in);
    }


    av_frame_free(&in);
    return ff_filter_frame(outlink, out);
}

static av_cold void uninit(AVFilterContext *ctx) {
    av_log(ctx, AV_LOG_DEBUG, "start vf_lutglshader uninit action\n");

    LutGLShaderContext *c = ctx->priv;

    // av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader ff_framesync_uninit\n");
    // ff_framesync_uninit(&c->frameSync); // @new

    av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader check window\n");
    if (c->window) { // @new
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader gl delete operations\n");
        glDeleteTextures(1, &c->frame_tex);
        glDeleteBuffers(1, &c->pos_buf);
        glDeleteProgram(c->program);
        glfwDestroyWindow(c->window);
    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader no window, do not need delete operations\n");
    }
    /*
    av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader check f_shader_source\n");
    if (c->f_shader_source) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader av freep shader source\n");
        av_freep(&c->f_shader_source);
    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_lutglshader no shader source, do not need av freep shader source\n");
    }
    */
    av_log(ctx, AV_LOG_DEBUG, "finished vf_lutglshader\n");

/*
  glDeleteTextures(1, &c->frame_tex);
  glDeleteBuffers(1, &c->pos_buf);
  glDeleteProgram(c->program);
  //glDeleteBuffers(1, &c->pos_buf);
  glfwDestroyWindow(c->window);
*/
}


static int query_formats(AVFilterContext *ctx) {
  static const enum AVPixelFormat formats[] = {AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE};
  return ff_set_common_formats(ctx, ff_make_format_list(formats));
}

static const AVFilterPad lutglshader_inputs[] = {
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
};

static const AVFilterPad lutglshader_outputs[] = {
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
};

AVFilter ff_vf_lutglshader = {
    .name          = "lutglshader",
    .description   = NULL_IF_CONFIG_SMALL("Render Frame by GL shader with LUT"),
    .priv_size     = sizeof(LutGLShaderContext),
    .init          = init,
    .uninit        = uninit,
    .query_formats = query_formats,
    //.activate      = activate,
    .inputs        = lutglshader_inputs,
    .outputs       = lutglshader_outputs,
    .priv_class    = &lutglshader_class,
    .flags         = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC
};
