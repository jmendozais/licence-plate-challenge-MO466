from shapely.geometry import Polygon
import os
import os.path as path
import numpy as np
import cv2
import sys
import scipy.stats as stats

TMP_PATH = "tmp.txt"

def _parse(line):
	tok = line.split(',')
	assert len(tok) == 9
	pts = []
	for i in range(4):
		pts.append((int(tok[2*i]), int(tok[2*i + 1])))
	roi = Polygon(pts)
	return roi, tok[8]

def _input():
	if len(sys.argv) < 3:
		print "Usage ./eval model_list data_dir"
		sys.exit()
	return sys.argv[1], sys.argv[2]

def show_real_predicted_rois(img, roi1, roi2):
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
	cv2.imshow('Real and predicted ROIs', img)
	cv2.waitKey()

def _copy_data(img_paths, out_dir):
	for img_path in img_paths:
		ann_path = img_path.split('.')[0] + ".txt"
		os.popen('cp {} {}/'.format(img_path, out_dir))
		os.popen('cp {} {}/'.format(ann_path, out_dir))

def evaluate_positives(data_dir, input_list, model_path):
	loc_score = []
	rec_score = []
	nones = []
	goods = []
	mids = []
	bads = []

	f = open(input_list)
	for line in f:
		img_path = path.join(data_dir, line.split(' ')[0])
		ann_path = img_path.split('.')[0] + ".txt"

		annotation_f = open(ann_path)
		lines = [line for line in annotation_f]
		assert len(lines) > 0
		roi1, str1 = _parse(lines[0])

		print "Annotation: {}".format(lines[0])

		os.popen('./lpdetect {} {} > {}'.format(img_path, model_path, TMP_PATH)).read()
		pred_f = open(TMP_PATH)
		lines = [line for line in pred_f]
		assert len(lines) > 0

		print "Prediction: {}".format(lines[0].replace('\n', ''))

		if lines[0].find('None') != -1:
			loc_score.append(0.0)
			rec_score.append(0.0)
			nones.append(img_path)
			continue

		roi2, str2 = _parse(lines[0])

		inter = roi1.intersection(roi2)
		overlap = inter.area / (roi1.area + roi2.area - inter.area)

		if overlap >= 0.7:
			loc_score.append(1.0)
			goods.append(img_path)
		elif overlap >= 0.5:
			loc_score.append(0.5)
			mids.append(img_path)
		else:
			loc_score.append(0.0)
			bads.append(img_path)

		if str1.strip() == str2.strip():
			rec_score.append(1.0)
		else:
			rec_score.append(0.0)

	return np.array(loc_score), np.array(rec_score), goods, mids, bads, nones

def evaluate_negatives(data_dir, input_list, model_path):
	loc_score = []
	rec_score = []

	f = open(input_list)
	for line in f:
		img_path = path.join(data_dir, line.replace('\n', ' ').split(' ')[0])
		ann_path = img_path.split('.')[0] + ".txt"
		annotation_f = open(ann_path)
		lines = [line for line in annotation_f]
		assert len(lines) > 0
		assert lines[0].find('None') != -1

		os.popen('./lpdetect {} {} > {}'.format(img_path, model_path, TMP_PATH)).read()

		pred_f = open(TMP_PATH)
		lines = [line for line in pred_f]
		assert len(lines) > 0

		score = 1.0 if lines[0].find('None') != -1 else 0.0

		loc_score.append(score)
		rec_score.append(score)

	return np.array(loc_score), np.array(rec_score)

def evaluate():
	model_path, data_dir = _input()
	model_home = model_path.split('.')[0] + "_out"
	pos_val = '{}_val_pos.txt'.format(data_dir)
	neg_val = '{}_val_bg.txt'.format(data_dir)
	loc_score = np.array([])
	rec_score = np.array([])

	# Create dirs to store results
	none_path = path.join(model_home, 'none')
	good_path = path.join(model_home, 'good')
	mid_path = path.join(model_home, 'mid')
	bad_path = path.join(model_home, 'bad')
	os.popen("rm -rf {0}; mkdir {0}".format(model_home))
	os.popen("mkdir {0}".format(none_path))
	os.popen("mkdir {0}".format(good_path))
	os.popen("mkdir {0}".format(mid_path))
	os.popen("mkdir {0}".format(bad_path))

	loc_score_p, rec_score_p, goods, mids, bads, nones = evaluate_positives(data_dir, \
	pos_val, model_path)
	_copy_data(goods, good_path)
	_copy_data(mids, mid_path)
	_copy_data(bads, bad_path)
	_copy_data(nones, none_path)

	loc_score_n, rec_score_n = evaluate_negatives(data_dir, neg_val, model_path)

	loc_score = np.append((loc_score_p, loc_score_n))
	rec_score = np.append((rec_score_p, rec_score_n))

	print "Total positives evaluated {}".format(len(loc_score_p))
	print "loc_score {}, rec_score {}".format(np.mean(loc_score_p), np.mean(rec_score_p))
	print "Negatives evaluated {}".format(len(loc_score_n))
	print "loc_score {}, rec_score {}".format(np.mean(loc_score_n), np.mean(rec_score_n))
	print "Total evaluated {}".format(len(loc_score))
	print "loc_score {}, rec_score {}".format(np.mean(loc_score), np.mean(rec_score))
	print "Score: {}".format(stats.hmean([np.mean(loc_score), np.mean(rec_score)]))

if __name__ == "__main__":
	evaluate()
