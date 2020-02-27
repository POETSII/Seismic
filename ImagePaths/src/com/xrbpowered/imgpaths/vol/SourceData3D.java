package com.xrbpowered.imgpaths.vol;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class SourceData3D {

	public final int w, l, h;
	public double[][][] d;

	public SourceData3D(int w, int l, int h) {
		this.w = w;
		this.l = l;
		this.h = h;
		d = new double[w][l][h];
	}
	
	public double get(double x, double y, double z) {
		int x0 = (int)Math.floor(x);
		int x1 = (x0+1) % w;
		double sx = x-x0;
		int y0 = (int)Math.floor(y);
		int y1 = (y0+1) % l;
		double sy = y-y0;
		int z0 = (int)Math.floor(z);
		int z1 = (z0+1) % h;
		double sz = z-z0;
		double d000 = d[x0][y0][z0];
		double d010 = d[x0][y1][z0];
		double d100 = d[x1][y0][z0];
		double d110 = d[x1][y1][z0];
		double d001 = d[x0][y0][z1];
		double d011 = d[x0][y1][z1];
		double d101 = d[x1][y0][z1];
		double d111 = d[x1][y1][z1];
		return lerp(
				lerp(lerp(d000, d010, sy), lerp(d100, d110, sy), sx),
				lerp(lerp(d001, d011, sy), lerp(d101, d111, sy), sx),
				sz);
	}
	
	public double line(double x0, double y0, double z0, double x1, double y1, double z1) {
		double dx = x1-x0;
		double dy = y1-y0;
		double dz = z1-z0;
		double dist = Math.sqrt(dx*dx+dy*dy+dz*dz);
		int steps = (int)(dist*2.0);
		double sum = 0;
		for(int i=0; i<=steps; i++) {
			double s = i / (double)steps;
			double x = lerp(x0, x1, s);
			double y = lerp(y0, y1, s);
			double z = lerp(z0, z1, s);
			sum += get(x, y, z);
		}
		return (sum / (double)steps) * dist;
	}
	
	public static double lerp(double x0, double x1, double s) {
		return x0 * (1.0-s) + x1 * s;
	}
	
	public static SourceData3D readBytes(String path, double bias, double scale) {
		try {
			DataInputStream in = new DataInputStream(new BufferedInputStream(new FileInputStream(new File(path))));
			int w = in.readInt();
			int l = in.readInt();
			int h = in.readInt();
			SourceData3D src = new SourceData3D(w, l, h);
			for(int x=0; x<w; x++)
				for(int y=0; y<l; y++)
					for(int z=0; z<h; z++) {
						int rgb = in.readByte();
						src.d[x][y][z] = (1.0-(rgb&0xff)/255.0)*scale+bias;
					}
			in.close();
			return src;
		}
		catch(IOException e) {
			e.printStackTrace();
			return null;
		}
	}

}
