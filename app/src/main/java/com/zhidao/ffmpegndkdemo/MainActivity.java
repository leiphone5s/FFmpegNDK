package com.zhidao.ffmpegndkdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Message;
import android.util.ArrayMap;
import android.util.Log;
import android.view.Surface;
import android.widget.TextView;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import io.reactivex.Observable;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.schedulers.Schedulers;

public class MainActivity extends AppCompatActivity {

    Disposable disposable;
    private final String TAG = "FFmpegNdk";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        dealVideoPathWithUsbOtg("/sdcard/Recfront_20200828_143515.mp4");
    }


    private void dealVideoPathWithUsbOtg(String path) {
        disposable = initExtractFrameRx(path)
                .subscribeOn(Schedulers.io())
                .observeOn(Schedulers.io())
                .subscribe(paths -> Log.d(TAG,"绝对路径为："), throwable -> {

                }, () -> {

                });

    }


    /**
     * 初始化Rxjava异步抽帧观察者
     *
     * @param path
     * @return
     */
    public Observable<List<String>> initExtractFrameRx(final String path) {

        return Observable.create(modelObservableEmitter -> {
            //List<String> paths =
            decodeVideo(path);
            modelObservableEmitter.onComplete();
        });
    }



    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public  native void decodeVideo(String path);
}
