#include <jni.h>
#include <string>
#include <android/log.h>
#include <unistd.h>
#include <bits/struct_file.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);


extern "C" {
//工具库
#include "libavutil/avutil.h"
//封装格式处理库
#include "libavformat/avformat.h"
//编码解码库
#include "libavcodec/avcodec.h"
//音频采样数据格式转换库
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"

#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavutil/imgutils.h>
}
#define MAX_AUDIO_FRME_SIZE  2 * 44100
#define __ANDROID_API__ 19



extern "C"
JNIEXPORT jstring JNICALL
Java_com_zhidao_informcollect_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {
    // TODO: implement stringFromJNI()
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


int encodejpg(const char *out_file,AVFrame *picture,AVCodecContext *pVideoCodecCtx) {
    AVFormatContext* pFormatCtx;
    AVOutputFormat* fmt;
    AVStream* video_st;
    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;

    AVPacket pkt;
    int got_picture=0;
    int ret= -1;
    pFormatCtx = avformat_alloc_context();
    //Guess format
    fmt = av_guess_format("mjpeg", NULL, NULL);
    pFormatCtx->oformat = fmt;
    //Output URL
    if (avio_open(&pFormatCtx->pb,out_file, AVIO_FLAG_READ_WRITE) < 0){
        return -1;
    }
    video_st = avformat_new_stream(pFormatCtx, 0);
    if (video_st==NULL){
        return -1;
    }

    pCodecCtx = video_st->codec;
    pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //pCodecCtx->bit_rate = 20000;
    //pCodecCtx->bit_rate_tolerance = 4000000;
    pCodecCtx->compression_level = 10;
    pCodecCtx->width = pVideoCodecCtx->width;
    pCodecCtx->height = pVideoCodecCtx->height;
    //Output some information
    //av_dump_format(pFormatCtx, 0, out_file, 1);

    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec){
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0){
        return -1;
    }

    avformat_write_header(pFormatCtx,NULL);

    int y_size = pCodecCtx->width * pCodecCtx->height;
    av_new_packet(&pkt,y_size*3);//enough size

    ret = avcodec_encode_video2(pCodecCtx, &pkt,picture, &got_picture);
    if(ret < 0){
        return -1;
    }
    if (got_picture==1){
        pkt.stream_index = video_st->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }
    av_free_packet(&pkt);
    //Write Trailer
    av_write_trailer(pFormatCtx);
    if (video_st){
        avcodec_close(video_st->codec);
        av_free(picture);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    return ret;
}





char * SaveFramePPM(AVFrame *pFrame, int  width, int  height, long  index)
{
    FILE  *pFile;
    static char  szFilename[32];
    int   y;

    // Open file
    sprintf(szFilename, "%s_%ld.ppm", "/sdcard/", index);
    pFile= fopen (szFilename,  "wb+" );
    LOGI("%s",szFilename);

    if (pFile==NULL)
        return nullptr;

    // Write header
    fprintf (pFile, "P6 %d %d 255 ", width, height);

    // Write pixel data
    for (y=0; y<height; y++)
    {
        fwrite (pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
    }

    // Close file
    fclose (pFile);
    return szFilename;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_zhidao_informcollect_MainActivity_decodeVideo(JNIEnv *env, jobject thiz, jstring path,
                                                       jdouble time_stamp, jstring save_name) {
    // TODO: implement decodeVideo()
    // TODO: implement decodeVideo()
    const char *str_ = env->GetStringUTFChars(path, nullptr);
    const char *saveName = env->GetStringUTFChars(save_name, nullptr);
    /**
     * 第一步
     * 初始化网络组件
     * **/
    avformat_network_init();
    AVDictionary *options = nullptr;
    av_dict_set(&options, "stimeout", "20000000", 0);
    /**
     * 第二步
     * 打开封装格式->打开文件
     * AVFormatContext：作用保存整个视频信息（解码器，编码器）
     * 信息：码率，帧率等
     * **/
    //获取AVFormat上下文
    AVFormatContext *pAVFContext = avformat_alloc_context();
    /**
     * 指定输入的参宿
     * 设置默认参数 AVDictionary* options = NULL;
     * **/
    int ret = avformat_open_input(&pAVFContext, str_, NULL, NULL);
    if (ret < 0) {
        LOGE("不能打开指定文件");
        if (pAVFContext) {
            //释放上下文
            avformat_free_context(pAVFContext);
        }
        return -1;
    }
    /**
     * 第三步
     * 获取视频信息
     * **/
    ret = avformat_find_stream_info(pAVFContext, NULL);
    if (ret < 0) {
        LOGE("查找视频信息失败");
        if (pAVFContext) {
            avformat_free_context(pAVFContext);
        }
        return -1;
    }
    /**
     * 第四步
     * 查找视频编码器
     * **/
    /**1.查找视频流位置索引**/
    int video_stream_index = -1;
    for (int i = 0; i < pAVFContext->nb_streams; ++i) {
        if (pAVFContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    /**2.根据索引值获取指定解码**/
    AVCodecParameters *pavCoderPar = pAVFContext->streams[video_stream_index]->codecpar;

    /**根据解码器上下文，获得解码器ID，根据ID查找解码器**/
    AVCodec *pAVCoder = avcodec_find_decoder(pavCoderPar->codec_id);
    AVCodecContext *avCtx = avcodec_alloc_context3(pAVCoder);
    avcodec_parameters_to_context(avCtx, pavCoderPar);
    /**
     * 第五步
     * 打开解码器
     * **/
    ret = avcodec_open2(avCtx, pAVCoder, nullptr);
    if (ret < 0) {
        LOGE("打开解码器失败");
        if (avCtx) {
            avcodec_free_context(&avCtx);
        }
        return -1;
    }
    //输出视频信息
    LOGI("视频的文件格式：%s", pAVFContext->iformat->name);
    LOGI("视频时长：%lld", (pAVFContext->duration) / (1000 * 1000));
    LOGI("视频的宽高：%d,%d", avCtx->width, avCtx->height);
    LOGI("解码器的名称：%s", pAVCoder->name);
    /**
     * 第六步
     * 读取视频数据->循环读取
     * **/
    AVPacket *packet = av_packet_alloc();
    AVFrame *avFrame_in = av_frame_alloc();
    AVFrame *rgba_frame = av_frame_alloc();
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, avCtx->width, avCtx->height, 1);
    auto out_buffer = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));
    av_image_fill_arrays(rgba_frame->data, rgba_frame->linesize, out_buffer, AV_PIX_FMT_YUV420P,
                         avCtx->width, avCtx->height, 1);

    SwsContext *pSwsContext = sws_getContext(avCtx->width, avCtx->height,
                                             avCtx->pix_fmt,
                                             avCtx->width, avCtx->height,
                                             AV_PIX_FMT_YUV420P,
                                             SWS_BICUBIC, nullptr, nullptr, nullptr);
    int index= 0;

    LOGI("解码器的名称：%s", "开始读取帧信息");
    //double sec = 1.25;//1.25秒
    //double sec = 1.25;
    int64_t ptsTime = time_stamp / av_q2d(pAVFContext->streams[video_stream_index]->time_base);

    LOGI("*******第一个pts时间为：%ld,",(long)ptsTime);
    //int64_t time = timeStamp* AV_TIME_BASE;
    ret = av_seek_frame(pAVFContext, video_stream_index, ptsTime, AVSEEK_FLAG_BACKWARD);//10(second)
    if (ret<0) {
        if (pAVFContext) {
            avformat_free_context(pAVFContext);
        }
        return -1;
    }
    // 开始读取帧
    while (av_read_frame(pAVFContext, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            int re = avcodec_send_packet(avCtx, packet);
            if (re != 0)
            {
                avcodec_free_context(&avCtx);
                return -1;
            }
            while (avcodec_receive_frame(avCtx, avFrame_in) == 0) {
                if (avFrame_in->pts < ptsTime) {
                    continue;
                }
                LOGI("********第二个pts时间为：%ld", (long) avFrame_in->pts);

                sws_scale(pSwsContext, avFrame_in->data, avFrame_in->linesize, 0, avCtx->height,
                          rgba_frame->data, rgba_frame->linesize);

                LOGI("正在抽帧的图片名称为:%s", saveName);
                rgba_frame->quality = 10;
                rgba_frame->pts = 0;
                LOGI("打印帧数--------------------");
                int code = encodejpg(saveName, rgba_frame, avCtx);

                // 释放 packet 引用
                av_packet_unref(packet);
                sws_freeContext(pSwsContext);
                // 释放 R8
                av_free(out_buffer);

                // 释放 R6
                av_frame_free(&avFrame_in);

                // 释放 R5
                av_packet_free(&packet);

                // 关闭 R3
                avcodec_close(avCtx);

                avcodec_free_context(&avCtx);

                avformat_close_input(&pAVFContext);

                if (pAVFContext) {
                    //释放上下文
                    avformat_free_context(pAVFContext);
                }

                // 释放 R1
                env->ReleaseStringUTFChars(path, str_);
                return code;

            }
        }
        // 释放 packet 引用
        av_packet_unref(packet);
    }
    sws_freeContext(pSwsContext);
    // 释放 R8
    av_free(out_buffer);
    // 释放 R7
    av_frame_free(&rgba_frame);
    // 释放 R6
    av_frame_free(&avFrame_in);
    // 释放 R5
    av_packet_free(&packet);
    // 释放 R4
    // 关闭 R3
    avcodec_close(avCtx);
    // 释放 R2
    avformat_close_input(&pAVFContext);
    // 释放 R1
    env->ReleaseStringUTFChars(path, str_);
}