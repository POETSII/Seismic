package com.xrbpowered.imgpaths.vol;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Random;

import javax.imageio.ImageIO;

public class GenNoise3D {

	public static final double multiplier = 2.5;
	
	public int w, l, h;
	public double[][][] data;
	
	private static Random random = new Random();
	
	public GenNoise3D(int w, int l, int h) {
		this.w = w;
		this.l = l;
		this.h = h;
		this.data = new double[w][l][h];
	}
	
	public GenNoise3D generate() {
		generate(System.currentTimeMillis());
		return this;
	}
	
	public GenNoise3D generate(long seed) {
		for(int x=0; x<w; x++)
			for(int y=0; y<l; y++)
				for(int z=0; z<h; z++) {
					double c = -1;
					int maxd = 5;
					for(int d=maxd; d>0; d--) {
						int dx = (x>>d)<<d;
						int dy = (y>>d)<<d;
						int dz = (z>>d)<<d;
						int j = 1<<d;
						int dx0 = (dx)%w;
						int dy0 = (dy)%l;
						int dz0 = (dz)%h;
						int dx1 = (dx+j)%w;
						int dy1 = (dy+j)%l;
						int dz1 = (dz+j)%h;
						random.setSeed(seedXYZ(seed+d, dx0, dy0, dz0));
						float c000 = random.nextFloat();
						random.setSeed(seedXYZ(seed+d, dx0, dy1, dz0));
						float c010 = random.nextFloat();
						random.setSeed(seedXYZ(seed+d, dx1, dy0, dz0));
						float c100 = random.nextFloat();
						random.setSeed(seedXYZ(seed+d, dx1, dy1, dz0));
						float c110 = random.nextFloat();
						random.setSeed(seedXYZ(seed+d, dx0, dy0, dz1));
						float c001 = random.nextFloat();
						random.setSeed(seedXYZ(seed+d, dx0, dy1, dz1));
						float c011 = random.nextFloat();
						random.setSeed(seedXYZ(seed+d, dx1, dy0, dz1));
						float c101 = random.nextFloat();
						random.setSeed(seedXYZ(seed+d, dx1, dy1, dz1));
						float c111 = random.nextFloat();
						float sx = -(dx-x) / (float)(1<<d);
						float sy = -(dy-y) / (float)(1<<d);
						float sz = -(dz-z) / (float)(1<<d);

						c += (multiplier / maxd) * lerp(
								lerp(lerp(c000, c010, sy), lerp(c100, c110, sy), sx),
								lerp(lerp(c001, c011, sy), lerp(c101, c111, sy), sx),
								sz);
					}
					if(c<0) c = 0;
					else if(c>1) c = 1;
					data[x][y][z] = c;
				}
		return this;
	}

	public BufferedImage imageCutYZ(int x) {
		BufferedImage img = new BufferedImage(l, h, BufferedImage.TYPE_INT_RGB);
		Graphics2D g = img.createGraphics();
		for(int y=0; y<l; y++)
			for(int z=0; z<h; z++) {
				float c = (float)data[x][y][z];
				g.setColor(new Color(c, c, c));
				g.fillRect(y, z, 1, 1);
			}
		return img;
	}

	public BufferedImage imageCutXZ(int y) {
		BufferedImage img = new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB);
		Graphics2D g = img.createGraphics();
		for(int x=0; x<w; x++)
			for(int z=0; z<h; z++) {
				float c = (float)data[x][y][z];
				g.setColor(new Color(c, c, c));
				g.fillRect(x, z, 1, 1);
			}
		return img;
	}

	public BufferedImage imageCutXY(int z) {
		BufferedImage img = new BufferedImage(w, l, BufferedImage.TYPE_INT_RGB);
		Graphics2D g = img.createGraphics();
		for(int x=0; x<w; x++)
			for(int y=0; y<l; y++) {
				float c = (float)data[x][y][z];
				g.setColor(new Color(c, c, c));
				g.fillRect(x, y, 1, 1);
			}
		return img;
	}
	
	public void writeImageCuts(String path, int x, int y, int z) {
		try {
			if(h>1) {
				ImageIO.write(imageCutYZ(x), "PNG", new File(String.format("%s_yz_at_x%d.png", path, x)));
				ImageIO.write(imageCutXZ(y), "PNG", new File(String.format("%s_xz_at_y%d.png", path, y)));
			}
			ImageIO.write(imageCutXY(z), "PNG", new File(String.format("%s_xy_at_z%d.png", path, z)));
		}
		catch(IOException e) {
			e.printStackTrace();
		}
	}
	
	public void writeBytes(String path) {
		try {
			DataOutputStream out = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(new File(path))));
			out.writeInt(w);
			out.writeInt(l);
			out.writeInt(h);
			for(int x=0; x<w; x++)
				for(int y=0; y<l; y++)
					for(int z=0; z<h; z++) {
						out.writeByte((int)(data[x][y][z]*255.0));
					}
			out.close();
		}
		catch(IOException e) {
			e.printStackTrace();
		}
	}
	
	private static double lerp(double x0, double x1, double s) {
		return x0 * (1.0-s) + x1 * s;
	}

	private static long nextSeed(long seed, long add) {
		// Multiply by Knuth's Random (Linear congruential generator) and add offset
		seed *= seed * 6364136223846793005L + 1442695040888963407L;
		seed += add;
		return seed;
	}

	private static long seedXYZ(long seed, long x, long y, long z) {
		seed = nextSeed(seed, x);
		seed = nextSeed(seed, y);
		seed = nextSeed(seed, z);
		seed = nextSeed(seed, x);
		seed = nextSeed(seed, y);
		seed = nextSeed(seed, z);
		return seed;
	}
	
}
