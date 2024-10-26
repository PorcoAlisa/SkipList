#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <mutex>
using namespace std;
 
#define STORE_FILE "store/dumpFile"  // 存储文件路径
string delimiter = ":";
 
template<class K, class V>
class Node
{
public:
    Node() {}
    Node(const K k, const V v, int level);
    ~Node();
    K GetKey() const;
    V GetVal() const;
    int GetLevel() const;
 
    Node<K, V> **_forwards;
private:
    K _key;
    V _value;
    int _level;
};
 
template<class K, class V>
Node<K, V>::Node(const K k, const V v, int level) {
    _key = k;
    _value = v;
    _level = level;
    _forwards = new Node<K, V> *[level + 1];
    memset(_forwards, 0, sizeof(Node<K, V> *) * (level + 1));
}
 
template<class K, class V>
K Node<K, V>::GetKey() const
{
    return _key;
}
 
template<class K, class V>
V Node<K, V>::GetVal() const
{
    return _value;
}
 
template<class K, class V>
int Node<K, V>::GetLevel() const
{
    return _level;
}
 
template<class K, class V>
Node<K, V>::~Node()
{
    delete[] _forwards;
}
 
template<class K, class V>
class SkipList
{
public:
    SkipList(int maxLevel);
    ~SkipList();
    int GetRandLvl();
    bool SearchElement(K key);
    int InsertElement(const K key, const V value);
    bool DeleteElement(K key);
    void DisplayList();
    void DumpFile();
    int LoadFile();
private:
    int _maxLevel;
    int _curMaxLevel;
    Node<K, V> *_header;
    int _elementCnt;
    ofstream _fileWritor;
    ifstream _fileReador;
    mutex _mtx;
};
 
template<class K, class V>
int SkipList<K, V>::GetRandLvl()
{
    int lvl = 1;
    while (rand() % 2) {
        lvl++;
    }
    lvl = lvl < _maxLevel ? lvl : _maxLevel;
    return lvl;
}
 
template<class K, class V>
SkipList<K, V>::SkipList(int maxLevel):_maxLevel(maxLevel), _curMaxLevel(0), _elementCnt(0)
{
    K k;
    V v;
    _header = new Node<K, V>(k, v, _maxLevel);
}
 
template<class K, class V>
SkipList<K, V>::~SkipList()
{
    Node<K, V> *node = _header->_forwards[0];
    Node<K, V> *nextNode;
    while (node != nullptr) {
        nextNode = node->_forwards[0];
        delete node;
        node = nextNode;
    }
    delete _header;
}
 
template<class K, class V>
int SkipList<K, V>::InsertElement(const K key, const V value)
{
    lock_guard<mutex> lock(_mtx);
    /* 找到需要插入位置的前一个节点 */
    Node<K, V> *curPos = _header; /* _header不会为空 */
    Node<K, V> *lastPos[_maxLevel + 1];
    memset(lastPos, 0, sizeof(Node<K, V> *) * (_maxLevel + 1));
    for (int curLvl = _curMaxLevel; curLvl >= 0; curLvl--) {
        while ((curPos->_forwards[curLvl] != nullptr) && (key < curPos->_forwards[curLvl]->GetKey())) {
            curPos = curPos->_forwards[curLvl];
        }
        lastPos[curLvl] = curPos;
    }
    curPos = curPos->_forwards[0];
    if ((curPos != nullptr) && curPos->GetKey() == key) { /* 说明要插入的key是本来就存在的，返回1 */
        return 1;
    }
 
    int level = GetRandLvl();
    Node<K, V> *node = new Node<K, V>(key, value, level);
 
    /* 如果level比当前_curMaxLevel大 */
    if (level > _curMaxLevel) {
        for (int i = _curMaxLevel + 1; i <= level; i++) {
            lastPos[i] = _header;
        }
        _curMaxLevel = level;
    }
    /* 循环遍历前一个节点从0到当前待插入节点level的所有level，改变它的指向 */
    /* 循环遍历当前待插入节点的全部level，改变它的指向 */
 
    for (int i = 0; i <= level; i++) {
        node->_forwards[i] = lastPos[i]->_forwards[i];
        lastPos[i]->_forwards[i] = node;
    }
    _elementCnt++;
    return 0;
}
 
