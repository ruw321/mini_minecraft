# Milestone 3:

[Milestone 3 demonstration](https://www.youtube.com/watch?v=YDFq1TONUFo)

## Additional Biomes, Procedurally placed assets, Water waves and Distance fog (Zongxin Cui)
### Aditional Biomes
 - initially struggling to get smooth transition betwween different biomes, then adjust the frequency of biome noise function and linear interpolation to fix it
 - then i encountered that the height field for grassland and desert are approximatelly the same. i fixed it by generate another two different terrian noise function and double interpolation among them to better distinguish.

### Procedurally placed assets
 - this process is pretty smooth and fun. I experiement with a lot of object and find different texture map online to try to make differnt fun assets.
 - interesting part is makeing lilypads on the water and flaming fire in the mountain. I took inspiration from the water wave to make fire moving.

### Water waves
 - Using time variant trigonometry function to make water waves, referencing Adam's lecture
 - Using noise funtion to create variant water sureface normal, and then using water surface normal and sunlight directions to apply Blinn-Phong effect and making shining water surface
 
### Distance Fog
 - Linear interpolating between each blocks original color and current suncolor based on block's distance to camera to create fog effect that is also relating to the day and night cycle
 - Referencing Adam's lecture

## Day and Night (Zhiyuan Liang)
I followed Adam's reference code of sky implementation.

- Sky:

The moving cloud is implemented by the WorleyFBM noise. The cloud(sky) color will be affected by the ambient(sun color). The moving cloud will not be drawn during night time.

I set a time cycle where I use a variable which changes periodic to represent the time. The value is ranged from -1 to 1. When the time is negative, it means that it is night now. When the time is positive, it means that it is daytime now. The sun will move as time passing. The sunlight will be calculated according to the sun position.

I hardcoded 4 time slots: night, day sunset and sunrise. Each time slot will use a color palette and the color will be interploted according to the time. 

I also added a feature of stars when it is night. I used some pepper noise to simulate the stars in the night sky.

- Lighting:

The velocity of sun's movement is fixed. So we can know the sun position at any time. I set the light direction as the real-timed sun direction in the lambert.frag.glsl. I changed lambert reflection model to the Blinn-Phong model. I use the real-timed sun light color as ambient term.


## Sound, Inventory GUI and Post-process Camera Overlay (Ruifan Wang)
- Sound:

I followed this tutorial to add sounds to the game:
https://www.youtube.com/watch?v=DS7CDpIrwN4

I also downloaded the sounds from freesound.com

I had problem with QSound class not being found, even after I modified the .pro file, it still couldn't be found. I then used QMediaPlayer to get around it, but then setMedia function for this class was not found either. 

I found out that it is the version problem, the new version 6.2 doesn't have that class so I instead started using the sound effect class. 

Then I had a problem with the sound being laggy and played with electric noise, later I found out that it was the problem of the sound file because other sounds played just fine.

However the sound still didn???t turn out to be too good, the wind wav file is still lagging, and when you enter the water, there is a static noise before it starts playing the actual water sound effect. I went to the Professor's office hours, but they couldn???t help much.  


- Inventory GUI:

I made inventory as a separate window, but it works just like an overlay so the user experience is the same. I started off with 10 blocks of each block type in the inventory, and players can gain more blocks by destroying the corresponding blocks, the number of blocks left in inventory is also shown. The player is not able to place any blocks if they don???t have anything left. 

For this, I had to modify the place block and remove block from the player class. Before the place block was a little buggy, it sometimes replaces a block. Now it will only place a block on top of a block or next to an existing one. It took me some time to make the GUI, and the difficult part was that there is so many things that you have to connect in order for a button from the GUI to work. So I spent a lot of time to debug and find where the missing connection is. 


- Post-process Camera Overlay:

I applied screen distortion. I basically used the same thing I used from homework 4, it is in the post.frag.glsl, it is a fraction of Professor Adam's code that was shown during homework 4. 


# Milestone 2:
[Milestone 2 demonstration](https://www.youtube.com/watch?v=hBTctWhnhlQ&t=14s)
## Texturing and Texture Animation (Zongxin Cui)
- Texture
   - Initially having trouble figure out how to load texture file, but evetually figured out
   - Constructed the VBO texture data in a hard coded way, soon change to using unordered map and set to store and retrive data
   - The normal texture did not worked as good as expected, but I added block face direction to the normal vector then normalize the normal vector to increase the contrast of different block face
- Texture Animation
   - Initially struggling to get the right changing offset for the uv to make runing water and lava, but later figure it out
- Currently working on fog effect

## Multi-Thread (Zhiyuan Liang)
I used to implement multi-thread in a wrong way. But my teammate told me that Adam said our multi-thread method was wrong. Then I checked the example code from the lecture. I re-implemented the multi-thread and only called thread.join when the application is going to end.
I used newChunks, BlockTypeBuffer and VBOdataBuffer to temporarily store the created chunks' uPtr. I used to two mutexes to make sure that the modifications to these buffers are atomic.

## Cave Systems (Ruifan Wang)
Cave: I initially tried the 3D perlin noises (summed and recursive version) but the cave turned out to be not really realistic. So I decided to use the 2D perlin noise to generated cave cell and cave floor, which makes the cave seem a lot more realistic.

Post shader: I utilized the provided framebuffer code. At first, we want to used two textures and I figured out how to set the texture slot id. After I implemented the framebuffer, the scene did not display normally. The scene was painted only in the left-bottom corner. I debugged for quite a while and multily the width and height of framebuffer by devicePixelRatio, then it displayed correctly. I also added a cool underwater-view effect.

Physics under water and lava: I first had trouble with detecting the water with either the camera perspective or the player perspective, and when to switch on the underwater-view effect. EVentually I used the camera perspective. The physics applied under water was also very tricky, especially the intersection between water and outside world, when to apply real gravity and when to apply water gravity, I had to make sure the player was able to get out of the water as well. I also fixed the physics when the player is not in flight mode, now the player will move in constant speed in stead of increasing acceleration. 


# Milestone 1: 

[Milestone 1 demonstration](https://www.youtube.com/watch?v=VI6Wz9TMFQs)

## Procedural Terrain (Zhiyuan Liang)

To achieve the realistic terrain effect. I tried different combinations of noise and tuned the parameters and seeds. I tested the noise effect in HW5 post-process shader.
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
I also implemented part of multi-thread of milestone 2 because I can not stand the torture of slow rendering speed.

## Game Engine Tick Function and Player Physics (Ruifan Wang)

- Jumping
   - to avoid click space before it lands, I had to add another global variable to inputs to store whether it is on the ground or not
   - to avoid holding space and flying up, I added a condition that checks if I am on the ground or not before jumping
- Collision detection
   - initially, I implemented the way that the player's movement will come to a complete halt when it collides
   - eventually, I did the same thing but one axis at a time to allow smoother collisions such as walking along the wall
- Placing block
   - initially, I made it that it could place a block only if the space was empty, but I had a hard time placing blocks this way
   - then I realized that I should check if gridMarch returns true, and then players wouldn't be able to place blocks in the air
- Velocity and acceleration
   - just the overall tuning of the velocity and acceleration is definitely a pain in the butt

## Efficient Terrain Rendering and Chunking (Zongxin Cui)
- Chunk Class (I model the majority of this part after HW5-6-7)
  - reorganize the basic element of the terrain from blocks to chunks, which is a 16x16x256 block bundle. 
  - I also change the VBO data into interleave style, eg:poscolnormposcolnorm, to speed up the transmission between CPU and GPU
  - organize the block information, eg: pos, UV, normal, collectively to improve future debugging
  - implemented a specific render process for interleaving VBO in the shader program
- Terrain Generation
  - the terrain is now generated by the zone unit, a 4x4 chunk bundle
  - a zone is only being generated if the player's position to the current generated terrain edge is less than 16 blocks
- Currently, I am working on the texture component of Milestone 2



    


    
