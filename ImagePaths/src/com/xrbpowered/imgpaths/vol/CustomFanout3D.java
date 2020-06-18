package com.xrbpowered.imgpaths.vol;

import java.io.IOException;
import java.util.Scanner;

public class CustomFanout3D extends Fanout3D {

	protected final int r;

	protected int total;
	protected final boolean[][][] e;

	public CustomFanout3D(int r) {
		this.r = r;
		this.e = new boolean[r+1][r+1][r+1];
		this.total = generate(r, e);
	}
	
	protected int generate(int r, boolean[][][] e) {
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
	public boolean hasEdge(int ri, int rj, int rk) {
		return e[ri][rj][rk];
	}
	
	public static Fanout3D read(Scanner in) throws IOException {
		int r = in.nextInt();
		CustomFanout3D fanout = new CustomFanout3D(r); 
		
		int total = in.nextInt();
		for(int rk=0; rk<=r; rk++)
			for(int rj=0; rj<=r; rj++)
				for(int ri=0; ri<=r; ri++) {
					boolean a = in.nextInt() != 0;
					fanout.e[ri][rj][rk] = a;
				}
		fanout.total = total;
		return fanout;
	}
	
}
