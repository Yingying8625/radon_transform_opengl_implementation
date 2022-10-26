

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

#include "tpi_helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;
const unsigned int COLOR_CHANNELS = 4;
GLuint FramebufferRadon = 0;
const char readDirPath[] = "input_pic/";
const char writeDirPath[] = "otput_pic/";

DIR* readDir, * writeDir;
struct dirent* readDirP;
unsigned int textureInput;
unsigned int VBO, VAO, EBO;

void dir_init()
{
    readDir = opendir(readDirPath);
    if (NULL == readDir)
    {
        printf("read Dir path %s can not be opened!\n", readDirPath);
    }

    writeDir = opendir(writeDirPath);
    if (NULL == readDir)
    {
        printf("write Dir path %s can not be opened!\n", writeDirPath);
    }
}

int LoadNextFilefromFolder(DIR* dir, char** filename)
{
    readDirP = readdir(dir);
    if(!readDirP)
    {
        printf("check1\n");
        return 0; //Folder empty
    }

    if ((strcmp(readDirP->d_name, ".") == 0) || (strcmp(readDirP->d_name, "..") == 0))
    {
        *filename = NULL;
        printf("check2 skip\n");
        return 1; //0 special for . .. 
    }

    char* readFileName = NULL;

    readFileName = (char *) malloc(255);
    if (NULL == readFileName)
    {
        printf("can not allocate memory for readFileName!\n");
        return 0;
    }
    memset(readFileName, 0, sizeof(readFileName));

    memcpy(readFileName, readDirPath, strlen(readDirPath));
    memcpy(readFileName + strlen(readDirPath), readDirP->d_name, strlen(readDirP->d_name));
    memcpy(readFileName + strlen(readDirPath) + strlen(readDirP->d_name), "\0", 1);

    *filename = readFileName;
    printf("read file:%s\n", *filename);
    return 1;
}


void gles_init()
{
    // ---------------------------------------------
    // Render to Texture - specific code begins here
    // ---------------------------------------------

    // // 1. generate framebuffer object 
    // // ---------------------------------------------------
    // // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    // glGenFramebuffers(1, &FramebufferRadon);
    // glBindFramebuffer(GL_FRAMEBUFFER, FramebufferRadon);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    // render
    // ------
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    

    // 6. define how to map the input picture to the inputTexture
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions             // texture coords
         1.0f,  1.0f, 0.0f,      1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,      1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,      0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,      0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    

    // create a texture for mapping the input picture to it
    // ----------------------------------------------------------
    glGenTextures(1, &textureInput);
    glBindTexture(GL_TEXTURE_2D, textureInput); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

int main()
{
    printf("Hello World!\n");
    time_t c_start,  c_end;
    c_start = clock();
    int width, height, nrChannels;
    char* file = NULL;
    unsigned char* inputData = NULL;

    dir_init();
    egl_init();
    gles_init();
    
    GLuint program = gles_api_create_program_with_file("texture.vs.hlsl","radon.fs.hlsl");

    // define output file and buffer used to store the results.
    char* writeFileName = (char *)malloc(255);
    if (NULL == writeFileName)
    {
        printf("can not allocate memory for writeFileName!\n");
        return -1;
    }
    memset(writeFileName, 0, sizeof(writeFileName));

    unsigned char* outputData = NULL;
    outputData = (unsigned char*)malloc(COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);
    if (NULL == outputData)
    {
        printf("Failed to allocate memory for output buffer\n");
        return -1;
    }
    memset(outputData, 0, COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);

    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

    while (LoadNextFilefromFolder(readDir, &file) > 0)
    {
        if (!file)
        {
            printf(". or ..\n");
            continue;            
        }

        memset(writeFileName, 0, 255);
        memcpy(writeFileName, "out", 3);
        memcpy(writeFileName+3, file+2, strlen(file)-2);
        //printf("file-2:%s\n",writeFileName);


        //6. active texture input
        // bind textures on corresponding texture units
        //glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureInput);
        inputData = stbi_load((const char*)file, &width, &height, &nrChannels, 0);
        //printf("load channels:%d\n",nrChannels);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, inputData);

        glUniform1i(glGetUniformLocation(program, "A"), 0);

        //7. draw the textureinput to framebufferRadon
        // render container
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glFinish();

        //8. read the output texture to a buffer
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, outputData);

        //9. save the buffer data to a jpg picture
        printf("write the result to :%s\n", writeFileName);
        stbi_write_jpg(writeFileName, width, height, 4, outputData, 0);
        
        //eglSwapBuffers(dpy,)
        stbi_image_free(inputData);
        free(file);
        memset(outputData, 0, COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);
    }
    
    printf("check4\n");

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    free(outputData);
    outputData = NULL;

    free(writeFileName);
    writeFileName = NULL;

    c_end = clock();
    printf("The time used for processing all pictures are %f ms \n", difftime(c_end, c_start));

    return 0;
}
