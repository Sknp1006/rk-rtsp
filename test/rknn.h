#pragma once
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <string.h>

#define _BASETSD_H
#include "RgaUtils.h"

#include "postprocess.h"

#include "rknn_api.h"
#include "preprocess.h"

#define PERF_WITH_POST 1
/*-------------------------------------------
                  Functions
-------------------------------------------*/

static void dump_tensor_attr(rknn_tensor_attr *attr)
{
    std::string shape_str = attr->n_dims < 1 ? "" : std::to_string(attr->dims[0]);
    for (int i = 1; i < attr->n_dims; ++i)
    {
        shape_str += ", " + std::to_string(attr->dims[i]);
    }

    printf("  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, w_stride = %d, size_with_stride=%d, fmt=%s, "
           "type=%s, qnt_type=%s, "
           "zp=%d, scale=%f\n",
           attr->index, attr->name, attr->n_dims, shape_str.c_str(), attr->n_elems, attr->size, attr->w_stride,
           attr->size_with_stride, get_format_string(attr->fmt), get_type_string(attr->type),
           get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

double __get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

static unsigned char *load_data(FILE *fp, size_t ofst, size_t sz)
{
    unsigned char *data;
    int ret;

    data = NULL;

    if (NULL == fp)
    {
        return NULL;
    }

    ret = fseek(fp, ofst, SEEK_SET);
    if (ret != 0)
    {
        printf("blob seek failure.\n");
        return NULL;
    }

    data = (unsigned char *)malloc(sz);
    if (data == NULL)
    {
        printf("buffer malloc failure.\n");
        return NULL;
    }
    ret = fread(data, 1, sz, fp);
    return data;
}

static unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp;
    unsigned char *data;

    fp = fopen(filename, "rb");
    if (NULL == fp)
    {
        printf("Open file %s failed.\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);

    data = load_data(fp, 0, size);

    fclose(fp);

    *model_size = size;
    return data;
}

static int saveFloat(const char *file_name, float *output, int element_size)
{
    FILE *fp;
    fp = fopen(file_name, "w");
    for (int i = 0; i < element_size; i++)
    {
        fprintf(fp, "%.6f\n", output[i]);
    }
    fclose(fp);
    return 0;
}

class RKNNHandle
{
public:
    RKNNHandle(const char *model_name)
    {
        /* Create the neural network */
        printf("Loading mode...\n");
        int model_data_size = 0;
        model_data = load_model(model_name, &model_data_size);
        ret = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
        if (ret < 0)
        {
            printf("rknn_init error ret=%d\n", ret);
            return;
        }

        rknn_sdk_version version;
        ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
        if (ret < 0)
        {
            printf("rknn_init error ret=%d\n", ret);
            return;
        }
        printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);

        ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
        if (ret < 0)
        {
            printf("rknn_init error ret=%d\n", ret);
            return;
        }
        printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

        // 初始化 input_attrs 和 output_attrs
        input_attrs = new rknn_tensor_attr[io_num.n_input];
        memset(input_attrs, 0, sizeof(input_attrs));
        for (int i = 0; i < io_num.n_input; i++)
        {
            input_attrs[i].index = i;
            ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
            if (ret < 0)
            {
                printf("rknn_init error ret=%d\n", ret);
                return;
            }
            dump_tensor_attr(&(input_attrs[i]));
        }

        output_attrs = new rknn_tensor_attr[io_num.n_output];
        memset(output_attrs, 0, sizeof(output_attrs));
        for (int i = 0; i < io_num.n_output; i++)
        {
            output_attrs[i].index = i;
            ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
            dump_tensor_attr(&(output_attrs[i]));
        }

        if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
        {
            printf("model is NCHW input fmt\n");
            channel = input_attrs[0].dims[1];
            height = input_attrs[0].dims[2];
            width = input_attrs[0].dims[3];
        }
        else
        {
            printf("model is NHWC input fmt\n");
            height = input_attrs[0].dims[1];
            width = input_attrs[0].dims[2];
            channel = input_attrs[0].dims[3];
        }

        printf("model input height=%d, width=%d, channel=%d\n", height, width, channel);

        inputs = new rknn_input[1];
        memset(inputs, 0, sizeof(inputs));
        inputs[0].index = 0;
        inputs[0].type = RKNN_TENSOR_UINT8;
        inputs[0].size = width * height * channel;
        inputs[0].fmt = RKNN_TENSOR_NHWC;
        inputs[0].pass_through = 0;

        // init rga context
        memset(&src, 0, sizeof(src));
        memset(&dst, 0, sizeof(dst));
    }

    ~RKNNHandle()
    {
        deinitPostProcess();

        // release
        ret = rknn_destroy(ctx);

        if (model_data)
        {
            free(model_data);
        }
    }

    int infer(cv::Mat &orig_img)
    {
        int img_width = 0;
        int img_height = 0;
        int img_channel = 0;

        rknn_output outputs[io_num.n_output];
        memset(outputs, 0, sizeof(outputs));
        for (int i = 0; i < io_num.n_output; i++)
        {
            outputs[i].index = i;
            outputs[i].want_float = 0;
        }

        cv::Mat img;
        cv::cvtColor(orig_img, img, cv::COLOR_BGR2RGB);
        img_width = img.cols;
        img_height = img.rows;
        printf("img width = %d, img height = %d\n", img_width, img_height);

        // 指定目标大小和预处理方式,默认使用LetterBox的预处理
        BOX_RECT pads;
        memset(&pads, 0, sizeof(BOX_RECT));
        cv::Size target_size(width, height);
        cv::Mat resized_img(target_size.height, target_size.width, CV_8UC3);
        // 计算缩放比例
        float scale_w = (float)target_size.width / img.cols;
        float scale_h = (float)target_size.height / img.rows;

        if (img_width != width || img_height != height)
        {
            // 直接缩放采用RGA加速
            if (option == "resize")
            {
                printf("resize image by rga\n");
                ret = resize_rga(src, dst, img, resized_img, target_size);
                if (ret != 0)
                {
                    fprintf(stderr, "resize with rga error\n");
                    return -1;
                }
                // // 保存预处理图片
                // cv::imwrite("resize_input.jpg", resized_img);
            }
            else if (option == "letterbox")
            {
                printf("resize image with letterbox\n");
                float min_scale = std::min(scale_w, scale_h);
                scale_w = min_scale;
                scale_h = min_scale;
                letterbox(img, resized_img, pads, min_scale, target_size);
                // // 保存预处理图片
                // cv::imwrite("letterbox_input.jpg", resized_img);
            }
            else
            {
                fprintf(stderr, "Invalid resize option. Use 'resize' or 'letterbox'.\n");
                return -1;
            }
            inputs[0].buf = resized_img.data;
        }
        else
        {
            inputs[0].buf = img.data;
        }

        gettimeofday(&start_time, NULL);
        rknn_inputs_set(ctx, io_num.n_input, inputs);
        ret = rknn_run(ctx, NULL);
        ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
        gettimeofday(&stop_time, NULL);
        printf("once run use %f ms\n", (__get_us(stop_time) - __get_us(start_time)) / 1000);

        // 后处理
        detect_result_group_t detect_result_group;
        std::vector<float> out_scales;
        std::vector<int32_t> out_zps;
        for (int i = 0; i < io_num.n_output; ++i)
        {
            out_scales.push_back(output_attrs[i].scale);
            out_zps.push_back(output_attrs[i].zp);
        }
        post_process((int8_t *)outputs[0].buf, (int8_t *)outputs[1].buf, (int8_t *)outputs[2].buf, height, width,
                     box_conf_threshold, nms_threshold, pads, scale_w, scale_h, out_zps, out_scales, &detect_result_group);

        // 画框和概率
        char text[256];
        for (int i = 0; i < detect_result_group.count; i++)
        {
            detect_result_t *det_result = &(detect_result_group.results[i]);
            sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);
            printf("%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left, det_result->box.top,
                   det_result->box.right, det_result->box.bottom, det_result->prop);
            int x1 = det_result->box.left;
            int y1 = det_result->box.top;
            int x2 = det_result->box.right;
            int y2 = det_result->box.bottom;
            rectangle(orig_img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(256, 0, 0, 256), 3);
            putText(orig_img, text, cv::Point(x1, y1 + 12), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255));
        }

        ret = rknn_outputs_release(ctx, io_num.n_output, outputs);
        return 0;
        return 0;
    }

private:
    unsigned char *model_data;
    rknn_context ctx;
    rknn_input_output_num io_num;
    rknn_tensor_attr *input_attrs;
    rknn_tensor_attr *output_attrs;
    rknn_input *inputs;

    int channel = 3;
    int width = 0;
    int height = 0;

    const float nms_threshold = NMS_THRESH;      // 默认的NMS阈值
    const float box_conf_threshold = BOX_THRESH; // 默认的置信度阈值

    struct timeval start_time, stop_time;

    int ret;

    rga_buffer_t src;
    rga_buffer_t dst;

    std::string option = "letterbox";
};