template<class K, class V>
bool SkipList<K, V>::SearchElement(K key)
{
    /* 找到需要插入位置的前一个节点 */
    lock_guard<mutex> lock(_mtx);
    Node<K, V> *curPos = _header; /* _header不会为空 */
    for (int curLvl = _curMaxLevel; curLvl >= 0; curLvl--) {
        while ((curPos->_forwards[curLvl] != nullptr) && (key < curPos->_forwards[curLvl]->GetKey())) {
            curPos = curPos->_forwards[curLvl];
        }
    }
    curPos = curPos->_forwards[0];
    if ((curPos != nullptr) && curPos->GetKey() == key) { /* 说明要插入的key是本来就存在的，返回1 */
        return true;
    }
    return false;
}
 
template<class K, class V>
bool SkipList<K, V>::DeleteElement(K key)
{
    lock_guard<mutex> lock(_mtx);
    /* 找到需要插入位置的前一个节点 */
    Node<K, V> *curPos = _header; /* _header不会为空 */
    Node<K, V> *lastPos[_maxLevel + 1];
    memset(lastPos, 0, sizeof(Node<K, V> *) * (_maxLevel + 1));
 
    for (int curLvl = _curMaxLevel; curLvl >= 0; curLvl--) {
        while ((curPos->_forwards[curLvl] != nullptr) && (key < curPos->_forwards[curLvl]->GetKey())) {
            curPos = curPos->_forwards[curLvl];
        }
        lastPos[curLvl] = curPos;
    }
    curPos = curPos->_forwards[0];
    if ((curPos == nullptr) || (curPos->GetKey() != key)) { /* 说明要删除的key不存在，返回失败 */
        return false;
    }
 
    for (int i = 0; i <= curPos->GetLevel(); i++) {
        lastPos[i]->_forwards[i] = curPos->_forwards[i];
    }
    delete curPos;
    while (_curMaxLevel > 0 && _header->_forwards[_curMaxLevel] == nullptr) {
        _curMaxLevel--;
    }
    _elementCnt--;
 
    return true;
}
 
template<class K, class V>
void SkipList<K, V>::DisplayList()
{
    Node<K, V> *curPos;
    for (int i = _curMaxLevel; i >= 0; i--) {
        cout << "Level " << i << ": ";
        curPos = _header->_forwards[i];
        while (curPos != nullptr) {
            cout << curPos->GetKey() << ":" << curPos->GetVal() << ";";
            curPos = curPos->_forwards[i];
        }
        cout << endl;
    }
}
 
template<class K, class V>
void SkipList<K, V>::DumpFile()
{
    _fileWritor.open(STORE_FILE);
 
    Node<K, V> *node = _header->_forwards[0];
    while (node != nullptr) {
        _fileWritor << node->GetKey() << ":" << node->GetVal() << "\n";
        node = node->_forwards[0];
    }
    _fileWritor.flush();
    _fileWritor.close();
}
 
template<class K, class V>
int SkipList<K, V>::LoadFile()
{
    _fileReador.open(STORE_FILE);
    string line;
    string key;
    string value;
     
    while (getline(_fileReador, line)) {
        if (line.empty() || line.find(delimiter) == string::npos) {
            return -1; /* 返回加载失败 */
        }
        key = line.substr(0, line.find(delimiter));
        value = line.substr(line.find(delimiter) + 1, line.length());
        if (key.empty() || value.empty()) {
            continue;
        }
        InsertElement(stoi(key), value);
        cout << "key: " << key << ", value: " << value << endl;
    }
    _fileReador.close();
    return 0;
}