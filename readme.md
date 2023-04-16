# Alpha Blending 

***mixing .bmp images using alpha blending method optimizing***

![](/images&font/alpha_blending_result.png?raw=true )

## What is alpha blending?

Alpha blending - is a method of mixing two pictures by using alpha component of foreground image as a mask.
There is a formula of mixed picture pixel color receiving: 
$$ mixed.color = foreground.color * foreground_alpha + background.color * (1 - foreground_alpha),$$
where foreground_alpha is a mask value which varies from 0 to 1.


## Stages

Every version follows the next structure: 
- receiving background and foreground images;
- making template for a mixed image consisting of background pixels;
- converting pixels in needed part of mixed picture;
- converting images in desired format (from BGRA (.bmp) to RGBA (SFML)) if needed;

### Naive version

Naive version ([naive version link](/alpha_blending_0.cpp)) works with sfml structure Image, addressing to it every time we need to get or set pixel and its color. That's why it is the slowest version, but the one where we can see alpha blending implementation most vividly.

### Ordinary version

The next version ([ordinary version link](/alpha_blending_1.cpp)) works with arrays of pixels so it spends less time for addressing. Now we need to convert the result to rgba which sfml requires.

### Optimized version

The last version ([optimized version link](/alpha_blending_2.cpp)) is logically like the previous one but accelerated with intrinsics usage in alpha blending calculation, which parallelling allows to treat 4 piels at the same time. 


**Note: only alpha blending calculations were optimized and only their work time was measured**

## Work time 

Some more notes:
- working fps was measured excluding rendering and calculated as 1/work_time;
- average time was measured with 10000 iterations in naive version and 100000 in others;
- all versions work with -o3 and -mavx2 flags and use SSE instructions;

| VERSION                   | FPS   | BOOST (%) |
| ------------------------- | ----- | --------- |
| naive                     | 0     |   -0      |
| not_optimized             | 1495  |    +0     |            
| optimized                 | 4415  |    +295   |

## Conclusion

Intrinsics including accelerates work in parts where we need to treat identically a lot of independent values at the same time. Practically, treating 4 pixels instead of 1 with XXM register treating (by SSE instructions for XMM - 128 bit values) we received acceleration in 3 times (comparing to not optimized). Obviously, we cannot get 4 time acceleration considering side processes.
Similarly, using avx-256 or avx-512 instructions would accelarate calculations in ~7 and ~15 times.