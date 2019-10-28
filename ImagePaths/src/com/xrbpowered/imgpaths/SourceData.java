package com.xrbpowered.imgpaths;

import java.awt.image.BufferedImage;

public class SourceData {

	public final BufferedImage img;
	public final int w, h;
	public double[][] d;
	
	public SourceData(BufferedImage img) {
		this.img = img;
		this.w = img.getWidth();
		this.h= img.getHeight();
		d = new double[w][h];
	}
	
	public double get(double x, double y) {
		int x0 = (int)Math.floor(x);
		double sx = x-x0;
		int y0 = (int)Math.floor(y);
		double sy = y-y0;
		double d00 = d[x0][y0];
		double d01 = d[x0][y0+1];
		double d10 = d[x0+1][y0];
		double d11 = d[x0+1][y0+1];
		return lerp(lerp(d00, d01, sy), lerp(d10, d11, sy), sx);
	}
	
	public double line(double x0, double y0, double x1, double y1) {
		double dx = x1-x0;
		double dy = y1-y0;
		double dist = Math.sqrt(dx*dx+dy*dy);
		int steps = (int)(dist*2.0);
		double sum = 0;
		for(int i=0; i<=steps; i++) {
			double s = i / (double)steps;
			double x = lerp(x0, x1, s);
			double y = lerp(y0, y1, s);
			sum += get(x, y);
		}
		return (sum / (double)steps) * dist;
	}
	
	public static double lerp(double x0, double x1, double s) {
		return x0 * (1.0-s) + x1 * s;
	}
}
