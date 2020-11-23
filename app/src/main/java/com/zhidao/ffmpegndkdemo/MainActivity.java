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
import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.disposables.Disposable;
import io.reactivex.schedulers.Schedulers;

public class MainActivity extends AppCompatActivity {

    Disposable disposable;
    private final String TAG = "FFmpegNdk";
    LinkedList<Long> timeList = new LinkedList<>();
    LinkedList<String> picList = new LinkedList<>();
    LinkedList<String> sList = new LinkedList<>();

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
        timeList.add(100L);
        timeList.add(200L);
        timeList.add(300L);
        timeList.add(400L);
        timeList.add(500L);
        timeList.add(600L);
        timeList.add(700L);
        timeList.add(800L);
        timeList.add(900L);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text = tv.getText().toString();
                Log.d(TAG,"输入时间戳为"+text);
                if(!TextUtils.isEmpty(text)){

                    dealVideoPathWithUsbOtg(timeList);

                }
            }
        });

    }


    private void dealVideoPathWithUsbOtg(LinkedList<Long> timeStamp) {
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
    public Observable<List<String>> initExtractFrameRx(final String path,final LinkedList<Long> times) {

        return Observable.create(modelObservableEmitter -> {
            String basePath = Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator;
            String fileSufix = ".jpeg";

            for(long time : times) {
                String fileName = basePath+time+fileSufix;
                int code = decodeVideo(path, time, fileName);
                if(code == 0){
                    picList.add(fileName);
                }
                Log.d(TAG,fileName+"返回值为:"+code);
            }

            for(String url : picList){
                String md5 = getMD5Three(url);
                if(md5 != null){
                    Log.d(TAG,url+"的MD5值为:"+md5);
                    sList.add(md5);
                }
            }

            modelObservableEmitter.onComplete();
        });
    }


    public static String getMD5Three(String path) {
        BigInteger bi = null;
        try {
            byte[] buffer = new byte[8192];
            int len = 0;
            MessageDigest md = MessageDigest.getInstance("MD5");
            File f = new File(path);
            FileInputStream fis = new FileInputStream(f);
            while ((len = fis.read(buffer)) != -1) {
                md.update(buffer, 0, len);
            }
            fis.close();
            byte[] b = md.digest();
            bi = new BigInteger(1, b);
        } catch (NoSuchAlgorithmException | IOException e) {
            e.printStackTrace();
        }
        if(bi != null) {
            return bi.toString(16);
        }else{
            return null;
        }
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

    public  native int decodeVideo(String path,long timeStamp,String save_name);
}
