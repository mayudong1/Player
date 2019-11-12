
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
static float degreeX = 0.0f;
static float degreeY = 0.0f;

#define ES_PI  (3.14159265f)

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
    glfwSetKeyCallback(window, key_callback);

    Shader ourShader("shader.vs", "shader.fs");


    float radius = 10;
    int numSlices = 128;
    int numParallels = numSlices / 2;
    int numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
    int numIndices = numParallels * numSlices * 6;
    float angleStep = (2.0f * ES_PI) / ((float) numSlices);
    
    float* vertices = (float*)malloc ( sizeof(float) * 3 * numVertices );
    float* texCoords = (float*)malloc ( sizeof(float) * 2 * numVertices );
    short* indices = (short*)malloc ( sizeof(short) * numIndices );

    for (int i = 0; i < numParallels + 1; i++ ) {
        for (int j = 0; j < numSlices + 1; j++ ) {
            int vertex = ( i * (numSlices + 1) + j ) * 3;
            
            if ( vertices ) {
                vertices[vertex + 0] = - radius * sinf ( angleStep * (float)i ) * sinf ( angleStep * (float)j );
                vertices[vertex + 1] = radius * sinf ( ES_PI/2 + angleStep * (float)i );
                vertices[vertex + 2] = radius * sinf ( angleStep * (float)i ) * cosf ( angleStep * (float)j );
            }
            
            if (texCoords) {
                int texIndex = ( i * (numSlices + 1) + j ) * 2;
                texCoords[texIndex + 0] = (1.0f - ((float) i / (float) (numParallels)));
                texCoords[texIndex + 1] = (float) j / (float) numSlices;
            }
        }
    }

    // Generate the indices
    if ( indices != NULL ) {
        short* indexBuf = indices;
        for (int i = 0; i < numParallels ; i++ ) {
            for (int j = 0; j < numSlices; j++ ) {
                *indexBuf++ = (short)(i * ( numSlices + 1 ) + j); // a
                *indexBuf++ = (short)(( i + 1 ) * ( numSlices + 1 ) + j); // b
                *indexBuf++ = (short)(( i + 1 ) * ( numSlices + 1 ) + ( j + 1 )); // c
                *indexBuf++ = (short)(i * ( numSlices + 1 ) + j); // a
                *indexBuf++ = (short)(( i + 1 ) * ( numSlices + 1 ) + ( j + 1 )); // c
                *indexBuf++ = (short)(i * ( numSlices + 1 ) + ( j + 1 )); // d
                
            }
        }
        
    }

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int VBO_texCoords;
    glGenBuffers(1, &VBO_texCoords);
    
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numVertices, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_texCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * numVertices, texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short) * numIndices, indices, GL_STATIC_DRAW);

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
    // int ret = avformat_open_input(&context, "http://192.168.72.27:8080/live/chenjin.flv", 0, 0);
    int ret = avformat_open_input(&context, "http://127.0.0.1:8000/out.mp4", 0, 0);
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


        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.use();

        glm::mat4 model = glm::mat4(1.0f);
        // model = glm::rotate(model, glm::radians(degreeX), glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, glm::radians(degreeY), glm::vec3(1.0f, 0.0f, 0.0f));
        // model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(degreeX), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(degreeY), glm::vec3(1.0f, 0.0f, 0.0f));
        int viewMatrix = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::mat4(1.0f);
        // projection = glm::perspective((double)3.14/1.8, (double)1.0, (double)0.1f, (double)100.0f);
        projection = glm::frustum((double)-0.5, (double)0.5, (double)-0.5, (double)0.5, (double)0.5, (double)100);
        int projMatrix = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projMatrix, 1, GL_FALSE, glm::value_ptr(projection));

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
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();

        av_frame_unref(frame);
        usleep(30*1000);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && (action & (GLFW_PRESS | GLFW_REPEAT)))
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    else if (key == GLFW_KEY_LEFT && (action & (GLFW_PRESS | GLFW_REPEAT)))
        degreeX = degreeX - 1.0;
    else if (key == GLFW_KEY_RIGHT && (action & (GLFW_PRESS | GLFW_REPEAT)))
        degreeX = degreeX + 1.0;
    else if (key == GLFW_KEY_UP && (action & (GLFW_PRESS | GLFW_REPEAT)))
        degreeY = degreeY - 1.0;
    else if (key == GLFW_KEY_DOWN && (action & (GLFW_PRESS | GLFW_REPEAT)))
        degreeY = degreeY + 1.0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}