rm -rf scaled
mkdir scaled
./scale train_bg.txt tr_bg_scaled.txt 640 0
./scale val_bg.txt val_bg_scaled.txt 640 0
./scale train_pos.txt tr_pos_scaled.txt 640 1
./scale val_pos.txt val_pos_scaled.txt 640 1
cat tr_bg_scaled.txt > bg_scaled.txt
cat val_bg_scaled.txt >> bg_scaled.txt
cat tr_pos_scaled.txt > pos_scaled.txt
cat val_pos_scaled.txt >> pos_scaled.txt
