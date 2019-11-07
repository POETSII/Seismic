package com.xrbpowered.imgpaths;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.Random;
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
	
	public final int numNodes;
	public final ArrayList<Edge> edges = new ArrayList<>();
	
	public Net(int numNodes) {
		this.numNodes = numNodes;
	}
	
	public void print(PrintStream out, int prec) {
		out.println(numNodes);
		String fmt = String.format("%%d %%.%df ", prec);
		for(int src=0; src<numNodes; src++) {
			int numEdges = 0;
			for(Edge e : edges) {
				if(e.src==src)
					numEdges++;
			}
			out.printf("%d ", numEdges);
			for(Edge e : edges) {
				if(e.src==src)
					out.printf(fmt, e.dst, e.w);
			}
			out.println();
		}
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

	public Edge addEdge(int src, int dst, double w) {
		Edge edge = new Edge(src, dst);
		edge.w = w;
		edges.add(edge);
		return edge;
	}
	
	public static Net read(String path) {
		try {
			Scanner in = new Scanner(new File(path));
			int numNodes = in.nextInt();
			Net net = new Net(numNodes);

			for(int src=0; src<numNodes; src++) {
				int numEdges = in.nextInt();
				for(int e=0; e<numEdges; e++) {
					int dst = in.nextInt();
					double w = in.nextDouble();
					net.addEdge(src, dst, w);
				}
			}

			in.close();
			return net;
		} catch(IOException e) {
			e.printStackTrace();
			return null;
		}
	}
	
	public static Net random(int nnodes, int nedges, int minw, int maxw) {
		if(nedges > nnodes * (nnodes-1) / 2)
			throw new InvalidParameterException();
		
		Random random = new Random();
		Net g = new Net(nnodes);
		for(int i=0; i<nedges; i++) {
			int src, dst;
			do {
				src = random.nextInt(nnodes);
				dst = random.nextInt(nnodes);
			} while(src==dst);
			int w = random.nextInt(maxw-minw+1)+minw;
			g.addEdge(src, dst, w);
			g.addEdge(dst, src, w);
		}
		return g;
	}
}
