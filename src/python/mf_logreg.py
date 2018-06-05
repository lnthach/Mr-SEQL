# Train logistic regression classification model from multiple sources of data
# A source is a directory. Each source should contain at least one train file begins with "train.x" and its corresponding test file begins with "test.x"
# The first source must include "train.y" and "test.y" for labels of train and test set
# All "train.*" files should have equal number of rows
# All "test.*" files should have equal number of rows

import sys
import numpy as np
import pandas as pd
import os

from sklearn import metrics
from sklearn.linear_model import LogisticRegression



# classification with logistic regression
def sklearn_logreg(train_x, train_y, test_x, test_y):

	logreg = LogisticRegression().fit(train_x, train_y)
	predicted = logreg.predict(test_x)
	print("Error rate with logreg: %10.8f" % (1.0 - metrics.accuracy_score(test_y, predicted)))
	#print("Confusion matrix:")
	#print(metrics.confusion_matrix(test_y, predicted))

# sort and keep only top k features
def top_k_logreg_features(k, train_x, train_y):

	logreg = LogisticRegression().fit(train_x, train_y)

	coef = logreg.coef_[0]
	sort_index = coef.argsort()

	print(','.join(map(str,sort_index[:k][::1])))
	print(','.join(map(str,coef[sort_index[:k][::1]])))
	#print(','.join(map(str,sort_index[-k:][::-1])))
	#print(','.join(map(str,coef[sort_index[-k:][::-1]])))

# read data from input
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
