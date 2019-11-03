package com.xrbpowered.imgpaths;

public class Profile {

	public final String name;
	public final long start;
	
	public Profile(String name) {
		this.name = name;
		this.start = System.currentTimeMillis();
	}
	
	public long print() {
		long t = finish();
		System.out.printf("%s: %dms\n", name, t);
		return t;
	}

	public long finish() {
		return System.currentTimeMillis() - start;
	}

	public static Profile start(String name) {
		return new Profile(name);
	}

}
