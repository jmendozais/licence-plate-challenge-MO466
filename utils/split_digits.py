from os import listdir
from os.path import isfile, join
from skimage import io
import numpy as np

def get_paths(root, suffix = 'jpg'):
    paths = []
    for doc in listdir(root):
        file_path = join(root, doc)
        if len(doc) > len(suffix):
            name_len = len(file_path)
            if (isfile(file_path) and (file_path[name_len - len(suffix):name_len] == suffix)):
               paths.append(join(root, doc))
    return paths

def save_list(paths, fname):
	fout = open(fname, 'w')
	for pt in paths:	
		fout.write(pt + '\n');
	fout.close()

data_path = 'digits'
pos_name = 'train.txt'
neg_name = 'test.txt'
split_param = 0.67

paths =  get_paths(data_path, 'jpg')
paths = np.array(paths);

idx = np.array(range(len(paths)))
np.random.shuffle(idx)

train = paths[idx < len(idx) * split_param]
test = paths[idx >= len(idx) * split_param]

save_list(train, 'digits_tr.txt');
save_list(test, 'digits_te.txt');
