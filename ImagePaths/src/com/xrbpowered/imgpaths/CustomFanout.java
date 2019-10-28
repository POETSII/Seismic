package com.xrbpowered.imgpaths;

import java.io.IOException;
import java.util.Scanner;

public class CustomFanout extends Fanout {

	private final int r;

	private int total;
	private final boolean[][] e;

	public CustomFanout(int r) {
		this.r = r;
		this.e = new boolean[r+1][r+1];
		this.total = generate(r, e);
	}
	
	protected int generate(int r, boolean[][] e) {
		return 0;
	}
	
	@Override
	public int getRadius() {
		return r;
	}
	
	@Override
	public int getMaxFanout() {
		return total;
	}
	
	@Override
	public boolean hasEdge(int ri, int rj) {
		return e[ri][rj];
	}
	
	public static Fanout read(Scanner in) throws IOException {
		int r = in.nextInt();
		CustomFanout fanout = new CustomFanout(r); 
		
		int total = in.nextInt();
		for(int rj=0; rj<=r; rj++)
			for(int ri=0; ri<=r; ri++) {
				boolean a = in.nextInt() != 0;
				fanout.e[ri][rj] = a;
			}
		fanout.total = total;
		return fanout;
	}
	
}
