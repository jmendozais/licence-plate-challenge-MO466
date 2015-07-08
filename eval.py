from shapely.geometry import Point, Polygon
import os
import numpy as np
import cv2

def parse(line):
	tok = line.split(',')
	assert len(tok) == 9
	pts = []
	for i in range(4):
		pts.append((int(tok[2*i]), int(tok[2*i + 1])))
	roi1 = Polygon(pts)
	return roi1, tok[8]

data_path = 'train'
pos_val = 'val_pos.txt'
neg_val = 'val_bg.txt'

f = open(pos_val)

lc = []
rc = []
for line in f:
	img_path = line.split(' ')[0];
	ann_path = img_path.split('.')[0] + ".txt";
	tmp_path = 'tmp.txt'
	print img_path

	annotation_f = open(ann_path);
	lines = [line for line in annotation_f]
	assert (len(lines) > 0)
	print "Annotation: {}".format(lines[0])
	roi1, str1 = parse(lines[0])

	out = os.popen('./bin/lpdetect {} > {}'.format(img_path, tmp_path)).read()
	
	pred_f = open(tmp_path)
	lines = [line for line in pred_f]
	assert (len(lines) > 0)
	print "Detection: {}".format(lines[0])

	if lines[0].find('None') != -1:
		lc.append(0.0)				
		rc.append(0.0)
		os.popen('cp {} fails/{}'.format(img_path, img_path));
		continue

	roi2, str2 = parse(lines[0])
		
	'''
	img = cv2.imread(img_path)
	pts1 = list(roi1.exterior.coords)
	for i in range(4):
		pts1[i] = (int(pts1[i][0]), int(pts1[i][1]))
	for i in range(4):
		cv2.line(img, pts1[i], pts1[(i+1)%4], (255, 0, 0))

	pts2 = list(roi2.exterior.coords)
	for i in range(4):
		pts2[i] = (int(pts2[i][0]), int(pts2[i][1]))
	for i in range(4):
		pts2[i] = (int(pts2[i][0]), int(pts2[i][1]))
		cv2.line(img, pts2[i], pts2[(i+1)%4], (0, 255, 0))
	cv2.imshow('Evaluating', img)
	cv2.waitKey()
	'''

	inter = roi1.intersection(roi2)
	union_area = roi1.area + roi2.area - inter.area
	overlap = inter.area / union_area
	print "Overlap {}".format(overlap)

	if overlap >= 0.7:
		lc.append(1.0)
	elif overlap >= 0.5:
		lc.append(0.5)
	else:
		lc.append(0.0)	
	
	if str1.strip() == str2.strip():
		rc.append(1.0)
	else:
		rc.append(0.0)

print "Total positives evaluated {}".format(len(lc))
lc = np.array(lc)
rc = np.array(rc)
print "lc {}, rc {}".format(np.mean(lc), np.mean(rc))

print "TODO: eval negatives" 
