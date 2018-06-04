# Train classification model from multiple sources of data
# A source is a directory. Each source should contain at least one train file begins with "train.x" and its corresponding test file begins with "test.x"
# The first source must include "train.y" and "test.y" for labels of train and test set
# All "train.*" files should have equal number of rows
# All "test.*" files should have equal number of rows

import sys
import numpy as np
import pandas as pd
import os
#import matplotlib.pyplot as plt
#from patsy import dmatrices

#from sklearn.cross_validation import train_test_split
from sklearn import metrics
#from sklearn.cross_validation import cross_val_score
from sklearn.linear_model import LogisticRegression






work_dir = sys.argv[1]




#def logreg():
#	train_y = [int(l) for l in open(work_dir + "/train.y","r").read().split(' ')]
#	test_y = [int(l) for l in open(work_dir + "/test.y","r").read().split(' ')]
#	train_x = pd.read_csv(work_dir + "/train.x", header=None, delim_whitespace = True)
#	test_x = pd.read_csv(work_dir + "/test.x", header=None, delim_whitespace = True)
#
#	logreg = LogisticRegression().fit(train_x, train_y)
#	predicted = logreg.predict(test_x)
#	print("Error rate with logreg: %10.8f" % (1.0 - metrics.accuracy_score(test_y, predicted)))

#def multi_logreg():
#	train_y = [int(l) for l in open(work_dir + "/train.y","r").read().split(' ')]
#	test_y = [int(l) for l in open(work_dir + "/test.y","r").read().split(' ')]
	#highest_proba = np.zeros(len(test_y))
	#final_predict = np.zeros(len(test_y))
	#lb = train_y[0]
	
	#a = pd.read_csv(work_dir + "/train.x." + str(lb), header=None, delim_whitespace = True)
#	train_x = np.concatenate([pd.read_csv(work_dir + "/train.x." + str(lb), header=None, delim_whitespace = True) for lb in np.unique(train_y)],axis=1)
#	test_x = np.concatenate([pd.read_csv(work_dir + "/test.x." + str(lb), header=None, delim_whitespace = True) for lb in np.unique(test_y)],axis=1)

#	logreg = LogisticRegression().fit(train_x, train_y)
#	predicted = logreg.predict(test_x)
#	print("Error rate with logreg: %10.8f" % (1.0 - metrics.accuracy_score(test_y, predicted)))


	
def sklearn_logreg(train_x, train_y, test_x, test_y):
	#train_y = [int(l) for l in open(trainy_source,"r").read().split(' ')]
	#test_y = [int(l) for l in open(testy_source,"r").read().split(' ')]

	#train_x = np.concatenate([pd.read_csv(sc, header=None, delim_whitespace = True) for sc in trainx_sources],axis=1)
	#test_x = np.concatenate([pd.read_csv(sc, header=None, delim_whitespace = True) for sc in testx_sources],axis=1)

	logreg = LogisticRegression().fit(train_x, train_y)
	predicted = logreg.predict(test_x)
	print("Error rate with logreg: %10.8f" % (1.0 - metrics.accuracy_score(test_y, predicted)))
	#print("Confusion matrix:")
	#print(metrics.confusion_matrix(test_y, predicted))

def top_k_logreg_features(k, train_x, train_y):
	#train_y = [int(l) for l in open(trainy_source,"r").read().split(' ')]
	#test_y = [int(l) for l in open(testy_source,"r").read().split(' ')]

	#train_x = np.concatenate([pd.read_csv(sc, header=None, delim_whitespace = True) for sc in trainx_sources],axis=1)
	#test_x = np.concatenate([pd.read_csv(sc, header=None, delim_whitespace = True) for sc in testx_sources],axis=1)

	logreg = LogisticRegression().fit(train_x, train_y)

	coef = logreg.coef_[0]
	sort_index = coef.argsort()
	
	print(','.join(map(str,sort_index[:k][::1])))
	print(','.join(map(str,coef[sort_index[:k][::1]])))
	#print(','.join(map(str,sort_index[-k:][::-1])))
	#print(','.join(map(str,coef[sort_index[-k:][::-1]])))
	
def train_and_test_from_sources():
	# source paths
	trainx_sources = []
	testx_sources = []
	trainy_source = sys.argv[1] + "/train.y"
	testy_source = sys.argv[1] + "/test.y"
	for i in range(1,len(sys.argv)):
		path = sys.argv[i]
		for fn in os.listdir(path):
			if fn.startswith("train.x"):
				trainx_sources.append(path+"/" + fn)
			if fn.startswith("test.x"):
				testx_sources.append(path+"/" + fn)
	trainx_sources.sort()
	testx_sources.sort()
	# read data
	train_y = [int(l) for l in open(trainy_source,"r").read().split(' ')]
	test_y = [int(l) for l in open(testy_source,"r").read().split(' ')]

	train_x = np.concatenate([pd.read_csv(sc, header=None, delim_whitespace = True) for sc in trainx_sources],axis=1)
	test_x = np.concatenate([pd.read_csv(sc, header=None, delim_whitespace = True) for sc in testx_sources],axis=1)

	sklearn_logreg(train_x, train_y, test_x, test_y)
	#top_k_logreg_features(10000, train_x, train_y)
	#print(trainx_sources)
	#print(testx_sources)
	#print(trainy_source)
	#print(testy_source)

train_and_test_from_sources()



