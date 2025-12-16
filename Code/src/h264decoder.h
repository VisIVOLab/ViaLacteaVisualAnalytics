#pragma once

#include <QImage>
#include <QByteArray>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// Minimal H.264 decoder based on libavcodec for client-side playback.
class H264Decoder {
public:
    H264Decoder();
    ~H264Decoder();

    bool init();
    // Decode Annex-B NAL buffer into QImage RGBA. Returns true on success.
    bool decode(const QByteArray &nalData, QImage &out);

private:
    void freeSws();

    AVCodecContext *m_ctx{nullptr};
    AVFrame *m_frame{nullptr};
    AVPacket *m_pkt{nullptr};
    SwsContext *m_sws{nullptr};
    int m_swsW{0};
    int m_swsH{0};
};
