from os import listdir
from os.path import isfile, join

import numpy as np

def _get_paths(root, suffix='jpg'):
	paths = []
	for doc in listdir(root):
		file_path = join(root, doc)
		if len(doc) > len(suffix):
			name_len = len(file_path)
			if isfile(file_path) and (file_path[name_len - len(suffix):name_len] == suffix):
			   paths.append(join(root, doc))
	return paths

def _save_list(paths, fname, condition='negatives'):
	out_file = open(fname, 'w')
	if condition == 'positives':
		for path in paths:
			img_path = path.split('.')[0] + '.jpg'
			f = open(path, 'r')
			lines = [line for line in f]
			toks = lines[0].split(',')
			assert len(toks) == 9
			minx = 1e10
			miny = 1e10
			maxx = 1e-10
			maxy = 1e-10
			for i in range(8):
				if i%2 == 0:
					minx = min(minx, int(toks[i]))
					maxx = max(maxx, int(toks[i]))
				else:
					miny = min(miny, int(toks[i]))
					maxy = max(maxy, int(toks[i]))
			out_file.write('{} 1 {} {} {} {}\n'.format(img_path, minx, miny, \
			maxx - minx, maxy - miny))
	elif condition == 'negatives':
		for path in paths:
			out_file.write(path.split('.')[0] + '.jpg\n')

def main():
	data_path = 'train'
	pos_name = 'pos.txt'
	neg_name = 'bg.txt'
	pos_train = 'train_pos.txt'
	neg_train = 'train_bg.txt'
	pos_val = 'val_pos.txt'
	neg_val = 'val_bg.txt'
	split_param = 0.67

	paths = _get_paths(data_path, 'txt')
	neg_paths = []
	pos_paths = []
	for path in paths:
		f = open(path, 'r')
		lines = [line for line in f]
		assert len(lines) > 0

		if lines[0].find('None') != -1:
			neg_paths.append(path)
		else:
			pos_paths.append(path)

	neg_paths = np.array(neg_paths)
	pos_paths = np.array(pos_paths)
	n_negs = len(neg_paths)
	n_pos = len(pos_paths)
	print "{} {}".format(n_negs, n_pos)
	idx_pos = np.array(range(n_pos))
	idx_negs = np.array(range(n_negs))
	np.random.shuffle(idx_pos)
	np.random.shuffle(idx_negs)

	train_negs = neg_paths[idx_negs < n_negs * split_param]
	train_pos = pos_paths[idx_pos < n_pos * split_param]
	val_negs = neg_paths[idx_negs >= n_negs * split_param]
	val_pos = pos_paths[idx_pos >= n_pos * split_param]

	_save_list(pos_paths, pos_name, 'positives')
	_save_list(neg_paths, neg_name, 'negatives')
	_save_list(train_negs, neg_train, 'negatives')
	_save_list(train_pos, pos_train, 'positives')
	_save_list(val_negs, neg_val, 'negatives')
	_save_list(val_pos, pos_val, 'positives')

if __name__ == "__main__":
	main()
