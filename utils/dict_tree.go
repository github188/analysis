package main
import "fmt"


type DictNode struct {
    tag rune
    left_child *DictNode
    right_sibling *DictNode
}


func (node *DictNode) isLeafNode() bool {
    return (nil == node.left_child)
}


func (root *DictNode) InsertWord(proot **DictNode, word string) {
    next := 0
    tags := []rune(word)

    if root != *proot {
        panic("dont do this")
    }

    parent := proot
    for next < len(tags) {
        node := *parent

        if nil == node {
            tnd := new(DictNode)

            tnd.tag = tags[next]
            next++

            *parent = tnd
            parent = &tnd.left_child
            continue
        }

        if tags[next] == node.tag {
            // tag已经存在
            next++

            parent = &node.left_child
            continue
        }

        parent = &node.right_sibling
    }
}


// 执行词汇匹配，返回能匹配的最大长度
func (root *DictNode) LongestMatch(word string) int {
    count := 0
    tags := []rune(word)

    node := root
    for nil != node {
        parent := node

        if tags[count] == node.tag {
            count++;
            node = parent.left_child
            continue
        }

        if nil == parent.right_sibling {
            break;
        }

        node = parent.right_sibling
    }

    return count
}


// 打印字典树
func (root *DictNode) DumpDictTree() {
    node := root

    if nil == node {
        return
    }

    if node.isLeafNode() {
        fmt.Println(string(node.tag))
    } else {
        if node.tag > 0 {
            fmt.Println(string(node.tag))
        }
        node.left_child.DumpDictTree()
        node.right_sibling.DumpDictTree()
    }
}


func main() {}
