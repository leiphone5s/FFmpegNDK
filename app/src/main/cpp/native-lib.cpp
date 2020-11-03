#include <jni.h>
#include <string>
#include <android/log.h>
#include <unistd.h>

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

extern "C" JNIEXPORT jstring JNICALL
Java_com_zhidao_ffmpegndkdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


void  SaveFrame(AVFrame *pFrame,  int  width,  int  height, int  index)
{
    FILE  *pFile;
    char  szFilename[32];
    int   y;

    // Open file
    sprintf(szFilename, "%s_%d.jpeg", "/sdcard/", index);
    pFile= fopen (szFilename,  "wb" );
    LOGI("%s",szFilename);

    if (pFile==NULL)
        return ;

    // Write header
    fprintf (pFile, "P6 %d %d 255 ", width, height);

    // Write pixel data
    for (y=0; y<height; y++)
    {
        fwrite (pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
    }

    // Close file
    fclose (pFile);

}


extern "C"
JNIEXPORT void JNICALL
Java_com_zhidao_ffmpegndkdemo_MainActivity_decodeVideo(JNIEnv *env, jobject thiz, jstring path) {
    // TODO: implement decodeVideo()
    const char *str_ = env->GetStringUTFChars(path, NULL);
    /**
     * 第一步
     * 初始化网络组件
     * **/
    avformat_network_init();
    AVDictionary *options = NULL;
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
        return;
    }
    /**
     * 第三步
     * 获取视频信息
     * **/
    ret = avformat_find_stream_info(pAVFContext, NULL);
    if (ret < 0) {
        LOGE("查找视频信息失败");
        return;
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
    ret = avcodec_open2(avCtx, pAVCoder, NULL);
    if (ret < 0) {
        LOGE("打开解码器失败");
        if (avCtx) {
            avcodec_free_context(&avCtx);
        }
        return;
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
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, avCtx->width, avCtx->height, 1);
    uint8_t *out_buffer = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));
    av_image_fill_arrays(rgba_frame->data, rgba_frame->linesize, out_buffer, AV_PIX_FMT_RGB24,
                         avCtx->width, avCtx->height, 1);

    SwsContext *pSwsContext = sws_getContext(avCtx->width, avCtx->height,
                                             avCtx->pix_fmt,
                                             avCtx->width, avCtx->height,
                                             AV_PIX_FMT_RGB24,
                                             SWS_BICUBIC, NULL, NULL, NULL);
    int index= 0;

    LOGI("解码器的名称：%s", "开始读取帧信息");

    // 开始读取帧
    while (av_read_frame(pAVFContext, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            avcodec_send_packet(avCtx, packet);
            ret = avcodec_receive_frame(avCtx, avFrame_in);
            if (ret == AVERROR(EAGAIN)) {
                LOGE("------->>>>>>:%d", ret);
                continue;
            }

            LOGI("解码器的名称：%s,%d", "解码成功，进行类型转码",index);
            /**解码成功**/
            /**进行类型转码,原始数据转为RGB**/
            sws_scale(pSwsContext, avFrame_in->data, avFrame_in->linesize, 0, avCtx->height,
                      rgba_frame->data, rgba_frame->linesize);
//            index++;
            SaveFrame(rgba_frame,avCtx->width,avCtx->height,index++);
            if(index > 50) break;
            usleep(16*1000);

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