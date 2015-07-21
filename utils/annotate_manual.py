import cv2
import sys

path = sys.argv[1]
ann_path = path.split('.')[0] + '.txt'

img = cv2.imread(path)
cv2.imshow("img", img)
key = raw_input()
fout = open(ann_path, 'w')
fout.write(key.upper());

