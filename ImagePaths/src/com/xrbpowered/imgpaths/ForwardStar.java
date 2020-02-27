package com.xrbpowered.imgpaths;

public class ForwardStar extends CustomFanout {

	public ForwardStar(int r) {
		super(r);
	}

	@Override
	protected int generate(int r, boolean[][] e) {
		e[0][0] = false;
		int count = 0;
		for(int x=1; x<=r; x++) {
			for(int i=0; i<x; i++) {
				boolean a = accept(x, i);
				if(a) count++;
				e[x][i] = a;
				e[i][x] = a;
				if(i==0)
					e[x][x] = a;
			}
		}
		return count*8;
		/*for(int x=r; x>=0; x--)
			for(int y=r; y>=0; y--) {
				e[x][y] = true;
				for(int m=2; m<=r; m++)
					if(x*m<=r && y*m<=r)
						e[x*m][y*m] = false;
			}
		e[0][0] = false;
		int count = 0;
		for(int x=0; x<=r; x++)
			for(int y=0; y<=r; y++) {
				if(e[x][y]) {
					int n = 4;
					if(x==0) n >>= 1;
					if(y==0) n >>= 1;
					count += n;
				}
			}
		return count;*/
	}
	
	public static int[] primes = calcPrimes(30);
	
	public static int[] calcPrimes(int count) {
		int[] primes = new int[count];
		int i = 0;
		for(int n=2; ; n++) {
			boolean prime = true;
			for(int m=0; m<i; m++)
				if(n%primes[m]==0) {
					prime = false;
					break;
				}
			if(prime) {
				primes[i] = n;
				i++;
				if(i==count)
					return primes;
			}
		}
	}
	
	private static boolean accept(int x, int n) {
		if(n==0)
			return x<2;
		for(int pi=0; primes[pi]<x; pi++) {
			int p = primes[pi];
			if(x%p==0 && n%p==0)
				return false;
		}
		return true;
	}
	
}
