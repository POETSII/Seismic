package com.xrbpowered.imgpaths.scripts;

import com.xrbpowered.imgpaths.ForwardStar;
import com.xrbpowered.imgpaths.GridNet;
import com.xrbpowered.imgpaths.ImagePaths;
import com.xrbpowered.imgpaths.Profile;
import com.xrbpowered.imgpaths.ShortestPath;
import com.xrbpowered.imgpaths.Solution;
import com.xrbpowered.imgpaths.SourceData;

public class ProfileJava {

	public static void profileInput(int g, int r) {
		String path = String.format("../data/inputs/ints/L%dr%d.txt", g, r);
		GridNet net = GridNet.read(path);
		int root = net.nodeIndex(net.w/2, 2);
		Profile prof = Profile.start(path);
		Solution res_pc = new ShortestPath(net).calculate(root).solution();
		long time = prof.finish();
		System.out.printf("%d\t%d\t%d\t%d\t%d\t%.0f", g, r, net.numNodes, net.edges.size(), time, res_pc.sum());
		System.out.println();
	}
	
	public static SourceData data = null;
	
	public static void profileGeneratedInput(int g, int r) {
		if(data==null)
			data = ImagePaths.read("../data/img/in3.png", 0.25, 1);
		GridNet net = GridNet.create(data, g, new ForwardStar(r));
		int root = net.nodeIndex(net.w/2, 2);
		Profile prof = Profile.start(String.format("g%dr%d", g, r));
		Solution res_pc = new ShortestPath(net).calculate(root).solution();
		long time = prof.finish();
		System.out.printf("%d\t%d\t%d\t%d\t%d\t%.0f", g, r, net.numNodes, net.edges.size(), time, res_pc.sum());
		System.out.println();
	}
	
	public static void main(String[] args) {
		int[] gs = {1}; //{32, 16, 8, 4};
		int[] rs = {3, 5, 7};
		System.out.println("g\tr\tnodes\tedges\ttime\tsum");
		for(int g : gs)
			for(int r : rs)
				profileGeneratedInput(g, r);
	}

}
