#include "my_ffmpeg.h"
#include "My_LOG.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include "libavutil/log.h"
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>


/**
 * 将相机的yuv数据放进来编码成MP4（h264格式）
 */
const char *outPath;
int width , height;
AVFormatContext *pOFC;
AVOutputFormat *oft;
AVStream *video_st;
int framecnt = 0;
AVCodec *pCodec;
AVFrame *pFrame;
int pic_size;
uint8_t *picture_buf;
AVPacket *pkt;
int y_size;
jbyte *y , *u , *v;

int init(const char *ouputPath , int w , int h){
    av_register_all();
    av_log_set_callback(custom_log);
    int ret = 0;
    outPath = ouputPath;
    LOGE(" INIT ...%s width = %d , height = %d" , outPath  , w , h );
    width = w;
    height = h;
    oft = av_guess_format(NULL, ouputPath, NULL);
    pOFC = avformat_alloc_context();
    if (pOFC == NULL) {
        LOGE(" POFG FAILD ");
        return -1;
    }
    if (oft == NULL) {
        LOGE(" guess fmt faild ");
        return -1;
    }
    LOGE(" FORMAT NAME %s ", oft->name);
    pOFC->oformat = oft;
    ret = avio_open(&pOFC->pb, outPath, AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        LOGE(" avio_open faild! ");
        return -1;
    }
    video_st = avformat_new_stream(pOFC, 0);

    if (video_st == NULL) {
        LOGE(" video_st FAILD !");
        return -1;
    }
    video_st->codec->codec_id = pOFC->oformat->video_codec;
    video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    video_st->codec->pix_fmt = AV_PIX_FMT_YUV420P;
    video_st->codec->width = width;
    video_st->codec->height = height;
    video_st->codec->bit_rate = 400000;
    video_st->codec->gop_size = 250;
    video_st->codec->time_base.num = 1;
    video_st->codec->time_base.den = 25;
    video_st->codec->qmin = 10;
    video_st->codec->qmax = 51;

    pCodec = avcodec_find_encoder(video_st->codec->codec_id);

    if (pCodec == NULL) {
        LOGE("  pCodec null ");
        return -1;
    }

    if (avcodec_open2(video_st->codec, pCodec, NULL) < 0) {
        LOGE("Failed to open encoder! \n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pic_size = avpicture_get_size(video_st->codec->pix_fmt, video_st->codec->width,
                                      video_st->codec->height);

    LOGE(" pic_size %d ", pic_size);
    picture_buf = (uint8_t *) av_malloc(pic_size);
    avpicture_fill((AVPicture *) pFrame, picture_buf, video_st->codec->pix_fmt,
                   video_st->codec->width, video_st->codec->height);
    avformat_write_header(pOFC, NULL);
    pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_new_packet(pkt, pic_size);
    y_size = video_st->codec->width * video_st->codec->height;
//    y = malloc(y_size);
//    u = malloc(y_size / 4);
//    v = malloc(y_size / 4);
    LOGE(" INIT SUCCESS ...");
    return ret;
}



int encodeCamera(jbyte *navtiveYuv){
    LOGE(" ENCODE..");
    if(navtiveYuv == NULL){
        return -1;
    }
//    memcpy(y ,navtiveYuv , y_size );
//    memcpy(u ,navtiveYuv , y_size );

    pFrame->data[0] = navtiveYuv;
    pFrame->data[1] = navtiveYuv + y_size;
    pFrame->data[2] = navtiveYuv + y_size * 5 / 4;


    pFrame->pts = framecnt * (video_st->time_base.den) / ((video_st->time_base.num) * 25);

    int got_picture = 0;

    int ret = avcodec_encode_video2(video_st->codec, pkt, pFrame, &got_picture);
    if (ret < 0) {
        LOGE(" FAILD ENCODE ");
        return -1;
    }

    if (got_picture == 1) {
        LOGE(" ENCODE success %d", framecnt);
//        framecnt++;
        pkt->stream_index = video_st->index;
        ret = av_write_frame(pOFC, pkt);
        av_free_packet(pkt);
    }
//    LOGE(" encodeCamera ...%d " , sizeof(navtiveYuv));
    framecnt++ ;
    return 0;
}

int close(){
    LOGE(" CLOSE ...");
    av_write_trailer(pOFC);

    //Clean
    if (video_st) {
        avcodec_close(video_st->codec);
        av_free(pFrame);
        av_free(picture_buf);
    }
    avio_close(pOFC->pb);
    avformat_free_context(pOFC);

    if(y != NULL){
        free(y);
    }
    if(u != NULL){
        free(u);
    }
    if(v != NULL){
        free(v);
    }
    return 0;
}
