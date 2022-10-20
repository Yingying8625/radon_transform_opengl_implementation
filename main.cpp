#include <glad/glad.h>
#include <glfw3.h>
#include <shader_s.h>
#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
int getInputOutputFileName(char* inputFileName, char * outputFileName);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;
const unsigned int COLOR_CHANNELS = 3;
const char readDirPath[] = "input_pic\\";
const char writeDirPath[] = "output_pic\\";

int main()
{
    time_t c_start,  c_end;
    c_start = clock();

    
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RadonTransform", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("texture.vs.hlsl", "radon.fs.hlsl");

    // ---------------------------------------------
    // Render to Texture - specific code begins here
    // ---------------------------------------------

    // 1. generate framebuffer object 
    // ---------------------------------------------------
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    GLuint FramebufferRadon = 0;
    glGenFramebuffers(1, &FramebufferRadon);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferRadon);


    // 2. generate texture output 
    // ---------------------------------------------------
    // The texture we're going to render to
    GLuint textureRadon;
    glGenTextures(1, &textureRadon);
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureRadon);
    // Give an empty image to OpenGL ( the last "0" means "empty" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 3. bind texture output with framebuffer 
    // ---------------------------------------------------
    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureRadon, 0);
    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    // Always check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Failed to check framebuffer" << std::endl;
        return -1;
    }

    //4. define parameters used for loading picture from specific path, these parameters will be fulfilled in a loop lataer
    //   for each picture in the path
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

    unsigned int VBO, VAO, EBO;
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
    unsigned int textureInput;
    glGenTextures(1, &textureInput);
    glBindTexture(GL_TEXTURE_2D, textureInput); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmapsN
    
  
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    
    //start to process the input picture directory
    DIR* readDir, * writeDir;
    struct dirent* readDirP;

    char* readFileName = NULL;
    char* writeFileName = NULL;
    char* pReplace = NULL;

    readFileName = (char *) malloc(255);
    if (NULL == readFileName)
    {
        printf("can not allocate memory for readFileName!\n");
        return -1;
    }
    memset(readFileName, 0, sizeof(readFileName));

    writeFileName = (char *)malloc(255);
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
    }

    writeDir = opendir(writeDirPath);
    if (NULL == readDir)
    {
        printf("write Dir path %s can not be opened!\n", writeDirPath);
    }

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

    while ((readDirP = readdir(readDir)) != NULL)
    {
        if ((strcmp(readDirP->d_name, ".") == 0) || (strcmp(readDirP->d_name, "..") == 0))
        {
            continue;
        }
        memcpy(readFileName, readDirPath, strlen(readDirPath));
        memcpy(readFileName + strlen(readDirPath), readDirP->d_name, strlen(readDirP->d_name));
        memcpy(readFileName + strlen(readDirPath) + strlen(readDirP->d_name), "\0", 1);

        memcpy(writeFileName, writeDirPath, strlen(writeDirPath));
        memcpy(writeFileName + strlen(writeDirPath), readDirP->d_name, strlen(readDirP->d_name));
        memcpy(writeFileName + strlen(writeDirPath) + strlen(readDirP->d_name), "\0", 1);
        pReplace = strstr(writeFileName, ".");
        memcpy(pReplace, "_out.jpg\0", 9);
        //printf("%s \r\n", readFileName);
        //printf("%s \r\n", writeFileName);

        inputData = stbi_load(readFileName, &width, &height, &nrChannels, 0);

        /*std::cout << "width=" << width << std::endl;
        std::cout << "height=" << height << std::endl;
        std::cout << "nrChannels=" << nrChannels << std::endl;*/

        if (inputData)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, inputData);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
              
       
        //5. active FramebufferRadon
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferRadon);
        glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //6. active texture input
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureInput);
        glUniform1i(glGetUniformLocation(ourShader.ID, "A"), 0);

        //7. draw the textureinput to framebufferRadon
        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glFinish();

        //8. read the output texture to a buffer
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, outputData);

        //9. save the buffer data to a jpg picture
        stbi_write_jpg(writeFileName, width, height, nrChannels, outputData, 0);
                
        memset(inputData, 0, COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);
        memset(outputData, 0, COLOR_CHANNELS * SCR_WIDTH * SCR_HEIGHT);

        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        //glfwSwapBuffers(window);
        //glfwPollEvents();
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

    free(readFileName);
    readFileName = NULL;

    free(writeFileName);
    writeFileName = NULL;

    c_end = clock();
    printf("The time used for processing all pictures are %f ms \n", difftime(c_end, c_start));

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

int getInputOutputFileName(char* inputFileName, char* outputFileName)
{
    return 0;
}
