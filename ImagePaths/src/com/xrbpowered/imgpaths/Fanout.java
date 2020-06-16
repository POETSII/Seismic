package com.xrbpowered.imgpaths;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;

public abstract class Fanout {

	public abstract int getRadius();
	public abstract boolean hasEdge(int ri, int rj);
	public abstract int getMaxFanout();
	
	public List<Double> angles() {
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
		return angles;
	}
	
	public void printAngles() {
		for(Double a : angles())
			System.out.printf("%.2f\n", a);
	}
	
	public List<Double> gaps() {
		List<Double> angles = angles();
		ArrayList<Double> gaps = new ArrayList<>();
		for(int i=1; i<angles.size(); i++)
			gaps.add(angles.get(i)-angles.get(i-1));
		return gaps;
	}
	
	public double meanGap() {
		double sum = 0f;
		List<Double> gaps = gaps();
		for(Double a : gaps)
			sum += a;
		return sum/gaps.size();
	}

	public double maxGap() {
		double max = 0f;
		for(Double a : gaps())
			if(a>max) max = a;
		return max;
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
