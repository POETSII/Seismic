package com.xrbpowered.imgpaths;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.Random;

import com.xrbpowered.imgpaths.Net.Edge;

public class SimPoetsPath extends ShortestPath {

	public SimPoetsPath(Net net) {
		super(net);
	}

	@Override
	public Solution calculate(int src) {
		nodes[src].dist = 0;
		LinkedList<Node> snodes = new LinkedList<>();
		LinkedList<Edge> sedges = new LinkedList<>();
		for(;;) {
			boolean changed = false;
			shuffle(snodes, Arrays.asList(nodes));
			for(Node u : snodes) {
				if(u.dist<0)
					continue;
				shuffle(sedges, u.edges);
				for(Edge e : sedges) {
					Node v = nodes[e.dst];
					double dist = u.dist + e.w;
					if(v.dist<0 || dist<v.dist) {
						v.dist = dist;
						v.parent = u;
						changed = true;
					}
				}
			}
			if(!changed)
				break;
		}
		return createSolution();
	}
	
	public static Random random = new Random();
	
	public static <T> void shuffleIn(LinkedList<T> out, T obj) {
		if(random.nextBoolean())
			out.addFirst(obj);
		else
			out.addLast(obj);
	}
	
	public static <T> void shuffle(LinkedList<T> out, Iterable<T> src) {
		out.clear();
		for(T obj : src)
			shuffleIn(out, obj);
	}
}
