#include <OpenGL/gl3.h>
#define __gl_h_
#include <OpenGL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const char * read_txt_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if(f == NULL)
    {
        printf("Could not read %s\n", filename);
        return NULL;
    }

    // Go to end of file.
    int err = fseek(f, 0, SEEK_END);
    if(err)
    {
        fclose(f);
        return NULL;
    }
    
    // Get current read position, this is the length!
    long length = ftell(f);
   
    // Go back to beginning of file
    err = fseek(f, 0, SEEK_SET);
    if(err)
    {
        fclose(f);
        return NULL;
    }

    // Allocate space
    char *contents = (char *)malloc((length+1) * sizeof(char));
    if(contents == NULL)
    {
        printf("Could not allocate space for file %s\n", filename);
        fclose(f);
        return NULL;
    }

    // Read the thing!
    size_t actual_length = fread(contents, sizeof(char), length, f);
    contents[actual_length] = '\0';

    fclose(f);

    return contents;
}

void printShaderLog(GLuint shader)
{
    if(!glIsShader(shader))
    {
        printf("GL ID %lu is not a shader\n", (unsigned long) shader);
        return;
    }

    GLsizei infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar infoLog[infoLogLength+1];
    glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
    infoLog[infoLogLength] = '\0';
    printf("Shader %lu Log\n", (unsigned long) shader);
    printf("%s\n", infoLog);
}

void printProgramLog(GLuint program)
{
    if(!glIsProgram(program))
    {
        printf("GL ID %lu is not a program\n", (unsigned long) program);
        return;
    }

    GLsizei infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    
    GLchar infoLog[infoLogLength+1];
    glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
    infoLog[infoLogLength] = '\0';
    
    printf("Program %lu Log\n", (unsigned long) program);
    printf("%s\n", infoLog);
}

GLuint compileShader(const char *shader_path, int shader_type)
{
    const char *shader_src = read_txt_file(shader_path);
    if(shader_src == NULL)
    {
        printf("Error reading shader file: %s\n", shader_path);
        return 0;
    }
    
    GLuint shaderID = glCreateShader(shader_type);

    glShaderSource(shaderID, 1, &shader_src, NULL);
    free((char *) shader_src);
    
    glCompileShader(shaderID);
    
    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if(!status)
    {
        printf("Error compiling shader\n");
        printShaderLog(shaderID);
        glDeleteShader(shaderID);
        return 0;
    }

    return shaderID;
}

int loadGLProgram(GLuint *programID, const char *vrtx_shader_path,
                  const char *frag_shader_path)
{
    GLuint vrtxID = compileShader(vrtx_shader_path, GL_VERTEX_SHADER);
    if(vrtxID == 0)
    {
        printf("Error making vertex shader\n");
        return 1;
    }

    GLuint fragID = compileShader(frag_shader_path, GL_FRAGMENT_SHADER);
    if(fragID == 0)
    {
        printf("Error making fragment shader\n");
        glDeleteShader(vrtxID);
        return 1;
    }


    *programID = glCreateProgram();
    glAttachShader(*programID, vrtxID);
    glAttachShader(*programID, fragID);
    glLinkProgram(*programID);
    
    GLint status;
    glGetProgramiv(*programID, GL_LINK_STATUS, &status);
    if(!status)
    {
        printf("Error Linking Program\n");
        printProgramLog(*programID);
        glDetachShader(*programID, vrtxID);
        glDetachShader(*programID, fragID);
        glDeleteShader(vrtxID);
        glDeleteShader(fragID);
        return 1;
    }

    glDetachShader(*programID, vrtxID);
    glDetachShader(*programID, fragID);

    return 0;
}

void dump_float_array(float *f, int n)
{
    float min = 1e30f;
    float max = -1e30f;
    double mean = 0.0;

    int i;

    for(i = 0; i < n; i++)
    {
        if(f[i] < min)
            min = f[i];
        if(f[i] > max)
            max = f[i];
        mean += f[i];
    }

    mean /= n;

    printf("array: %e %le %e\n", min, mean, max);
}

