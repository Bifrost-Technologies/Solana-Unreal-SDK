package com.Plugins.SolanaMobileUE5;

import android.app.Activity;
import android.support.annotation.Keep;


@Keep
public class asyncHelloWorldClass {

	//Calling *.cpp code
	@Keep
	public static native void CallBackCppAndroid(String returnStr);

	// Calling Java code asynchronously and returning the value back to C++
	@Keep
	public static void asyncHelloWorldOnAndroid(final Activity activity, final String text) {
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run()
			{
				CallBackCppAndroid(text+" on Android");
			}
		});
	}

}


