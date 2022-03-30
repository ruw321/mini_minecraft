Milestone 1:

Procedural Terrain
To achieve the realistic terrian effect. I tried different combinations of noise and tuned the parameters and seeds. I tested the noise effect in HW5 post-process shader.
The noise I tried:
1. FBM only
2. Worley Noise only
3. Perlin Noise only (including summed-perlin, recursive perlin)
4. FBM warped by FBM/Worley/Perlin
5. Worley warped by FBM/Worley/Perlin
6. Perlin warped by FBM/Worley/Perlin
7. Simplex Noise only (including summed-perlin)
8. Simplex Noise warped by FBM/Worley/Perlin

After tuning the parameter, I found the Simplex noise with Perlin noise warp had the best effect.

I also designed the API for the additional biomes for milestone 3.
I also implemented part of multi-thread of milestone 2 because I can not stand with the torture of slow rendering speed.