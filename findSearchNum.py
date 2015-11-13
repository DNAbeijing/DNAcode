# -*- coding: utf-8 -*-
"""
Created on Sun Nov 08 15:06:08 2015

@author: Stanley
"""

import urllib
import re
import scipy.stats as ss

####
thresh = 100
####

virusDic = {}

fvirus = open('bnVSbnc.txt')
for i in fvirus.readlines():
    fName = re.findall('\w*',i)[0]
    vName1 = re.findall('\w*',i)[7].split('_')[0]
    vName2 = re.findall('\w*',i)[7].split('_')[1]
    vName = '' + vName1 + ' ' + vName2
    vName = vName1
    if virusDic.has_key(vName):
        virusDic[vName].append(fName)
    else:
        virusDic[vName] = [fName]
fvirus.close()

url1 = 'http://search.who.int/search?q='
url2 = '&site=who'

queryList = []
numList = []

for query in virusDic.keys():
    queryList.append(query)
    url = url1 + query + url2
    mf = urllib.urlopen(url)
    content = mf.read()
    numRaw = re.findall('<M>\d*</M>', content)
    if len(numRaw) == 0:
        num = 0
    else:
        num = re.findall('\d*', numRaw[0])[3]
    if num >= thresh:
        numList.append(int(num))
    print query, num
    
ind = list(ss.rankdata(numList,method='ordinal'))

queryOrder = open('bacteriaqueryOrder.txt', 'w')
queryOrder1 = open('bacteriaqueryOrder1.txt', 'w')
for i in reversed(range(len(queryList))):
    id = int(ind.index(i+1))
    for fileName in virusDic[queryList[id]]:
        queryOrder.write('{0}.txt\t{1}\t{2}\n'.format(fileName, queryList[id], numList[id]))
        queryOrder1.write('{0}.txt\n'.format(fileName))
queryOrder.close()
queryOrder1.close()