#include "h264decoder.h"

#include <vector>

H264Decoder::H264Decoder() = default;

H264Decoder::~H264Decoder() {
    if (m_ctx) {
        avcodec_free_context(&m_ctx);
        m_ctx = nullptr;
    }
    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }
    if (m_pkt) {
        av_packet_free(&m_pkt);
        m_pkt = nullptr;
    }
    freeSws();
}

bool H264Decoder::init() {
    if (m_ctx) return true;
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) return false;
    m_ctx = avcodec_alloc_context3(codec);
    if (!m_ctx) return false;
    m_ctx->thread_count = 0; // auto
    m_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    if (avcodec_open2(m_ctx, codec, nullptr) < 0) return false;
    m_frame = av_frame_alloc();
    m_pkt = av_packet_alloc();
    return m_frame && m_pkt;
}

void H264Decoder::freeSws() {
    if (m_sws) {
        sws_freeContext(m_sws);
        m_sws = nullptr;
    }
    m_swsW = m_swsH = 0;
}

bool H264Decoder::decode(const QByteArray &nalData, QImage &out) {
    if (!m_ctx && !init()) return false;
    av_packet_unref(m_pkt);
    m_pkt->data = reinterpret_cast<uint8_t *>(const_cast<char *>(nalData.constData()));
    m_pkt->size = nalData.size();
    if (avcodec_send_packet(m_ctx, m_pkt) < 0) return false;
    int ret = avcodec_receive_frame(m_ctx, m_frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return false;
    if (ret < 0) return false;

    const int w = m_frame->width;
    const int h = m_frame->height;
    if (w <= 0 || h <= 0) return false;

    if (!m_sws || m_swsW != w || m_swsH != h) {
        freeSws();
        m_sws = sws_getCachedContext(nullptr,
                                     w, h, static_cast<AVPixelFormat>(m_frame->format),
                                     w, h, AV_PIX_FMT_RGBA,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);
        m_swsW = w; m_swsH = h;
    }
    if (!m_sws) return false;

    std::vector<uint8_t> rgba(static_cast<size_t>(w * h * 4));
    uint8_t *dstData[4] = {rgba.data(), nullptr, nullptr, nullptr};
    int dstLinesize[4] = {w * 4, 0, 0, 0};

    sws_scale(m_sws,
              m_frame->data, m_frame->linesize,
              0, h,
              dstData, dstLinesize);

    out = QImage(rgba.data(), w, h, QImage::Format_RGBA8888).copy();
    return true;
}
