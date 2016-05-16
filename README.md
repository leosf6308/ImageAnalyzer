# ImageAnalyzer
Image analyzing tool for rocket plume.
It's on C++, don't blame me. For me, messing with C++ pointers is easier.

Lets you apply filters to objects, given an input image. Needs leosf6308/libpng and leosf6308/libjpeg to get PNGs/JPEGs contents.
Currently 4bit color quantization, border detection and pixel clustering are the available options.
Color quantization seeks for the closest color based on pixel's color, using 3D euclidean distance.
Border detection subtracts a pixel from all of it's 8 neighbours and returns an intensity. When color varies too much, intensity also varies with it.
Pixel clustering compares each pixel with it's top and left regions (given by pixel), and joins it to a region if depending on threshold.


TODO:
Find a way to identify object and rotation given a region (cluster).
    IDEA 1:
    Take region statistics by pixel disposition.
    Rotate objet so it gets to 0º rotation.
    Compare pixel count by bounding box area ratio.
    Use http://www.ijareeie.com/upload/june/48_2D%20GEOMETRIC.pdf idea (compare ratio to known ranges).
    IDEA 2:
    Take region statistics
    Check line distribution
    Take some statistics and compare to database.
A better and faster clustering algorithm should be developed.
