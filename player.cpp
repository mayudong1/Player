
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <iostream>
#include <unistd.h>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

#include "shader.h"

static int pic_width = 176;
static int pic_height = 144;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader ourShader("shader.vs", "shader.fs");

    float vertices[] = {
        0.5f, 0.5f, 0.0f,   1.0f, 1.0f,// 右上角
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f,// 右下角
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,// 左下角
        -0.5f, 0.5f, 0.0f,   0.0f, 1.0f// 左上角
    }; 
    unsigned int indices[] = {
        0, 1, 3, // 第一个三角形
        1, 2, 3  // 第二个三角形
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture[3];
    glGenTextures(3, texture);
    for(int i=0;i<3;i++)
    {
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    AVFormatContext* context = NULL;
    int ret = avformat_open_input(&context, "http://127.0.0.1:8000/1.mp4", 0, 0);
    if(ret != 0)
    {
        std::cout << "open url failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    ret = avformat_find_stream_info(context, 0);
    if(ret != 0)
    {
        std::cout << "find stream info failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    AVStream* st = context->streams[0];
    AVCodec* dec = avcodec_find_decoder(st->codecpar->codec_id);
    if(!dec)
    {
        std::cout << "can not find codec" << std::endl;
        return -1;
    }
    AVCodecContext* dec_ctx = avcodec_alloc_context3(dec);
    if(!dec_ctx)
    {
        std::cout << "alloc decode context failed" << std::endl;
        return -1;
    }
    ret = avcodec_parameters_to_context(dec_ctx, st->codecpar);
    if(ret < 0)
    {
        std::cout << "avcodec_parameters_to_context failed" << std::endl;
        return -1;
    }
    ret = avcodec_open2(dec_ctx, dec, NULL);
    if(ret < 0)
    {
        std::cout << "open codec failed" << std::endl;
        return -1;
    }
    
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    int count = 0;

    AVFrame* frame = av_frame_alloc();
    int got_frame = 0;

    while (!glfwWindowShouldClose(window))
    {
        ret = av_read_frame(context, &pkt);
        if(ret < 0)
        {
            std::cout << "read frame failed" << std::endl;
            break;
        }
        else
        {
            if(pkt.stream_index != 0)
                continue;

            ret = avcodec_decode_video2(dec_ctx, frame, &got_frame, &pkt);
            if(ret < 0)
            {
                std::cout << "decode failed" << std::endl;
                continue;
            }

        }
        if(!got_frame)
        {
            std::cout << "not decode image" << std::endl;
            continue;
        }

        processInput(window);

        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame->linesize[0], frame->height, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[0]);
        ourShader.setInt("tex_y", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame->linesize[1], frame->height/2, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[1]);
        ourShader.setInt("tex_u", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame->linesize[2], frame->height/2, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[2]);
        ourShader.setInt("tex_v", 2);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();

        av_frame_unref(frame);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}