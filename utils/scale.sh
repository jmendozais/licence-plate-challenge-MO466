scale_set() {
	data=$1
	major_side=$2
	rm -rf ${data}_scaled
	mkdir ${data}_scaled

	./scale ${data} ${data}_scaled ${data}_tr_bg.txt ${data}_tr_bg_scaled.txt $major_side 0
	./scale ${data} ${data}_scaled ${data}_val_bg.txt ${data}_val_bg_scaled.txt $major_side 0
	./scale ${data} ${data}_scaled ${data}_tr_pos.txt ${data}_tr_pos_scaled.txt $major_side 1
	./scale ${data} ${data}_scaled ${data}_val_pos.txt ${data}_val_pos_scaled.txt $major_side 1

	cat ${data}_tr_bg_scaled.txt > ${data}_bg_scaled.txt
	cat ${data}_val_bg_scaled.txt >> ${data}_bg_scaled.txt
	cat ${data}_tr_pos_scaled.txt > ${data}_pos_scaled.txt
	cat ${data}_val_pos_scaled.txt >> ${data}_pos_scaled.txt
}
scale_set $1 640
