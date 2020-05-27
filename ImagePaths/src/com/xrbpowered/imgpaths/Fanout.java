package com.xrbpowered.imgpaths;

import java.io.PrintStream;
import java.util.ArrayList;

public abstract class Fanout {

	public abstract int getRadius();
	public abstract boolean hasEdge(int ri, int rj);
	public abstract int getMaxFanout();
	
	public void printAngles() {
		int r = getRadius();
		ArrayList<Double> angles = new ArrayList<>();
		for(int rj=0; rj<=r; rj++)
			for(int ri=1; ri<=r; ri++) {
				if(hasEdge(ri, rj)) {
					double a = Math.atan2(rj, ri)*180.0/Math.PI;
					if(!angles.contains(a))
						angles.add(a);
				}
			}
		angles.sort(null);
		for(Double a : angles)
			System.out.printf("%.2f\n", a);
	}
	
	public void print(PrintStream out) {
		int r = getRadius();
		out.printf("%d %d\n", r, getMaxFanout());
		for(int rj=0; rj<=r; rj++) {
			for(int ri=0; ri<=r; ri++) {
				out.print(hasEdge(ri, rj) ? "1 " : "0 ");
			}
			out.println();
		}
	}
	
	public void print() {
		print(System.out);
	}
	
	public static class Full extends Fanout {
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
			return d*d-1;
		}
		
		@Override
		public boolean hasEdge(int ri, int rj) {
			return ri!=0 || rj!=0;
		}
	}
	
	public static class Bresenham extends Fanout {
		protected final int r;
		
		public Bresenham(int r) {
			this.r = r;
		}
		
		@Override
		public int getRadius() {
			return r;
		}
		
		@Override
		public int getMaxFanout() {
			return (2*r+1)*6-10;
		}
		
		@Override
		public boolean hasEdge(int ri, int rj) {
			return ri==1 || rj==1;
		}
	}
	
}
