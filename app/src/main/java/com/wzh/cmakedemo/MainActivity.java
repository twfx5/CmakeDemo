package com.wzh.cmakedemo;

import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.os.Bundle;
import android.os.Looper;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private ConstraintLayout container;
    private MyTextView sample_text;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        container = findViewById(R.id.container);
        sample_text = findViewById(R.id.sample_text);

        container.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(MainActivity.this, "container", Toast.LENGTH_SHORT).show();
            }
        });

        sample_text.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(MainActivity.this, "sample_text", Toast.LENGTH_SHORT).show();
            }
        });


        int i[] = {11,22,33};
        String j[] = {"动","脑"};
//        testJNI(i, j);

        test("123", i, j);

        testBean(new Bean());

        dynamicTest();
        dynamicTest2(88);

        testThread();
    }



    public native void testJNI(int[] i, String[] j);

    native void test(String str, int [] i, String[] j);

    native void testBean(Bean bean);

    // 使用JNI_OnLoad动态注册
    native void dynamicTest();
    native int dynamicTest2(int i);

    native void testThread();

    public void updateUI() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, "c++ 线程更新UI",Toast.LENGTH_SHORT).show();
            }
        });
    }

}
