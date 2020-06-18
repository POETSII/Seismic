package com.xrbpowered.imgpaths.vol;

public class ForwardStar3D extends CustomFanout3D {

	public final boolean spherical;
	
	public ForwardStar3D(int r, boolean spherical) {
		super(r);
		this.spherical = spherical;
		this.total = generate(r, e);
	}

	@Override
	protected int generate(int r, boolean[][][] e) {
		for(int x=0; x<=r; x++)
			for(int y=0; y<=r; y++)
				for(int z=0; z<=r; z++) {
					e[x][y][z] = spherical ? (Math.ceil(Math.sqrt(x*x+y*y+z*z))<=r) : true;
				}
		for(int x=0; x<=r; x++)
			for(int y=0; y<=r; y++)
				for(int z=0; z<=r; z++) {
					for(int m=2; m<=r; m++)
						if(x*m<=r && y*m<=r && z*m<=r)
							e[x*m][y*m][z*m] = false;
				}
		e[0][0][0] = false;
		int count = 0;
		for(int x=0; x<=r; x++)
			for(int y=0; y<=r; y++)
				for(int z=0; z<=r; z++) {
					if(e[x][y][z]) {
						int n = 8;
						if(x==0) n >>= 1;
						if(y==0) n >>= 1;
						if(z==0) n >>= 1;
						count += n;
					}
				}
		return count;
	}
}
