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

from src.timeseries.TimeSeriesLoader import uv_load
from src.transformation.SFA import *


class MySFA:
	def __init__(self,ucr_data):
		self.sfa = {}
		self.train, self.test = uv_load(ucr_data) # store train data only
		#self.acscores = np.zeros((self.raw["Samples"],self.raw["Size"]))
	
	def lookup(self,windowLength, wordLength, symbols, sequence, val):
		normMean = True
		key = (windowLength, wordLength, symbols)
		if key not in self.sfa:
			sfa = SFA("EQUI_DEPTH")
			sfa.fitWindowing(self.raw, windowLength, wordLength, symbols, normMean, True)
			self.sfa[key] = []
			for i in range(self.raw["Samples"]):
				sfa_sr = []	
				wordList = sfa.transformWindowing(self.raw[i])
				for word in wordList:
					sfa_sr.append(self.sfaToDWord(word, symbols))
				#print(str(i) + "-th transformed time series SFA word " + "\t" + sfaToWordList(wordList))
				self.sfa[key].append(sfa_sr)
		for i in range(self.raw["Samples"]):
			for j in range(len(self.sfa[key][i])):
				if sequence in self.sfa[key][i][j]:					
					self.acscores[i,j:(j+windowLength)] += val
			
	def toMultiSFA(self,train_file,test_file):
		ftrain = open(train_file,'w')
		ftest = open(test_file,'w')
		normMean = True
		wordLength = 8
		symbols = 4
		minwl = 20
		windowLength = minwl
		L = self.train["Size"]
		step = np.sqrt(L - 10)
		config = 0
		while windowLength < (L - 10):
			print(windowLength)
			#sfa = SFA("EQUI_DEPTH")
			#sfa.fitWindowing(self.train, windowLength, wordLength, symbols, normMean, True)			
			#for i in range(self.train["Samples"]):
			#	sfa_set = set()
			#	wordList = sfa.transformWindowing(self.train[i])
			#	for word in wordList:
			#		sfa_set.add(self.sfaToDWord(word, symbols))
			#	sfa_str = str(config) + ' ' + str(self.train[i].label) + ' ' + ' '.join(sfa_set) + '\n'
			#	ftrain.write(sfa_str)
			#for i in range(self.test["Samples"]):
			#	sfa_set = set()
			#	wordList = sfa.transformWindowing(self.test[i])
			#	for word in wordList:
			#		sfa_set.add(self.sfaToDWord(word, symbols))
			#	sfa_str = str(config) + ' ' + str(self.test[i].label) + ' ' + ' '.join(sfa_set) + '\n'
			#	ftest.write(sfa_str)
			config += 1
			#windowLength = 	int(minwl + config*np.sqrt(L-10))
			windowLength = windowLength + int(np.sqrt(L-10))
		ftrain.close()
		ftest.close()
			

	def sfaToDWord(self,word,symbols):
		first_char = ord('0')
		word_string = ""
		for w in word:	
			word_string += chr(first_char + w)
			first_char += symbols
		return word_string
	
	def printSFA(self,windowLength, wordLength, symbols):
		print(self.sfa[(windowLength, wordLength, symbols)])
	
	def scoreToFile(self, path):
		np.savetxt(path, self.acscores, delimiter=",", fmt = "%0.5f")

if __name__ == "__main__":
	transformer = MySFA("MoteStrain")
	transformer.toMultiSFA('motestrain.sfa.train','motestrain.sfa.test')
