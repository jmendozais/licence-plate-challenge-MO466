run_cascade()
{
	model_name=$1
	feat_type=$2
	boost_type=$3

	rm -rf $model_name
	mkdir $model_name
	opencv_traincascade -data $m/del_name -vec tr_scaled.vec -bg tr_bg_scaled.txt -numPos 400 -numNeg 600 -featureType $feat_type -mode CORE -bt $boost_type -w 60 -h 24
}
# train_set  86/649, val_set 25/252 - opencv_haartraining -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -npos 140 -nneg 500 -nstages 10 -mode CORE -bt RAB
# train_set 166/649, val_set 31/252 opencv_haartraining -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -npos 140 -nneg 500 -nstages 10 -mode CORE -bt GAB
# train_set 333/649, val_set 76/278 opencv_haartraining -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -npos 135 -nneg 500 -nstages 10 -mode CORE -bt GAB -w 48 -h 24
# margin 0.2: train_set 395/649, val_set opencv_haartraining -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -npos 135 -nneg 500 -nstages 10 -mode CORE -bt GAB -w 48 -h 24
# opencv_haartraining -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -npos 135 -nneg 500 -nstages 7 -mode CORE -bt GAB -w 60 -h 24

ftypes=(HAAR HOG LBP)
btypes=(GAB RAB LB)

mkdir models
for feat in "${ftypes[@]}"
do 
	(
	for boost in "${btypes[@]}"
	do
		model_name=models/${feat}_${boost}
		run_cascade $model_name $feat $boost
	done
	)
done
	
# opencv_traincascade -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -numPos 400 -numNeg 600 -featureType HAAR -mode CORE -bt GAB -w 60 -h 24
# opencv_traincascade -data model -vec tr_scaled.vec -bg tr_bg_scaled.txt -numPos 400 -numNeg 600 -featureType HOG -bt GAB -w 60 -h 24
