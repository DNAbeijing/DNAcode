# -*- coding: utf-8 -*-
"""
Created on Sun Nov 08 15:06:08 2015

@author: Stanley
"""

import urllib
import re

virusDic = {}

fvirus = open('vnVSvnc.txt')
for i in fvirus.readlines():
    fName = re.findall('\w*',i)[0]
    vName = re.findall('\w*',i)[7].split('_')[0]
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
    numList.append(num)