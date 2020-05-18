package com.xrbpowered.imgpaths;

import java.awt.image.BufferedImage;

public class SourceData {
	
	public static double lineStep = 1.0;

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
		int x1 = x0+1;
		if(x1>=w) x1 = x0;
		double sx = x-x0;
		int y0 = (int)Math.floor(y);
		int y1 = y0+1;
		if(y1>=h) y1 = y0;
		double sy = y-y0;
		double d00 = d[x0][y0];
		double d01 = d[x0][y1];
		double d10 = d[x1][y0];
		double d11 = d[x1][y1];
		return lerp(lerp(d00, d01, sy), lerp(d10, d11, sy), sx);
	}
	
	public double line(double x0, double y0, double x1, double y1) {
		double dx = x1-x0;
		double dy = y1-y0;
		double dist = Math.sqrt(dx*dx+dy*dy);
		int steps = (int)(dist*lineStep);
		double sum = 0;
		double prev = get(x0, y0);
		for(int i=1; i<=steps; i++) {
			double s = i / (double)steps;
			double x = lerp(x0, x1, s);
			double y = lerp(y0, y1, s);
			double v = get(x, y);
			sum += (prev+v)/2.0;
			prev = v;
		}
		return sum * dist / (double)steps;
	}
	
	public static double lerp(double x0, double x1, double s) {
		return x0 * (1.0-s) + x1 * s;
	}
}
