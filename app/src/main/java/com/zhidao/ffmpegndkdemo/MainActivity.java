package com.zhidao.ffmpegndkdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;
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
                    dealVideoPathWithUsbOtg(4000);

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


            String picPath = decodeVideo(path,timeStamp,"/sdcard/success.jpeg");

           // Log.d("lei","文件头为："+getFileHeader(picPath));
            //Bitmap bitmap = getimage(picPath);

//            byte[] data = readStream(picPath);
//            buff2Image(data,"/sdcard/test.jpg");

            modelObservableEmitter.onComplete();
        });
    }

    

    /**
     * 获取图片的字节数组
     *
     * @param imagepath
     * @return byte[]
     */
    public static byte[] readStream(String imagepath) {
        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        ;
        try {
            FileInputStream fs = new FileInputStream(imagepath);
            byte[] buffer = new byte[1024];
            int len = 0;
            while (-1 != (len = fs.read(buffer))) {
                outStream.write(buffer, 0, len);
            }
            outStream.close();
            fs.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return outStream.toByteArray();
    }



    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public  native String decodeVideo(String path,long timeStamp,String save_name);
}
