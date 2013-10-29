QueryByHumming
==============

A query by humming system based on locality sensitive hashing indexes

基于局部敏感哈希索引的哼唱检索系统实现

论文下载：http://www.sciencedirect.com/science/article/pii/S016516841200326X

根据论文思想实现的哼唱检索系统部分实现，不包括底层LSH和引用他人的代码。

基本思想如下：

哼唱检索中，由于特征的不稳定性，所以核心思想是采用近邻搜索，寻找与查询段相似的候选段。 局部敏感哈希（LSH）是一种近邻搜索的方法，它提供了一种优越的方法来建立高效索引，但实际上仍然缺乏建立并搜索索引的具体实现。 这篇论文提供一套高效算法实现基于LSH的QBH系统。提供一种基于音符的LSH（note-based locality sensitive hashing，NLSH）建立索引 的算法，一种NLSH两级滤波器算法和基于音高的LSH算法（pitch-based locality sensitive hashing，PLSH）来筛选候选片段（candidate fragments）， 一种边界对齐线性缩放（boundary alignment linear scaling，BALS）算法来定位候选片段的准确界限， 一种重音移位递归对齐（key transposition recursive alignment，KTRA）算法来解决重音移位的问题。 实验结果显示，该种方法能实现平均倒数排序（mean reciprocal rank，MRR）为0.873（从歌曲随机位置开始哼唱）， 0.912（从歌曲开头开始哼唱），同比当前最先进的方法的结果提高0.118和0.050.
