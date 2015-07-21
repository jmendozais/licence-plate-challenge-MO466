# Rule to calc cascade classifier parameters
# http://answers.opencv.org/question/3085/why-always-opencv-error-assertion-failed-elements_read-1-in-unknown-function/

# For train partition
pos = 621
negs_imgs = 194
negs = 400
stages = 20
minhr = 0.995
upper_numPos = (pos - negs) * 1.0/(1 + stages*(1.0 - minhr))
print "On train:" + str(upper_numPos)

# For complete set
pos = 926
negs_imgs = 289
negs = 600
stages = 20
minhr = 0.995
upper_numPos = (pos - negs) * 1.0/(1 + stages*(1.0 - minhr))
print "On complete set:" + str(upper_numPos)

