package ru.cs.detector.view;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.Button;
import ru.cs.detector.tools.NativeTools;

import java.io.IOException;
import java.util.List;

/**
 * Author: Aleksey Vladiev (Avladiev2@gmail.com)
 */


abstract public class ViewBase implements SurfaceHolder.Callback, Runnable, View.OnClickListener, Camera.AutoFocusCallback {
    private final static String TAG = "SurfaceViewTAG";
    private static final String CAPTURE = "Capture";
    private static final String CLEAN = "Clean";

    private Camera mCamera;
    private final SurfaceHolder mHolder;
    private int mFrameWidth;
    private int mFrameHeight;
    private int mScreenWidth;
    private int mScreenHeight;
    protected boolean isInit = false;

    private int mWidthScreen;
    private int mHeightScreen;
    protected volatile boolean isClear;
    private volatile byte[] mFrameData;
    private final Thread mThread = new Thread(this);
    protected volatile boolean isCapture;

    private final OnePreviewCallback onePreviewCallback = new OnePreviewCallback();
    private final PreviewCallback previewCallback = new PreviewCallback();


    public ViewBase(final SurfaceHolder holder) {
        mHolder = holder;

    }

    public int getFrameWidth() {
        return mFrameWidth;
    }

    public int getFrameHeight() {
        return mFrameHeight;
    }

    public void surfaceCreated(final SurfaceHolder holder) {
        mCamera = Camera.open();
        mCamera.setPreviewCallback(previewCallback);
        mThread.start();
    }


    @Override
    public void surfaceChanged(final SurfaceHolder holder, final int format, final int width, final int height) {
        if (mCamera != null) {
            Camera.Parameters params = mCamera.getParameters();
            List<Camera.Size> sizes = params.getSupportedPreviewSizes();
            int widthF = width;
            int heightF = height;


            // selecting optimal camera preview size
            {
                double minDiff = Double.MAX_VALUE;
                for (Camera.Size size : sizes) {
                    if (Math.abs(size.height - height) < minDiff) {
                        widthF = size.width;
                        heightF = size.height;
                        minDiff = Math.abs(size.height - height);
                    }
                }
            }
            params.setPreviewSize(widthF, heightF);


            mCamera.setParameters(params);
            try {
                mCamera.setPreviewDisplay(null);
            } catch (IOException e) {
                Log.e(TAG, "mCamera.setPreviewDisplay fails: " + e);
            }
            mWidthScreen = width;
            mHeightScreen = height;
            mFrameHeight = heightF;
            mFrameWidth = widthF;

            mCamera.startPreview();
        }

    }

    public void surfaceDestroyed(final SurfaceHolder holder) {
        mThread.interrupt();
        if (mCamera != null) {
            synchronized (this) {
                mCamera.stopPreview();
                mCamera.setPreviewCallback(null);
                mCamera.release();
                mCamera = null;
            }
        }
        NativeTools.releaseResources();
        try {
            mThread.join();
        } catch (InterruptedException ignored) {
        }
    }

    protected abstract Bitmap processFrame(final byte[] data);


    public void run() {
        Bitmap bmp;
        float scaleX;
        float scaleY;

        byte[] frameData;
        while (!Thread.currentThread().isInterrupted()) {
            synchronized (this) {
                try {
                    while (this.mFrameData == null) {
                        this.wait();
                    }
                    Log.i(TAG, "postWait");
                    frameData = this.mFrameData;
                    this.mFrameData = null;
                } catch (InterruptedException e) {
                    return;
                }
            }
            bmp = processFrame(frameData);
            final Canvas canvas = mHolder.lockCanvas();
            if (canvas != null) {
                scaleX = ((float) mWidthScreen) / bmp.getWidth();
                scaleY = ((float) mWidthScreen) / bmp.getWidth();
                canvas.scale(scaleX, scaleY);
                canvas.drawBitmap(bmp, 0, 0, null);
                mHolder.unlockCanvasAndPost(canvas);
            }
            bmp.recycle();
        }
    }


    public void onAutoFocus(boolean success, Camera camera) {
        if (success) {
            mCamera.setOneShotPreviewCallback(onePreviewCallback);
        } else {

        }
    }


    public void onClick(final View v) {
        final Button button = (Button) v;
        if (button.getText().toString().equals(CAPTURE)) {
            isCapture = true;
            mCamera.autoFocus(this);
            button.setText(CLEAN);
            isClear = false;
        } else {
            button.setText(CAPTURE);
            isClear = true;
        }


    }


    final class OnePreviewCallback implements Camera.PreviewCallback {
        @Override
        public void onPreviewFrame(final byte[] data, final Camera camera) {
            synchronized (ViewBase.this) {
                mFrameData = data;
                isCapture = true;
                ViewBase.this.notify();
            }
            camera.setPreviewCallback(previewCallback);
        }
    }

    final class PreviewCallback implements Camera.PreviewCallback {
        @Override
        public void onPreviewFrame(final byte[] data, final Camera camera) {
            synchronized (ViewBase.this) {
                if (mFrameData == null) {
                    mFrameData = data;
                    ViewBase.this.notify();
                }
            }

        }
    }


}
