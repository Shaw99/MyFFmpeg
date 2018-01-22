package module.video.jnc.myffmpeg;

/**
 * Created by xhc on 2017/11/1.
 */

public class FFmpegUtils {

    static {
        System.loadLibrary("avutil");
        System.loadLibrary("avcodec");
        System.loadLibrary("avformat");
        System.loadLibrary("avdevice");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("avfilter");
        System.loadLibrary("ffmpeg");
    }


    public static native String stringNative();

    public static native String stringJni();

    public static native int encode(String inFilePath , String outputPath);

    public static native int decode(String filePath , String outputStr);

    public static native int stream(String inputUrl , String ouputUrl);

    public static native int encodeYuv(String inputUrl , String outputPath);

    public static native int transcodeing(String inputUrl , String outputPath);

    public static native int addfilter(String inputStr , String outputPath);

    public static native int swscale(String inputUrl , String outputPath);

    public static native int muxer(String output , String inputVPath , String inputAPath);

    public static native int demuxer(String inputUrl , String outputVPath , String outputAPath);

    public static native int myInit(String outputPath , int width , int height);

    public static native int nv21ToYv12(byte[] bytes);

    public static native int encodePcm(byte[] bytes , int size);

    public static native int testArray(byte[] bytes);

    public static native void closeMyFFmpeg();

    public static native void encodeCamera(byte[] bytes);

    public static native int initMyCameraMuxer(String outputPath , int width , int height , int aSize);

    public static native int encodeMyMuxerCamera(byte[] bytes);

    public static native int encodeMyMuxerAudio(byte[] bytes);

    public static native int closeMyMuxer();

    public static native int init_audio_(String outputPath , int aSise);
    public static native int encode_audio_(byte[] bytes);
    public static native int close_audio_();
}
