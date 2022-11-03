/**
 * Filter for FFMpeg: Render video by your own shader files
 * @Author  Mail:porschegt23@foxmail.com
 *          QQ: 531365872
 *          Wechat: numberwolf11
 *          Discord: numberwolf#8694
 *          Github: https://github.com/numberwolf
 */
#include "libavutil/opt.h"
#include "internal.h"


#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#define TS2T(ts, tb) ((ts) == AV_NOPTS_VALUE ? NAN : (double)(ts)*av_q2d(tb))

#ifndef OPENGL_RENDER_GRAPH_GL_SHADER_FORAMT
#define OPENGL_RENDER_GRAPH_GL_SHADER_FORAMT
#define	NUMBERWOLF_STRINGIZE(x)	#x
#define	NUMBERWOLF_GL_SHADER(shader) "" NUMBERWOLF_STRINGIZE(shader)
#endif //OPENGL_RENDER_GRAPH_GL_SHADER_FORAMT

static const float position[12] = {
  -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

static const GLchar *v_shader_source = (const GLchar *) NUMBERWOLF_GL_SHADER (
    attribute vec2 position;
    uniform float playTime;
    varying vec2 texCoord;
    float PI = 3.1415926;
    void main(void) {
        float duration = 1.0;
        float progress = mod(playTime, duration) / duration; // 0~1
        float moveX = 0.0;
        float moveY = 0.0;
        float rotate = 360.0 * progress * 0.0;
        float radians = rotate * PI / 180.0;
        float s = sin(radians);
        float c = cos(radians);
        mat4 zRotation = mat4(
                c,      -s,     0.0,    0.0, // 1:scale-x 4:y axis - turn anticlockwise
                s,      c,      0.0,    0.0, // 2:scale-y 4:x axis - turn clockwise , alse for uniform scale
                0.0,    0.0,    1.0,    0.0,
                0.0,    0.0,    0.0,    1.0 // 1:mv right 2:mv up
        );
        gl_Position = zRotation * vec4(position, 0, 1);
        texCoord = position;
    }
);

static const GLchar *f_shader_source = (const GLchar *) NUMBERWOLF_GL_SHADER (
    uniform sampler2D tex;
    uniform float playTime;
    varying vec2 texCoord;
    void main() {
        gl_FragColor = texture2D(tex, texCoord * 0.5 + 0.5);
        //gl_FragColor = texture2D(tex, texCoord);
        //gl_FragColor.a = 0.5;
        //float gray = (gl_FragColor.r + gl_FragColor.g + gl_FragColor.b) / 3.0;
        //gl_FragColor.r = gray;
        //gl_FragColor.g = gray;
        //gl_FragColor.b = gray;
    }
);


typedef struct {
    const AVClass *class;
    //FFFrameSync frameSync;
    double          startPlayTime;
    AVRational      vTimebase;

    //判断通道值
    int alpha;
    int pix_fmt;

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

    // GL
    // input shader vertex
    GLchar          *sdsource_data;
    GLchar          *vxsource_data;
    GLint           playTime;
    // GL Obj
    GLuint          program;
    GLuint          frame_tex;
    GLFWwindow      *window;
    GLuint          pos_buf;
} PlusGLShaderContext;

/**
 * @brief cli params
 */
#define OFFSET(x) offsetof(PlusGLShaderContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM
static const AVOption plusglshader_options[] = {
    {"sdsource", "gl fragment shader source path (default is render gray color)", OFFSET(sdsource), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS}, 
    {"vxsource", "gl vertex shader source path (default is render gray color)", OFFSET(vxsource), AV_OPT_TYPE_STRING, {.str = NULL}, CHAR_MIN, CHAR_MAX, FLAGS},
    {"start", "gl render start timestamp, if you set this option, must greater than zero(no trim)", OFFSET(r_start_time), AV_OPT_TYPE_DURATION, {.i64 = 0.}, 0, INT64_MAX, FLAGS},
    {"duration", "gl render duration, if you set this option, must greater than zero(no trim)", OFFSET(duration), AV_OPT_TYPE_DURATION, {.i64 = 0.}, 0, INT64_MAX, FLAGS},
    {NULL}
};

AVFILTER_DEFINE_CLASS(plusglshader);

static const enum AVPixelFormat alpha_pix_fmts[] = {
    AV_PIX_FMT_ARGB, AV_PIX_FMT_ABGR, AV_PIX_FMT_RGBA,
    AV_PIX_FMT_BGRA, AV_PIX_FMT_NONE
};

static GLuint build_shader(AVFilterContext *ctx, const GLchar *shader_source, GLenum type) {
    GLuint shader = glCreateShader(type);
    if (!shader || !glIsShader(shader)) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_plusglshader build_shader glCreateShader glIsShader FAILED!\n");
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
        av_log(ctx, AV_LOG_ERROR, "doing vf_plusglshader build_shader ERROR: %s\n", &ShaderErrorMessage);
    }

    GLuint ret = status == GL_TRUE ? shader : 0;
    return ret;
}

