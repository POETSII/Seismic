package com.xrbpowered.imgpaths;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Scanner;

public class Net {

	public class Edge {
		public final int src, dst;
		public double w;
		
		public Edge(int src, int dst) {
			this.src = src;
			this.dst = dst;
		}
	}
	
	public final int w, h, step;
	public final Fanout fanout;
	public final ArrayList<Edge> edges = new ArrayList<>();
	
	public Net(int w, int h, int step, Fanout fanout) {
		this.w = w;
		this.h= h;
		this.step = step;
		this.fanout = fanout;
	}
	
	public void print(PrintStream out, int prec) {
		out.printf("%d %d %d ", w, h, step);
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
	
	public void print(int prec) {
		print(System.out, prec);
	}
	
	public void write(String path, int prec) {
		try {
			PrintStream out = new PrintStream(new File(path));
			print(out, prec);
			out.close();
		} catch(FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	public void printEdges(PrintStream out, int prec) {
		out.println(edges.size());
		String fmt = String.format("%%d %%d %%.%df\n", prec);
		for(Edge e : edges) {
			out.printf(fmt, e.src, e.dst, e.w);
		}
	}
	
	public void printEdges(int prec) {
		printEdges(System.out, prec);
	}
	
	public void writeEdges(String path, int prec) {
		try {
			PrintStream out = new PrintStream(new File(path));
			printEdges(out, prec);
			out.close();
		} catch(FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	public Edge addEdge(int src, int dst, double w) {
		Edge edge = new Edge(src, dst);
		edge.w = w;
		edges.add(edge);
		return edge;
	}
	
	public int nodeIndex(int i, int j) {
		return j*w+i;
	}
	
	public int indexi(int index) {
		return index%w;
	}
	
	public int indexj(int index) {
		return index/w;
	}
	
	public double itox(int i) {
		return i*step + step/2.0;
	}

	public double jtoy(int j) {
		return j*step + step/2.0;
	}
	
	public double nodeX(int index) {
		return itox(indexi(index));
	}

	public double nodeY(int index) {
		return jtoy(indexj(index));
	}

	public static Net create(SourceData src, int step, Fanout fanout) {
		Net net = new Net(src.w/step, src.h/step, step, fanout);
		
		int r = fanout.getRadius();
		for(int sj=0; sj<net.h; sj++)
			for(int si=0; si<net.w; si++) {
				int s = net.nodeIndex(si, sj);
				double sx = net.itox(si);
				double sy = net.jtoy(sj);
				for(int rj=-r; rj<=r; rj++)
					for(int ri=-r; ri<=r; ri++) {
						if(fanout.hasEdge(Math.abs(ri), Math.abs(rj))) {
							int di = si+ri;
							int dj = sj+rj;
							if(di<0 || dj<0 || di>=net.w || dj>=net.h)
								continue;
							int d = net.nodeIndex(di, dj);
							double dx = net.itox(di);
							double dy = net.jtoy(dj);
							net.addEdge(s, d, src.line(sx, sy, dx, dy));
						}
					}
			}
		
		return net;
	}
	
	public static Net read(String path) {
		try {
			Scanner in = new Scanner(new File(path));
			int w = in.nextInt();
			int h = in.nextInt();
			int step = in.nextInt();
			Fanout fanout = CustomFanout.read(in);
			in.nextInt();
			Net net = new Net(w, h, step, fanout);

			int r = fanout.getRadius();
			for(int sj=0; sj<net.h; sj++)
				for(int si=0; si<net.w; si++) {
					int s = net.nodeIndex(si, sj);
					for(int rj=-r; rj<=r; rj++)
						for(int ri=-r; ri<=r; ri++) {
							if(fanout.hasEdge(Math.abs(ri), Math.abs(rj))) {
								int di = si+ri;
								int dj = sj+rj;
								if(di<0 || dj<0 || di>=net.w || dj>=net.h)
									continue;
								int d = net.nodeIndex(di, dj);
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
