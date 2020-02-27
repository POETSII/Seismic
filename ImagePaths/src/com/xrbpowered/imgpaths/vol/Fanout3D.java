package com.xrbpowered.imgpaths.vol;

import java.io.PrintStream;

public abstract class Fanout3D {

	public abstract int getRadius();
	public abstract boolean hasEdge(int ri, int rj, int rk);
	public abstract int getMaxFanout();
	
	public void print(PrintStream out) {
		int r = getRadius();
		out.printf("%d %d\n", r, getMaxFanout());
		for(int rk=0; rk<=r; rk++) {
			out.println();
			for(int rj=0; rj<=r; rj++) {
				for(int ri=0; ri<=r; ri++) {
					out.print(hasEdge(ri, rj, rk) ? "1 " : "0 ");
				}
				out.println();
			}
		}
		out.println();
	}
	
	public void print() {
		print(System.out);
	}
	
	public static class Full extends Fanout3D {
		protected final int r;
		
		public Full(int r) {
			this.r = r;
		}
		
		@Override
		public int getRadius() {
			return r;
		}
		
		@Override
		public int getMaxFanout() {
			int d = 2*r+1;
			return d*d*d-1;
		}
		
		@Override
		public boolean hasEdge(int ri, int rj, int rk) {
			return ri!=0 || rj!=0 || rk!=0;
		}
	}
	
}
