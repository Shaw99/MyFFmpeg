//
// Created by Administrator on 2018/10/17/017.
//

#include <string.h>
#include <my_log.h>
#include "OpenGlTest.h"


OpenGlTest::OpenGlTest(const char* path , ANativeWindow *win){
    int len = strlen(path);
    this->path = (char *)malloc(len);
    strcpy(this->path , path);
    this->win = win;
    outWidth = 640 ;
    outHeight = 360 ;
    LOGE("path %s " , this->path);

}

GLuint OpenGlTest::InitShader(const char *code, GLint type) {
    GLuint sh = glCreateShader((GLenum) type);
    if (sh == 0) {
        LOGE("glCreateShader FAILD ");
        return 0;
    }
    //加载shader
    glShaderSource(sh, 1, &code, 0);
    //编译shader
    glCompileShader(sh);
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        LOGE("glCompieshader FAILD ");
        return (GLuint) -1;
    }
    return sh;
}

void OpenGlTest::start(){
    int pixSize = outWidth * outHeight;
    if(!initOpenglFlag){
        initOpenglFlag = true;
        init_opengl();
    }

    uint8_t *buf_y = (uint8_t *)malloc(pixSize);
    uint8_t *buf_u = (uint8_t *)malloc(pixSize / 4 );
    uint8_t *buf_v = (uint8_t *)malloc(pixSize / 4);
    FILE *yuv = fopen(this->path , "r");
    int allSize = (int)(pixSize * (float)3 / 2);
    LOGE("ALL SIZE %d" , allSize);
    uint8_t *buf =  (uint8_t *)malloc(allSize);

    fread(buf , 1 , allSize , yuv );

    memcpy(buf_y , buf , pixSize);
    memcpy(buf_u , buf , pixSize / 4);
    memcpy(buf_v , buf , pixSize / 4);

    showYuv(buf_y , buf_u ,  buf_v);
}


// 0 y , 1 u , 2 v
int OpenGlTest::showYuv(uint8_t *buf_y, uint8_t *buf_u, uint8_t *buf_v) {

    //激活第1层纹理,绑定到创建的opengl纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texts[0]);
    //替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, outWidth, outHeight, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE,
                    buf_y);

    //激活第2层纹理,绑定到创建的opengl纹理
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texts[1]);
    //替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, outWidth / 2, outHeight / 2, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE, buf_u);

    //激活第2层纹理,绑定到创建的opengl纹理
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, texts[2]);
    //替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, outWidth / 2, outHeight / 2, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE, buf_v);

    //三维绘制
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //窗口显示
    eglSwapBuffers(display, winsurface);
    ////纹理的修改和显示

    return RESULT_SUCCESS;
}

int OpenGlTest::init_opengl() {
    if (outHeight == 0 || outWidth == 0) {
        LOGE(" outHeight == 0 || outWidth == 0 ");
        return RESULT_FAILD;
    }
    initOpenglFlag = true;
    LOGE("  out width %d , out height %d ", outWidth, outHeight);
    //获取原始窗口

    //egl display 创建和初始化
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGE(" eglGetDisplay FAILD ! ");
        return -1;
    }
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        LOGE(" eglInitialize FAILD ! ");
        return -1;
    }
    //窗口配置，输出配置
    EGLConfig config;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE,
            EGL_WINDOW_BIT,
            EGL_NONE
    };
    if (EGL_TRUE != eglChooseConfig(display, configSpec, &config, 1, &configNum)) {
        LOGE("eglchooseconfig faild !");
        return -1;
    }
    //创建surface
    winsurface = eglCreateWindowSurface(display, config, win, 0);
    if (winsurface == EGL_NO_SURFACE) {
        LOGE(" eglCreateWindowSurface FAILD !");
        return -1;
    }
    //3 context 创建关联的上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //context 创建关联的 上下文
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext FAILD !");
    }
    if (EGL_TRUE != eglMakeCurrent(display, winsurface, winsurface, context)) {
        LOGE(" eglMakeCurrent faild ! ");
        return -1;
    }


    vsh = InitShader(vertexShader2, GL_VERTEX_SHADER);
    fsh = InitShader(fragYUV420P2, GL_FRAGMENT_SHADER);

    program = glCreateProgram();

    if (program == 0) {
        LOGE(" glCreateProgram FAILD ! ");
        return -1;
    }

    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        LOGE(" GLINK PROGRAM faild !");
        return -1;
    }
    glUseProgram(program);

    //加入三维顶点数据 两个三角形组成正方形
    const float vers[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
    };

    apos = (GLuint) glGetAttribLocation(program, "aPosition");
    glEnableVertexAttribArray(apos);

    //传递顶点
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, vers);

    //加入材质坐标数据
    const float txts[] = {
            1.0f, 0.0f, //右下
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0, 1.0
    };

    atex = (GLuint) glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(atex);

    glVertexAttribPointer(atex, 2, GL_FLOAT, GL_FALSE, 8, txts);

    //材质纹理初始化
    //设置纹理层
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0); //对于纹理第1层
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1); //对于纹理第2层
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2); //对于纹理第3层


    //创建三个纹理
    glGenTextures(3, texts);
    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 outWidth, outHeight, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 outWidth / 2,
                 outHeight / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 outWidth / 2, outHeight / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );


    LOGE(" INIT shader SUCCESS ");
    return RESULT_SUCCESS;
}

OpenGlTest::~OpenGlTest(){
    initOpenglFlag = false;
    glDisableVertexAttribArray(apos);
    glDisableVertexAttribArray(atex);
    glDetachShader(vsh, program);
    glDetachShader(fsh, program);
    glDeleteShader(vsh);
    glDeleteShader(fsh);
    glDeleteProgram(program);
    glDeleteTextures(3, texts);
    eglDestroyContext(display, context);
    eglDestroySurface(display, winsurface);
    eglTerminate(display);
    if (win != NULL) {
        ANativeWindow_release(win);
    }
//    if(buf_y != NULL){
//        free(buf_y);
//    }
//    if(buf_u != NULL){
//        free(buf_u);
//    }
//    if(buf_v != NULL){
//        free(buf_v);
//    }
    LOGE(" destroyShader ");
}