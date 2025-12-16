#include "video_encoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <QtMath>

QByteArray VideoEncoder::encodeH264(const QByteArray &rgba, int width, int height,
                                    const QString &quality, QString &error) {
    error.clear();
    if (width <= 0 || height <= 0 || rgba.size() < width * height * 4) {
        error = QStringLiteral("Invalid frame");
        return {};
    }

    const AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        error = QStringLiteral("H.264 encoder not found");
        return {};
    }

    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        error = QStringLiteral("Cannot alloc codec context");
        return {};
    }

    ctx->width = width;
    ctx->height = height;
    ctx->time_base = {1, 25};
    ctx->framerate = {25, 1};
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ctx->gop_size = 12;
    ctx->max_b_frames = 0;

    // Bitrate / crf
    int crf = (quality.compare("interactive", Qt::CaseInsensitive) == 0) ? 28 : 18;
    int bitrate = (quality.compare("interactive", Qt::CaseInsensitive) == 0) ? 2'000'000 : 8'000'000;

    av_opt_set_int(ctx->priv_data, "crf", crf, 0);
    av_opt_set(ctx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
    av_opt_set(ctx->priv_data, "annexb", "1", 0); // produce Annex-B stream for easier decoding
    ctx->bit_rate = bitrate;

    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        error = QStringLiteral("Cannot open encoder");
        avcodec_free_context(&ctx);
        return {};
    }

    // Prepare frames
    AVFrame *frame = av_frame_alloc();
    frame->format = ctx->pix_fmt;
    frame->width = width;
    frame->height = height;
    if (av_frame_get_buffer(frame, 32) < 0) {
        error = QStringLiteral("Cannot alloc frame buffer");
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        return {};
    }

    // RGBA -> YUV420P
    SwsContext *sws = sws_getContext(width, height, AV_PIX_FMT_RGBA,
                                     width, height, AV_PIX_FMT_YUV420P,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws) {
        error = QStringLiteral("Cannot create sws context");
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        return {};
    }
    const uint8_t *srcSlice[1] = {reinterpret_cast<const uint8_t *>(rgba.constData())};
    int srcStride[1] = {4 * width};
    sws_scale(sws, srcSlice, srcStride, 0, height, frame->data, frame->linesize);

    QByteArray nal;
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        error = QStringLiteral("Cannot alloc packet");
        sws_freeContext(sws);
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        return {};
    }

    if (avcodec_send_frame(ctx, frame) == 0) {
        while (avcodec_receive_packet(ctx, pkt) == 0) {
            nal.append(reinterpret_cast<const char *>(pkt->data), pkt->size);
            av_packet_unref(pkt);
        }
    } else {
        error = QStringLiteral("Encoding failed");
    }

    av_packet_free(&pkt);
    sws_freeContext(sws);
    av_frame_free(&frame);
    avcodec_free_context(&ctx);

    return nal;
}
