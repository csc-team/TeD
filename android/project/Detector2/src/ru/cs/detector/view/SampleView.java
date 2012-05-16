package ru.cs.detector.view;

import android.graphics.Bitmap;
import android.util.Log;
import android.view.SurfaceHolder;
import ru.cs.detector.tools.NativeTools;

/**
 * Author: Aleksey Vladiev (Avladiev2@gmail.com)
 */
public class SampleView extends ViewBase {
    private final static String TAG = "SampleViewTAG";
    protected volatile boolean isTracking = false;

    public SampleView(final SurfaceHolder holder) {
        super(holder);
    }



    @Override
    protected Bitmap processFrame(final byte[] data) {
        final int widthFrame = getFrameWidth();
        final int heightFrame = getFrameHeight();
        int frameSize = widthFrame * heightFrame;
        int[] rgba = new int[frameSize];
        if (isInit) {
            if (isCapture) {
                Log.i(TAG,"captureText");
                isTracking = NativeTools.captureText(widthFrame, heightFrame, data, rgba);
                Log.i(TAG,"post captureText");
                isCapture = false;
            } else {
                    if (isTracking && !isClear) {
                    Log.i(TAG,"trackText");
                    isTracking = NativeTools.trackText(widthFrame, heightFrame, data, rgba);
                    Log.i(TAG," post trackText");
                } else {
                    Log.i(TAG," toRGBA");
                    NativeTools.toRGBA(widthFrame, heightFrame, data, rgba);
                    Log.i(TAG," post toRGBA");
                }
            }

        } else {
            NativeTools.initResources(getFrameWidth(), getFrameHeight(), data, rgba);
            isInit = true;
        }
        Bitmap bmp = Bitmap.createBitmap(widthFrame, heightFrame, Bitmap.Config.ARGB_8888);
        bmp.setPixels(rgba, 0, widthFrame, 0, 0, widthFrame, heightFrame);
        return bmp;
    }
}
