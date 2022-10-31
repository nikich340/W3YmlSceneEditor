#ifndef SIMPLETRIE_H
#define SIMPLETRIE_H
#include "constants.h"
#include <QVector>
#include <QMap>
#include <QHash>

struct SimpleTrieNode {
    QHash<QChar, int> m_chars;
    QVector<int> m_leavesData;
    SimpleTrieNode() {}
};

class SimpleTrie {
private:
    int m_nodeIndex = -1;
    QVector<SimpleTrieNode> m_nodes;
	
public:
    SimpleTrie() {
        askNode();
    }
    int askNode() {
        m_nodes.push_back( SimpleTrieNode() );
        m_nodeIndex += 1;
        return m_nodeIndex;
    }
    void add(const QString& str, int data) {
        int node_idx = 0;
        upn(i, 0, str.length() - 1) {
            if (m_nodes[node_idx].m_chars.contains(str[i])) {
                node_idx = m_nodes[node_idx].m_chars[str[i]];
            } else {
                int newNode_idx = askNode();
                m_nodes[node_idx].m_chars[str[i]] = newNode_idx;
                node_idx = newNode_idx;
            }
        }
        m_nodes[node_idx].m_leavesData.push_back(data);
    }
    QVector<int> datasForPrefix(const QString& str, int max = -1) {
        int node_idx = 0;
        upn(i, 0, str.length() - 1) {
            if (m_nodes[node_idx].m_chars.contains(str[i]))
                node_idx = m_nodes[node_idx].m_chars[str[i]];
            else
                return QVector<int>();
        }

        max = std::min( max, m_nodes[node_idx].m_leavesData.size() );
        if (max < 0) {
            return m_nodes[node_idx].m_leavesData;
        }

        return QVector<int>( m_nodes[node_idx].m_leavesData.cbegin(), m_nodes[node_idx].m_leavesData.cbegin() + max );
    }
};
#endif // SIMPLETRIE_H
