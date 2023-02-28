package com.Plugins.SolanaMobileUE5;

import android.support.annotation.Keep;


@Keep
public class HelloWorldClass {

	//----- Calling Java code synchronously -------------------
	@Keep
	public static String HelloWorldOnAndroid(String text) {
		text += " on Android";
		return text;
	}

}

