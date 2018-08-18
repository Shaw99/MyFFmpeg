package module.video.jnc.myffmpeg;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by xhc on 2018/5/10.
 */

public class MyVideoGpuShow extends GLSurfaceView implements SurfaceHolder.Callback  ,  GLSurfaceView.Renderer{

    public MyVideoGpuShow(Context context) {
        super(context);
    }

    public MyVideoGpuShow(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setPlayPath(String path){
        new MyThread(path).start();
    }

    private boolean renderSet = false;

    public boolean isRender(){
        return renderSet;
    }

    @Override
    public void surfaceCreated(SurfaceHolder var1) {
        Log.e("xhc" ,  " MyVideoGpuShow surfaceCreated 1");
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8,8,8,8,16,0);
        setRenderer(this);//android 8.0需要设置
        setRenderMode(RENDERMODE_WHEN_DIRTY);
        renderSet = true;
        Log.e("xhc" ,  " MyVideoGpuShow surfaceCreated 2");
    }

    @Override
    public void surfaceChanged(SurfaceHolder var1, int var2, int var3, int var4) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder var1) {

    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }

    class MyThread extends Thread {
        String path ;
        MyThread(String path){
            this.path = path;
        }

        @Override
        public void run() {
            super.run();
            FFmpegUtils.initMp4Play(path, getHolder().getSurface());
        }
    }
}
