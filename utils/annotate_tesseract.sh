for f in digits/*.jpg; do tesseract $f ${f%%.*} -psm 10; echo $f ${f%%.*}.txt; cat ${f%%.*}.txt; done
