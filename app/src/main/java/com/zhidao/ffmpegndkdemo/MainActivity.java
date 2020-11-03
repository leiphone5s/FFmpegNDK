package com.zhidao.ffmpegndkdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import java.util.List;

import io.reactivex.Observable;
import io.reactivex.disposables.Disposable;
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
        EditText tv = findViewById(R.id.sample_text);
        Button button = findViewById(R.id.buttonPanel);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text = tv.getText().toString();
                Log.d(TAG,"输入时间戳为"+text);
                if(!TextUtils.isEmpty(text)){
                    long timeStamp = Long.parseLong(text);
                    long videoStamp = 1598596515000L;
                    long offset = timeStamp-videoStamp;
                    Log.d(TAG,"偏移量为"+offset);
                    dealVideoPathWithUsbOtg(offset);

                }
            }
        });

    }


    private void dealVideoPathWithUsbOtg(long timeStamp) {
        disposable = initExtractFrameRx("/sdcard/Recfront_20200828_143515.mp4",timeStamp)
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
    public Observable<List<String>> initExtractFrameRx(final String path,final long timeStamp) {

        return Observable.create(modelObservableEmitter -> {
            //List<String> paths =
            decodeVideo(path,timeStamp);
            modelObservableEmitter.onComplete();
        });
    }



    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public  native void decodeVideo(String path,long timeStamp);
}
