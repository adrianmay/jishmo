/*
 * Copyright (c) 2012 Stefano Sabatini
 * Copyright (c) 2014 Clément Bœsch
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

 *Modified by: Jishnu Jaykumar Padalunkal and Adrian May

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <libavutil/motion_vector.h>
#include <libavformat/avformat.h>

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL;
static AVStream *video_stream = NULL;
static const char *src_filename = NULL;

static int video_stream_idx = -1;
static AVFrame *frame = NULL;
static int video_frame_count = 0;

static int print_motion_vectors_data(AVMotionVector *mv, int video_frame_count){
  printf("| #:%d | p/f:%2d | %2d x %2d | src:(%4d,%4d) | dst:(%4d,%4d) | dx:%4d | dy:%4d | motion_x:%4d | motion_y:%4d | motion_scale:%4d | 0x%"PRIx64" |\n",
      video_frame_count,
      mv->source,
      mv->w,
      mv->h,
      mv->src_x,
      mv->src_y,
      mv->dst_x,
      mv->dst_y,
      mv->dst_x - mv->src_x,
      mv->dst_y - mv->src_y,
      mv->motion_x,
      mv->motion_y,
      mv->motion_scale,
      mv->flags);
  printf("---------------------------------------------------------------------------------------------------------------------------------------------\n");
  return 0;
}

static int print_frame_data(AVFrame * frame){
  printf("%s\n", frame->data[0]);
  return 0;
}

static int decode_packet(const AVPacket *pkt)
{
    int ret = avcodec_send_packet(video_dec_ctx, pkt);
    char szFileName[255] = {0};
    FILE *file=NULL;
    if (ret < 0) {
        fprintf(stderr, "Error while sending a packet to the decoder: %s\n", av_err2str(ret));
        return ret;
    }

    while (ret >= 0)  {
        ret = avcodec_receive_frame(video_dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            fprintf(stderr, "Error while receiving a frame from the decoder: %s\n", av_err2str(ret));
            return ret;
        }

        if (ret >= 0) {
            int i;
            AVFrameSideData *sd;
	    video_frame_count++;
            sprintf(szFileName, "./output/mv/%d.json", video_frame_count);
            file = fopen(szFileName,"w");
            if (file == NULL)
            {
                fprintf(stderr, "Couldn't open file for reading\n");
                exit(1);
            }
            fprintf(file, "[\n");
            sd = av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
            if (sd) {
                const AVMotionVector *mvs = (const AVMotionVector *)sd->data;
                for (i = 0; i < sd->size / sizeof(*mvs); i++) {
                    const AVMotionVector *mv = &mvs[i];
                    // print_motion_vectors_data(mv, video_frame_count);
                    if(i==(sd->size / sizeof(*mvs))-1){
                      fprintf(file, "\t{\n");
                      fprintf(file, "\t\t\"source\" : %d,\n", mv->source);
                      fprintf(file, "\t\t\"width\" : %d,\n", mv->w);
                      fprintf(file, "\t\t\"height\" : %d,\n", mv->h);
                      if(mv->source<0){
                        fprintf(file, "\t\t\"src_x\" : %d,\n", ((mv->src_x)/abs(mv->source)));
                        fprintf(file, "\t\t\"src_y\" : %d,\n", (mv->src_y)/abs(mv->source));
                        fprintf(file, "\t\t\"dst_x\" : %d,\n", (mv->dst_x/abs(mv->source)));
                        fprintf(file, "\t\t\"dst_y\" : %d,\n", (mv->dst_y/abs(mv->source)));
                        fprintf(file, "\t\t\"dx\" : %d,\n", ((mv->dst_x - mv->src_x)/abs(mv->source)));
                        fprintf(file, "\t\t\"dy\" : %d\n", ((mv->dst_y - mv->src_y)/abs(mv->source)));
                      }else{
                        fprintf(file, "\t\t\"src_x\" : %d,\n", (mv->dst_x/abs(mv->source)));
                        fprintf(file, "\t\t\"src_y\" : %d,\n", (mv->dst_y/abs(mv->source)));
                        fprintf(file, "\t\t\"dst_x\" : %d,\n", (mv->src_x/abs(mv->source)));
                        fprintf(file, "\t\t\"dst_y\" : %d,\n", (mv->src_y/abs(mv->source)));
                        fprintf(file, "\t\t\"dx\" : %d,\n", ((mv->src_x - mv->dst_x)/abs(mv->source)));
                        fprintf(file, "\t\t\"dy\" : %d\n", ((mv->src_y - mv->dst_y)/abs(mv->source)));
                      }
                      fprintf(file, "\t}\n");
                    }else{
                      fprintf(file, "\t{\n");
                      fprintf(file, "\t\t\"source\" : %d,\n", mv->source);
                      fprintf(file, "\t\t\"width\" : %d,\n", mv->w);
                      fprintf(file, "\t\t\"height\" : %d,\n", mv->h);
                      if(mv->source<0){
                        fprintf(file, "\t\t\"src_x\" : %d,\n", (mv->src_x/abs(mv->source)));
                        fprintf(file, "\t\t\"src_y\" : %d,\n", (mv->src_y/abs(mv->source)));
                        fprintf(file, "\t\t\"dst_x\" : %d,\n", (mv->dst_x/abs(mv->source)));
                        fprintf(file, "\t\t\"dst_y\" : %d,\n", (mv->dst_y/abs(mv->source)));
                        fprintf(file, "\t\t\"dx\" : %d,\n", ((mv->dst_x - mv->src_x)/abs(mv->source)));
                        fprintf(file, "\t\t\"dy\" : %d\n", ((mv->dst_y - mv->src_y)/abs(mv->source)));
                      }else{
                        fprintf(file, "\t\t\"src_x\" : %d,\n", (mv->dst_x/abs(mv->source)));
                        fprintf(file, "\t\t\"src_y\" : %d,\n", (mv->dst_y/abs(mv->source)));
                        fprintf(file, "\t\t\"dst_x\" : %d,\n", (mv->src_x/abs(mv->source)));
                        fprintf(file, "\t\t\"dst_y\" : %d,\n", (mv->src_y/abs(mv->source)));
                        fprintf(file, "\t\t\"dx\" : %d,\n", ((mv->src_x - mv->dst_x)/abs(mv->source)));
                        fprintf(file, "\t\t\"dy\" : %d\n", ((mv->src_y - mv->dst_y)/abs(mv->source)));
                      }
                      fprintf(file, "\t},\n");
                    }
                }
                // printf("%s\n", strcat( "./output/", strcat(video_frame_count, ".json")));
            }
            fprintf(file, "]\n");
            fclose(file);
	    printf("\rTotal Processed Frames:%d", video_frame_count);
	    fflush(stdout);
            //Print frame data
            // print_frame_data(frame);
            av_frame_unref(frame);
        }
    }
    return 0;
}

static int open_codec_context(AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret;
    AVStream *st;
    AVCodecContext *dec_ctx = NULL;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    } else {
        int stream_idx = ret;
        st = fmt_ctx->streams[stream_idx];

        dec_ctx = avcodec_alloc_context3(dec);
        if (!dec_ctx) {
            fprintf(stderr, "Failed to allocate codec\n");
            return AVERROR(EINVAL);
        }

        ret = avcodec_parameters_to_context(dec_ctx, st->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters to codec context\n");
            return ret;
        }

        /* Init the video decoder */
        av_dict_set(&opts, "flags2", "+export_mvs", 0);
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }

        video_stream_idx = stream_idx;
        video_stream = fmt_ctx->streams[video_stream_idx];
        video_dec_ctx = dec_ctx;
    }

    return 0;
}

