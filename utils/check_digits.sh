for f in digits/*.jpg; do cat ${f%.*}.txt; echo ""; open $f; read -p "press a key ..."; done
