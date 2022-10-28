#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader_s.h"
#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/time.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int winSysInit();
int openGLInit();
int readWriteResourceInit();
int getInputOutputFileName(char** inputFileName, char** outputFileName);
int readWriteResourceRelease();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;
const unsigned int COLOR_CHANNELS = 3;
const char readDirPath[] = "input_pic/";
const char writeDirPath[] = "output_pic/";
GLFWwindow* window = NULL;
unsigned int textureInput;
unsigned int VBO, VAO, EBO;
DIR* readDir, * writeDir;
struct dirent* readDirP;

char* readFileName = NULL;
char* writeFileName = NULL;
int main()
{
    struct timeval startTime,  endTime;
    gettimeofday(&startTime, NULL);

    
    

    int width, height, nrChannels;
    unsigned char* inputData = NULL;

    //5. define output buffer used to load the output texture from GPU
    unsigned char* outputData = NULL;
    outputData = (unsigned char*)malloc(COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);
    if (NULL == outputData)
    {
        std::cout << "Failed to allocate memory for output buffer" << std::endl;
        return -1;
    }
    memset(outputData, 0, COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);
    winSysInit();
    openGLInit();
    readWriteResourceInit();
    // load image, create texture and generate mipmapsN
    
    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("texture.vs.hlsl", "radon.fs.hlsl");
  
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    
    //start to process the input picture directory
    while ((readDirP = readdir(readDir)) != NULL)
    {
        if ((strcmp(readDirP->d_name, ".") == 0) || (strcmp(readDirP->d_name, "..") == 0))
        {
            continue;
        }

        getInputOutputFileName(&readFileName, &writeFileName );
  
        inputData = stbi_load(readFileName, &width, &height, &nrChannels, 0);
        //printf("width=%d, height=%d, nrchannels=%d\r\n", width, height, nrChannels);
 
        if (inputData)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, inputData);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            printf("Failed to load texture\r\n");
        }
 
        glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //6. active texture input
        // bind textures on corresponding texture units
        glBindTexture(GL_TEXTURE_2D, textureInput);
        glUniform1i(glGetUniformLocation(ourShader.ID, "A"), 0);
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glFinish();

        // read the output texture to a buffer
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, outputData);

        // save the buffer data to a jpg picture
        stbi_write_jpg(writeFileName, width, height, nrChannels, outputData, 0);
                
        memset(inputData, 0, COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);
        memset(outputData, 0, COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);
         
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        //glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    free(outputData);
    outputData = NULL;

    stbi_image_free(inputData);
    inputData = NULL;

    readWriteResourceRelease();
    gettimeofday(&endTime, NULL);
 
    printf("The time used for processing all pictures are %f s \n", (endTime.tv_sec-startTime.tv_sec)+(double)(endTime.tv_usec-startTime.tv_usec)/1000000);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int winSysInit()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RadonTransform", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window\r\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return 0;
}

int openGLInit()
{
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\r\n");
        return -1;
    }

    // ---------------------------------------------
    // Render to Texture - specific code begins here
    // ---------------------------------------------

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


    // 7. create a texture for mapping the input picture to it
    // ----------------------------------------------------------
    glGenTextures(1, &textureInput);
    glBindTexture(GL_TEXTURE_2D, textureInput); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return 0;
}

int readWriteResourceInit()
{
    readFileName = (char*)malloc(255);
    if (NULL == readFileName)
    {
        printf("can not allocate memory for readFileName!\n");
        return -1;
    }
    memset(readFileName, 0, sizeof(readFileName));

    writeFileName = (char*)malloc(255);
    if (NULL == writeFileName)
    {
        printf("can not allocate memory for writeFileName!\n");
        return -1;
    }
    memset(writeFileName, 0, sizeof(writeFileName));

    readDir = opendir(readDirPath);
    if (NULL == readDir)
    {
        printf("read Dir path %s can not be opened!\n", readDirPath);
        return -1;
    }

    writeDir = opendir(writeDirPath);
    if (NULL == readDir)
    {
        printf("write Dir path %s can not be opened!\n", writeDirPath);
        return -1;
    }

    return 0;
}

int getInputOutputFileName(char** inputFileName, char** outputFileName)
{
    char* pReplace = NULL;

    memcpy(*inputFileName, readDirPath, strlen(readDirPath));
    memcpy(*inputFileName + strlen(readDirPath), readDirP->d_name, strlen(readDirP->d_name));
    memcpy(*inputFileName + strlen(readDirPath) + strlen(readDirP->d_name), "\0", 1);

    memcpy(*outputFileName, writeDirPath, strlen(writeDirPath));
    memcpy(*outputFileName + strlen(writeDirPath), readDirP->d_name, strlen(readDirP->d_name));
    memcpy(*outputFileName + strlen(writeDirPath) + strlen(readDirP->d_name), "\0", 1);
    pReplace = strstr(*outputFileName, ".");
    memcpy(pReplace, "_out.jpg\0", 9);
    
    //printf("%s \r\n", *inputFileName);
    //printf("%s \r\n", *outputFileName);
    
    return 0;
}

int readWriteResourceRelease()
{
    free(readFileName);
    readFileName = NULL;

    free(writeFileName);
    writeFileName = NULL;
    return 0;
}