int extract_motion_vectors(char *videopath){
    int ret = 0;
    AVPacket pkt = { 0 };
    struct stat sb;
    // char command[50];
    // char output[13]="./output/mv/";
    // if (!(stat(output, &sb) == 0 && S_ISDIR(sb.st_mode)))
    // {
    //     sprintf(command,"mkdir %s", output);
	  //     system(command);
    // }

    /*if (argc != 2) {
        fprintf(stderr, "Usage: %s <video>\n", argv[0]);
        exit(1);
    }*/

    src_filename = videopath;

    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    open_codec_context(fmt_ctx, AVMEDIA_TYPE_VIDEO);

    av_dump_format(fmt_ctx, 0, src_filename, 0);

    if (!video_stream) {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    printf("\n");
    printf("**************************************************************************************\n");
    printf("*       Tool : MV-Tractus                                                            *\n");
    printf("*     Author : Jishnu Jaykumar Padalunkal (https://jishnujayakumar.github.io)        *\n");
    printf("*  Used Libs : FFmpeg                                                                *\n");
    printf("*Description : A simple tool to extract motion vectors from MPEG videos              *\n");
    printf("**************************************************************************************\n");
    printf("\n");
    printf("--------------------------------------------------------------------------------------\n");
    printf("framenum,source,blockw,blockh,srcx,srcy,dstx,dsty,motion_x,motion_y,motion_scale,flags\n");
    printf("--------------------------------------------------------------------------------------\n");

    /* read frames from the file */
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_idx)
            ret = decode_packet(&pkt);
        av_packet_unref(&pkt);
        if (ret < 0)
            break;
    }

    /* flush cached frames */
    decode_packet(NULL);
    printf("\n--------------------------------------------------------------------------------------\n");

end:
    avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    return ret < 0;
}

int main(int argc, char **argv)
{
	return extract_motion_vectors(argv[1]);
}

