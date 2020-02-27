package com.xrbpowered.imgpaths.vol;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Scanner;

import com.xrbpowered.imgpaths.Net;

public class GridNet3D extends Net {

	public final int w, l, h, step;
	public final Fanout3D fanout;
	
	public GridNet3D(int w, int l, int h, int step, Fanout3D fanout) {
		super(w*l*h);
		this.w = w;
		this.l = l;
		this.h= h;
		this.step = step;
		this.fanout = fanout;
	}
	
	public void print(PrintStream out, int prec) {
		out.printf("%d %d %d %d ", w, l, h, step);
		fanout.print(out);
		out.print(edges.size());
		String fmt = String.format("%%.%df ", prec);
		int src = -1;
		for(Edge e : edges) {
			if(e.src!=src) {
				out.println();
				src = e.src;
			}
			out.printf(fmt, e.w);
		}
		out.println();
	}
	
	public int nodeIndex(int i, int j, int k) {
		return (k*l + j)*w + i;
	}
	
	public int indexi(int index) {
		return index%w;
	}
	
	public int indexj(int index) {
		return index/w%l;
	}

	public int indexk(int index) {
		return index/w/l;
	}

	public double itox(int i) {
		return i*step;
	}

	public double jtoy(int j) {
		return j*step;
	}

	public double ktoz(int k) {
		return k*step;
	}

	public double nodeX(int index) {
		return itox(indexi(index));
	}

	public double nodeY(int index) {
		return jtoy(indexj(index));
	}

	public double nodeZ(int index) {
		return ktoz(indexk(index));
	}

	public static GridNet3D create(SourceData3D src, int tile, int step, Fanout3D fanout) {
		GridNet3D net = new GridNet3D(src.w*tile/step, src.l*tile/step, src.h*tile/step, step/tile, fanout);
		
		int r = fanout.getRadius();
		int count = 0;
		int msgCount = 0;
		System.out.println("GridNet3D.create...");
		for(int sk=0; sk<net.h; sk++)
			for(int sj=0; sj<net.l; sj++)
				for(int si=0; si<net.w; si++) {
					int s = net.nodeIndex(si, sj, sk);
					double sx = net.itox(si);
					double sy = net.jtoy(sj);
					double sz = net.ktoz(sk);
					for(int rk=-r; rk<=r; rk++)
						for(int rj=-r; rj<=r; rj++)
							for(int ri=-r; ri<=r; ri++) {
								if(fanout.hasEdge(Math.abs(ri), Math.abs(rj), Math.abs(rk))) {
									int di = si+ri;
									int dj = sj+rj;
									int dk = sk+rk;
									if(di<0 || dj<0 || dk<0 || di>=net.w || dj>=net.l || dk>=net.h)
										continue;
									int d = net.nodeIndex(di, dj, dk);
									if(s<d) {
										double dx = net.itox(di);
										double dy = net.jtoy(dj);
										double dz = net.ktoz(dk);
										double x = src.line(sx, sy, sz, dx, dy, dz);
										net.addEdge(s, d, x);
										net.addEdge(d, s, x);
									}
								}
							}
					count++;
					if(count==1000) {
						msgCount++;
						System.out.printf("\r%d of %d", msgCount*1000, net.h*net.w*net.l);
						count = 0;
					}
				}
		
		System.out.println("\nCreated");
		return net;
	}
	
	public static GridNet3D read(String path) {
		try {
			Scanner in = new Scanner(new File(path));
			int w = in.nextInt();
			int l = in.nextInt();
			int h = in.nextInt();
			int step = in.nextInt();
			Fanout3D fanout = CustomFanout3D.read(in);
			in.nextInt();
			GridNet3D net = new GridNet3D(w, l, h, step, fanout);

			int r = fanout.getRadius();
			for(int sk=0; sk<net.h; sk++)
				for(int sj=0; sj<net.l; sj++)
					for(int si=0; si<net.w; si++) {
						int s = net.nodeIndex(si, sj, sk);
						for(int rk=-r; rk<=r; rk++)
							for(int rj=-r; rj<=r; rj++)
								for(int ri=-r; ri<=r; ri++) {
									if(fanout.hasEdge(Math.abs(ri), Math.abs(rj), Math.abs(rk))) {
										int di = si+ri;
										int dj = sj+rj;
										int dk = sk+rk;
										if(di<0 || dj<0 || dk<0 || di>=net.w || dj>=net.l || dk>=net.h)
											continue;
										int d = net.nodeIndex(di, dj, dk);
										net.addEdge(s, d, in.nextDouble());
									}
								}
					}

			in.close();
			return net;
		} catch(IOException e) {
			e.printStackTrace();
			return null;
		}
	}

}
