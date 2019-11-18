package com.xrbpowered.imgpaths;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Scanner;

public class Solution {

	public class Node {
		public final int i, j, index;
		public double dist;
		public int parent;
		public boolean marked = false;
		
		public Node(int index) {
			this.index = index;
			this.i = net.indexi(index);
			this.j = net.indexj(index);
		}
	}
	
	public final GridNet net;
	//public ArrayList<Node> nodes = new ArrayList<>();
	public final Node[] nodes;
	
	public Solution(GridNet net) {
		this.net = net;
		this.nodes = new Node[net.w*net.h];
	}
	
	public Node addNode(int index, double dist, int parent) {
		Node n = new Node(index);
		n.dist = dist;
		n.parent = parent;
		nodes[index] = n;
		return n;
	}
	
	public void markPath(int index) {
		Node n = nodes[index];
		if(n.marked)
			return;
		n.marked = true;
		if(n.parent>=0)
			markPath(n.parent);
	}
	
	public void markPath(int i, int j) {
		markPath(net.nodeIndex(i, j));
	}
	
	public void print(PrintStream out, int prec) {
		out.printf("%d %d %d\n%d\n", net.w, net.h, net.step, nodes.length);
		String fmt = String.format("%%d %%d %%.%df\n", prec);
		for(Node n : nodes) {
			out.printf(fmt, n.index, n.parent, n.dist);
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
	
	public double sum() {
		double sum = 0;
		for(Node n : nodes) {
			sum += n.dist;
		}
		return sum;
	}
	
	public BufferedImage render(BufferedImage srcImg, boolean all) {
		BufferedImage img = new BufferedImage(srcImg.getWidth(), srcImg.getHeight(), BufferedImage.TYPE_INT_RGB);
		Graphics2D g2 = img.createGraphics();
		g2.drawImage(srcImg, 0, 0, null);
		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		g2.setColor(Color.RED);
		for(Node n : nodes) {
			int x = (int)net.nodeX(n.index);
			int y = (int)net.nodeY(n.index);
			if(n.parent<0) {
				g2.fillOval(x-3, y-3, 7, 7);
			}
			else if(all || n.marked && nodes[n.parent].marked) {
				int x1 = (int)net.nodeX(n.parent);
				int y1 = (int)net.nodeY(n.parent);
				g2.drawLine(x, y, x1, y1);
			}
		}
		
		return img;
	}

	public BufferedImage render(BufferedImage srcImg) {
		return render(srcImg, true);
	}
	
	public static Solution read(GridNet net, String path) {
		try {
			Scanner in = new Scanner(new File(path));
			int w = in.nextInt();
			int h = in.nextInt();
			int step = in.nextInt();
			if(net==null)
				net = new GridNet(w, h, step, null);
			Solution res = new Solution(net);
			
			int size = in.nextInt();
			for(int i=0; i<size; i++) {
				int index = in.nextInt();
				int parent = in.nextInt();
				double dist = in.nextDouble();
				res.addNode(index, dist, parent);
			}

			in.close();
			return res;
		} catch(IOException e) {
			e.printStackTrace();
			return null;
		}
	}

	public static Solution read(String path) {
		return read(null, path);
	}
	
	public static Solution readOutput(GridNet net, String path) {
		try {
			Scanner in = new Scanner(new File(path));
			net = new GridNet(net.w, net.h, net.step, null);
			Solution res = new Solution(net);
			
			while(in.hasNextLine()) {
				String line = in.nextLine();
				if(line.startsWith("Started"))
					break;
				else if(line.startsWith("Nodes:")) {
					int nodes = Integer.parseInt(line.split("\\:\\s*", 2)[1]);
					if(nodes!=net.numNodes) {
						in.close();
						throw new IOException("Num nodes mismatch");
					}
				}
			}
			
			for(int i=0; i<net.numNodes; i++) {
				int index = in.nextInt();
				int parent = in.nextInt();
				double dist = in.nextDouble();
				res.addNode(index, dist, parent);
			}

			in.close();
			return res;
		} catch(IOException e) {
			e.printStackTrace();
			return null;
		}
	}

}
