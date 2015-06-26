from PIL import Image
import numpy as np

'''
In an integral image each pixel is the sum of all pixels in the original image 
that are 'left and above' the pixel.

Original    Integral
+--------   +------------
| 1 2 3 .   | 0  0  0  0 .
| 4 5 6 .   | 0  1  3  6 .
| . . . .   | 0  5 12 21 .
            | . . . . . .

'''
import cv
from PIL import Image
class IntegralImage:

    def __init__(self, image, label):
        self.original = image
        self.sum = 0
        self.label = label
        self.calculate_integral()
        self.weight = 0
    
    def calculate_integral(self):
        assert len(self.original.shape) == 2
        self.integral = self.original.cumsum(0).cumsum(1)

    def get_area_sum(self, topLeft, bottomRight):
        # swap tuples
        topLeft = (topLeft[1], topLeft[0])
        bottomRight = (bottomRight[1], bottomRight[0])
        if topLeft == bottomRight:
            return self.integral[topLeft]
        topRight = (bottomRight[0], topLeft[1])
        bottomLeft = (topLeft[0], bottomRight[1])
        return self.integral[bottomRight] - self.integral[topRight] - self.integral[bottomLeft] + self.integral[topLeft] 
    
    def set_label(self, label):
        self.label = label
    
    def set_weight(self, weight):
        self.weight = weight