static void vbo_setup(PlusGLShaderContext *gs) {
    glGenBuffers(1, &gs->pos_buf);
    glBindBuffer(GL_ARRAY_BUFFER, gs->pos_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

    GLint loc = glGetAttribLocation(gs->program, "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

static void tex_setup(AVFilterLink *inlink) {
    AVFilterContext     *ctx = inlink->dst;
    PlusGLShaderContext *gs = ctx->priv;

    glGenTextures(1, &gs->frame_tex);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, gs->frame_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, gs->pix_fmt, inlink->w, inlink->h, 0, gs->pix_fmt, GL_UNSIGNED_BYTE, NULL);

    glUniform1i(glGetUniformLocation(gs->program, "tex"), 0);
}

/**
 * @brief Build shader vertex
 */
static int build_program(AVFilterContext *ctx) {
    av_log(ctx, AV_LOG_DEBUG, "start vf_plusglshader build_program action\n");
    GLuint v_shader, f_shader;
    PlusGLShaderContext *gs = ctx->priv;

    // gl function codes
    // init
    gs->sdsource_data = NULL;
    gs->vxsource_data = NULL;

    /*
     * fragments shader
     */
    if (gs->sdsource) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader build_program shader params: %s\n", gs->sdsource);
        FILE *f = fopen(gs->sdsource, "rb");
        if (!f) {
            av_log(ctx, AV_LOG_ERROR, 
                "doing vf_plusglshader build_program shader: invalid shader source file \"%s\"\n", gs->sdsource);
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
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader build_program no shader param, use default option\n");
        // ...
    }

    /*
     * vertex shader
     */
    if (gs->vxsource) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader build_program vertex params: %s\n", gs->vxsource);
        FILE *f = fopen(gs->vxsource, "rb");
        if (!f) {
            av_log(ctx, AV_LOG_ERROR, 
                "doing vf_plusglshader build_program shader: invalid shader source file \"%s\"\n", gs->vxsource);
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
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader build_program no vertex param, use default option\n");
        // ...
    }

    // dst shader source
    const char *gl_sdsource_dst = gs->sdsource_data ? gs->sdsource_data : f_shader_source;
    const char *gl_vxsource_dst = gs->vxsource_data ? gs->vxsource_data : v_shader_source;

    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_plusglshader build_program build_shader debug shaders ===================================>\n");
    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_plusglshader build_program build_shader use fragment shaders:\n%s\n", gl_sdsource_dst);
    av_log(ctx, AV_LOG_DEBUG, 
        "doing vf_plusglshader build_program build_shader use vertex shader:\n%s\n", gl_vxsource_dst);

    if (gs->r_start_time > 0) {
        //gs->duration_tb = TS2T(gs->duration, gs->vTimebase);
        gs->r_start_time_tb = av_rescale_q(gs->r_start_time, AV_TIME_BASE_Q, gs->vTimebase);
        gs->r_start_time_ft = TS2T(gs->r_start_time_tb, gs->vTimebase);
        gs->duration += gs->r_start_time;
    } else {
        gs->duration_tb = 0;
        gs->duration_ft = 0;
    }
    av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader r_start_time:%ld, r_start_time_tb:%ld, r_start_time_ft:%f\n",
           gs->r_start_time, gs->r_start_time_tb, gs->r_start_time_ft);

    if (gs->duration > 0) {
        //gs->duration_tb = TS2T(gs->duration, gs->vTimebase);
        gs->duration_tb = av_rescale_q(gs->duration, AV_TIME_BASE_Q, gs->vTimebase);
        gs->duration_ft = TS2T(gs->duration_tb, gs->vTimebase);
    } else {
        gs->duration_tb = -1;
        gs->duration_ft = -1;
    }
    av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader duration:%ld, duration_tb:%ld, duration_ft:%f\n",
           gs->duration, gs->duration_tb, gs->duration_ft);

    av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader build_program build_shader\n");
    /*
    if (!((v_shader = build_shader(ctx, v_shader_source, GL_VERTEX_SHADER)) &&
        (f_shader = build_shader(ctx, f_shader_source, GL_FRAGMENT_SHADER)))) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_plusglshader build_program failed!\n");
        return -1;
    }
    */

    if (!((v_shader = build_shader(ctx, gl_vxsource_dst, GL_VERTEX_SHADER)) &&
        (f_shader = build_shader(ctx, gl_sdsource_dst, GL_FRAGMENT_SHADER)))) {
        av_log(ctx, AV_LOG_ERROR, "doing vf_plusglshader build_program failed!\n");
        return -1;
    }
    // build shader finished

    // render shader object
    av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader build_program create program\n");
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

    av_log(ctx, AV_LOG_DEBUG, "finished vf_plusglshader build_program!\n");
    return status == GL_TRUE ? 0 : -1;
}

/*
static int activate(AVFilterContext *ctx) {
    PlusGLShaderContext *c = ctx->priv;
    return ff_framesync_activate(&c->frameSync);
}
*/


/**
 * @brief setup uniform values
 * playTime
 */
