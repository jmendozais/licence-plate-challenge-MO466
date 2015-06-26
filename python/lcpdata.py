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

def extend_idx(idx, factor):
    ey = []
    for val in idx:
        ey.extend(range(val*factor, (val+1)*factor))
    return ey

# Dataset
def load(data_dir): 
    ''' 
    Args:
        data_dir: Path to *.txt and *.jpg directory

    Returns:
        X: Images
        Y1: nx8x2 array with the coordinates of the license plate
        Y2: License plate string

    '''
    
    paths = get_paths(data_dir)
    X = []
    Y1 = np.full(2*4*len(paths), -1, dtype=np.int)
    Y1 = Y1.reshape((len(paths), 4, 2))
    Y2 = []
    for path in paths:
        X.append(io.imread(path))
    for i in range(len(paths)):
        filename = paths[i].split('/')[-1]
        annotation_file = open(join(data_dir, filename.split('.')[0] + '.txt'), 'r')
        line = annotation_file.readlines()[0]
        if line[:4] == 'None':
            Y2.append(line)
        else:
            toks = line.split(',')
            assert len(toks) == 9
            for j in range(4):
                Y1[i][j][0] = int(toks[2*j])
                Y1[i][j][1] = int(toks[2*j+1])
            Y2.append(toks[8])
    return np.array(X), np.array(Y1), Y2

