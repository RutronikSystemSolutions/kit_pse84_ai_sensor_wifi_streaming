import numpy as np
import cv2

def process_camera_data(raw_data):
    WIDTH_PX = 320
    HEIGHT_PX = 240

    img = np.zeros([HEIGHT_PX, WIDTH_PX, 3], dtype=np.ubyte)

    # Extract
    for x in range(WIDTH_PX):
        for y in range(HEIGHT_PX):
            # img[y,x,]
            p = raw_data[y * WIDTH_PX + x]
            r = ((p >> 11) & 0x1f) << 3
            g = ((p >> 5) & 0x3f) << 2
            b = ((p >> 0) & 0x1f) << 3

            img[y,x,0] = b
            img[y,x,1] = g
            img[y,x,2] = r

    

    #cv2.rectangle(img, (0,0),(10,10),(0,255,0),2)
    #cv2.imshow('Raw', img)
    #cv2.waitKey(0)

    return img
