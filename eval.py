from shapely.geometry import Point, Polygon
import os
import os.path as path
import numpy as np
import cv2
import sys

def parse(line):
	tok = line.split(',')
	assert len(tok) == 9
	pts = []
	for i in range(4):
		pts.append((int(tok[2*i]), int(tok[2*i + 1])))
	roi1 = Polygon(pts)
	return roi1, tok[8]

def input():
	if len(sys.argv) < 4:
		print "Usage ./eval model_list data_dir opt"
		print "opt : {model, data}"
		sys.exit()
	return sys.argv[1], sys.argv[2], sys.argv[3]

model_path, data_dir, opt = input()

model_home = model_path.split('.')[0] + "_out";
pos_val = '{}_val_pos.txt'.format(data_dir)
neg_val = '{}_val_bg.txt'.format(data_dir)

none_path = path.join(model_home, 'none')
good_path = path.join(model_home, 'good')
mid_path = path.join(model_home, 'mid')
bad_path = path.join(model_home, 'bad')

os.popen("rm -rf {0}; mkdir {0}".format(model_home))
os.popen("mkdir {0}".format(none_path))
os.popen("mkdir {0}".format(good_path))
os.popen("mkdir {0}".format(mid_path))
os.popen("mkdir {0}".format(bad_path))

lc = []
rc = []
f = open(pos_val)
for line in f:
	img_path = path.join(data_dir, line.split(' ')[0]);
	ann_path = img_path.split('.')[0] + ".txt";
	tmp_path = 'tmp.txt'
	print img_path

	annotation_f = open(ann_path);
	lines = [line for line in annotation_f]
	assert len(lines) > 0
	print "Annotation: {}".format(lines[0])
	roi1, str1 = parse(lines[0])

	out = os.popen('./lpdetect {} {} > {}'.format(img_path, model_path, tmp_path)).read()
	
	pred_f = open(tmp_path)
	lines = [line for line in pred_f]
	assert (len(lines) > 0)
	print "Detection: {}".format(lines[0])

	if lines[0].find('None') != -1:
		lc.append(0.0)				
		rc.append(0.0)
		os.popen('cp {} {}/'.format(img_path, none_path));
		os.popen('cp {} {}/'.format(ann_path, none_path));
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
		os.popen('cp {} {}/'.format(img_path, good_path));
		os.popen('cp {} {}/'.format(ann_path, good_path));
	elif overlap >= 0.5:
		lc.append(0.5)
		os.popen('cp {} {}/'.format(img_path, mid_path));
		os.popen('cp {} {}/'.format(ann_path, mid_path));
	else:
		lc.append(0.0)	
		os.popen('cp {} {}/'.format(img_path, bad_path));
		os.popen('cp {} {}/'.format(ann_path, bad_path));
	
	if str1.strip() == str2.strip():
		rc.append(1.0)
	else:
		rc.append(0.0)

print "Total positives evaluated {}".format(len(lc))
lc = np.array(lc)
rc = np.array(rc)
print "lc {}, rc {}".format(np.mean(lc), np.mean(rc))

print "TODO: eval negatives" 
