#from __future__ import print_function,division
import sys
import numpy as np
from numpy import array
from numpy.linalg import eig

A = np.loadtxt( sys.stdin )
#print(A)
#A = np.random.rand(512,512)
#A = np.array([[0.9, 0.3], [0.1, 0.7]], float)
x, V = eig(A)
#print(A)
#print(x[0])
print( np.real( np.sort(x)[-2] ) )
#print(V[:,0])
