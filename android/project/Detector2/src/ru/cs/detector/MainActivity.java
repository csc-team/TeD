package ru.cs.detector;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.*;
import android.widget.Button;
import ru.cs.detector.view.SampleView;
import ru.cs.detector.view.ViewBase;

public class MainActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        setContentView(R.layout.main);

        final SurfaceView surfaceView = (SurfaceView) findViewById(R.id.preview_view);
        final SurfaceHolder surfaceHolder = surfaceView.getHolder();
        final ViewBase viewBase = new SampleView(surfaceHolder);
        surfaceHolder.addCallback(viewBase);
        final Button button = (Button) findViewById(R.id.Button01);
        button.setOnClickListener(viewBase);
    }

}
