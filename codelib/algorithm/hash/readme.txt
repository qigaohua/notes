常用的字符串Hash函数还有ELFHash，APHash等等，都是十分简单有效的方法。这些函数使用位运算使得每一个字符都对最后的函数值产生影响。
另外还有以MD5和SHA1为代表的杂凑函数，这些函数几乎不可能找到碰撞。


常用字符串哈希函数有BKDRHash，APHash，DJBHash，JSHash，RSHash，SDBMHash，PJWHash，ELFHash等等。对于以上几种哈希函数，我对其进行了一个小小的评测。

BKDRHash无论是在实际效果还是 编码实现中，效果都是最突出的。APHash也是较为优秀的算法。
DJBHash,JSHash,RSHash与 SDBMHash各有千秋。
PJWHash与ELFHash效果最差，但得分相似，其算法本质是相似的。
在信息修竞赛中，要本着易于编码调试的原则，个人认为BKDRHash是最适合记忆和使用的。

http://www.cppblog.com/bellgrade/archive/2009/09/29/97565.html

