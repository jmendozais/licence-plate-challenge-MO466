# On training
opencv_traincascade -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -numPos 400 -numNeg 600 -featureType HOG -bt GAB -w 60 -h 24

# opencv_traincascade -data hc_model -vec hc_scaled.vec -bg bg_scaled.txt -numPos 300 -numNeg 450 -featureType HOG -bt GAB -w 36 -h 24
