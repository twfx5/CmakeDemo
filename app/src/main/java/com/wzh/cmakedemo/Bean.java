package com.wzh.cmakedemo;

import android.util.Log;

public class Bean {

    private static final String TAG = "Bean";

    private int i = 123;

    public int getI() {
        Log.e(TAG, "getI: 方法调用");
        return i;
    }

    public void setI(int i) {
        this.i = i;
        Log.e(TAG, "setI: 方法调用 , i = " + i );
    }

    private void print() {
        Log.e(TAG, "print: 方法调用");
    }
}
