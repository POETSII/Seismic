package com.xrbpowered.imgpaths;

import java.awt.image.BufferedImage;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import javax.imageio.ImageIO;

public class ImagePaths {

	public static BufferedImage readImage(String path) {
		try {
			return ImageIO.read(new File(path));
		} catch(IOException e) {
			e.printStackTrace();
			return null;
		}
	}
	
	public static void writeImageBytes(BufferedImage img, String path) {
		try {
			DataOutputStream out = new DataOutputStream(new FileOutputStream(path));
			int w = img.getWidth();
			int h = img.getHeight();
			out.writeInt(w);
			out.writeInt(h);
			for(int y=0; y<h; y++) {
				for(int x=0; x<w; x++) {
					int v = img.getRGB(x, y)&0xff;
					out.writeByte(v);
				}
			}
			out.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
	}
	
	public static SourceData read(String path, double bias, double scale) {
		BufferedImage img = readImage(path);
		SourceData src = new SourceData(img);
		for(int y=0; y<src.h; y++) {
			for(int x=0; x<src.w; x++) {
				int rgb = img.getRGB(x, y);
				src.d[x][y] = (1.0-(rgb&0xff)/255.0)*scale+bias;
			}
		}
		
		return src;
	}
	
	public static void write(Solution res, BufferedImage srcImg, String path, boolean all) {
		try {
			BufferedImage img = res.render(srcImg, all);
			ImageIO.write(img, "PNG", new File(path));
		} catch(IOException e) {
			e.printStackTrace();
		}
	}

	public static void write(Solution res, BufferedImage srcImg, String path) {
		write(res, srcImg, path);
	}
}
