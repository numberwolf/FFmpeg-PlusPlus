/*
 * Copyright (c) 2013 Stefano Sabatini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * libavformat/libavcodec demuxing and muxing API example.
 *
 * Remux streams from one container format to another.
 * @example remuxing.c
 */

#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#include <libavformat/avio.h>
#include <libavutil/opt.h>

struct buffer_data
{
    uint8_t *buf;
    size_t size;
    uint8_t *ptr;
    size_t room; ///< size left in the buffer
};

static int iowrite_to_buffer(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    while (buf_size > bd->room)
    {
        int64_t offset = bd->ptr - bd->buf;
        bd->buf = av_realloc_f(bd->buf, 2, bd->size);
        if (!bd->buf)
            return AVERROR(ENOMEM);
        bd->size = bd->size*2;
        bd->ptr = bd->buf + offset;
        bd->room = bd->size - offset;
    }
    /* copy buffer data to buffer_data buffer */
    memcpy(bd->ptr, buf, buf_size);
    bd->ptr += buf_size;
    bd->room -= buf_size;
    printf("write packet pkt_size:%d used_buf_size:%zu buf_size:%zu buf_room:%zu\n", buf_size, bd->ptr - bd->buf, bd->size, bd->room);
    return buf_size;
}

static int64_t seek_buffer(void *ptr, int64_t pos, int whence)
{
    struct buffer_data *bd = (struct buffer_data *)ptr;
    int64_t ret = -1;

    switch (whence)
    {
    case AVSEEK_SIZE:
        ret = bd->size;
        break;
    case SEEK_SET:
        bd->ptr = bd->buf + pos;
        // bd->room = bd->size - pos;
        ret = bd->ptr;
        break;
    }
    printf("whence=%d , offset=%ld , buffer_size=%ld, buffer_room=%ld\n", whence, pos, bd->size, bd->room);
    return ret;
}

static int ioread_from_buffer(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->room); //bd->size改为bd->room

    if (!buf_size)
        return AVERROR_EOF;

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    // bd->room -= buf_size; //bd->size -= buf_size
    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);
    return buf_size;
}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

int remux_start(const char *in_filename, const char *out_filename)
{
    av_log_set_level(AV_LOG_DEBUG);

    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;

    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;

    struct buffer_data bd_zm = {0};
    AVIOContext *avio_ctx_zm = NULL;

    uint8_t *avio_ctx_buffer_zm = NULL;
    size_t avio_ctx_buffer_size_zm = 4096;
    const size_t bd_zm_buf_size = 1024;

    const AVOutputFormat *output_format = av_guess_format("mp4", NULL, NULL);

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    avformat_alloc_output_context2(&ofmt_ctx, output_format, NULL, NULL);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    bd_zm.ptr = bd_zm.buf = av_malloc(bd_zm_buf_size);
    if (!bd_zm.buf) {
        ret = AVERROR(ENOMEM);
        return ret;
    }
    bd_zm.size = bd_zm.room = bd_zm_buf_size;
    
    avio_ctx_buffer_zm = av_malloc(avio_ctx_buffer_size_zm);
    if (!avio_ctx_buffer_zm)
    {
        av_log(NULL, AV_LOG_ERROR, "allocate buffer error\n");
        return AVERROR(ENOMEM);
    }
    avio_ctx_zm = avio_alloc_context(avio_ctx_buffer_zm, avio_ctx_buffer_size_zm,
                                     1, &bd_zm, NULL, iowrite_to_buffer, seek_buffer);
    if (!avio_ctx_zm)
    {
        av_log(NULL, AV_LOG_ERROR, "allocate avio context error\n");
        return AVERROR(ENOMEM);
    }
    ofmt_ctx->pb = avio_ctx_zm;
    if (!ofmt_ctx->pb) {
        av_log(NULL, AV_LOG_ERROR, "output format context pb is empty\n");
        return AVERROR(ENOMEM);
    }
    // ofmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ofmt_ctx->oformat = output_format;
    // end AVIO init

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        stream_mapping[i] = stream_index++;

        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    av_dump_format(ofmt_ctx, 0, NULL, 1);

    /*if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            goto end;
        }
    }*/

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }

    while (1) {
        AVStream *in_stream, *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index >= stream_mapping_size ||
            stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        log_packet(ofmt_ctx, &pkt, "out");

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    av_write_trailer(ofmt_ctx);

    //zhimo starts:  write buffer to file
    FILE *pFile = fopen(out_filename, "wb");

    if (pFile) {
        fwrite(bd_zm.buf, bd_zm.size - bd_zm.room, 1, pFile);  //跟ffmpeg命令行比, 最后多个8bit的空值
        puts("Wrote to file!");
    } else {
        puts("Something wrong writing to File.");
    }
    fclose(pFile);
    //zhimo ends

end:

    printf("start END avformat_close_input\n");
    avformat_close_input(&ifmt_ctx);

    /* close output */
    /*if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE)) {
        printf("start END avio_closep\n");
        avio_closep(&ofmt_ctx->pb);
    }*/
    printf("start END avformat_free_context\n");
    avformat_free_context(ofmt_ctx);

    printf("start END av_freep\n");
    av_freep(&stream_mapping);

    av_freep(&avio_ctx_zm->buffer);
    av_free(avio_ctx_zm);
    av_free(bd_zm.buf);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s input output\n"
               "API example program to remux a media file with libavformat and libavcodec.\n"
               "The output format is guessed according to the file extension.\n"
               "\n", argv[0]);
        return 1;
    }

    const char *in_filename  = argv[1];
    const char *out_filename  = argv[2];

    while(1) {
        remux_start(in_filename, out_filename);
    }

    return 0;
}
