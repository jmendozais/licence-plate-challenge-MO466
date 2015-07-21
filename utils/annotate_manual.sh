for f in digits/*.jpg; do echo $f; python utils/annotate_manual.py $f; done