static void uni_setup(AVFilterLink *inLink) {
    AVFilterContext         *ctx = inLink->dst;
    PlusGLShaderContext    *c = ctx->priv;
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
    AVFilterContext     *ctx    = inlink->dst;
    PlusGLShaderContext *gs    = ctx->priv;

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

    glViewport(0, 0, inlink->w, inlink->h);
    gs->vTimebase = inlink->time_base;

    int ret;
    if((ret = build_program(ctx)) < 0) {
        return ret;
    }

    glUseProgram(gs->program);

    //glEnable(GL_ALPHA);
    //glEnable(GL_BLEND);
    // //glEnable(GL_DEPTH_TEST);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vbo_setup(gs);
    tex_setup(inlink);
    uni_setup(inlink);
    return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *in) {
    AVFilterContext *ctx    = inlink->dst;
    // AVFilterLink *inlink    = ctx->inputs[0];
    AVFilterLink *outlink   = ctx->outputs[0];
    PlusGLShaderContext *gs = ctx->priv;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // @TODO trans to RGB/RGBA
    // ...

    double playTime = TS2T(in->pts, gs->vTimebase);
    // check start time
    if (gs->startPlayTime < 0) {
        gs->startPlayTime = playTime;
    }
    playTime -= gs->startPlayTime;
    av_log(ctx, AV_LOG_DEBUG,
           "start vf_plusglshader filter_frame get pts:%ld ,time->%f, duration:%f\n", in->pts, playTime, gs->duration_ft);

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
               "doing vf_plusglshader filter_frame gl render pts:%ld ,time->%f, duration:%f\n", in->pts, playTime, gs->duration_ft);

        glUniform1f(gs->playTime, playTime - gs->r_start_time_ft);

        glTexImage2D(GL_TEXTURE_2D, 0, gs->pix_fmt, inlink->w, inlink->h, 0, gs->pix_fmt, GL_UNSIGNED_BYTE, in->data[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glReadPixels(0, 0, outlink->w, outlink->h, gs->pix_fmt, GL_UNSIGNED_BYTE, (GLvoid *) out->data[0]);

    } else {
        av_log(ctx, AV_LOG_DEBUG,
               "doing vf_plusglshader filter_frame copy pts:%ld ,time->%f, duration:%f\n", in->pts, playTime, gs->duration_ft);
        av_frame_copy(out, in);
    }


    av_frame_free(&in);
    return ff_filter_frame(outlink, out);
}

static av_cold void uninit(AVFilterContext *ctx) {
    av_log(ctx, AV_LOG_DEBUG, "start vf_plusglshader uninit action\n");

    PlusGLShaderContext *c = ctx->priv;

    // av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader ff_framesync_uninit\n");
    // ff_framesync_uninit(&c->frameSync); // @new

    av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader check window\n");
    if (c->window) { // @new
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader gl delete operations\n");
        glDeleteTextures(1, &c->frame_tex);
        glDeleteBuffers(1, &c->pos_buf);
        glDeleteProgram(c->program);
        glfwDestroyWindow(c->window);
    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader no window, do not need delete operations\n");
    }
    /*
    av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader check f_shader_source\n");
    if (c->f_shader_source) {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader av freep shader source\n");
        av_freep(&c->f_shader_source);
    } else {
        av_log(ctx, AV_LOG_DEBUG, "doing vf_plusglshader no shader source, do not need av freep shader source\n");
    }
    */
    av_log(ctx, AV_LOG_DEBUG, "finished vf_plusglshader\n");

/*
  glDeleteTextures(1, &c->frame_tex);
  glDeleteBuffers(1, &c->pos_buf);
  glDeleteProgram(c->program);
  //glDeleteBuffers(1, &c->pos_buf);
  glfwDestroyWindow(c->window);
*/
}


static int query_formats(AVFilterContext *ctx) {
  static const enum AVPixelFormat pix_fmts[] = {
        AV_PIX_FMT_RGB24,    AV_PIX_FMT_BGR24,
        AV_PIX_FMT_ARGB,     AV_PIX_FMT_ABGR,
        AV_PIX_FMT_RGBA,     AV_PIX_FMT_BGRA,
        AV_PIX_FMT_NONE
    };
    AVFilterFormats *fmts_list;

    fmts_list = ff_make_format_list(pix_fmts);
    if (!fmts_list) {
      return AVERROR(ENOMEM);
    }
    return ff_set_common_formats(ctx, fmts_list);
}

static const AVFilterPad plusglshader_inputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO,
        .config_props = config_props,
        .filter_frame = filter_frame
    },
    {
        NULL
    }
};

static const AVFilterPad plusglshader_outputs[] = {
  {.name = "default", .type = AVMEDIA_TYPE_VIDEO}, {NULL}};

AVFilter ff_vf_plusglshader = {
    .name          = "plusglshader",
    .description   = NULL_IF_CONFIG_SMALL("Render Frame by GL shader filters' filter"),
    .priv_size     = sizeof(PlusGLShaderContext),
    .init          = init,
    .uninit        = uninit,
    .query_formats = query_formats,
    //.activate      = activate,
    .inputs        = plusglshader_inputs,
    .outputs       = plusglshader_outputs,
    .priv_class    = &plusglshader_class,
    .flags         = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC
};
