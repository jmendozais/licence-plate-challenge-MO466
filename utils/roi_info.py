import sys
from os import listdir
from os.path import isfile, join
import math

import numpy as np
import matplotlib.pyplot as plt

def _get_paths(root, suffix='jpg'):
	paths = []
	for doc in listdir(root):
		file_path = join(root, doc)
		if len(doc) > len(suffix):
			name_len = len(file_path)
			if isfile(file_path) and (file_path[name_len - len(suffix):name_len] == suffix):
				paths.append(join(root, doc))

	return paths

def _load(data_dir):
    paths = _get_paths(data_dir)
    X = []
    Y1 = np.full(2*4*len(paths), -1, dtype=np.int)
    Y1 = Y1.reshape((len(paths), 4, 2))
    Y2 = []
    for path in paths:
        X.append(path)
    for i in range(len(paths)):
        filename = paths[i].split('/')[-1]
        annotation_file = open(join(data_dir, filename.split('.')[0] + '.txt'), 'r')
        line = annotation_file.readlines()[0]
        if line.strip() == "None":
            Y2.append(line)
        else:
            toks = line.split(',')
            assert len(toks) == 9
            for j in range(4):
                Y1[i][j][0] = int(toks[2*j])
                Y1[i][j][1] = int(toks[2*j+1])
            Y2.append(toks[8])

    return np.array(X), np.array(Y1), Y2

def _angle_from_location(pts):
	assert len(pts) == 4

	offset = 0
	mins = pts[0][0] + pts[0][1]
	for i in range(4):
		if mins > (pts[i][0] + pts[i][1]):
			mins = (pts[i][0] + pts[i][1])
			offset = i

	slope = (pts[(offset + 1)%4][1] - pts[offset][1]) * 1.0 / \
	(pts[(offset + 1)%4][0] - pts[offset][0] + 1e-9)
	return math.atan(slope) * 180.0 / math.pi

def data_info(data_dir, verbose=False):
	path_imgs, locations, _ = _load(data_dir)
	width = []
	height = []
	angle = []
	abs_angle = []

	for i in range(len(locations)):
		[minx, miny] = locations[i][0]
		[maxx, maxy] = locations[i][0]
		for j in range(4):
			minx = min(minx, locations[i][j][0])
			miny = min(miny, locations[i][j][1])
			maxx = max(maxx, locations[i][j][0])
			maxy = max(maxy, locations[i][j][1])

		width.append(maxx - minx)
		height.append(maxy - miny)
		angle.append(_angle_from_location(locations[i]))
		abs_angle.append(math.fabs(angle[-1]))

		if verbose:
			print "{} ang: {}".format(path_imgs[i], angle[-1])

	print "Mean width: {}".format(np.mean(width))
	print "Mean height: {}".format(np.mean(height))
	print "Mean abs angle: {}".format(np.mean(abs_angle))

	return width, height, angle

def model_info(model_dir, bins, verbose=False):
	print "None"
	_, _, angle_none = data_info(join(model_dir, "none"), verbose)
	print "Total {}".format(len(angle_none))
	print "Good"
	_, _, angle_good = data_info(join(model_dir, "good"), verbose)
	print "Total {}".format(len(angle_good))
	print "Mid"
	_, _, angle_mid = data_info(join(model_dir, "mid"), verbose)
	print "Total {}".format(len(angle_mid))
	print "Bad"
	_, _, angle_bad = data_info(join(model_dir, "bad"), verbose)
	print "Total {}".format(len(angle_bad))
	if len(angle_none) > 3:
		plt.hist(angle_none, bins, alpha=.5, label="None")
	if len(angle_good) > 3:
		plt.hist(angle_good, bins, alpha=.5, label="Good")
	if len(angle_mid) > 3:
		plt.hist(angle_mid, bins, alpha=.5, label="Mid")
	if len(angle_bad) > 3:
		plt.hist(angle_bad, bins, alpha=.5, label="Bad")
	plt.legend()
	plt.show()

def _input():
	if len(sys.argv) < 3:
		print "Usage ./roi_info model_dir opt bins"
		print "opt : {model, data}"
		print "bins : number of bins of the hists"
		sys.exit()

	return sys.argv[1], sys.argv[2], int(sys.argv[3])

def main():
	model_dir, opt, bins = _input()
	if opt == 'model':
		model_info(model_dir, bins)
	else:
		_, _, angle = data_info(model_dir)
		plt.hist(angle, bins, alpha=.5)
		plt.show()

if __name__ == "__main__":
	main()

