# Generate samples in VEC format given background and list with annotated positives.
opencv_createsamples -bg bg_scaled.txt -info pos_scaled.txt -vec scaled.vec -num 926 -w 60 -h 24 
