package com.xrbpowered.imgpaths;

import java.util.ArrayList;

import com.xrbpowered.imgpaths.Net.Edge;

public class ShortestPath {

	public class Node {
		public final int index;
		
		public double dist = -1;
		public Node parent = null;
		public boolean processed = false;
		
		public ArrayList<Edge> edges = new ArrayList<>();
		
		public Node(int index) {
			this.index = index;
		}
	}
	
	public final Net net;
	public final Node[] nodes;
	
	public ShortestPath(Net net) {
		this.net = net;
		nodes = new Node[net.w*net.h];
		for(int j=0; j<net.h; j++)
			for(int i=0; i<net.w; i++) {
				int index = net.nodeIndex(i, j);
				nodes[index] = new Node(index);
			}
		for(Edge e : net.edges) {
			nodes[e.src].edges.add(e);
		}
	}
	
	public Solution calculate(int src) {
		nodes[src].dist = 0;
		for(int count=0; count<nodes.length; count++) {
			double min = -1;
			Node u = null;
			for(Node v : nodes) {
				if(!v.processed && v.dist>=0 && (min<0 || v.dist<min)) {
					min = v.dist;
					u = v;
				}
			}
			u.processed = true;
			for(Edge e : u.edges) {
				Node v = nodes[e.dst];
				if(!v.processed) {
					double dist = u.dist + e.w;
					if(v.dist<0 || dist<v.dist) {
						v.dist = dist;
						v.parent = u;
					}
				}
			}
		}
		
		Solution res = new Solution(net);
		for(Node n : nodes) {
			res.addNode(n.index, n.dist, n.parent==null ? -1 : n.parent.index);
		}
		return res;
	}
	
}
