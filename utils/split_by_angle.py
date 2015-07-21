from os import listdir
from os.path import isfile, join
from skimage import io
import numpy as np
import math
import os
import sys

def _get_paths(root, suffix = 'jpg'):
	paths = []
	for doc in listdir(root):
		file_path = join(root, doc)
		if len(doc) > len(suffix):
			name_len = len(file_path)
			if (isfile(file_path) and (file_path[name_len - len(suffix):name_len] == suffix)):                
				paths.append(join(root, doc))

	return paths

def _load(paths):
    Y1 = np.full(2*4*len(paths), -1, dtype=np.int)
    Y1 = Y1.reshape((len(paths), 4, 2))
    Y2 = []

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

    return np.array(paths), np.array(Y1), Y2

def _angle_from_location(pts):
	assert len(pts) == 4

	offset = 0
	mins = pts[0][0] + pts[0][1]
	for i in range(4):
		if mins > (pts[i][0] + pts[i][1]):
			mins = (pts[i][0] + pts[i][1])
			offset = i

	m = (pts[(offset + 1)%4][1] - pts[offset][1]) * 1.0 / (pts[(offset + 1)%4][0] - pts[offset][0] + 1e-9)

	return math.atan(m) * 180.0 / math.pi

def _create_list(paths, list_path):
	assert len(paths) > 0

	f = open(list_path, 'w')
	for path in paths:
		f.write(path)	
	f.close()
	
def split_by_angle(data_dir, img_list, angle = 10):
	list_f = open(img_list)
	paths = []
	lines = [line for line in list_f]
	
	for line in lines:
		toks = line.split(' ')
		paths.append(toks[0])
	
	paths, locations, licenses = _load(paths)			

	splits = [[],[],[]]

	for i in range(len(locations)):
		angi = _angle_from_location(locations[i])
		if angi < -angle:
			splits[0].append(lines[i])	
		elif angi >= angle:
			splits[2].append(lines[i])	
		else:
			splits[1].append(lines[i])

	list_prefix = img_list.split('.')[0]
	out_list = '{}_l{}.txt'.format(list_prefix, angle)	
	_create_list(splits[0], out_list)
	out_list = '{}_m{}.txt'.format(list_prefix, angle)	
	_create_list(splits[1], out_list)
	out_list = '{}_r{}.txt'.format(list_prefix, angle)	
	_create_list(splits[2], out_list)

def _input():
	if len(sys.argv) < 3:
		print "Usage ./split_by_angle data_dir img_list angle"
		print "Data is divided in three sets: train_l<angle>, train_m<angle>, train_r<angle>"
		print "with plates with angles x < -<angle>, -<angle> < x < <angle>, <angle> < x respectively"
		sys.exit()
	
	return sys.argv[1], sys.argv[2], int(sys.argv[3])

def main():
	data_dir, img_list, angle = _input()
	split_by_angle(data_dir, img_list, angle)

if __name__ == "__main__":
	main()
