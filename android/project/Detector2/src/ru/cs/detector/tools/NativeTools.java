package ru.cs.detector.tools;

/**
 * Author: Aleksey Vladiev (Avladiev2@gmail.com)
 */
public  class NativeTools {

    static {
        System.loadLibrary("native_tools");
    }

    public static native void releaseResources();

    //====================================================
    public static native void toRGBA(int width, int height, byte yuv[], int[] rgba);

    //init + concert in RGBA;
    public static native void initResources(int width, int height, byte[] yuvData, int[] rgba);


    /**
     * @param width
     * @param height
     * @param yuvData
     * @param rgba
     * @return true if capture
     */
    public static native boolean captureText(int width, int height, byte[] yuvData, int[] rgba);

    public static native boolean trackText(int width, int height, byte[] yuvData, int[] rgba);
    //==========================================================



    public static native void initResources(int width, int height, byte[] yuvData);

    public static native Rect captureText(int width, int height, byte[] yuvData);

    public static native Rect trackText(int width, int height, byte[] yuvData);

}