/*
 * Copyright (c) 2025 Adrien ARNAUD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package com.aarnaud.vkwsamples;

import static com.aarnaud.vkwsamples.SampleInfo.SAMPLE_ID_KEY;
import static com.aarnaud.vkwsamples.SampleInfo.SAMPLE_NAME_KEY;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.activity.EdgeToEdge;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class SampleActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("vkw-samples");
    }

    private SurfaceView samplesSurfaceView = null;
    private String sampleName;
    private int sampleId;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("vkw-samples", "SampleActivity.onCreate()");
        super.onCreate(savedInstanceState);

        final Intent intent = getIntent();
        sampleName = intent.getStringExtra(SAMPLE_NAME_KEY);
        sampleId = intent.getIntExtra(SAMPLE_ID_KEY, 0);

        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_sample);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        samplesSurfaceView = findViewById(R.id.samples_surface_view);
        samplesSurfaceView.getHolder().addCallback(this);

        if (!InitSample(sampleId)) {
            AlertDialog alertDialog = new AlertDialog.Builder(SampleActivity.this).create();
            alertDialog.setTitle(sampleName);
            alertDialog.setMessage("Could not initialize sample, device is probably not compatible");
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            finish(); // Finish the activity
                        }
                    });
            alertDialog.show();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d("vkw-samples", "SampleActivity.onStart()");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d("vkw-samples", "SampleActivity.onResume()");
        StartSample();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d("vkw-samples", "SampleActivity.onPause()");
        StopSample();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d("vkw-samples", "SampleActivity.onStop()");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d("vkw-samples", "SampleActivity.onDestroy()");
        DestroySample();
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                this.finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.d("vkw-samples", "SampleActivity.surfaceCreated()");
        InitNativeWindow(holder.getSurface());
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.d("vkw-samples", "SampleActivity.surfaceChanged()");
        ResizeNativeWindow();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.d("vkw-samples", "SampleActivity.surfaceDestroyed()");
        DestroyNativeWindow();
    }

    // Native methods
    public static native boolean InitSample(int sampleId);

    public static native boolean DestroySample();

    public static native boolean StartSample();

    public static native boolean StopSample();

    public static native boolean InitNativeWindow(Surface surface);

    public static native boolean ResizeNativeWindow();

    public static native boolean DestroyNativeWindow();
}