void dump_uint_array(unsigned int *f, int n)
{
    unsigned int min = 0xffffffff;
    unsigned int max = 0u;
    unsigned int mean = 0u;

    int i;

    for(i = 0; i < n; i++)
    {
        if(f[i] < min)
            min = f[i];
        if(f[i] > max)
            max = f[i];
        mean += f[i];
    }

    mean /= n;

    printf("array: %u %u %u\n", min, mean, max);
}

void dump_int_array(int *f, int n)
{
    int min = 2000000000;
    int max = -2000000000;
    int mean = 0;

    int i;

    for(i = 0; i < n; i++)
    {
        if(f[i] < min)
            min = f[i];
        if(f[i] > max)
            max = f[i];
        mean += f[i];
    }

    mean /= n;

    printf("array: %d %d %d\n", min, mean, max);
}

void dump_byte_array(unsigned char *f, int n)
{
    int min = 100000000;
    int max = 0;
    int mean = 0;

    int i;

    for(i = 0; i < n; i++)
    {
        if(f[i] < min)
            min = f[i];
        if(f[i] > max)
            max = f[i];
        mean += f[i];
    }

    mean /= n;

    printf("array: %d %d %d (%x %x %x)\n", min, mean, max, min, mean, max);
}

int main( int argc, char* args[] )
{
	SDL_Window* window = NULL;
    SDL_GLContext context;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	    SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
	}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);

    //Create window
    window = SDL_CreateWindow("Painting With Maths",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if( window == NULL )
    {
        printf("Window could not be created! SDL_Error: %s\n",
               SDL_GetError() );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
    }
    context = SDL_GL_CreateContext(window);

    if(context == NULL)
    {
        printf("OpenGL context could not be created! SDL_Error: %s\n",
               SDL_GetError() );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
    }

    //SDL_GL_SetSwapInterval(1);

    GLuint programID;
    int status = loadGLProgram(&programID, "canvas.glsl", "paint.glsl");
    if(status)
    {
        printf("Couldn't load GL Program\n");
        glDeleteProgram(programID);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    GLuint screenProgramID;
    status = loadGLProgram(&screenProgramID, "screen_vertex.glsl",
                                "screen_fragment.glsl");
    if(status)
    {
        printf("Couldn't load screen GL Program\n");
        glDeleteProgram(programID);
        glDeleteProgram(screenProgramID);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    glClearColor(0.5f, 0.8f, 0.4f, 1.0f); 

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    
    GLfloat xloc, yloc;
    if(width < height)
    {
        xloc = 1.0;
        yloc = ((GLfloat) width) / height;
    }
    else
    {
        xloc = ((GLfloat) height) / width;
        yloc = 1.0;
    }

    xloc = 1.0f;
    yloc = 1.0f;

    GLfloat vertexData[] = {-xloc, -yloc, xloc, -yloc,
                            xloc, yloc, -xloc, yloc};
    GLfloat uvData[] = {0.0f, 0.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 0.0f, 1.0f};

    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat),
                 vertexData, GL_STATIC_DRAW);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat),
                 uvData, GL_STATIC_DRAW);

    GLint vertexPositionLocation = glGetAttribLocation(programID,
                                                        "vertexPosition");
    GLint vertexUVLocation = glGetAttribLocation(programID,
                                                        "vertexUV");
    if(vertexPositionLocation < 0)
    {
        printf("Bad variable name\n");
        glDeleteProgram(programID);
        glDeleteProgram(screenProgramID);
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
    }

    GLint resID = glGetUniformLocation(programID, "iresolution");
    GLint sph_xyID = glGetUniformLocation(programID, "sph_xy");

    /*
	SDL_Surface* screenSurface = NULL;
    screenSurface = SDL_GetWindowSurface( window );

    SDL_FillRect(screenSurface, NULL,
                 SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
    
    SDL_UpdateWindowSurface( window );
    */

    /*
     * Setup FrameBuffer
     */

    // First make and bind (make active) a framebuffer object (FBO)
    GLuint framebufferID;
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

    // Make and bind a texture to store color.
    GLuint textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           textureColorbuffer, 0);

    // Make and bind a texture to store depth.
    
    GLuint textureDepthbuffer;
    glGenTextures(1, &textureDepthbuffer);
    glBindTexture(GL_TEXTURE_2D, textureDepthbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height,
                 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           textureDepthbuffer, 0);
   
    GLint screenSceneTexLocation = glGetUniformLocation(screenProgramID,
                                                        "sceneTexture");
    GLint screenDepthTexLocation = glGetUniformLocation(screenProgramID,
                                                        "depthTexture");

    printf("screen sceneTex: %d\n", screenSceneTexLocation);
    printf("screen depthTex: %d\n", screenDepthTexLocation);

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    printf("Paint Program: %u\n", programID);
    printf("Screen Program: %u\n", screenProgramID);
    printf("Current Program: %d\n", currentProgram);
    // Check if we're ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Incomplete framebuffer\n");
        glDeleteProgram(programID);
        glDeleteProgram(screenProgramID);
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
    }

    // Good to go. Rebind the default framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int quit = 0;
    float sph_x = 0.0f;
    float sph_y = 5.0f;
    float sph_step = 0.5f;

    //Enter event loop
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                switch(e.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = 1;
                        break;
                    case SDLK_q:
                        quit = 1;
                        break;
                    case SDLK_LEFT:
                        sph_x -= sph_step;
                        break;
                    case SDLK_RIGHT:
                        sph_x += sph_step;
                        break;
                    case SDLK_DOWN:
                        sph_y -= sph_step;
                        break;
                    case SDLK_UP:
                        sph_y += sph_step;
                        break;
                }
            }

            // First pass, render to off-screen framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
            glEnable(GL_DEPTH_TEST);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(programID);

            glEnableVertexAttribArray(vertexPositionLocation);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glVertexAttribPointer(vertexPositionLocation, 2, GL_FLOAT,
                                  GL_FALSE, 0, NULL);

            glEnableVertexAttribArray(vertexUVLocation);
            glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
            glVertexAttribPointer(vertexUVLocation, 2, GL_FLOAT,
                                  GL_FALSE, 0, NULL);

            SDL_GetWindowSize(window, &width, &height);
            glUniform2i(resID, width, height);
            glUniform2f(sph_xyID, sph_x, sph_y);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glDisableVertexAttribArray(vertexPositionLocation);


            GLfloat * texData = malloc(width*height*sizeof(GLfloat));
            glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT,
                         GL_FLOAT, texData);
            dump_float_array((float *)texData, width*height);
            free(texData);

            GLubyte * texData2 = malloc(width*height);
            glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT,
                         GL_UNSIGNED_BYTE, texData2);
            dump_byte_array((unsigned char *)texData2, width*height);
            free(texData2);


            //Second pass, copy back to on-screen window
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.8f, 0.5f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(screenProgramID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureDepthbuffer);
            glUniform1i(screenSceneTexLocation, 0);
            glUniform1i(screenDepthTexLocation, 1);

            glEnableVertexAttribArray(vertexPositionLocation);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glVertexAttribPointer(vertexPositionLocation, 2, GL_FLOAT,
                                  GL_FALSE, 0, NULL);

            glEnableVertexAttribArray(vertexUVLocation);
            glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
            glVertexAttribPointer(vertexUVLocation, 2, GL_FLOAT,
                                  GL_FALSE, 0, NULL);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glDisableVertexAttribArray(vertexPositionLocation);

            SDL_GL_SwapWindow(window);

        }
    }

	//Destroy window

    glDeleteProgram(programID);
    glDeleteProgram(screenProgramID);
	SDL_DestroyWindow( window );
	SDL_Quit();

	return 0;
}
